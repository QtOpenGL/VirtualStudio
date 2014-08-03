#ifndef SIMULATION_WINDOW_H
#define SIMULATION_WINDOW_H

//#include "opengl_window.h"
#include <QWindow>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

class QOpenGLShaderProgram;
class Scene;
class AVIGenerator;
class SimulationWindow : public QWindow
{
	Q_OBJECT

public:
	SimulationWindow( Scene* scene, QWindow* parent = 0 );

	void initializeGL();
	void paintGL();
	void paintForPick();

	void startSimulate();
	void record();

	QString getClothMotionFile() { return cloth_motion_file_; }

public slots:
	void updateAnimation(int);	// int参数是当前帧数
	void restoreToBindpose();

	void resizeGL();   

protected:
	void mousePressEvent( QMouseEvent *event );
	void mouseMoveEvent( QMouseEvent *event );
	void wheelEvent( QWheelEvent *event );

private:
	QOpenGLContext* context_;
	Scene*	scene_;

	QPoint cur_pos_;
	QPoint prev_pos_;

	QString cloth_motion_file_;

	AVIGenerator * AviGen;
	LPBITMAPINFOHEADER lpbih;
	BYTE * bmBits;

};

#endif // SIMULATION_WINDOW_H
