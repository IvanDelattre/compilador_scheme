/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_HPP_INCLUDED
# define YY_YY_PARSER_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    KW_DEFINE = 258,               /* KW_DEFINE  */
    KW_IF = 259,                   /* KW_IF  */
    KW_COND = 260,                 /* KW_COND  */
    KW_ELSE = 261,                 /* KW_ELSE  */
    KW_LET = 262,                  /* KW_LET  */
    KW_LETSTAR = 263,              /* KW_LETSTAR  */
    KW_LAMBDA = 264,               /* KW_LAMBDA  */
    KW_BEGIN = 265,                /* KW_BEGIN  */
    KW_AND = 266,                  /* KW_AND  */
    KW_OR = 267,                   /* KW_OR  */
    KW_NOT = 268,                  /* KW_NOT  */
    KW_QUOTE = 269,                /* KW_QUOTE  */
    BUILTIN_DISPLAY = 270,         /* BUILTIN_DISPLAY  */
    BUILTIN_NEWLINE = 271,         /* BUILTIN_NEWLINE  */
    BUILTIN_CAR = 272,             /* BUILTIN_CAR  */
    BUILTIN_CDR = 273,             /* BUILTIN_CDR  */
    BUILTIN_CONS = 274,            /* BUILTIN_CONS  */
    BUILTIN_LIST = 275,            /* BUILTIN_LIST  */
    BUILTIN_NULLP = 276,           /* BUILTIN_NULLP  */
    BUILTIN_PAIRP = 277,           /* BUILTIN_PAIRP  */
    LPAREN = 278,                  /* LPAREN  */
    RPAREN = 279,                  /* RPAREN  */
    QUOTE = 280,                   /* QUOTE  */
    NIL_LIT = 281,                 /* NIL_LIT  */
    INT_LIT = 282,                 /* INT_LIT  */
    FLOAT_LIT = 283,               /* FLOAT_LIT  */
    BOOL_LIT = 284,                /* BOOL_LIT  */
    STRING_LIT = 285,              /* STRING_LIT  */
    IDENT = 286,                   /* IDENT  */
    OP_ARITH = 287,                /* OP_ARITH  */
    OP_REL = 288                   /* OP_REL  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 694 "parser.y"

    int    ival;
    double fval;
    bool   bval;
    char*  sval;
    ASTNode* node;

#line 105 "parser.tab.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSER_TAB_HPP_INCLUDED  */
