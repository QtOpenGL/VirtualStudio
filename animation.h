#ifndef ANIMATION_H
#define ANIMATION_H

#include <map>
//#include <tuple>

#include <QMatrix4x4>
#include <QPair>
#include <QVector3D>
#include <QVector2D>
#include <QQuaternion>
#include <QAbstractTableModel>
#include <QAbstractItemModel>
#include <QList>
#include <QStringList>
#include <QGraphicsObject>
#include <QOpenGLVertexArrayObject>

#include <assimp/scene.h>
#include <assimp/matrix4x4.h>

#include "material.h"

/************************************************************************/
/* 关节，与骨骼同义                                                      */
/************************************************************************/
struct Joint 
{
	QString name;						// 关节名称
	Joint* parent;						// 父关节
	QVector<Joint*> children;			// 子关节
	QMatrix4x4 inverse_bindpose_matrix;	// 从mesh space到bone space (bind pose)之变换矩阵
	QMatrix4x4 local_transform;			// 相对于父关节的变换
	QMatrix4x4 global_transform;		// 在世界空间中的变换(由根关节累积至此得来)
	int channel_index;					// 动画通道索引
	int index;                          // 索引 用于indexed GPU skinning

	explicit Joint(const char* sName) 
		: name(sName), parent(nullptr), channel_index(-1), index(-1)	
	{}

	~Joint() 
	{
		qDeleteAll(children.begin(), children.end());
		children.clear();
	}
};

/************************************************************************/
/* 旋转值                                                               */
/************************************************************************/
struct QuaternionKey 
{
	double time;
	QQuaternion value;
};

/************************************************************************/
/* 位移值 缩放值                                                         */
/************************************************************************/
// 
struct VectorKey 
{
	double time;
	QVector3D value;
};

/************************************************************************/
/* ASSIMP --> Qt数据类型转换函数                                         */
/************************************************************************/
void aiQuatKeyToQautKey(QuaternionKey& qkey, const aiQuatKey& akey); 
void aiMatToQMat(QMatrix4x4& qmat, const aiMatrix4x4& amat);
void aiVecKeyToVecKey(VectorKey& qkey, const aiVectorKey& akey);

/************************************************************************/
/* 动画通道                                                             */
/************************************************************************/
struct AnimationChannel 
{
	Joint *joint;
	QVector<VectorKey>		position_keys;
	QVector<QuaternionKey>	rotation_keys;
	QVector<VectorKey>		scaling_keys;

	AnimationChannel() {}
	AnimationChannel(const aiNodeAnim* pNodeAnim);
};
typedef QVector<AnimationChannel> ChannelList;

/************************************************************************/
/* 顶点信息                                                             */
/************************************************************************/
struct VertexInfo 
{
	QVector<QPair<Joint*, float> > joint_weights;
};
typedef QVector<VertexInfo> SkinInfo;

class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
/************************************************************************/
/* 蒙皮                                                                 */
/************************************************************************/
struct Skin 
{
	enum { MAX_NUM_WEIGHTS = 4 };   // 每个顶点最多受四个关节影响
	SkinInfo		    skininfo;	// 蒙皮信息
	QVector<QVector3D>	bindpose_pos;	// 绑定姿态位置
	QVector<QVector3D>	bindpose_shrinked;	// 绑定姿态位置
	QVector<QVector3D>  bindpose_norm;  // 绑定姿态法线

	// 当前姿态
	QVector<QVector3D>	positions;	
	QVector<QVector3D>	normals;
	QVector<QVector2D>	texcoords;
	QVector<uint>		indices;
	QVector<QVector4D>  joint_indices_; // 四个关节索引依次存储于x y z w
	QVector<QVector4D>  joint_weights_; // 四个关节权重依次存储于x y z w
	uint			    texid;
	uint			    num_triangles;

	// VBO indexing rendering
	QOpenGLBuffer*	position_buffer_;
	QOpenGLBuffer*	normal_buffer_;
	QOpenGLBuffer*	texcoords_buffer_;
	QOpenGLBuffer*	index_buffer_;

