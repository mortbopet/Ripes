#include "cachegraphic.h"

#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QPen>

#include "processorhandler.h"
#include "radix.h"

namespace {

// Return a set of all keys in a map
template <typename TK, typename TV>
std::set<TK> keys(std::map<TK, TV> const &input_map) {
  std::set<TK> keyset;
  for (auto const &element : input_map) {
    keyset.insert(element.first);
  }
  return keyset;
}
} // namespace

namespace Ripes {

/**
 * @brief The FancyPolyLine class
 * A polygon line with an optional starting dot and ending arrow. The size of
 * these two additions are controlled by the "scale" parameter.
 */
class FancyPolyLine : public QGraphicsItem {
public:
  FancyPolyLine(qreal scale, QGraphicsItem *parent) : QGraphicsItem(parent) {
    QPainterPath pp;
    m_line = new QGraphicsPathItem(pp, this);

    m_startDot =
        new QGraphicsEllipseItem(-scale / 2, -scale / 2, scale, scale, this);
    m_startDot->setBrush(QBrush(Qt::black, Qt::SolidPattern));

    QPolygonF arrow;
    arrow << QPointF{-scale, -scale / 2} << QPointF{0, 0}
          << QPointF{-scale, scale / 2};
    m_arrow = new QGraphicsPolygonItem(arrow, this);
    m_arrow->setBrush(QBrush(Qt::black, Qt::SolidPattern));

    m_startDot->setVisible(false);
    m_arrow->setVisible(false);
  }

  QRectF boundingRect() const override { return childrenBoundingRect(); }
  void paint(QPainter *, const QStyleOptionGraphicsItem *,
             QWidget * = nullptr) override {}

