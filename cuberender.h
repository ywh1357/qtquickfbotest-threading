#ifndef CUBERENDER_H
#define CUBERENDER_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QOpenGLTexture>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QThread>
#include <QMutex>
#include <QEvent>
//#include <memory>

class RenderEvent: public QEvent{
public:
    enum Method{shutdown};
    RenderEvent(Method method);
    virtual ~RenderEvent();
    Method method();
    static QEvent::Type registeredType();
private:
    static QEvent::Type m_type;
    Method m_method;
};

class CubeRender : public QObject, public QOpenGLFunctions
{
    Q_OBJECT
public:
    CubeRender(QThread *renderThread = nullptr);
    void createSurface(const QSurfaceFormat format);
    void createContext(QOpenGLContext* sharedContext);
    bool initialized();
    void initialization();
    void setSize(QSize size);
    void setTexture(const QString path);
    void start();
    void stop();
    virtual bool event(QEvent *event) override;
private:
    QOpenGLFramebufferObject * makeFBO(QSize size);
    void makeMVP();
    void processRenderEvent(QEvent *event);
signals:
    void textureReady(int id, const QSize size);
//    void viewPortSizeChanged(QSize size);
    void textureChanged(const QString path);
public slots:
    void render();
    //void handleViewPortSizeChanged(QSize newSize);
    void handleTextureChanged(const QString path);
    void shutdown();
public:
	QOpenGLContext *m_context = nullptr;
	QOffscreenSurface *m_surface = nullptr;
private:
    QOpenGLShaderProgram *m_program;
    QThread *m_renderThread;
    QSize m_viewPortSize;
    QOpenGLFramebufferObject *m_renderFBO = nullptr;
    QOpenGLFramebufferObject *m_displayFBO = nullptr;
    QMutex m_mutex;
    QVector<QVector3D> m_vertexs;
    QVector<QVector2D> m_texCoords;
    int location_vertex;
    int location_texCoord;
    int location_matrix;
    int location_texture0;
    QMatrix4x4 m_mvp;
    bool m_initialized = false;
    QOpenGLTexture *m_texture = nullptr;
	QString m_texturePath;
	bool m_textureDirty = true;
    float m_radius = 0;
};

#endif // CUBERENDER_H
