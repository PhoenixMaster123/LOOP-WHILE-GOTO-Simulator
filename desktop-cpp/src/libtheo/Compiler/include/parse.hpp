#ifndef __LIBTHEO_C_PARSE_HPP_
#define __LIBTHEO_C_PARSE_HPP_

#include <map>

#include "Compiler/include/ast.hpp"
#include "Compiler/include/scan.hpp"

#define THEO_MACRO_PASSES 1024

namespace Theo {

struct ParseResult {
  std::vector<std::string> missing_files;
  AST a;
};

/**
 * parse a number of strings;
 */
ParseResult parse(std::map<FileName, FileContent> files, FileName main);

};  // namespace Theo

#endif
