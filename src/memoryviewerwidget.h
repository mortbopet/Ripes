#pragma once

#include <QWidget>

#include "memorymodel.h"

namespace Ripes {

class RadixSelectorWidget;
class GoToComboBox;

namespace Ui {
class MemoryViewerWidget;
}

class MemoryViewerWidget : public QWidget {
    Q_OBJECT

public:
    MemoryViewerWidget(QWidget* parent = nullptr);
    ~MemoryViewerWidget();

    void updateModel();

    MemoryModel* m_memoryModel = nullptr;

public slots:
    void updateView();
    void setCentralAddress(uint32_t address);

private:
    void setupNavigationWidgets();

    Ui::MemoryViewerWidget* m_ui = nullptr;

    RadixSelectorWidget* m_radixSelector = nullptr;
    GoToComboBox* m_goToSection = nullptr;
    GoToComboBox* m_goToRegister = nullptr;
};
}  // namespace Ripes
