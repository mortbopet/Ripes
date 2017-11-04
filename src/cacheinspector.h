#ifndef CACHEINSPECTOR_H
#define CACHEINSPECTOR_H

#include <QWidget>

#include "defines.h"
#include "qcustomplot.h"

enum dataRole { hit, miss };

namespace Ui {
class CacheInspector;
}

class CacheInspector : public QWidget {
  Q_OBJECT

public:
  explicit CacheInspector(QWidget *parent = 0);
  ~CacheInspector();

  void updateData(int value, dataRole role, cacheLevel level);

private:
  Ui::CacheInspector *m_ui;

  QCPBars *m_hits;
  QCPBars *m_misses;

  QVector<double> m_missData;
  QVector<double> m_hitData;

  void setupBar(QCPBars *bar);
};

#endif // CACHEINSPECTOR_H
