#pragma once

#include <QDialog>
#include "ccmanager.h"

QT_FORWARD_DECLARE_CLASS(QLineEdit);
QT_FORWARD_DECLARE_CLASS(QLabel);

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
    QLabel* m_compileInfo = nullptr;
    QLabel* m_compileInfoHeader = nullptr;

    QWidget* createEditorPage();
    QWidget* createSimulatorPage();
    QWidget* createEnvironmentPage();

    Ui::SettingsDialog* m_ui;

    void addPage(const QString& name, QWidget* page);

    std::map<QString, int> m_pageIndex;
};

}  // namespace Ripes
