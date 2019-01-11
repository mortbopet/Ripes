#ifndef CACHEINSPECTOR_H
#define CACHEINSPECTOR_H

#include <QWidget>

#include "defines.h"
#include "qcustomplot.h"

enum class dataRole { hit, miss };

namespace Ui {
class CacheInspector;
}

class CacheInspector : public QWidget {
    Q_OBJECT

public:
    explicit CacheInspector(QWidget* parent = nullptr);
    ~CacheInspector() override;

    void updateData(int value, dataRole role, cacheLevel level);
    void changePlotView(int view);

private:
    Ui::CacheInspector* m_ui;

    void setupCacheRequestPlot();
    void setupCacheRequestRatioPlot();
    void setupTemporalPlot();
    void setupTemporalRatioPlot();

    // Miss data vector:

    QVector<QVector<double>> m_missData;  // For each cache, inner vector shows
                                          // missData for each instruction request
    QVector<QVector<double>> m_hitData;

    void setupBar(QCPBars* bar);
};

#endif  // CACHEINSPECTOR_H
