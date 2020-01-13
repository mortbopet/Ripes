#pragma once

#include <QWidget>

#include "memorymodel.h"

namespace Ripes {

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
    Ui::MemoryViewerWidget* m_ui;
};
}  // namespace Ripes
