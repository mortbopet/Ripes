#include "programviewer.h"

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QFontMetricsF>
#include <QMenu>
#include <QTextBlock>

#include "colors.h"
#include "fonts.h"
#include "ripessettings.h"

namespace Ripes {

ProgramViewer::ProgramViewer(QWidget* parent) : HighlightableTextEdit(parent) {
    m_breakpointArea = new BreakpointArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged, this, &ProgramViewer::updateSidebarWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &ProgramViewer::updateSidebar);
    updateSidebarWidth(0);

    // Set font for the entire widget. calls to fontMetrics() will get the
    // dimensions of the currently set font
    m_font = QFont(Fonts::monospace, 11);
    setFont(m_font);
    m_fontTimer.setSingleShot(true);

    setTabStopDistance(QFontMetricsF(m_font).width(' ') * 4);

    setLineWrapMode(QPlainTextEdit::NoWrap);
}

void ProgramViewer::clearBreakpoints() {
    ProcessorHandler::clearBreakpoints();
}

void ProgramViewer::resizeEvent(QResizeEvent* e) {
    QPlainTextEdit::resizeEvent(e);

    const QRect cr = contentsRect();
    m_breakpointArea->setGeometry(cr.left(), cr.top(), m_breakpointArea->width(), cr.height());

    // we need to update the highlighted lines whenever resizing the window to recalculate the highlighting gradient,
    // reflecting the new widget size
    updateHighlightedAddresses();
}

void ProgramViewer::updateProgram(bool binary) {
    m_labelAddrOffsetMap.clear();
    const QString text = binary ? Assembler::binobjdump(ProcessorHandler::getProgram(), m_labelAddrOffsetMap)
                                : Assembler::objdump(ProcessorHandler::getProgram(), m_labelAddrOffsetMap);

    clearBlockHighlights();
    setPlainText(text);
    updateHighlightedAddresses();
}

void ProgramViewer::updateSidebar(const QRect& rect, int /*dy*/) {
    m_breakpointArea->update(0, rect.y(), m_breakpointArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateSidebarWidth(0);
}

void ProgramViewer::updateSidebarWidth(int /* newBlockCount */) {
    // Set margins of the text edit area
    m_sidebarWidth = m_breakpointArea->width();
    setViewportMargins(m_sidebarWidth, 0, 0, 0);
}

void ProgramViewer::setCenterAddress(const AInt address) {
    auto block = blockForAddress(address);
    if (block.isValid()) {
        auto newCursor = QTextCursor(block);
        if (newCursor.blockNumber() != 0) {
            setTextCursor(newCursor);
            ensureCursorVisible();
        }
    }
}

void ProgramViewer::updateCenterAddressFromProcessor() {
    const auto stageInfo = ProcessorHandler::getProcessor()->stageInfo(0);
    setCenterAddress(stageInfo.pc);
}

void ProgramViewer::setFollowEnabled(bool enabled) {
    RipesSettings::setValue(RIPES_SETTING_FOLLOW_EXEC, enabled);
    m_following = enabled;

    if (enabled) {
        updateCenterAddressFromProcessor();
    }
}

void ProgramViewer::updateHighlightedAddresses() {
    clearBlockHighlights();
    const unsigned stages = ProcessorHandler::getProcessor()->stageCount();
    auto colorGenerator = Colors::incrementalRedGenerator(stages);
    QList<QTextEdit::ExtraSelection> highlights;

    for (unsigned sid = 0; sid < stages; sid++) {
        const auto stageInfo = ProcessorHandler::getProcessor()->stageInfo(sid);
        if (stageInfo.stage_valid) {
            auto block = blockForAddress(stageInfo.pc);
            if (!block.isValid())
                continue;

            // Record the stage name for the highlighted block for later painting
            QString stageString = ProcessorHandler::getProcessor()->stageName(sid);
            if (!stageInfo.namedState.isEmpty())
                stageString += " (" + stageInfo.namedState + ")";
            highlightBlock(block, colorGenerator(), stageString);
        }
    }

    if (m_following) {
        updateCenterAddressFromProcessor();
    }
}

void ProgramViewer::breakpointAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(m_breakpointArea);

    // When caret flashes in QPlainTextEdit a paint event is sent to this widget,
    // with a height of a line in the edit. We override this paint event by always
    // redrawing the visible breakpoint area
    auto area = m_breakpointArea->rect();
    QLinearGradient gradient = QLinearGradient(area.topLeft(), area.bottomRight());
    gradient.setColorAt(0, Colors::FoundersRock.lighter(120));
    gradient.setColorAt(1, Colors::FoundersRock);

    painter.fillRect(area, gradient);

    QTextBlock block = firstVisibleBlock();
    if (block.isValid()) {
        int top, bottom;
        top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
        bottom = top + static_cast<int>(blockBoundingRect(block).height());

        bool ok;
        while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                const AInt address = addressForBlock(block, ok);
                if (ok) {
                    if (ProcessorHandler::hasBreakpoint(address)) {
                        painter.drawPixmap(m_breakpointArea->padding, top, m_breakpointArea->imageWidth,
                                           m_breakpointArea->imageHeight, m_breakpointArea->m_breakpoint);
                    }
                }
            }

            block = block.next();
            top = bottom;
            bottom = top + static_cast<int>(blockBoundingRect(block).height());
        }
    }
}