  void setPolygon(const QPolygonF &poly) {
    QPainterPath pp;
    pp.addPolygon(poly);
    m_line->setPath(pp);

    m_startDot->setPos(poly.first());
    m_arrow->setPos(poly.last());
    const auto &p1 = poly.at(poly.size() - 2);
    const auto &p2 = poly.last();
    const qreal ang = std::atan2(p2.y() - p1.y(), p2.x() - p1.x());
    m_arrow->setRotation(ang * (180.0 / M_PI));
  }
  void showStartDot(bool visible) { m_startDot->setVisible(visible); }
  void showEndArrow(bool visible) { m_arrow->setVisible(visible); }
  void setPen(const QPen &pen) { m_line->setPen(pen); }

private:
  QGraphicsEllipseItem *m_startDot = nullptr;
  QGraphicsPathItem *m_line = nullptr;
  QGraphicsPolygonItem *m_arrow = nullptr;
};

CacheGraphic::CacheGraphic(CacheSim &cache)
    : QGraphicsObject(nullptr), m_cache(cache), m_fm(m_font) {
  // Connect to CacheSim::dataChanged using QueuedConnection, to ensure allow
  // for cross thread signalling (CacheSim is executed in the same thread as the
  // simulator).
  connect(&cache, &CacheSim::dataChanged, this, &CacheGraphic::dataChanged,
          Qt::QueuedConnection);
  connect(&cache, &CacheSim::wayInvalidated, this,
          &CacheGraphic::wayInvalidated);
  connect(&cache, &CacheSim::cacheInvalidated, this,
          &CacheGraphic::cacheInvalidated);

  cacheInvalidated();
}

void CacheGraphic::updateLineReplFields(unsigned lineIdx) {
  auto *cacheLine = m_cache.getLine(lineIdx);

  if (cacheLine == nullptr) {
    // Nothing to do
    return;
  }

  if (m_cacheTextItems.at(0).at(0).lru == nullptr) {
    // The current cache configuration does not have any replacement field
    return;
  }

  for (const auto &way : m_cacheTextItems[lineIdx]) {
    // If LRU was just initialized, the actual (software) LRU value may be very
    // large. Mask to the number of actual LRU bits.
    unsigned lruVal = cacheLine->at(way.first).lru;
    lruVal &= vsrtl::generateBitmask(m_cache.getWaysBits());
    const QString lruText = QString::number(lruVal);
    way.second.lru->setText(lruText);

    // LRU text might have changed; update LRU field position to center in
    // column
    const qreal y = lineIdx * m_lineHeight + way.first * m_setHeight;
    const qreal x =
        m_widthBeforeLRU + m_lruWidth / 2 - m_fm.horizontalAdvance(lruText) / 2;

    way.second.lru->setPos(x, y);
  }
}

QString CacheGraphic::addressString() const {
  return "0x" +
         QString("0").repeated(ProcessorHandler::currentISA()->bytes() * 2);
}

void CacheGraphic::updateWay(unsigned lineIdx, unsigned wayIdx) {
  const auto &it = m_cacheTextItems.find(lineIdx);
  if (it == m_cacheTextItems.end()) {
    return;
  }
  const auto &wayIt = it->second.find(wayIdx);
  if (wayIt == it->second.end()) {
    return;
  }
  CacheWay &way = wayIt->second;

  CacheSim::CacheWay simWay = CacheSim::CacheWay();

  if (auto *cacheLine = m_cache.getLine(lineIdx)) {
    simWay = cacheLine->at(wayIdx);
  };

  const unsigned bytes = ProcessorHandler::currentISA()->bytes();
  // ======================== Update block text fields ======================
  if (simWay.valid) {
    for (int i = 0; i < m_cache.getBlocks(); ++i) {
      QGraphicsSimpleTextItem *blockTextItem = nullptr;
      if (way.blocks.count(i) == 0) {
        // Block text item has not yet been created
        const qreal x =
            m_widthBeforeBlocks + i * m_blockWidth +
            (m_blockWidth / 2 - m_fm.horizontalAdvance(addressString()) / 2);
        const qreal y = lineIdx * m_lineHeight + wayIdx * m_setHeight;

        way.blocks[i] = createGraphicsTextItemSP(x, y);
        blockTextItem = way.blocks[i].get();
      } else {
        blockTextItem = way.blocks.at(i).get();
      }

      // Update block text
      const AInt addressForBlock = m_cache.buildAddress(simWay.tag, lineIdx, i);
      const auto data =
          ProcessorHandler::getMemory().readMemConst(addressForBlock, bytes);
      const QString text = encodeRadixValue(
          data, Radix::Hex, ProcessorHandler::currentISA()->bytes());
      blockTextItem->setText(text);
      QString tooltip =
          "Address: " +
          encodeRadixValue(addressForBlock, Radix::Hex,
                           ProcessorHandler::currentISA()->bytes());
      if (simWay.dirtyBlocks.count(i)) {
        tooltip += "\n> Dirty";
      }
      blockTextItem->setToolTip(tooltip);
      // Store the address within the userrole of the block text. Doing this, we
      // are able to easily retrieve the address for the block if the block is
      // clicked.
      blockTextItem->setData(Qt::UserRole,
                             QVariant::fromValue(addressForBlock));
    }
  } else {
    // The way is invalid so no block text should be present
    way.blocks.clear();
  }

  // =========================== Update dirty field =========================
  if (way.dirty) {
    way.dirty->setText(QString::number(simWay.dirty));
  }

  // =========================== Update valid field =========================
  if (way.valid) {
    way.valid->setText(QString::number(simWay.valid));
  }

  // ============================ Update tag field ==========================
  if (simWay.valid) {
    QGraphicsSimpleTextItem *tagTextItem = way.tag.get();
    if (tagTextItem == nullptr) {
      const qreal x =
          m_widthBeforeTag +
          (m_tagWidth / 2 - m_fm.horizontalAdvance(addressString()) / 2);
      const qreal y = lineIdx * m_lineHeight + wayIdx * m_setHeight;
      way.tag = createGraphicsTextItemSP(x, y);
      tagTextItem = way.tag.get();
    }
    const QString tagText = encodeRadixValue(
        simWay.tag, Radix::Hex, ProcessorHandler::currentISA()->bytes());
    tagTextItem->setText(tagText);
  } else {
    // The way is invalid so no tag text should be present
    if (way.tag) {
      way.tag.reset();
    }
  }

  // ==================== Update dirty blocks highlighting ==================
  const std::set<unsigned> graphicDirtyBlocks = keys(way.dirtyBlocks);
  std::set<unsigned> newDirtyBlocks;
  std::set<unsigned> dirtyBlocksToDelete;
  std::set_difference(
      graphicDirtyBlocks.begin(), graphicDirtyBlocks.end(),
      simWay.dirtyBlocks.begin(), simWay.dirtyBlocks.end(),
      std::inserter(dirtyBlocksToDelete, dirtyBlocksToDelete.begin()));
  std::set_difference(simWay.dirtyBlocks.begin(), simWay.dirtyBlocks.end(),
                      graphicDirtyBlocks.begin(), graphicDirtyBlocks.end(),
                      std::inserter(newDirtyBlocks, newDirtyBlocks.begin()));

  // Delete blocks which are not in sync with the current dirty status of the
  // way
  for (const unsigned &blockToDelete : dirtyBlocksToDelete) {
    way.dirtyBlocks.erase(blockToDelete);
  }
  // Create all required new blocks
  for (const auto &blockIdx : newDirtyBlocks) {
    const auto topLeft = QPointF(blockIdx * m_blockWidth + m_widthBeforeBlocks,
                                 lineIdx * m_lineHeight + wayIdx * m_setHeight);
    const auto bottomRight =
        QPointF((blockIdx + 1) * m_blockWidth + m_widthBeforeBlocks,
                lineIdx * m_lineHeight + (wayIdx + 1) * m_setHeight);
    way.dirtyBlocks[blockIdx] =
        std::make_unique<QGraphicsRectItem>(QRectF(topLeft, bottomRight), this);

    const auto &dirtyRectItem = way.dirtyBlocks[blockIdx];
    dirtyRectItem->setZValue(-1);
    dirtyRectItem->setOpacity(0.4);
    dirtyRectItem->setBrush(Qt::darkCyan);
  }
}

QGraphicsSimpleTextItem *
CacheGraphic::tryCreateGraphicsTextItem(QGraphicsSimpleTextItem **item, qreal x,
                                        qreal y) {
  if (*item == nullptr) {
    *item = new QGraphicsSimpleTextItem(this);
    (*item)->setFont(m_font);
    (*item)->setPos(x, y);
  }
  return *item;
}

std::unique_ptr<QGraphicsSimpleTextItem>
CacheGraphic::createGraphicsTextItemSP(qreal x, qreal y) {
  std::unique_ptr<QGraphicsSimpleTextItem> ptr =
      std::make_unique<QGraphicsSimpleTextItem>(this);
  ptr->setFont(m_font);
  ptr->setPos(x, y);
  return ptr;
}

void CacheGraphic::drawIndexingItems() {
  QPen wirePen;
  wirePen.setColor(Qt::darkGray);

  const auto drawBundleDropWires = [=](const qreal sourceD,
                                       const QVector<QPointF> &dropoffs,
                                       bool horizontal) {
    const qreal maxD = std::max_element(dropoffs.begin(), dropoffs.end(),
                                        [=](const auto &p1, const auto &p2) {
                                          return horizontal ? p1.x() > p2.x()
                                                            : p1.y() > p2.y();
                                        })
                           ->y();
    const qreal dPos = maxD - m_setHeight;
    const QPointF pSource =
        QPointF(horizontal ? dPos : sourceD, horizontal ? sourceD : dPos);
    std::vector<FancyPolyLine *> lines;
    int i = 1;
    for (const auto &pDrop : dropoffs) {
      QPolygonF poly;
      poly << pSource;
      poly << QPointF(horizontal ? pSource.x() : pDrop.x(),
                      horizontal ? pDrop.y() : pSource.y());
      poly << pDrop;
      auto *polyLine = new FancyPolyLine(m_setHeight / 3, this);
      polyLine->showStartDot(false);
      polyLine->showEndArrow(true);
      polyLine->setPolygon(poly);
      polyLine->setPen(wirePen);
      lines.push_back(polyLine);
      polyLine->setZValue(-i);
      i++;
    }
    return lines;
  };

  // Draw address box
  const QString addressText = QString("-").repeated(32);
  m_addressTextItem = drawText(addressText, 0, 0 - m_setHeight * 3.5);
  auto addressTextRect = m_addressTextItem->boundingRect();
  addressTextRect.moveTo(m_addressTextItem->pos());
  auto *addressRectItem = new QGraphicsRectItem(addressTextRect, this);
  addressRectItem->setBrush(Qt::white);
  addressRectItem->setZValue(z_wires);

  // Draw address box compartments, starting from the righthand side
  const QString botBits = "0";
  QPointF nextBitsPos = {m_addressTextItem->pos().x() + addressTextRect.width(),
                         m_addressTextItem->pos().y()};
  auto smallerFont = m_font;
  auto smallerFontMetric = QFontMetricsF(smallerFont);
  smallerFont.setPointSize(m_font.pointSize() * 0.8);
  drawText(botBits, nextBitsPos.x(), nextBitsPos.y() - m_setHeight,
           &smallerFont);
  const QString topBits = "31";
  drawText(topBits,
           m_addressTextItem->pos().x() -
               smallerFontMetric.horizontalAdvance(topBits),
           m_addressTextItem->pos().y() - m_setHeight, &smallerFont);

  unsigned bitIdx = 0;
  auto drawNextBitPos = [&](unsigned additionalOffset) {
    nextBitsPos -=
        {m_fm.horizontalAdvance(QString("-").repeated(additionalOffset)), 0};
    bitIdx += additionalOffset;
    auto line = new QGraphicsLineItem(
        QLineF{nextBitsPos, nextBitsPos + QPointF{0, m_setHeight}}, this);
    auto pen = line->pen();
    pen.setWidthF(pen.widthF() * 0.8);
    line->setPen(pen);
    const QString thisBitText = QString::number(bitIdx - 1);
    const QString nextBitText = QString::number(bitIdx);
    drawText(thisBitText, nextBitsPos - QPointF{0, smallerFontMetric.height()},
             &smallerFont);
    drawText(nextBitText,
             nextBitsPos - QPointF{m_fm.horizontalAdvance(nextBitText),
                                   smallerFontMetric.height()},
             &smallerFont);
  };

  drawNextBitPos(log2Ceil(
      ProcessorHandler::currentISA()->bytes())); // Account for word indexing
  if (m_cache.getBlockBits() > 0) {
    m_blockIndexStartPoint = nextBitsPos;
    drawNextBitPos(m_cache.getBlockBits());
    m_blockIndexStartPoint.rx() -=
        (m_blockIndexStartPoint - nextBitsPos).x() / 2;
    m_blockIndexStartPoint.ry() += m_setHeight;
  }
  if (m_cache.getLineBits() > 0) {
    m_lineIndexStartPoint = nextBitsPos;
    drawNextBitPos(m_cache.getLineBits());
    m_lineIndexStartPoint.rx() -= (m_lineIndexStartPoint - nextBitsPos).x() / 2;
    m_lineIndexStartPoint.ry() += m_setHeight;
  }

  m_tagAddressStartPoint = m_addressTextItem->pos();
  m_tagAddressStartPoint.ry() += m_setHeight;
  m_tagAddressStartPoint.rx() += (nextBitsPos - m_tagAddressStartPoint).x() / 2;

  m_lineIndexingLine = new FancyPolyLine(m_setHeight / 3, this);
  m_lineIndexingLine->setZValue(z_wires);
  m_lineIndexingLine->showStartDot(false);
  m_lineIndexingLine->showEndArrow(true);
  m_lineIndexingLine->setVisible(false);

  m_blockIndexingLine = new FancyPolyLine(m_setHeight / 3, this);
  m_blockIndexingLine->setZValue(z_wires);
  m_blockIndexingLine->showStartDot(false);
  m_blockIndexingLine->showEndArrow(true);
  m_blockIndexingLine->setVisible(false);

  // Draw top header
  const QString addrStr = "Access address";
  QPointF addrStrPos = m_addressTextItem->pos();
  addrStrPos.rx() -= m_fm.horizontalAdvance(addrStr) * 1.1;
  //  addrStrPos.ry() += m_setHeight;
  drawText(addrStr, addrStrPos);
}

void CacheGraphic::cacheInvalidated() {
  // Remove all items
  m_highlightingItems.clear();
  m_cacheTextItems.clear();
  m_addressTextItem = nullptr;
  m_blockIndexingLine = nullptr;
  m_lineIndexingLine = nullptr;
  for (const auto &item : childItems()) {
    delete item;
  }

  // Determine cell dimensions
  m_setHeight = m_fm.height();
  m_lineHeight = m_setHeight * m_cache.getWays();
  m_blockWidth = m_fm.horizontalAdvance(" " + addressString() + " ");
  m_bitWidth = m_fm.horizontalAdvance("00");
  m_lruWidth = m_fm.horizontalAdvance(QString::number(m_cache.getWays()) + " ");
  m_cacheHeight = m_lineHeight * m_cache.getLines();
  m_tagWidth = m_blockWidth;

  // Draw cache:
  new QGraphicsLineItem(0, 0, 0, m_cacheHeight, this);
  qreal width = 0;
  // Draw valid bit column
  new QGraphicsLineItem(m_bitWidth, 0, m_bitWidth, m_cacheHeight, this);
  const QString validBitText = "V";
  auto *validItem = drawText(validBitText, 0, -m_fm.height());
  validItem->setToolTip("Valid bit");
  width += m_bitWidth;

  if (m_cache.getWritePolicy() == WritePolicy::WriteBack) {
    m_widthBeforeDirty = width;

    // Draw dirty bit column
    new QGraphicsLineItem(width + m_bitWidth, 0, width + m_bitWidth,
                          m_cacheHeight, this);
    const QString dirtyBitText = "D";
    auto *dirtyItem =
        drawText(dirtyBitText, m_widthBeforeDirty, -m_fm.height());
    dirtyItem->setToolTip("Dirty bit");
    width += m_bitWidth;
  }

  m_widthBeforeLRU = width;

  if (m_cache.getReplacementPolicy() == ReplPolicy::LRU &&
      m_cache.getWays() > 1) {
    // Draw LRU bit column
    new QGraphicsLineItem(width + m_lruWidth, 0, width + m_lruWidth,
                          m_cacheHeight, this);
    const QString LRUBitText = "LRU";
    auto *textItem = drawText(LRUBitText,
                              width + m_lruWidth / 2 -
                                  m_fm.horizontalAdvance(LRUBitText) / 2,
                              -m_fm.height());
    textItem->setToolTip("Least Recently Used bits");
    width += m_lruWidth;
  }

  m_widthBeforeTag = width;

  // Draw tag column
  new QGraphicsLineItem(m_tagWidth + width, 0, m_tagWidth + width,
                        m_cacheHeight, this);
  const QString tagText = "Tag";
  drawText(tagText,
           width + m_tagWidth / 2 - m_fm.horizontalAdvance(tagText) / 2,
           -m_fm.height());

  width += m_tagWidth;
  m_widthBeforeBlocks = width;

  // Draw horizontal lines between cache blocks
  for (int i = 0; i < m_cache.getBlocks(); ++i) {
    const QString blockText = "Word " + QString::number(i);
    drawText(blockText,
             width + m_tagWidth / 2 - m_fm.horizontalAdvance(blockText) / 2,
             -m_fm.height());
    width += m_blockWidth;
    new QGraphicsLineItem(width, 0, width, m_cacheHeight, this);
  }

  m_cacheWidth = width;

  // Draw cache line rows
  for (int i = 0; i <= m_cache.getLines(); ++i) {
    qreal verticalAdvance = i * m_lineHeight;
    new QGraphicsLineItem(0, verticalAdvance, m_cacheWidth, verticalAdvance,
                          this);

    if (i < m_cache.getLines()) {
      // Draw cache set rows
      for (int j = 1; j < m_cache.getWays(); j++) {
        verticalAdvance += m_setHeight;
        auto *setLine = new QGraphicsLineItem(0, verticalAdvance, m_cacheWidth,
                                              verticalAdvance, this);
        auto pen = setLine->pen();
        pen.setStyle(Qt::DashLine);
        setLine->setPen(pen);
      }
    }
  }

  // Draw line index numbers
  for (int i = 0; i < m_cache.getLines(); ++i) {
    const QString text = QString::number(i);

    const qreal y = i * m_lineHeight + m_lineHeight / 2 - m_setHeight / 2;
    const qreal x = -m_fm.horizontalAdvance(text) * 1.2;
    drawText(text, x, y);
  }

  // Draw index column text
  const QString indexText = "Index";
  const qreal x = -m_fm.horizontalAdvance(indexText) * 1.2;
  drawText(indexText, x, -m_fm.height());

  // Draw indexing - this does not change the 'width' advancement, and is purely
  // based on the tag column positioning.
  if (m_indexingVisible) {
    drawIndexingItems();
  }

  initializeControlBits();

  // Update all entries in the cache
  for (int lineIdx = 0; lineIdx < m_cache.getLines(); lineIdx++) {
    if (const auto *line = m_cache.getLine(lineIdx)) {
      for (const auto &way : *line) {
        updateWay(lineIdx, way.first);
      }
      updateLineReplFields(lineIdx);
    }
  }

  if (auto *_scene = scene()) {
    // Invalidate the scene rect to resize it to the current dimensions of the
    // CacheGraphic
    _scene->setSceneRect({});
  }
}

void CacheGraphic::updateAddressing(
    bool valid, const CacheSim::CacheTransaction &transaction) {
  if (m_indexingVisible) {
    if (valid) {
      const auto &it = m_cacheTextItems.find(transaction.index.line);
      if (it == m_cacheTextItems.end()) {
        return;
      }
      const auto &wayIt = it->second.find(transaction.index.way);
      if (wayIt == it->second.end()) {
        return;
      }

      const CacheWay &way = wayIt->second;
      m_addressTextItem->setText(
          QString::number(transaction.address, 2).rightJustified(32, '0'));

      if (way.tag) {
        QPolygonF lineIndexingPoly;
        m_lineIndexingLine->setVisible(true);
        lineIndexingPoly << m_lineIndexStartPoint;
        lineIndexingPoly << m_lineIndexStartPoint +
                                QPointF(0, (m_setHeight / 2));
        lineIndexingPoly << QPointF(-m_tagWidth * 0.75,
                                    lineIndexingPoly.last().y());
        lineIndexingPoly << QPointF{lineIndexingPoly.last().x(),
                                    (transaction.index.line + 0.5) *
                                        m_lineHeight};
        lineIndexingPoly << QPointF{
            -m_fm.horizontalAdvance(QString::number(m_cache.getLines())) * 1.25,
            lineIndexingPoly.last().y()};
        m_lineIndexingLine->setPolygon(lineIndexingPoly);

        QPolygonF blockIndexingPoly;
        blockIndexingPoly << m_blockIndexStartPoint;
        blockIndexingPoly << m_blockIndexStartPoint +
                                 QPointF(0, (m_setHeight / 2) * 1.5);
        blockIndexingPoly << QPointF(m_widthBeforeBlocks +
                                         m_blockWidth *
                                             (transaction.index.block + 0.5),
                                     blockIndexingPoly.last().y());
        blockIndexingPoly << QPointF(
            blockIndexingPoly.last().x(),
            -m_addressTextItem->boundingRect().size().height());
        m_blockIndexingLine->setPolygon(blockIndexingPoly);
        m_blockIndexingLine->showStartDot(false);
        m_blockIndexingLine->showEndArrow(true);
        m_blockIndexingLine->setVisible(true);
      }

    } else {
      m_addressTextItem->setText(QString("-").repeated(32));
    }
  }
}

void CacheGraphic::wayInvalidated(unsigned lineIdx, unsigned wayIdx) {
  updateWay(lineIdx, wayIdx);
  updateLineReplFields(lineIdx);
}

void CacheGraphic::dataChanged(CacheSim::CacheTransaction transaction) {
  if (transaction.type != MemoryAccess::None) {
    wayInvalidated(transaction.index.line, transaction.index.way);
    updateAddressing(true, transaction);
    updateHighlighting(true, transaction);
  } else {
    updateHighlighting(false, transaction);
    updateAddressing(false, transaction);
  }
}

QGraphicsSimpleTextItem *CacheGraphic::drawText(const QString &text,
                                                const QPointF &pos,
                                                const QFont *otherFont) {
  return drawText(text, pos.x(), pos.y(), otherFont);
}

QGraphicsSimpleTextItem *CacheGraphic::drawText(const QString &text, qreal x,
                                                qreal y,
                                                const QFont *otherFont) {
  auto *textItem = new QGraphicsSimpleTextItem(text, this);
  if (otherFont) {
    textItem->setFont(*otherFont);
  } else {
    textItem->setFont(m_font);
  }
  textItem->setPos(x, y);
  return textItem;
}

void CacheGraphic::updateHighlighting(
    bool active, const CacheSim::CacheTransaction &transaction) {
  m_highlightingItems.clear();

  if (active) {
    // Redraw highlighting rectangles indicating the current indexing

    // Draw cache line highlighting rectangle
    QPointF topLeft = QPointF(0, transaction.index.line * m_lineHeight);
    QPointF bottomRight =
        QPointF(m_cacheWidth, (transaction.index.line + 1) * m_lineHeight);
    m_highlightingItems.emplace_back(std::make_unique<QGraphicsRectItem>(
        QRectF(topLeft, bottomRight), this));
    auto *lineRectItem = m_highlightingItems.rbegin()->get();
    lineRectItem->setZValue(-2);
    lineRectItem->setOpacity(0.25);
    lineRectItem->setBrush(Qt::yellow);

    // Draw cache block highlighting rectangle
    topLeft = QPointF(
        transaction.index.block * m_blockWidth + m_widthBeforeBlocks, 0);
    bottomRight = QPointF((transaction.index.block + 1) * m_blockWidth +
                              m_widthBeforeBlocks,
                          m_cacheHeight);
    m_highlightingItems.emplace_back(std::make_unique<QGraphicsRectItem>(
        QRectF(topLeft, bottomRight), this));
    auto *blockRectItem = m_highlightingItems.rbegin()->get();
    blockRectItem->setZValue(-2);
    blockRectItem->setOpacity(0.25);
    blockRectItem->setBrush(Qt::yellow);

    // Draw highlighting on the currently accessed block
    topLeft =
        QPointF(transaction.index.block * m_blockWidth + m_widthBeforeBlocks,
                transaction.index.line * m_lineHeight +
                    transaction.index.way * m_setHeight);
    bottomRight = QPointF((transaction.index.block + 1) * m_blockWidth +
                              m_widthBeforeBlocks,
                          transaction.index.line * m_lineHeight +
                              (transaction.index.way + 1) * m_setHeight);
    m_highlightingItems.emplace_back(std::make_unique<QGraphicsRectItem>(
        QRectF(topLeft, bottomRight), this));
    auto *hitRectItem = m_highlightingItems.rbegin()->get();
    hitRectItem->setZValue(-1);
    if (transaction.isHit) {
      hitRectItem->setOpacity(0.4);
      hitRectItem->setBrush(Qt::green);
    } else {
      hitRectItem->setOpacity(0.8);
      hitRectItem->setBrush(Qt::red);
    }
  }
}

void CacheGraphic::initializeControlBits() {
  for (int lineIdx = 0; lineIdx < m_cache.getLines(); lineIdx++) {
    auto &line = m_cacheTextItems[lineIdx];
    for (int setIdx = 0; setIdx < m_cache.getWays(); setIdx++) {
      const qreal y = lineIdx * m_lineHeight + setIdx * m_setHeight;
      qreal x;

      // Create valid field
      x = m_bitWidth / 2 - m_fm.horizontalAdvance("0") / 2;
      line[setIdx].valid = drawText("0", x, y);

      if (m_cache.getWritePolicy() == WritePolicy::WriteBack) {
        // Create dirty bit field
        x = m_widthBeforeDirty + m_bitWidth / 2 -
            m_fm.horizontalAdvance("0") / 2;
        line[setIdx].dirty = drawText("0", x, y);
      }

      if (m_cache.getReplacementPolicy() == ReplPolicy::LRU &&
          m_cache.getWays() > 1) {
        // Create LRU field
        const QString lruText = QString::number(m_cache.getWays() - 1);
        x = m_widthBeforeLRU + m_lruWidth / 2 -
            m_fm.horizontalAdvance(lruText) / 2;
        line[setIdx].lru = drawText(lruText, x, y);
      }
    }
  }
}

} // namespace Ripes
