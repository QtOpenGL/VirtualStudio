#define NOMINMAX // for the stupid name pollution, you can also use #undef max
#include "scene.h"

#include <limits>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <QString>
#include <QImage>
#include <QtOpenGL/QGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions_4_0_Core>

#include "camera.h"
#include "light.h"
#include "animation_editor_widget.h"
#include "ClothMotion\cloth_motion.h"

/************************************************************************/
/* 仿真场景                                                              */
/************************************************************************/
const QVector4D Scene::ori_color_[4] = { 
	QVector4D(1.0f, 1.0f, 1.0f, 1.0f),
	QVector4D(0.5f, 0.5f, 1.0f, 1.0f),
	QVector4D(0.5f, 1.0f, 0.5f, 1.0f),
	QVector4D(1.0f, 0.5f, 0.5f, 1.0f) };

Scene::Scene( QObject* parent )
	: ai_scene_( nullptr ), 
	  synthetic_animation_( nullptr ), 
	  camera_( new Camera(this) ),
	  avatar_( nullptr ),
	  display_mode_(SHADING),
	  display_mode_subroutines_(DISPLAY_MODE_COUNT),
	  interaction_mode_(ROTATE),
	  glfunctions_(nullptr),
	  // wunf
	  cloth_loaded(false),
	  replay_(false),
	  cloth_has_texture_(false),
	  cloth_handler_(new ClothHandler())
{
	model_matrix_.setToIdentity();

	// Initialize the camera position and orientation

	display_mode_names_ << QStringLiteral( "shade" )
						<< QStringLiteral( "shadeWithNoMaterial" )
						<< QStringLiteral( "shadingwireframe" );

	interaction_mode_names_ << QStringLiteral( "rotate" )
							<< QStringLiteral( "pan" )
							<< QStringLiteral( "zoom" )
							<< QStringLiteral( "select" );
	reset_transform();
}

Scene::~Scene()
{
	for (int i = 0; i < lights_.size(); ++i)
		delete lights_[i];

	delete avatar_;

	for (int i = 0; i < clothes_.size(); ++i)
		delete clothes_[i];

	delete synthetic_animation_;
	delete cloth_handler_;

	aiReleaseImport(ai_scene_);
}

void Scene::initialize()
{
	glfunctions_ = m_context->versionFunctions<QOpenGLFunctions_4_0_Core>();
	if ( !glfunctions_ ) {
		qFatal("Requires OpenGL >= 4.0");
		exit(1);
	}
	glfunctions_->initializeOpenGLFunctions();

	// Initialize resources
	prepareShaders( ":/shaders/lbs.vert", ":/shaders/phong.frag" );
	prepareFloorTex();
	prepareFloorVAO();

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	glClearColor( 0.65f, 0.77f, 1.0f, 1.0f );

	QOpenGLShaderProgramPtr shader = material_->shader();

	// Get subroutine indices DISPLAY_MODE_COUNT
	for ( int i = 0; i < 2; ++i) {
		display_mode_subroutines_[i] =
			glfunctions_->glGetSubroutineIndex( shader->programId(),
			GL_FRAGMENT_SHADER,
			display_mode_names_.at( i ).toLatin1() );
	}
}

