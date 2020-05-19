#pragma once

#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QLineEdit);

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
    void CCPathChanged(bool valid);
    QLineEdit* m_ccpath = nullptr;

    QWidget* createEditorPage();
    QWidget* createSimulatorPage();
    QWidget* createEnvironmentPage();

    Ui::SettingsDialog* m_ui;

    void addPage(const QString& name, QWidget* page);

    std::map<QString, int> m_pageIndex;
};

}  // namespace Ripes
