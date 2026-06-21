#include <algorithm>

#include "Compiler/include/lexer.hpp"
#include "Compiler/include/macro.hpp"
#include "Compiler/include/parse.hpp"
#include "Compiler/include/scan.hpp"

using namespace Theo;

struct ParseState {
  AST &a;
  std::vector<Token>::iterator &pos;

  Theo::Token::Type lookahead() { return pos->t; }

  void match(Theo::Token::Type t) {
    if (lookahead() != t) {
      a.errors.push_back({pos->line, pos->file,
                          "expected '" + token_string(t) +
                              "' token, but got \"" + pos->text +
                              "\", which is '" + token_string(pos->t) + "'"});
      while (lookahead() != Theo::Token::PROGSEP &&
             lookahead() != Theo::Token::T_EOF)
        pos++;
    }
    if (lookahead() != Theo::Token::T_EOF) pos++;
  }

  Node *matchmk(Token::Type t, Node::Type n, Node *left, Node *right) {
    Node *nn = a.mk(n, pos->line, pos->file, pos->text, left, right);
    match(t);
    return nn;
  }
};

/**
 * Syntax analysis is implemented as a recursive-descent parser,
 * according to the following LL(1) grammar:
 * S -> PROGRAM id PORTS do P end S
 * S -> P
 * PORTS -> in ARGS OPORTS
 * PORTS ->
 * OPORTS -> out id
 * OPORTS ->
 * ARGS   -> ID MARGS
 * MARGS  -> , ARGS
 * MARGS  ->
 * P      -> id PID
 * PID    -> := VALUE MOREP
 * PID    -> : P MOREP
 * P      -> loop id do P end MOREP
 * P      -> while id do P end MOREP
 * P      -> goto id MOREP
 * P      -> if id = int then goto id MOREP
 * P      -> stop MOREP
 * MOREP  -> ; P
 * MOREP  ->
 * VALUE  -> id
 * VALUE  -> int
 * VALUE  -> run id with VARGS end
 * VARGS  ->
 * VARGS  -> VALUE MVARGS
 * MVARGS -> , VALUE MVARGS
 * MVARGS ->
 */
Theo::Node *PORTS(ParseState &ps);
Theo::Node *OPORTS(ParseState &ps);
Theo::Node *ARGS(ParseState &ps);
Theo::Node *MARGS(ParseState &ps);
Theo::Node *P(ParseState &ps);
Theo::Node *MOREP(ParseState &ps);
Theo::Node *VALUE(ParseState &ps);
Theo::Node *VARGS(ParseState &ps);
Theo::Node *MVARGS(ParseState &ps);

Theo::Node *S(ParseState &ps) {
  switch (ps.lookahead()) {
    case Token::PROGRAM: {
      ps.match(Token::PROGRAM);
      Node *name = ps.matchmk(Token::ID, Node::Type::NAME, NULL, NULL);
      Node *port = PORTS(ps);
      ps.match(Token::DO);
      Node *body = P(ps);
      Node *end = ps.matchmk(Token::END, Node::Type::NAME, NULL, NULL);
      Node *more = S(ps);

      return ps.a.mk(
          Node::Type::SPLIT, name->line, name->file, "",
          ps.a.mk(Node::Type::PROGRAM, name->line, name->file, "",
                  ps.a.mk(Node::Type::SPLIT, name->line, name->file, "", name,
                          port),
                  ps.a.mk(Node::Type::SPLIT, name->line, name->file, "", body,
                          ps.a.mk(Node::Type::MARK, end->line, end->file, "",
                                  end, NULL))),
          more);
    }
    default: {
      return P(ps);
    }
  }
}

Theo::Node *PORTS(ParseState &ps) {
  switch (ps.lookahead()) {
    case Token::IN: {
      ps.match(Token::IN);
      Node *args = ARGS(ps);
      Node *outs = OPORTS(ps);
      return ps.a.mk(Node::Type::SPLIT, args->line, args->file, "", args, outs);
    }
    default:
      return NULL;
  }
}

Theo::Node *OPORTS(ParseState &ps) {
  switch (ps.lookahead()) {
    case Token::OUT: {
      ps.match(Token::OUT);
      Node *name = ps.matchmk(Token::ID, Node::Type::NAME, NULL, NULL);
      return name;
    }
    default:
      return NULL;
  }
}

Theo::Node *ARGS(ParseState &ps) {
  Node *id = ps.matchmk(Token::ID, Node::Type::NAME, NULL, NULL);
  Node *more = MARGS(ps);
  return ps.a.mk(Node::Type::SPLIT, id->line, id->file, "", id, more);
}

Theo::Node *MARGS(ParseState &ps) {
  if (ps.lookahead() != Theo::Token::ARGSEP) return NULL;
  ps.match(Theo::Token::ARGSEP);
  return ARGS(ps);
}

