#pragma once

#include <QByteArray>
#include <QFile>
#include <QWidget>
#include <map>
#include <memory>

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
    QString getAssemblyText();
    void newProgram();
    void clearAssemblyEditor();

    bool isEditorEnabled() const { return m_editorEnabled; }

    const QByteArray& getBinaryData();

    void loadFile(const LoadFileParams&);
    /**
     * @brief loadExternalFile
     * sets the current source type to whatever is specified by @p params and calls loadFile(@p params);
     */
    void loadExternalFile(const LoadFileParams& params);

signals:
    void programChanged(const Program* program);
    void editorStateChanged(bool enabled);

public slots:
    void updateProgramViewerHighlighting();

    void emitProgramChanged();

    /**
     * @brief sourceTypeChanged
     * Called whenever the user requested to change the current input type (Assembly or C)
     */
    void sourceTypeChanged();

    /**
     * @brief enableAssemblyInput
     * Called whenever the user wants to switch from binary/ELF file input to typed assembly input
     */
    void enableAssemblyInput();

private slots:
    void sourceCodeChanged();
    void on_disassembledViewButton_toggled();

private:
    void assemble();
    void compile();

    void updateProgramViewer();
    bool loadFlatBinaryFile(Program& program, QFile& file, unsigned long entryPoint, unsigned long loadAt);
    bool loadAssemblyFile(Program& program, QFile& file);
    bool loadElfFile(Program& program, QFile& file);

    void setupActions();
    void enableEditor();
    void disableEditor();

    Ui::EditTab* m_ui = nullptr;
    std::unique_ptr<Assembler> m_assembler;

    Program m_activeProgram;

    SourceType m_currentSourceType;

    bool m_editorEnabled = true;
};
}  // namespace Ripes
