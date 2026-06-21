#ifndef __LIBTHEO_C_PARSERGENERATOR_LRPARSER_HPP_
#define __LIBTHEO_C_PARSERGENERATOR_LRPARSER_HPP_

#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "Compiler/include/ParserGenerator/grammar.hpp"
#include "Compiler/include/ParserGenerator/lrdea.hpp"

namespace Theo {

template <typename SemanticType, typename TokenType>
struct LRParser {
  struct GenerationResult {
    enum Type { SHIFT_REDUCE_ERR = 1, REDUCE_REDUCE_ERR = 2 };
    Type t;
    std::string msg;
  };

  struct ParseResult {
    enum Type { ACCEPT = 0, REJECT = 1 };
    Type t;
    std::string msg;
    SemanticType st;  // uninitialized when t != ACCEPT
  };

  using Translator = std::function<Grammar::Symbol(TokenType)>;
  using Creator = std::function<SemanticType(TokenType)>;

  /**
   * @param accept_prefix wether the parser accepts prefixes
   *                      of the input which are words in G
   * @param t             a function translating tokens to
   *                      terminal grammar symbols
   * @param c             a function creating semantic values
   *                      from tokens (leaf values of the parse tree)
   * @param S             starting symbol of G
   * @param eof           terminal symbol to be treated as '$'
   */
  LRParser(SemanticGrammar<SemanticType> G, bool accept_prefix, Translator t,
           Creator c, Grammar::Symbol S, Grammar::Symbol eof)
      : G(G),
        accept_prefix(accept_prefix),
        translator(t),
        creator(c),
        S(S),
        eof(eof) {};

  LRParser() {};

  /**
   * generate the parse tables from the Grammar;
   * needs to be called before any parsing takes place
   * @return wether or not G is LR(1), i.e. wether there are
   *         parse conflicts in the generated table
   *         empty vector means generation was successfull
   */
  std::vector<GenerationResult> generateParseTables();

  /**
   * the main parse function;
   * @param in      a iterable container with internal type TokenType
   */
  template <typename Iterable>
  ParseResult parse(Iterable in);

 private:
  bool accept_prefix;
  Translator translator;
  Creator creator;
  SemanticGrammar<SemanticType> G;
  Grammar::Symbol S;
  Grammar::Symbol eof;

  struct Action {
    enum Type { SHIFT, REDUCE, ACCEPT, ERR };
    Type t;

    // shift action
    int state;

    // reduce action
    int left;  // left side of the rule that is reduced, used as index into jump
               // table
    int beta;  // number of symbols popped of the value and state stacks
    std::function<SemanticType(std::vector<SemanticType>)> action;
  };

