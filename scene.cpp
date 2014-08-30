#include "cmheader.h"
#define NOMINMAX // for the stupid name pollution, you can also use #undef max
#include "scene.h"

#include "camera.h"
#include "light.h"
#include "gadget.h"
#include "bounding_volume.h"
#include "animation_editor_widget.h"
#include "ClothMotion\cloth_motion.h"

const QVector4D Scene::ori_color_[4] = { 
    QVector4D(1.0f, 1.0f, 1.0f, 1.0f),
    QVector4D(0.5f, 0.5f, 1.0f, 1.0f),
    QVector4D(0.5f, 1.0f, 0.5f, 1.0f),
    QVector4D(1.0f, 0.5f, 0.5f, 1.0f) 
};

/************************************************************************/
/* ���泡��                                                              */
/************************************************************************/
Scene::Scene( QObject* parent ) 
    : ai_scene_( nullptr ), 
    camera_( new Camera ),
    floor_(nullptr),
    synthetic_animation_( nullptr ), 
    //camera_( new Camera(this) ),
    avatar_( nullptr ),
    display_mode_(SHADING),
    display_mode_subroutines_(DISPLAY_MODE_COUNT),
    interaction_mode_(ROTATE),
    glfunctions_(nullptr),
    floor_sampler_(new Sampler),
    avatar_sampler_(new Sampler),
    avatar_tex_(new Texture),
    floor_tex_(new Texture),
    is_dual_quaternion_skinning_(true),
    is_joint_label_visible_(false),
    cloth_loaded_(false),
    replay_(false),
    cloth_has_texture_(false),
    cloth_handler_(new ClothHandler())
{
    model_matrix_.setToIdentity();

    // Initialize the camera position and orientation
    display_mode_names_ << QStringLiteral( "shade" )
	<< QStringLiteral( "shadeWithNoMaterial" )
	<< QStringLiteral( "shadeWithPureColor" )
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
    if ( !glfunctions_ ) 
    {
	qFatal("Requires OpenGL >= 4.0");
	exit(-1);
    }
    glfunctions_->initializeOpenGLFunctions();

    // Initialize resources
    shading_display_material_ = MaterialPtr(new Material);
    shading_display_material_->setShaders(":/shaders/skinning.vert", ":/shaders/phong.frag");
    simple_line_material_ = MaterialPtr(new Material);
    simple_line_material_->setShaders(":/shaders/simple.vert", ":/shaders/simple.frag");

    //prepareShaders( ":/shaders/lbs.vert", ":/shaders/phong.frag" );
    //prepareFloorTex();
    //prepareFloorVAO();

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    glClearColor( 0.65f, 0.77f, 1.0f, 1.0f );

    // Ĭ��Ϊshading��ʾ
    QOpenGLShaderProgramPtr shader = shading_display_material_->shader();
    //QOpenGLShaderProgramPtr shader = material_->shader();

    // Get subroutine indices DISPLAY_MODE_COUNT
    for ( int i = 0; i < 3; ++i) 
    {
 	display_mode_subroutines_[i] = 
	glfunctions_->glGetSubroutineIndex( shader->programId(), GL_FRAGMENT_SHADER, display_mode_names_.at( i ).toLatin1() );
    }

    // prepare floor rendering data
    prepareFloor();

    // position the cemara
    camera_->setEye(10, 10, 10);
    camera_->setCenter(0, 0, 0);
}