	// 用于GPU skinning
	QOpenGLBuffer*  joint_weights_buffer_; // 关节权重VBO
	QOpenGLBuffer*  joint_indices_buffer_; // 关节索引VBO

	// VAO
	QOpenGLVertexArrayObject* vao_;
};
typedef QVector<Skin> SkinList;

class Avatar;
/************************************************************************/
/* 动画                                                                 */
/************************************************************************/
class Animation 
{
public:
	Animation(const aiAnimation* pAnimation, Avatar* luke);	// 卢克，快使用原力
	Animation(const Animation* anim/*, int offset = 0*/, double weight = 1.0);	// offset单位毫秒
// 	Animation& operator=(const Animation& anim);

	void addKeyframes(const ChannelList& cl, int offset, int length, double weight);	// 添加关键帧 偏移时间单位毫秒

	const aiAnimation*	ai_anim;	        // 原始ASSIMP动画	
	QString				name;		        // 名称
	double				duration;	        // 节拍数
	double				ticks_per_second;   // 节拍/秒 ticks per second
	ChannelList			channels;	        // 动画通道 关键帧数据
	Avatar*				avatar;		        // 所属Avatar
};
typedef QList<Animation> AnimationList;

class AnimationTrack;
/************************************************************************/
/* 动画片段                                                             */
/************************************************************************/
// 用于混合、插值
// 一切修改仅在合成动画中进行 原始动画不受影响
class AnimationClip : public QGraphicsObject
{
	Q_OBJECT

	friend bool startLaterThan(const AnimationClip* lhs, const AnimationClip* rhs); // clip排序准则
	friend AnimationTrack;

public:
	enum { SECOND_WIDTH = 100 }; // 每一秒对应宽度为100像素
	enum { Type = UserType + 1 };

	AnimationClip(QGraphicsScene *scene, Animation* anim, AnimationTrack* track);

	// @override
	int	type() const { return Type; }
	void setPos( qreal x, qreal y );

	double weight() const { return weight_; }
	void setWeight(double w) { weight_ = w; }
	qreal width() const { return boundingRect().width(); }
	void setStartTime(int time);
	int startTime() const { return start_time_; }

	int length() const { return length_; }
	Avatar* avatar() const;
	ChannelList& channels() { return animation_->channels; }
	Animation* animation() const { return animation_; }

signals:
	void clipMoved(AnimationClip*, AnimationTrack* track); // clip移动了 要检测该clip与其他clip区间有无重叠 可能需要修改AnimationTrackScene的end_frame

protected:
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	int roundness(double size) const;	// 圆角率

private:		
	Animation*		animation_;	// 原始动画
	AnimationTrack* track_;		// 所属轨道
	QString			name_;		// 片段名称 默认为原始动画名称
	int				length_;	// 片段长度 单位毫秒
	int				y_pos_;		// 在scene所处的y位置 用于锁定y值
	int				start_time_;// 起始时刻 单位毫秒
	double			weight_;	// 混合权重
	
};
typedef QList<AnimationClip> AnimationClipList;

/************************************************************************/
/* 动画轨道                                                             */
/************************************************************************/ 
class AnimationTrack
{
public:
	AnimationTrack() : locked_(false), visible_(true), weight_(1.0) {}
	// TODO 析构不必删除所有clip 

	bool isEmpty() const { return clips_.isEmpty(); }
	void addSection(int start, int end) { vacant_sections_.push_back(qMakePair(start, end)); }
	void addClip(AnimationClip* clip);

	QList<AnimationClip*> clips_;
	bool locked_;
	bool visible_;
	double weight_;

private:
	QList< QPair<int, int> > vacant_sections_;	// 空闲区间 用于防止区间重叠
};
typedef QList<AnimationTrack> AnimationTrackList;

// Qt项视图类
/************************************************************************/
/* 动画表格模型                                                          */
/************************************************************************/
class AnimationTableModel : public QAbstractTableModel 
{
public:
	AnimationTableModel(AnimationList* animations, QObject *parent = 0);