void Scene::render()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	material_->bind();
	QOpenGLShaderProgramPtr shader = material_->shader();
	shader->bind();

	// Set the fragment shader display mode subroutine
	glfunctions_->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &display_mode_subroutines_[display_mode_]);

	// Pass in the usual transformation matrices
	model_matrix_.setToIdentity();
	QMatrix4x4 view_matrix = camera_->getViewMatrix();
	QMatrix4x4 mv = view_matrix * model_matrix_;
	QMatrix4x4 projection_matrix = camera_->getProjectionMatrix();
	QMatrix4x4 MVP = projection_matrix * mv;
	shader->setUniformValue("ModelViewMatrix", mv);
	shader->setUniformValue("ViewMatrix", view_matrix);
	shader->setUniformValue("NormalMatrix", mv.normalMatrix());
	shader->setUniformValue("MVP", MVP);  
	shader->setUniformValue("GPUSkinning", false);
	shader->setUniformValue("Light2.Direction", /*view_matrix * */QVector4D(0.0f, -1.0f, 0.0f, 0.0f));
	shader->setUniformValue("Light2.Intensity", QVector3D(1.0f, 1.0f, 1.0f));
	shader->setUniformValue("Material.Ka", QVector3D( 0.5f, 0.5f, 0.5f ));
	shader->setUniformValue("Material.Kd", QVector3D( 0.5f, 0.5f, 0.5f ));
	shader->setUniformValue("Material.Ks", QVector3D( 0.0f, 0.0f, 0.0f ));
	shader->setUniformValue("Material.Shininess", 10.0f);

	// render floor
	/*glfunctions_->glBindTexture(GL_TEXTURE_2D, texture_ids_[0]);
	{
		QOpenGLVertexArrayObject::Binder binder( &floor_vao_ );
		glDrawElements( GL_TRIANGLES, floor_indices_.size(), GL_UNSIGNED_INT, floor_indices_.constData() );
	}*/

	// render avatar
	glfunctions_->glBindTexture(GL_TEXTURE_2D, texture_ids_[1]);
	if (avatar_ && !avatar_->bindposed_ && avatar_->gpu_skinning_) {
		shader->setUniformValueArray("JointMatrices", avatar_->jointMatrices().data(), avatar_->jointMatrices().count());
		shader->setUniformValue("GPUSkinning", true);
	}
	renderAvatar();

	// render cloth 渲染服装，wunf
	if(!cloth_has_texture_)
		glfunctions_->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &display_mode_subroutines_[1]);
	else
		glfunctions_->glBindTexture(GL_TEXTURE_2D, texture_ids_[2]);
	if(cloth_loaded || replay_)
		renderClothes(shader);
	reset_transform();

	shader->release();
}

void Scene::update(float t)
{

}

void Scene::resize( int width, int height )
{
	glViewport( 0, 0, width, height );
	camera_->setViewportWidth(width);
	camera_->setViewportHeight(height);
}

void Scene::importAvatar( const QString& filename )
{
	ai_scene_ = aiImportFile(filename.toStdString().c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_LimitBoneWeights | aiProcess_OptimizeMeshes);
	delete avatar_;
	avatar_ = new Avatar(ai_scene_, filename);
	prepareAvatarTex();  //2014.3.25
	prepareAvatarVAO();

	buildNameAnimationMap(avatar_);	// 建立动画名称映射表 应该移至Avatar内部实现
	aiReleaseImport(ai_scene_);
}

// wunf
void Scene::importCloth(const QString& filename)
{
	SmtClothPtr cloth = ClothHandler::load_cloth_from_obj(filename.toStdString().c_str());
	zfCloth * zfcloth = new zfCloth(cloth);
	cloth_handler_->add_clothes_to_handler(cloth);
	//cloth->load_zfcloth(filename.toStdString().c_str());
	clothes_.push_back(zfcloth);
	color_.push_back(ori_color_[(clothes_.size() - 1) % 4]);
	prepareClothVAO();
	//prepareClothTex();
	cloth_loaded = true;
	replay_ = false;
}

void Scene::renderAvatar() const
{
	if (!avatar_)
		return;

	for (auto skin_it = avatar_->skins().begin(); skin_it != avatar_->skins().end(); ++skin_it) {
		QOpenGLVertexArrayObject::Binder binder( skin_it->vao_ );
		glDrawElements(GL_TRIANGLES, skin_it->indices.size(), GL_UNSIGNED_INT, skin_it->indices.constData());
	}
}

// wunf
void Scene::renderClothes(QOpenGLShaderProgramPtr & shader) const
{
	/*if (!avatar_)
		return;*/
	//clothes_[0]->update(transform_);
	cloth_handler_->transform_cloth(transform_, 0);	
	for(size_t i = 0; i < clothes_.size(); ++i)
	{
		shader->setUniformValue("Color", color_[i]);
		clothes_[i]->cloth_update_buffer();
		QOpenGLVertexArrayObject::Binder binder( clothes_[i]->vao() );
		glDrawArrays(GL_TRIANGLES, 0, clothes_[i]->face_count() * 3);
	}
}

