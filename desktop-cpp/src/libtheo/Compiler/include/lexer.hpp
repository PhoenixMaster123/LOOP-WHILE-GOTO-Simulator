#ifndef __LIBLOOP_C_LEXER_HPP__
#define __LIBLOOP_C_LEXER_HPP__

// wrapper around lex.yy.h
#include "Compiler/include/scanner_info.hpp"
#include "Compiler/include/token.hpp"

// gotta love lex

#define YY_DECL int yylex(Theo::Token *ret, yyscan_t yyscanner)
#include "Compiler/include/lex.yy.h"
YY_DECL;

#endif
