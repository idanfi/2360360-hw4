%{ /* Declarations section */
#include <string>
using std::string;
#include "hw3_output.hpp"
#include "Types.h"
#include "parser.tab.hpp"
%}

%option yylineno
%option noyywrap
%option nounput

positive_digit ([1-9])
digit ([[0-9]])
letter ([a-zA-Z])
whitespace ([\t\n\r ])

_void (void)
_int (int)
_byte (byte)
_b (b)
_bool (bool)
_and (and)
_or (or)
_not (not)
_true (true)
_false (false)
_return (return)
_if (if)
_else (else)
_while (while)
_break (break)
_continue (continue)
_switch (switch)
_case (case)
_default (default)
_colon (:)
_sc (;)
_comma (,)
_lparen (\()
_rparen (\))
_lbrace (\{)
_rbrace (\})
_rel ((\<=)|(\>=)|(\<)|(\>))
_eq ((==)|(!=))
_assign (=)
_mul (\*)
_dev (\/)
_plus (\+)
_minus (\-)
_id ([a-zA-Z][a-zA-Z0-9]*)
_num (0|([1-9][0-9]*))
_string (\"([^\n\r\"\\]|\\[rnt"\\])+\")
_comment (\/\/[^\r\n]*(\r|\n|\r\n)?)

%%
{whitespace} ;
{_void} {yylval = new TypeExp(TYPE_VOID); return VOID;}
{_int} {yylval = new TypeExp(TYPE_INT); return INT;}
{_byte} {yylval = new TypeExp(TYPE_BYTE); return BYTE;}
{_b} return B;
{_bool} {yylval = new TypeExp(TYPE_BOOL); return BOOL;}
{_and} return AND;
{_or} return OR;
{_not} return NOT;
{_true} {yylval = new TypeExp(TYPE_BOOL); return TRUE;}
{_false} {yylval = new TypeExp(TYPE_BOOL); return FALSE;}
{_return} return RETURN;
{_if} return IF;
{_else} return ELSE;
{_while} return WHILE;
{_break} return BREAK;
{_continue} return CONTINUE;
{_switch} return SWITCH;
{_case} return CASE;
{_default} return DEFAULT;
{_colon} return COLON;
{_sc} return SC;
{_comma} return COMMA;
{_lparen} return LPAREN;
{_rparen} return RPAREN;
{_lbrace} return LBRACE;
{_rbrace} return RBRACE;
{_eq} return EQUALITY;
{_rel} return RELATIONAL;
{_assign} return ASSIGN;
{_comment} ;
{_plus} return PLUS;
{_minus} return MINUS;
{_mul} return MUL;
{_dev} return DEV;
{_id} {yylval = new IDExp(yytext); return ID;}
{_num} {yylval = new IDExp(yytext); return NUM;}
{_string} {yylval = new StringExp(yytext); return STRING;}
. {
      output::errorLex(yylineno);
      exit(-1);
  }

%%
