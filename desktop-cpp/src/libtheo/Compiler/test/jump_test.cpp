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
   * the Result of Jump(Hull({[S' -> . S, $]}), c) should be:
   * {
   *   [C  -> c . C   , c],
   *   [C  -> c . C   , d],
   *   [C  -> . c C   , c],
   *   [C  -> . c C   , d],
   *   [C  -> . d     , c],
   *   [C  -> . d     , d]
   * }
   */
  std::set<LRElement> expect = {{C, 0, 1, c}, {C, 0, 1, d}, {C, 0, 0, c},
                                {C, 0, 0, d}, {C, 1, 0, c}, {C, 1, 0, d}};

  LRElement start = {// [S' -> . S , $]
                     S_prime, 0, 0, dollar};

  std::set<LRElement> result = jump(hull({start}, sg), c, sg);
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
