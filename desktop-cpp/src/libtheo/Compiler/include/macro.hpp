#ifndef __LIBTHEO_C_MACRO_HPP_
#define __LIBTHEO_C_MACRO_HPP_

#include <optional>
#include <vector>

#include "Compiler/include/ParserGenerator/lrparser.hpp"
#include "Compiler/include/parse_error.hpp"
#include "Compiler/include/token.hpp"

namespace Theo {

struct MacroDefinition {
  int priority;
  /*sequence of tokens to match*/
  std::vector<Token> rule;
  // location of ID and INT tokens in .rule (to match specific var-names)
  std::vector<unsigned int> content_constraint_token_indices;
  // location of grammar tokens in .rule (<P>, <V>, <ID>, <INT>, <ARGS>)
  std::vector<unsigned int> template_token_indices;
  /* sequence of Tokens to replace with */
  std::vector<Token> replacement;
};

struct MacroExtractionResult {
  std::vector<Theo::ParseError> errors;
  /* token vector without macro definitions  */
  std::vector<Theo::Token> tokens;
  std::vector<Theo::MacroDefinition> macros;
};

struct MacroApplicationResult {
  std::vector<ParseError> errors;
  std::vector<Token> transformed_sequence;
};

/**
 * Extract the macro definitions from a token stream;
 * @param tokens output from scanner
 */
Theo::MacroExtractionResult extract_macros(std::vector<Theo::Token> tokens);

/**
 * Apply macros to a token stream;
 * Possible Erros:
 *  - a macro is not linearly parseable (because of open-endedness or else)
 *  - the maximum number of passes was reached
 * @param input       input, cleared of macro definitions (output of
 * extract_macros)
 * @param definitions macro definitions extracted by extract_macros
 * @param passes      maximum number of macro expansions to perform
 */
Theo::MacroApplicationResult apply_macros(
    std::vector<Theo::Token> input,
    std::vector<Theo::MacroDefinition> &definitions, unsigned int passes);

/**
 * attempt to back-convert a token sequence into a string;
 */
std::string recover_from_tokens(const std::vector<Token> &tok);
};  // namespace Theo

#endif
