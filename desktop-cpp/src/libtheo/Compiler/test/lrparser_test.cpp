#include <iostream>

#include "Compiler/include/ParserGenerator/lrparser.hpp"

using namespace Theo;

int test(bool accept_prefix) {
  /*
   * A small calculator for expressions with +, *, single character digits, (
   * and ) no spaces!
   */

  SemanticGrammar<int> sg = SemanticGrammar<int>();

  auto E_prime = sg.createNonTerminal(), E = sg.createNonTerminal(),
       T = sg.createNonTerminal(), F = sg.createNonTerminal(),
       plus = Grammar::Symbol::Terminal(0),
       times = Grammar::Symbol::Terminal(1),
       p_open = Grammar::Symbol::Terminal(2),
       p_close = Grammar::Symbol::Terminal(3),
       digit = Grammar::Symbol::Terminal(4), end = Grammar::Symbol::Terminal(5),
       eof = Grammar::Symbol::Terminal(6);

  if (!accept_prefix)
    sg.add(E_prime >> (E), [](std::vector<int> i) -> int { return i[0]; });
  else
    // if we accept inputs that start with words of <sg>, we need an explicit
    // non-repeating end sequence
    sg.add(E_prime >> (E, end), [](std::vector<int> i) -> int { return i[1]; });
  sg.add(E >> (E, plus, T),
         [](std::vector<int> i) -> int { return i[0] + i[2]; });   // E -> E + T
  sg.add(E >> T, [](std::vector<int> i) -> int { return i[0]; });  // E -> T
  sg.add(T >> (T, times, F),
         [](std::vector<int> i) -> int { return i[0] * i[2]; });   // T -> T * F
  sg.add(T >> F, [](std::vector<int> i) -> int { return i[0]; });  // T -> F
  sg.add(F >> (p_open, E, p_close),
         [](std::vector<int> i) -> int { return i[1]; });  // F -> (E)
  sg.add(F >> digit, [](std::vector<int> i) -> int {
    return i[0];
  });  // F -> 0 | 1 | 2 | ... | 9

  auto translator = [=](char c) -> Grammar::Symbol {
    switch (c) {
      case '+':
        return plus;
      case '*':
        return times;
      case '(':
        return p_open;
      case ')':
        return p_close;
      default:
        if (accept_prefix && c == '$') return end;
        if (c >= '0' && c <= '9') return digit;
        return eof;
    };
  };

  auto creator = [](char c) -> int {
    if (c >= '0' && c <= '9') return c - '0';
    return -1000000;
  };

  LRParser<int, char> p =
      LRParser<int, char>(sg, accept_prefix, translator, creator, E_prime, eof);
  auto rp = p.generateParseTables();
  for (auto &r : rp)
    std::cerr << "generation of parse tables reported an error " << r.t << ": "
              << std::endl
              << r.msg << std::endl;
  if (rp.size() != 0) return 1;

  // there needs to be an explicit EOF symbol in the stream!!
  auto pr = p.parse(std::string("2+2*2$"));

  if (pr.t != pr.ACCEPT) {
    std::cerr << "Parser rejected 2+2*2: " << pr.msg << std::endl;
    return 1;
  }

  if (pr.st != 6) {
    std::cerr << "Your idiotic parser thinks 2+2*2 = " << pr.st << std::endl;
    return 1;
  }

  pr = p.parse(std::string("(2+2)*2$"));
  if (pr.t != pr.ACCEPT) {
    std::cerr << "Parser rejected (2+2)*2: " << pr.msg << std::endl;
    return 1;
  }

  if (pr.st != 8) {
    std::cerr << "Your idiotic parser thinks (2+2)*2 = " << pr.st << std::endl;
    return 1;
  }

  return 0;
}

int main() {
  if (test(false)) {
    std::cerr << "non-prefix test failed" << std::endl;
    return 1;
  }
  if (test(true)) {
    std::cerr << "prefix acceptance test failed" << std::endl;
    return 1;
  }
  return 0;
}