void Scene::render()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //model_matrix_.setToIdentity();
    QMatrix4x4 view_matrix = camera_->viewMatrix();
    QMatrix4x4 mv = view_matrix * model_matrix_;
    QMatrix4x4 projection_matrix = camera_->projectionMatrix();
    QMatrix4x4 MVP = projection_matrix * mv;

    shading_display_material_->bind();
    //material_->bind();
    QOpenGLShaderProgramPtr shading_display_shader = shading_display_material_->shader();
    //QOpenGLShaderProgramPtr shader = material_->shader();
    shading_display_shader->bind();
    //shader->bind();

    // Set the fragment shader display mode subroutine
    glfunctions_->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &display_mode_subroutines_[display_mode_]);

    // Pass in the usual transformation matrices
    //model_matrix_.setToIdentity();
    //QMatrix4x4 view_matrix = camera_->getViewMatrix();
    //QMatrix4x4 mv = view_matrix * model_matrix_;
    //QMatrix4x4 projection_matrix = camera_->getProjectionMatrix();
    //QMatrix4x4 MVP = projection_matrix * mv;
    shading_display_shader->setUniformValue("ModelViewMatrix", mv);
    shading_display_shader->setUniformValue("NormalMatrix", mv.normalMatrix());
    shading_display_shader->setUniformValue("MVP", MVP);  
    shading_display_shader->setUniformValue("Skinning", false);
    shading_display_shader->setUniformValue("DQSkinning", false);
    shading_display_shader->setUniformValue("Light.Position", view_matrix * QVector4D(1.0f, 1.0f, 1.0f, 0.0f));
    shading_display_shader->setUniformValue("Light.Intensity", QVector3D(0.5f, 0.5f, 0.5f));
    shading_display_shader->setUniformValue("Material.Ka", QVector3D( 0.5f, 0.5f, 0.5f ));
    shading_display_shader->setUniformValue("Material.Kd", QVector3D( 0.5f, 0.5f, 0.5f ));
    shading_display_shader->setUniformValue("Material.Ks", QVector3D( 0.0f, 0.0f, 0.0f ));
    shading_display_shader->setUniformValue("Material.Shininess", 5.0f);

    shading_display_shader->setUniformValue("GPUSkinning", false);
    shading_display_shader->setUniformValue("ViewMatrix", view_matrix);
    shading_display_shader->setUniformValue("Light2.Direction", /*view_matrix * */QVector4D(0.0f, -1.0f, 0.0f, 0.0f));
    shading_display_shader->setUniformValue("Light2.Intensity", QVector3D(1.0f, 1.0f, 1.0f));

    //renderFloor();

    is_dual_quaternion_skinning_ = false;

    switch (display_mode_)
    {
    case SHADING:
        {
	    //glfunctions_->glBindTexture(GL_TEXTURE_2D, texture_ids_[1]);
            /*if (avatar_ && !avatar_->bindposed_) // �ѷϳ������Ƥ ��ȫ����GPU��Ƥ
            {
                shading_display_shader->setUniformValueArray("JointMatrices", avatar_->joint_matrices_.data(), avatar_->joint_matrices_.count());
                shading_display_shader->setUniformValueArray("JointDQs", avatar_->joint_dual_quaternions_.data(), avatar_->joint_dual_quaternions_.count());
                shading_display_shader->setUniformValue("Skinning", true);
                shading_display_shader->setUniformValue("DQSkinning", is_dual_quaternion_skinning_);
            }*/

            renderAvatar();

	    if(!cloth_has_texture_)
		glfunctions_->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &display_mode_subroutines_[1]);
	    else
		glfunctions_->glBindTexture(GL_TEXTURE_2D, texture_ids_[2]);
	    if(cloth_loaded_ || replay_)
		renderClothes(shader);
	    reset_transform();

	    //shader->release();
            shading_display_shader->release();
        }
        break;
    case SKELETON:
        {
            simple_line_material_->bind();
            QOpenGLShaderProgramPtr simple_line_shader = simple_line_material_->shader();
            simple_line_shader->bind();
            simple_line_shader->setUniformValue("MVP", MVP);

            renderSkeleton();

            simple_line_shader->release();
        }
        break;
    case XRAY:
        {
            if (avatar_ && !avatar_->bindposed_)
            {
                shading_display_shader->setUniformValueArray("JointMatrices", avatar_->joint_matrices_.data(), avatar_->joint_matrices_.count());
                shading_display_shader->setUniformValueArray("JointDQs", avatar_->joint_dual_quaternions_.data(), avatar_->joint_dual_quaternions_.count());
                shading_display_shader->setUniformValue("Skinning", true);
                shading_display_shader->setUniformValue("DQSkinning", is_dual_quaternion_skinning_);
            }
            renderAvatar();

            shading_display_shader->release();
        }

        glDisable(GL_DEPTH_TEST);
        {
            simple_line_material_->bind();
            QOpenGLShaderProgramPtr simple_line_shader = simple_line_material_->shader();
            simple_line_shader->bind();
            simple_line_shader->setUniformValue("MVP", MVP);

            renderSkeleton();

            simple_line_shader->release();
        }
        glEnable(GL_DEPTH_TEST);
        break;
    }
}

