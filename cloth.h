#ifndef CLOTH_H
#define CLOTH_H

#include "scene_node.h"


class QOpenGLVertexArrayObject;
struct Cloth;

typedef std::tr1::shared_ptr<Cloth> SmtClothPtr;

class zfCloth
{
public:
	zfCloth(void);
	zfCloth(SmtClothPtr cloth);
	~zfCloth(void);

	void updateAnimation();
	//void load_zfcloth(const char * filename) {cloth_handler.add_clothes_to_handler(filename);}
	QOpenGLBuffer* position_buffer() { return position_buffer_; }
	QOpenGLBuffer* normal_buffer() { return normal_buffer_; }
	QOpenGLBuffer* texcoord_buffer() { return texcoord_buffer_; }
	QOpenGLVertexArrayObject* vao() { return vao_; }
	void setVAO(QOpenGLVertexArrayObject* pvao) { vao_ = pvao; cloth_init_buffer(); }
	size_t face_count();
	void update(const float * trans) { /*cloth_handler.transform_cloth(trans);*/ cloth_update_buffer();}
	void initOBS(double * position, double * texcoords, int * indices, size_t faceNum);
	void updateOBS(double * position);
	void startSimulate();
	void simulateStep();
	void initCmFile(const char * filename, int totalFrame);
	void writeToCmFile(int frame);
	void saveCmFile();
	void loadFrame(int frame) {/*cloth_handler.load_frame(frame);*/cloth_update_buffer();}
	void loadCmFile(const char * filename);
	void cloth_update_buffer();

private:
	void cloth_init_buffer();

	// 网格
	const SmtClothPtr cloth_;

	// wunf的服装动画处理对象
	//ClothHandler cloth_handler;
	// 网格存在cloth handler里，这里直接创建VBO，wunf
	QOpenGLBuffer*	position_buffer_;
	QOpenGLBuffer*	normal_buffer_;
	QOpenGLBuffer*	texcoord_buffer_;
	QOpenGLVertexArrayObject* vao_;

	std::vector<float> cloth_position_buffer_;
	std::vector<float> cloth_normal_buffer_;
	std::vector<float> cloth_texcoord_buffer_;
};

#endif // CLOTH_H
