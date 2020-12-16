#include "gnudirectives.h"
#include "assembler.h"

namespace Ripes {
namespace Assembler {

#define add_directive(container, directive) container.push_back(std::make_shared<Directive>(directive));

DirectiveVec gnuDirectives() {
    DirectiveVec directives;

    add_directive(directives, stringDirective());
    add_directive(directives, ascizDirective());
    add_directive(directives, zeroDirective());
    add_directive(directives, byteDirective());
    add_directive(directives, wordDirective());
    add_directive(directives, halfDirective());
    add_directive(directives, shortDirective());
    add_directive(directives, twoByteDirective());
    add_directive(directives, fourByteDirective());
    add_directive(directives, longDirective());

    add_directive(directives, dataDirective());
    add_directive(directives, textDirective());
    add_directive(directives, bssDirective());

    return directives;
}

std::optional<Error> assembleData(const TokenizedSrcLine& line, QByteArray& byteArray, size_t size) {
    Q_ASSERT(size >= 1 && size <= 4);
    bool ok;
    for (const auto& token : line.tokens) {
        qlonglong val = getImmediate(token, ok);
        if (!ok) {
            return {Error(line.sourceLine, "Invalid immediate value")};
        }

        for (size_t i = 0; i < size; i++) {
            byteArray.append(val & 0xff);
            val >>= 8;
        }
    }
    return {};
}

template <size_t width>
HandleDirectiveRes dataFunctor(const AssemblerBase*, const TokenizedSrcLine& line) {
    if (line.tokens.length() < 1) {
        return {Error(line.sourceLine, "Invalid number of arguments (expected >1)")};
    }
    QByteArray bytes;
    auto err = assembleData(line, bytes, width);
    if (err) {
        return {err.value()};
    } else {
        return {bytes};
    }
};

HandleDirectiveRes stringFunctor(const AssemblerBase*, const TokenizedSrcLine& line) {
    if (line.tokens.length() != 1) {
        return {Error(line.sourceLine, "Invalid number of arguments (expected 1)")};
    }
    QString string = line.tokens.at(0);
    string.replace("\\n", "\n");
    string.remove('\"');
    string.append('\0');
    return {string.toUtf8()};
};

Directive ascizDirective() {
    return Directive(".asciz", &stringFunctor);
}

Directive byteDirective() {
    return Directive(".byte", &dataFunctor<1>);
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

Directive::DirectiveHandler genSegmentChangeFunctor(const QString& segment) {
    return [segment](const AssemblerBase* assembler, const TokenizedSrcLine& line) {
        if (line.tokens.length() != 0) {
            return HandleDirectiveRes{Error(line.sourceLine, "Invalid number of arguments (expected 0)")};
        }
        auto err = assembler->setCurrentSegment(segment);
        if (err) {
            // Embed source line into error message
            err.value().first = line.sourceLine;
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
    auto zeroFunctor = [](const AssemblerBase*, const TokenizedSrcLine& line) -> HandleDirectiveRes {
        if (line.tokens.length() != 1) {
            return {Error(line.sourceLine, "Invalid number of arguments (expected 1)")};
        }
        bool ok;
        int value = getImmediate(line.tokens.at(0), ok);
        if (!ok) {
            return {Error(line.sourceLine, "Invalid argument")};
        }
        QByteArray bytes;
        for (int i = 0; i < value; i++) {
            bytes.append(static_cast<char>(0x0));
        }
        return {bytes};
    };
    return Directive(".zero", zeroFunctor);
}

}  // namespace Assembler
}  // namespace Ripes
