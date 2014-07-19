#include "wunf_camera.h"

wunf_camera::wunf_camera( QObject* parent /*= 0 */ ) : 
	eye_(0.0, -1.0, 0.0f),
	center_(0.0, 0.0, 0.0),
	distance_exponent_(2400.0f),
	fovy_(45.0f),
	near_(0.1f),
	far_(1000.f)
{
}

QMatrix4x4 wunf_camera::getViewMatrix()
{
	QMatrix4x4 view;
	view.lookAt(eye_, center_, QVector3D(0.0, 0.0f, 1.0));
	return view;
}

QMatrix4x4 wunf_camera::getProjectionMatrix()
{
	QMatrix4x4 projection;
	float aspect_ratio = 1.0f * viewport_width_ / viewport_height_;
	projection.perspective(fovy_, aspect_ratio, near_, far_);

	return projection;
}

QVector3D wunf_camera::getViewDirection()
{
	QVector3D v3 = center_ - eye_;
	v3.normalize();
	return v3;
}

void wunf_camera::rotate(const QPoint& prevPos, const QPoint& curPos)
{
	float dx = curPos.x() - prevPos.x();
	float dy = curPos.y() - prevPos.y();

}

QQuaternion wunf_camera::rotateForCloth(const QPoint& prevPos, const QPoint& curPos)
{
	return QQuaternion();
}

void wunf_camera::pan(float dx, float dy)
{
	
}

void wunf_camera::zoom(float factor)
{
	
}