#include <set>
#include <vector>

#include "Compiler/include/ParserGenerator/lrdea.hpp"

using namespace Theo;

bool Theo::operator<(const LRElement &l1, const LRElement &l2) {
  if (l1.left < l2.left) return true;
  if (l2.left < l1.left) return false;
  if (l1.alternative < l2.alternative) return true;
  if (l2.alternative < l1.alternative) return false;
  if (l1.dot < l2.dot) return true;
  if (l2.dot < l1.dot) return false;
  return l1.follow < l2.follow;
}

static Grammar::Alternative fetch_right(const LRElement &e, Grammar &G) {
  return G.right_sides[e.left][e.alternative];
}

static Grammar::Symbol get_before(Grammar::Alternative &a, const LRElement &e) {
  if (a.size() == 0 || e.dot >= a.size()) return Grammar::Symbol::Epsilon();
  return a[e.dot];
}

#include <iostream>
static std::vector<Grammar::Symbol> get_follow_string(Grammar::Alternative &a,
                                                      const LRElement &e) {
  std::vector<Grammar::Symbol> res = {};
  for (unsigned int i = e.dot + 1; i < a.size(); i++) {
    res.push_back(a[i]);
  }
  res.push_back(e.follow);
  return res;
}

std::set<LRElement> Theo::hull(std::set<LRElement> I, Grammar &G) {
  bool changed = true;
  while (changed) {
    changed = false;
    for (const LRElement &elem : I) {
      Grammar::Alternative a = fetch_right(elem, G);
      Grammar::Symbol expecting = get_before(a, elem);

      if (expecting.t != Grammar::Symbol::NON_TERMINAL) continue;

      std::set<Grammar::Symbol> lookaheads =
          G.first(get_follow_string(a, elem));

      std::vector<Grammar::Alternative> &Bs = G.right_sides[expecting];

      for (unsigned int right_index = 0; right_index < Bs.size();
           right_index++) {
        for (const Grammar::Symbol &la : lookaheads) {
          LRElement new_elem = {expecting, right_index, 0, la};

          if (!I.contains(new_elem)) {
            changed = true;
            I.insert(new_elem);
            break;
          }
        }
        if (changed) break;
      }
      if (changed) break;
    }
  }
  return I;
}

std::set<LRElement> Theo::jump(std::set<LRElement> I, Grammar::Symbol X,
                               Grammar &G) {
  std::set<LRElement> J = {};

  for (auto &i : I) {
    Grammar::Alternative right = fetch_right(i, G);
    Grammar::Symbol expecting = get_before(right, i);
    if (!(expecting < X || X < expecting)) {  // expecting == X
      LRElement next = i;
      next.dot++;
      J.insert(next);
    }
  }

  return hull(J, G);
}

bool Theo::operator<(const LRState &l1, const LRState &l2) {
  return l1.elements < l2.elements;
}

std::set<Grammar::Symbol> get_befores(std::set<LRElement> I, Grammar &G) {
  std::set<Grammar::Symbol> res = {};
  for (const LRElement &s : I) {
    Grammar::Alternative a = fetch_right(s, G);
    Grammar::Symbol before = get_before(a, s);
    if (before.t == Grammar::Symbol::EPSILON) continue;
    res.insert(before);
  }
  return res;
}

std::vector<LRState> Theo::elements(Grammar::Symbol S, Grammar::Symbol eof,
                                    Grammar &G) {
  std::vector<LRState> result = {};

  // insert S' -> S into grammar
  auto S_prime = G.createNonTerminal();
  G.right_sides.insert(
      std::make_pair(S_prime, std::vector<Grammar::Alternative>{}));
  G.right_sides[S_prime].push_back(Grammar::Alternative{S});

  // make sure that FIRST(EOF) is calculated
  auto E = G.createNonTerminal();
  G.right_sides.insert(std::make_pair(E, std::vector<Grammar::Alternative>{}));
  G.right_sides[E].push_back(Grammar::Alternative{eof});

  G.calculateFirstSets();

  // initial state is Hull({[S' -> . S, $]})
  std::set<LRElement> h = hull(std::set<LRElement>{{S_prime, 0, 0, eof}}, G);
  LRState init = {h, {}};
  result.push_back(init);
  std::map<LRState, int> registry = {{init, 0}};

  for (unsigned int i = 0; i < result.size(); i++) {
    // fetch jump characters
    std::set<Grammar::Symbol> possible_transitions =
        get_befores(result[i].elements, G);
    // construct jumps
    for (const Grammar::Symbol &t : possible_transitions) {
      std::set<LRElement> r = jump(result[i].elements, t, G);
      LRState candidate = {r, {}};
      if (!registry.contains(candidate)) {
        result.push_back(candidate);
        registry.insert(std::make_pair(candidate, result.size() - 1));
      }
      if (!result[i].jump.contains(t)) {
        result[i].jump.insert(std::make_pair(t, registry[candidate]));
      }
    }
  }

  return result;
}
