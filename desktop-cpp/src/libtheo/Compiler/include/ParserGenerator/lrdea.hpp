#ifndef __LIBTHEO_C_PARSER_GENERATOR_LRDEA_HPP_
#define __LIBTHEO_C_PARSER_GENERATOR_LRDEA_HPP_

#include <set>

#include "Compiler/include/ParserGenerator/grammar.hpp"

namespace Theo {
/* an LR(1) Element */
struct LRElement {
  // reference to production
  Grammar::Symbol left;
  unsigned int alternative;
  /* parse location (index of the right-side
     element before which the dot is located) */
  unsigned int dot;
  // lookahead (must be a terminal)
  Grammar::Symbol follow;
};

bool operator<(const LRElement& l1, const LRElement& l2);

/**
 * Constructs a State of the LR(1) prefix DEA;
 * Analogous to the epsilon-hull of the part-set construction;
 * @param I set of LRElements (core elements) to build the hull
 * @param G grammar that the LRElements reference
 * @return  the epsilon-hull of LRElements
 */
std::set<LRElement> hull(std::set<LRElement> I, Grammar& G);

/**
 * Returns the state I transitions to with input X;
 * The state transition function of the prefix DEA;
 * @param I input state
 * @param X input symbol
 * @param G grammar that the LRElements reference
 * @return  delta(I, X)
 */
std::set<LRElement> jump(std::set<LRElement> I, Grammar::Symbol X, Grammar& G);

struct LRState {
  std::set<LRElement> elements;
  std::map<Grammar::Symbol, int> jump;  // int is index into elements()
};

bool operator<(const LRState& l1, const LRState& l2);

/**
 * construction of the sets-of-elements, i.e. the states of
 * the lr prefix dea
 * @param S   starting symbol
 * @param eof the symbol serving as eof or '$'
 * @param G   grammar
 * @return    the states of the dea, 0 is the starting state
 */
std::vector<LRState> elements(Grammar::Symbol S, Grammar::Symbol eof,
                              Grammar& G);
};  // namespace Theo

#endif
