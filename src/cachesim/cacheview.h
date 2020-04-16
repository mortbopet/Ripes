#pragma once

#include <QGraphicsView>

namespace Ripes {

class CacheView : public QGraphicsView {
public:
    CacheView(QWidget* parent);

protected:
    void wheelEvent(QWheelEvent*) override;

private slots:
    void setupMatrix();
    void zoomIn(int level = 1);
    void zoomOut(int level = 1);

private:
    qreal m_zoom;
};

}  // namespace Ripes
