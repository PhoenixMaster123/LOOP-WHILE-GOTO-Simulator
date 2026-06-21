#include <iostream>
#include <vector>

#include "Compiler/include/ParserGenerator/lrdea.hpp"

using namespace Theo;

void print_set(std::set<LRElement> s, std::ostream &o) {
  for (auto &_s : s) {
    o << _s.left.index << " " << _s.alternative << " " << _s.dot << " "
      << _s.follow.index << std::endl;
  }
}

void print_jumps(LRState &s, std::ostream &o) {
  for (auto &p : s.jump) {
    o << p.first.t << ":" << p.first.index << " -> " << p.second << std::endl;
  }
}

int main() {
  SemanticGrammar<int> sg = SemanticGrammar<int>();

  Grammar::Symbol S = sg.createNonTerminal(), C = sg.createNonTerminal(),
                  eof = Grammar::Symbol::Terminal(0),
                  c = Grammar::Symbol::Terminal(1),
                  d = Grammar::Symbol::Terminal(2);

  auto _ = [](std::vector<int>) -> int { return 0; };

  sg.add(S >> (C, C), _);
  sg.add(C >> (c, C), _);
  sg.add(C >> d, _);

  std::vector<LRState> states = elements(S, eof, sg);

  if (states.size() != 10) {
    std::cerr << "expected 10 states, got " << states.size() << std::endl;
    for (unsigned int i = 0; i < states.size(); i++) {
      std::cerr << " State " << i << ": " << std::endl;
      print_set(states[i].elements, std::cerr);
      print_jumps(states[i], std::cerr);
    }
    return 1;
  }

  bool err = false;

  // assumption: second-to last created non-terminal (by elements()) is S'
  Grammar::Symbol S_prime = {Grammar::Symbol::NON_TERMINAL,
                             sg.total_non_terminals - 2};

  // the canonical sets of LR(1) elements (Compilerbau Teil 1, Abb. 4.39)
  std::vector<LRState> expect = {
      // I_0
      {{
           {S_prime, 0, 0, eof},
           {S, 0, 0, eof},
           {C, 0, 0, c},
           {C, 0, 0, d},
           {C, 1, 0, c},
           {C, 1, 0, d},
       },
       {{S, 1}, {C, 2}, {c, 3}, {d, 4}}},
      // I_1
      {{{S_prime, 0, 1, eof}}, {}},
      // I_2
      {{{S, 0, 1, eof}, {C, 0, 0, eof}, {C, 1, 0, eof}},
       {{C, 5}, {c, 6}, {d, 7}}},
      // I_3
      {{
           {C, 0, 1, c},
           {C, 0, 0, c},
           {C, 1, 0, c},
           {C, 0, 1, d},
           {C, 0, 0, d},
           {C, 1, 0, d},
       },
       {{C, 8}, {c, 3}, {d, 4}}},
      // I_4
      {{
           {C, 1, 1, c},
           {C, 1, 1, d},
       },
       {}},
      // I_5
      {{{S, 0, 2, eof}}, {}},
      // I_6
      {{{C, 0, 1, eof}, {C, 0, 0, eof}, {C, 1, 0, eof}},
       {{c, 6}, {d, 7}, {C, 9}}},
      // I_7
      {{{C, 1, 1, eof}}, {}},
      // I_8
      {{{C, 0, 2, c}, {C, 0, 2, d}}, {}},
      // I_9
      {{{C, 0, 2, eof}}, {}}};

  std::vector<LRState> ordered_states = {
      // result so that indices match up with book
      states[0],
      states[states[0].jump[S]],
      states[states[0].jump[C]],
      states[states[0].jump[c]],
      states[states[0].jump[d]],
      states[states[states[0].jump[C]].jump[C]],
      states[states[states[0].jump[C]].jump[c]],
      states[states[states[0].jump[C]].jump[d]],
      states[states[states[0].jump[c]].jump[C]],
      states[states[states[states[0].jump[C]].jump[c]].jump[C]],
  };

  for (unsigned int i = 0; i < expect.size(); i++) {
    LRState got = ordered_states[i];
    LRState exp = expect[i];

    bool local_err = false;
    for (auto &c : exp.elements)
      if (!got.elements.contains(c)) local_err = true;
    for (auto &c : exp.jump)
      if (!got.jump.contains(c.first) ||
          // check that elements of jump target are the same
          states[got.jump[c.first]].elements <
              expect[exp.jump[c.first]].elements ||
          expect[exp.jump[c.first]].elements <
              states[got.jump[c.first]].elements)
        local_err = true;

    if (local_err || got.elements.size() != exp.elements.size() ||
        got.jump.size() != exp.jump.size()) {
      err = true;
      std::cerr << "# STATE " << i << " is wrong, expected: " << std::endl;
      print_set(exp.elements, std::cerr);
      std::cerr << "but got: " << std::endl;
      print_set(got.elements, std::cerr);
    }
  }

  return err ? 1 : 0;
}
