#pragma once
#include <QDialog>

namespace Ripes {
namespace Ui {
class CompilerErrorDialog;
}

/**
 * @brief The CompilerErrorDialog class
 * Small class for displaying a QRichTextEdit along with a label, to nicely display a compiler output message. This
 * class is used instead of QMessageBox and its "set detailed text" functionality, due to the missing option of manually
 * resizing a QMessageBox.
 */
class CompilerErrorDialog : public QDialog {
    Q_OBJECT

public:
    explicit CompilerErrorDialog(QWidget* parent = nullptr);
    ~CompilerErrorDialog();

    void setText(const QString&);
    void setErrorText(const QString&);

private:
    Ui::CompilerErrorDialog* m_ui;
};

}  // namespace Ripes
