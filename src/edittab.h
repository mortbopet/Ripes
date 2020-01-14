#pragma once

#include <QByteArray>
#include <QFile>
#include <QWidget>
#include <map>

#include "assembler.h"
#include "program.h"
#include "ripestab.h"

namespace Ripes {

namespace Ui {
class EditTab;
}

class Assembler;
class LoadFileParams;

class EditTab : public RipesTab {
    Q_OBJECT

public:
    EditTab(QToolBar* toolbar, QWidget* parent = nullptr);
    ~EditTab() override;

    void setAssemblyText(const QString& text);
    void setDisassemblerText();
    QString getAssemblyText();
    void newProgram();
    void clear();

    const QByteArray& getBinaryData();

    void loadFile(const LoadFileParams&);

signals:
    void programChanged(const Program& program);

public slots:
    void emitProgramChanged();

private slots:
    void assemble();
    void on_disassembledViewButton_toggled();

private:
    void loadFlatBinaryFile(const LoadFileParams&);
    void loadAssemblyFile(const LoadFileParams&);
    void loadElfFile(const LoadFileParams&);

    void setupActions();
    void enableEditor();
    void disableEditor();

    Ui::EditTab* m_ui;
    Assembler* m_assembler = nullptr;
};
}  // namespace Ripes