  std::vector<std::vector<Action>> action;  // action[state][terminal]
  std::vector<std::vector<int>> jump;       // jump[state][left_index]
};

/* definition of LRParse methods */
template <typename SemanticType, typename TokenType>
std::vector<typename LRParser<SemanticType, TokenType>::GenerationResult>
LRParser<SemanticType, TokenType>::generateParseTables() {
  std::vector<GenerationResult> res = {};
  std::vector<LRState> C = elements(S, eof, G);

  int tables_height = C.size();
  int action_width = G.max_used_terminal + 1;
  int jump_width = G.total_non_terminals;

  action = std::vector<std::vector<Action>>(
      tables_height, std::vector<Action>(action_width, {.t = Action::ERR}));
  jump = std::vector<std::vector<int>>(tables_height,
                                       std::vector<int>(jump_width, -1));

  auto place_shift = [&](int state, int terminal, int target) -> void {
    switch (action[state][terminal].t) {
      case Action::REDUCE: {
        res.push_back({GenerationResult::SHIFT_REDUCE_ERR,
                       "(ps) shift-reduce error in state " +
                           std::to_string(state) + " on terminal " +
                           std::to_string(terminal)});
        break;
      }
      default:
        action[state][terminal] = {.t = Action::SHIFT, .state = target};
    }
  };

  auto is_reduce_rule = [&](const LRElement &e) -> std::pair<bool, int> {
    Grammar::Alternative a = G.right_sides[e.left][e.alternative];
    int s = a.size();
    return std::make_pair(e.dot == s, s);
  };

  auto place_reduce = [&](int state, int terminal, const LRElement &left,
                          int size) -> void {
    switch (action[state][terminal].t) {
      case Action::REDUCE: {
        res.push_back({GenerationResult::REDUCE_REDUCE_ERR,
                       "(pr) reduce-reduce error in state " +
                           std::to_string(state) + " on terminal " +
                           std::to_string(terminal)});
        break;
      }
      case Action::SHIFT: {
        res.push_back({GenerationResult::SHIFT_REDUCE_ERR,
                       "(pr) shift-reduce error in state " +
                           std::to_string(state) + " on terminal " +
                           std::to_string(terminal)});
        break;
      }
      default:
        auto a = G.actions[left.left][left.alternative];
        action[state][terminal] = {.t = Action::REDUCE,
                                   .left = (int)left.left.index,
                                   .beta = size,
                                   .action = a};
    }
  };

  auto place_accept = [&](int state, int terminal) {
    switch (action[state][terminal].t) {
      case Action::REDUCE: {
        res.push_back({GenerationResult::REDUCE_REDUCE_ERR,
                       "accept-reduce error in state " + std::to_string(state) +
                           " on terminal " + std::to_string(terminal)});
        break;
      }
      case Action::SHIFT: {
        res.push_back({GenerationResult::SHIFT_REDUCE_ERR,
                       "accept-shift error in state " + std::to_string(state) +
                           " on terminal " + std::to_string(terminal)});
        break;
      }
      default:
        action[state][terminal] = {.t = Action::ACCEPT};
    }
  };

  for (unsigned int state = 0; state < C.size(); state++) {
    const LRState &lrs = C[state];
    for (auto &p : lrs.jump) {
      /* translate all terminal symbol jumps to shift actions*/
      if (p.first.t == Grammar::Symbol::TERMINAL) {
        place_shift(state, p.first.index, p.second);
      }
      /* translate all non-terminal symbol jumps to jump entries */
      if (p.first.t == Grammar::Symbol::NON_TERMINAL) {
        jump[state][p.first.index] = p.second;
      }
    }
    for (auto &e : lrs.elements) {
      auto check = is_reduce_rule(e);
      if (!check.first)  // dot is at leftern edge
        continue;
      if (e.follow.index == eof.index && accept_prefix) {
        for (int i = 0; i < action_width; i++)
          if (e.left.index == G.total_non_terminals - 2)  // A != S'
            place_accept(state, i);
          else
            place_reduce(state, i, e, check.second);
      } else {
        if (e.left.index == G.total_non_terminals - 2)  // A != S'
          place_accept(state, e.follow.index);
        else
          place_reduce(state, e.follow.index, e, check.second);
      }
    }
  }
  return res;
}

template <typename SemanticType, typename TokenType>
template <typename Iterable>
typename LRParser<SemanticType, TokenType>::ParseResult
LRParser<SemanticType, TokenType>::parse(Iterable in) {
  // Algorithmus 4.7, Compilerbau Teil 1
  auto ip = in.begin();
  std::vector<int> states = {0};
  std::vector<SemanticType> values = {};
  for (;;) {
    int s = states.back();
    int a = translator(*ip).index;
    if (action[s].size() <= a) return {ParseResult::REJECT, "not ok"};
    switch (action[s][a].t) {
      case Action::SHIFT: {
        int s_prime = action[s][a].state;
        states.push_back(s_prime);
        values.push_back(creator(*ip));
        ip++;
        break;
      }
      case Action::REDUCE: {
        int beta = action[s][a].beta;
        int left = action[s][a].left;
        std::vector<SemanticType> popped = {};
        for (int i = 0; i < beta; i++) {
          popped.push_back(values.back());
          values.pop_back();
          states.pop_back();
        }
        int s_prime = states.back();
        states.push_back(jump[s_prime][left]);
        values.push_back(action[s][a].action(popped));
        break;
      }
      case Action::ACCEPT: {
        return {ParseResult::ACCEPT, "ok", values.back()};
        break;
      }
      default:
        return {ParseResult::REJECT, "not ok"};
        break;
    }
  }
}

};  // namespace Theo

#endif
