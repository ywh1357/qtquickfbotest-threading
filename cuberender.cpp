#include "cuberender.h"
#include <QMutexLocker>
#include <QGuiApplication>

QEvent::Type RenderEvent::m_type = QEvent::None;

RenderEvent::RenderEvent(RenderEvent::Method method):QEvent(registeredType()),m_method(method)
{
    qDebug() << "RenderEvent created";
}

RenderEvent::~RenderEvent()
{
    qDebug() << "RenderEvent destroyed";
}

RenderEvent::Method RenderEvent::method()
{
    return m_method;
}

QEvent::Type RenderEvent::registeredType()
{
    if(m_type == QEvent::None){
        int type = QEvent::registerEventType();
        m_type = static_cast<QEvent::Type>(type);
    }
    return m_type;
}

CubeRender::CubeRender(QThread *renderThread):QObject(nullptr),m_renderThread(renderThread),
    m_mutex()
{
    //connect(this,&CubeRender::viewPortSizeChanged,this,&CubeRender::handleViewPortSizeChanged);
    if(m_renderThread)
        this->moveToThread(m_renderThread);
    connect(this,&CubeRender::textureChanged,this,&CubeRender::handleTextureChanged);
    m_renderThread->setObjectName("RenderThread");
}

void CubeRender::createSurface(const QSurfaceFormat format)
{
    qDebug() << "createSurface";
    m_surface = new QOffscreenSurface();
    m_surface->setFormat(format);
    m_surface->create();
//    if(m_renderThread)
//        m_surface->moveToThread(m_renderThread);
}

void CubeRender::createContext(QOpenGLContext *sharedContext)
{
    qDebug() << "createContext";
    m_context = new QOpenGLContext();
    m_context->setFormat(sharedContext->format());
    m_context->setShareContext(sharedContext);
    m_context->create();
    if(m_renderThread)
        m_context->moveToThread(m_renderThread);
}

bool CubeRender::initialized()
{
    return m_initialized;
}

void CubeRender::initialization()
{
    qDebug() << "initialization";
    initializeOpenGLFunctions();

    m_program  = new QOpenGLShaderProgram();
    if(m_renderThread)
        m_program->moveToThread(m_renderThread);
    m_program->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex,":/cubevs.vsh");
    m_program->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment,":/cubefs.fsh");
    m_program->link();

    location_vertex = m_program->attributeLocation("Vertex");
    location_texCoord = m_program->attributeLocation("aTexCoord");
    location_matrix = m_program->uniformLocation("mvp");
    location_texture0 = m_program->uniformLocation("Texture0");

//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    float vertices[] = {
        // positions
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
    };
    float texCoords[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,

        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f
    };

    auto vertexLen = sizeof(vertices)/sizeof(float);

    for(int i = 0; i < vertexLen; i += 3){
        m_vertexs.push_back(QVector3D(vertices[i],vertices[i+1],vertices[i+2]));
    }

    auto texCoordsLen = sizeof(texCoords)/sizeof(float);

    for(int i = 0; i < texCoordsLen; i += 2){
        m_texCoords.push_back(QVector2D(texCoords[i],texCoords[i+1]));
    }

//    qDebug() << m_vertexs.size() << m_texCoords.size();

    m_program->enableAttributeArray(location_vertex);
    m_program->enableAttributeArray(location_texCoord);
    m_program->setAttributeArray(location_vertex,m_vertexs.constData());
    m_program->setAttributeArray(location_texCoord,m_texCoords.constData());
//    m_program->disableAttributeArray(location_vertex);
//    m_program->disableAttributeArray(location_vertex);

    makeMVP();

    m_initialized = true;
}

void CubeRender::setSize(QSize newSize)
{
    QMutexLocker locker(&m_mutex);
    if(newSize == m_viewPortSize)
        return;
    m_viewPortSize = newSize;
    locker.unlock();
	makeMVP();
}

void CubeRender::setTexture(const QString path)
{
    emit textureChanged(path);
}

void CubeRender::start()
{
    if(m_renderThread)
        m_renderThread->start();
}

