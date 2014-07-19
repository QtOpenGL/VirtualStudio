#ifndef CLOTH_H
#define CLOTH_H

#include <QVector3D>
#include <QOpenGLBuffer>
#include "ClothMotion\cloth_motion.h"

// ������
const double DAMPING = 0.01;							// ����
const double TIME_STEP = 0.5;							// ʱ�䲽��
const double TIME_STEP_SQUARE = TIME_STEP * TIME_STEP;	// ʱ�䲽��ƽ��
const int	CONSTRAINT_ITERATIONS = 15;					// ÿ֡��Լ��������constraint satisfaction����������

// Pattern
// 2D Design

// ���ӣ��ʵ㣩
//class Particle
//{
//public:
//	Particle(const QVector3D& pos)
//		: position_(pos), old_position_(pos), acceleration_(0,0,0), mass_(1), movable_(true), accumulated_normal_(0,0,0)
//	{}
//
//	// ʩ��������
//	void addForce(const QVector3D& f)
//	{
//		acceleration_ += f/mass_;
//	}
//
//	// ʱ��Ƭ����
//	void timeStep()
//	{
//		if (movable_) {
//			QVector3D temp = position_;
//			position_ = position_ + (position_ - old_position_) * (1.0 - DAMPING) + acceleration_ * TIME_STEP_SQUARE; // verlet����
//			old_position_ = temp;
//			acceleration_ = QVector3D(0, 0, 0);	// ������Ϻ� ���ٶȹ���
//		}
//	}
//
//	// ���ٶȹ���
//	void resetAcceleration()
//	{
//		acceleration_ = QVector3D(0, 0, 0);
//	}
//
//	// �ƶ�����
//	void offsetPostion(const QVector3D& v)
//	{
//		if (movable_)
//			position_ += v;
//	}
//
//	// �ۼӷ���
//	void addToNormal(const QVector3D& normal)
//	{
//		accumulated_normal_ += normal.normalized();
//	}
//
//	QVector3D position() const { return position_; }
//	void setPosition(const QVector3D& pos) { position_ = pos; }
//	bool movable() const { return movable_; }
//	void setMovable(bool val) { movable_ = val; }
//	QVector3D accumulaterNormal() const { return accumulated_normal_; }
//	void resetAccumulatedNormal() { accumulated_normal_ = QVector3D(0, 0, 0); }
//
//private:
//	bool		movable_;				// �Ƿ���ƶ� ����ê������
//	float		mass_;					// ����
//	QVector3D	position_;				// ��ǰλ��
//	QVector3D	old_position_;			// ��һ��ʱ�䲽����λ�� ����Verlet��ֵ����
//	QVector3D	acceleration_;			// ���ٶ�
//	QVector3D	accumulated_normal_;	// �ۻ�����
//};

// Լ�������ɣ�
//class Constraint
//{
//public:
//	Constraint(Particle *pt1, Particle* pt2)
//		: p1(pt1), p2(pt2), rest_length_((pt1->position() - pt2->position()).length())
//	{
//	}
//
//	// Լ������
//	void satisfyConstraint()
//	{
//		QVector3D p1_to_p2 = p2->position() - p1->position();
//		float current_distance = p1_to_p2.length();
//		QVector3D correctionVector = p1_to_p2 * (1 - rest_length_/current_distance);	// �����ʵ�������Ȼ����֮����
//		p1->offsetPostion( 0.5 * correctionVector);
//		p2->offsetPostion(-0.5 * correctionVector);
//	}
//
//	// �������ӵ������ʵ�
//	Particle*	p1;
//	Particle*	p2;
//
//private:
//	float rest_length_;	// ������Ȼ���ȣ������� ��ѹ�� ����ƽ�⣩ ֹ������
//};

// ����
// 3D Simulation

class QOpenGLVertexArrayObject;

class zfCloth
{
public:
	zfCloth(void);
	~zfCloth(void);

	void updateAnimation();
	void load(const char * filename) {cloth_handler.add_clothes_to_handler(filename);}
	QOpenGLBuffer* position_buffer() { return position_buffer_; }
	QOpenGLBuffer* normal_buffer() { return normal_buffer_; }
	QOpenGLBuffer* texcoord_buffer() { return texcoord_buffer_; }
	QOpenGLVertexArrayObject* vao() { return vao_; }
	void setVAO(QOpenGLVertexArrayObject* pvao) { vao_ = pvao; init_buffer(); }
	size_t face_count() { return cloth_handler.face_count(); }
	void update(const float * trans) { cloth_handler.transform_cloth(trans); update_buffer();}
	void initOBS(double * position, double * texcoords, int * indices, size_t faceNum);
	void updateOBS(double * position);
	void startSimulate();
	void simulateStep();
	void initCmFile(const char * filename, int totalFrame);
	void writeToCmFile(int frame);
	void saveCmFile();
	void loadFrame(int frame) {cloth_handler.load_frame(frame);update_buffer();}
	void loadCmFile(const char * filename);
private:
	void init_buffer();
	void update_buffer();

	// ����

	// wunf�ķ�װ�����������
	ClothHandler cloth_handler;
	// �������cloth handler�����ֱ�Ӵ���VBO��wunf
	QOpenGLBuffer*	position_buffer_;
	QOpenGLBuffer*	normal_buffer_;
	QOpenGLBuffer*	texcoord_buffer_;
	QOpenGLVertexArrayObject* vao_;

	// �������
};

#endif // CLOTH_H
