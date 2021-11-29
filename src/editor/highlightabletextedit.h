#pragma once

#include <QFont>
#include <QObject>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTimer>

#include <set>

namespace Ripes {

// This implements a QPlainTextEdit with support for highlighting certain text blocks within the document.

class HighlightableTextEdit : public QPlainTextEdit {
    Q_OBJECT

    struct BlockHighlight {
        // Store the color which the block is highlighted with so that we can rehighlight a selection.
        QColor color;
        QTextEdit::ExtraSelection selection;
    };

public:
    HighlightableTextEdit(QWidget* parent = nullptr);
    void paintEvent(QPaintEvent* e) override;
    /// Adds a highlight to the given block. An optional string may be provided, which will be painted at the right-hand
    /// side of each block.
    void highlightBlock(const QTextBlock& block, const QColor& color, const QString& text = QString());
    /// Clears any currently active block highlightings.
    void clearBlockHighlights();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setBlockGradient(BlockHighlight& highlight);
    void applyHighlighting();

    /// A list of strings which will be printed at the right-hand side of each block
    std::map<QTextBlock, QStringList> m_highlightedBlocksText;
    /// A set containing the currently highlighted blocks
    std::set<QTextBlock> m_highlightedBlocks;
    /// A list containing the current highlightings being applied
    QList<BlockHighlight> m_blockHighlights;
};

}  // namespace Ripes
