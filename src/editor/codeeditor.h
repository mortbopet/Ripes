#pragma once

#include <QApplication>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTimer>

#include "assembler/assembler.h"
#include "assembler/program.h"

#include "highlightabletextedit.h"
#include "syntaxhighlighter.h"

#include <memory>
#include <set>

// Extended version of Qt's CodeEditor example
// http://doc.qt.io/qt-5/qtwidgets-widgets-codeeditor-example.html

namespace Ripes {

class LineNumberArea;

class CodeEditor : public HighlightableTextEdit {
    Q_OBJECT
public:
    CodeEditor(QWidget* parent = nullptr);

    void setSourceType(SourceType type, const std::set<QString>& supportedOpcodes);
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();
    void setupChangedTimer();
    void rehighlight();
    void onSave();

    void setErrors(const std::shared_ptr<Assembler::Errors>& errors) { m_errors = errors; }

signals:
    /**
     * @brief timedTextChanged
     * Using m_changeTimer, timedTextChanged will be emitted whenever the user has stopped editing the text document for
     * some time.
     * The signal may be used as an alternative to QPlainTextEdit::textChanged(), to reduce the amount of updates
     * performed wrt. text changes.
     */
    void timedTextChanged();

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* e) override;
    void updateHighlighting();

private slots:
    void updateSidebarWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateSidebar(const QRect&, int);

private:
    std::unique_ptr<SyntaxHighlighter> m_highlighter;

    LineNumberArea* m_lineNumberArea;
    int m_sidebarWidth;

    bool m_breakpointAreaEnabled = false;
    SourceType m_sourceType = SourceType::Assembly;
    std::shared_ptr<Assembler::Errors> m_errors;

    QFont m_font;

    // A timer is needed for only catching one of the multiple wheel events that
    // occur on a regular mouse scroll
    QTimer m_fontTimer;
    QTimer* m_changeTimer;

    bool eventFilter(QObject* observed, QEvent* event) override;

    /**
     * @brief keyPressEvent
     * We override the key press event to provide any editor quality of life changes that may apply.
     */
    void keyPressEvent(QKeyEvent* e) override;
};

class LineNumberArea : public QWidget {
public:
    LineNumberArea(CodeEditor* editor) : QWidget(editor) { codeEditor = editor; }

    QSize sizeHint() const override { return QSize(codeEditor->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent* event) override { codeEditor->lineNumberAreaPaintEvent(event); }

    void wheelEvent(QWheelEvent* event) override {
        codeEditor->verticalScrollBar()->setValue(codeEditor->verticalScrollBar()->value() +
                                                  (-event->angleDelta().y()) / 30);
    }

private:
    CodeEditor* codeEditor;
};

}  // namespace Ripes
