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

class RegisterWidget : public QWidget {
  Q_OBJECT

public:
  explicit RegisterWidget(QWidget *parent = 0);
  ~RegisterWidget();

  void setAlias(QString text);
  void setNumber(int number);
  void setDisplayType(QString type);
  void setRegPtr(uint32_t *ptr) { m_regPtr = ptr; }

private:
  Ui::RegisterWidget *m_ui;
  QIntValidator m_validator;

  uint32_t *m_regPtr;
};

#endif // REGISTERWIDGET_H
