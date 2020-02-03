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
    void clearAssemblyEditor();

    bool isEditorEnabled() const { return m_editorEnabled; }

    const QByteArray& getBinaryData();

    void loadFile(const LoadFileParams&);

signals:
    void programChanged(const Program* program);
    void editorStateChanged(bool enabled);

public slots:
    void emitProgramChanged();

    /**
     * @brief enableAssemblyInput
     * Called whenever the user wants to switch from binary/ELF file input to typed assembly input
     */
    void enableAssemblyInput();

private slots:
    void assemble();
    void on_disassembledViewButton_toggled();

private:
    bool loadFlatBinaryFile(Program& program, QFile& file, uint32_t entryPoint, uint32_t loadAt);
    bool loadAssemblyFile(Program& program, QFile& file);
    bool loadElfFile(Program& program, QFile& file);

    void setupActions();
    void enableEditor();
    void disableEditor();

    Ui::EditTab* m_ui;
    Assembler* m_assembler = nullptr;

    LoadFileParams m_loadedFile;
    Program m_activeProgram;

    bool m_editorEnabled = true;
};
}  // namespace Ripes
