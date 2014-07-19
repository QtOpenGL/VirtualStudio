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
/* �ؽڣ������ͬ��                                                      */
/************************************************************************/
struct Joint 
{
	QString name;						// �ؽ�����
	Joint* parent;						// ���ؽ�
	QVector<Joint*> children;			// �ӹؽ�
	QMatrix4x4 inverse_bindpose_matrix;	// ��mesh space��bone space (bind pose)֮�任����
	QMatrix4x4 local_transform;			// ����ڸ��ؽڵı任
	QMatrix4x4 global_transform;		// ������ռ��еı任(�ɸ��ؽ��ۻ����˵���)
	int channel_index;					// ����ͨ������
	int index;                          // ���� ����indexed GPU skinning

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
/* ��תֵ                                                               */
/************************************************************************/
struct QuaternionKey 
{
	double time;
	QQuaternion value;
};

/************************************************************************/
/* λ��ֵ ����ֵ                                                         */
/************************************************************************/
// 
struct VectorKey 
{
	double time;
	QVector3D value;
};

/************************************************************************/
/* ASSIMP --> Qt��������ת������                                         */
/************************************************************************/
void aiQuatKeyToQautKey(QuaternionKey& qkey, const aiQuatKey& akey); 
void aiMatToQMat(QMatrix4x4& qmat, const aiMatrix4x4& amat);
void aiVecKeyToVecKey(VectorKey& qkey, const aiVectorKey& akey);

/************************************************************************/
/* ����ͨ��                                                             */
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
/* ������Ϣ                                                             */
/************************************************************************/
struct VertexInfo 
{
	QVector<QPair<Joint*, float> > joint_weights;
};
typedef QVector<VertexInfo> SkinInfo;

class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
/************************************************************************/
/* ��Ƥ                                                                 */
/************************************************************************/
struct Skin 
{
	enum { MAX_NUM_WEIGHTS = 4 };   // ÿ������������ĸ��ؽ�Ӱ��
	SkinInfo		    skininfo;	// ��Ƥ��Ϣ
	QVector<QVector3D>	bindpose_pos;	// ����̬λ��
	QVector<QVector3D>	bindpose_shrinked;	// ����̬λ��
	QVector<QVector3D>  bindpose_norm;  // ����̬����

	// ��ǰ��̬
	QVector<QVector3D>	positions;	
	QVector<QVector3D>	normals;
	QVector<QVector2D>	texcoords;
	QVector<uint>		indices;
	QVector<QVector4D>  joint_indices_; // �ĸ��ؽ��������δ洢��x y z w
	QVector<QVector4D>  joint_weights_; // �ĸ��ؽ�Ȩ�����δ洢��x y z w
	uint			    texid;
	uint			    num_triangles;

	// VBO indexing rendering
	QOpenGLBuffer*	position_buffer_;
	QOpenGLBuffer*	normal_buffer_;
	QOpenGLBuffer*	texcoords_buffer_;
	QOpenGLBuffer*	index_buffer_;

	// ����GPU skinning
	QOpenGLBuffer*  joint_weights_buffer_; // �ؽ�Ȩ��VBO
	QOpenGLBuffer*  joint_indices_buffer_; // �ؽ�����VBO

	// VAO
	QOpenGLVertexArrayObject* vao_;
};
typedef QVector<Skin> SkinList;

class Avatar;
/************************************************************************/
/* ����                                                                 */
/************************************************************************/
class Animation 
{
public:
	Animation(const aiAnimation* pAnimation, Avatar* luke);	// ¬�ˣ���ʹ��ԭ��
	Animation(const Animation* anim/*, int offset = 0*/, double weight = 1.0);	// offset��λ����
// 	Animation& operator=(const Animation& anim);

	void addKeyframes(const ChannelList& cl, int offset, int length, double weight);	// ��ӹؼ�֡ ƫ��ʱ�䵥λ����

	const aiAnimation*	ai_anim;	        // ԭʼASSIMP����	
	QString				name;		        // ����
	double				duration;	        // ������
	double				ticks_per_second;   // ����/�� ticks per second
	ChannelList			channels;	        // ����ͨ�� �ؼ�֡����
	Avatar*				avatar;		        // ����Avatar
};
typedef QList<Animation> AnimationList;

class AnimationTrack;
/************************************************************************/
/* ����Ƭ��                                                             */
/************************************************************************/
// ���ڻ�ϡ���ֵ
// һ���޸Ľ��ںϳɶ����н��� ԭʼ��������Ӱ��
class AnimationClip : public QGraphicsObject
{
	Q_OBJECT

