%{
#include <string>
#include <cstdlib>
#include <cstring>
#include "ast.h"
#include "parser.tab.hpp"   // gerado pelo Bison

extern int yylineno;

static std::string processString(const char* s) {
    std::string result;
    for (int i = 1; s[i+1] != '\0'; i++) {
        if (s[i] == '\\') {
            i++;
            switch(s[i]) {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                default: result += s[i]; break;
            }
        } else {
            result += s[i];
        }
    }
    return result;
}
%}

%option noyywrap
%option yylineno

DIGIT      [0-9]
LETTER     [a-zA-Z]
IDENT_CHAR [a-zA-Z0-9_\-?!*/<>=+]
WS         [ \t\r\n]

%%

{WS}+               { /* skip whitespace */ }
";"[^\n]*          { /* comment */ }

"("               { return LPAREN; }
")"               { return RPAREN; }
"'"               { return QUOTE; }

"+"               { yylval.sval = strdup("+");  return OP_ARITH; }
"-"               { yylval.sval = strdup("-");  return OP_ARITH; }
"*"               { yylval.sval = strdup("*");  return OP_ARITH; }
"/"               { yylval.sval = strdup("/");  return OP_ARITH; }

"<="              { yylval.sval = strdup("<="); return OP_REL; }
">="              { yylval.sval = strdup(">="); return OP_REL; }
"<"               { yylval.sval = strdup("<");  return OP_REL; }
">"               { yylval.sval = strdup(">");  return OP_REL; }
"="               { yylval.sval = strdup("=="); return OP_REL; }

"#t"              { yylval.bval = true;  return BOOL_LIT; }
"#f"              { yylval.bval = false; return BOOL_LIT; }
"#true"           { yylval.bval = true;  return BOOL_LIT; }
"#false"          { yylval.bval = false; return BOOL_LIT; }

"'()"             { return NIL_LIT; }
"(quote ())"      { return NIL_LIT; }

{DIGIT}+\.{DIGIT}+ {
    yylval.fval = atof(yytext);
    return FLOAT_LIT;
}
-{DIGIT}+\.{DIGIT}+ {
    yylval.fval = atof(yytext);
    return FLOAT_LIT;
}
{DIGIT}+           {
    yylval.ival = atoi(yytext);
    return INT_LIT;
}
-{DIGIT}+          {
    yylval.ival = atoi(yytext);
    return INT_LIT;
}

\"([^\"\\]|\\.)*\"  {
    yylval.sval = strdup(processString(yytext).c_str());
    return STRING_LIT;
}

{LETTER}{IDENT_CHAR}*  {
    // Keywords and builtins
    if (strcmp(yytext, "define") == 0) return KW_DEFINE;
    if (strcmp(yytext, "if") == 0)     return KW_IF;
    if (strcmp(yytext, "cond") == 0)   return KW_COND;
    if (strcmp(yytext, "else") == 0)   return KW_ELSE;
    if (strcmp(yytext, "let") == 0)    return KW_LET;
    if (strcmp(yytext, "let*") == 0)   return KW_LETSTAR;
    if (strcmp(yytext, "lambda") == 0) return KW_LAMBDA;
    if (strcmp(yytext, "begin") == 0)  return KW_BEGIN;
    if (strcmp(yytext, "and") == 0)    return KW_AND;
    if (strcmp(yytext, "or") == 0)     return KW_OR;
    if (strcmp(yytext, "not") == 0)    return KW_NOT;
    if (strcmp(yytext, "quote") == 0)  return KW_QUOTE;

    if (strcmp(yytext, "display") == 0) return BUILTIN_DISPLAY;
    if (strcmp(yytext, "newline") == 0) return BUILTIN_NEWLINE;
    if (strcmp(yytext, "car") == 0)     return BUILTIN_CAR;
    if (strcmp(yytext, "cdr") == 0)     return BUILTIN_CDR;
    if (strcmp(yytext, "cons") == 0)    return BUILTIN_CONS;
    if (strcmp(yytext, "list") == 0)    return BUILTIN_LIST;
    if (strcmp(yytext, "null?") == 0)   return BUILTIN_NULLP;
    if (strcmp(yytext, "pair?") == 0)   return BUILTIN_PAIRP;

    yylval.sval = strdup(yytext);
    return IDENT;
}

.  {
    fprintf(stderr, "[Scanner] Erro: caractere desconhecido '%s' na linha %d\n",
            yytext, yylineno);
}

%%
