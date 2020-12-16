#pragma once

#include <QString>
#include <iostream>
#include <map>
#include <vector>

namespace Ripes {
namespace Assembler {

// An error is defined as a reference to a source line index + an error string
using Error = std::pair<unsigned, QString>;
class Errors : public std::vector<Error> {
public:
    void print() const {
        for (const auto& iter : *this) {
            std::cout << "line " << iter.first << ": " << iter.second.toStdString() << "\n";
        }
        std::cout << std::flush;
    }

    /**
     * @brief toMap
     * @return map representation of [source line: error message].
     * A map cache is created whenever a discrepancy between size(this) != size(_mapcache). @todo this obviously assumes
     * that any element pushed onto *this error vector is never modified or removed from the set of errors. A bit iffy,
     * but currently, Errors objects are returned from the Assemblers and intended to be immutable outside of the
     * assembler.
     */
    const std::map<unsigned, QString>& toMap() const {
        if (_mapcache.size() != size()) {
            _mapcache.clear();
            std::map<unsigned, QString> m;
            for (const auto& iter : *this) {
                _mapcache[iter.first] = iter.second;
            }
        }
        return _mapcache;
    }

private:
    mutable std::map<unsigned, QString> _mapcache;
};

}  // namespace Assembler
}  // namespace Ripes