	friend bool startLaterThan(const AnimationClip* lhs, const AnimationClip* rhs); // clip����׼��
	friend AnimationTrack;

public:
	enum { SECOND_WIDTH = 100 }; // ÿһ���Ӧ���Ϊ100����
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
	void clipMoved(AnimationClip*, AnimationTrack* track); // clip�ƶ��� Ҫ����clip������clip���������ص� ������Ҫ�޸�AnimationTrackScene��end_frame

protected:
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	int roundness(double size) const;	// Բ����

private:		
	Animation*		animation_;	// ԭʼ����
	AnimationTrack* track_;		// �������
	QString			name_;		// Ƭ������ Ĭ��Ϊԭʼ��������
	int				length_;	// Ƭ�γ��� ��λ����
	int				y_pos_;		// ��scene������yλ�� ��������yֵ
	int				start_time_;// ��ʼʱ�� ��λ����
	double			weight_;	// ���Ȩ��
	
};
typedef QList<AnimationClip> AnimationClipList;

/************************************************************************/
/* �������                                                             */
/************************************************************************/ 
class AnimationTrack
{
public:
	AnimationTrack() : locked_(false), visible_(true), weight_(1.0) {}
	// TODO ��������ɾ������clip 

	bool isEmpty() const { return clips_.isEmpty(); }
	void addSection(int start, int end) { vacant_sections_.push_back(qMakePair(start, end)); }
	void addClip(AnimationClip* clip);

	QList<AnimationClip*> clips_;
	bool locked_;
	bool visible_;
	double weight_;

private:
	QList< QPair<int, int> > vacant_sections_;	// �������� ���ڷ�ֹ�����ص�
};
typedef QList<AnimationTrack> AnimationTrackList;

// Qt����ͼ��
/************************************************************************/
/* �������ģ��                                                          */
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
/* �Ǽ�����ģ��                                                          */
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

// aiNodeӳ�䵽joint
typedef std::map<const aiNode*, Joint*> NodeToJointMap;

// Nameӳ�䵽aiNode
typedef std::map<QString, const aiNode*> NameToNodeMap;

// Nameӳ�䵽joint
typedef std::map<QString, Joint*> NameToJointMap;

class Scene;
/************************************************************************/
/* ��������                                                             */
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
	Joint* buildSkeleton(aiNode* pNode, Joint* pParent);							// ����Ǽ�	
	void calcGlobalTransform(Joint* pJoint);										// ����ؽ�ȫ�ֱ任	
	void updateTransforms(Joint* pJoint, const QVector<QMatrix4x4>& vTransforms);	// �Ը��ؽ�ʵʩ�任
	void updateJointMatrices();                                                     // ���¾����ɫ��
	void makeAnimationCache();														// ������ASSIMP���صĶ���	
	void makeSkinCache();															// ������Ƥ	
	void makeSkinCacheTest();
	void makeSkinCacheTest2();

	void loadDiffuseTexture();
	bool createVertexBuffers();

	const aiScene*			ai_scene_;	    // ASSIMP����
	Joint*					root_;			// ���ؽ�
	QVector<Joint*>			joints_;		// ���йؽ�
	AnimationList			animations_;	// �ؼ�֡
	QVector<QMatrix4x4>		transforms_;    // �����ؽڵľֲ��任
	QVector<QMatrix4x4>		joint_matrices_;// �����ɫ��(�ؽ�ȫ�ֱ任 * inverse bindpose����)
	SkinList				skins_;			// ��Ƥ
	bool					has_materials_;	// �Ƿ��в�������
	bool					has_animations_;// �Ƿ��ж���
	bool                    gpu_skinning_;  // �Ƿ����GPU��Ƥ
	bool                    bindposed_;     // �Ƿ��ڰ���̬
	QString					file_dir_;		// ģ���ļ�Ŀ¼

	//std::vector<std::tuple<uint, uint, uint> > last_playheads_;	// ��¼��һ�β��ŵĹؼ�֡λ��

	NodeToJointMap	node_joint_;
	NameToNodeMap	name_node_;
	NameToJointMap	name_joint_;
	QString			diffuse_tex_path_;

	// UI��Ա
	AnimationTableModel*	animation_model_;
	SkeletonModel*			skeleton_model_;

	double scale_factor_;
	double xtranslate_;
	double ytranslate_;
	double ztranslate_;
};

#endif //ANIMATION_H
