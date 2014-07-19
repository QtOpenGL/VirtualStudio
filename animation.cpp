#include "animation.h"

#include <cmath>
#include <fstream>

#include <QFileInfo>
#include <QMimeData>
#include <QtGui>
#include <QApplication>
#include <QGraphicsScene>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QStyleOptionGraphicsItem>

/************************************************************************/
/* ASSIMP --> Qt数据类型转换函数                                         */
/************************************************************************/
void aiQuatKeyToQautKey( QuaternionKey& qkey, const aiQuatKey& akey )
{
	qkey.value.setScalar(akey.mValue.w);
	qkey.value.setVector(akey.mValue.x, akey.mValue.y, akey.mValue.z);
	qkey.time = akey.mTime;
}
//	aiMatrix4x4
// | a1 a2 a3 a4 |
// | b1 b2 b3 b4 |
// | c1 c2 c3 c4 |
// | d1 d2 d3 d4 |
// 
// QMatrix4x4
// | m11 m12 m13 m14 |
// | m21 m22 m23 m24 |
// | m31 m32 m33 m34 |
// | m41 m42 m43 m44 |
void aiMatToQMat(QMatrix4x4& qmat, const aiMatrix4x4& amat) 
{
	qmat = QMatrix4x4(amat.a1, amat.a2, amat.a3, amat.a4,
		amat.b1, amat.b2, amat.b3, amat.b4,
		amat.c1, amat.c2, amat.c3, amat.c4,
		amat.d1, amat.d2, amat.d3, amat.d4);
}

void aiVecKeyToVecKey( VectorKey& qkey, const aiVectorKey& akey )
{
	qkey.value.setX(akey.mValue.x);
	qkey.value.setY(akey.mValue.y);
	qkey.value.setZ(akey.mValue.z);
	qkey.time = akey.mTime;
}

/************************************************************************/
/* 动画通道                                                             */
/************************************************************************/
AnimationChannel::AnimationChannel( const aiNodeAnim* pNodeAnim )
{
	for (uint pos_key_index = 0; pos_key_index < pNodeAnim->mNumPositionKeys; ++pos_key_index) {
		VectorKey temp;
		aiVecKeyToVecKey(temp, pNodeAnim->mPositionKeys[pos_key_index]);
		position_keys.push_back(temp);
	}

	for (uint rot_key_index = 0; rot_key_index < pNodeAnim->mNumRotationKeys; ++rot_key_index) {
		QuaternionKey temp;
		aiQuatKeyToQautKey(temp, pNodeAnim->mRotationKeys[rot_key_index]);
		rotation_keys.push_back(temp);
	}

	for (uint scale_key_index = 0; scale_key_index < pNodeAnim->mNumScalingKeys; ++scale_key_index) {
		VectorKey temp;
		aiVecKeyToVecKey(temp, pNodeAnim->mScalingKeys[scale_key_index]);
		scaling_keys.push_back(temp);
	}
}

/************************************************************************/
/* 动画                                                                 */
/************************************************************************/
Animation::Animation( const aiAnimation* pAnimation, Avatar* luke )
	: ai_anim(pAnimation), name(pAnimation->mName.data), duration(pAnimation->mDuration), ticks_per_second(pAnimation->mTicksPerSecond), avatar(luke)
{
	for (uint channel_index = 0; channel_index < pAnimation->mNumChannels; ++channel_index) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[channel_index];
		AnimationChannel new_channel(pNodeAnim);
		QString joint_name(pNodeAnim->mNodeName.C_Str());
		new_channel.joint = luke->finddJointByName(joint_name);
		channels.push_back(new_channel);
	}
}

Animation::Animation( const Animation* anim/*, int offset*/ /*= 0*/, double weight /*= 1.0 */ ) 
	: ai_anim(anim->ai_anim), name(anim->name), duration(anim->duration), channels(anim->channels), ticks_per_second(anim->ticks_per_second), avatar(anim->avatar)
{
	//double offset_ticks = offset * 0.001 * ticks_per_second;
	for(int channel_index = 0; channel_index < channels.size(); ++channel_index) {
		AnimationChannel& channel = channels[channel_index];
		for (int key_index = 0; key_index < channel.position_keys.size(); ++key_index) {
			//channel.position_keys[key_index].time += offset_ticks;
			channel.position_keys[key_index].value *= weight;
		}

		for (int key_index = 0; key_index < channel.rotation_keys.size(); ++key_index) {
			//channel.rotation_keys[key_index].time += offset_ticks;
			channel.rotation_keys[key_index].value *= weight;
		}

		for (int key_index = 0; key_index < channel.scaling_keys.size(); ++key_index) {
			//channel.scaling_keys[key_index].time += offset_ticks;
			channel.scaling_keys[key_index].value *= weight;
		}
	}
}

// Animation& Animation::operator=( const Animation& anim )
// {
// 	if (this != &anim) {
// 		ai_anim = anim.ai_anim;
// 		name = anim.name;
// 		duration = anim.duration;
// 		tps = anim.tps;
// 		avatar = anim.avatar;
// 		weight = anim.weight;
// 		channels.clear();
// 	}
// 	return *(this);
// }

void Animation::addKeyframes( const ChannelList& cl, int offset, int length, double weight )
{
	// 效率有待改进
	double offset_ticks = offset * 0.001 * ticks_per_second;
	for(int channel_index = 0; channel_index < cl.size(); ++channel_index) {
		// 找到对应的通道 优化时可考虑用Hashtable加速
		auto it = channels.begin();
		while (it != channels.end()) {
			if (it->joint->name == cl[channel_index].joint->name) {
				break;
			}
			++it;
		}

		Q_ASSERT(it != channels.end());
		// 平移
		for (int key_index = 0; key_index < cl[channel_index].position_keys.size(); ++key_index) {
			VectorKey key = cl[channel_index].position_keys[key_index];
			key.time += offset_ticks;// 单位转换毫秒-->节拍
			key.value *= weight;
			it->position_keys.push_back(key);
		}
		// 旋转
		for (int key_index = 0; key_index < cl[channel_index].rotation_keys.size(); ++key_index) {
			QuaternionKey key = cl[channel_index].rotation_keys[key_index];
			key.time += offset_ticks;
			key.value *= weight;
			it->rotation_keys.push_back(key);
		}
		// 缩放
		for (int key_index = 0; key_index < cl[channel_index].scaling_keys.size(); ++key_index) {
			VectorKey key = cl[channel_index].scaling_keys[key_index];
			key.time += offset_ticks;
			key.value *= weight;
			it->scaling_keys.push_back(key);
		}
	}

	double tmp = (offset + length) * 0.001 * ticks_per_second;
	duration = qMax(duration, tmp);
}

