#include <iostream>
#include <set>

#include "Compiler/include/ParserGenerator/lrdea.hpp"

using namespace Theo;

int main() {
  SemanticGrammar<int> sg = SemanticGrammar<int>();

  Grammar::Symbol S_prime = sg.createNonTerminal(), S = sg.createNonTerminal(),
                  C = sg.createNonTerminal(), DUMMY = sg.createNonTerminal(),
                  c = Grammar::Symbol::Terminal(1),
                  d = Grammar::Symbol::Terminal(2),
                  dollar = Grammar::Symbol::Terminal(0);

  auto _ = [](std::vector<int>) -> int { return 0; };

  /**
   * (Beispiel 4.42 aus Compilerbau Teil 1, 2. Auflage, S.283)
   * S' -> S
   * S  -> CC
   * C  -> cC | d
   */
  sg.add(S_prime >> S, _);
  sg.add(S >> (C, C), _);
  sg.add(C >> (c, C), _);
  sg.add(C >> d, _);
  sg.add(DUMMY >> dollar,
         _);  // so .calculateFirstSets() will calculate a first set for $

  sg.calculateFirstSets();
  /**
   * the Hull of {[S' -> . S, $]} should be:
   * {
   *   [S' -> . S   , $],
   *   [S  -> . C C , $],
   *   [C  -> . c C , c],
   *   [C  -> . c C , d],
   *   [C  -> . d   , c],
   *   [C  -> . d   , d]
   * }
   */
  std::set<LRElement> expect = {{S_prime, 0, 0, dollar},
                                {S, 0, 0, dollar},
                                {C, 0, 0, c},
                                {C, 0, 0, d},
                                {C, 1, 0, c},
                                {C, 1, 0, d}};

  LRElement start = {// [S' -> . S , $]
                     S_prime, 0, 0, dollar};

  std::set<LRElement> result = hull({start}, sg);
  bool err = false;

  for (auto &e : expect)
    if (!result.contains(e)) err = true;

  if (result.size() != 6 || err) {
    err = true;
    std::cerr << "expected result to be: " << std::endl;
    for (auto &e : expect)
      std::cerr << e.left.index << " " << e.alternative << " " << e.dot << " "
                << e.follow.index << std::endl;
    std::cerr << "but got: " << std::endl;
    for (auto &e : result)
      std::cerr << e.left.index << " " << e.alternative << " " << e.dot << " "
                << e.follow.index << std::endl;
  }

  return err ? 1 : 0;
}
