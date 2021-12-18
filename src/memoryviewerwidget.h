#pragma once

#include <QWidget>

#include "ripes_types.h"

namespace Ripes {

class RadixSelectorWidget;
class GoToComboBox;
class MemoryModel;

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
    void setCentralAddress(Ripes::AInt address);

private:
    void setupNavigationWidgets();

    Ui::MemoryViewerWidget* m_ui = nullptr;

    RadixSelectorWidget* m_radixSelector = nullptr;
    GoToComboBox* m_goToSection = nullptr;
    GoToComboBox* m_goToRegister = nullptr;
};
}  // namespace Ripes