/************************************************************************/
/* 动画片段                                                             */
/************************************************************************/
AnimationClip::AnimationClip( QGraphicsScene *scene, Animation* anim, AnimationTrack* track )
	: QGraphicsObject(), animation_(anim), track_(track), name_(anim->name), y_pos_(0), start_time_(0), weight_(1.0)
{
	if (anim->ticks_per_second) {
		length_ = (anim->duration / anim->ticks_per_second) * 1000; // 节拍数换算成时长
	}
	else {
		length_ = anim->duration * 1000;
	}

	setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges | ItemIsFocusable);
	scene->clearSelection();
	scene->addItem(this);
	track->addClip(this);
	setSelected(true);
	setToolTip(tr("Drag the clip to change start time"));
	setFocus();
}

void AnimationClip::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
	Q_UNUSED(painter);
	Q_UNUSED(widget);
	QPen pen("royalblue");
	if (option->state & QStyle::State_Selected) {
		pen.setColor("crimson");
		pen.setStyle(Qt::DotLine);
		pen.setWidth(2);
	}
	QRectF rect = boundingRect();

	painter->setPen(pen);
	QColor color("yellowgreen");
	color.setAlphaF(0.8);
	painter->setBrush(color);
	painter->drawRoundRect(rect, roundness(rect.width()), roundness(rect.height()));

	painter->setFont(QFont("Consolas"));
	painter->setPen(Qt::black);
	painter->drawText(rect, Qt::AlignCenter, name_);
}

QRectF AnimationClip::boundingRect() const
{
	const int Padding = 8;
	QFontMetricsF metrics = qApp->fontMetrics();
	QRectF rect = metrics.boundingRect(name_);
	rect.setWidth(length_ * 0.1);	//注意单位换算
	rect.adjust(0, -Padding, 0, +Padding);
	rect.translate(-rect.center());
	return rect;
}

int AnimationClip::roundness( double size ) const
{
	const int Diameter = 12;
	return 100 * Diameter / int(size);
}

QVariant AnimationClip::itemChange( GraphicsItemChange change, const QVariant &value )
{
	switch (change) {
	case ItemPositionHasChanged:
		if (this->scenePos().x() < width()/2)
			this->setX(width()/2);	// 锁定X值 让item呆在scene内
		this->setY(y_pos_);	// 锁定Y值 让item只在同一轨道上移动

		// TODO: 防止clip 重叠
		// TODO: 检查全部轨道 判断end_frame是否需要更新
		emit clipMoved(this, track_);
		break;
	default:
		break;
	};

	return QGraphicsItem::itemChange(change, value);
}

Avatar* AnimationClip::avatar() const
{
	return animation_->avatar;
}

void AnimationClip::setPos( qreal x, qreal y )
{
	y_pos_ = y; // 锁定y值
	start_time_ = x * 10; // 1像素=10ms
	QGraphicsObject::setPos(x, y);
}

void AnimationClip::setStartTime( int time )
{
	start_time_ = time;
	QGraphicsObject::setPos(time/10, y_pos_);
}

bool startLaterThan(const AnimationClip* lhs, const AnimationClip* rhs)
{
	return lhs->start_time_ < rhs->start_time_;
}

/************************************************************************/
/* 动画轨道                                                             */
/************************************************************************/ 
void AnimationTrack::addClip( AnimationClip* clip )
{
	clips_.append(clip);	
	qSort(clips_.begin(), clips_.end(), startLaterThan); // 按start_time升序排列
}

// UI相关方法
/************************************************************************/
/* 动画表格模型                                                          */
/************************************************************************/
AnimationTableModel::AnimationTableModel( AnimationList* animations, QObject *parent /*= 0*/ )
	: animations_(animations), QAbstractTableModel(parent)
{
	titles_ << tr("Name") << tr("Duration");
}

void AnimationTableModel::clear()
{
	animations_ = nullptr;
	removeRows(0, rowCount(QModelIndex()), QModelIndex());
	//reset();
}

bool AnimationTableModel::isEmpty() const
{
	if (animations_)
		return animations_->empty();
	else
		return false;
}

int AnimationTableModel::rowCount( const QModelIndex& parent ) const
{
	Q_UNUSED(parent);
	if (animations_)
		return static_cast<int>(animations_->size());
	else
		return 0;
}

int AnimationTableModel::columnCount( const QModelIndex& parent ) const
{
	Q_UNUSED(parent);
	return COLUMN_COUNT;
}

QVariant AnimationTableModel::data( const QModelIndex &index, int role ) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::TextAlignmentRole) {
		return int(Qt::AlignVCenter | Qt::AlignHCenter);
	} 
	else if (role == Qt::DisplayRole) {
		if (animations_) {
			if (index.column() == 0) {
				return (*animations_).at(index.row()).name;
			}
			else if (index.column() == 1)
				return (*animations_).at(index.row()).duration;
		}
	}
	else if (role == Qt::UserRole) {
		if (animations_) {
			QString name((*animations_).at(index.row()).name);
			if (name.isEmpty()) {
				name.append("clip_" + QString::number(index.row()));
			}
			return name;
		}
	}
	else if (role == Qt::UserRole+1) {
		if (animations_) {
			return (*animations_).at(index.row()).duration;
		}
	}
	return QVariant();
}