void CubeRender::stop()
{
    auto event = new RenderEvent(RenderEvent::shutdown);
    QCoreApplication::postEvent(this,event);
}

bool CubeRender::event(QEvent *event)
{
    if(event->type() == RenderEvent::registeredType()){
        processRenderEvent(event);
        return true;
    }else{
        return QObject::event(event);
    }
}

QOpenGLFramebufferObject * CubeRender::makeFBO(QSize size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setInternalTextureFormat(GL_RGBA8);
    return new QOpenGLFramebufferObject(size,format);
}

void CubeRender::makeMVP()
{
    qDebug() << "makeMVP with " << m_viewPortSize;
    QMutexLocker locker(&m_mutex);
    QSize size = m_viewPortSize;
    QMatrix4x4 model;
    model.rotate(50,0.5,1.0,0);
    QMatrix4x4 view;
    view.translate(0.0f, 0.0f, -3.0f);
    QMatrix4x4 projection;
    projection.perspective(45, size.width() / size.height(), 0.1f, 100.0f);
    m_mvp = projection * view * model;
}

void CubeRender::processRenderEvent(QEvent *event)
{
    auto renderEvent = static_cast<RenderEvent*>(event);
    if(renderEvent->method() == RenderEvent::shutdown){
        qDebug() << "process shutdown event";
        shutdown();
    }
}

void CubeRender::render()
{
    QMutexLocker locker(&m_mutex);
    QSize size = m_viewPortSize;
	auto mvp = m_mvp;
    locker.unlock();

    QMatrix4x4 rotation;
    rotation.rotate(m_radius,0,1,0);
    mvp = mvp * rotation;
    m_radius += 0.1f;

    m_context->makeCurrent(m_surface);
    if(!m_initialized)
        initialization();

    if(!m_displayFBO){
        m_displayFBO = makeFBO(size);
    }
    if(!m_renderFBO){
        m_renderFBO = makeFBO(size);
    }else if(m_renderFBO->size() != size){
        delete m_renderFBO;
        m_renderFBO = nullptr;
        m_renderFBO = makeFBO(size);
        qDebug() << "renderFBO resize to " << size;
    }

	if (m_textureDirty) {
		if (m_texture)
			delete m_texture;
		m_texture = new QOpenGLTexture(QImage(m_texturePath).mirrored());
		m_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
		m_textureDirty = false;
	}

    m_renderFBO->bind();
    glViewport(0,0,size.width(),size.height());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

//    glEnable(GL_CULL_FACE);
//    glFrontFace(GL_CW);
//    glCullFace(GL_BACK);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    m_program->bind();
    m_program->setUniformValue(location_matrix, mvp);
    m_program->setUniformValue(location_texture0,1);
    m_texture->bind(1);
//    m_program->enableAttributeArray(location_vertex);
//    m_program->enableAttributeArray(location_texCoord);
//    m_program->setAttributeArray(location_vertex,m_vertexs.constData());
//    m_program->setAttributeArray(location_texCoord,m_texCoords.constData());
    glDrawArrays(GL_TRIANGLES,0,m_vertexs.size());
//    m_program->disableAttributeArray(location_vertex);
//    m_program->disableAttributeArray(location_vertex);
    m_program->release();

    glFlush();
    m_renderFBO->bindDefault();
    std::swap(m_renderFBO,m_displayFBO);

    emit textureReady(m_displayFBO->texture(),m_displayFBO->size());
}

void CubeRender::handleTextureChanged(QString path)
{
	m_textureDirty = true;
	m_texturePath = path;
}

void CubeRender::shutdown()
{
    qDebug() << "shutdown";
    m_context->makeCurrent(m_surface);
    delete m_renderFBO;
    delete m_displayFBO;
    m_context->doneCurrent();
    delete m_context;

    // schedule this to be deleted only after we're done cleaning up
    m_surface->deleteLater();

    // Stop event processing, move the thread to GUI and make sure it is deleted.
    if(m_renderThread)
        m_renderThread->exit();
    this->moveToThread(QGuiApplication::instance()->thread());
}

