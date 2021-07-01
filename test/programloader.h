#pragma once

#include <QObject>
#include <QTest>

#include "edittab.h"
#include "processorhandler.h"
#include "processorregistry.h"

#include <optional>

namespace Ripes {

/// Utility class around a Ripes EditTab which helps in loading and assembling programs from tests.
class ProgramLoader : public QObject {
    Q_OBJECT

public:
    ProgramLoader() {
        m_editTab = new EditTab(new QToolBar(), nullptr);
        connect(m_editTab, &EditTab::programChanged, ProcessorHandler::get(), &ProcessorHandler::loadProgram);
    }

    void loadTest(LoadFileParams params) {
        currentTestType = params.type;
        m_editTab->loadExternalFile(params);
        processNewTest();
    }

    void loadTest(const QString& assembly) {
        currentTestType = SourceType::Assembly;
        m_editTab->loadSourceText(assembly);
        processNewTest();
    }

private:
    // Load a program through the edittab. This is not really suited for automatic testing, since the edit tab will
    // trigger assembling after some timeout. To work around this, we allow for a bit of delay when loading the program.
    void processNewTest() {
        if (currentTestType == SourceType::Assembly)
            m_editTab->sourceCodeChanged();

        int timeouts = 5;
        while (timeouts-- > 0 && !ProcessorHandler::getProgram()) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 500);
        }
        if (!ProcessorHandler::getProgram()) {
            QFAIL("No program was loaded!");
        }
        if (m_editTab->errors() && !m_editTab->errors()->empty())
            QFAIL("Errors during assembly!");
    }

    SourceType currentTestType;
    EditTab* m_editTab = nullptr;
};

}  // namespace Ripes
