#ifndef SCENE_H
#define SCENE_H

#include <memory>

#include <QOpenGLBuffer>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QColor>
#include <QStringList>
#include <QTreeWidget>

#include "abstractscene.h"
#include "material.h"
#include "animation.h"
#include "cloth.h"

class Camera;
class Light;
struct DecorativeObject;
class QOpenGLFunctions_4_0_Core;
class ClothHandler;
/************************************************************************/
/* ���泡��                                                              */
/************************************************************************/
class SceneModel;
class Scene : public AbstractScene
{
	friend class SceneModel;
public:
	Scene( QObject* parent = 0 );
	~Scene();

	virtual void initialize();
	virtual void render();
	virtual void update(float t);
	virtual void resize( int w, int h );

	int						numClothes() const;
	Avatar*                 avatar();
	QString					avatarDiffuseTexPath();
	AnimationTableModel*    avatarAnimationTableModel();
	SkeletonTreeModel*		avatarSkeletonTreeModel();
	NameToAnimMap*          avatarNameAnimationMap();
	NameToChIdMap*          avatarNameChannelIndexMap();
	
	void importAvatar(const QString& filename);

	void updateAvatarAnimation(const Animation* anim, int frame);	// ����avatar����
	void restoreToBindpose();						                // �л�������̬

	void renderFloor() const;
	void renderAvatar() const;
	void renderClothes(QOpenGLShaderProgramPtr & shader) const;
	void renderSkeleton() const;

	bool pick(const QPoint& pt);    // ʰȡ�����е�����

	enum InteractionMode 
	{ 
		SELECT,
		ROTATE, 
		TRANSLATE, 
		SCALE, 
		INTERACTION_MODE_COUNT 
	};

	enum DisplayMode 
	{ 
		SHADING, 
		SKELETON, 
		XRAY, 
		DISPLAY_MODE_COUNT
	};

	enum { JOINT_SPHERE_RADIUS = 10 }; // ����ʰȡ�ؽڵİ�Χ��뾶

	InteractionMode  interactionMode() const { return interaction_mode_; }
	void setInteractionMode( InteractionMode mode ) { interaction_mode_ = mode; }

	DisplayMode displayMode() const { return display_mode_; }
	void setDisplayMode( DisplayMode mode ) { display_mode_ = mode; }

	// Camera control
	void rotate(const QPoint& prevPos, const QPoint& curPos);
	void pan(float dx, float dy);
	void zoom(float factor);  

	// wnf��ӣ�����OBJ��װ
	void importCloth(QString file_name);

private:
	// uncopyable
	Scene( const Scene& );
	Scene& operator=( const Scene& rhs );

	void scaleAvatar();
	void prepareFloor();    // ׼���ذ� �ƹ����̨����
	void prepareAvatar();   // ׼��ģ�ػ�������
	void prepareSkeleton(); // ׼���Ǽܻ�������
	void prepareCloth();   // ׼����װ��������
	void reset_transform();

private:
	const aiScene*	ai_scene_;	// ASSIMP����

	// ���泡������
	Camera*			camera_;	// ���
	QVector<Light*>	lights_;	// ��Դ
	Avatar*			avatar_;	// ģ��
	QVector<Cloth*>	clothes_;	// ����

	// װ���Զ���
	DecorativeObject* floor_;
	TexturePtr floor_tex_;
	SamplerPtr floor_sampler_;

	TexturePtr avatar_tex_;
	SamplerPtr avatar_sampler_;

	MaterialPtr	shading_display_material_;
	MaterialPtr simple_line_material_;

	QMatrix4x4	model_matrix_;

	DisplayMode			display_mode_;
	QStringList			display_mode_names_;
	QVector<GLuint>		display_mode_subroutines_;

	InteractionMode		interaction_mode_;
	QStringList			interaction_mode_names_;

	QOpenGLFunctions_4_0_Core*	glfunctions_;
	
	bool is_dual_quaternion_skinning_; // �Ƿ����˫��Ԫ��
	bool is_joint_label_visible_; // �Ƿ���ʾ�ؽ�����
	bool is_normal_visible_;

	// wnf��ӣ���װ����ģ�⹦��ģ��
	typedef size_t ClothIndex;
	ClothHandler * cloth_handler_;
	QVector<QVector4D> color_;
	static const QVector4D ori_color_[4];
	ClothIndex cur_cloth_index_;
	ClothIndex hover_cloth_index_;
	float transform_[8];
};

// UI�����
class SceneGraphTreeWidget : public QTreeWidget
{
public:
	SceneGraphTreeWidget(QWidget* parent = 0);
};

// �����ļ���ʽ����json��ʽ
// *Simulation
// Scene
//	Avatar
//	Cloth
//	Light
//	Camera
//	Force Field (wind gravity)
// *Design
// Pattern

#endif // SCENE_H
