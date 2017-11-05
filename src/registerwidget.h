#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QValidator>
#include <QWidget>

const static QStringList displayTypes = QStringList() << "Hex"
                                                      << "Binary"
                                                      << "Decimal"
                                                      << "Unsigned"
                                                      << "ASCII"
                                                      << "Float (IEEE-754)";

namespace Ui {
class RegisterWidget;
}

namespace {
typedef QPair<long long, long long> rangePair;
}

/*
 * Widget for representing register contents
*/

class RegisterWidget : public QWidget {
  Q_OBJECT

public:
  explicit RegisterWidget(QWidget *parent = 0);
  ~RegisterWidget();

  void setAlias(QString text);
  void setNumber(int number);
  void setDisplayType(QString type);
  void setRegPtr(uint32_t *ptr) { m_regPtr = ptr; }
  void enableInput(bool state);

public slots:
  void setText();

private slots:
  void validateInput();

private:
  Ui::RegisterWidget *m_ui;
  QIntValidator m_validator;
  QString m_displayType;
  int m_displayBase = 10;
  uint32_t *m_regPtr;

  rangePair m_range;
};

#endif // REGISTERWIDGET_H
