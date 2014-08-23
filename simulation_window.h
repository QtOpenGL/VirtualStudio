#ifndef SIMULATION_WINDOW_H
#define SIMULATION_WINDOW_H

^^^^^^^ HEAD
#include <QWindow>
#include <QTime>

class Scene;
class Animation;
=======
//#include "opengl_window.h"
#include <QWindow>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

class QOpenGLShaderProgram;
class Scene;
class AVIGenerator;
>>>>>>> dev
class SimulationWindow : public QWindow
{
	Q_OBJECT

public:
^^^^^^^ HEAD
    SimulationWindow( Scene* scene, QWindow* parent = 0 );

	void initializeGL();
	void paintGL();

public slots:
	void updateAnimation(const Animation* anim, int frame);
	void restoreToBindpose();

    void resizeGL();

protected:
    void mousePressEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent* e );
    void mouseMoveEvent( QMouseEvent *event );
    void wheelEvent( QWheelEvent *event );

//     void keyPressEvent( QKeyEvent* e );
//     void keyReleaseEvent( QKeyEvent* e );
=======
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
>>>>>>> dev

private:
	QOpenGLContext* context_;
	Scene*	scene_;

^^^^^^^ HEAD
    bool m_leftButtonPressed;
	QPoint cur_pos_;
	QPoint prev_pos_;
=======
	QPoint cur_pos_;
	QPoint prev_pos_;

	QString cloth_motion_file_;

	AVIGenerator * AviGen;
	LPBITMAPINFOHEADER lpbih;
	BYTE * bmBits;

>>>>>>> dev
};

#endif // SIMULATION_WINDOW_H
