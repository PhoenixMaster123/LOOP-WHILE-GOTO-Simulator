#include <sys/types.h>

#include <iostream>
#include <ostream>

#include "Compiler/include/scan.hpp"

int main() {
  std::string main_theo =
      "\
INCLUDE \"side1.theo\"\n\
INCLUDE \"side3.theo\"\n\
(\
";
  std::string side1_theo =
      "\
INCLUDE \"side2.theo\"\n\
LOOP\n";

  std::string side2_theo =
      "\
END\n\
";
  std::string side3_theo =
      "\
:=\n\
INCLUDE \"nothing\"\n\
";

  Theo::ScanResult res = Theo::scan({{"main.theo", main_theo},
                                     {"side1.theo", side1_theo},
                                     {"side2.theo", side2_theo},
                                     {"side3.theo", side3_theo}},
                                    "main.theo");

  bool err = false;
  if (res.errors.size() == 0) {
    std::cerr << "did not recognize missing file error" << std::endl;
    err = true;
  } else {
    if (res.errors[0].t != Theo::ParseError::FILE_NOT_FOUND ||
        res.errors[0].file_request != "nothing") {
      std::cerr << "reported wrong error " << res.errors[0].t << " "
                << res.errors[0].msg << std::endl;
      err = true;
    }
    if (res.errors[0].file != "side3.theo" || res.errors[0].line != 2) {
      std::cerr << "reported wrong error location " << res.errors[0].file << " "
                << res.errors[0].line << std::endl;
      err = true;
    }
  }

  //  END(side2, 1) LOOP(side1, 2) ASSIGN(side3, 1) PAREN_OPEN(main, 3)
  std::vector<Theo::Token> compare = {
      Theo::Token(Theo::Token::Type::END, "END", "side2.theo", 1),
      Theo::Token(Theo::Token::Type::LOOP, "LOOP", "side1.theo", 2),
      Theo::Token(Theo::Token::Type::ASSIGN, ":=", "side3.theo", 1),
      Theo::Token(Theo::Token::Type::PAREN_OPEN, "(", "main.theo", 3)};

  if (compare.size() + 1 != res.toks.size()) {
    std::cerr << "expected 5 tokens, got " << res.toks.size() << std::endl;
    err = true;
  } else
    for (auto i = 0; i < compare.size(); i++) {
      std::cout << compare[i].t << " " << res.toks[i].t << std::endl
                << compare[i].text << " " << res.toks[i].text << std::endl
                << compare[i].file << " " << res.toks[i].file << std::endl
                << compare[i].line << " " << res.toks[i].line << std::endl;

      if (compare[i].t != res.toks[i].t ||
          compare[i].text != res.toks[i].text ||
          compare[i].file != res.toks[i].file ||
          compare[i].line != res.toks[i].line) {
        std::cerr << "token mismatch" << std::endl;
        err = true;
      }
    }
  return err ? 1 : 0;
}
