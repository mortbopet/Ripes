#ifndef LEXERUTILITIES_H
#define LEXERUTILITIES_H

#include <QStringList>

static QStringList splitQuotes(const QStringList& list) {
    // Manual string splitter - splits at ' ' (space) and ',' (comma) characters, unless test is delimitered by quotes or parentheses
    QStringList ret;
    for (const auto& s : list) {
        QString outString;
        bool inQuote = false;
        bool inParen = false;
        for (int i = 0; i < s.length(); i++) {
            inQuote ^= s[i] == '"';
            inParen = (inParen || (s[i] == '(')) && s[i] != ')';
            if ((s[i] == ' ' || s[i] == ',') && !inQuote && !inParen) {
                ret.append(outString);
                outString.clear();
            } else {
                outString.append(s[i]);
                if (i == s.length() - 1)
                    ret.append(outString);
            }
        }
    }
    ret.removeAll("");
    return ret;
}

#endif  // LEXERUTILITIES_H
