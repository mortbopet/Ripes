#pragma once

#include "directive.h"

namespace Ripes {
namespace Assembler {

/** @brief
 * A collection of standard GNU assembler directives
 */
/**
 * @brief gnuDirectives
 * @returns a collection of all of the available directives
 */
DirectiveVec gnuDirectives();

Directive zeroDirective();
Directive stringDirective();
Directive ascizDirective();

Directive wordDirective();
Directive halfDirective();
Directive shortDirective();
Directive byteDirective();
Directive twoByteDirective();
Directive fourByteDirective();
Directive longDirective();

Directive textDirective();
Directive dataDirective();
Directive bssDirective();

}  // namespace Assembler
}  // namespace Ripes
