#include <cmath>
#include <climits>
#include <QPointF>
#include <QLineF>

#include "camera.h"

#define PI 3.1415926
#define DEG_TO_RAD 0.01745329
#define RAD_TO_DEG 57.2957805

Camera::Camera( QObject* parent /*= 0 */ )
	: eye_(0.0, 0.0, 1.0f),
	center_(0.0, 0.0, 0.0),
	distance_exponent_(1200.0f),
	fovy_(45.0f),
	near_(0.1f),
	far_(1000.f)
{
	rotation_ = QQuaternion::fromAxisAndAngle(QVector3D(1.f, 0.f, 0.f), -90.f);
}

QVector3D Camera::screenToBall( const QPoint& pt )
{
	// map screen coordinates to [-1, 1]
	float x =   2.0 * pt.x() / viewport_width_ - 1.0;
	float y = -(2.0 * pt.y() / viewport_height_ - 1.0);
	float z = 0.0f;

	float mag = x * x + y * y;
	if (mag > 1.0f) {
		float scale = 1.0f / sqrtf(mag);
		x *= scale;
		y *= scale;
	}
	else {
		z = sqrtf(1.0f - mag);
	}

	return QVector3D(x, y, z);
}

//QVector3D Camera::screenToBall( const QPoint& pt )
//{
//	// map screen coordinates to [-1, 1]
//	float x =   2.0 * pt.x() / viewport_width_ - 1.0;
//	float z = -(2.0 * pt.y() / viewport_height_ - 1.0);
//	float y = 0.0f;
//
//	float mag = x * x + z * z;
//	if (mag > 1.0f) {
//		float scale = 1.0f / sqrtf(mag);
//		x *= scale;
//		z *= scale;
//	}
//	else {
//		y = sqrtf(1.0f - mag);
//	}
//
//	return QVector3D(x, y, z);
//}

void Camera::calcRotation( const QVector3D& vFrom, const QVector3D& vTo )
{
	QVector3D vPart;
	float fDot = QVector3D::dotProduct(vFrom, vTo);
	vPart = QVector3D::crossProduct(vFrom, vTo);

	QQuaternion newRot(fDot, vPart);
	rotation_ = newRot * rotation_;
	rotation_.normalize();
}

void Camera::rotate(const QPoint& prevPos, const QPoint& curPos)
{
	QVector3D from = screenToBall(prevPos);
	QVector3D to = screenToBall(curPos);
	calcRotation(from, to);
}

QQuaternion Camera::calcRotationForCloth( const QVector3D& vFrom, const QVector3D& vTo )
{
	QVector3D vPart;
	float fDot = QVector3D::dotProduct(vFrom, vTo);
	vPart = QVector3D::crossProduct(vFrom, vTo);

	QQuaternion newRot(fDot, vPart);
	newRot.normalize();

	return newRot;
}

QQuaternion Camera::rotateForCloth(const QPoint& prevPos, const QPoint& curPos)
{
	QVector3D from = screenToBall(prevPos);
	QVector3D to = screenToBall(curPos);
	return calcRotationForCloth(from, to);
}

//void Camera::pan( float dx, float dy )
//{
//	eye_.setX(eye_.x() - dx);
//	eye_.setY(eye_.y() - dy);
//
//	center_.setX(center_.x() - dx);
//	center_.setY(center_.y() - dy);
//}

void Camera::pan( float dx, float dy )
{
	eye_.setX(eye_.x() - dx);
	eye_.setZ(eye_.z() - dy);

	center_.setX(center_.x() - dx);
	center_.setZ(center_.z() - dy);
}

void Camera::zoom(float factor)
{
	distance_exponent_ += factor;
	if (distance_exponent_ < -32 * 120)
		distance_exponent_ = -32 * 120;
	if (distance_exponent_ > 32 * 120)
		distance_exponent_ = 32 * 120;
}

