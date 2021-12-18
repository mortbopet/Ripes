#pragma once

#include <QGraphicsView>

#include "ripes_types.h"

namespace Ripes {

class CacheView : public QGraphicsView {
    Q_OBJECT
public:
    CacheView(QWidget* parent);
    void fitScene();

protected:
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;

signals:
    void cacheAddressSelected(Ripes::AInt);

private slots:
    void setupMatrix();
    void zoomIn(int level = 1);
    void zoomOut(int level = 1);

private:
    qreal m_zoom;
};

}  // namespace Ripes
