#include "gnudirectives.h"
#include "assembler.h"

namespace Ripes {
namespace Assembler {

static QString numTokensError(unsigned expected, const TokenizedSrcLine& line) {
    QString err = QString::fromStdString("Invalid number of directive arguments; expected " + std::to_string(expected) +
                                         " but got " + std::to_string(line.tokens.size()) + "[");
    for (auto t : llvm::enumerate(line.tokens)) {
        err += t.value();
        if (t.index() < (line.tokens.size() - 1))
            err += ", ";
    };
    err += "].";
    return err;
}

#define add_directive(container, directive) container.push_back(std::make_shared<Directive>(directive));

DirectiveVec gnuDirectives() {
    DirectiveVec directives;

    add_directive(directives, stringDirective());
    add_directive(directives, ascizDirective());
    add_directive(directives, zeroDirective());
    add_directive(directives, byteDirective());
    add_directive(directives, doubleDirective());
    add_directive(directives, wordDirective());
    add_directive(directives, halfDirective());
    add_directive(directives, shortDirective());
    add_directive(directives, twoByteDirective());
    add_directive(directives, fourByteDirective());
    add_directive(directives, longDirective());
    add_directive(directives, equDirective());
    add_directive(directives, alignDirective());

    add_directive(directives, dataDirective());
    add_directive(directives, textDirective());
    add_directive(directives, bssDirective());

    add_directive(directives, dummyDirective(".global"));
    add_directive(directives, dummyDirective(".globl"));

    return directives;
}

#define getImmediateErroring(token, res, srcline)        \
    auto exprRes##res = assembler->evalExpr(token);      \
    if (auto* err = std::get_if<Error>(&exprRes##res)) { \
        err->first = srcline;                            \
        return {*err};                                   \
    }                                                    \
    res = std::get<ExprEvalVT>(exprRes##res);

template <size_t size>
std::optional<Error> assembleData(const AssemblerBase* assembler, const TokenizedSrcLine& line, QByteArray& byteArray) {
    static_assert(size >= 1, "");
    for (const auto& token : line.tokens) {
        int64_t val;
        static_assert(sizeof(val) >= size, "Requested data width greater than what is representable");
        getImmediateErroring(token, val, line.sourceLine);

        if (isUInt<size * 8>(val) || isInt<size * 8>(val)) {
            for (size_t i = 0; i < size; ++i) {
                byteArray.append(val & 0xff);
                val >>= 8;
            }
        } else {
            return {Error(line.sourceLine, QString("'%1' does not fit in %2 bytes").arg(val).arg(size))};
        }
    }
    return {};
}

template <size_t size>
HandleDirectiveRes dataFunctor(const AssemblerBase* assembler, const DirectiveArg& arg) {
    if (arg.line.tokens.length() < 1) {
        return {Error(arg.line.sourceLine, "Invalid number of arguments (expected >1)")};
    }
    QByteArray bytes;
    auto err = assembleData<size>(assembler, arg.line, bytes);
    if (err) {
        return {err.value()};
    } else {
        return {bytes};
    }
}

HandleDirectiveRes stringFunctor(const AssemblerBase*, const DirectiveArg& arg) {
    if (arg.line.tokens.length() != 1) {
        return HandleDirectiveRes{Error(arg.line.sourceLine, numTokensError(1, arg.line))};
    }
    QString string = arg.line.tokens.at(0);
    string.replace("\\n", "\n");
    string.remove('\"');
    return {string.toUtf8().append('\0')};
}

Directive ascizDirective() {
    return Directive(".asciz", &stringFunctor);
}

Directive byteDirective() {
    return Directive(".byte", &dataFunctor<1>);
}

Directive doubleDirective() {
    return Directive(".dword", &dataFunctor<8>);
}

Directive wordDirective() {
    return Directive(".word", &dataFunctor<4>);
}

Directive halfDirective() {
    return Directive(".half", &dataFunctor<2>);
}

Directive shortDirective() {
    return Directive(".short", &dataFunctor<2>);
}

Directive twoByteDirective() {
    return Directive(".2byte", &dataFunctor<2>);
}

Directive fourByteDirective() {
    return Directive(".4byte", &dataFunctor<4>);
}

Directive longDirective() {
    return Directive(".long", &dataFunctor<4>);
}

Directive stringDirective() {
    return Directive(".string", &stringFunctor);
}

/**
 * @brief dummyDirective
 * Generates a directive handler for @p name, which returns nothing. To be used for compatability reasons.
 */
Directive dummyDirective(const QString& name) {
    return Directive(name,
                     [](const AssemblerBase*, const DirectiveArg&) -> HandleDirectiveRes { return {QByteArray()}; });
}

Directive::DirectiveHandler genSegmentChangeFunctor(const QString& segment) {
    return [segment](const AssemblerBase* assembler, const DirectiveArg& arg) {
        if (arg.line.tokens.length() != 0) {
            return HandleDirectiveRes{Error(arg.line.sourceLine, numTokensError(0, arg.line))};
        }
        auto err = assembler->setCurrentSegment(segment);
        if (err) {
            // Embed source line into error message
            err.value().first = arg.line.sourceLine;
            return HandleDirectiveRes{err.value()};
        }
        return HandleDirectiveRes(std::nullopt);
    };
}

Directive textDirective() {
    return Directive(".text", genSegmentChangeFunctor(".text"));
}

Directive bssDirective() {
    return Directive(".bss", genSegmentChangeFunctor(".bss"));
}

Directive dataDirective() {
    return Directive(".data", genSegmentChangeFunctor(".data"));
}

Directive zeroDirective() {
    auto zeroFunctor = [](const AssemblerBase* assembler, const DirectiveArg& arg) -> HandleDirectiveRes {
        if (arg.line.tokens.length() != 1) {
            return HandleDirectiveRes{Error(arg.line.sourceLine, numTokensError(1, arg.line))};
        }
        int64_t value;
        getImmediateErroring(arg.line.tokens.at(0), value, arg.line.sourceLine);
        QByteArray bytes;
        bytes.fill(0x0, value);
        return {bytes};
    };
    return Directive(".zero", zeroFunctor);
}

Directive equDirective() {
    auto equFunctor = [](const AssemblerBase* assembler, const DirectiveArg& arg) -> HandleDirectiveRes {
        if (arg.line.tokens.length() != 2) {
            return HandleDirectiveRes{Error(arg.line.sourceLine, numTokensError(2, arg.line))};
        }
        int64_t value;
        getImmediateErroring(arg.line.tokens.at(1), value, arg.line.sourceLine);

        auto err = assembler->addSymbol(arg.line, arg.line.tokens.at(0), value);
        if (err) {
            return err.value();
        }

        return HandleDirectiveRes(std::nullopt);
    };
    return Directive(".equ", equFunctor,
                     true /* Constants should be made available during ie. pseudo instruction expansion */);
}

Directive alignDirective() {
    auto alignFunctor = [](const AssemblerBase* assembler, const DirectiveArg& arg) -> HandleDirectiveRes {
        if (arg.line.tokens.length() == 0 || arg.line.tokens.length() > 3) {
            return {Error(arg.line.sourceLine, "Invalid number of arguments (expected at least 1, at most 3)")};
        }
        int boundary, fill, max;
        fill = max = 0;
        bool hasMax = false;

        getImmediateErroring(arg.line.tokens.at(0), boundary, arg.line.sourceLine);
        if (arg.line.tokens.size() > 1) {
            getImmediateErroring(arg.line.tokens.at(1), fill, arg.line.sourceLine);
        }
        if (arg.line.tokens.size() > 2) {
            getImmediateErroring(arg.line.tokens.at(2), max, arg.line.sourceLine);
            hasMax = true;
        }

        if (boundary < 0 || fill < 0 || (hasMax && max < 0)) {
            return {Error(arg.line.sourceLine, ".align arguments must be positive")};
        }
        if (fill > UINT8_MAX) {
            return {Error(arg.line.sourceLine, ".align fill value must be in range [0;255]")};
        }
        if (boundary == 0) {
            return {QByteArray()};
        }
        int byteOffset = (arg.section->address + arg.section->data.size()) % boundary;
        int bytesToSkip = byteOffset != 0 ? boundary - byteOffset : 0;
        if (max > 0 && bytesToSkip > max) {
            return {QByteArray()};
        }

        return {QByteArray(1, static_cast<char>(fill)).repeated(bytesToSkip)};
    };
    return Directive(".align", alignFunctor);
}

}  // namespace Assembler
}  // namespace Ripes
