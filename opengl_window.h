#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H
 
class QPainter;
class QOpenGLContext;
class QOpenGLPaintDevice;
 
class OpenGLWindow : public QWindow, protected QOpenGLFunctions
{
 	Q_OBJECT
public:
 	explicit OpenGLWindow(QWindow *parent = 0);
 	~OpenGLWindow();
 
 	virtual void paintGL(QPainter *painter);
 	virtual void paintGL();
 
 	virtual void initializeGL();
 
 	void setAnimating(bool animating);
 
public slots:
 	void renderLater();
 	void renderNow();
 
protected:
 	bool event(QEvent *event);
 
 	void exposeEvent(QExposeEvent *event);
 
private:
 	bool m_update_pending;
 	bool m_animating;
 
 	QOpenGLContext *m_context;
 	QOpenGLPaintDevice *m_device;
};
 
#endif // OPENGLWINDOW_H
