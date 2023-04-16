#pragma once

#include <QGridLayout>
#include <QLabel>
#include <QPen>
#include <QVariant>
#include <QWidget>
#include <QtCore/QPropertyAnimation>
#include <QtWidgets/QAbstractButton>

#include "iobase.h"

namespace Ripes {

/**
 * Toggle button with slider. Based on
 * https://codereview.stackexchange.com/questions/249076/implementing-toggle-button-using-qt
 */

class ToggleButton : public QAbstractButton {
  Q_OBJECT
  Q_PROPERTY(int mOffset READ offset WRITE setOffset NOTIFY mOffsetChanged);

public:
  explicit ToggleButton(int trackRadius, int thumbRadius, bool rotated,
                        QWidget *parent = nullptr);
  ~ToggleButton();

  QSize sizeHint() const override;

signals:
  void mOffsetChanged(int);

protected:
  void paintEvent(QPaintEvent *) override;
  void resizeEvent(QResizeEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void enterEvent(QEnterEvent *event) override;
  void setChecked(bool checked);

  int offset();
  void setOffset(int value);

private:
  bool mRotated = false;
  qreal mOffset;
  qreal mBaseOffset;
  qreal mMargin;
  qreal mTrackRadius;
  qreal mThumbRadius;
  qreal mOpacity;
  QPropertyAnimation *mAnimation;

  QHash<bool, qreal> mEndOffset;
  QHash<bool, QBrush> mTrackColor;
  QHash<bool, QBrush> mThumbColor;
  QHash<bool, QColor> mTextColor;
  QHash<bool, QString> mThumbText;
};

class IOSwitches : public IOBase {
  Q_OBJECT

  enum Parameters { SWITCHES };

public:
  IOSwitches(QWidget *parent);
  ~IOSwitches() { unregister(); };

  virtual unsigned byteSize() const override { return 4; }
  virtual QString description() const override;
  virtual QString baseName() const override { return "Switches"; };

  virtual const std::vector<RegDesc> &registers() const override {
    return m_regDescs;
  };
  virtual const std::vector<IOSymbol> *extraSymbols() const override {
    return &m_extraSymbols;
  }

  /**
   * Hardware read/write functions
   */
  virtual VInt ioRead(AInt offset, unsigned size) override;
  virtual void ioWrite(AInt offset, VInt value, unsigned size) override;

protected:
  virtual void parameterChanged(unsigned) override { updateSwitches(); };

private:
  void updateSwitches();

  uint32_t regRead(AInt offset) const;
  std::map<unsigned, std::pair<QLabel *, ToggleButton *>> m_switches;
  QGridLayout *m_switchLayout;
  std::vector<RegDesc> m_regDescs;
  std::vector<IOSymbol> m_extraSymbols;
};
} // namespace Ripes
