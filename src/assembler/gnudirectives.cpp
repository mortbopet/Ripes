#include "gnudirectives.h"
#include "assembler.h"

namespace Ripes {
namespace AssemblerTmp {

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

std::optional<Error> assembleData(const QStringList& tokens, QByteArray& byteArray, size_t size) {
    Q_ASSERT(size >= 1 && size <= 4);
    bool ok;
    for (int i = 1; i < tokens.size(); i++) {
        qlonglong val = getImmediate(tokens[i], ok);

        for (size_t i = 0; i < size; i++) {
            byteArray.append(val & 0xff);
            val >>= 8;
        }
    }
    return {};
}

template <size_t width>
HandleDirectiveRes dataFunctor(const AssemblerBase*, const TokenizedSrcLine& line) {
    if (line.tokens.length() < 2) {
        return {Error(line.sourceLine, "Invalid number of arguments")};
    }
    QByteArray bytes;
    auto err = assembleData(line.tokens, bytes, width);
    if (err) {
        return {err.value()};
    } else {
        return {bytes};
    }
};

HandleDirectiveRes stringFunctor(const AssemblerBase*, const TokenizedSrcLine& line) {
    if (line.tokens.length() != 2) {
        return {Error(line.sourceLine, "Invalid number of arguments")};
    }

    QByteArray byteArray;
    QString string;
    // Merge input fields
    for (int i = 1; i < line.tokens.size(); i++) {
        QString strarg = line.tokens[i];
        strarg.replace("\\n", "\n");
        string += strarg;
    }
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

template <char const* str>
HandleDirectiveRes segmentChangeFunctor(const AssemblerBase* assembler, const TokenizedSrcLine& line) {
    auto err = assembler->setCurrentSegment(str);
    if (err) {
        // Embed source line into error message
        err.value().first = line.sourceLine;
        return {err.value()};
    }
    return std::nullopt;
};

constexpr char s_text[] = ".text";
Directive textDirective() {
    return Directive(".text", &segmentChangeFunctor<s_text>);
}

constexpr char s_bss[] = ".bss";
Directive bssDirective() {
    return Directive(".bss", &segmentChangeFunctor<s_bss>);
}

constexpr char s_data[] = ".data";
Directive dataDirective() {
    return Directive(".data", &segmentChangeFunctor<s_data>);
}

Directive zeroDirective() {
    auto zeroFunctor = [](const AssemblerBase*, const TokenizedSrcLine& line) -> HandleDirectiveRes {
        if (line.tokens.length() != 2) {
            return {Error(line.sourceLine, "Invalid number of arguments")};
        }
        bool ok;
        int value = getImmediate(line.tokens[1], ok);
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

}  // namespace AssemblerTmp
}  // namespace Ripes
