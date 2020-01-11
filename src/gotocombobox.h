#ifndef GOTOCOMBOBOX_H
#define GOTOCOMBOBOX_H

#include <QComboBox>
#include <QMetaType>

#include "processorhandler.h"

enum class GoToFunction { Select, Address };
Q_DECLARE_METATYPE(GoToFunction);

class GoToComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit GoToComboBox(QWidget* parent = nullptr);
    void setHandler(ProcessorHandler* handler);
    void showPopup() override;

signals:
    void indexChanged();
    void jumpToAddress(uint32_t address);

public slots:

private:
    void signalFilter(int index);
    ProcessorHandler* m_handler;
};

#endif  // GOTOCOMBOBOX_H
