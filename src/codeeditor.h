#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QApplication>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTimer>

#include "assembler.h"
#include "syntaxhighlighter.h"

#include <set>

// Extended version of Qt's CodeEditor example
// http://doc.qt.io/qt-5/qtwidgets-widgets-codeeditor-example.html

class LineNumberArea;
class BreakpointArea;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    CodeEditor(QWidget* parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent* event);
    void breakpointAreaPaintEvent(QPaintEvent* event);
    void breakpointClick(QMouseEvent* event, int forceState = 0);
    void clearBreakpoints();
    void updateBreakpoints();
    int lineNumberAreaWidth();
    void setupSyntaxHighlighter();
    void setupAssembler();
    void enableBreakpointArea();
    void reset() { m_highlighter->reset(); }
    void setTimerEnabled(bool state) { m_timerEnabled = state; }
    const QByteArray& getCurrentOutputArray() { return m_assembler->getTextSegment(); }
    void clearOutputArray() {
        m_assembler->clear();
        m_tooltipForLine.clear();
    }
signals:
    void readyToTranslate();  // Emitted when syntax has been accepted, and the input text can be translated to
    void assembledSuccessfully(const QByteArray& bytearray, bool clearSimulatorMemory, uint32_t baseAddress);

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* e) override;

private slots:
    void updateSidebarWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateSidebar(const QRect&, int);
    void updateTooltip(int line, QString tip);
    void assembleCode();

private:
    Assembler* m_assembler;
    SyntaxHighlighter* m_highlighter;
    LineNumberArea* m_lineNumberArea;
    BreakpointArea* m_breakpointArea;
    int m_sidebarWidth;

    bool m_syntaxChecking = false;
    bool m_breakpointAreaEnabled = false;

    QMap<int, QString> m_tooltipForLine;

    QFont m_font = font();

    // A timer is needed for only catching one of the multiple wheel events that
    // occur on a regular mouse scroll
    QTimer m_fontTimer;
    QTimer m_changeTimer;
    bool m_timerEnabled = true;

    bool eventFilter(QObject* observed, QEvent* event) override;
};

// base class for side area widgets that are attached to the code editor
class SideArea : public QWidget {
    Q_OBJECT
public:
    SideArea(QWidget* parent = nullptr);

private:
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

class BreakpointArea : public QWidget {
public:
    BreakpointArea(CodeEditor* editor);

    QSize sizeHint() const override { return QSize(width(), 0); }
    int width() const { return imageWidth + padding * 2; }
    QSize breakpointSize() { return QSize(imageWidth, imageHeight); }

    int imageWidth = 16;
    int imageHeight = 16;
    int padding = 3;  // padding on each side of the breakpoint
    QPixmap m_breakpoint = QPixmap(":/icons/breakpoint_enabled.png").scaled(imageWidth, imageHeight);
    QPixmap m_breakpoint_disabled = QPixmap(":/icons/breakpoint_disabled.png").scaled(imageWidth, imageWidth);

    void setBlockHeight(int height) { m_blockHeight = height; }

protected:
    void paintEvent(QPaintEvent* event) override { codeEditor->breakpointAreaPaintEvent(event); }

    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    CodeEditor* codeEditor;
    int m_blockHeight = 0;

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            codeEditor->breakpointClick(event);
        }
    }

    void wheelEvent(QWheelEvent* event) override {
        codeEditor->verticalScrollBar()->setValue(codeEditor->verticalScrollBar()->value() +
                                                  (-event->angleDelta().y()) / 30);
    }

    QAction* m_removeAction;
    QAction* m_addAction;
    QAction* m_removeAllAction;

    QMouseEvent* m_event;
};

#endif  // CODEEDITOR_H