void Scene::reset_transform()
{
	for(int i = 0; i < 3; ++i) transform_[i] = 0.f;
	transform_[3] = 1.f;
	QQuaternion quater;
	quater.fromAxisAndAngle(QVector3D(0.f, 1.f, 0.f), 0.f);
	transform_[4] = quater.x();
	transform_[5] = quater.y();
	transform_[6] = quater.z();
	transform_[7] = quater.scalar();
}

AnimationTableModel* Scene::avatarAnimationModel()
{
	if (avatar_ && avatar_->hasAnimations())
		return avatar_->animation_model_;
	else
		return nullptr;
}

SkeletonModel* Scene::avatarSkeletonModel()
{
	if (avatar_ )
		return avatar_->skeleton_model_;
	else
		return nullptr;
}

void Scene::buildNameAnimationMap(Avatar* avatar)
{
	for (int i = 0; i < avatar->animations_.size(); ++i) {
		if (avatar->animations_[i].name.isEmpty())
			avatar->animations_[i].name = "anim" + QString::number(i);
		name_animation_.insert(std::pair<QString, Animation*>(avatar->animations_[i].name, &(avatar->animations_[i])));
	}
}

void Scene::updateAvatarAnimation(int frame)
{
	if (synthetic_animation_) {	
		avatar_->updateAnimation(*synthetic_animation_, frame * RemixerWidget::getSampleInterval()); //2014.3.25
		avatar_->skinning(); //2014.3.25
		//avatar_->skinningtest(); //2014.3.25
		//avatar_->skinningtest2(); //2014.3.25
	}
	avatar_->setBindposed(false);
}

void Scene::updateAvatarAnimationSim(int frame)
{
	if (synthetic_animation_) {	
		avatar_->updateAnimation(*synthetic_animation_, frame * RemixerWidget::getSimInterval()); //2014.3.25
		avatar_->skinning(); //2014.3.25
		//avatar_->skinningtest(); //2014.3.25
		//avatar_->skinningtest2(); //2014.3.25
	}
	avatar_->setBindposed(false);
}

void Scene::restoreToBindpose()
{
	avatar_->setBindposed(true);
}

void Scene::prepareShaders( const QString& vertexShaderPath, const QString& fragmentShaderPath )
{
	material_ = MaterialPtr(new Material);
	material_->setShaders(vertexShaderPath, fragmentShaderPath);
}

void Scene::prepareFloorTex()
{
	QImage woodImage = QGLWidget::convertToGLFormat(QImage(":/images/wood.png"));
	glfunctions_->glActiveTexture( GL_TEXTURE0 );
	glfunctions_->glGenTextures(10, texture_ids_);
	glfunctions_->glBindTexture(GL_TEXTURE_2D, texture_ids_[0]);
	glfunctions_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, woodImage.width(), woodImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, woodImage.bits());
	glfunctions_->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glfunctions_->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	int loc = material_->shader()->uniformLocation("Tex1");
	glfunctions_->glUniform1i(loc, 0);
	glfunctions_->glActiveTexture( GL_TEXTURE0 );
}

