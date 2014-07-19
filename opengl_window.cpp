#include "opengl_window.h"

#include <QCoreApplication>

#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QPainter>


OpenGLWindow::OpenGLWindow( QWindow *parent /*= 0*/ )
	: QWindow(parent),
	m_update_pending(false),
	m_animating(false),
	m_context(nullptr),
	m_device(0)
{
	setSurfaceType(QWindow::OpenGLSurface);
}

OpenGLWindow::~OpenGLWindow()
{
	delete m_device;
}

void OpenGLWindow::paintGL( QPainter *painter )
{
	Q_UNUSED(painter);
}

void OpenGLWindow::paintGL()
{
	if (!m_device)
		m_device = new QOpenGLPaintDevice;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_device->setSize(size());

	QPainter painter(m_device);
	paintGL(&painter);
}

void OpenGLWindow::initializeGL()
{
}

void OpenGLWindow::setAnimating( bool animating )
{
	m_animating = animating;

	if (animating)
		renderLater();
}

void OpenGLWindow::renderLater()
{
	if (!m_update_pending) {
		m_update_pending = true;
		QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
	}
}

void OpenGLWindow::renderNow()
{
	if (!isExposed())
		return;

	bool needsInitialize = false;

	if (!m_context) {
		m_context = new QOpenGLContext(this);
		m_context->setFormat(requestedFormat());
		m_context->create();

		needsInitialize = true;
	}

	m_context->makeCurrent(this);

	if (needsInitialize) {
		initializeOpenGLFunctions();
		initializeGL();
	}

	paintGL();

	m_context->swapBuffers(this);

	if (m_animating)
		renderLater();
}

bool OpenGLWindow::event(QEvent *event)
{
	switch(event->type()) {
	case QEvent::UpdateRequest:
		m_update_pending = false;
		renderNow();
		return true;
	default:
		return QWindow::event(event);
	}
}

void OpenGLWindow::exposeEvent( QExposeEvent *event )
{
	Q_UNUSED(event);

	if (isExposed())
		renderNow();
}
