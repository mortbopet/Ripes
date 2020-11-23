#pragma once

#include <QString>
#include <iostream>
#include <vector>

namespace Ripes {
namespace AssemblerTmp {

// An error is defined as a reference to a source line index + an error string
using Error = std::pair<unsigned, QString>;
struct Errors : public std::vector<Error> {
    void print() {
        for (const auto& iter : *this) {
            std::cout << "line " << iter.first << ": " << iter.second.toStdString() << "\n";
        }
        std::cout << std::flush;
    }
};

}  // namespace AssemblerTmp
}  // namespace Ripes
