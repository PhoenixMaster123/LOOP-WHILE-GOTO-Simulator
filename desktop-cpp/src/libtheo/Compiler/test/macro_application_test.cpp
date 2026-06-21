#include <iostream>
#include <ostream>

#include "Compiler/include/macro.hpp"
#include "Compiler/include/scan.hpp"

int main() {
  std::string included_theo =
      "\
DEFINE PRIORITY 10\n\
    <V> + <V>\n\
AS\n\
    add($0, $1)\n\
END DEFINE\n\
DEFINE PRIORITY 30\n\
   <ID>(<ARGS>)\n\
AS\n\
   RUN $0 WITH $1 END\n\
END DEFINE\n\
  ";
  std::string main_theo =
      "\
include \"included.theo\"\n\
DEFINE PRIORITY 20\n\
    <V> * <V>\n\
AS\n\
   mul($0, $1)\n\
END DEFINE\n\
DEFINE PRIORITY 5\n\
   (<V>)\n\
AS\n\
   $0\n\
END DEFINE\n\
((1+2)*3)*4\n\
\n";
  Theo::ScanResult sr =
      Theo::scan({{"main.theo", main_theo}, {"included.theo", included_theo}},
                 "main.theo");

  auto pprinter = [](const std::vector<Theo::ParseError>& vpe) -> void {
    for (auto& e : vpe)
      std::cerr << e.file << ":" << e.line << " " << e.msg << " (" << e.t << ")"
                << std::endl;
  };

  if (sr.errors.size() != 0) {
    std::cerr << "scan errors: " << std::endl;
    pprinter(sr.errors);
    return 1;
  }

  Theo::MacroExtractionResult mer = Theo::extract_macros(sr.toks);

  if (mer.errors.size() != 0) {
    std::cerr << "macro extraction errors: " << std::endl;
    pprinter(mer.errors);
    return 1;
  }

  Theo::MacroApplicationResult mar =
      Theo::apply_macros(mer.tokens, mer.macros, 100);

  if (mar.errors.size() != 0) {
    std::cerr << "macro application errors: " << std::endl;
    pprinter(mar.errors);
    return 1;
  }

  // RUN mul WITH RUN mul WITH RUN add WITH 1, 2 END, 3 END, 4 END
  Theo::Token run = {Theo::Token::RUN, "RUN", "", 0};
  Theo::Token with = {Theo::Token::WITH, "WITH", "", 0};
  Theo::Token end = {Theo::Token::END, "END", "", 0};
  Theo::Token coma = {Theo::Token::ARGSEP, ",", "", 0};
  Theo::Token mul = {Theo::Token::ID, "mul", "", 0};
  Theo::Token add = {Theo::Token::ID, "add", "", 0};
  Theo::Token one = {Theo::Token::INT, "1", "", 0};
  Theo::Token two = {Theo::Token::INT, "2", "", 0};
  Theo::Token tre = {Theo::Token::INT, "3", "", 0};
  Theo::Token four = {Theo::Token::INT, "4", "", 0};
  Theo::Token eof = mar.transformed_sequence.back();
  std::vector<Theo::Token> expect = {run, mul,  with, run,  mul, with, run,
                                     add, with, one,  coma, two, end,  coma,
                                     tre, end,  coma, four, end, eof};

  if (mar.transformed_sequence.size() != expect.size()) {
    std::cerr << "expected " << expect.size() << " tokens in output but got "
              << mar.transformed_sequence.size() << std::endl;
    std::cerr << Theo::recover_from_tokens(mar.transformed_sequence)
              << std::endl;
    return 1;
  }

  for (auto i = 0; i < expect.size(); i++) {
    if (expect[i].t != mar.transformed_sequence[i].t ||
        expect[i].text != mar.transformed_sequence[i].text) {
      std::cerr << "Token " << i << " mismatch: " << std::endl
                << expect[i].t << " " << mar.transformed_sequence[i].t
                << std::endl
                << expect[i].text << " " << mar.transformed_sequence[i].text
                << std::endl;
      std::cerr << Theo::recover_from_tokens(mar.transformed_sequence)
                << std::endl;
      return 1;
    }
  }

  return 0;
}
