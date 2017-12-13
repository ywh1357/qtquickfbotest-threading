#ifndef DISPLAYPANE_H
#define DISPLAYPANE_H

#include <QObject>
#include <QQuickItem>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGTextureMaterial>

#include "cuberender.h"

class TextureNode: public QObject ,public QSGGeometryNode
{
    Q_OBJECT

public:
    TextureNode(QQuickWindow *win);
    ~TextureNode();

signals:
    void requestNewTexture();
    void newTextureReady();

public slots:
    // This function gets called on the FBO rendering thread and will store the
    // texture id and size and schedule an update on the window.
    void newTexture(int id, const QSize size);

    // Before the scene graph starts to render, we update to the pending texture
    void prepareNode();

private:
    int m_id;
    QSize m_size;
    QSGTexture *m_texture;
    QQuickWindow *m_window;
    QSGTextureMaterial *m_material;
    QSGGeometry *m_geometry;
    QMutex m_mutex;
};

class DisplayPane : public QQuickItem
{
    Q_OBJECT
public:
    DisplayPane();
    ~DisplayPane();
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);
private:

signals:
    void renderReady();

public slots:
    void handleWindowChanged(QQuickWindow *win);
    void handleRenderReady();
    void sync();
//    void cleanup();
private:
    CubeRender *m_render;
    QThread m_renderThread;
};

#endif // DISPLAYPANE_H
