#include "cloth.h"


zfCloth::zfCloth(void)
{
}


zfCloth::~zfCloth(void)
{
}

void zfCloth::init_buffer()
{
	if(!cloth_handler.cloth_num())
		return;
	cloth_handler.update_buffer();
	std::vector<float> positions = cloth_handler.get_position();
	std::vector<float> normals = cloth_handler.get_normal();
	std::vector<float> texcoords = cloth_handler.get_texcoord();
	
	position_buffer_ = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	position_buffer_->create();
	position_buffer_->setUsagePattern(QOpenGLBuffer::StaticDraw);
	position_buffer_->bind();
	position_buffer_->allocate(&positions[0], static_cast<int>(positions.size() * sizeof(float)));
	position_buffer_->release();

	normal_buffer_ = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	normal_buffer_->create();
	normal_buffer_->setUsagePattern(QOpenGLBuffer::StaticDraw);
	normal_buffer_->bind();
	normal_buffer_->allocate(&normals[0], static_cast<int>(normals.size() * sizeof(float)));
	normal_buffer_->release();

	texcoord_buffer_ = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	texcoord_buffer_->create();
	texcoord_buffer_->setUsagePattern(QOpenGLBuffer::StaticDraw);
	texcoord_buffer_->bind();
	texcoord_buffer_->allocate(&texcoords[0], static_cast<int>(texcoords.size() * sizeof(float)));
	texcoord_buffer_->release();
}

void zfCloth::update_buffer()
{
	if(!cloth_handler.cloth_num())
		return;
	cloth_handler.update_buffer();
	std::vector<float> positions = cloth_handler.get_position();
	std::vector<float> normals = cloth_handler.get_normal();
	std::vector<float> texcoords = cloth_handler.get_texcoord();
	
	position_buffer_->bind();
	position_buffer_->allocate(&positions[0], static_cast<int>(positions.size() * sizeof(float)));
	position_buffer_->release();

	normal_buffer_->bind();
	normal_buffer_->allocate(&normals[0], static_cast<int>(normals.size() * sizeof(float)));
	normal_buffer_->release();

	texcoord_buffer_->bind();
	texcoord_buffer_->allocate(&texcoords[0], static_cast<int>(texcoords.size() * sizeof(float)));
	texcoord_buffer_->release();
}

void zfCloth::initOBS(double * position, double * texcoords, int * indices, size_t faceNum) 
{
	cloth_handler.init_avatars_to_handler(position, texcoords, indices, faceNum);
}

void zfCloth::updateOBS(double * position)
{
	cloth_handler.update_avatars_to_handler(position);
}

void zfCloth::startSimulate()
{
	cloth_handler.begin_simulate();
}

void zfCloth::simulateStep()
{
	cloth_handler.sim_next_step();
}

void zfCloth::initCmFile(const char * filename, int totalFrame)
{
	cloth_handler.init_cmfile(filename, totalFrame);
}

void zfCloth::writeToCmFile(int frame)
{
	cloth_handler.write_frame_to_cmfile(frame);
}

void zfCloth::saveCmFile()
{
	cloth_handler.save_cmfile();
}

void zfCloth::loadCmFile(const char * filename)
{
	cloth_handler.load_cmfile_to_replay(filename);
	cloth_handler.load_frame(1);
	init_buffer();
}