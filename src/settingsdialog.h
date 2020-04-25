#pragma once

#include <QDialog>

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
    QWidget* createEditorPage();
    QWidget* createSimulatorPage();

    Ui::SettingsDialog* m_ui;

    void addPage(const QString& name, QWidget* page);

    std::map<QString, int> m_pageIndex;
};

}  // namespace Ripes
