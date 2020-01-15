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
    const LoadFileParams& getParams() const { return m_params; }

private slots:
    void validateCurrentFile();
    void openFileButtonTriggered();
    void inputTypeChanged();

    void updateAssemblyPageState();
    void updateBinaryPageState();
    void updateELFPageState();

    void loadFileError(const QString& filename);

private:
    enum TypeButtonID { Assembly, FlatBinary, ELF };
    static TypeButtonID s_typeIndex;

    bool fileTypeValidate(const QFile& file);
    bool validateAssemblyFile(const QFile& file);
    bool validateBinaryFile(const QFile& file);
    bool validateELFFile(const QFile& file);

    void paletteValidate(QWidget* w, bool valid);

    FileType m_fileType;
    LoadFileParams m_params;

    Ui::LoadDialog* m_ui;
    QButtonGroup* m_fileTypeButtons = nullptr;
};  // namespace Ripes

}  // namespace Ripes