void Scene::prepareFloorVAO()
{
	floor_pos_.push_back(QVector3D(-25, 0, 25));
	floor_pos_.push_back(QVector3D(25, 0, 25));
	floor_pos_.push_back(QVector3D(25, 0, -25));
	floor_pos_.push_back(QVector3D(-25, 0, -25));
	floor_pos_buffer_.create();
	floor_pos_buffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	floor_pos_buffer_.bind();
	floor_pos_buffer_.allocate(floor_pos_.data(), floor_pos_.size() * sizeof(QVector3D));
	floor_pos_buffer_.release();

	for (int i = 0; i < 4; ++i)
		floor_norm_.push_back(QVector3D(0, 1, 0));
	floor_norm_buffer_.create();
	floor_norm_buffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	floor_norm_buffer_.bind();
	floor_norm_buffer_.allocate(floor_norm_.data(), floor_norm_.size() * sizeof(QVector3D));
	floor_norm_buffer_.release();

	floor_texcoords_.push_back(QVector2D(0.0, 1.0));
	floor_texcoords_.push_back(QVector2D(1.0, 1.0));
	floor_texcoords_.push_back(QVector2D(1.0, 0.0));
	floor_texcoords_.push_back(QVector2D(0.0, 0.0));
	floor_texcoords_buffer_.create();
	floor_texcoords_buffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	floor_texcoords_buffer_.bind();
	floor_texcoords_buffer_.allocate(floor_texcoords_.data(), floor_texcoords_.size() * sizeof(QVector2D));
	floor_texcoords_buffer_.release();

	floor_indices_.push_back(0);
	floor_indices_.push_back(1);
	floor_indices_.push_back(2);
	floor_indices_.push_back(0);
	floor_indices_.push_back(2);
	floor_indices_.push_back(3);
	floor_indices_buffer_.create();
	floor_indices_buffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	floor_indices_buffer_.bind();
	floor_indices_buffer_.allocate(floor_indices_.data(), floor_indices_.size() * sizeof(uint));
	floor_indices_buffer_.release();

	floor_vao_.create();
	QOpenGLVertexArrayObject::Binder binder(&floor_vao_);
	QOpenGLShaderProgramPtr shader = material_->shader();
	shader->bind();
	floor_pos_buffer_.bind();
	shader->enableAttributeArray("VertexPosition");
	shader->setAttributeBuffer("VertexPosition", GL_FLOAT, 0, 3);

	floor_norm_buffer_.bind();
	shader->enableAttributeArray("VertexNormal");
	shader->setAttributeBuffer("VertexNormal", GL_FLOAT, 0, 3);

	floor_texcoords_buffer_.bind();
	shader->enableAttributeArray( "VertexTexCoord" );
	shader->setAttributeBuffer( "VertexTexCoord", GL_FLOAT, 0, 2 );
}

void Scene::prepareAvatarVAO()
{
	for (auto skin_it = avatar_->skins_.begin(); skin_it != avatar_->skins_.end(); ++skin_it) {
		skin_it->vao_ = new QOpenGLVertexArrayObject(this);
		skin_it->vao_->create();
		QOpenGLVertexArrayObject::Binder binder( skin_it->vao_ );
		QOpenGLShaderProgramPtr shader = material_->shader();
		shader->bind();

		skin_it->position_buffer_->bind();
		shader->enableAttributeArray( "VertexPosition" );
		shader->setAttributeBuffer( "VertexPosition", GL_FLOAT, 0, 3 );	

		skin_it->normal_buffer_->bind();
		shader->enableAttributeArray( "VertexNormal" );
		shader->setAttributeBuffer( "VertexNormal", GL_FLOAT, 0, 3 );	

		if (avatar_->hasMaterials()) {
			skin_it->texcoords_buffer_->bind();
			shader->enableAttributeArray( "VertexTexCoord" );
			shader->setAttributeBuffer( "VertexTexCoord", GL_FLOAT, 0, 2 );
		}

		// 关节索引和关节权重
		skin_it->joint_indices_buffer_->bind();
		shader->enableAttributeArray("JointIndices");
		shader->setAttributeBuffer("JointIndices", GL_FLOAT, 0, 4);

		skin_it->joint_weights_buffer_->bind();
		shader->enableAttributeArray("JointWeights");
		shader->setAttributeBuffer("JointWeights", GL_FLOAT, 0, 4);
	}
}

void Scene::prepareAvatarTex()
{
	QImage texImage = QGLWidget::convertToGLFormat(QImage(avatar_->diffuse_tex_path_));
	glfunctions_->glActiveTexture( GL_TEXTURE0 );
	glfunctions_->glBindTexture(GL_TEXTURE_2D, texture_ids_[1]);
	glfunctions_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texImage.width(), texImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texImage.bits());
	glfunctions_->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glfunctions_->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	int loc = material_->shader()->uniformLocation("Tex1");
	glfunctions_->glUniform1i(loc, 0);
	glfunctions_->glActiveTexture( GL_TEXTURE0 );
}

