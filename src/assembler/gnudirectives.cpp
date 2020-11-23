#include "gnudirectives.h"

namespace Ripes {
namespace AssemblerTmp {

#define add_directive(container, directive) container.push_back(std::make_shared<Directive>(directive));
namespace {
int getImmediate(QString string, bool& canConvert) {
    string = string.toUpper();
    canConvert = false;
    int immediate = string.toInt(&canConvert, 10);
    int sign = 1;
    if (!canConvert) {
        // Could not convert directly to integer - try hex or bin. Here, extra care is taken to account for a
        // potential sign, and include this is the range validation
        if (string[0] == '-' || string[0] == '+') {
            sign = string[0] == '-' ? -1 : 1;
            string.remove(0, 1);
        }
        if (string.startsWith(QLatin1String("0X"))) {
            immediate = string.remove("0X").toUInt(&canConvert, 16);
        } else if (string.startsWith(QLatin1String("0B"))) {
            immediate = string.remove("0B").toUInt(&canConvert, 2);
        } else {
            canConvert = false;
        }
    }
    return sign * immediate;
}
}  // namespace

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
HandleDirectiveRes dataFunctor(const Directive&, const TokenizedSrcLine& line) {
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

HandleDirectiveRes stringFunctor(const Directive&, const TokenizedSrcLine& line) {
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

Directive zeroDirective() {
    auto zeroFunctor = [](const Directive&, const TokenizedSrcLine& line) -> HandleDirectiveRes {
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
