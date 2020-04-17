#include "cacheview.h"

#include <qmath.h>
#include <QGraphicsSimpleTextItem>
#include <QWheelEvent>

namespace Ripes {

CacheView::CacheView(QWidget* parent) : QGraphicsView(parent) {
    m_zoom = 250;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setRenderHint(QPainter::Antialiasing, false);
    setInteractive(true);
    setupMatrix();
}

void CacheView::mousePressEvent(QMouseEvent* event) {
    // If we press on a cache data block, get the address stored for that block and emit a signal indicating that the
    // address was selected through the cache
    for (const auto& item : items(event->pos())) {
        if (auto* textItem = dynamic_cast<QGraphicsSimpleTextItem*>(item)) {
            const QVariant data = textItem->data(Qt::UserRole);
            if (data.isValid()) {
                emit cacheAddressSelected(data.toUInt());
                break;
            }
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void CacheView::wheelEvent(QWheelEvent* e) {
    if (e->modifiers() & Qt::ControlModifier) {
        if (e->delta() > 0)
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

    QMatrix matrix;
    matrix.scale(scale, scale);

    setMatrix(matrix);
}

}  // namespace Ripes