	void clear();
	bool isEmpty() const;

	int rowCount(const QModelIndex& parent) const;
	int columnCount(const QModelIndex& parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	//bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;

	QMimeData* mimeData(const QModelIndexList &indexes) const;
	QStringList mimeTypes() const;

private:
	enum {COLUMN_COUNT = 2};

	QStringList titles_;
	AnimationList* animations_;
};

/************************************************************************/
/* 骨架树型模型                                                          */
/************************************************************************/
class SkeletonModel : public QAbstractItemModel
{
public:
	SkeletonModel(Joint* root, QObject *parent = 0);

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
	enum {COLUMN_COUNT = 1};
	Joint* itemFromIndex(const QModelIndex &index) const;
	Joint* root_;
};

// aiNode映射到joint
typedef std::map<const aiNode*, Joint*> NodeToJointMap;

// Name映射到aiNode
typedef std::map<QString, const aiNode*> NameToNodeMap;

// Name映射到joint
typedef std::map<QString, Joint*> NameToJointMap;

class Scene;
/************************************************************************/
/* 虚拟人物                                                             */
/************************************************************************/
class Avatar
{
	friend Scene;
public:
	Avatar(const aiScene* pScene, const QString& filename);
	~Avatar();

	Joint* finddJointByName(const QString& name) const;
	void updateAnimation(Animation& animation, int elapsed_time);
	void skinning();
	void skinningtest();
	void skinningtest2();

	QVector<QMatrix4x4>& jointMatrices() { return joint_matrices_; }

	bool hasAnimations() const		{ return has_animations_; }
	bool hasMaterials() const		{ return has_materials_; }
	const SkinList& skins() const	{ return skins_; }
	
	const AnimationTableModel* animationModel() const	{ return animation_model_; }
	const SkeletonModel* skeletonModel() const			{ return skeleton_model_; }

	bool bindposed() const;
	void setBindposed(bool val);

private:
	Joint* buildSkeleton(aiNode* pNode, Joint* pParent);							// 构造骨架	
	void calcGlobalTransform(Joint* pJoint);										// 计算关节全局变换	
	void updateTransforms(Joint* pJoint, const QVector<QMatrix4x4>& vTransforms);	// 对各关节实施变换
	void updateJointMatrices();                                                     // 更新矩阵调色板
	void makeAnimationCache();														// 缓存由ASSIMP加载的动画	
	void makeSkinCache();															// 缓存蒙皮	
	void makeSkinCacheTest();
	void makeSkinCacheTest2();

	void loadDiffuseTexture();
	bool createVertexBuffers();

	const aiScene*			ai_scene_;	    // ASSIMP场景
	Joint*					root_;			// 根关节
	QVector<Joint*>			joints_;		// 所有关节
	AnimationList			animations_;	// 关键帧
	QVector<QMatrix4x4>		transforms_;    // 各个关节的局部变换
	QVector<QMatrix4x4>		joint_matrices_;// 矩阵调色板(关节全局变换 * inverse bindpose矩阵)
	SkinList				skins_;			// 蒙皮
	bool					has_materials_;	// 是否有材质纹理
	bool					has_animations_;// 是否有动画
	bool                    gpu_skinning_;  // 是否采用GPU蒙皮
	bool                    bindposed_;     // 是否处于绑定姿态
	QString					file_dir_;		// 模型文件目录

	//std::vector<std::tuple<uint, uint, uint> > last_playheads_;	// 记录上一次播放的关键帧位置

	NodeToJointMap	node_joint_;
	NameToNodeMap	name_node_;
	NameToJointMap	name_joint_;
	QString			diffuse_tex_path_;

	// UI成员
	AnimationTableModel*	animation_model_;
	SkeletonModel*			skeleton_model_;

	double scale_factor_;
	double xtranslate_;
	double ytranslate_;
	double ztranslate_;
};

#endif //ANIMATION_H
