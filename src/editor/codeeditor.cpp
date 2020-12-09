#include "codeeditor.h"
#include "defines.h"

#include <QAction>
#include <QApplication>
#include <QLinearGradient>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QTextBlock>
#include <QTimer>
#include <QToolTip>
#include <QWheelEvent>

#include <iterator>

#include "syntaxhighlighter.h"

#include "csyntaxhighlighter.h"
#include "processorhandler.h"
#include "rvsyntaxhighlighter.h"

namespace Ripes {

CodeEditor::CodeEditor(QWidget* parent) : QPlainTextEdit(parent) {
    m_lineNumberArea = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateSidebarWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateSidebar);
    updateSidebarWidth(0);

    // Set font for the entire widget. calls to fontMetrics() will get the
    // dimensions of the currently set font
    m_font = QFont("Inconsolata", 11);
    setFont(m_font);
    m_fontTimer.setSingleShot(true);
    setTabStopDistance(QFontMetricsF(m_font).width(' ') * 4);

    // set event filter for catching scroll events
    installEventFilter(this);

    // needed for instant tooltip displaying
    setMouseTracking(true);

    setWordWrapMode(QTextOption::NoWrap);
    setupChangedTimer();
}

void CodeEditor::setupChangedTimer() {
    m_changeTimer = new QTimer(this);
    // configures the change-timer and assembler connectivity with Parser
    m_changeTimer->setInterval(500);
    m_changeTimer->setSingleShot(true);

    // A change in the document will start the timer - when the timer elapses, the contents will be assembled if there
    // is no syntax error. By doing this, the timer is restartet each time a change occurs (ie. a user is continuously
    // typing).
    // Note, QTextDocument::contentsChange seems to be the only signal in the stack of QPlainTextEdit etc. which
    // (contrary to documentation) does not emit a change signal when formatting is applied. This is required to avoid a
    // signal loop in the case of:
    // (content change -> assemble -> assembling fails -> rehighlight to reflect assembler error -> content change)
    connect(this->document(), &QTextDocument::contentsChange, [this](int, int, int) { m_changeTimer->start(); });

    connect(m_changeTimer, &QTimer::timeout, this, &CodeEditor::timedTextChanged);
}

int CodeEditor::lineNumberAreaWidth() {
    int digits = 1;
    int rightPadding = 6;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = rightPadding + fontMetrics().width(QString("1")) * digits;
    return space;
}

void CodeEditor::updateSidebarWidth(int /* newBlockCount */) {
    // Set margins of the text edit area
    m_sidebarWidth = lineNumberAreaWidth();
    setViewportMargins(m_sidebarWidth, 0, 0, 0);
}

bool CodeEditor::eventFilter(QObject* /*observed*/, QEvent* event) {
    // Event filter for catching ctrl+Scroll events, for text resizing
    if (event->type() == QEvent::Wheel && QApplication::keyboardModifiers() == Qt::ControlModifier) {
        auto wheelEvent = static_cast<QWheelEvent*>(event);
        // Since multiple wheelevents are issued on a scroll,
        // start a timer to only catch the first one

        // change font size
        if (!m_fontTimer.isActive()) {
            if (wheelEvent->angleDelta().y() > 0) {
                if (m_font.pointSize() < 30)
                    m_font.setPointSize(m_font.pointSize() + 1);
            } else {
                if (m_font.pointSize() > 6)
                    m_font.setPointSize(m_font.pointSize() - 1);
            }
            m_fontTimer.start(50);
        }
        setFont(m_font);
        return true;
    }

    return false;
}

void CodeEditor::rehighlight() {
    if (m_highlighter) {
        m_highlighter->rehighlight();
    }
}

bool CodeEditor::event(QEvent* event) {
    // Override event handler for receiving tool tips
    if (event->type() == QEvent::ToolTip) {
        // Tooltips are updated through slot handler updateTooltip
        auto* helpEvent = static_cast<QHelpEvent*>(event);
        QTextCursor textAtCursor = cursorForPosition(helpEvent->pos());
        const int row = textAtCursor.block().firstLineNumber();

        if (m_errors && m_errors->toMap().count(row) != 0) {
            QToolTip::showText(helpEvent->globalPos(), m_errors->toMap().at(row));
        } else {
            QToolTip::hideText();
            event->ignore();
        }

        return true;
    }
    return QPlainTextEdit::event(event);
}

void CodeEditor::updateSidebar(const QRect& rect, int dy) {
    if (dy) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect()))
        updateSidebarWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent* e) {
    QPlainTextEdit::resizeEvent(e);

    const QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::setSourceType(SourceType type, const std::set<QString>& supportedOpcodes) {
    m_sourceType = type;

    // Creates AsmHighlighter object and connects it to the current document
    switch (m_sourceType) {
        case SourceType::Assembly: {
            auto* isa = ProcessorHandler::get()->currentISA();
            if (isa->isaID() == ISA::RV32I) {
                m_highlighter = std::make_unique<RVSyntaxHighlighter>(document(), m_errors, supportedOpcodes);
            } else {
                Q_ASSERT(false && "Unknown ISA selected");
            }
            break;
        }
        case SourceType::C:
            m_highlighter = std::make_unique<CSyntaxHighlighter>(document(), m_errors);
            break;
        default:
            break;
    }

    m_highlighter->rehighlight();
}

void CodeEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Colors::Medalist).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor(Qt::lightGray).lighter(120));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(Qt::gray).darker(130));
            painter.drawText(0, top, m_lineNumberArea->width() - 3, fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

}  // namespace Ripes
