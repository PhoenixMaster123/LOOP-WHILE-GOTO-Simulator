#ifndef __LIBTHEO_C_PARSER_GENERATOR_GRAMMAR_HPP_
#define __LIBTHEO_C_PARSER_GENERATOR_GRAMMAR_HPP_

#include <functional>
#include <map>
#include <set>
#include <utility>
#include <vector>
namespace Theo {

struct Grammar {
  struct Symbol {
    enum Type { EPSILON = 0, TERMINAL = 1, NON_TERMINAL = 2 };
    Type t;
    unsigned int index;

    static Symbol Epsilon();
    static Symbol Terminal(unsigned int index);
  };

  typedef std::vector<Symbol> Alternative;  // one alternative of a right side

  unsigned int total_non_terminals;
  std::map<Symbol, std::vector<Alternative>>
      right_sides;  // index is non_terminal, value is all the alternatives for
                    // that symbol
  std::map<Symbol, std::set<Symbol>> first_sets;
  unsigned int max_used_terminal;  // maximum .index value of a terminal symbol,
                                   // filled after first sets are calculated

  Grammar() : total_non_terminals(0), right_sides({}) {};

  /**
   * create a new ton-terminal symbol (a new left side)
   */
  Grammar::Symbol createNonTerminal();

  void calculateFirstSets();

  /**
   * compute the set of terminal symbols a string of grammar symbols might start
   * with;
   */
  std::set<Grammar::Symbol> first(std::vector<Grammar::Symbol> string);
};

bool operator<(const Grammar::Symbol& g1, const Grammar::Symbol& g2);

/* symbol concatenation */
Grammar::Alternative operator,(const Grammar::Symbol left,
                               const Grammar::Symbol right);
Grammar::Alternative operator,(Grammar::Alternative left,
                               const Grammar::Symbol right);

/* rule construction */
std::pair<Grammar::Symbol, Grammar::Alternative> operator>>(
    const Grammar::Symbol left, const Grammar::Alternative right);

std::pair<Grammar::Symbol, Grammar::Alternative> operator>>(
    const Grammar::Symbol left, const Grammar::Symbol right);

template <typename SemanticType>
struct SemanticGrammar : public Grammar {
  using SemanticAction = std::function<SemanticType(std::vector<SemanticType>)>;
  using RightSideAction = std::vector<SemanticAction>;
  std::map<Symbol, RightSideAction> actions;

  SemanticGrammar() : Grammar() {};

  void add(std::pair<Symbol, std::vector<Symbol>> rule, SemanticAction a) {
    if (rule.first.t != Symbol::NON_TERMINAL) return;
    // remove all epsilons from the rule, empty left sides are implicitly
    // interpreted as epsilon
    std::erase_if(rule.second, [](Symbol s) { return s.t == Symbol::EPSILON; });
    if (!right_sides.contains(rule.first)) {
      right_sides.insert(
          std::make_pair(rule.first, std::vector<Alternative>{rule.second}));
      actions.insert(
          std::make_pair(rule.first, std::vector<SemanticAction>{a}));
      return;
    }
    right_sides[rule.first].push_back(rule.second);
    actions[rule.first].push_back(a);
  }
};
};  // namespace Theo

#endif
