#include <algorithm>

#include "Compiler/include/ParserGenerator/grammar.hpp"

using namespace Theo;

bool Theo::operator<(const Grammar::Symbol &g1, const Grammar::Symbol &g2) {
  if (g1.t < g2.t) return true;
  if (g1.t > g2.t) return false;
  return g1.index < g2.index;
}

std::vector<Grammar::Symbol> Theo::operator,(const Grammar::Symbol left,
                                             const Grammar::Symbol right) {
  return {left, right};
}

std::vector<Grammar::Symbol> Theo::operator,(std::vector<Grammar::Symbol> left,
                                             const Grammar::Symbol right) {
  left.push_back(right);
  return left;
}

std::pair<Grammar::Symbol, std::vector<Grammar::Symbol>> Theo::operator>>(
    const Grammar::Symbol left, const std::vector<Grammar::Symbol> right) {
  return std::make_pair(left, right);
}

std::pair<Grammar::Symbol, Grammar::Alternative> Theo::operator>>(
    const Grammar::Symbol left, const Grammar::Symbol right) {
  return std::make_pair(left, Grammar::Alternative{right});
}

Grammar::Symbol Grammar::createNonTerminal() {
  Symbol s = {Symbol::NON_TERMINAL, total_non_terminals};
  total_non_terminals++;
  return s;
}

Grammar::Symbol Theo::Grammar::Symbol::Epsilon() {
  return {Grammar::Symbol::EPSILON, 0};
}

Grammar::Symbol Theo::Grammar::Symbol::Terminal(unsigned int index) {
  return {Grammar::Symbol::TERMINAL, index};
}

void add_terminals(std::set<Grammar::Symbol> &to,
                   std::set<Grammar::Symbol> &from) {
  for (auto &f : from) {
    if (f.t != Grammar::Symbol::TERMINAL) continue;
    to.insert(f);
  }
}

void Grammar::calculateFirstSets() {
  bool changed = true;
  first_sets.insert(std::make_pair(Symbol::Epsilon(), std::set<Symbol>{}));
  max_used_terminal = 0;
  while (changed) {  // repeat until nothing changes:
    changed = false;
    // 1) if X is a terminal, FIRST(X) = {X}
    // 2) if X -> ø exists, FIRST(X) = FIRST(X) v {ø}
    for (auto &rule : right_sides) {
      Symbol left = rule.first;
      for (auto &alternative : rule.second) {
        if (alternative.size() ==
            0) {  // 2 -> empty right side is implicit epsilon
          if (!first_sets.contains(left)) {
            changed = true;
            // epsilons in the first sets are now explicitly managed
            first_sets.insert(
                std::make_pair(left, std::set<Symbol>{Symbol::Epsilon()}));
          } else if (!first_sets[left].contains(Symbol::Epsilon())) {
            changed = true;
            first_sets[left].insert(Symbol::Epsilon());
          }
        }
        for (auto &symbol : alternative) {
          if (symbol.t != Symbol::TERMINAL) {  // 1 -> symbol is a terminal
                                               // appearing within the grammar
            if (symbol.t == Symbol::NON_TERMINAL &&
                !first_sets.contains(symbol))
              first_sets.insert(std::make_pair(symbol, std::set<Symbol>{}));
            continue;
          }
          max_used_terminal = std::max(symbol.index, max_used_terminal);
          if (!first_sets.contains(symbol)) {
            changed = true;
            first_sets.insert(std::make_pair(symbol, std::set<Symbol>{symbol}));
          }
        }
      }
    }
    // 3) X -> Y_1Y_2Y_3 ..., FIRST(X) = FIRST(Y_1), if ø in FIRST(Y_1), then
    // FIRST(X) = FIRST(X) v FIRST(Y_2) ...
    for (auto &rule : right_sides) {
      Symbol left = rule.first;
      if (!first_sets.contains(left))
        first_sets.insert(std::make_pair(left, std::set<Symbol>{}));
      for (auto &alternative : rule.second) {
        bool all_contain_epsilons = true;
        for (auto &symbol : alternative) {
          int size_before = first_sets[left].size();
          std::set<Symbol> &sset = first_sets[symbol];
          add_terminals(first_sets[left], sset);
          if (first_sets[left].size() > size_before) changed = true;
          if (!sset.contains(Symbol::Epsilon())) {
            all_contain_epsilons = false;
            break;
          }
        }
        if (all_contain_epsilons &&
            !first_sets[left].contains(Symbol::Epsilon())) {
          changed = true;
          first_sets[left].insert(Symbol::Epsilon());
        }
      }
    }
  }
}

std::set<Grammar::Symbol> Grammar::first(std::vector<Grammar::Symbol> string) {
  std::set<Symbol> result = {};
  bool all_epsilons = true;
  for (auto &sym : string) {
    add_terminals(result, first_sets[sym]);
    if (!first_sets[sym].contains(Symbol::Epsilon())) {
      all_epsilons = false;
      break;
    }
  }
  if (all_epsilons) result.insert(Symbol::Epsilon());
  return result;
}