void Scene::renderForPick()
{
    if(cloth_loaded_ || replay_)
    {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	material_->bind();
	QOpenGLShaderProgramPtr shader = material_->shader();
	shader->bind();

	// Set the fragment shader display mode subroutine
	glfunctions_->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &display_mode_subroutines_[2]);

	// Pass in the usual transformation matrices
	model_matrix_.setToIdentity();
	QMatrix4x4 view_matrix = camera_->/*getV*/viewMatrix();
	QMatrix4x4 mv = view_matrix * model_matrix_;
	QMatrix4x4 projection_matrix = camera_->/*getP*/projectionMatrix();
	QMatrix4x4 MVP = projection_matrix * mv;
	shader->setUniformValue("ModelViewMatrix", mv);
	shader->setUniformValue("ViewMatrix", view_matrix);
	shader->setUniformValue("NormalMatrix", mv.normalMatrix());
	shader->setUniformValue("MVP", MVP);  
	shader->setUniformValue("GPUSkinning", false);

	renderClothesForPick(shader);
	reset_transform();

	shader->release();
    }
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

int Scene::numClothes() const
{
    return clothes_.size();
}

QString Scene::avatarDiffuseTexPath()
{
    Q_ASSERT(avatar_);
    return avatar_->diffuse_tex_path_;
}

NameToAnimMap* Scene::avatarNameAnimationMap()
{
    Q_ASSERT(avatar_);
    return &avatar_->name_animation_;
}

NameToChIdMap* Scene::avatarNameChannelIndexMap()
{
    Q_ASSERT(avatar_);
    return &avatar_->name_channelindex_;
}

void Scene::importAvatar( const QString& filename )
{
    ai_scene_ = aiImportFile(filename.toStdString().c_str(),
    aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_LimitBoneWeights); // ���ǻ� ����ƽ�� ����ÿ���ؽ����4��Ȩ��
    delete avatar_;
    avatar_ = new Avatar(ai_scene_, filename);
    //scaleAvatar();
    prepareAvatar();
    //prepareAvatarTex();  //2014.3.25
    //prepareAvatarVAO();
    prepareSkeleton();
    Sphere s(avatar_->bounding_aabb_);
    camera_->fitBoundingSphere(s);
    aiReleaseImport(ai_scene_);
    //buildNameAnimationMap(avatar_);	// ������������ӳ��� Ӧ������Avatar�ڲ�ʵ��
}

void Scene::renderFloor() const
{
    if (!floor_)
        return;
    //shading_display_material_->setTextureUnitConfiguration(0, floor_tex_, floor_sampler_, QByteArrayLiteral("Tex1"));
    floor_tex_->bind();
    //glfunctions_->glBindTexture(GL_TEXTURE_2D, floor_tex_->textureId());
    QOpenGLVertexArrayObject::Binder binder( floor_->vao );
    glDrawElements(GL_TRIANGLES, floor_->indices.size(), GL_UNSIGNED_INT, floor_->indices.constData());
}