//void Scene::prepareClothTex()
//{
//	QImage texImage = QGLWidget::convertToGLFormat(QImage("yama.jpg"));
//	glfunctions_->glActiveTexture( GL_TEXTURE0 );
//	glfunctions_->glBindTexture(GL_TEXTURE_2D, texture_ids_[2]);
//	glfunctions_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texImage.width(), texImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texImage.bits());
//	glfunctions_->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glfunctions_->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//
//	int loc = material_->shader()->uniformLocation("Tex1");
//	glfunctions_->glUniform1i(loc, 0);
//	glfunctions_->glActiveTexture( GL_TEXTURE0 );
//}

// wunf
void Scene::prepareClothVAO()
{
	for(size_t i = 0; i < clothes_.size(); ++i)
	{
		if(!clothes_[i]->vao())
		{
			clothes_[i]->setVAO(new QOpenGLVertexArrayObject(this));
			clothes_[i]->vao()->create();
			QOpenGLVertexArrayObject::Binder binder( clothes_[i]->vao() );
			QOpenGLShaderProgramPtr shader = material_->shader();
			shader->bind();

			clothes_[i]->position_buffer()->bind();
			shader->enableAttributeArray( "VertexPosition" );
			shader->setAttributeBuffer( "VertexPosition", GL_FLOAT, 0, 3 );	

			clothes_[i]->normal_buffer()->bind();
			shader->enableAttributeArray( "VertexNormal" );
			shader->setAttributeBuffer( "VertexNormal", GL_FLOAT, 0, 3 );

			clothes_[i]->texcoord_buffer()->bind();
			shader->enableAttributeArray( "VertexTexCoord" );
			shader->setAttributeBuffer( "VertexTexCoord", GL_FLOAT, 0, 2 );
		}
	}
}

void Scene::rotate( const QPoint& prevPos, const QPoint& curPos )
{
	camera_->rotate(prevPos, curPos);
}

void Scene::pan( float dx, float dy )
{
	camera_->pan(dx, dy);
}

void Scene::zoom( float factor )
{
	camera_->zoom(factor);
}

void Scene::cloth_rotate(const QPoint& prevPos, const QPoint& curPos)
{
	QQuaternion quater = camera_->rotateForCloth(prevPos, curPos);
	transform_[4] = quater.x();
	transform_[5] = quater.y();
	transform_[6] = quater.z();
	transform_[7] = quater.scalar();
}

void Scene::cloth_move(float dx, float dy)
{
	QVector3D vz = camera_->getViewDirection();
	QVector3D vy(0.f, 0.f, 1.f);
	QVector3D vx = QVector3D::crossProduct(vz, vy);
	vx.normalize();
	QVector3D v = -vx * dx * 0.5f + vy * dy * 0.5f;
	transform_[0] = v[0];
	transform_[1] = v[1];
	transform_[2] = v[2];
}

void Scene::cloth_scale(float dy)
{
	transform_[3] = 1.f + dy * 0.1f;
}

int Scene::totalFrame()
{
	double length;
	if (synthetic_animation_->ticks_per_second) {
		length = (synthetic_animation_->duration / synthetic_animation_->ticks_per_second) * 1000; // 节拍数换算成时长
	}
	else {
		length = synthetic_animation_->duration * 1000;
	}
	int total_frame = static_cast<int>(length / RemixerWidget::getSampleInterval());
	return total_frame;
}

