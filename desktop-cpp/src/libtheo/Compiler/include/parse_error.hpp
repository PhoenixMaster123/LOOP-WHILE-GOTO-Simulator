#ifndef __LIBTHEO_C_PARSE_ERROR_HPP_
#define __LIBTHEO_C_PARSE_ERROR_HPP_

#include <string>

namespace Theo {

struct ParseError {
  enum Type {
    MAIN_FILE_NOT_FOUND,
    EXPECTED_FILENAME,
    FILE_NOT_FOUND,
    RECURSIVE_INCLUDE,
    MACRO_EXTRACT_EXPECT,
    MACRO_EXTRACT_NESTED,
    MACRO_EXTRACT_EMPTY_DEFINE,
    MACRO_COMPILE_NON_LR,
    MACRO_APPLY_REACHED_MAX_PASSES,
    UNKNOWN_TOKEN
  };
  Type t;
  std::string msg;
  std::string file;
  int line;
  std::string file_request;
};

};  // namespace Theo

#endif
