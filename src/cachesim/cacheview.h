#pragma once

#include <QGraphicsView>

namespace Ripes {

class CacheView : public QGraphicsView {
    Q_OBJECT
public:
    CacheView(QWidget* parent);

protected:
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;

signals:
    void cacheAddressSelected(uint32_t);

private slots:
    void setupMatrix();
    void zoomIn(int level = 1);
    void zoomOut(int level = 1);

private:
    qreal m_zoom;
};

}  // namespace Ripes