QVariant AnimationTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
			return titles_[section];
		else 
			return section+1;
	}
	return QVariant();
}

QMimeData* AnimationTableModel::mimeData( const QModelIndexList &indexes ) const
{
	QMimeData* mime = new QMimeData;
	QByteArray encodedData;

	QDataStream stream(&encodedData, QIODevice::WriteOnly);

	foreach (QModelIndex index , indexes) {
		if (index.isValid() && animations_) {
			//Animation* anim = itemFromIndex(index);
			QString text = qvariant_cast<QString>(data(index, Qt::UserRole));
			double duration = qvariant_cast<double>(data(index, Qt::UserRole+1));
			stream << text << duration;
		}
	}
	mime->setData("application/x-animation", encodedData);
	return mime;
}

QStringList AnimationTableModel::mimeTypes() const
{
	QStringList types;
	types << "application/x-animation";
	return types;
}

Qt::ItemFlags AnimationTableModel::flags( const QModelIndex &index ) const
{
	if (!index.isValid())
		return 0;
	return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

/************************************************************************/
/* 骨架树型模型                                                          */
/************************************************************************/
SkeletonModel::SkeletonModel( Joint* root, QObject *parent /*= 0*/ )
	: root_(root), QAbstractItemModel(parent)
{
}

QVariant SkeletonModel::data( const QModelIndex &index, int role ) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	Joint *joint = itemFromIndex(index);
	if (!joint)
		return QVariant();

	return joint->name;
}

Qt::ItemFlags SkeletonModel::flags( const QModelIndex &index ) const
{
	if (!index.isValid())
		return 0;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SkeletonModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
	Q_UNUSED(section);
	Q_UNUSED(orientation);
	Q_UNUSED(role);
	return QVariant();
}

QModelIndex SkeletonModel::index( int row, int column, const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	Joint* parentItem = itemFromIndex(parent);
	Joint* child = parentItem->children[row];
	if (!child)
		return QModelIndex();
	return createIndex(row, column, child);
}

QModelIndex SkeletonModel::parent( const QModelIndex &index ) const
{
	Joint* joint = itemFromIndex(index);
	if (!joint)
		return QModelIndex();
	Joint* parent = joint->parent;
	if (!parent)
		return QModelIndex();
	Joint* grandparent = parent->parent;
	if (!grandparent)
		return QModelIndex();

	int row = 0;
	while (row < grandparent->children.size()) {
		if (parent == grandparent->children[row])
			break;
		++row;
	}

	return createIndex(row, 0, parent);
}

int SkeletonModel::rowCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	Joint* parentItem = itemFromIndex(parent);
	if (!parentItem)
		return 0;
	return parentItem->children.size();
}

int SkeletonModel::columnCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	Q_UNUSED(parent);
	return COLUMN_COUNT;
}

Joint* SkeletonModel::itemFromIndex( const QModelIndex &index ) const
{
	if (index.isValid())
		return static_cast<Joint*>(index.internalPointer());
	else
		return root_;
}

/************************************************************************/
/* 虚拟人物                                                             */
/************************************************************************/
Avatar::Avatar(const aiScene* pScene, const QString& filename)
	: ai_scene_(pScene), 
	root_(nullptr),
	has_materials_(ai_scene_->HasMaterials()),
	has_animations_(ai_scene_->HasAnimations()),
	bindposed_( true ),
	gpu_skinning_(false),
	file_dir_(QFileInfo(filename).absolutePath())
{
	// 从ASSIMP读取的数据中抽取骨骼，构造骨架
	for (uint mesh_index = 0; mesh_index < pScene->mNumMeshes; ++mesh_index) {
		const aiMesh* pMesh = pScene->mMeshes[mesh_index];
		for (unsigned int bone_index = 0; bone_index < pMesh->mNumBones; ++bone_index) {
			const aiBone* pJoint = pMesh->mBones[bone_index];
			QString bone_name(pJoint->mName.data);
			name_node_[bone_name] = pScene->mRootNode->FindNode(pJoint->mName);
		}
	}

	root_ = buildSkeleton(ai_scene_->mRootNode, nullptr);
	// 设置关节的索引 用于indexed GPU skinning
	qDebug() << joints_.count();
	for (int i = 0; i < joints_.size(); ++i) {
		joints_[i]->index = i;
		qDebug() << joints_[i]->name << " " << joints_[i]->index;
	}
	skeleton_model_ = new SkeletonModel(root_);
	makeSkinCache(); //2014.3.25
	//makeSkinCacheTest(); //2014.3.25
	//makeSkinCacheTest2(); //2014.3.25
	createVertexBuffers();
	if (has_materials_) {
		loadDiffuseTexture();
	}

	if (has_animations_) {
		makeAnimationCache();	
	}
	// 否则提示用户缺少动画 是否导入动画数据

	animation_model_ = new AnimationTableModel(&animations_);	

	std::ifstream ifs("patameters/avatar_parameter.txt");
	if(!ifs.is_open())
	{
		scale_factor_ = 1.0f;
		xtranslate_ = 0.f;
		ytranslate_ = 0.f;
		ztranslate_ = 0.f;
	}
	else
	{
		ifs >> scale_factor_;
		ifs >> xtranslate_;
		ifs >> ytranslate_;
		ifs >> ztranslate_;
	}
	ifs.close();
}

Avatar::~Avatar()
{
	delete root_;	// 删除某个关节时 会递归删除其子关节
	delete animation_model_;
	delete skeleton_model_;

	// 销毁蒙皮VBO
	for (auto skin_it = skins_.begin();skin_it != skins_.end(); ++skin_it) {
		delete skin_it->position_buffer_;
		delete skin_it->normal_buffer_;
		delete skin_it->texcoords_buffer_;
		delete skin_it->index_buffer_;
	}
}

