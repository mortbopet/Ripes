#include "consolewidget.h"
#include "ui_consolewidget.h"

#include "console.h"
#include "fonts.h"
#include "ripessettings.h"
#include "syscall/systemio.h"

#include <QPushButton>
#include <QScrollBar>

namespace Ripes {

ConsoleWidget::ConsoleWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::ConsoleWidget) {
    m_ui->setupUi(this);

    connect(m_ui->clearConsoleButton, &QPushButton::clicked, m_ui->console, &Console::clearConsole);
    m_ui->clearConsoleButton->setIcon(QIcon(":/icons/clear.svg"));
    m_ui->clearConsoleButton->setToolTip("Clear console");

    // Send input data from the console to the SystemIO stdin stream
    connect(m_ui->console, &Console::sendData, &SystemIO::get(), &SystemIO::putStdInData);
}

ConsoleWidget::~ConsoleWidget() {
    delete m_ui;
}

void ConsoleWidget::putData(const QByteArray& d) {
    m_ui->console->putData(d);
}

void ConsoleWidget::clearConsole() {
    m_ui->console->clearConsole();
}

}  // namespace Ripes
