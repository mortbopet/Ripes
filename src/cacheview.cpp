#include "cacheview.h"

#include <qmath.h>
#include <QWheelEvent>

namespace Ripes {

CacheView::CacheView(QWidget* parent) : QGraphicsView(parent) {
    m_zoom = 250;

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setRenderHint(QPainter::Antialiasing, false);
    setInteractive(true);
    setupMatrix();
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
