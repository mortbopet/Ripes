#include "syntaxhighlighter.h"

#include <QList>
#include <QRegularExpressionMatchIterator>
#include <QTextDocument>

#include "defines.h"
#include "lexerutilities.h"

FieldType::FieldType(Type type, int lowerBound, int upperBound, SyntaxHighlighter* highlighter) {
    m_type = type;
    m_lowerBound = lowerBound;
    m_upperBound = upperBound;
    m_highlighter = highlighter;
}

QString FieldType::validateField(const QString& field) const {
    switch (m_type) {
        case Type::Immediate: {
            // Check that immediate can be converted to int from either base 10, hex or binary
            bool ok = false;
            int immediate = field.toInt(&ok, 10);
            int sign = 1;
            if (!ok) {
                // Could not convert directly to integer - try hex or bin. Here, extra care is taken to account for a
                // potential sign, and include this is the range validation
                QString field_ = field;
                if (field_[0] == '-' || field_[0] == '+') {
                    sign = field_[0] == '-' ? -1 : 1;
                    field_.remove(0, 1);
                }
                if (field_.startsWith(QLatin1String("0x"))) {
                    immediate = field_.remove("0x").toUInt(&ok, 16);  // .toInt gives errors where it shouldnt - convert
                                                                      // to UInt and implicitely cast to int when
                                                                      // assigning to variable $immediate
                    if (!ok)
                        return QString("Invalid immediate field - got %1").arg(field);
                } else if (field_.startsWith(QLatin1String("0b"))) {
                    immediate = field_.remove("0b").toUInt(&ok, 2);
                    if (!ok)
                        return QString("Invalid immediate field - got %1").arg(field);
                } else {
                    return QString("Invalid immediate field - got %1").arg(field);
                }
            }
            // Check that immediate is within range
            if ((sign * immediate) >= m_lowerBound && (sign * immediate) <= m_upperBound) {
                return QString();
            } else {
                return QString(
                           "Immediate %1 out of valid range; must be within [%2 "
                           ": %3]")
                    .arg(field)
                    .arg(m_lowerBound)
                    .arg(m_upperBound);
            }
        }
        case Type::Register: {
            if (ABInames.contains(field) || RegNames.contains(field)) {
                return QString();
            } else {
                return QString("Register %1 is unrecognized").arg(field);
            }
        }
        case Type::Offset: {
            // Check if label is defined in highlighter
            Q_ASSERT(m_highlighter != nullptr);
            if (m_highlighter->m_labelPosMap.contains(field)) {
                return QString();
            } else {
                return QString("label \"%1\" is undefined").arg(field);
            }
        }
        case Type::String: {
            if (field.length() > 1 && field[0] == "\"" && field[field.length() - 1] == "\"") {
                return QString();
            } else {
                return QString("Invalid string - must be delimitered with quotes (\")");
            }
        }
    }
    return QString("Validation error");
}

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
    connect(this, &SyntaxHighlighter::rehighlightInvalidBlock, this, &SyntaxHighlighter::rehighlightBlock);
    createSyntaxRules();
    errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    errorFormat.setUnderlineColor(Qt::red);

    HighlightingRule rule;

    // Create rules for name-specific registers
    regFormat.setForeground(QColor(0x800000));
    QStringList registerPatterns;
    registerPatterns << "\\bzero\\b"
                     << "\\bra\\b"
                     << "\\bsp\\b"
                     << "\\bgp\\b"
                     << "\\btp\\b"
                     << "\\bfp\\b";
    for (const auto& pattern : registerPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = regFormat;
        m_highlightingRules.append(rule);
    }

    // Create rules for registers that can easily be regex matched
    // (saved, temporary and argument registers)
    rule.pattern = QRegularExpression("\\b[(a|s|t|x)][0-9]{1,2}");
    rule.format = regFormat;
    m_highlightingRules.append(rule);

    // Create match cases for instructions
    QStringList instructionPatterns;
    instructionPatterns << "\\bla\\b"
                        << "\\brd\\b"
                        << "\\blw\\b"
                        << "\\blh\\b"
                        << "\\blui\\b"
                        << "\\blb\\b"
                        << "\\bsb\\b"
                        << "\\bsh\\b"
                        << "\\bsw\\b"
                        << "\\bnop\\b"
                        << "\\bli\\b"
                        << "\\bmv\\b"
                        << "\\bnot\\b"
                        << "\\bneg\\b"
                        << "\\bnegw\\b"
                        << "\\bsext.w\\b"
                        << "\\bseqz\\b"
                        << "\\bsnez\\b"
                        << "\\bsltz\\b"
                        << "\\bsgtz\\b"
                        << "\\bbeqz\\b"
                        << "\\bbnez\\b"
                        << "\\bblez\\b"
                        << "\\bbgez\\b"
                        << "\\bbltz\\b"
                        << "\\bbgtz\\b"
                        << "\\bbgt\\b"
                        << "\\bble\\b"
                        << "\\bbgtu\\b"
                        << "\\bbleu\\b"
                        << "\\bj\\b"
                        << "\\bjal\\b"
                        << "\\bjr\\b"
                        << "\\bjalr\\b"
                        << "\\bret\\b"
                        << "\\bcall\\b"
                        << "\\btail\\b"
                        /*
                        << "\\bfence\\b"
                        << "\\brdinstret\\b"
                        << "\\brdcycle\\b"
                        << "\\brdtime\\b"
                        << "\\bcsrr\\b"
                        << "\\bcsrw\\b"
                        << "\\bcsrs\\b"
                        << "\\bcsrc\\b"
                        << "\\bcsrwi\\b"
                        << "\\bcsrsi\\b"
                        << "\\bcsrci\\b"
                                               */
                        << "\\bauipc\\b"
                        << "\\badd\\b"
                        << "\\baddi\\b"
                        << "\\bxor\\b"
                        << "\\bxori\\b"
                        << "\\bsub\\b"
                        << "\\bsubw\\b"
                        << "\\baddiw\\b"
                        << "\\bsltiu\\b"
                        << "\\bsltu\\b"
                        << "\\bslt\\b"
                        << "\\bbeq\\b"
                        << "\\bbne\\b"
                        << "\\bbge\\b"
                        << "\\bblt\\b"
                        << "\\bbltu\\b"
                        << "\\bbgeu\\b"
                        << "\\bsrl\\b"
                        << "\\bsrli\\b"
                        << "\\bsll\\b"
                        << "\\bslli\\b"
                        << "\\bsra\\b"
                        << "\\bsrai\\b"
                        << "\\bor\\b"
                        << "\\bori\\b"
                        << "\\band\\b"
                        << "\\bandi\\b"
                        << "\\becall\\b"

                        << "\\bmul\\b"
                        << "\\bmulh\\b"
                        << "\\bmulhu\\b"
                        << "\\bmulhsu\\b"

                        << "\\bdiv\\b"
                        << "\\bdivu\\b"
                        << "\\brem\\b"
                        << "\\bremu\\b";

    instrFormat.setForeground(QColor(Colors::BerkeleyBlue));
    for (const auto& pattern : instructionPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = instrFormat;
        m_highlightingRules.append(rule);
    }

    // Create immediate hightlighting rule
    immFormat.setForeground(QColor(Qt::darkGreen));
    rule.pattern = QRegularExpression("\\b(?<![A-Za-z])[-+]?\\d+");
    rule.format = immFormat;
    m_highlightingRules.append(rule);

    // Create immediate highlighting rule for prefixed numbers (0x, 0b)
    immFormat.setForeground(QColor(Qt::darkGreen));
    rule.pattern = QRegularExpression("([-+]?0[xX][0-9a-fA-F]+|[-+]?0[bB][0-1]+)");
    rule.format = immFormat;
    m_highlightingRules.append(rule);

    // Create comment highlighting rule
    commentFormat.setForeground(QColor(Colors::Medalist));
    rule.pattern = QRegularExpression("[#]+.*");
    rule.format = commentFormat;
    m_highlightingRules.append(rule);

    // Create label highlighting rule
    commentFormat.setForeground(QColor(Colors::Medalist));
    rule.pattern = QRegularExpression("(.*?[:])");
    rule.format = commentFormat;
    m_highlightingRules.append(rule);

    // Create string highlighting rule
    stringFormat.setForeground(QColor(0x800000));
    rule.pattern = QRegularExpression(R"("(?:[^"]|\.)*")");
    rule.format = stringFormat;
    m_highlightingRules.append(rule);
}

void SyntaxHighlighter::highlightBlock(const QString& text) {
    QString tooltip = checkSyntax(text);
    int row = currentBlock().firstLineNumber();
    if (tooltip != QString()) {
        setFormat(0, text.length(), errorFormat);
        emit setTooltip(row, tooltip);
    } else {
        emit setTooltip(row, QString());
        for (const HighlightingRule& rule : m_highlightingRules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }
}

void SyntaxHighlighter::createSyntaxRules() {
    // Create syntax rules for all base- and pseudoinstructions
    auto rule = SyntaxRule();
    QList<FieldType> types = QList<FieldType>();
    QStringList names;

    // nop
    rule.instr = "nop";
    rule.fields = 1;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);

    // ecall
    rule.instr = "ecall";
    rule.fields = 1;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);

    // call
    types.clear();
    types << FieldType(Type::Offset, 0, 0, this);
    rule.instr = "call";
    rule.fields = 2;
    rule.inputs = types;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);

    // jr
    types.clear();
    types << FieldType(Type::Register);
    rule.instr = "jr";
    rule.fields = 2;
    rule.inputs = types;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);

    // jalr -- pseudo and regular op.
    types.clear();
    types << FieldType(Type::Register) << FieldType(Type::Register) << FieldType(Type::Immediate, -2048, 2047);
    rule.instr = "jalr";
    rule.fields = 4;
    rule.fields2 = 2;
    rule.inputs = types;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);
    rule.fields2 = -1;

    // jal -- pseudo and regular op
    types.clear();
    types << FieldType(Type::Register) << FieldType(Type::Offset, 0, 0, this);
    rule.instr = "jal";
    rule.fields = 3;
    rule.fields2 = 2;
    rule.inputs = types;
    rule.skipFirstField = true;  // jal can be written as "jal offset" - allow skipping of first field
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);
    rule.skipFirstField = false;  // revert
    rule.fields2 = -1;

    // lui -- pseudo
    types.clear();
    types << FieldType(Type::Register) << FieldType(Type::Immediate, 0, 1048575, this);
    rule.instr = "lui";
    rule.fields = 3;
    rule.inputs = types;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);

    // j
    types.clear();
    types << FieldType(Type::Offset, 0, 0, this);
    rule.instr = "j";
    rule.fields = 2;
    rule.inputs = types;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);

    // li
    types.clear();
    types << FieldType(Type::Register) << FieldType(Type::Immediate, -2147483648, 2147483647);
    rule.instr = "li";
    rule.fields = 3;
    rule.inputs = types;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);

    // la
    types.clear();
    types << FieldType(Type::Register) << FieldType(Type::Offset, 0, 0, this);
    rule.instr = "la";
    rule.fields = 3;
    rule.inputs = types;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);

    // auipc
    types.clear();
    types << FieldType(Type::Register) << FieldType(Type::Immediate, 0, 1048575);
    rule.instr = "auipc";
    rule.fields = 3;
    rule.inputs = types;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);

    // 2-register pseudoinstructions
    types.clear();
    names.clear();
    types << FieldType(Type::Register) << FieldType(Type::Register);
    names << "mv"
          << "not"
          << "neg"
          << "negw"
          << "sext"
          << "seqz"
          << "snez"
          << "sltz"
          << "sgtz";
    for (const auto& name : names) {
        rule.instr = name;
        rule.fields = 3;
        rule.inputs = types;
        m_syntaxRules.insert(name, QList<SyntaxRule>() << rule);
    }

    // Branch instructions
    types.clear();
    names.clear();
    types << FieldType(Type::Register) << FieldType(Type::Register) << FieldType(Type::Offset, 0, 0, this);
    names << "beq"
          << "bne"
          << "blt"
          << "bge"
          << "ble"
          << "bgt"
          << "bltu"
          << "bgeu"
          << "bleu";
    for (const auto& name : names) {
        rule.instr = name;
        rule.fields = 4;
        rule.inputs = types;
        m_syntaxRules.insert(name, QList<SyntaxRule>() << rule);
    }

    // Branch pseudo-instructions
    types.clear();
    names.clear();
    types << FieldType(Type::Register) << FieldType(Type::Offset, 0, 0, this);
    names << "beqz"
          << "bnez"
          << "blez"
          << "bgez"
          << "bltz"
          << "bgtz";
    for (const auto& name : names) {
        rule.instr = name;
        rule.fields = 3;
        rule.inputs = types;
        m_syntaxRules.insert(name, QList<SyntaxRule>() << rule);
    }

    // I type instructions
    types.clear();
    names.clear();
    types << FieldType(Type::Register) << FieldType(Type::Register) << FieldType(Type::Immediate, -2048, 2047);
    names << "addi"
          << "slti"
          << "sltiu"
          << "xori"
          << "ori"
          << "andi"
          << "slli"
          << "srli"
          << "srai";
    for (const auto& name : names) {
        rule.instr = name;
        rule.fields = 4;
        rule.inputs = types;
        m_syntaxRules.insert(name, QList<SyntaxRule>() << rule);
    }

    // No-op instructions and assembler directives
    types.clear();
    names.clear();
    types.clear();
    names << "ecall"
          << "nop"
          << "ret"
          << ".text"
          << ".data";
    for (const auto& name : names) {
        rule.instr = name;
        rule.fields = 1;
        rule.inputs = types;
        m_syntaxRules.insert(name, QList<SyntaxRule>() << rule);
    }

    // Load instructions
    QMap<QString, QList<SyntaxRule>> loadRules;
    types.clear();
    names.clear();
    types << FieldType(Type::Register) << FieldType(Type::Immediate, -2048, 2047) << FieldType(Type::Register);
    names << "lb"
          << "lh"
          << "lw"
          << "lbu"
          << "lhu";
    for (const auto& name : names) {
        rule.instr = name;
        rule.fields = 4;
        rule.inputs = types;
        loadRules.insert(name, QList<SyntaxRule>() << rule);
    }

    // Load pseudoinstructions
    types.clear();
    names.clear();
    types << FieldType(Type::Register) << FieldType(Type::Offset, 0, 0, this);
    names << "lb"
          << "lh"
          << "lw";

    for (const auto& name : names) {
        rule.instr = name;
        rule.fields = 3;
        rule.inputs = types;
        loadRules.insert(name, loadRules[name] << rule);
    }
    m_syntaxRules.unite(loadRules);

    // R type instructions
    types.clear();
    names.clear();
    types << FieldType(Type::Register) << FieldType(Type::Register) << FieldType(Type::Register);
    names << "add"
          << "sub"
          << "sll"
          << "slt"
          << "sltu"
          << "xor"
          << "srl"
          << "sra"
          << "or"
          << "and"
          << "mul"
          << "mulh"
          << "mulhu"
          << "mulhsu"
          << "rem"
          << "remu"
          << "div"
          << "divu";
    for (const auto& name : names) {
        rule.instr = name;
        rule.fields = 4;
        rule.inputs = types;
        m_syntaxRules.insert(name, QList<SyntaxRule>() << rule);
    }

    // S type instructions
    QMap<QString, QList<SyntaxRule>> storeRules;
    types.clear();
    names.clear();
    types << FieldType(Type::Register) << FieldType(Type::Immediate, -2048, 2047) << FieldType(Type::Register);
    names << "sb"
          << "sh"
          << "sw";
    for (const auto& name : names) {
        rule.instr = name;
        rule.fields = 4;
        rule.inputs = types;
        storeRules.insert(name, QList<SyntaxRule>() << rule);
    }

    // S type pseudo-ops
    types.clear();
    names.clear();
    types << FieldType(Type::Register) << FieldType(Type::Offset, 0, 0, this) << FieldType(Type::Register);
    names << "sb"
          << "sh"
          << "sw";
    for (const auto& name : names) {
        rule.instr = name;
        rule.fields = 4;
        rule.inputs = types;
        storeRules.insert(name, storeRules[name] << rule);
    }
    m_syntaxRules.unite(storeRules);

    // .byte
    types.clear();
    types << FieldType(Type::Immediate, -128, 255);
    rule.hasListField = true;  // a list of immediates can be provided
    rule.instr = ".byte";
    rule.fields = 2;
    rule.inputs = types;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);

    // .2byte & half
    types.clear();
    types << FieldType(Type::Immediate, -32768, 65535);
    rule.hasListField = true;  // a list of immediates can be provided
    rule.instr = ".2byte";
    rule.fields = 2;
    rule.inputs = types;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);
    rule.instr = ".half";
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);
    rule.instr = ".short";
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);

    // .4byte & word
    types.clear();
    types << FieldType(Type::Immediate, INT_MIN, INT_MAX);
    rule.hasListField = true;  // a list of immediates can be provided
    rule.instr = ".4byte";
    rule.fields = 2;
    rule.inputs = types;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);
    rule.instr = ".word";
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);
    rule.instr = ".long";
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);
    rule.hasListField = false;

    // Emit zero's (array allocation on static data segment)
    types.clear();
    types << FieldType(Type::Immediate, 1, 0xFFFF);
    rule.fields = 2;
    rule.inputs = types;
    rule.instr = ".zero";
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);

    // .string & .asciz
    types.clear();
    types << FieldType(Type::String);
    rule.instr = ".string";
    rule.fields = 2;
    rule.inputs = types;
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);
    rule.instr = ".asciz";
    m_syntaxRules.insert(rule.instr, QList<SyntaxRule>() << rule);
}

