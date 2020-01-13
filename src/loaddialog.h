#pragma once
#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QButtonGroup);
QT_FORWARD_DECLARE_CLASS(QFile);

#include "program.h"

namespace Ripes {

namespace Ui {
class LoadDialog;
}

class LoadDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoadDialog(QWidget* parent = nullptr);
    ~LoadDialog();

    void accept() override;

private slots:
    void validateCurrentFile();
    void openFileButtonTriggered();
    void inputTypeChanged();

    void updateAssemblyPageState();
    void updateBinaryPageState();
    void updateELFPageState();

    void loadFileError(const QString& filename);

private:
    bool fileTypeValidate(const QFile& file);
    bool validateAssemblyFile(const QFile& file);
    bool validateBinaryFile(const QFile& file);
    bool validateELFFile(const QFile& file);

    void paletteValidate(QWidget* w, bool valid);

    FileType m_currentFileType;
    Ui::LoadDialog* m_ui;
    QButtonGroup* m_fileTypeButtons = nullptr;
};

}  // namespace Ripes