Joint* Avatar::buildSkeleton( aiNode* pNode, Joint* pParent )
{
	Joint* pJoint = new Joint(pNode->mName.data);
	pJoint->parent = pParent;

	node_joint_[pNode] = pJoint;
	name_joint_[pJoint->name] = pJoint;
	joints_.append(pJoint);

	aiMatToQMat(pJoint->local_transform, pNode->mTransformation);

	calcGlobalTransform(pJoint);

	// 确定关节对应的animation channel索引
	if (has_animations_) {
		pJoint->channel_index = -1;
		const aiAnimation* current_animation = ai_scene_->mAnimations[0];
		for (uint channel_index = 0; channel_index < current_animation->mNumChannels; ++channel_index) {
			if (current_animation->mChannels[channel_index]->mNodeName.data == pJoint->name) {
				pJoint->channel_index = channel_index;
				break;
			}
		}
	}
	// 递归地构造整个人体骨架
	for (uint child_index = 0; child_index < pNode->mNumChildren; ++child_index) {
		Joint* child = buildSkeleton(pNode->mChildren[child_index], pJoint);
		pJoint->children.push_back(child);
	}

	return pJoint;
}

void Avatar::calcGlobalTransform( Joint* pJoint )
{
	pJoint->global_transform = pJoint->local_transform;
	Joint* pParent = pJoint->parent;

	while (pParent) {
		pJoint->global_transform = pParent->local_transform * pJoint->global_transform;
		pParent = pParent->parent;
	}
}

void Avatar::updateTransforms( Joint* pJoint, const QVector<QMatrix4x4>& vTransforms )
{
	if (pJoint->channel_index != -1) {
		if (pJoint->channel_index >= vTransforms.size())
			return;

		pJoint->local_transform = vTransforms[pJoint->channel_index];
	}

	pJoint->global_transform = pJoint->local_transform;
	Joint* pParentJoint = pJoint->parent;

	while (pParentJoint) {
		pJoint->global_transform = pParentJoint->local_transform * pJoint->global_transform;
		pParentJoint = pParentJoint->parent;
	}

	for (auto it = pJoint->children.begin(); it != pJoint->children.end(); ++it)
		updateTransforms(*it, vTransforms);
}

void Avatar::makeAnimationCache() 
{
	animations_.clear();
	for (uint anim_index = 0; anim_index < ai_scene_->mNumAnimations; ++anim_index) {
		const aiAnimation* pAnimation = ai_scene_->mAnimations[anim_index];
		Animation new_animation(pAnimation, this);
		animations_.push_back(new_animation);
	}
}

void Avatar::updateAnimation( Animation& animation, int elapsed_time )
{
	//Animation& animation = animations_[0];
	// 以下所有涉及时间的计算，单位均采用节拍tick
	//elapsed_time = elapsed_time * 0.001 * animation.tps; // elapsed_time单位毫秒

	// 将时间换算成节拍
	double time = elapsed_time * 0.001 * animation.ticks_per_second;

	// #ifdef _DEBUG
	// 	qDebug() << time;
	// #endif

	if (time > animation.duration) 
		return;

	//  	if (animation.duration > 0.0)
	//  		time = std::fmod(time, animation.duration);

	if (transforms_.size() != animation.channels.size())
		transforms_.resize(animation.channels.size());

	// 计算各channel之变换
	for (int channel_index = 0; channel_index < animation.channels.size();  ++channel_index) {
		const AnimationChannel& channel = animation.channels.at(channel_index);
		/************************************************************************/
		/* 位移                                                                 */
		/************************************************************************/
		QVector3D present_position(0, 0, 0);
		if (!channel.position_keys.empty()) {
			int frame = 0;
			while (frame < channel.position_keys.size() - 1) {
				if (time < channel.position_keys[frame+1].time)
					break;
				++frame;
			}

			// 在相邻两个关键帧之间插值
			uint next_frame = (frame+1) % channel.position_keys.size();
			const VectorKey& key = channel.position_keys[frame];
			const VectorKey& next_key = channel.position_keys[next_frame];
			double diff_time = next_key.time - key.time;
			if (diff_time < 0.0)
				diff_time += animation.duration;
			if (diff_time > 0) {
				float factor =  static_cast<float>((time - key.time) / diff_time);
				present_position = key.value + (next_key.value - key.value) * factor;
			} 
			else {
				present_position = key.value;
			}
		}

		/************************************************************************/
		/* 旋转                                                                 */
		/************************************************************************/
		QQuaternion present_rotation(1, 0, 0, 0);
		if (!channel.rotation_keys.empty()) {
			int frame = 0;
			while (frame < channel.rotation_keys.size() - 1) {
				if (time < channel.rotation_keys[frame+1].time)
					break;
				++frame;
			}

			// SLERP
			unsigned int next_frame = (frame+1) % channel.rotation_keys.size();
			const QuaternionKey& key = channel.rotation_keys[frame];
			const QuaternionKey& next_key = channel.rotation_keys[next_frame];
			double diff_time = next_key.time - key.time;
			if (diff_time < 0)
				diff_time += animation.duration;
			if (diff_time > 0) {
				float factor =  static_cast<float>((time - key.time) / diff_time);
				present_rotation = QQuaternion::slerp(key.value, next_key.value, factor);
			} 
			else {
				present_rotation = key.value;
			}
		}

		/************************************************************************/
		/* 缩放                                                                 */
		/************************************************************************/
		QVector3D present_scaling(1, 1, 1);
		if (!channel.scaling_keys.empty()) {
			int frame = 0;
			while (frame < channel.scaling_keys.size() - 1) {
				if (time < channel.scaling_keys[frame+1].time)
					break;
				++frame;
			}

			present_scaling = channel.scaling_keys[frame].value;
		}

		/************************************************************************/
		/* 构造变换矩阵                                                          */
		/************************************************************************/
		// Matrix Pallette
		QMatrix4x4& mat = transforms_[channel_index];
		mat.setToIdentity();
		mat.translate(present_position);
		mat.rotate(present_rotation);
		mat.scale(present_scaling);
	}

	updateTransforms(root_, transforms_);
	updateJointMatrices();
}

