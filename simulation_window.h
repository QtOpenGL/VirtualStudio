#ifndef SIMULATION_WINDOW_H
#define SIMULATION_WINDOW_H

#include <QWindow>
#include <QTime>

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

class Animation;
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

public slots:
    void updateAnimation(const Animation* anim, int frame);
    void restoreToBindpose();

    void resizeGL();

protected:
    void mousePressEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent* e );
    void mouseMoveEvent( QMouseEvent *event );
    void wheelEvent( QWheelEvent *event );

private:
    QOpenGLContext* context_;
    Scene*	scene_;

    bool m_leftButtonPressed;
    QPoint cur_pos_;
    QPoint prev_pos_;

    QString cloth_motion_file_;

    AVIGenerator * AviGen;
    LPBITMAPINFOHEADER lpbih;
    BYTE * bmBits;

};

#endif // SIMULATION_WINDOW_H
