#include "programviewer.h"

#include "defines.h"
#include "processorhandler.h"

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QFontMetricsF>
#include <QMenu>
#include <QTextBlock>

namespace Ripes {

ProgramViewer::ProgramViewer(QWidget* parent) : QPlainTextEdit(parent) {
    m_breakpointArea = new BreakpointArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged, this, &ProgramViewer::updateSidebarWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &ProgramViewer::updateSidebar);
    updateSidebarWidth(0);

    // Set font for the entire widget. calls to fontMetrics() will get the
    // dimensions of the currently set font
    m_font = QFont("Inconsolata", 11);
    setFont(m_font);
    m_fontTimer.setSingleShot(true);

    setTabStopDistance(QFontMetricsF(m_font).width(' ') * 4);
}

bool ProgramViewer::eventFilter(QObject*, QEvent* event) {
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

void ProgramViewer::clearBreakpoints() {
    ProcessorHandler::get()->clearBreakpoints();
}

void ProgramViewer::resizeEvent(QResizeEvent* e) {
    QPlainTextEdit::resizeEvent(e);

    const QRect cr = contentsRect();
    m_breakpointArea->setGeometry(cr.left(), cr.top(), m_breakpointArea->width(), cr.height());
}

void ProgramViewer::updateProgram(const Program& program, bool binary) {
    m_labelAddrOffsetMap.clear();
    const auto text = binary ? Parser::getParser()->binarize(program, m_labelAddrOffsetMap)
                             : Parser::getParser()->disassemble(program, m_labelAddrOffsetMap);
    setPlainText(text);
}

void ProgramViewer::updateSidebar(const QRect& rect, int dy) {
    m_breakpointArea->update(0, rect.y(), m_breakpointArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateSidebarWidth(0);
}

void ProgramViewer::updateSidebarWidth(int /* newBlockCount */) {
    // Set margins of the text edit area
    m_sidebarWidth = m_breakpointArea->width();
    setViewportMargins(m_sidebarWidth, 0, 0, 0);
}

void ProgramViewer::breakpointAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(m_breakpointArea);

    // When caret flashes in QPlainTextEdit a paint event is sent to this widget,
    // with a height of a line in the edit. We override this paint event by always
    // redrawing the visible breakpoint area
    auto area = m_breakpointArea->rect();
    QLinearGradient gradient = QLinearGradient(area.topLeft(), area.bottomRight());
    gradient.setColorAt(0, QColor(Colors::FoundersRock).lighter(120));
    gradient.setColorAt(1, QColor(Colors::FoundersRock));

    painter.fillRect(area, gradient);

    QTextBlock block = firstVisibleBlock();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        volatile int line = block.blockNumber();
        const bool invalidLine = m_labelAddrOffsetMap.count(block.blockNumber());
        if (block.isVisible() && bottom >= event->rect().top() && !invalidLine) {
            int adjustedLineNumber = block.blockNumber();
            auto offsetsToLine = m_labelAddrOffsetMap.lower_bound(adjustedLineNumber);
            if (offsetsToLine != m_labelAddrOffsetMap.end()) {
                adjustedLineNumber -= offsetsToLine->second;
            }
            const unsigned long address = ProcessorHandler::get()->getTextStart() +
                                          adjustedLineNumber * ProcessorHandler::get()->currentISA()->bytes();
            if (ProcessorHandler::get()->hasBreakpoint(address)) {
                painter.drawPixmap(m_breakpointArea->padding, top, m_breakpointArea->imageWidth,
                                   m_breakpointArea->imageHeight, m_breakpointArea->m_breakpoint);
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
    }
}

long ProgramViewer::addressForPos(const QPoint& pos) const {
    // Get line height
    QTextBlock block = firstVisibleBlock();

    const auto height = blockBoundingRect(block).height();

    // Find block index in the ProgramViewer
    int index;
    if (block == document()->findBlockByLineNumber(0)) {
        index = static_cast<int>((pos.y() - contentOffset().y()) / height);
    } else {
        index = static_cast<int>((pos.y() + contentOffset().y()) / height);
    }
    // Get actual block index
    while (index > 0) {
        block = block.next();
        index--;
    }
    if (!block.isValid())
        return -1;

    const int lineNumber = block.blockNumber();

    // Clicking an invalid line? (non-instruction line)
    if (m_labelAddrOffsetMap.count(lineNumber)) {
        return -1;
    }

    // To identify the program address corresponding to the selected line, we find the lower bound of the selected block
    // in the m_labelAddrOffsetMap and subtract the invalid line count up to the given point.
    int adjustedLineNumber = lineNumber;
    auto offsetsToLine = m_labelAddrOffsetMap.lower_bound(lineNumber);
    if (offsetsToLine != m_labelAddrOffsetMap.end()) {
        adjustedLineNumber -= offsetsToLine->second;
    }

    // Toggle breakpoint
    return adjustedLineNumber * ProcessorHandler::get()->currentISA()->bytes() +
           ProcessorHandler::get()->getTextStart();
}

bool ProgramViewer::hasBreakpoint(const QPoint& pos) const {
    return ProcessorHandler::get()->hasBreakpoint(static_cast<unsigned>(addressForPos(pos)));
}

void ProgramViewer::breakpointClick(const QPoint& pos) {
    const auto address = addressForPos(pos);
    if (!(address < 0)) {
        ProcessorHandler::get()->toggleBreakpoint(static_cast<unsigned>(address));
        repaint();
    }
}

// -------------- breakpoint area ----------------------------------

BreakpointArea::BreakpointArea(ProgramViewer* viewer) : QWidget(viewer) {
    m_programViewer = viewer;
    setCursor(Qt::PointingHandCursor);
}

void BreakpointArea::contextMenuEvent(QContextMenuEvent* event) {
    // setup context menu
    QMenu contextMenu;

    // Create and connect actions for removing and setting breakpoints
    auto* toggleAction = contextMenu.addAction("Toggle breakpoint");
    auto* removeAllAction = contextMenu.addAction("Remove all breakpoints");

    connect(toggleAction, &QAction::triggered, [=] { m_programViewer->breakpointClick(event->pos()); });
    connect(removeAllAction, &QAction::triggered, [=] {
        m_programViewer->clearBreakpoints();
        repaint();
    });

    contextMenu.exec(event->globalPos());
}

}  // namespace Ripes