void Avatar::makeSkinCache()
{
	skins_.clear();

	for (uint mesh_index = 0; mesh_index < ai_scene_->mNumMeshes; ++mesh_index) {
		const aiMesh* pMesh = ai_scene_->mMeshes[mesh_index];
		Skin new_skin;
		new_skin.bindpose_pos.resize(pMesh->mNumVertices);
		new_skin.bindpose_shrinked.resize(pMesh->mNumVertices);
		new_skin.bindpose_norm.resize(pMesh->mNumVertices);
		new_skin.positions.resize(pMesh->mNumVertices);
		new_skin.normals.resize(pMesh->mNumVertices);
		new_skin.texcoords.resize(pMesh->mNumVertices);
		new_skin.indices.resize(pMesh->mNumFaces * 3);
		new_skin.skininfo.resize(pMesh->mNumVertices);
		new_skin.joint_indices_.resize(pMesh->mNumVertices);
		new_skin.joint_weights_.resize(pMesh->mNumVertices);
		new_skin.texid = pMesh->mMaterialIndex;

		// 缓存蒙皮信息
		for (uint bone_index = 0; bone_index < pMesh->mNumBones; ++bone_index) {
			const aiBone* pBone = pMesh->mBones[bone_index];
			const QString bone_name(pBone->mName.C_Str());
			NameToJointMap::iterator it = name_joint_.find(bone_name);
			Joint* joint = it->second;
			// 注意设置关节的inverse bindpose matrix
			aiMatToQMat(joint->inverse_bindpose_matrix, pBone->mOffsetMatrix);

			for (uint weight_index = 0; weight_index < pBone->mNumWeights; ++weight_index) {
				uint vertex_index = pBone->mWeights[weight_index].mVertexId;
				new_skin.skininfo[vertex_index].joint_weights.append(qMakePair(joint, pBone->mWeights[weight_index].mWeight));
			}
		}

		new_skin.num_triangles = pMesh->mNumFaces;
		for (unsigned int face_index = 0; face_index < pMesh->mNumFaces; ++face_index) {
			aiFace& face = pMesh->mFaces[face_index];
			new_skin.indices[face_index * 3    ] = face.mIndices[0];
			new_skin.indices[face_index * 3 + 1] = face.mIndices[1];
			new_skin.indices[face_index * 3 + 2] = face.mIndices[2];
		}


		for (uint vertex_index = 0; vertex_index < pMesh->mNumVertices; ++vertex_index) {
			new_skin.bindpose_pos[vertex_index].setX(pMesh->mVertices[vertex_index].x);
			new_skin.bindpose_pos[vertex_index].setY(pMesh->mVertices[vertex_index].y);
			new_skin.bindpose_pos[vertex_index].setZ(pMesh->mVertices[vertex_index].z);

			new_skin.bindpose_shrinked[vertex_index].setX(pMesh->mVertices[vertex_index].x * scale_factor_);
			new_skin.bindpose_shrinked[vertex_index].setY(pMesh->mVertices[vertex_index].y * scale_factor_);
			new_skin.bindpose_shrinked[vertex_index].setZ(pMesh->mVertices[vertex_index].z * scale_factor_);

			new_skin.bindpose_norm[vertex_index].setX(pMesh->mNormals[vertex_index].x); 
			new_skin.bindpose_norm[vertex_index].setY(pMesh->mNormals[vertex_index].y);
			new_skin.bindpose_norm[vertex_index].setZ(pMesh->mNormals[vertex_index].z);

			//new_skin.normals[vertex_index].setX(pMesh->mNormals[vertex_index].x); 
			//new_skin.normals[vertex_index].setY(pMesh->mNormals[vertex_index].y);
			//new_skin.normals[vertex_index].setZ(pMesh->mNormals[vertex_index].z);


			if ( (pMesh->mTextureCoords != nullptr) && (pMesh->mTextureCoords[0] != nullptr) ) {
				new_skin.texcoords[vertex_index].setX(pMesh->mTextureCoords[0][vertex_index].x);
				new_skin.texcoords[vertex_index].setY(pMesh->mTextureCoords[0][vertex_index].y);
			} 

			// 生成关节索引 关节权重数组
			// 最多受四个关节影响
			Q_ASSERT(new_skin.skininfo[vertex_index].joint_weights.size() <= Skin::MAX_NUM_WEIGHTS);
			QVector<QPair<Joint*, float> >& jw = new_skin.skininfo[vertex_index].joint_weights;

			int x = 0; float p = 0;
			int y = 0; float q = 0;
			int z = 0; float r = 0;
			int w = 0; float s = 0;

			Q_ASSERT(!jw.isEmpty());
			x = jw[0].first->index;
			p = jw[0].second;

			if (jw.size() > 1) {
				y = jw[1].first->index;
				q = jw[1].second;
			}
			if (jw.size() > 2) {
				z = jw[2].first->index;
				r = jw[2].second;
			}
			if (jw.size() > 3) {
				w = jw[3].first->index;
				s = jw[3].second;
			}

			new_skin.joint_indices_[vertex_index].setX(x);
			new_skin.joint_indices_[vertex_index].setY(y);
			new_skin.joint_indices_[vertex_index].setZ(z);
			new_skin.joint_indices_[vertex_index].setW(w);
			
			new_skin.joint_weights_[vertex_index].setX(p);
			new_skin.joint_weights_[vertex_index].setY(q);
			new_skin.joint_weights_[vertex_index].setZ(r);
			new_skin.joint_weights_[vertex_index].setW(s);
		}

		skins_.push_back(new_skin);
	}
}