// void ArcBall::fitBoudningSphere( const Sphere& sphere )
// {
// 	float camera_to_sphere = sphere.radius / std::tan(0.5f * fovy_ * DEG_TO_RAD);
// 	center_ = sphere.center;
// 	eye_ = center_;
// 	eye_.setZ(center_.z() + sphere.radius + camera_to_sphere);
// 
// 	rotation_ = QQuaternion(1.0, 0.0, 0.0, 0.0);
// 	distExp_ = 600.0f;
// }

QMatrix4x4 Camera::getViewMatrix()
{
	// View matrix
	QMatrix4x4 view;
	view.lookAt(eye_, center_, QVector3D(0.0, 0.0f, 1.0));
	view(2, 3) -= 2.0f * exp(distance_exponent_ / 1200.0f);

	// convert the quaternion into a rotation matrix
	float x2 = rotation_.x() * rotation_.x();
	float y2 = rotation_.y() * rotation_.y();
	float z2 = rotation_.z() * rotation_.z();
	float xy = rotation_.x() * rotation_.y();
	float xz = rotation_.x() * rotation_.z();
	float yz = rotation_.y() * rotation_.z();
	float wx = rotation_.scalar() * rotation_.x();
	float wy = rotation_.scalar() * rotation_.y();
	float wz = rotation_.scalar() * rotation_.z();

	view(0, 0) = 1 - 2*y2 - 2*z2;
	view(1, 0) = 2*xy + 2*wz;
	view(2, 0) = 2*xz - 2*wy;

	view(0, 1) = 2*xy - 2*wz;
	view(1, 1) = 1 - 2*x2 - 2*z2;
	view(2, 1) = 2*yz + 2*wx;

	view(0, 2) = 2*xz + 2*wy;
	view(1, 2) = 2*yz - 2*wx;
	view(2, 2) = 1 - 2*x2 - 2*y2;

	return view;
}

//QMatrix4x4 Camera::getViewMatrix()
//{
//	// View matrix
//	QMatrix4x4 view;
//	view.lookAt(eye_, center_, QVector3D(0.0, 0.0f, 1.0));
//	view(2, 3) -= 2.0f * exp(distance_exponent_ / 1200.0f);
//
//	// convert the quaternion into a rotation matrix
//	float x2 = rotation_.x() * rotation_.x();
//	float y2 = rotation_.y() * rotation_.y();
//	float z2 = rotation_.z() * rotation_.z();
//	float xz = rotation_.x() * rotation_.z();
//	float xy = rotation_.x() * rotation_.y();
//	float yz = rotation_.y() * rotation_.z();
//	float wx = rotation_.scalar() * rotation_.x();
//	float wy = rotation_.scalar() * rotation_.y();
//	float wz = rotation_.scalar() * rotation_.z();
//
//	view(0, 0) = 1 - 2*y2 - 2*z2;
//	view(1, 0) = 2*xz + 2*wy;
//	view(2, 0) = 2*xy - 2*wz;
//
//	view(0, 1) = 2*xz - 2*wy;
//	view(1, 1) = 1 - 2*x2 - 2*y2;
//	view(2, 1) = 2*yz + 2*wx;
//
//	view(0, 2) = 2*xy + 2*wz;
//	view(1, 2) = 2*yz - 2*wx;
//	view(2, 2) = 1 - 2*x2 - 2*z2;
//
//	return view;
//}

QMatrix4x4 Camera::getProjectionMatrix()
{
	QMatrix4x4 projection;
	float aspect_ratio = 1.0f * viewport_width_ / viewport_height_;
	projection.perspective(fovy_, aspect_ratio, near_, far_);

	return projection;
}


QVector3D Camera::getViewDirection()
{
	QVector3D v3 = center_ - eye_;
	v3.normalize();
	QVector4D v4(v3, 1);
	v4 = v4 * getViewMatrix();
	v3 = QVector3D(v4);
	v3.normalize();
	return v3;
}