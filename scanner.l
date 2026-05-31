%{
#include <string>
#include <cstdlib>
#include "ast.h"
#include "parser.tab.hpp"   // gerado pelo Bison

extern int yylineno;

// Converte escape sequences dentro de strings
static std::string processString(const char* s) {
    std::string result;
    // Pula as aspas inicial e final
    for (int i = 1; s[i+1] != '\0'; i++) {  // até penúltimo char
        if (s[i] == '\\') {
            i++;
            switch(s[i]) {
                case 'n':  result += '\n'; break;
                case 't':  result += '\t'; break;
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                default:   result += s[i]; break;
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

/* Definições de padrões */
DIGIT       [0-9]
LETTER      [a-zA-Z]
IDENT_CHAR  [a-zA-Z0-9_\-?!*/<>=+]
WS          [ \t\r\n]

%%

    /* Comentários — ignorados */
";"[^\n]*           { /* comentário de linha */ }

    /* Espaços em branco — ignorados */
{WS}+               { /* whitespace */ }

    /* Palavras-chave */
"define"            { return KW_DEFINE;  }
"if"                { return KW_IF;      }
"cond"              { return KW_COND;    }
"else"              { return KW_ELSE;    }
"let"               { return KW_LET;     }
"let*"              { return KW_LETSTAR; }
"lambda"            { return KW_LAMBDA;  }
"begin"             { return KW_BEGIN;   }
"and"               { return KW_AND;     }
"or"                { return KW_OR;      }
"not"               { return KW_NOT;     }
"quote"             { return KW_QUOTE;   }

    /* Funções built-in */
"display"           { return BUILTIN_DISPLAY;  }
"newline"           { return BUILTIN_NEWLINE;  }
"car"               { return BUILTIN_CAR;      }
"cdr"               { return BUILTIN_CDR;      }
"cons"              { return BUILTIN_CONS;      }
"list"              { return BUILTIN_LIST;      }
"null?"             { return BUILTIN_NULLP;     }
"pair?"             { return BUILTIN_PAIRP;     }

    /* Operadores aritméticos e relacionais */
"+"                 { yylval.sval = strdup("+");  return OP_ARITH; }
"-"                 { yylval.sval = strdup("-");  return OP_ARITH; }
"*"                 { yylval.sval = strdup("*");  return OP_ARITH; }
"/"                 { yylval.sval = strdup("/");  return OP_ARITH; }
"="                 { yylval.sval = strdup("=="); return OP_REL;   }
"<"                 { yylval.sval = strdup("<");  return OP_REL;   }
">"                 { yylval.sval = strdup(">");  return OP_REL;   }
"<="                { yylval.sval = strdup("<="); return OP_REL;   }
">="                { yylval.sval = strdup(">="); return OP_REL;   }

    /* Booleanos */
"#t"                { yylval.bval = true;  return BOOL_LIT; }
"#f"                { yylval.bval = false; return BOOL_LIT; }
"#true"             { yylval.bval = true;  return BOOL_LIT; }
"#false"            { yylval.bval = false; return BOOL_LIT; }

    /* Lista vazia */
"'()"               { return NIL_LIT; }
"(quote ())"        { return NIL_LIT; }

    /* Números */
{DIGIT}+\.{DIGIT}+  {
    yylval.fval = atof(yytext);
    return FLOAT_LIT;
}
{DIGIT}+            {
    yylval.ival = atoi(yytext);
    return INT_LIT;
}
-{DIGIT}+\.{DIGIT}+ {
    yylval.fval = atof(yytext);
    return FLOAT_LIT;
}
-{DIGIT}+           {
    yylval.ival = atoi(yytext);
    return INT_LIT;
}

    /* Strings */
\"([^\"\\]|\\.)*\"  {
    yylval.sval = strdup(processString(yytext).c_str());
    return STRING_LIT;
}

    /* Identificadores */
{LETTER}{IDENT_CHAR}*  {
    yylval.sval = strdup(yytext);
    return IDENT;
}

    /* Pontuação */
"("                 { return LPAREN; }
")"                 { return RPAREN; }
"'"                 { return QUOTE;  }

    /* Qualquer coisa desconhecida */
.                   {
    fprintf(stderr, "[Scanner] Erro: caractere desconhecido '%s' na linha %d\n",
            yytext, yylineno);
}

%%