QTextBlock ProgramViewer::blockForAddress(AInt addr) const {
    uint64_t adjustedLineNumber = 0;
    auto m_program = ProcessorHandler::getProgram();
    if (m_program) {
        auto& disassembleRes = m_program->getDisassembled();
        if (auto index = disassembleRes.addressToIndex(addr); index.has_value())
            adjustedLineNumber = index.value();
    }

    if (m_labelAddrOffsetMap.empty()) {
        return document()->findBlockByNumber(adjustedLineNumber);
    }

    uint64_t lineNumber = adjustedLineNumber;
    auto low = m_labelAddrOffsetMap.lower_bound(lineNumber);
    if (low == m_labelAddrOffsetMap.begin() && lineNumber < low->first) {
        // The line number is less that the position of the first offset block; block is directly inferred from
        // linenumber.
        return document()->findBlockByNumber(adjustedLineNumber);
    }

    auto high = low;
    if (low != m_labelAddrOffsetMap.begin()) {
        low = std::prev(m_labelAddrOffsetMap.lower_bound(lineNumber));
    }
    high = std::next(low);

    auto validBlockRange = [&] {
        if (m_labelAddrOffsetMap.count(lineNumber))
            return false;

        bool valid = true;
        valid &= (low->first - low->second.first) <= adjustedLineNumber;
        if (high != m_labelAddrOffsetMap.end()) {
            valid &= adjustedLineNumber < (high->first - high->second.first);
        }
        return valid;
    };

    // Adjust low and high iterators to locate the range bounds of the address
    while (!validBlockRange()) {
        low = m_labelAddrOffsetMap.lower_bound(lineNumber);
        high = std::next(low);
        lineNumber = low->first + 1;
    }

    const int offsetSum = high == m_labelAddrOffsetMap.end() ? low->second.first + 1 : high->second.first;
    lineNumber = offsetSum + adjustedLineNumber;

    return document()->findBlockByNumber(lineNumber);
}

AInt ProgramViewer::addressForBlock(QTextBlock block, bool& ok) const {
    ok = true;
    const static auto calcAddressFunc = [](int lineNumber) {
        auto m_program = ProcessorHandler::getProgram();
        if (m_program) {
            auto& disassembleRes = m_program->getDisassembled();
            if (disassembleRes.numInstructions() < static_cast<unsigned>(lineNumber))
                return 0;
            if (auto addr = disassembleRes.indexToAddress(lineNumber); addr.has_value())
                return static_cast<int>(addr.value());
        }
        return 0;
    };

    const int lineNumber = block.blockNumber();

    // Clicking an invalid line? (non-instruction line)
    if (m_labelAddrOffsetMap.count(lineNumber)) {
        ok = false;
        return 0;
    }

    // To identify the program address corresponding to the selected line, we find the lower bound of the selected block
    // in the m_labelAddrOffsetMap and subtract the invalid line count up to the given point.
    int adjustedLineNumber = lineNumber;

    if (m_labelAddrOffsetMap.empty())
        return calcAddressFunc(adjustedLineNumber);

    auto low = m_labelAddrOffsetMap.lower_bound(lineNumber);

    if ((low == m_labelAddrOffsetMap.begin()) && static_cast<unsigned>(lineNumber) < low->first) {
        // The line number is less that the position of the first offset block; address is directly inferred from
        // linenumber.
        return calcAddressFunc(adjustedLineNumber);
    }

    if (low != m_labelAddrOffsetMap.begin()) {
        low = std::prev(m_labelAddrOffsetMap.lower_bound(lineNumber));
    }

    adjustedLineNumber -= (low->second.first + 1);
    return calcAddressFunc(adjustedLineNumber);
}

AInt ProgramViewer::addressForPos(const QPoint& pos, bool& ok) const {
    ok = true;
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
    if (!block.isValid()) {
        ok = false;
        return 0;
    }

    return addressForBlock(block, ok);
}

bool ProgramViewer::hasBreakpoint(const QPoint& pos) const {
    bool ok;
    AInt address = addressForPos(pos, ok);
    if (ok) {
        return ProcessorHandler::hasBreakpoint(address);
    } else {
        return false;
    }
}

void ProgramViewer::breakpointClick(const QPoint& pos) {
    bool ok;
    const auto address = addressForPos(pos, ok);
    if (ok) {
        ProcessorHandler::toggleBreakpoint(static_cast<unsigned>(address));
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

    connect(toggleAction, &QAction::triggered, m_programViewer,
            [=] { m_programViewer->breakpointClick(event->pos()); });
    connect(removeAllAction, &QAction::triggered, m_programViewer, [=] {
        m_programViewer->clearBreakpoints();
        repaint();
    });

    contextMenu.exec(event->globalPos());
}

}  // namespace Ripes