void expected_end_or_semicolon(ParseState &ps) {
  for (;;) switch (ps.lookahead()) {
      case Token::ID:
      case Token::LOOP:
      case Token::WHILE:
      case Token::GOTO:
      case Token::IF:
      case Token::STOP: {
        ps.a.errors.push_back(
            {ps.pos->line, ps.pos->file,
             "probable missing ';' before '" + ps.pos->text + "'"});
        P(ps);
        break;
      }
      case Token::PROGRAM: {
        ps.a.errors.push_back(
            {ps.pos->line, ps.pos->file,
             "program definition not allowed here; all program definitions "
             "must happen before global instructions"});
        S(ps);
        break;
      }
      case Token::PROGSEP: {
        MOREP(ps);
        break;
      }
      default:
        return;
    }
}

Theo::Node *P(ParseState &ps) {
  switch (ps.lookahead()) {
    case Token::ID: {
      Node *left = ps.matchmk(Token::ID, Node::Type::NAME, NULL, NULL);
      Node *comb = NULL;
      if (ps.lookahead() == Token::ASSIGN) {
        ps.match(Token::ASSIGN);
        comb = ps.a.mk(Node::Type::ASSIGN, left->line, left->file, "", left,
                       VALUE(ps));
      } else if (ps.lookahead() == Token::LABELDEC) {
        ps.match(Token::LABELDEC);
        comb =
            ps.a.mk(Node::Type::MARK, left->line, left->file, "", left, NULL);
        Node *p = P(ps);
        comb = ps.a.mk(Node::Type::SPLIT, left->line, left->file, "", comb, p);
      } else {
        ps.a.errors.push_back(
            {ps.pos->line, ps.pos->file,
             "expected assignment (:=), label declaration (:) "
             ", but found '" +
                 ps.pos->text + "'"});
      }

      Node *more = MOREP(ps);
      expected_end_or_semicolon(ps);
      return ps.a.mk(Node::Type::SPLIT, left->line, left->file, "", comb, more);
    }
    case Token::LOOP: {
      ps.match(Token::LOOP);
      Node *name = ps.matchmk(Token::ID, Node::Type::NAME, NULL, NULL);
      ps.match(Token::DO);
      Node *body = P(ps);
      Node *end = ps.matchmk(Token::END, Node::Type::NAME, NULL, NULL);
      end = ps.a.mk(Node::Type::MARK, end->line, end->file, "", end, NULL);

      Node *loop =
          ps.a.mk(Node::Type::LOOP, name->line, name->file, "", name, body);
      loop = ps.a.mk(Node::Type::SPLIT, name->line, name->file, "", loop, end);
      Node *more = MOREP(ps);
      expected_end_or_semicolon(ps);
      return ps.a.mk(Node::Type::SPLIT, name->line, name->file, "", loop, more);
    }
    case Token::WHILE: {
      ps.match(Token::WHILE);
      Node *name = ps.matchmk(Token::ID, Node::Type::NAME, NULL, NULL);
      ps.match(Token::NEQ_ZERO);
      ps.match(Token::DO);
      Node *body = P(ps);
      Node *end = ps.matchmk(Token::END, Node::Type::NAME, NULL, NULL);
      end = ps.a.mk(Node::Type::MARK, end->line, end->file, "", end, NULL);

      Node *loop =
          ps.a.mk(Node::Type::WHILE, name->line, name->file, "", name, body);
      loop = ps.a.mk(Node::Type::SPLIT, name->line, name->file, "", loop, end);
      Node *more = MOREP(ps);
      expected_end_or_semicolon(ps);
      return ps.a.mk(Node::Type::SPLIT, name->line, name->file, "", loop, more);
    }
    case Token::GOTO: {
      ps.match(Token::GOTO);
      Node *name = ps.matchmk(Token::ID, Node::Type::NAME, NULL, NULL);
      Node *more = MOREP(ps);
      expected_end_or_semicolon(ps);
      Node *go =
          ps.a.mk(Node::Type::GOTO, name->line, name->file, "", name, NULL);
      return ps.a.mk(Node::Type::SPLIT, name->line, name->file, "", go, more);
    }
    case Token::IF: {
      ps.match(Token::IF);
      Node *id = ps.matchmk(Token::ID, Node::Type::NAME, NULL, NULL);
      ps.match(Token::EQ);
      Node *c = ps.matchmk(Token::INT, Node::Type::NUMBER, NULL, NULL);
      ps.match(Token::THEN);
      ps.match(Token::GOTO);
      Node *go = ps.matchmk(Token::ID, Node::Type::NAME, NULL, NULL);
      Node *more = MOREP(ps);
      expected_end_or_semicolon(ps);

      Node *eq = ps.a.mk(Node::Type::EQ, id->line, id->file, "", id, c);
      go = ps.a.mk(Node::Type::GOTO, go->line, go->file, "", go, NULL);
      Node *t = ps.a.mk(Node::Type::IF, id->line, id->file, "", eq, go);

      return ps.a.mk(Node::Type::SPLIT, id->line, id->file, "", t, more);
    }
    case Token::STOP: {
      Node *stop = ps.matchmk(Token::Type::STOP, Node::Type::STOP, NULL, NULL);
      Node *more = MOREP(ps);
      expected_end_or_semicolon(ps);
      return ps.a.mk(Node::Type::SPLIT, stop->line, stop->file, "", stop, more);
    }
    default: {
      ps.a.errors.push_back(
          {ps.pos->line, ps.pos->file,
           "expected program component: assignment, label declaration, loop / "
           "while / goto statement, but found '" +
               ps.pos->text + "'"});
      expected_end_or_semicolon(ps);
      return NULL;
    }
  }
}

