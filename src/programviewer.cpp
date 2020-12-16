#include "programviewer.h"

#include "defines.h"
#include "ripessettings.h"

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

    setLineWrapMode(QPlainTextEdit::NoWrap);
}

void ProgramViewer::clearBreakpoints() {
    ProcessorHandler::get()->clearBreakpoints();
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
    const QString text = binary ? Assembler::binobjdump(ProcessorHandler::get()->getProgram(), m_labelAddrOffsetMap)
                                : Assembler::objdump(ProcessorHandler::get()->getProgram(), m_labelAddrOffsetMap);

    // A memory occurs within QPlainTextEdit::clear if extra selections has been set. This is most possibly a bug,
    // but seems to be fixed if we manually clear the selections before we clear (and add new text) to the text
    // edit.
    setExtraSelections({});

    setPlainText(text);
    updateHighlightedAddresses();
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

void ProgramViewer::setCenterAddress(const long address) {
    auto block = blockForAddress(address);
    setTextCursor(QTextCursor(block));
    ensureCursorVisible();
}

void ProgramViewer::updateCenterAddressFromProcessor() {
    const auto stageInfo = ProcessorHandler::get()->getProcessor()->stageInfo(0);
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
    const unsigned stages = ProcessorHandler::get()->getProcessor()->stageCount();
    QColor bg = QColor(Qt::red).lighter(120);
    const int decRatio = 100 + 80 / stages;
    QList<QTextEdit::ExtraSelection> highlights;
    std::set<unsigned long> highlightedPCs;
    m_highlightedBlocksText.clear();

    for (unsigned sid = 0; sid < stages; sid++) {
        const auto stageInfo = ProcessorHandler::get()->getProcessor()->stageInfo(sid);
        if (stageInfo.stage_valid) {
            auto block = blockForAddress(stageInfo.pc);
            if (!block.isValid())
                continue;

            // Record the stage name for the highlighted block for later painting
            m_highlightedBlocksText[block] << ProcessorHandler::get()->getProcessor()->stageName(sid);

            // If a stage has already been highlighted (ie. an instruction exists in more than 1 stage at once), keep
            // the already set highlighting.
            if (highlightedPCs.count(stageInfo.pc)) {
                continue;
            }

            QTextEdit::ExtraSelection es;
            es.cursor = QTextCursor(block);
            es.format.setProperty(QTextFormat::FullWidthSelection, true);

            const auto bbr = blockBoundingRect(block);
            QLinearGradient grad(bbr.topLeft(), bbr.bottomRight());
            grad.setColorAt(0, palette().base().color());
            grad.setColorAt(1, bg);
            es.format.setBackground(grad);

            highlights << es;
            highlightedPCs.insert(stageInfo.pc);
        }
        bg = bg.lighter(decRatio);
    }
    setExtraSelections(highlights);

    if (m_following) {
        updateCenterAddressFromProcessor();
    }
}

void ProgramViewer::paintEvent(QPaintEvent* event) {
    QPlainTextEdit::paintEvent(event);

    // Draw stage names for highlighted addresses
    QPainter painter(viewport());

    for (const auto& hb : m_highlightedBlocksText) {
        const QString stageString = hb.second.join('/');
        const auto bbr = blockBoundingGeometry(hb.first);
        painter.setFont(font());
        painter.drawText(bbr.width() - painter.fontMetrics().boundingRect(stageString).width() - /*padding*/ 10,
                         bbr.top() + bbr.height(), stageString);
    }
    painter.end();
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
        if (block.isVisible() && bottom >= event->rect().top()) {
            const long address = addressForBlock(block);
            if (address >= 0) {
                if (ProcessorHandler::get()->hasBreakpoint(address)) {
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

namespace {}

QTextBlock ProgramViewer::blockForAddress(unsigned long addr) const {
    const long adjustedLineNumber =
        (addr - ProcessorHandler::get()->getTextStart()) / ProcessorHandler::get()->currentISA()->bytes();

    if (m_labelAddrOffsetMap.empty()) {
        return document()->findBlockByNumber(adjustedLineNumber);
    }

    long lineNumber = adjustedLineNumber;
    auto low = m_labelAddrOffsetMap.lower_bound(lineNumber);

    if (lineNumber < low->first && (low == m_labelAddrOffsetMap.begin())) {
        // The line number is less that the position of the first offset block; block is directly inferred from
        // linenumber.
        return document()->findBlockByNumber(adjustedLineNumber);
    }

    auto high = low;
    if (low != m_labelAddrOffsetMap.begin()) {
        low = std::prev(m_labelAddrOffsetMap.lower_bound(lineNumber));
    }
    high = std::next(low);

    auto validBlockRange = [this, adjustedLineNumber](auto& low, auto& high, long lineNumber) {
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
    while (!validBlockRange(low, high, lineNumber)) {
        low = m_labelAddrOffsetMap.lower_bound(lineNumber);
        high = std::next(low);
        lineNumber = low->first + 1;
    }

    const int offsetSum = high == m_labelAddrOffsetMap.end() ? low->second.first + 1 : high->second.first;
    lineNumber = offsetSum + adjustedLineNumber;

    return document()->findBlockByNumber(lineNumber);
}

long ProgramViewer::addressForBlock(QTextBlock block) const {
    const static auto calcAddressFunc = [](int lineNumber) {
        return lineNumber * ProcessorHandler::get()->currentISA()->bytes() + ProcessorHandler::get()->getTextStart();
    };

    const int lineNumber = block.blockNumber();

    // Clicking an invalid line? (non-instruction line)
    if (m_labelAddrOffsetMap.count(lineNumber)) {
        return -1;
    }

    // To identify the program address corresponding to the selected line, we find the lower bound of the selected block
    // in the m_labelAddrOffsetMap and subtract the invalid line count up to the given point.
    int adjustedLineNumber = lineNumber;

    if (m_labelAddrOffsetMap.empty())
        return calcAddressFunc(adjustedLineNumber);

    auto low = m_labelAddrOffsetMap.lower_bound(lineNumber);

    if ((low == m_labelAddrOffsetMap.begin()) && lineNumber < low->first) {
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

    return addressForBlock(block);
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
