#include "codeeditor.h"

#include <QAction>
#include <QApplication>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QTextBlock>
#include <QTimer>
#include <QToolTip>
#include <QWheelEvent>

#include <iterator>

#include "colors.h"
#include "csyntaxhighlighter.h"
#include "fonts.h"
#include "formattermanager.h"
#include "processorhandler.h"
#include "ripessettings.h"
#include "rvsyntaxhighlighter.h"
#include "statusmanager.h"
#include "syntaxhighlighter.h"

namespace Ripes {

CodeEditor::CodeEditor(QWidget* parent) : HighlightableTextEdit(parent) {
    m_lineNumberArea = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateSidebarWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateSidebar);
    updateSidebarWidth(0);

    connect(ProcessorHandler::get(), &ProcessorHandler::procStateChangedNonRun, this, &CodeEditor::updateHighlighting);

    // Set font for the entire widget. calls to fontMetrics() will get the
    // dimensions of the currently set font
    m_font = QFont(Fonts::monospace, 11);
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

struct ClangFormatResult {
    // remapped cursor position. see clang-format --cursor for more info.
    unsigned cursor;
    QString formattedText;
};

static std::optional<ClangFormatResult> parseClangFormatOutput(const QString& cfOutput) {
    // The first line should contain a JSON formatted array detailing the remapped cursor position and formatting
    // status. If this is not the case, the clang-format version might be outdated or something else went wrong.
    unsigned firstNewline = cfOutput.indexOf('\n');
    auto firstLine = cfOutput.left(firstNewline);
    auto rest = cfOutput.mid(firstNewline + 1);
    auto jsonHeader = QJsonDocument::fromJson(firstLine.toUtf8());
    if (jsonHeader.isNull())
        return std::nullopt;

    if (!jsonHeader.object().contains("Cursor"))
        return std::nullopt;

    ClangFormatResult res;
    res.cursor = jsonHeader["Cursor"].toInt();
    res.formattedText = rest;
    return res;
}

void CodeEditor::onSave() {
    // run the formatter, if applicable.

    if (m_sourceType != SourceType::C)
        return;

    if (!RipesSettings::value(RIPES_SETTING_FORMAT_ON_SAVE).toBool())
        return;

    if (!FormatterManager::hasValidProgram())
        return;

    // We want to execute the formatter in the savefile's directory to adhere to any formatting config files.
    auto savePath = RipesSettings::value(RIPES_SETTING_SAVEPATH).toString();
    if (savePath.isEmpty())
        return;

    auto tmpDir = QFileInfo(savePath).dir();
    const auto tempFileTemplate =
        QString(tmpDir.path() + QDir::separator() + QCoreApplication::applicationName() + ".XXXXXX.c");
    QTemporaryFile tmpSrc(tempFileTemplate);
    tmpSrc.setAutoRemove(true);
    if (tmpSrc.open()) {
        tmpSrc.write(this->document()->toPlainText().toUtf8());
        tmpSrc.close();
        // Feed the current program to the formatter as stdin
        QStringList clangFormatArgs;
        // Track cursor changes
        clangFormatArgs << "--cursor=" + QString::number(this->textCursor().position());
        // Temporary file to apply format changes to
        clangFormatArgs << tmpSrc.fileName();
        auto [stdOut, stdErr] = FormatterManager::run(clangFormatArgs);
        if (!stdErr.isEmpty()) {
            QMessageBox::warning(this, "Ripes",
                                 "Error while executing formatter. Make sure the formatter path points to a "
                                 "valid version of clang-format\n"
                                 "Error message was:\n" +
                                     stdErr,
                                 QMessageBox::Ok);
            return;
        }

        if (!stdOut.isEmpty()) {
            auto cfOutput = parseClangFormatOutput(stdOut);
            if (!cfOutput)
                return;
            QTextCursor curs(this->document());
            curs.select(QTextCursor::Document);
            curs.insertText(cfOutput->formattedText);
            curs.setPosition(cfOutput->cursor);
            this->setTextCursor(curs);
        }
    }
}

void CodeEditor::setupChangedTimer() {
    m_changeTimer = new QTimer(this);
    // configures the change-timer and assembler connectivity with Parser
    m_changeTimer->setInterval(500);
    m_changeTimer->setSingleShot(true);

    // A change in the document will start the timer - when the timer elapses, the contents will be assembled if there
    // is no syntax error. By doing this, the timer is restartet each time a change occurs (ie. a user is continuously
    // typing).
    // An issue here is that QPlainTextEdit emits signals on text AND formatting changes. To only get the text changes,
    // we do as here https://stackoverflow.com/a/22872958
    connect(this, &CodeEditor::modificationChanged, [this] {
        this->document()->setModified(false);
        m_changeTimer->start();
    });

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
    int space = rightPadding + fontMetrics().horizontalAdvance(QString("1")) * digits;
    return space;
}

void CodeEditor::updateSidebarWidth(int /* newBlockCount */) {
    // Set margins of the text edit area
    m_sidebarWidth = lineNumberAreaWidth();
    setViewportMargins(m_sidebarWidth, 0, 0, 0);
}

static int indentationOf(const QString& text) {
    int indent = 0;
    for (const auto& ch : text) {
        if (ch == " ") {
            indent++;
        } else {
            break;
        }
    }
    return indent;
}

static const std::map<QString, QString> c_bracketPair{{"[", "]"}, {"{", "}"}, {"(", ")"}};
static const QStringList c_closingBrackets = QStringList{")", "}", "]"};
static const QStringList c_indentStartCharacters = QStringList{":", "(", "{", "["};

void CodeEditor::keyPressEvent(QKeyEvent* e) {
    const unsigned indentAmt = RipesSettings::value(RIPES_SETTING_INDENTAMT).toInt();

    const auto preCursorText = textCursor().block().text().left(textCursor().positionInBlock());
    const auto postCursorText =
        textCursor().block().text().right(textCursor().block().length() - textCursor().positionInBlock() - 1);
    const QString preCursorChar = preCursorText.isEmpty() ? QString() : *preCursorText.rbegin();
    const QString postCursorChar = postCursorText.isEmpty() ? QString() : *postCursorText.begin();

    /**
     * The following is a collection of quality-of-life changes to the behaviour of the editor, mimmicking features
     * generally found in IDEs.
     */
    if (c_closingBrackets.count(e->text()) && postCursorChar == e->text()) {
        // Skip closing bracket insertion if already present after the current character
        auto cursor = textCursor();
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
        setTextCursor(cursor);
    } else if (c_bracketPair.count(e->text()) && (postCursorChar == " " || postCursorChar.isEmpty())) {
        // Add closing bracket on bracket start
        QPlainTextEdit::keyPressEvent(e);
        insertPlainText(c_bracketPair.at(e->text()));
        auto cursor = textCursor();
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
        setTextCursor(cursor);
    } else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        QString toInsert;
        unsigned indent = 0;
        unsigned postIndent = 0;
        // Maintain level of indentation on enter key
        indent = indentationOf(preCursorText);

        // If this is the start of a bracket, the remainder of the line text is to be moved to the 2nd line following
        // the current cursor position. At this line we maintain the current indentation level.
        auto bracketCharIt = c_bracketPair.find(preCursorChar);
        if (bracketCharIt != c_bracketPair.end() && bracketCharIt->second == postCursorChar) {
            postIndent = indent;
        }

        // Add additional indent if last character was an indent start character, e.g., a bracket
        if (c_indentStartCharacters.contains(preCursorChar)) {
            indent += indentAmt;
        }
        QPlainTextEdit::keyPressEvent(e);
        insertPlainText(QString(" ").repeated(indent));
        if (postIndent != 0) {
            auto prePos = textCursor().position();
            insertPlainText("\n" + QString(" ").repeated(postIndent));
            auto cursor = textCursor();
            cursor.setPosition(prePos, QTextCursor::MoveAnchor);
            setTextCursor(cursor);
        }
    } else if (e->key() == Qt::Key_Tab) {
        insertPlainText(QString(" ").repeated(indentAmt));
    } else if (e->key() == Qt::Key_Backspace) {
        // Delete indentation, if present
        if (preCursorText.endsWith(QString(" ").repeated(indentAmt))) {
            for (unsigned i = 0; i < indentAmt; ++i) {
                textCursor().deletePreviousChar();
            }
        } else if (!preCursorChar.isEmpty() && !postCursorChar.isEmpty() && c_bracketPair.count(preCursorChar) &&
                   postCursorChar == c_bracketPair.at(preCursorChar)) {
            // Delete closing bracket if right after opening bracket
            textCursor().deletePreviousChar();
            textCursor().deleteChar();
        } else {
            QPlainTextEdit::keyPressEvent(e);
        }
    } else {
        QPlainTextEdit::keyPressEvent(e);
    }
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
            auto* isa = ProcessorHandler::currentISA();
            if (isa->isaID() == ISA::RV32I || isa->isaID() == ISA::RV64I) {
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

        QColor lineColor = Colors::Medalist.lighter(160);

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
    painter.fillRect(event->rect(), QColorConstants::LightGray.lighter(120));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColorConstants::Gray.darker(130));
            painter.drawText(0, top, m_lineNumberArea->width() - 3, fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::updateHighlighting() {
    clearBlockHighlights();

    if (!RipesSettings::value(RIPES_SETTING_EDITORSTAGEHIGHLIGHTING).toBool())
        return;

    auto* proc = ProcessorHandler::getProcessor();
    auto program = ProcessorHandler::getProgram().get();

    // Is the current source in sync with the in-memory source?
    if (!program || !program->isSameSource(document()->toPlainText().toUtf8()))
        return;

    auto sourceMapping = program->sourceMapping;

    // Do nothing if no soruce mappings are available.
    if (sourceMapping.empty())
        return;

    // Iterate over the processor stages and use the source mappings to determine the source line which originated the
    // instruction.
    const unsigned stages = proc->stageCount();
    auto colorGenerator = Colors::incrementalRedGenerator(stages);
    QList<QTextEdit::ExtraSelection> highlights;

    for (unsigned sid = 0; sid < stages; sid++) {
        const auto stageInfo = proc->stageInfo(sid);
        QColor stageColor = colorGenerator();
        if (stageInfo.stage_valid) {
            auto mappingIt = sourceMapping.find(stageInfo.pc);
            if (mappingIt == sourceMapping.end()) {
                // No source line registerred for this PC.
                continue;
            }

            for (auto sourceLine : mappingIt->second) {
                // Find block
                QTextBlock block = document()->findBlockByLineNumber(sourceLine);
                if (!block.isValid())
                    continue;

                // Record the stage name for the highlighted block for later painting
                QString stageString = ProcessorHandler::getProcessor()->stageName(sid);
                if (!stageInfo.namedState.isEmpty())
                    stageString += " (" + stageInfo.namedState + ")";
                highlightBlock(block, stageColor, stageString);
            }
        }
    }
}

}  // namespace Ripes