Theo::Node *MOREP(ParseState &ps) {
  if (ps.lookahead() != Theo::Token::PROGSEP) {
    return NULL;
  }
  ps.match(Theo::Token::PROGSEP);
  if (ps.lookahead() == Token::END || ps.lookahead() == Token::T_EOF) {
    ps.a.errors.push_back(
        {ps.pos->line, ps.pos->file,
         "probable excess semicolon before '" + ps.pos->text + "'"});
  }
  return P(ps);
}

Theo::Node *VALUE(ParseState &ps) {
  switch (ps.lookahead()) {
    case Token::ID: {
      return ps.matchmk(Token::ID, Node::Type::NAME, NULL, NULL);
    }
    case Token::INT: {
      return ps.matchmk(Token::INT, Node::Type::NUMBER, NULL, NULL);
    }
    case Token::RUN: {
      ps.match(Token::RUN);
      Node *id = ps.matchmk(Token::ID, Node::Type::NAME, NULL, NULL);
      ps.match(Token::WITH);
      Node *vargs = VARGS(ps);
      ps.match(Token::END);
      return ps.a.mk(Node::Type::CALL, id->line, id->file, "", id, vargs);
    }
    default: {
      ps.a.errors.push_back(
          {ps.pos->line, ps.pos->file,
           "expected value: ID, INT, ID + INT or function call"});
      return NULL;
    }
  }
}

Theo::Node *VARGS(ParseState &ps) {
  if (ps.lookahead() != Token::ID && ps.lookahead() != Token::INT &&
      ps.lookahead() != Token::RUN)
    return NULL;
  Node *a1 = VALUE(ps);
  Node *re = MVARGS(ps);
  return ps.a.mk(Node::Type::SPLIT, a1->line, a1->file, "", a1, re);
}

Theo::Node *MVARGS(ParseState &ps) {
  if (ps.lookahead() != Theo::Token::ARGSEP) return NULL;
  ps.match(Theo::Token::ARGSEP);
  Node *v = VALUE(ps);
  Node *m = MVARGS(ps);
  return ps.a.mk(Node::Type::SPLIT, v->line, v->file, "", v, m);
}

ParseResult Theo::parse(std::map<FileName, FileContent> files, FileName main) {
  std::string standard_macros =
      "\
DEFINE PRIO 1000000 <ID> + <INT> AS RUN __INC__ WITH $0, $1 END END DEFINE\n\
DEFINE PRIO 1000000 <ID> - <INT> AS RUN __DEC__ WITH $0, $1 END END DEFINE\n\
  ";

  files.insert(std::make_pair("__standards__", standard_macros));
  std::string incl_phrase = "include \"__standards__\"";
  if (files.contains(main))
    files[main].insert(files[main].begin(), incl_phrase.begin(),
                       incl_phrase.end());

  AST a;
  a.parsed_correctly = false;
  a.root = NULL;
  a.all_allocated_nodes = {};
  a.errors = {};

  Theo::ScanResult sr = Theo::scan(files, main);

  std::vector<std::string> file_requests;

  std::for_each(sr.errors.begin(), sr.errors.end(),
                [&file_requests](const ParseError &pe) -> void {
                  if (pe.t == ParseError::FILE_NOT_FOUND ||
                      pe.t == ParseError::MAIN_FILE_NOT_FOUND)
                    file_requests.push_back(pe.file_request);
                });

  Theo::MacroExtractionResult mer = Theo::extract_macros(sr.toks);

  Theo::MacroApplicationResult mar =
      Theo::apply_macros(mer.tokens, mer.macros, THEO_MACRO_PASSES);

  std::vector<Token>::iterator it = mar.transformed_sequence.begin();
  ParseState ps = {a, it};

  a.root = S(ps);

  while (ps.pos < mar.transformed_sequence.end() &&
         ps.lookahead() != Token::T_EOF) {
    ps.a.errors.push_back(
        {ps.pos->line, ps.pos->file,
         "expected EOF, but got excess input: '" + ps.pos->text + "'"});
    ps.match(ps.lookahead());
    if (ps.lookahead() == Token::T_EOF) break;
    S(ps);
  }

  std::vector<std::vector<ParseError>> errs = {sr.errors, mer.errors,
                                               mar.errors};

  for (auto &err : errs)
    for (auto &e : err) {
      a.errors.push_back({e.line, e.file, e.msg});
    }
  if (a.errors.size() == 0) a.parsed_correctly = true;

  return {file_requests, a};
}
