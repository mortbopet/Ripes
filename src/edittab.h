#pragma once

#include <QByteArray>
#include <QFile>
#include <QWidget>
#include <map>
#include <memory>

#include "assembler/assembler.h"
#include "assembler/program.h"
#include "ripestab.h"

namespace Ripes {

namespace Ui {
class EditTab;
}

struct LoadFileParams;

class EditTab : public RipesTab {
    Q_OBJECT

public:
    EditTab(QToolBar* toolbar, QWidget* parent = nullptr);
    ~EditTab() override;

    void setSourceText(const QString& text);
    QString getAssemblyText();
    void newProgram();
    void clearAssemblyEditor();
    SourceType getSourceType() { return m_currentSourceType; }
    bool isEditorEnabled() const { return m_editorEnabled; }

    const QByteArray* getBinaryData();

    /// Loads a file into the editor. Returns true if the file was successfully loaded.
    bool loadFile(const LoadFileParams&);
    void loadSourceText(const QString& text);
    Assembler::Errors* errors();

    /// sets the current source type to whatever is specified by @p params and calls loadFile(@p params). Returns true
    /// if the file loaded successfully.
    bool loadExternalFile(const LoadFileParams& params);

signals:
    void programChanged(const std::shared_ptr<Program>& program);
    void editorStateChanged(bool enabled);

public slots:
    void onSave();
    void onProcessorChanged();
    void updateProgramViewerHighlighting();

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

    void showSymbolNavigator();
    void sourceCodeChanged();
    void on_disassembledViewButton_toggled();

private:
    void assemble();
    void compile();

    void updateProgramViewer();
    bool loadFlatBinaryFile(Program& program, QFile& file, unsigned long entryPoint, unsigned long loadAt);
    bool loadSourceFile(Program& program, QFile& file);
    bool loadElfFile(Program& program, QFile& file);

    void setupActions();
    void enableEditor();
    void disableEditor();

    QAction* m_buildAction = nullptr;
    QAction* m_followAction = nullptr;
    QAction* m_symbolNavigatorAction = nullptr;

    Ui::EditTab* m_ui = nullptr;
    std::shared_ptr<Assembler::Errors> m_sourceErrors;

    SourceType m_currentSourceType = SourceType::Assembly;

    bool m_editorEnabled = true;
};
}  // namespace Ripes
