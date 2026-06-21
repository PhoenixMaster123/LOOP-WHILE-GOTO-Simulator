#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "Compiler/include/macro.hpp"
#include "Compiler/include/scan.hpp"

using namespace Theo;

int main() {
  std::string included_theo =
      "\
DEFINE \n\
  <V> + <V>\n\
AS add($1, $2) END DEFINE\n\
1 +2 +3 +4\n\
";

  std::string main_theo =
      "\
INCLUDE \"included.theo\"\n\
DEFINE PRIORITY 10\n\
  <V> <V> * \n\
AS mul($1, $2) END DEFINE\n\
DEFINE AS illegal END DEFINE \n\
DEFINE DEFINE AS illegal2 illegal3 END DEFINE\n\
this is some nonsensical input\n\
DEFINE PRIORITY unfinished macro\n\
";

  bool error = false;
  ScanResult sr =
      Theo::scan({{"main.theo", main_theo}, {"included.theo", included_theo}},
                 "main.theo");

  if (sr.errors.size() != 0) {
    std::cerr << "errors during scanning, abort" << std::endl;
    return 1;
  }

  MacroExtractionResult mer = Theo::extract_macros(sr.toks);

  if (mer.macros.size() != 3) {
    std::cerr << "expected to find 3 macros, found " << mer.macros.size()
              << std::endl;
    error = true;
  } else {
    if (mer.macros[0].rule.size() != 3 ||
        mer.macros[0].content_constraint_token_indices[0] != 1 ||
        mer.macros[0].template_token_indices[0] != 0 ||
        mer.macros[0].template_token_indices[1] != 2) {
      std::cerr << "first macro of unexpected format:" << std::endl
                << 3 << " == " << mer.macros[0].rule.size() << std::endl
                << 1
                << " == " << mer.macros[0].content_constraint_token_indices[0]
                << std::endl
                << 0 << " == " << mer.macros[0].template_token_indices[0]
                << std::endl
                << 2 << " == " << mer.macros[0].template_token_indices[1]
                << std::endl;
      error = true;
    }
    if (mer.macros[1].rule.size() != 3 ||
        mer.macros[1].content_constraint_token_indices[0] != 2 ||
        mer.macros[1].template_token_indices[0] != 0 ||
        mer.macros[1].template_token_indices[1] != 1 ||
        mer.macros[1].priority != 10) {
      std::cerr << "second macro of unexpected format:" << std::endl
                << 3 << " == " << mer.macros[1].rule.size() << std::endl
                << 2
                << " == " << mer.macros[1].content_constraint_token_indices[0]
                << std::endl
                << 0 << " == " << mer.macros[1].template_token_indices[0]
                << std::endl
                << 1 << " == " << mer.macros[1].template_token_indices[1]
                << std::endl
                << 10 << " == " << mer.macros[1].priority << std::endl;
      error = true;
    }
  }

  if (mer.errors.size() != 5) {
    std::cerr << "expected to find five errors, but found " << mer.errors.size()
              << std::endl;
    for (ParseError pe : mer.errors) {
      std::cerr << pe.t << std::endl
                << pe.msg << std::endl
                << pe.file << std::endl
                << pe.line << std::endl;
    }
    error = true;
  } else {
    std::vector<ParseError> compare = {
        {Theo::ParseError::Type::MACRO_EXTRACT_EMPTY_DEFINE, "", "main.theo",
         5},
        {Theo::ParseError::Type::MACRO_EXTRACT_NESTED, "", "main.theo", 6},
        {Theo::ParseError::Type::MACRO_EXTRACT_EXPECT, "", "main.theo", 8},
        {Theo::ParseError::Type::MACRO_EXTRACT_EXPECT, "", "main.theo", 8},
        {Theo::ParseError::Type::MACRO_EXTRACT_EXPECT, "", "main.theo", 8}};

    for (auto i = 0; i < mer.errors.size(); i++) {
      ParseError pe = mer.errors[i];
      if (mer.errors[i].t != compare[i].t ||
          mer.errors[i].file != compare[i].file ||
          mer.errors[i].line != compare[i].line) {
        std::cerr << "Error " << i << " is " << std::endl;
        std::cerr << pe.t << std::endl
                  << pe.msg << std::endl
                  << pe.file << std::endl
                  << pe.line << std::endl;
        std::cerr << "but should be " << std::endl;
        pe = compare[i];
        std::cerr << pe.t << std::endl
                  << pe.msg << std::endl
                  << pe.file << std::endl
                  << pe.line << std::endl;
        error = true;
      }
    }
  }
  return error ? 1 : 0;
}
