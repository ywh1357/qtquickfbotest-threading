#include "displaypane.h"
#include <QQuickWindow>

TextureNode::TextureNode(QQuickWindow *win):m_id(0),m_size(0,0),m_texture(0),m_window(win)
{
    m_texture = m_window->createTextureFromId(0, QSize(1, 1));
    m_material = new QSGTextureMaterial();
    m_material->setTexture(m_texture);
    m_material->setFlag(QSGMaterial::Blending);
    m_material->setFiltering(QSGTexture::Linear);
    setMaterial(m_material);
//        setFlag(OwnsMaterial);
    m_geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(),4);
    m_geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
    setGeometry(m_geometry);
    markDirty(DirtyGeometry | DirtyMaterial);
}

TextureNode::~TextureNode()
{
    if(m_texture)
        delete m_texture;
}

void TextureNode::newTexture(int id, const QSize size)
{
    m_mutex.lock();
    m_id = id;
    m_size = size;
    m_mutex.unlock();
    // We cannot call QQuickWindow::update directly here, as this is only allowed
    // from the rendering thread or GUI thread.
    emit newTextureReady();
}

void TextureNode::prepareNode()
{
    m_mutex.lock();
    int newId = m_id;
    QSize size = m_size;
    m_id = 0;
    m_mutex.unlock();
    if (newId) {
        delete m_texture;
        // note: include QQuickWindow::TextureHasAlphaChannel if the rendered content
        // has alpha.
        m_texture = m_window->createTextureFromId(newId, size,QQuickWindow::TextureHasAlphaChannel);
        m_material->setTexture(m_texture);
        m_material->setFlag(QSGMaterial::Blending);
//        qDebug() << m_texture->hasAlphaChannel();
//        m_material->setHorizontalWrapMode(QSGTexture::Repeat);
//        m_material->setVerticalWrapMode(QSGTexture::Repeat);

        auto *vertexs = static_cast<QSGGeometry::TexturedPoint2D*>(m_geometry->vertexDataAsTexturedPoint2D());
        vertexs[0].set(0,0,0,0);
        vertexs[1].set(m_size.width(),0,1,0);
        vertexs[2].set(0,m_size.height(),0,1);
        vertexs[3].set(m_size.width(),m_size.height(),1,1);

        markDirty(DirtyMaterial | DirtyGeometry);

        // This will notify the rendering thread that the texture is now being rendered
        // and it can start rendering to the other one.
        emit requestNewTexture();
    }
}

DisplayPane::DisplayPane():m_render(nullptr)
{
//    auto sizeChangedSender = [this]{ emit sizeChanged(size().toSize()); };
//    connect(this,&QQuickItem::heightChanged,sizeChangedSender);
//    connect(this,&QQuickItem::widthChanged,sizeChangedSender);
    connect(this,&QQuickItem::windowChanged,this,&DisplayPane::handleWindowChanged);
	connect(this, &DisplayPane::renderReady, this, &DisplayPane::handleRenderReady);
    setFlag(this->ItemHasContents);
    setFocus(true);

    m_render = new CubeRender(&m_renderThread);
}

DisplayPane::~DisplayPane()
{
    qDebug() << "~DisplayPane";
    m_render->stop();
    m_render->deleteLater();
    m_renderThread.wait();
}

QSGNode *DisplayPane::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
	if (!m_render->m_context) {
		QOpenGLContext *current = window()->openglContext();
		current->doneCurrent();
		m_render->createContext(current);
		current->makeCurrent(window());
		emit renderReady();
	}
	m_render->setSize(size().toSize());

    TextureNode *node;
    if(!oldNode){
        node = new TextureNode(window());
        connect(node,&TextureNode::requestNewTexture,m_render,&CubeRender::render,Qt::QueuedConnection);
        connect(node,&TextureNode::newTextureReady,window(),&QQuickWindow::update,Qt::QueuedConnection);
        connect(m_render,&CubeRender::textureReady,node,&TextureNode::newTexture,Qt::DirectConnection);
        connect(window(),&QQuickWindow::beforeRendering,node,&TextureNode::prepareNode,Qt::DirectConnection);
    }else{
        node = static_cast<TextureNode*>(oldNode);
    }
    return node;
}

void DisplayPane::handleWindowChanged(QQuickWindow *win) {
	if (win) {
		connect(win, &QQuickWindow::beforeSynchronizing, this, &DisplayPane::sync,Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, m_render, &CubeRender::stop, Qt::QueuedConnection);
	}
}

void DisplayPane::handleRenderReady()
{
	m_render->createSurface(m_render->m_context->format());
	m_render->setTexture(":/texture0.jpg");
	m_render->start();
    QMetaObject::invokeMethod(m_render, "render", Qt::QueuedConnection);
    update();
}

void DisplayPane::sync()
{
//    qDebug() << "sync";
	m_render->setSize(size().toSize());
}
