#pragma once

#include <QDialog>

namespace Ripes {

namespace Ui {
class SaveDialog;
}

class SaveDialog : public QDialog {
    Q_OBJECT

public:
    explicit SaveDialog(QWidget* parent = nullptr);
    ~SaveDialog();

    static const QString& getPath() { return m_path; }
    static QString assemblyPath() { return m_saveAssembly ? m_path + ".s" : QString(); }
    static QString binaryPath() { return m_saveBinary ? m_path + ".bin" : QString(); }

    void accept() override;

private:
    void openFileButtonTriggered();
    void pathChanged();

    Ui::SaveDialog* m_ui;

    static QString m_path;
    static bool m_saveAssembly;
    static bool m_saveBinary;
};

}  // namespace Ripes