namespace {
// Shorthand macro for returning unknown instruction string, based on whether user tried to input an assembler directive
#define UNKNOWN_OP(string)                             \
    if (string[0] == ".")                              \
        return QString("Unknown assembler directive"); \
    return QString("Unknown operation");
}  // namespace

QString SyntaxHighlighter::checkSyntax(const QString& input) {
    QStringList fields = input.split(splitter);
    fields.removeAll("");
    fields = splitQuotes(fields);

    // Empty fields? return
    if (fields.isEmpty())
        return QString();

    // Throw away case information
    std::transform(fields.begin(), fields.end(), fields.begin(), [](const QString& s) { return s.toLower(); });

    int pos = currentBlock().firstLineNumber();

    // check for labels
    QString string = fields[0];
    if (string.contains(':')) {
        if (string.count(':') > 1) {
            return QString("Multiple instances of ':' in label");
        }
        // Label detected - check if already defined, else add to label definitions
        QStringList splitFirst = string.split(':');
        string = splitFirst[0];  // get label
        if (string.isEmpty()) {
            return QString("Empty label name");
        }
        // Update map entries at given block
        if (m_labelPosMap.contains(string) && m_labelPosMap[string] != pos) {
            // duplicate label found
            if (pos < m_labelPosMap[string]) {
                // Label is redefined before previous use of label
                m_posLabelMap.remove(m_labelPosMap[string]);
                m_labelPosMap[string] = pos;
                m_posLabelMap[pos] = string;
            } else {
                return QString("Multiple definitions of label %1").arg(string);
            }
        } else {
            // no duplicates, update label at given pos
            m_labelPosMap.remove(m_posLabelMap[pos]);
            m_labelPosMap[string] = pos;
            m_posLabelMap[pos] = string;
        }
        // Remove label from fields vector
        if (splitFirst.size() > 1) {
            fields[0] = splitFirst[1];
        } else {
            fields.removeAt(0);
        }
        // Return if empty fields vector
        fields.removeAll("");
        if (fields.isEmpty())
            return QString();
    }

    // Remove comments from syntax evaluation
    const static auto commentRegEx = QRegularExpression("[#](.*)");
    int commentIndex = fields.indexOf(commentRegEx);
    if (commentIndex != -1) {
        int index = fields.length();
        while (index >= commentIndex) {
            fields.removeAt(index);
            index--;
        }
        // Empty fields? return
        if (fields.isEmpty())
            return QString();
    }

    // -- Validate remaining fields --
    // For the corresponding rules to an instruction in m_syntaxRules, the SyntaxRule list will be iterated through
    // until either a rule has validated a field, or the end of the syntaxrule list is reached
    QStringList resList;  // Tooltip results
    if (fields.size() > 0) {
        auto ruleIter = m_syntaxRules.find(fields[0]);
        if (ruleIter != m_syntaxRules.end()) {
            // A rule for the instruction has been found
            for (const auto& rule : *ruleIter) {
                if (fields.size() == rule.fields || (rule.fields2 != -1 && fields.size() == rule.fields2) ||
                    rule.hasListField) {
                    // fields size is correct, check each instruction
                    int nFields = fields.size();
                    QString currentRes = QString();
                    for (int index = 1; index < nFields; index++) {
                        // Get input rule - skipFirstField is a special case handler for jal pseudo-op when only 2
                        // fields are available
                        int ruleIndex = index;
                        if (rule.hasListField) {
                            // For list arguments, the last available syntax rule will be the rule for all inputs
                            ruleIndex = rule.inputs.size();
                        }

                        auto inputRule = rule.skipFirstField && nFields == rule.fields2 ? rule.inputs[ruleIndex]
                                                                                        : rule.inputs[ruleIndex - 1];
                        // If an offset is used, store line number, to re-update after all variable declarations have
                        // been mapped (after syntax checking the entire document)
                        if (inputRule.m_type == Type::Offset) {
                            m_rowsUsingLabels << pos;
                        }
                        // Validation for the current rule continues if res is still an empty string. If not, the string
                        // in res is kept, which will prompt the algorithm to either return res or validate subsequent
                        // validation rules
                        currentRes = inputRule.validateField(fields[index]);
                        if (currentRes == QString()) {
                            // continue
                            // index++;
                        } else {
                            resList.append(currentRes);
                            break;
                        }
                    }
                    if (currentRes.isEmpty()) {
                        // Validation successfull
                        return QString();
                    }
                } else {
                    // Invalid number of arguments
                    resList.append(QString("Invalid number of arguments"));
                }
            }
            if (!resList.isEmpty())
                return resList[0];
            /* return */ UNKNOWN_OP(fields[0]);
        } else {
            /* return */ UNKNOWN_OP(fields[0]);
        }
    }
    return QString();
}

void SyntaxHighlighter::reset() {
    m_labelPosMap.clear();
    m_posLabelMap.clear();
    m_rowsUsingLabels.clear();
}

void SyntaxHighlighter::clearAndRehighlight() {
    reset();
    rehighlight();
    for (const auto& row : m_rowsUsingLabels) {
        // Rehighlight rows which use labels, now that all labels have been recorded
        rehighlightBlock(document()->findBlockByLineNumber(row));
    }
}

void SyntaxHighlighter::invalidateLabels(const QTextCursor& cursor) {
    if (m_posLabelMap.contains(cursor.block().firstLineNumber())) {
        // a current label exists at the cursor position - if current action was a newline event, we need to reset
        // resets label mapping and rehighlights required lines
        clearAndRehighlight();
    }
}
