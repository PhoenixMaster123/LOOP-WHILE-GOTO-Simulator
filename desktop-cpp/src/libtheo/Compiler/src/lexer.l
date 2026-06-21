%{
#define YY_DECL int yylex(Theo::Token *ret, yyscan_t yyscanner)
#define TOK(t) {*ret = Theo::Token(t, std::string(yytext), yyextra->filename, yylineno); return 1;}
%}

%{
#include "Compiler/include/token.hpp"
#include "Compiler/include/scanner_info.hpp"
%}

%option nounistd never-interactive
%option reentrant noyywrap yylineno noline
%option extra-type="Theo::ScannerInfo*"

id [a-zA-Z\_][a-zA-Z0-9_]*
int (0|([1-9][0-9]*))
include (INCLUDE|Include|include)
fname \"[^\"]*\"
define (DEFINE|Define|Def|define|def)
as (AS|As|as)
enddefine (END\ DEFINE|End\ Define|end\ define|ENDDEF|Enddef|enddef)
priority (PRIORITY|Priority|priority|PRIO|Prio|prio)
do (DO|do|Do)
loop (LOOP|Loop|loop)
while (WHILE|While|while)
goto (GOTO|Goto|goto)
if (IF|If|if)
then (THEN|Then|then)
stop (STOP|Stop|stop)
end (END|End|end)
program (PROGRAM|Program|program|PROG|Prog|prog)
in (IN|In|in)
out (OUT|Out|out)
progtemp (<{program}>|<P>|<p>)
value (VALUE|Value|value|VAL|Val|val)
valtemp (<{value}>|<V>|<v>)
idtemp (<ID>|<id>)
inttemp (<INT>|<Int>|<int>)
argstemp (<ARGS>|<Args>|<args>|<A>|<a>)
run (RUN|Run|run)
with (WITH|With|with)
ws [ \t\n]+

%%

{ws} {}
\( {TOK(Theo::Token::Type::PAREN_OPEN)}
\) {TOK(Theo::Token::Type::PAREN_CLOSE)}
\, {TOK(Theo::Token::Type::ARGSEP)}
\; {TOK(Theo::Token::Type::PROGSEP)}
\: {TOK(Theo::Token::Type::LABELDEC)}
\:\= {TOK(Theo::Token::Type::ASSIGN)}
!=\ 0 {TOK(Theo::Token::Type::NEQ_ZERO)}
= {TOK(Theo::Token::Type::EQ)}
{run} {TOK(Theo::Token::Type::RUN)}
{with} {TOK(Theo::Token::Type::WITH)}
{do} {TOK(Theo::Token::Type::DO)}
{loop} {TOK(Theo::Token::Type::LOOP)}
{while} {TOK(Theo::Token::Type::WHILE)}
{goto} {TOK(Theo::Token::Type::GOTO)}
{if} {TOK(Theo::Token::Type::IF)}
{then} {TOK(Theo::Token::Type::THEN)}
{stop} {TOK(Theo::Token::Type::STOP)}
{end} {TOK(Theo::Token::Type::END)}
{program} {TOK(Theo::Token::Type::PROGRAM)}
{in} {TOK(Theo::Token::Type::IN)}
{out} {TOK(Theo::Token::Type::OUT)}
{include} {TOK(Theo::Token::Type::INCLUDE)}
{fname} {TOK(Theo::Token::Type::FNAME)}
{define} {TOK(Theo::Token::Type::DEFINE)}
{as} {TOK(Theo::Token::Type::AS)}
{priority} {TOK(Theo::Token::Type::PRIORITY)}
{enddefine} {TOK(Theo::Token::Type::END_DEFINE)}
{progtemp} {TOK(Theo::Token::Type::PROG_TEMP)}
{valtemp} {TOK(Theo::Token::Type::VALUE_TEMP)}
{idtemp} {TOK(Theo::Token::Type::ID_TEMP)}
{inttemp} {TOK(Theo::Token::Type::INT_TEMP)}
\${int} {TOK(Theo::Token::Type::INSERTION)}
\#{int} {TOK(Theo::Token::Type::TEMP_VAL)}
{id} {TOK(Theo::Token::Type::ID)}
{int} {TOK(Theo::Token::Type::INT)}
{argstemp} {TOK(Theo::Token::Type::ARGS_TEMP)}
\/\/.* {}
.|\n  {TOK(Theo::Token::Type::NV_ID)}
%%