//void Avatar::makeSkinCacheTest()
//{
//	skins_.clear();
//
//	for (uint mesh_index = 0; mesh_index < ai_scene_->mNumMeshes; ++mesh_index) {
//		const aiMesh* pMesh = ai_scene_->mMeshes[mesh_index];
//		Skin new_skin;
//		new_skin.bindpose_pos.resize(pMesh->mNumVertices);
//		new_skin.bindpose_shrinked.resize(pMesh->mNumVertices);
//		new_skin.bindpose_norm.resize(pMesh->mNumVertices);
//		new_skin.positions.resize(pMesh->mNumVertices);
//		new_skin.normals.resize(pMesh->mNumVertices);
//		new_skin.texcoords.resize(pMesh->mNumVertices);
//		new_skin.indices.resize(pMesh->mNumFaces * 3);
//		new_skin.skininfo.resize(pMesh->mNumVertices);
//		new_skin.joint_indices_.resize(pMesh->mNumVertices);
//		new_skin.joint_weights_.resize(pMesh->mNumVertices);
//		new_skin.texid = pMesh->mMaterialIndex;
//
//		// 缓存蒙皮信息
//		for (uint bone_index = 0; bone_index < pMesh->mNumBones; ++bone_index) {
//			const aiBone* pBone = pMesh->mBones[bone_index];
//			const QString bone_name(pBone->mName.C_Str());
//			NameToJointMap::iterator it = name_joint_.find(bone_name);
//			Joint* joint = it->second;
//			// 注意设置关节的inverse bindpose matrix
//			aiMatToQMat(joint->inverse_bindpose_matrix, pBone->mOffsetMatrix);
//
//			for (uint weight_index = 0; weight_index < pBone->mNumWeights; ++weight_index) {
//				uint vertex_index = pBone->mWeights[weight_index].mVertexId;
//				new_skin.skininfo[vertex_index].joint_weights.append(qMakePair(joint, pBone->mWeights[weight_index].mWeight));
//			}
//		}
//
//		new_skin.num_triangles = pMesh->mNumFaces;
//		for (unsigned int face_index = 0; face_index < pMesh->mNumFaces; ++face_index) {
//			aiFace& face = pMesh->mFaces[face_index];
//			new_skin.indices[face_index * 3    ] = face.mIndices[0];
//			new_skin.indices[face_index * 3 + 1] = face.mIndices[1];
//			new_skin.indices[face_index * 3 + 2] = face.mIndices[2];
//		}
//
//
//		for (uint vertex_index = 0; vertex_index < pMesh->mNumVertices; ++vertex_index) {
//			new_skin.bindpose_pos[vertex_index].setX(pMesh->mVertices[vertex_index].x);
//			new_skin.bindpose_pos[vertex_index].setY(pMesh->mVertices[vertex_index].y);
//			new_skin.bindpose_pos[vertex_index].setZ(pMesh->mVertices[vertex_index].z);
//
//			new_skin.bindpose_shrinked[vertex_index].setX(pMesh->mVertices[vertex_index].x * 0.07f); 
//			new_skin.bindpose_shrinked[vertex_index].setY(pMesh->mVertices[vertex_index].y * 0.07f); 
//			new_skin.bindpose_shrinked[vertex_index].setZ(pMesh->mVertices[vertex_index].z * 0.07f);
//
//			new_skin.bindpose_norm[vertex_index].setX(pMesh->mNormals[vertex_index].x); 
//			new_skin.bindpose_norm[vertex_index].setY(pMesh->mNormals[vertex_index].y);
//			new_skin.bindpose_norm[vertex_index].setZ(pMesh->mNormals[vertex_index].z);
//
//			//new_skin.normals[vertex_index].setX(pMesh->mNormals[vertex_index].x); 
//			//new_skin.normals[vertex_index].setY(pMesh->mNormals[vertex_index].y);
//			//new_skin.normals[vertex_index].setZ(pMesh->mNormals[vertex_index].z);
//
//
//			if ( (pMesh->mTextureCoords != nullptr) && (pMesh->mTextureCoords[0] != nullptr) ) {
//				new_skin.texcoords[vertex_index].setX(pMesh->mTextureCoords[0][vertex_index].x);
//				new_skin.texcoords[vertex_index].setY(pMesh->mTextureCoords[0][vertex_index].y);
//			} 
//		}
//
//		skins_.push_back(new_skin);
//	}
//}
//
//void Avatar::makeSkinCacheTest2()
//{
//	skins_.clear();
//
//	for (uint mesh_index = 0; mesh_index < ai_scene_->mNumMeshes; ++mesh_index) {
//		const aiMesh* pMesh = ai_scene_->mMeshes[mesh_index];
//		Skin new_skin;
//		new_skin.bindpose_pos.resize(pMesh->mNumVertices);
//		new_skin.bindpose_shrinked.resize(pMesh->mNumVertices);
//		new_skin.bindpose_norm.resize(pMesh->mNumVertices);
//		new_skin.positions.resize(pMesh->mNumVertices);
//		new_skin.normals.resize(pMesh->mNumVertices);
//		new_skin.texcoords.resize(pMesh->mNumVertices);
//		new_skin.indices.resize(pMesh->mNumFaces * 3);
//		new_skin.skininfo.resize(pMesh->mNumVertices);
//		new_skin.joint_indices_.resize(pMesh->mNumVertices);
//		new_skin.joint_weights_.resize(pMesh->mNumVertices);
//		new_skin.texid = pMesh->mMaterialIndex;
//
//		// 缓存蒙皮信息
//		for (uint bone_index = 0; bone_index < pMesh->mNumBones; ++bone_index) {
//			const aiBone* pBone = pMesh->mBones[bone_index];
//			const QString bone_name(pBone->mName.C_Str());
//			NameToJointMap::iterator it = name_joint_.find(bone_name);
//			Joint* joint = it->second;
//			// 注意设置关节的inverse bindpose matrix
//			aiMatToQMat(joint->inverse_bindpose_matrix, pBone->mOffsetMatrix);
//
//			for (uint weight_index = 0; weight_index < pBone->mNumWeights; ++weight_index) {
//				uint vertex_index = pBone->mWeights[weight_index].mVertexId;
//				new_skin.skininfo[vertex_index].joint_weights.append(qMakePair(joint, pBone->mWeights[weight_index].mWeight));
//			}
//		}
//
//		new_skin.num_triangles = pMesh->mNumFaces;
//		for (unsigned int face_index = 0; face_index < pMesh->mNumFaces; ++face_index) {
//			aiFace& face = pMesh->mFaces[face_index];
//			new_skin.indices[face_index * 3    ] = face.mIndices[0];
//			new_skin.indices[face_index * 3 + 1] = face.mIndices[1];
//			new_skin.indices[face_index * 3 + 2] = face.mIndices[2];
//		}
//
//
//		for (uint vertex_index = 0; vertex_index < pMesh->mNumVertices; ++vertex_index) {
//			new_skin.bindpose_pos[vertex_index].setX(pMesh->mVertices[vertex_index].x);
//			new_skin.bindpose_pos[vertex_index].setY(pMesh->mVertices[vertex_index].y);
//			new_skin.bindpose_pos[vertex_index].setZ(pMesh->mVertices[vertex_index].z);
//
//			new_skin.bindpose_shrinked[vertex_index].setX(pMesh->mVertices[vertex_index].x * 0.07f);
//			new_skin.bindpose_shrinked[vertex_index].setY(pMesh->mVertices[vertex_index].y * 0.07f);
//			new_skin.bindpose_shrinked[vertex_index].setZ(pMesh->mVertices[vertex_index].z * 0.07f); 
//
//			new_skin.bindpose_norm[vertex_index].setX(pMesh->mNormals[vertex_index].x); 
//			new_skin.bindpose_norm[vertex_index].setY(pMesh->mNormals[vertex_index].y);
//			new_skin.bindpose_norm[vertex_index].setZ(pMesh->mNormals[vertex_index].z);
//
//			//new_skin.normals[vertex_index].setX(pMesh->mNormals[vertex_index].x); 
//			//new_skin.normals[vertex_index].setY(pMesh->mNormals[vertex_index].y);
//			//new_skin.normals[vertex_index].setZ(pMesh->mNormals[vertex_index].z);
//
//
//			if ( (pMesh->mTextureCoords != nullptr) && (pMesh->mTextureCoords[0] != nullptr) ) {
//				new_skin.texcoords[vertex_index].setX(pMesh->mTextureCoords[0][vertex_index].x);
//				new_skin.texcoords[vertex_index].setY(pMesh->mTextureCoords[0][vertex_index].y);
//			} 
//		}
//
//		skins_.push_back(new_skin);
//	}
//}

