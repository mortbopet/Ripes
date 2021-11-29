#pragma once

#include <QFont>
#include <QObject>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTimer>

#include "assembler/objdump.h"
#include "assembler/program.h"
#include "highlightabletextedit.h"
#include "processorhandler.h"

namespace Ripes {

class BreakpointArea;

class ProgramViewer : public HighlightableTextEdit {
    Q_OBJECT
public:
    ProgramViewer(QWidget* parent = nullptr);

    void breakpointAreaPaintEvent(QPaintEvent* event);
    void breakpointClick(const QPoint& pos);
    bool hasBreakpoint(const QPoint& pos) const;
    void clearBreakpoints();
    void setFollowEnabled(bool enabled);

    AInt addressForPos(const QPoint& pos, bool& ok) const;
    AInt addressForBlock(QTextBlock block, bool& ok) const;
    QTextBlock blockForAddress(AInt) const;
    void setCenterAddress(const AInt address);

    const Assembler::AddrOffsetMap& addressOffsetMap() const { return m_labelAddrOffsetMap; }

    ///
    /// \brief updateProgram
    /// Refreshes the programViewer view, using the parser to either disassemble or (if @p binary is
    /// true) show the raw binary version of the loaded program.
    ///
    void updateProgram(bool binary = false);

public slots:
    void updateHighlightedAddresses();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateSidebar(const QRect&, int);
    void updateSidebarWidth(int newBlockCount);

private:
    /**
     * @brief updateCenterAddress
     * If program following is enabled, this function will ensure that the current instruction in the first stage of the
     * processor is always visible within the program viewer.
     */
    void updateCenterAddressFromProcessor();

    // A timer is needed for only catching one of the multiple wheel events that
    // occur on a regular mouse scroll
    QTimer m_fontTimer;
    QTimer m_changeTimer;

    bool m_following = true;

    QFont m_font;
    int m_sidebarWidth;

    BreakpointArea* m_breakpointArea;

    /**
     * @brief m_labelAddrOffsetMap
     * To correctly correlate a line index with program address location whilst accounting for additional output in the
     * disassembled view of a program, we provide the AddrOffsetMap. Key in this map indicate line numbers (starting
     * from 0) which does >not< correspond with an address. The value of the key corresponds to the total sum of
     * non-address lines encountered up to and including the given label.
     */
    Assembler::AddrOffsetMap m_labelAddrOffsetMap;
};

class BreakpointArea : public QWidget {
public:
    BreakpointArea(ProgramViewer* viewer);

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
    void paintEvent(QPaintEvent* event) override { m_programViewer->breakpointAreaPaintEvent(event); }

    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    ProgramViewer* m_programViewer;
    int m_blockHeight = 0;

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            m_programViewer->breakpointClick(event->pos());
        }
    }

    void wheelEvent(QWheelEvent* event) override {
        m_programViewer->verticalScrollBar()->setValue(m_programViewer->verticalScrollBar()->value() +
                                                       (-event->angleDelta().y()) / 30);
    }
};

}  // namespace Ripes
