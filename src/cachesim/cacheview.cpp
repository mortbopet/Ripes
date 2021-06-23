#include "cacheview.h"

#include <qmath.h>
#include <QGraphicsSimpleTextItem>
#include <QWheelEvent>

namespace Ripes {

CacheView::CacheView(QWidget* parent) : QGraphicsView(parent) {
    m_zoom = 250;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    setBackgroundBrush(Qt::white);
    setInteractive(true);
    setupMatrix();
}

void CacheView::mousePressEvent(QMouseEvent* event) {
    // If we press on a cache data block, get the address stored for that block and emit a signal indicating that the
    // address was selected through the cache
    const auto viewItems = items(event->pos());
    for (const auto& item : qAsConst(viewItems)) {
        if (auto* textItem = dynamic_cast<QGraphicsSimpleTextItem*>(item)) {
            const QVariant userData = textItem->data(Qt::UserRole);
            if (userData.isValid()) {
                emit cacheAddressSelected(userData.toULongLong());
                break;
            }
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void CacheView::fitScene() {
    scene()->setSceneRect(scene()->itemsBoundingRect());
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}

void CacheView::wheelEvent(QWheelEvent* e) {
    if (e->modifiers() & Qt::ControlModifier) {
        if (e->angleDelta().y() > 0)
            zoomIn(6);
        else
            zoomOut(6);
        e->accept();
    } else {
        QGraphicsView::wheelEvent(e);
    }
}

void CacheView::zoomIn(int level) {
    m_zoom += level;
    setupMatrix();
}

void CacheView::zoomOut(int level) {
    m_zoom -= level;
    setupMatrix();
}

void CacheView::setupMatrix() {
    qreal scale = qPow(qreal(2), (m_zoom - 250) / qreal(50));

    QTransform matrix;
    matrix.scale(scale, scale);

    setTransform(matrix);
}

}  // namespace Ripes
