#pragma once

#include <QDialog>
#include <QPlainTextEdit>

#include "ccmanager.h"

QT_FORWARD_DECLARE_CLASS(QLineEdit);
QT_FORWARD_DECLARE_CLASS(QLabel);
QT_FORWARD_DECLARE_CLASS(QGridLayout);
QT_FORWARD_DECLARE_CLASS(QGroupBox);

namespace Ripes {

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;

    void accept() override;

private:
    /**
     * @brief CCPathChanged
     * Rehighlightss the current CCPath if the CC is @param valid
     */
    void CCPathChanged(CCManager::CCRes res);
    QLineEdit* m_ccpath = nullptr;
    QPlainTextEdit* m_compileInfo = nullptr;
    QLabel* m_compileInfoHeader = nullptr;

    QWidget* createCompilerPage();
    QWidget* createSimulatorPage();
    QWidget* createEnvironmentPage();
    QWidget* createEditorPage();

    Ui::SettingsDialog* m_ui;

    void addPage(const QString& name, QWidget* page);
    void appendToLayout(std::pair<QLabel*, QWidget*> settingsWidgets, QGridLayout* pageLayout,
                        const QString& description = QString());
    void appendToLayout(QWidget* widget, QGridLayout* pageLayout, int colSpan = 2);
    void appendToLayout(QLayout* layout, QGridLayout* pageLayout, int colSpan = 2);

    std::map<QString, int> m_pageIndex;
};

}  // namespace Ripes
