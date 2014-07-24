#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <map>

#include <QOpenGLBuffer>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QColor>
#include <QStringList>

#include "abstractscene.h"
#include "material.h"
#include "animation.h"
#include "cloth.h"

class Camera;
class Light;
class QOpenGLFunctions_4_0_Core;
class ClothHandler;
/************************************************************************/
/* 仿真场景                                                              */
/************************************************************************/
typedef std::map<QString, Animation*> NameToAnimMap;
class Scene : public AbstractScene
{
public:
	Scene( QObject* parent = 0 );
	~Scene();

	virtual void initialize();
	virtual void render();
	virtual void update(float t);
	virtual void resize( int w, int h );

	int						num_clothes() const { return clothes_.size(); }
	QString					avatarDiffyseTexPath() { return avatar_->diffuse_tex_path_; }
	Animation*&				synthesizedAnimation() { return synthetic_animation_; }
	AnimationTableModel*    avatarAnimationModel();
	SkeletonModel*			avatarSkeletonModel();
	NameToAnimMap*          avatarNameAnimationMap() { return &name_animation_; }
	
	
	void importAvatar(const QString& filename);
	// 读入OBJ服装，wunf
	void importCloth(const QString& filename);
	// 为avatar建立动画名称映射表
	void buildNameAnimationMap(Avatar* avatar);	
	// 更新avatar动画
	void updateAvatarAnimation(int frame);	
	void updateAvatarAnimationSim(int frame);
	
	// 切换到绑定姿态
	void restoreToBindpose();						

	void renderAvatar() const;
	void renderClothes() const;

	int totalFrame();

	enum InteractionMode { 
		SELECT,
		ROTATE, 
		PAN, 
		ZOOM,
		CLOTH_ROTATE,
		CLOTH_MOVE,
		CLOTH_SCALE,
		INTERACTION_MODE_COUNT 
	};

	enum DisplayMode { 
		SHADING, 
		SKELETON, 
		XRAY, 
		DISPLAY_MODE_COUNT
	};

	InteractionMode  interactionMode() const { return interaction_mode_; }
	void setInteractionMode( InteractionMode mode ) { interaction_mode_ = mode; }

	DisplayMode displayMode() const { return display_mode_; }
	void setDisplayMode( DisplayMode mode ) { display_mode_ = mode; }

	// Camera control
	void rotate(const QPoint& prevPos, const QPoint& curPos);
	void pan(float dx, float dy);
	void zoom(float factor);  

	// cloth control
	void cloth_rotate(const QPoint& prevPos, const QPoint& curPos);
	void cloth_move(float dx, float dy);
	void cloth_scale(float dy);  

	void initAvatarToSimulate();
	void updateAvatarToSimulate();
	void startSimulate();
	void simulateStep();
	void initCmFile(const char * filename);
	void writeAFrame(int frame);
	void save();
	// 更新cloth动画，wunf
	void updateClothAnimation(int frame);
	void setClothColor(QVector4D color) { color_ = color; cloth_has_texture_ = false; }

	bool is_replay() {return replay_;}
	void load_cm_file(const char * filename);
	void setClothTexture(QString texture_name);

private:
	bool replay_;

private:
	// 禁止拷贝
	Scene( const Scene& );
	Scene& operator=( const Scene& rhs );

	void prepareShaders( const QString& vertexShaderPath, const QString& fragmentShaderPath );

	void prepareFloorVAO();
	void prepareFloorTex();

	void prepareAvatarVAO();
	void prepareAvatarTex();

	// wunf
	//void prepareClothTex();
	void prepareClothVAO();

private:
	const aiScene*					ai_scene_;				// ASSIMP场景
	std::map<QString, Animation*>	name_animation_;		// 动画名称映射表
	Animation*						synthetic_animation_;	// 合成动画

	// 仿真场景物体
	Camera*			camera_;	// 相机
	QVector<Light*>	lights_;	// 灯光
	Avatar*			avatar_;	// 模特
	QVector<zfCloth*>	clothes_;	// 布料
	ClothHandler*	cloth_handler_;

	MaterialPtr	material_;
	// The floor "object"
	QOpenGLVertexArrayObject floor_vao_;
	QVector<QVector3D> floor_pos_;
	QVector<QVector3D> floor_norm_;
	QVector<QVector2D> floor_texcoords_;
	QVector<uint> floor_indices_;
	QOpenGLBuffer floor_pos_buffer_;
	QOpenGLBuffer floor_norm_buffer_;
	QOpenGLBuffer floor_texcoords_buffer_;
	QOpenGLBuffer floor_indices_buffer_;

	GLuint texture_ids_[10];

	QMatrix4x4	model_matrix_;

	DisplayMode			display_mode_;
	QStringList			display_mode_names_;
	QVector<GLuint>		display_mode_subroutines_;

	InteractionMode		interaction_mode_;
	QStringList			interaction_mode_names_;

	QOpenGLFunctions_4_0_Core*	glfunctions_;

	// wunf
	void reset_transform();

	bool cloth_loaded;
	float transform_[8];

	QVector4D color_;

	bool cloth_has_texture_;
};

// UI相关类
// 场景模型
class SceneModel : public QAbstractItemModel
{
public:
	SceneModel( Scene* scene, QObject *parent = 0 );

// 	QVariant data(const QModelIndex &index, int role) const;
// 	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
// 	Qt::ItemFlags flags(const QModelIndex &index) const;
// 	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
// 	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
// 	QModelIndex parent(const QModelIndex &index) const;
// 	int rowCount(const QModelIndex &parent = QModelIndex()) const;
// 	int columnCount(const QModelIndex &parent = QModelIndex()) const;
// 
// private:
// 	enum {COLUMN_COUNT = 1};
// 
// 	Scene* scene_;
};

// 工程文件格式采用json格式
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