void Scene::initAvatarToSimulate()
{
	const Skin & skin = avatar_->skins().at(0);

	size_t size = skin.positions.size();
	double * position = new double[size * 3];
	for(size_t i = 0; i < size; ++i)
	{
		const QVector3D & point = skin.positions[i];
		for(int j = 0; j < 3; ++j)
			position[i * 3 + j] = point[j];
	}

	size = skin.texcoords.size();
	double * texcoord = new double[size * 2];
	for(size_t i = 0; i < size; ++i)
	{
		const QVector3D & tex = skin.texcoords[i];
		for(int j = 0; j < 2; ++j)
			texcoord[i * 2 + j] = tex[j];
	}

	size = skin.indices.size();
	int * indices = new int[size];
	for(size_t i = 0; i < size; ++i)
	{
		const uint & index = skin.indices[i];
		indices[i] = index;
	}

	/*clothes_[0]->initOBS(
		position, 
		texcoord, 
		indices, 
		skin.num_triangles);*/
	cloth_handler_->init_avatars_to_handler(
		position, 
		texcoord, 
		indices, 
		skin.num_triangles);

	delete[] position;
	delete[] texcoord;
	delete[] indices;
}

void Scene::updateAvatarToSimulate()
{
	const Skin & skin = avatar_->skins().at(0);

	size_t size = skin.positions.size();
	double * position = new double[size * 3];
	for(size_t i = 0; i < size; ++i)
	{
		const QVector3D & point = skin.positions[i];
		for(int j = 0; j < 3; ++j)
			position[i * 3 + j] = point[j];
	}

	//clothes_[0]->updateOBS(position);
	cloth_handler_->update_avatars_to_handler(position);
	delete[] position;
}

void Scene::startSimulate()
{
	//clothes_[0]->startSimulate();
	cloth_handler_->begin_simulate();
	//replay_ = true;
	//cloth_loaded = false;
}

void Scene::simulateStep()
{
	//clothes_[0]->simulateStep();
	cloth_handler_->sim_next_step();
}

void Scene::finishedSimulate()
{
	replay_ = true;
}

// wunf
void Scene::updateClothAnimation(int frame)
{
	//clothes_[0]->loadFrame(frame);
	cloth_handler_->load_frame(frame);
}

void Scene::initCmFile(const char * filename)
{
	//clothes_[0]->initCmFile(filename, totalFrame());
}

void Scene::writeAFrame(int frame)
{
	cloth_handler_->write_frame(frame);
}

void Scene::save()
{
	//clothes_[0]->saveCmFile();
}

void Scene::load_cm_file(const char * filename)
{
	/*clothes_.push_back(new zfCloth);
	clothes_[0]->loadCmFile(filename);
	prepareClothVAO();
	replay_ = true;
	cloth_loaded = false;*/
}

void Scene::setClothTexture(QString texture_name)
{
	cloth_has_texture_ = true;
	QImage texImage = QGLWidget::convertToGLFormat(QImage(texture_name));
	glfunctions_->glActiveTexture( GL_TEXTURE0 );
	glfunctions_->glBindTexture(GL_TEXTURE_2D, texture_ids_[2]);
	glfunctions_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texImage.width(), texImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texImage.bits());
	glfunctions_->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glfunctions_->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	int loc = material_->shader()->uniformLocation("Tex1");
	glfunctions_->glUniform1i(loc, 0);
	glfunctions_->glActiveTexture( GL_TEXTURE0 );
}

// UI类
/************************************************************************/
/* 仿真场景对象列表                                                      */
/************************************************************************/
SceneModel::SceneModel( Scene* scene, QObject *parent /*= 0*/ )
	: QAbstractItemModel(parent)
{

}

// QVariant SceneModel::data( const QModelIndex &index, int role ) const
// {
// 
// }
// 
// bool SceneModel::setData( const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/ )
// {
// 
// }
// 
// Qt::ItemFlags SceneModel::flags( const QModelIndex &index ) const
// {
// 
// }
// 
// QVariant SceneModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
// {
// 
// }
// 
// QModelIndex SceneModel::index( int row, int column, const QModelIndex &parent /*= QModelIndex()*/ ) const
// {
// 
// }
// 
// QModelIndex SceneModel::parent( const QModelIndex &index ) const
// {
// 
// }
// 
// int SceneModel::rowCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
// {
// 
// }
// 
// int SceneModel::columnCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
// {
// 
// }
