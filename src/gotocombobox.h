#pragma once

#include <QComboBox>
#include <QMetaType>

#include "ripes_types.h"

namespace Ripes {

enum class GoToFunction { Select, Address, Custom };
struct GoToUserData {
    GoToFunction func;
    unsigned arg;
};

class GoToComboBox : public QComboBox {
    Q_OBJECT
public:
    GoToComboBox(QWidget* parent = nullptr);
    void showPopup() override;

signals:
    void indexChanged();
    void jumpToAddress(Ripes::AInt address);

private:
    void signalFilter(int index);
    virtual void addTargets() = 0;
    virtual AInt addrForIndex(int i) = 0;
};

class GoToSectionComboBox : public GoToComboBox {
public:
    GoToSectionComboBox(QWidget* parent = nullptr) : GoToComboBox(parent) {}

private:
    void addTargets();
    AInt addrForIndex(int i);
};

class GoToRegisterComboBox : public GoToComboBox {
public:
    GoToRegisterComboBox(QWidget* parent = nullptr) : GoToComboBox(parent) {}

private:
    void addTargets();
    AInt addrForIndex(int i);
};

}  // namespace Ripes

Q_DECLARE_METATYPE(Ripes::GoToUserData);
