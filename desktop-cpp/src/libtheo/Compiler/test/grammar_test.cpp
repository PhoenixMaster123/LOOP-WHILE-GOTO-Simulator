#include "Compiler/include/ParserGenerator/grammar.hpp"

/* testing the grammar rule syntax and the FIRST set construction*/

using namespace Theo;

#include <iostream>
int main() {
  SemanticGrammar<int> sg = SemanticGrammar<int>();

  auto E = sg.createNonTerminal(), T = sg.createNonTerminal(),
       F = sg.createNonTerminal(), X = sg.createNonTerminal(),
       Y = sg.createNonTerminal(), A = sg.createNonTerminal(),
       B = sg.createNonTerminal(), C = sg.createNonTerminal(),
       D = sg.createNonTerminal();

  auto plus = Grammar::Symbol::Terminal(0),
       times = Grammar::Symbol::Terminal(1),
       p_open = Grammar::Symbol::Terminal(2),
       p_close = Grammar::Symbol::Terminal(3),
       id = Grammar::Symbol::Terminal(4), a = Grammar::Symbol::Terminal(5),
       b = Grammar::Symbol::Terminal(6), c = Grammar::Symbol::Terminal(7),
       d = Grammar::Symbol::Terminal(8), e = Grammar::Symbol::Epsilon();

  auto nothing = [](std::vector<int> a) -> int {
    return 0;
  };  // do nothing action

  sg.add(X >> (A, B, C), nothing);     // X -> ABC
  sg.add(Y >> (A, B, C, D), nothing);  // Y -> ABCD
  sg.add(A >> a, nothing);             // A -> a
  sg.add(A >> e, nothing);             // A -> ø
  sg.add(B >> b, nothing);             // B -> b
  sg.add(B >> e, nothing);             // B -> ø
  sg.add(C >> c, nothing);             // C -> c
  sg.add(C >> e, nothing);             // C -> ø
  sg.add(D >> d, nothing);             // D -> d

  sg.add(E >> e, nothing);              // E -> ø; to have FIRST(E) contain +
  sg.add(E >> (E, plus, T), nothing);   // E -> E + T
  sg.add(E >> T, nothing);              // E -> T
  sg.add(T >> (T, times, F), nothing);  // T -> T * F
  sg.add(T >> F, nothing);              // T -> F
  sg.add(F >> (p_open, E, p_close), nothing);  // F -> ( E )
  sg.add(F >> id, nothing);                    // F -> id

  // Constructing the FIRST set of all rules (should be {(, id} in all cases)
  sg.calculateFirstSets();

  if (sg.first({plus}).size() != 1 || !sg.first({plus}).contains(plus)) {
    std::cerr << "first(+) does not have +" << std::endl;
    return 1;
  }
  if (sg.first({times}).size() != 1 || !sg.first({times}).contains(times)) {
    std::cerr << "first(*) does not have *" << std::endl;
    return 1;
  }
  if (sg.first({p_open}).size() != 1 || !sg.first({p_open}).contains(p_open)) {
    std::cerr << "first('(') does not have '('" << std::endl;
    return 1;
  }
  if (sg.first({p_close}).size() != 1 ||
      !sg.first({p_close}).contains(p_close)) {
    std::cerr << "first(')') does not have ')'" << std::endl;
    return 1;
  }
  if (sg.first({id}).size() != 1 || !sg.first({id}).contains(id)) {
    std::cerr << "first(id) does not have id" << std::endl;
    return 1;
  }

  // FIRST(E) = {ø, (, id, +}
  // FIRST(F) = FIRST(T) = {id, (}
  // FIRST(X) = {ø, a, b, c}
  // FIRST(Y) = {a, b, c, d}
  // FIRST(XY) = {a, b, c, d}

  auto _e = sg.first({E});
  if (_e.size() != 4 || !_e.contains({e}) || !_e.contains({p_open}) ||
      !_e.contains({id}) || !_e.contains({plus})) {
    std::cerr << "wrong FIRST(E): " << std::endl;
    for (auto s : _e) {
      std::cout << s.t << " " << s.index << std::endl;
    }
    return 1;
  }
  _e = sg.first({F});
  if (_e.size() != 2 || !_e.contains({id}) || !_e.contains({p_open})) {
    std::cerr << "wrong FIRST(F): " << std::endl;
    for (auto s : _e) {
      std::cout << s.t << " " << s.index << std::endl;
    }
    return 1;
  }
  _e = sg.first({T});
  if (_e.size() != 2 || !_e.contains({id}) || !_e.contains({p_open})) {
    std::cerr << "wrong FIRST(T): " << std::endl;
    for (auto s : _e) {
      std::cout << s.t << " " << s.index << std::endl;
    }
    return 1;
  }

  _e = sg.first({X});
  if (_e.size() != 4 || !_e.contains({a}) || !_e.contains({b}) ||
      !_e.contains({c}) || !_e.contains({e})) {
    std::cerr << "wrong FIRST(X): " << std::endl;
    for (auto s : _e) {
      std::cout << s.t << " " << s.index << std::endl;
    }
    return 1;
  }

  _e = sg.first({Y});
  if (_e.size() != 4 || !_e.contains({a}) || !_e.contains({b}) ||
      !_e.contains({c}) || !_e.contains({d})) {
    std::cerr << "wrong FIRST(Y): " << std::endl;
    for (auto s : _e) {
      std::cout << s.t << " " << s.index << std::endl;
    }
    return 1;
  }
  _e = sg.first({X, Y});
  if (_e.size() != 4 || !_e.contains({a}) || !_e.contains({b}) ||
      !_e.contains({c}) || !_e.contains({d})) {
    std::cerr << "wrong FIRST(XY): " << std::endl;
    for (auto s : _e) {
      std::cout << s.t << " " << s.index << std::endl;
    }
    return 1;
  }
  return 0;
}
