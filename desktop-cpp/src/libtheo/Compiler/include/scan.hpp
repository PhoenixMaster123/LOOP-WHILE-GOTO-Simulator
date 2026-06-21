#ifndef __LIBTHEO_C_SCAN_HPP_
#define __LIBTHEO_C_SCAN_HPP_

#include <map>
#include <string>
#include <vector>

#include "Compiler/include/parse_error.hpp"
#include "Compiler/include/token.hpp"

namespace Theo {
typedef std::string FileName, FileContent;

struct ScanResult {
  std::vector<Theo::Token> toks;
  std::vector<ParseError> errors;
};

/**
 * get a verbal description for a token type
 * (useful for debugging purposes)
 */
std::string token_string(Theo::Token::Type t);

/**
 * convert multiple files to one token stream;
 * @param files input files
 * @param main  key of main file
 */
ScanResult scan(std::map<FileName, FileContent> files, FileName main);
};  // namespace Theo

#endif