void Avatar::skinning()
{
	if (gpu_skinning_) {
	}
	else {
		for (auto skin_it = skins_.begin(); skin_it != skins_.end(); ++skin_it) {
			for (int info_index = 0; info_index < skin_it->skininfo.size(); ++info_index) {
				VertexInfo& info = skin_it->skininfo[info_index];
				QVector4D position;
				QVector4D normal;
				for (auto jw_it = info.joint_weights.begin(); jw_it != info.joint_weights.end(); ++jw_it) {
					Joint* joint = jw_it->first;
					float weight = jw_it->second;

					QMatrix4x4 transform;
					transform = joint->global_transform * joint->inverse_bindpose_matrix;
					QVector4D pos(skin_it->bindpose_pos[info_index], 1.0);
					position += transform * pos * weight;
					// 法线变换
					QVector4D norm(skin_it->bindpose_norm[info_index]);
					normal += transform * norm * weight;
				}
				skin_it->positions[info_index].setX(position.x() * scale_factor_ + xtranslate_);
				skin_it->positions[info_index].setY(-(position.z() * scale_factor_ + ztranslate_));
				skin_it->positions[info_index].setZ(position.y() * scale_factor_ + ytranslate_);

				skin_it->normals[info_index].setX(normal.x());
				skin_it->normals[info_index].setY(-normal.z());
				skin_it->normals[info_index].setZ(normal.y());
			}
		}
	}
}

//void Avatar::skinningtest()
//{
//	if (gpu_skinning_) {
//	}
//	else {
//		for (auto skin_it = skins_.begin(); skin_it != skins_.end(); ++skin_it) {
//			for (int info_index = 0; info_index < skin_it->skininfo.size(); ++info_index) {
//
//				skin_it->positions[info_index].setX(skin_it->bindpose_pos[info_index].x());
//				skin_it->positions[info_index].setY(skin_it->bindpose_pos[info_index].y());
//				skin_it->positions[info_index].setZ(skin_it->bindpose_pos[info_index].z());
//
//				skin_it->normals[info_index].setX(skin_it->bindpose_norm[info_index].x());
//				skin_it->normals[info_index].setY(skin_it->bindpose_norm[info_index].y());
//				skin_it->normals[info_index].setZ(skin_it->bindpose_norm[info_index].z());
//			}
//		}
//	}
//}

//void Avatar::skinningtest2()
//{
//	if (gpu_skinning_) {
//	}
//	else {
//		for (auto skin_it = skins_.begin(); skin_it != skins_.end(); ++skin_it) {
//			for (int info_index = 0; info_index < skin_it->skininfo.size(); ++info_index) {
//
//				skin_it->positions[info_index].setX(skin_it->bindpose_shrinked[info_index].x());
//				skin_it->positions[info_index].setY(skin_it->bindpose_shrinked[info_index].y());
//				skin_it->positions[info_index].setZ(skin_it->bindpose_shrinked[info_index].z());
//
//				skin_it->normals[info_index].setX(skin_it->bindpose_norm[info_index].x());
//				skin_it->normals[info_index].setY(skin_it->bindpose_norm[info_index].y());
//				skin_it->normals[info_index].setZ(skin_it->bindpose_norm[info_index].z());
//			}
//		}
//	}
//}

Joint* Avatar::finddJointByName( const QString& name ) const
{
	auto it = name_joint_.find(name);
	if (it != name_joint_.end())
		return it->second;
	return nullptr;
}