// wunf
void Scene::importCloth(const QString& filename)
{
    SmtClothPtr cloth = ClothHandler::load_cloth_from_obj(filename.toStdString().c_str());
    cloth_handler_->add_clothes_to_handler(cloth);
    //cloth->load_zfcloth(filename.toStdString().c_str());
    clothes_.push_back(zfcloth);
    color_.push_back(ori_color_[(clothes_.size() - 1) % 4]);
    prepareClothVAO();
    //prepareClothTex();
    cloth_loaded_ = true;
    replay_ = false;
    cur_cloth_index_ = clothes_.size() - 1;
}

void Scene::renderAvatar() const
{
    if (!avatar_)
	return;
    //shading_display_material_->setTextureUnitConfiguration(0, avatar_tex_, avatar_sampler_, QByteArrayLiteral("Tex1"));
    avatar_tex_->bind();
    //glfunctions_->glBindTexture(GL_TEXTURE_2D, avatar_tex_->textureId());
    for (auto skin_it = avatar_->skins().begin(); skin_it != avatar_->skins().end(); ++skin_it) 
    {
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
    cloth_handler_->transform_cloth(transform_, cur_cloth_index_);	
    for(size_t i = 0; i < clothes_.size(); ++i)
    {
	QVector4D color = color_[i];
	if(i == hover_cloth_index_)
	    color = color + QVector4D(0.2f, 0.2f, 0.2f, 0.0f);
	shader->setUniformValue("Color", color);
	clothes_[i]->cloth_update_buffer();
	QOpenGLVertexArrayObject::Binder binder( clothes_[i]->vao() );
	glDrawArrays(GL_TRIANGLES, 0, clothes_[i]->face_count() * 3);
    }
}

void Scene::renderClothesForPick(QOpenGLShaderProgramPtr & shader) const
{
    cloth_handler_->transform_cloth(transform_, cur_cloth_index_);
    for(size_t i = 0; i < clothes_.size(); ++i)
    {
	float fred = float(i) * 10 / 255;
	QVector4D color(fred, 0.0f, 0.0f, 1.0f);
	shader->setUniformValue("Color", color);
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

/*void Scene::updateAvatarAnimation(int frame)
{
    if (synthetic_animation_) {	
	avatar_->updateAnimation(*synthetic_animation_, frame * RemixerWidget::getSampleInterval()); //2014.3.25
	avatar_->skinning(); //2014.3.25
	//avatar_->skinningtest(); //2014.3.25
	//avatar_->skinningtest2(); //2014.3.25
    }
    avatar_->setBindposed(false);
}*/

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

void Scene::renderClothes() const
{

}

void Scene::renderSkeleton() const
{
    if (!avatar_)
        return;
    // draw joints and bones
    glPointSize(5.0f);
    QOpenGLVertexArrayObject::Binder joint_binder(avatar_->joint_vao_);
    glDrawArrays(GL_POINTS, 0, avatar_->joint_positions_.size());
    glPointSize(1.0f);

    glLineWidth(2.0f);
    QOpenGLVertexArrayObject::Binder bone_binder(avatar_->bone_vao_);        
    glDrawArrays(GL_LINES, 0, avatar_->bone_positions_.size());
    glLineWidth(1.0f);
}

Avatar* Scene::avatar()
{
    return avatar_;
}

AnimationTableModel* Scene::avatarAnimationTableModel()
{
	if (avatar_ && avatar_->hasAnimations())
		return avatar_->animation_table_model_;
	return nullptr;
}

SkeletonTreeModel* Scene::avatarSkeletonTreeModel()
{
	if (avatar_ )
		return avatar_->skeleton_tree_model_;
	return nullptr;
}

void Scene::updateAvatarAnimation(const Animation* anim, int frame)
{
    avatar_->setBindposed(false);
    if (anim) 
    {	
        avatar_->updateAnimation(anim, frame * AnimationClip::SAMPLE_SLICE);
    }
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

		// �ؽ������͹ؽ�Ȩ��
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

void Scene::scaleAvatar()
{
    Sphere bounding_sphere(avatar_->bounding_aabb_);

    QVector3D vHalf =  bounding_sphere.c;
    float fScale = 10.0f / bounding_sphere.r;

    model_matrix_.setToIdentity();
    model_matrix_.translate(vHalf);
    model_matrix_.scale(fScale);
}

void Scene::prepareAvatar()
{
    // ׼����ƤVAO
	for (auto skin_it = avatar_->skins_.begin(); skin_it != avatar_->skins_.end(); ++skin_it) 
    {
		skin_it->vao_ = new QOpenGLVertexArrayObject(this);
		skin_it->vao_->create();
		QOpenGLVertexArrayObject::Binder binder( skin_it->vao_ );
		QOpenGLShaderProgramPtr shading_display_shader = shading_display_material_->shader();
		shading_display_shader->bind();

		skin_it->position_buffer_->bind();
		shading_display_shader->enableAttributeArray( "VertexPosition" );
		shading_display_shader->setAttributeBuffer( "VertexPosition", GL_FLOAT, 0, 3 );	

		skin_it->normal_buffer_->bind();
		shading_display_shader->enableAttributeArray( "VertexNormal" );
		shading_display_shader->setAttributeBuffer( "VertexNormal", GL_FLOAT, 0, 3 );	

		if (avatar_->hasMaterials()) 
        {
			skin_it->texcoords_buffer_->bind();
			shading_display_shader->enableAttributeArray( "VertexTexCoord" );
			shading_display_shader->setAttributeBuffer( "VertexTexCoord", GL_FLOAT, 0, 2 );
		}

        // �ؽ������͹ؽ�Ȩ��
        skin_it->joint_indices_buffer_->bind();
        shading_display_shader->enableAttributeArray("JointIndices");
        shading_display_shader->setAttributeBuffer("JointIndices", GL_FLOAT, 0, 4);

        skin_it->joint_weights_buffer_->bind();
        shading_display_shader->enableAttributeArray("JointWeights");
        shading_display_shader->setAttributeBuffer("JointWeights", GL_FLOAT, 0, 4);
	}

    // ׼����������
    avatar_sampler_->create();
    avatar_sampler_->setMinificationFilter( GL_LINEAR );
    avatar_sampler_->setMagnificationFilter( GL_LINEAR );
    avatar_sampler_->setWrapMode( Sampler::DirectionS, GL_REPEAT );//GL_CLAMP_TO_EDGE
    avatar_sampler_->setWrapMode( Sampler::DirectionT, GL_REPEAT );//GL_CLAMP_TO_EDGE
    QImage avatarImage(avatar_->diffuse_tex_path_);//
    glfunctions_->glActiveTexture(GL_TEXTURE0);

    avatar_tex_->create();
    avatar_tex_->bind();
    avatar_tex_->setImage(avatarImage);
    shading_display_material_->setTextureUnitConfiguration(0, avatar_tex_, avatar_sampler_, QByteArrayLiteral("Tex1"));
    glfunctions_->glActiveTexture( GL_TEXTURE0 );
}

void Scene::prepareSkeleton()
{
    // ׼���ؽ�
    avatar_->joint_vao_ = new QOpenGLVertexArrayObject(this);
    avatar_->joint_vao_->create();
    QOpenGLVertexArrayObject::Binder joint_binder(avatar_->joint_vao_);
    QOpenGLShaderProgramPtr simple_line_shader = simple_line_material_->shader();
    simple_line_shader->bind();

    avatar_->joint_pos_buffer_->bind();
    simple_line_shader->enableAttributeArray("VertexPosition");
    simple_line_shader->setAttributeBuffer( "VertexPosition", GL_FLOAT, 0, 4 );	

    // ׼���Ǽ�
    avatar_->bone_vao_ = new QOpenGLVertexArrayObject(this);
    avatar_->bone_vao_->create();
    QOpenGLVertexArrayObject::Binder bone_binder(avatar_->bone_vao_);
    simple_line_shader->bind();

    avatar_->bone_pos_buffer_->bind();
    simple_line_shader->enableAttributeArray("VertexPosition");
    simple_line_shader->setAttributeBuffer( "VertexPosition", GL_FLOAT, 0, 4 );	
}

void Scene::prepareFloor()
{
    floor_ = new DecorativeObject();
    float s = 100.f;
    float rep = 100.f;
    float h = 0.0f;
    floor_->positions.append(QVector3D(-s, h, -s)); floor_->normals.append(QVector3D(0.0, 1.0, 0.0)); floor_->texcoords.append(QVector2D(0.f, 0.f));
    floor_->positions.append(QVector3D(-s, h, s)); floor_->normals.append(QVector3D(0.0, 1.0, 0.0)); floor_->texcoords.append(QVector2D(rep, 0.f));
    floor_->positions.append(QVector3D(s, h, s)); floor_->normals.append(QVector3D(0.0, 1.0, 0.0)); floor_->texcoords.append(QVector2D(rep, rep)); 
    floor_->positions.append(QVector3D(s, h, -s)); floor_->normals.append(QVector3D(0.0, 1.0, 0.0)); floor_->texcoords.append(QVector2D(0.f, rep));
    floor_->indices.append(0);
    floor_->indices.append(1);
    floor_->indices.append(2);
    floor_->indices.append(2);
    floor_->indices.append(3);
    floor_->indices.append(0);

    if (!floor_->createVBO())
    {
        qWarning() << "Failed to create floor";
        return;
    }
    // �����ذ�VAO
    floor_->vao = new QOpenGLVertexArrayObject(this);
    floor_->vao->create();
    QOpenGLVertexArrayObject::Binder binder(floor_->vao);
    QOpenGLShaderProgramPtr shading_display_shader = shading_display_material_->shader();
    shading_display_shader->bind();

    floor_->pb->bind();
    shading_display_shader->enableAttributeArray("VertexPosition");
    shading_display_shader->setAttributeBuffer( "VertexPosition", GL_FLOAT, 0, 3 );	

    floor_->nb->bind();
    shading_display_shader->enableAttributeArray( "VertexNormal" );
    shading_display_shader->setAttributeBuffer( "VertexNormal", GL_FLOAT, 0, 3 );	

    floor_->tb->bind();
    shading_display_shader->enableAttributeArray( "VertexTexCoord" );
    shading_display_shader->setAttributeBuffer( "VertexTexCoord", GL_FLOAT, 0, 2 );

    // ���صذ�����
    floor_sampler_->create();
    floor_sampler_->setMinificationFilter(GL_LINEAR);
    floor_sampler_->setMagnificationFilter(GL_LINEAR);
    floor_sampler_->generateMipmap(GL_TEXTURE_2D);
    floor_sampler_->setWrapMode(Sampler::DirectionS, GL_REPEAT);
    floor_sampler_->setWrapMode(Sampler::DirectionT, GL_REPEAT);
    QImage floorImage(":/images/floortile.ppm");
    glfunctions_->glActiveTexture(GL_TEXTURE0);

    floor_tex_->create();
    floor_tex_->bind();
    floor_tex_->setImage(floorImage);
    shading_display_material_->setTextureUnitConfiguration(0, floor_tex_, floor_sampler_, QByteArrayLiteral("Tex1"));
    glfunctions_->glActiveTexture( GL_TEXTURE0 );
}

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

// pt : mouse position
bool Scene::pick(const QPoint& pt)
{
    if (!avatar_)
        return false;
    // Get the Pick ray from the mouse position
    // Compute the vector of the Pick ray in screen space
    QMatrix4x4 matProj = camera_->projectionMatrix();

    // Screen to projection window transformation �Ƶ������������Picking�½�
    QVector3D vPickRayDir;
    vPickRayDir.setX(  ( ( ( 2.0f * pt.x() ) / camera_->viewportWidth()  ) - 1 ) / matProj(0, 0));
    vPickRayDir.setY( -( ( ( 2.0f * pt.y() ) / camera_->viewportHeight() ) - 1 ) / matProj(1, 1));
    vPickRayDir.setZ(1.0f);

    // Get the inverse view matrix
    QMatrix4x4 view_matrix = camera_->viewMatrix();
    QMatrix4x4 model_view = view_matrix * model_matrix_ ;
    QMatrix4x4 m = model_view.inverted();
    QVector3D vPickRayOrig(0, 0, 0);  
    
    // Transform the screen space Pick ray into 3D space
    vPickRayOrig = m * vPickRayOrig;

    QMatrix3x3 normal_matrix = m.normalMatrix(); 
    QMatrix4x4 temp(normal_matrix);
    vPickRayDir = temp * vPickRayDir;
    vPickRayDir.normalize();

    // ���������ؽ� ���Ƿ�ѡ��
    QVector<int> selected_joint_indices;
    for (int i = 0; i < avatar_->joints_.size(); ++i)
    {
        Sphere s(avatar_->joints_[i]->pos().toVector3D(), 2);
        if (IntersectSphere( vPickRayOrig, vPickRayDir, s))
        {
            selected_joint_indices.append(i);
        }
    }

    qDebug() << "----------------";
    qDebug() << selected_joint_indices.size() << " selected";
    for (int i = 0; i < selected_joint_indices.size(); ++i)
    {
        qDebug() << avatar_->joints_[i]->name;
    }

    return true;
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
		length = (synthetic_animation_->duration / synthetic_animation_->ticks_per_second) * 1000; // �����������ʱ��
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

void Scene::pickCloth(BYTE red, bool hover)
{
	int index = red / 10;
	if(index >= 0 && index < clothes_.size())
	{
		if(hover)
			hover_cloth_index_ = index;
		else
			cur_cloth_index_ = index;
	}
	else
		hover_cloth_index_ = -1;
}

// UI �������νṹ
SceneGraphTreeWidget::SceneGraphTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setColumnCount(1);
    setHeaderHidden(true);

    QTreeWidgetItem* root = invisibleRootItem();
    QTreeWidgetItem* avatar_group = new QTreeWidgetItem(root);
    avatar_group->setText(0, "Avatar");
    QTreeWidgetItem* avatar_0_item = new QTreeWidgetItem(avatar_group);
    avatar_0_item->setText(0, "man");

    QTreeWidgetItem* cloth_item = new QTreeWidgetItem(root);
    cloth_item->setText(0, "Cloth");
    QTreeWidgetItem* cloth_panel_0 = new QTreeWidgetItem(cloth_item);
    cloth_panel_0->setText(0, "cloth_panel_0");
    QTreeWidgetItem* cloth_panel_1 = new QTreeWidgetItem(cloth_item);
    cloth_panel_1->setText(0, "cloth_panel_1");

    QTreeWidgetItem* light_group = new QTreeWidgetItem(root);
    light_group->setText(0, "Light");
    QTreeWidgetItem* light_0_item = new QTreeWidgetItem(light_group);
    light_0_item->setText(0, "light_0");

    QTreeWidgetItem* camera_group = new QTreeWidgetItem(root);
    camera_group->setText(0, "Camera");
    QTreeWidgetItem* camera_0 = new QTreeWidgetItem(camera_group);
    camera_0->setText(0, "camera_0");

    QTreeWidgetItem* env_group = new QTreeWidgetItem(root);
    env_group->setText(0, "Environment");
    QTreeWidgetItem* floor_item = new QTreeWidgetItem(env_group);
    floor_item->setText(0, "floor");
    QTreeWidgetItem* gravity_item = new QTreeWidgetItem(env_group);
    gravity_item->setText(0, "gravity");
}


// UI��
/************************************************************************/
/* ���泡�������б�                                                      */
/************************************************************************/

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
