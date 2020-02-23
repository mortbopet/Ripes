#pragma once

#include <QFont>
#include <QObject>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTimer>

#include "parser.h"
#include "processorhandler.h"
#include "program.h"

namespace Ripes {

class BreakpointArea;

class ProgramViewer : public QPlainTextEdit {
    Q_OBJECT
public:
    ProgramViewer(QWidget* parent = nullptr);
    void paintEvent(QPaintEvent* e) override;

    void breakpointAreaPaintEvent(QPaintEvent* event);
    void breakpointClick(const QPoint& pos);
    bool hasBreakpoint(const QPoint& pos) const;
    void clearBreakpoints();

    long addressForPos(const QPoint& pos) const;
    long addressForBlock(QTextBlock block) const;
    QTextBlock blockForAddress(unsigned long) const;

    ///
    /// \brief updateProgram
    /// Refreshes the programViewer view with @p program, using the parser to either disassemble or (if @p binary is
    /// true) show the raw binary version of the loaded program.
    ///
    void updateProgram(const Program& program, bool binary = false);

public slots:
    void updateHighlightedAddresses();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateSidebar(const QRect&, int);
    void updateSidebarWidth(int newBlockCount);

private:
    // A timer is needed for only catching one of the multiple wheel events that
    // occur on a regular mouse scroll
    QTimer m_fontTimer;
    QTimer m_changeTimer;

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
    AddrOffsetMap m_labelAddrOffsetMap;
    std::map<QTextBlock, QStringList> m_highlightedBlocksText;
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