void Avatar::loadDiffuseTexture()
{
	// 	/* scan scene's materials for textures */
	// 	for (uint material_index = 0; material_index < scene_->mNumMaterials; ++material_index)	{
	// 		int texIndex = 0;
	// 		aiString path;  // filename
	// 
	// 		// 一般只有一个diffuse texture
	// 		aiReturn texFound = scene_->mMaterials[material_index]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
	// 		while (texFound == AI_SUCCESS) {
	// 			//fill map with textures
	// 			diffuse_tex_path_ = file_dir_;
	// 			diffuse_tex_path_.append("/");
	// 			diffuse_tex_path_ += path.data;
	// 			// more textures?
	// 			texIndex++;
	// 			texFound = scene_->mMaterials[material_index]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
	// 		}
	// 	}
	// 仅加载一张diffuse texture
	aiString path;
	aiReturn texFound = ai_scene_->mMaterials[0]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
	if (texFound == AI_SUCCESS) {
		diffuse_tex_path_ = file_dir_;
		diffuse_tex_path_.append("/");
		diffuse_tex_path_ += path.data;
	}
}

bool Avatar::createVertexBuffers()
{
	for (auto skin_it = skins_.begin(); skin_it != skins_.end(); ++skin_it) {
		skin_it->position_buffer_ = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		skin_it->position_buffer_->create();
		if ( !skin_it->position_buffer_->bind() ) {
			qWarning() << "Could not bind vertex buffer to the context";
			return false;
		}
		skin_it->position_buffer_->setUsagePattern( QOpenGLBuffer::DynamicDraw );
		skin_it->position_buffer_->allocate( skin_it->bindpose_shrinked.data(), skin_it->bindpose_shrinked.size() * sizeof(QVector3D) );
		skin_it->position_buffer_->release();

		skin_it->normal_buffer_ = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		skin_it->normal_buffer_->create();
		if ( !skin_it->normal_buffer_->bind() ) {
			qWarning() << "Could not bind normal buffer to the context";
			return false;
		}
		skin_it->normal_buffer_->setUsagePattern(QOpenGLBuffer::DynamicDraw);
		skin_it->normal_buffer_->allocate(skin_it->bindpose_norm.data(), skin_it->bindpose_norm.size() * sizeof(QVector3D));
		skin_it->normal_buffer_->release();

		if (hasMaterials()) {
			skin_it->texcoords_buffer_ = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
			skin_it->texcoords_buffer_->create();
			if (!skin_it->texcoords_buffer_->bind()) {
				qWarning() << "Could not bind uv buffer to the context";
				return false;
			}
			skin_it->texcoords_buffer_->setUsagePattern(QOpenGLBuffer::DynamicDraw);
			skin_it->texcoords_buffer_->allocate(skin_it->texcoords.data(), skin_it->texcoords.size() * sizeof(QVector2D));
			skin_it->texcoords_buffer_->release();
		}

		skin_it->index_buffer_ = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
		skin_it->index_buffer_->create();
		if (!skin_it->index_buffer_->bind()) {
			qWarning() << "Could not bind index buffer to the context";
			return false;
		}
		skin_it->index_buffer_->setUsagePattern(QOpenGLBuffer::DynamicDraw);
		skin_it->index_buffer_->allocate( skin_it->indices.data(), skin_it->indices.size() * sizeof(uint));
		skin_it->index_buffer_->release();

		// 创建关节索引VBO 关节权重VBO
		skin_it->joint_indices_buffer_ = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		skin_it->joint_indices_buffer_->create();
		skin_it->joint_indices_buffer_->bind();
		skin_it->joint_indices_buffer_->setUsagePattern(QOpenGLBuffer::StaticRead);
		skin_it->joint_indices_buffer_->allocate(skin_it->joint_indices_.data(), skin_it->joint_indices_.size() * sizeof(QVector4D));
		skin_it->joint_indices_buffer_->release();

		skin_it->joint_weights_buffer_ = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		skin_it->joint_weights_buffer_->create();
		skin_it->joint_weights_buffer_->bind();
		skin_it->joint_weights_buffer_->setUsagePattern(QOpenGLBuffer::StaticRead);
		skin_it->joint_weights_buffer_->allocate(skin_it->joint_weights_.data(), skin_it->joint_weights_.size() * sizeof(QVector4D));
		skin_it->joint_weights_buffer_->release();
	}

	return true;
}

bool Avatar::bindposed() const
{
	return bindposed_;
}

void Avatar::setBindposed(bool val)
{
	bindposed_ = val;
	// 根据是否处于绑定姿态 更新蒙皮VBO VAO
	if (bindposed_) {
		for (auto skin_it = skins_.begin(); skin_it != skins_.end(); ++skin_it) {
			skin_it->position_buffer_->bind();
			skin_it->position_buffer_->allocate(skin_it->bindpose_shrinked.data(), skin_it->bindpose_shrinked.size() * sizeof(QVector3D));
			skin_it->position_buffer_->release();

			skin_it->normal_buffer_->bind();
			skin_it->normal_buffer_->allocate(skin_it->bindpose_norm.data(), skin_it->bindpose_norm.size() * sizeof(QVector3D));
			skin_it->normal_buffer_->release();
		}
	}
	else {
		if (!gpu_skinning_) {
			for (auto skin_it = skins_.begin(); skin_it != skins_.end(); ++skin_it) {
				skin_it->position_buffer_->bind();
				skin_it->position_buffer_->allocate(skin_it->positions.data(), skin_it->positions.size() * sizeof(QVector3D));
				skin_it->position_buffer_->release();

				skin_it->normal_buffer_->bind();
				skin_it->normal_buffer_->allocate(skin_it->normals.data(), skin_it->normals.size() * sizeof(QVector3D));
				skin_it->normal_buffer_->release();
			}
		}
	}
}

void Avatar::updateJointMatrices()
{
	joint_matrices_.resize(joints_.size());
	for (int jm_index = 0; jm_index < joint_matrices_.size(); ++jm_index) {
		joint_matrices_[jm_index] = joints_[jm_index]->global_transform * joints_[jm_index]->inverse_bindpose_matrix;
		qDebug() << joint_matrices_[jm_index];
	}
}
