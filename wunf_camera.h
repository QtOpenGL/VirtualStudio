#ifndef WUNF_CAMERA_H
#define WUNF_CAMERA_H

#include <QObject>
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>

class wunf_camera : public QObject
{
public:
	explicit wunf_camera( QObject* parent = 0 );

	QVector3D eye() const { return eye_; }
	void setEye(const QVector3D& eye) { eye_= eye; }
	QVector3D center() const { return center_; }
	void setCenter(const QVector3D& c) { center_ = c; }

	void setViewportWidth(int width) { viewport_width_ = width; }
	void setViewportHeight(int height) { viewport_height_ = height; }
	void setNearPlane(float n) { near_ = n; }
	void setFarPlane(float f) { far_ = f; }
	//void fitBoudningSphere(const Sphere& sphere);

	QMatrix4x4 getViewMatrix();
	QMatrix4x4 getProjectionMatrix();
	QVector3D getViewDirection();

	void rotate(const QPoint& prevPos, const QPoint& curPos);
	QQuaternion rotateForCloth(const QPoint& prevPos, const QPoint& curPos);
	void pan(float dx, float dy);
	void zoom(float factor);  

private:
	int viewport_width_;
	int viewport_height_;

	QQuaternion rotation_;

	QVector3D eye_;
	QVector3D center_;
	float distance_exponent_;
	float fovy_;
	float near_;
	float far_;
};

#endif