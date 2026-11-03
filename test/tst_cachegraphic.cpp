#include <QCoreApplication>
#include <QGraphicsSimpleTextItem>
#include <QRegularExpression>
#include <QtTest/QTest>

#include <optional>

#include "cachesim/cachegraphic.h"
#include "cachesim/cachesim.h"

using namespace Ripes;

// Tests that hovering a cache cell describes what is drawn in that cell.
class tst_cachegraphic : public QObject {
  Q_OBJECT

private slots:
  void tst_tooltips();
};

// Returns the center of the cache cell displaying the word at @p address.
static std::optional<QPointF> cellCenter(const CacheGraphic &graphic,
                                         AInt address) {
  for (auto *item : graphic.childItems()) {
    const QVariant data = item->data(Qt::UserRole);
    if (data.isValid() && data.value<AInt>() == address) {
      return item->mapToParent(item->boundingRect().center());
    }
  }
  return std::nullopt;
}

void tst_cachegraphic::tst_tooltips() {
  CacheSim cache(nullptr);
  cache.setWays(2);
  cache.setLines(1);
  cache.setBlocks(1);
  cache.setWritePolicy(WritePolicy::WriteBack);
  cache.setWriteAllocatePolicy(WriteAllocPolicy::WriteAllocate);
  cache.setReplacementPolicy(ReplPolicy::LRU);
  CacheGraphic graphic(cache);

  QVERIFY(graphic.toolTipAt({-10, -10}).isEmpty());

  const AInt address = 0x1000;
  cache.access(address, MemoryAccess::Read);
  QCoreApplication::processEvents(); // dataChanged is a queued connection

  const auto center = cellCenter(graphic, address);
  QVERIFY(center.has_value());
  QString tooltip = graphic.toolTipAt(*center);
  QVERIFY2(tooltip.contains(QRegularExpression("Address: 0x0*1000\\n")),
           qPrintable(tooltip));
  QVERIFY2(tooltip.contains("Read miss (red)"), qPrintable(tooltip));
  QVERIFY2(tooltip.contains("empty way"), qPrintable(tooltip));
  QVERIFY2(tooltip.contains("Highlighted row"), qPrintable(tooltip));
  QVERIFY2(tooltip.contains("Highlighted column"), qPrintable(tooltip));
  QVERIFY2(tooltip.contains("LRU: 0 (most recently used)"),
           qPrintable(tooltip));

  // The other way of the same line has not been filled yet.
  const qreal setHeight = QFontMetricsF(QFont(Fonts::monospace, 12)).height();
  tooltip = graphic.toolTipAt(*center + QPointF(0, setHeight));
  QVERIFY2(tooltip.contains("Empty"), qPrintable(tooltip));
  QVERIFY2(!tooltip.contains("Read miss"), qPrintable(tooltip));

  cache.access(address, MemoryAccess::Write);
  QCoreApplication::processEvents();
  tooltip = graphic.toolTipAt(*center);
  QVERIFY2(tooltip.contains("Write hit (green)"), qPrintable(tooltip));
  QVERIFY2(tooltip.contains("Dirty (cyan)"), qPrintable(tooltip));
}

QTEST_MAIN(tst_cachegraphic)
#include "tst_cachegraphic.moc"
