#include <algorithm>

#include "Compiler/include/macro.hpp"
#include "Compiler/include/scan.hpp"

int main() {
  std::string included_theo =
      "\
DEFINE\n\
  <V> + <V>\n\
AS\n\
  RUN add WITH $1, $2 END\n\
END DEFINE";

  // <P> at the end should cause accept/shift conflicts (do we accept the
  // current prefix, or dare to push another ';'?)
  std::string main_theo =
      "\
include \"included.theo\"\n\
DEFINE\n\
  <P> BUT FIRST <P>\n\
AS\n\
    $2 ; $1\n\
END DEFINE";

  Theo::ScanResult sr =
      Theo::scan({{"main.theo", main_theo}, {"included.theo", included_theo}},
                 "main.theo");

  auto printer = [](Theo::ParseError e) -> void {
    std::cerr << e.t << " " << e.file << " " << e.line << " " << e.msg
              << std::endl;
  };

  if (sr.errors.size() != 0) {
    std::cerr << "scanning failed" << std::endl;
    std::for_each(sr.errors.begin(), sr.errors.end(), printer);
    return 1;
  }

  Theo::MacroExtractionResult mer = Theo::extract_macros(sr.toks);

  if (mer.macros.size() != 2) {
    std::cerr << "expected to extract 2 macros, but got " << mer.macros.size()
              << std::endl;
    return 1;
  }

  if (mer.errors.size() != 0) {
    std::cerr << "macro extraction failed" << std::endl;
    std::for_each(mer.errors.begin(), mer.errors.end(), printer);
    return 1;
  }

  Theo::MacroApplicationResult mar =
      Theo::apply_macros(mer.tokens, mer.macros, 100);

  if (mar.errors.size() == 0) {
    std::cerr << "expected at least  macro compilation error, but got nothing "
              << std::endl;
    return 1;
  }

  bool err = false;
  for (auto &e : mar.errors) {
    if (e.file != "main.theo" || e.line != 3) {
      std::cerr << "expected error in main.theo:2, but got: " << std::endl;
      printer(e);
      err = true;
    }
  }
  return err ? 1 : 0;
}
