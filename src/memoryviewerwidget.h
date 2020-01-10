#pragma once

#include <QWidget>

#include "memorymodel.h"
#include "processorhandler.h"

namespace Ui {
class MemoryViewerWidget;
}

class MemoryViewerWidget : public QWidget {
    Q_OBJECT

public:
    explicit MemoryViewerWidget(QWidget* parent = nullptr);
    ~MemoryViewerWidget();

    void updateModel();
    void setHandler(ProcessorHandler* handler);

    NewMemoryModel* m_memoryModel = nullptr;

public slots:
    void updateView();

private:
    Ui::MemoryViewerWidget* m_ui;
    ProcessorHandler* m_handler = nullptr;
};
