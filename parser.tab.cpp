/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser.y"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "ast.h"
#include "symtable.h"

/* Declarações externas do Flex */
extern int  yylex();
extern int  yylineno;
extern FILE* yyin;

void yyerror(const char* msg);

/* Tabela de símbolos global */
static SymbolTable symTable;

/* Saída Python acumulada */
static std::ostringstream pythonOutput;

struct Diagnostic {
    std::string kind;
    std::string message;
    int line = 0;
    bool fatal = false;
};

static std::vector<Diagnostic> diagnostics;

/* Nível de indentação atual */
static int indentLevel = 0;

/* Contagem de erros fatais detectados durante compilação */
static int errorCount = 0;

/* Helpers de indentação */
static std::string indent() {
    return std::string(indentLevel * 4, ' ');
}

/* -------- Verificação de tipos -------- */
void collectDiagnostic(const char* kind, const std::string& msg, int line, bool fatal = false) {
    diagnostics.push_back(Diagnostic{kind, msg, line, fatal});
    if (fatal) {
        ++errorCount;
    }
}

static void flushDiagnostics() {
    for (const auto& diagnostic : diagnostics) {
        fprintf(stderr, "%s linha %d: %s\n",
                diagnostic.kind.c_str(), diagnostic.line, diagnostic.message.c_str());
    }
    diagnostics.clear();
}

static std::string translateSyntaxMessage(const std::string& msg) {
    std::string translated = msg;

    auto replaceAll = [&](const std::string& from, const std::string& to) {
        std::size_t pos = 0;
        while ((pos = translated.find(from, pos)) != std::string::npos) {
            translated.replace(pos, from.size(), to);
            pos += to.size();
        }
    };

    replaceAll("syntax error", "erro de sintaxe");
    replaceAll("unexpected end of file", "fim de arquivo inesperado");
    replaceAll("unexpected", "símbolo inesperado");
    replaceAll("expecting", "esperando");
    replaceAll("cannot back up", "não foi possível retroceder");
    replaceAll("LPAREN", "parêntese de abertura");
    replaceAll("RPAREN", "parêntese de fechamento");
    replaceAll("IDENT", "identificador");
    replaceAll("INT_LIT", "literal inteiro");
    replaceAll("FLOAT_LIT", "literal decimal");
    replaceAll("BOOL_LIT", "literal booleano");
    replaceAll("STRING_LIT", "literal de string");
    replaceAll("KW_DEFINE", "define");
    replaceAll("KW_IF", "if");
    replaceAll("KW_COND", "cond");
    replaceAll("KW_ELSE", "else");
    replaceAll("KW_LET", "let");
    replaceAll("KW_LETSTAR", "let*");
    replaceAll("KW_LAMBDA", "lambda");

    return translated;
}

static void typeWarning(const std::string& msg, int line) {
    collectDiagnostic("[Aviso]", msg, line, false);
}
static void typeError(const std::string& msg, int line) {
    collectDiagnostic("[Erro de tipo]", msg, line, true);
}
static void checkDefined(const std::string& name, int line) {
    if (!symTable.exists(name))
        typeWarning("Identificador '" + name + "' pode não estar definido", line);
}

static ASTNode* makeRecoveryNode(int line) {
    auto n = new ASTNode(NodeType::NIL, line);
    n->dataType = DataType::UNKNOWN;
    return n;
}

static void validateNode(ASTNodePtr node, SymbolTable scope);
static void validateBindingExpr(ASTNodePtr node, SymbolTable& scope);

static void validateBodyList(ASTNodePtr node, SymbolTable scope) {
    if (!node) return;
    for (auto& child : node->children)
        validateNode(child, scope);
}

static void validateNode(ASTNodePtr node, SymbolTable scope) {
    if (!node) return;

    switch (node->type) {
    case NodeType::IDENT:
        if (!scope.exists(node->sval))
            typeWarning("Identificador '" + node->sval + "' pode não estar definido", node->line);
        return;

    case NodeType::DEFINE_VAR: {
        validateNode(node->children[0], scope);
        SymbolInfo si;
        si.name = node->sval;
        si.dataType = node->children[0]->dataType;
        si.isFunction = false;
        scope.define(node->sval, si);
        return;
    }

    case NodeType::DEFINE_FUNC: {
        SymbolInfo si;
        si.name = node->funcName;
        si.dataType = DataType::FUNCTION;
        si.isFunction = true;
        si.arity = (int)node->params.size();
        scope.define(node->funcName, si);

        SymbolTable local = scope;
        local.pushScope();
        for (const auto& param : node->params) {
            SymbolInfo paramInfo;
            paramInfo.name = param;
            paramInfo.dataType = DataType::ANY;
            paramInfo.isFunction = false;
            local.define(param, paramInfo);
        }
        for (auto& child : node->children)
            validateNode(child, local);
        return;
    }

    case NodeType::LET_EXPR: {
        for (const auto& binding : node->bindings)
            validateNode(binding.second, scope);

        SymbolTable local = scope;
        local.pushScope();
        for (const auto& binding : node->bindings) {
            SymbolInfo bindingInfo;
            bindingInfo.name = binding.first;
            bindingInfo.dataType = binding.second ? binding.second->dataType : DataType::UNKNOWN;
            bindingInfo.isFunction = false;
            local.define(binding.first, bindingInfo);
        }
        for (auto& child : node->children)
            validateNode(child, local);
        return;
    }

    case NodeType::LETSTAR_EXPR: {
        SymbolTable local = scope;
        local.pushScope();
        for (const auto& binding : node->bindings) {
            validateNode(binding.second, local);
            SymbolInfo bindingInfo;
            bindingInfo.name = binding.first;
            bindingInfo.dataType = binding.second ? binding.second->dataType : DataType::UNKNOWN;
            bindingInfo.isFunction = false;
            local.define(binding.first, bindingInfo);
        }
        for (auto& child : node->children)
            validateNode(child, local);
        return;
    }

    case NodeType::LAMBDA_EXPR: {
        SymbolTable local = scope;
        local.pushScope();
        for (const auto& param : node->params) {
            SymbolInfo paramInfo;
            paramInfo.name = param;
            paramInfo.dataType = DataType::ANY;
            paramInfo.isFunction = false;
            local.define(param, paramInfo);
        }
        for (auto& child : node->children)
            validateNode(child, local);
        return;
    }

    case NodeType::PROGRAM:
    case NodeType::BEGIN_EXPR: {
        SymbolTable local = scope;
        for (auto& child : node->children)
            validateNode(child, local);
        return;
    }

    case NodeType::BINOP:
    case NodeType::UNOP:
    case NodeType::IF_EXPR:
    case NodeType::CALL_EXPR:
    case NodeType::DISPLAY_EXPR:
    case NodeType::NEWLINE_EXPR:
    case NodeType::LIST_EXPR:
    case NodeType::CAR_EXPR:
    case NodeType::CDR_EXPR:
    case NodeType::CONS_EXPR:
    case NodeType::NULL_CHECK:
    case NodeType::PAIR_CHECK:
    case NodeType::COND_EXPR:
    case NodeType::INT_LIT:
    case NodeType::FLOAT_LIT:
    case NodeType::STRING_LIT:
    case NodeType::BOOL_LIT:
    case NodeType::NIL:
        break;
    }

    for (auto& child : node->children)
        validateNode(child, scope);
    for (auto& binding : node->bindings)
        validateNode(binding.second, scope);
}

/* -------- Geração de código Python -------- */
static std::string genExpr(ASTNodePtr node);
static void        genStmt(ASTNodePtr node);

/* Converte identificador Scheme → Python (hifens viram underscore, ? vira _p, ! vira _b) */
static std::string schemeToPyIdent(const std::string& s) {
    std::string r;
    for (char c : s) {
        if (c == '-') r += '_';
        else if (c == '?') r += "_p";
        else if (c == '!') r += "_b";
        else if (c == '*') r += "_star";
        else r += c;
    }
    return r;
}

/* Converte operador aritmético Scheme → Python */
static std::string schemeOpToPython(const std::string& op) {
    if (op == "==") return "==";
    return op; // +, -, *, /, <, >, <=, >= são iguais
}

/* Gera expressão Python a partir de um nó */
static std::string genExpr(ASTNodePtr node) {
    if (!node) return "None";

    switch (node->type) {

    case NodeType::INT_LIT:
        return std::to_string(node->ival);

    case NodeType::FLOAT_LIT: {
        std::ostringstream oss;
        oss << node->fval;
        // Garante que tem ponto decimal
        std::string s = oss.str();
        if (s.find('.') == std::string::npos) s += ".0";
        return s;
    }

    case NodeType::STRING_LIT:
        return "\"" + node->sval + "\"";

    case NodeType::BOOL_LIT:
        return node->bval ? "True" : "False";

    case NodeType::NIL:
        return "[]";

    case NodeType::IDENT:
        return schemeToPyIdent(node->sval);

    case NodeType::BINOP: {
        // (op arg1 arg2 ...)  →  (arg1 op arg2 op ...)
        if (node->children.empty()) return "0";
        if (node->children.size() == 1) {
            // Unário: (- x) → -x
            return "(-" + genExpr(node->children[0]) + ")";
        }
        std::string result = genExpr(node->children[0]);
        std::string pyOp = schemeOpToPython(node->op);
        for (size_t i = 1; i < node->children.size(); i++) {
            result += " " + pyOp + " " + genExpr(node->children[i]);
        }
        return "(" + result + ")";
    }

    case NodeType::IF_EXPR: {
        // (if cond then else)
        std::string cond = genExpr(node->children[0]);
        std::string thenE = genExpr(node->children[1]);
        std::string elseE = (node->children.size() > 2)
                            ? genExpr(node->children[2])
                            : "None";
        return "(" + thenE + " if " + cond + " else " + elseE + ")";
    }

    case NodeType::LET_EXPR: {
        // let: todos os bindings avaliados no escopo externo
        std::string params, args;
        for (size_t i = 0; i < node->bindings.size(); i++) {
            if (i > 0) { params += ", "; args += ", "; }
            params += schemeToPyIdent(node->bindings[i].first);
            args   += genExpr(node->bindings[i].second);
        }
        std::string body = genExpr(node->children[0]);
        return "(lambda " + params + ": " + body + ")(" + args + ")";
    }

    case NodeType::LETSTAR_EXPR: {
        // let*: bindings sequenciais — aninha lambdas
        // (let* ((a 1)(b (+ a 1))) body) →
        //   (lambda a: (lambda b: body)(a+1))(1)
        if (node->bindings.empty())
            return genExpr(node->children[0]);
        std::string result = genExpr(node->children[0]);
        // Constrói de dentro para fora
        for (int i = (int)node->bindings.size() - 1; i >= 0; i--) {
            std::string pname = schemeToPyIdent(node->bindings[i].first);
            std::string val   = genExpr(node->bindings[i].second);
            result = "(lambda " + pname + ": " + result + ")(" + val + ")";
        }
        return result;
    }

    case NodeType::LAMBDA_EXPR: {
        std::string params;
        for (size_t i = 0; i < node->params.size(); i++) {
            if (i > 0) params += ", ";
            params += schemeToPyIdent(node->params[i]);
        }
        std::string body = genExpr(node->children[0]);
        return "(lambda " + params + ": " + body + ")";
    }

    case NodeType::CALL_EXPR: {
        std::string funcName = genExpr(node->children[0]);
        std::string args;
        for (size_t i = 1; i < node->children.size(); i++) {
            if (i > 1) args += ", ";
            args += genExpr(node->children[i]);
        }
        return funcName + "(" + args + ")";
    }

    case NodeType::DISPLAY_EXPR:
        return "print(" + genExpr(node->children[0]) + ", end='')";

    case NodeType::NEWLINE_EXPR:
        return "print()";

    case NodeType::LIST_EXPR: {
        std::string elems;
        for (size_t i = 0; i < node->children.size(); i++) {
            if (i > 0) elems += ", ";
            elems += genExpr(node->children[i]);
        }
        return "[" + elems + "]";
    }

    case NodeType::CAR_EXPR:
        return genExpr(node->children[0]) + "[0]";

    case NodeType::CDR_EXPR:
        return genExpr(node->children[0]) + "[1:]";

    case NodeType::CONS_EXPR:
        return "[" + genExpr(node->children[0]) + "] + " + genExpr(node->children[1]);

    case NodeType::NULL_CHECK:
        return "(" + genExpr(node->children[0]) + " == [])";

    case NodeType::PAIR_CHECK:
        return "(isinstance(" + genExpr(node->children[0]) + ", list) and len(" + genExpr(node->children[0]) + ") > 0)";

    case NodeType::BEGIN_EXPR: {
        // Em contexto de expressão, usa vírgula (operador vírgula não existe em Python)
        // Usamos uma função lambda de múltiplas expressões via lista
        if (node->children.empty()) return "None";
        if (node->children.size() == 1) return genExpr(node->children[0]);
        // Hack Python: [expr1, expr2, ..., exprN][-1]
        std::string elems;
        for (size_t i = 0; i < node->children.size(); i++) {
            if (i > 0) elems += ", ";
            elems += genExpr(node->children[i]);
        }
        return "[" + elems + "][-1]";
    }

    case NodeType::COND_EXPR: {
        // (cond (c1 e1) (c2 e2) (else eN))
        // → (e1 if c1 else (e2 if c2 else eN))
        if (node->children.empty()) return "None";
        // children são pares: [cond, expr, cond, expr, ..., elseExpr]
        // Montados com flag else no último
        std::string result = genExpr(node->children.back()); // else clause
        for (int i = (int)node->children.size() - 3; i >= 0; i -= 2) {
            std::string cond = genExpr(node->children[i]);
            std::string expr = genExpr(node->children[i+1]);
            result = "(" + expr + " if " + cond + " else " + result + ")";
        }
        return result;
    }

    default:
        return "None";
    }
}

/* Gera statement Python (pode ter múltiplas linhas) */
static void genStmt(ASTNodePtr node) {
    if (!node) return;

    switch (node->type) {

    case NodeType::DEFINE_VAR: {
        std::string val = genExpr(node->children[0]);
        pythonOutput << indent() << schemeToPyIdent(node->sval) << " = " << val << "\n";
        break;
    }

    case NodeType::DEFINE_FUNC: {
        // Cabeçalho
        std::string params;
        for (size_t i = 0; i < node->params.size(); i++) {
            if (i > 0) params += ", ";
            params += schemeToPyIdent(node->params[i]);
        }
        pythonOutput << indent() << "def " << schemeToPyIdent(node->funcName)
                     << "(" << params << "):\n";
        indentLevel++;

        // Corpo: pode ser múltiplos nós (begin implícito)
        if (node->children.size() == 1) {
            // Caso simples: única expressão → return
            std::string body = genExpr(node->children[0]);
            pythonOutput << indent() << "return " << body << "\n";
        } else {
            // Múltiplas expressões: últimas é return
            for (size_t i = 0; i < node->children.size() - 1; i++) {
                // Statements intermediários
                if (node->children[i]->type == NodeType::DISPLAY_EXPR ||
                    node->children[i]->type == NodeType::NEWLINE_EXPR) {
                    pythonOutput << indent() << genExpr(node->children[i]) << "\n";
                } else {
                    // define interno, etc.
                    genStmt(node->children[i]);
                }
            }
            // Última expressão é o valor de retorno
            std::string body = genExpr(node->children.back());
            pythonOutput << indent() << "return " << body << "\n";
        }
        indentLevel--;
        pythonOutput << "\n";
        break;
    }

    case NodeType::DISPLAY_EXPR:
        pythonOutput << indent() << genExpr(node) << "\n";
        break;

    case NodeType::NEWLINE_EXPR:
        pythonOutput << indent() << "print()\n";
        break;

    case NodeType::IF_EXPR: {
        // Como statement (sem else obrigatório)
        pythonOutput << indent() << "if " << genExpr(node->children[0]) << ":\n";
        indentLevel++;
        genStmt(node->children[1]);
        indentLevel--;
        if (node->children.size() > 2) {
            pythonOutput << indent() << "else:\n";
            indentLevel++;
            genStmt(node->children[2]);
            indentLevel--;
        }
        break;
    }

    case NodeType::BEGIN_EXPR:
        for (auto& child : node->children)
            genStmt(child);
        break;

    case NodeType::PROGRAM:
        for (auto& child : node->children)
            genStmt(child);
        break;

    default:
        // Expressão genérica como statement
        pythonOutput << indent() << genExpr(node) << "\n";
        break;
    }
}

/* Nó raiz do programa */
static ASTNodePtr programRoot;


#line 602 "parser.tab.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_KW_DEFINE = 3,                  /* KW_DEFINE  */
  YYSYMBOL_KW_IF = 4,                      /* KW_IF  */
  YYSYMBOL_KW_COND = 5,                    /* KW_COND  */
  YYSYMBOL_KW_ELSE = 6,                    /* KW_ELSE  */
  YYSYMBOL_KW_LET = 7,                     /* KW_LET  */
  YYSYMBOL_KW_LETSTAR = 8,                 /* KW_LETSTAR  */
  YYSYMBOL_KW_LAMBDA = 9,                  /* KW_LAMBDA  */
  YYSYMBOL_KW_BEGIN = 10,                  /* KW_BEGIN  */
  YYSYMBOL_KW_AND = 11,                    /* KW_AND  */
  YYSYMBOL_KW_OR = 12,                     /* KW_OR  */
  YYSYMBOL_KW_NOT = 13,                    /* KW_NOT  */
  YYSYMBOL_KW_QUOTE = 14,                  /* KW_QUOTE  */
  YYSYMBOL_BUILTIN_DISPLAY = 15,           /* BUILTIN_DISPLAY  */
  YYSYMBOL_BUILTIN_NEWLINE = 16,           /* BUILTIN_NEWLINE  */
  YYSYMBOL_BUILTIN_CAR = 17,               /* BUILTIN_CAR  */
  YYSYMBOL_BUILTIN_CDR = 18,               /* BUILTIN_CDR  */
  YYSYMBOL_BUILTIN_CONS = 19,              /* BUILTIN_CONS  */
  YYSYMBOL_BUILTIN_LIST = 20,              /* BUILTIN_LIST  */
  YYSYMBOL_BUILTIN_NULLP = 21,             /* BUILTIN_NULLP  */
  YYSYMBOL_BUILTIN_PAIRP = 22,             /* BUILTIN_PAIRP  */
  YYSYMBOL_LPAREN = 23,                    /* LPAREN  */
  YYSYMBOL_RPAREN = 24,                    /* RPAREN  */
  YYSYMBOL_QUOTE = 25,                     /* QUOTE  */
  YYSYMBOL_NIL_LIT = 26,                   /* NIL_LIT  */
  YYSYMBOL_INT_LIT = 27,                   /* INT_LIT  */
  YYSYMBOL_FLOAT_LIT = 28,                 /* FLOAT_LIT  */
  YYSYMBOL_BOOL_LIT = 29,                  /* BOOL_LIT  */
  YYSYMBOL_STRING_LIT = 30,                /* STRING_LIT  */
  YYSYMBOL_IDENT = 31,                     /* IDENT  */
  YYSYMBOL_OP_ARITH = 32,                  /* OP_ARITH  */
  YYSYMBOL_OP_REL = 33,                    /* OP_REL  */
  YYSYMBOL_YYACCEPT = 34,                  /* $accept  */
  YYSYMBOL_program = 35,                   /* program  */
  YYSYMBOL_expr_list = 36,                 /* expr_list  */
  YYSYMBOL_expr = 37,                      /* expr  */
  YYSYMBOL_38_1 = 38,                      /* $@1  */
  YYSYMBOL_39_2 = 39,                      /* $@2  */
  YYSYMBOL_40_3 = 40,                      /* $@3  */
  YYSYMBOL_41_4 = 41,                      /* $@4  */
  YYSYMBOL_body_list = 42,                 /* body_list  */
  YYSYMBOL_param_list = 43,                /* param_list  */
  YYSYMBOL_binding_list = 44,              /* binding_list  */
  YYSYMBOL_binding = 45,                   /* binding  */
  YYSYMBOL_cond_clauses = 46,              /* cond_clauses  */
  YYSYMBOL_cond_clause = 47                /* cond_clause  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   243

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  34
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  14
/* YYNRULES -- Number of rules.  */
#define YYNRULES  51
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  118

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   288


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   566,   566,   581,   585,   590,   599,   605,   611,   617,
     623,   629,   637,   644,   661,   660,   698,   706,   715,   723,
     722,   753,   752,   782,   781,   811,   818,   831,   842,   851,
     859,   868,   877,   887,   898,   907,   916,   942,   948,   953,
     963,   967,   978,   982,   991,   999,  1006,  1019,  1023,  1030,
    1038,  1044
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "KW_DEFINE", "KW_IF",
  "KW_COND", "KW_ELSE", "KW_LET", "KW_LETSTAR", "KW_LAMBDA", "KW_BEGIN",
  "KW_AND", "KW_OR", "KW_NOT", "KW_QUOTE", "BUILTIN_DISPLAY",
  "BUILTIN_NEWLINE", "BUILTIN_CAR", "BUILTIN_CDR", "BUILTIN_CONS",
  "BUILTIN_LIST", "BUILTIN_NULLP", "BUILTIN_PAIRP", "LPAREN", "RPAREN",
  "QUOTE", "NIL_LIT", "INT_LIT", "FLOAT_LIT", "BOOL_LIT", "STRING_LIT",
  "IDENT", "OP_ARITH", "OP_REL", "$accept", "program", "expr_list", "expr",
  "$@1", "$@2", "$@3", "$@4", "body_list", "param_list", "binding_list",
  "binding", "cond_clauses", "cond_clause", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-60)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-3)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -60,     2,    90,   -60,   -18,    56,   -60,   -60,   -60,   -60,
     -60,   -60,   -60,   -60,   -17,   -22,   212,   -60,   -13,    -6,
      -5,   212,   212,     1,   212,   212,   212,   -60,   212,   212,
     -60,   212,   -60,   -60,    -3,   212,   212,     7,   -60,   -60,
     -60,   -60,    99,     8,   -60,    15,    23,   212,   109,    24,
      28,   123,   212,   133,   -60,    29,   203,    31,   194,   -60,
     -60,    12,    14,   -20,    34,   -60,   -60,   -60,   -60,   -60,
      38,   -60,   -60,   -60,   -60,    43,   -60,   -19,   -60,   -60,
      44,   -60,   212,   212,    45,    39,   -60,   -60,   -60,   -60,
     -60,   -60,   -60,   -60,   -60,   -60,    68,    69,   -60,   185,
     212,   212,   212,   212,   -60,   -60,    70,    71,   142,   154,
     166,   175,   -60,   -60,   -60,   -60,   -60,   -60
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       3,     0,     0,     1,     0,     0,    10,     6,     7,     9,
       8,    11,     4,     5,     0,     0,     0,    47,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     3,     0,     0,
       3,     0,     3,    12,     0,     0,     0,     0,    42,    42,
      40,    37,     0,     0,    29,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,    18,
      48,     0,     0,     0,     0,    25,    38,    28,    30,    31,
       0,    33,    34,    35,    26,     0,    36,     0,    13,    17,
       0,    49,     0,     0,     0,     0,    19,    43,    21,    23,
      41,    39,    32,    27,    14,    16,     0,     0,    44,     0,
       0,     0,     0,     0,    50,    51,     0,     0,     0,     0,
       0,     0,    46,    45,    20,    22,    24,    15
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -60,   -60,   -11,    -2,   -60,   -60,   -60,   -60,   -59,    42,
      59,   -60,   -60,   -60
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     2,    41,   103,   100,   101,   102,    42,    63,
      61,    87,    37,    60
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      12,    34,     3,    32,    89,    94,    13,    33,    57,    35,
      38,    90,    90,    84,    36,    84,    48,    39,    40,    51,
      43,    53,    45,    46,    47,    44,    49,    50,    54,    52,
      58,    59,    67,    55,    56,    85,    86,    85,    88,    68,
      66,   108,   109,   110,   111,    70,    12,    69,    72,    12,
      75,    12,    73,    78,    80,    81,    83,    14,    91,    15,
      16,    17,    92,    18,    19,    20,    21,    93,    95,    98,
      99,    22,    23,    24,    25,    26,    27,    28,    29,     5,
      96,    97,     6,     7,     8,     9,    10,    11,    30,    31,
      -2,     4,   104,   105,   112,   113,    77,   107,    62,     0,
      64,     0,     0,     0,     0,     0,    66,    66,    66,    66,
       4,     0,     0,     5,     0,     0,     6,     7,     8,     9,
      10,    11,     5,    65,     4,     6,     7,     8,     9,    10,
      11,     0,     5,    71,     4,     6,     7,     8,     9,    10,
      11,     0,     0,    64,     0,     0,     5,    74,     0,     6,
       7,     8,     9,    10,    11,    64,     5,    76,     0,     6,
       7,     8,     9,    10,    11,     5,   114,    64,     6,     7,
       8,     9,    10,    11,     0,     0,    64,     5,   115,     0,
       6,     7,     8,     9,    10,    11,   106,     0,     0,     5,
     116,     0,     6,     7,     8,     9,    10,    11,     5,   117,
      82,     6,     7,     8,     9,    10,    11,     0,     5,     0,
       0,     6,     7,     8,     9,    10,    11,     5,     0,     0,
       6,     7,     8,     9,    10,    11,     5,    79,     0,     6,
       7,     8,     9,    10,    11,     5,     0,     0,     6,     7,
       8,     9,    10,    11
};

static const yytype_int8 yycheck[] =
{
       2,    23,     0,     5,    24,    24,    24,    24,     1,    31,
      23,    31,    31,     1,    16,     1,    27,    23,    23,    30,
      22,    32,    24,    25,    26,    24,    28,    29,    31,    31,
      23,    24,    24,    35,    36,    23,    24,    23,    24,    24,
      42,   100,   101,   102,   103,    47,    48,    24,    24,    51,
      52,    53,    24,    24,    56,    24,    58,     1,    24,     3,
       4,     5,    24,     7,     8,     9,    10,    24,    24,    24,
      31,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      82,    83,    26,    27,    28,    29,    30,    31,    32,    33,
       0,     1,    24,    24,    24,    24,    54,    99,    39,    -1,
       1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,
       1,    -1,    -1,    23,    -1,    -1,    26,    27,    28,    29,
      30,    31,    23,    24,     1,    26,    27,    28,    29,    30,
      31,    -1,    23,    24,     1,    26,    27,    28,    29,    30,
      31,    -1,    -1,     1,    -1,    -1,    23,    24,    -1,    26,
      27,    28,    29,    30,    31,     1,    23,    24,    -1,    26,
      27,    28,    29,    30,    31,    23,    24,     1,    26,    27,
      28,    29,    30,    31,    -1,    -1,     1,    23,    24,    -1,
      26,    27,    28,    29,    30,    31,     1,    -1,    -1,    23,
      24,    -1,    26,    27,    28,    29,    30,    31,    23,    24,
       6,    26,    27,    28,    29,    30,    31,    -1,    23,    -1,
      -1,    26,    27,    28,    29,    30,    31,    23,    -1,    -1,
      26,    27,    28,    29,    30,    31,    23,    24,    -1,    26,
      27,    28,    29,    30,    31,    23,    -1,    -1,    26,    27,
      28,    29,    30,    31
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    35,    36,     0,     1,    23,    26,    27,    28,    29,
      30,    31,    37,    24,     1,     3,     4,     5,     7,     8,
       9,    10,    15,    16,    17,    18,    19,    20,    21,    22,
      32,    33,    37,    24,    23,    31,    37,    46,    23,    23,
      23,    37,    42,    37,    24,    37,    37,    37,    36,    37,
      37,    36,    37,    36,    31,    37,    37,     1,    23,    24,
      47,    44,    44,    43,     1,    24,    37,    24,    24,    24,
      37,    24,    24,    24,    24,    37,    24,    43,    24,    24,
      37,    24,     6,    37,     1,    23,    24,    45,    24,    24,
      31,    24,    24,    24,    24,    24,    37,    37,    24,    31,
      39,    40,    41,    38,    24,    24,     1,    37,    42,    42,
      42,    42,    24,    24,    24,    24,    24,    24
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    34,    35,    36,    36,    36,    37,    37,    37,    37,
      37,    37,    37,    37,    38,    37,    37,    37,    37,    39,
      37,    40,    37,    41,    37,    37,    37,    37,    37,    37,
      37,    37,    37,    37,    37,    37,    37,    42,    42,    42,
      43,    43,    44,    44,    44,    45,    45,    46,    46,    46,
      47,    47
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     2,     3,     1,     1,     1,     1,
       1,     1,     3,     5,     0,     9,     6,     5,     4,     0,
       8,     0,     8,     0,     8,     4,     4,     5,     4,     3,
       4,     4,     5,     4,     4,     4,     4,     1,     2,     3,
       0,     2,     0,     2,     3,     4,     4,     0,     2,     3,
       4,     4
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: expr_list  */
#line 567 "parser.y"
        {
            auto prog = std::make_shared<ASTNode>(NodeType::PROGRAM, yylineno);
            // expr_list é um nó PROGRAM com children
            // Transfere os filhos
            ASTNode* raw = (yyvsp[0].node);
            auto wrapped = std::shared_ptr<ASTNode>(raw, [](ASTNode*){});
            prog->children = wrapped->children;
            programRoot = prog;
            (yyval.node) = prog.get();
        }
#line 2009 "parser.tab.cpp"
    break;

  case 3: /* expr_list: %empty  */
#line 581 "parser.y"
        {
            auto n = new ASTNode(NodeType::PROGRAM, yylineno);
            (yyval.node) = n;
        }
#line 2018 "parser.tab.cpp"
    break;

  case 4: /* expr_list: expr_list expr  */
#line 586 "parser.y"
        {
            (yyvsp[-1].node)->children.push_back(std::shared_ptr<ASTNode>((yyvsp[0].node), [](ASTNode*){}));
            (yyval.node) = (yyvsp[-1].node);
        }
#line 2027 "parser.tab.cpp"
    break;

  case 5: /* expr_list: expr_list error RPAREN  */
#line 591 "parser.y"
        {
            yyerrok;
            (yyval.node) = (yyvsp[-2].node);
        }
#line 2036 "parser.tab.cpp"
    break;

  case 6: /* expr: INT_LIT  */
#line 600 "parser.y"
        {
            auto n = new ASTNode(NodeType::INT_LIT, yylineno);
            n->ival = (yyvsp[0].ival); n->dataType = DataType::INT;
            (yyval.node) = n;
        }
#line 2046 "parser.tab.cpp"
    break;

  case 7: /* expr: FLOAT_LIT  */
#line 606 "parser.y"
        {
            auto n = new ASTNode(NodeType::FLOAT_LIT, yylineno);
            n->fval = (yyvsp[0].fval); n->dataType = DataType::FLOAT;
            (yyval.node) = n;
        }
#line 2056 "parser.tab.cpp"
    break;

  case 8: /* expr: STRING_LIT  */
#line 612 "parser.y"
        {
            auto n = new ASTNode(NodeType::STRING_LIT, yylineno);
            n->sval = (yyvsp[0].sval); n->dataType = DataType::STRING;
            (yyval.node) = n;
        }
#line 2066 "parser.tab.cpp"
    break;

  case 9: /* expr: BOOL_LIT  */
#line 618 "parser.y"
        {
            auto n = new ASTNode(NodeType::BOOL_LIT, yylineno);
            n->bval = (yyvsp[0].bval); n->dataType = DataType::BOOL;
            (yyval.node) = n;
        }
#line 2076 "parser.tab.cpp"
    break;

  case 10: /* expr: NIL_LIT  */
#line 624 "parser.y"
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            n->dataType = DataType::LIST;
            (yyval.node) = n;
        }
#line 2086 "parser.tab.cpp"
    break;

  case 11: /* expr: IDENT  */
#line 630 "parser.y"
        {
            auto n = new ASTNode(NodeType::IDENT, yylineno);
            n->sval = (yyvsp[0].sval);
            (yyval.node) = n;
        }
#line 2096 "parser.tab.cpp"
    break;

  case 12: /* expr: LPAREN error RPAREN  */
#line 638 "parser.y"
        {
            yyerrok;
            (yyval.node) = makeRecoveryNode(yylineno);
        }
#line 2105 "parser.tab.cpp"
    break;

  case 13: /* expr: LPAREN KW_DEFINE IDENT expr RPAREN  */
#line 645 "parser.y"
        {
            auto n = new ASTNode(NodeType::DEFINE_VAR, yylineno);
            n->sval = (yyvsp[-2].sval);
            auto child = std::shared_ptr<ASTNode>((yyvsp[-1].node), [](ASTNode*){});
            n->children.push_back(child);
            n->dataType = DataType::VOID;
            // Registra na tabela de símbolos
            SymbolInfo si;
            si.name = (yyvsp[-2].sval); si.dataType = child->dataType;
            si.isFunction = false; si.definedAt = yylineno;
            symTable.define((yyvsp[-2].sval), si);
            (yyval.node) = n;
        }
#line 2123 "parser.tab.cpp"
    break;

  case 14: /* $@1: %empty  */
#line 661 "parser.y"
        {
            // Registra a função ANTES do corpo (permite recursão)
            SymbolInfo fsi;
            fsi.name = (yyvsp[-2].sval); fsi.dataType = DataType::FUNCTION;
            fsi.isFunction = true; fsi.arity = (int)(yyvsp[-1].node)->params.size();
            fsi.definedAt = yylineno;
            symTable.define((yyvsp[-2].sval), fsi);
            // Abre escopo para o corpo da função
            symTable.pushScope();
            // Registra parâmetros
            ASTNode* plist = (yyvsp[-1].node);
            for (auto& p : plist->params) {
                SymbolInfo si;
                si.name = p; si.dataType = DataType::ANY;
                si.isFunction = false; si.definedAt = yylineno;
                symTable.define(p, si);
            }
        }
#line 2146 "parser.tab.cpp"
    break;

  case 15: /* expr: LPAREN KW_DEFINE LPAREN IDENT param_list RPAREN $@1 body_list RPAREN  */
#line 680 "parser.y"
        {
            auto n = new ASTNode(NodeType::DEFINE_FUNC, yylineno);
            n->funcName = (yyvsp[-5].sval);
            n->params = (yyvsp[-4].node)->params;
            // Filhos = body
            ASTNode* body = (yyvsp[-1].node);
            for (auto& c : body->children)
                n->children.push_back(c);
            n->dataType = DataType::FUNCTION;
            symTable.popScope();
            // Atualiza aridade correta agora que params foram lidos
            if (SymbolInfo* si = symTable.lookup((yyvsp[-5].sval)))
                si->arity = (int)n->params.size();
            delete (yyvsp[-4].node); delete body;
            (yyval.node) = n;
        }
#line 2167 "parser.tab.cpp"
    break;

  case 16: /* expr: LPAREN KW_IF expr expr expr RPAREN  */
#line 699 "parser.y"
        {
            auto n = new ASTNode(NodeType::IF_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-3].node), [](ASTNode*){}));
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-2].node), [](ASTNode*){}));
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-1].node), [](ASTNode*){}));
            (yyval.node) = n;
        }
#line 2179 "parser.tab.cpp"
    break;

  case 17: /* expr: LPAREN KW_IF expr expr RPAREN  */
#line 707 "parser.y"
        {
            auto n = new ASTNode(NodeType::IF_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-2].node), [](ASTNode*){}));
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-1].node), [](ASTNode*){}));
            (yyval.node) = n;
        }
#line 2190 "parser.tab.cpp"
    break;

  case 18: /* expr: LPAREN KW_COND cond_clauses RPAREN  */
#line 716 "parser.y"
        {
            (yyvsp[-1].node)->type = NodeType::COND_EXPR;
            (yyval.node) = (yyvsp[-1].node);
        }
#line 2199 "parser.tab.cpp"
    break;

  case 19: /* $@2: %empty  */
#line 723 "parser.y"
        {
            symTable.pushScope();
            ASTNode* bl = (yyvsp[-1].node);
            for (auto& b : bl->bindings) {
                SymbolInfo si;
                si.name = b.first; si.dataType = b.second->dataType;
                si.definedAt = yylineno;
                symTable.define(b.first, si);
            }
        }
#line 2214 "parser.tab.cpp"
    break;

  case 20: /* expr: LPAREN KW_LET LPAREN binding_list RPAREN $@2 body_list RPAREN  */
#line 734 "parser.y"
        {
            auto n = new ASTNode(NodeType::LET_EXPR, yylineno);
            n->bindings = (yyvsp[-4].node)->bindings;
            ASTNode* body = (yyvsp[-1].node);
            // Corpo: se múltiplos, último é o valor
            if (body->children.size() == 1)
                n->children.push_back(body->children[0]);
            else {
                auto bg = std::make_shared<ASTNode>(NodeType::BEGIN_EXPR, yylineno);
                bg->children = body->children;
                n->children.push_back(bg);
            }
            symTable.popScope();
            delete (yyvsp[-4].node); delete body;
            (yyval.node) = n;
        }
#line 2235 "parser.tab.cpp"
    break;

  case 21: /* $@3: %empty  */
#line 753 "parser.y"
        {
            symTable.pushScope();
            ASTNode* bl = (yyvsp[-1].node);
            for (auto& b : bl->bindings) {
                SymbolInfo si;
                si.name = b.first; si.dataType = b.second->dataType;
                si.definedAt = yylineno;
                symTable.define(b.first, si);
            }
        }
#line 2250 "parser.tab.cpp"
    break;

  case 22: /* expr: LPAREN KW_LETSTAR LPAREN binding_list RPAREN $@3 body_list RPAREN  */
#line 764 "parser.y"
        {
            auto n = new ASTNode(NodeType::LETSTAR_EXPR, yylineno);
            n->bindings = (yyvsp[-4].node)->bindings;
            ASTNode* body = (yyvsp[-1].node);
            if (body->children.size() == 1)
                n->children.push_back(body->children[0]);
            else {
                auto bg = std::make_shared<ASTNode>(NodeType::BEGIN_EXPR, yylineno);
                bg->children = body->children;
                n->children.push_back(bg);
            }
            symTable.popScope();
            delete (yyvsp[-4].node); delete body;
            (yyval.node) = n;
        }
#line 2270 "parser.tab.cpp"
    break;

  case 23: /* $@4: %empty  */
#line 782 "parser.y"
        {
            symTable.pushScope();
            ASTNode* pl = (yyvsp[-1].node);
            for (auto& p : pl->params) {
                SymbolInfo si;
                si.name = p; si.dataType = DataType::ANY;
                si.definedAt = yylineno;
                symTable.define(p, si);
            }
        }
#line 2285 "parser.tab.cpp"
    break;

  case 24: /* expr: LPAREN KW_LAMBDA LPAREN param_list RPAREN $@4 body_list RPAREN  */
#line 793 "parser.y"
        {
            auto n = new ASTNode(NodeType::LAMBDA_EXPR, yylineno);
            n->params = (yyvsp[-4].node)->params;
            ASTNode* body = (yyvsp[-1].node);
            if (body->children.size() == 1)
                n->children.push_back(body->children[0]);
            else {
                auto bg = std::make_shared<ASTNode>(NodeType::BEGIN_EXPR, yylineno);
                bg->children = body->children;
                n->children.push_back(bg);
            }
            n->dataType = DataType::FUNCTION;
            symTable.popScope();
            delete (yyvsp[-4].node); delete body;
            (yyval.node) = n;
        }
#line 2306 "parser.tab.cpp"
    break;

  case 25: /* expr: LPAREN KW_BEGIN body_list RPAREN  */
#line 812 "parser.y"
        {
            (yyvsp[-1].node)->type = NodeType::BEGIN_EXPR;
            (yyval.node) = (yyvsp[-1].node);
        }
#line 2315 "parser.tab.cpp"
    break;

  case 26: /* expr: LPAREN OP_ARITH expr_list RPAREN  */
#line 819 "parser.y"
        {
            auto n = new ASTNode(NodeType::BINOP, yylineno);
            n->op = (yyvsp[-2].sval); free((yyvsp[-2].sval));
            ASTNode* args = (yyvsp[-1].node);
            n->children = args->children;
            // Inferência de tipo simples
            n->dataType = DataType::ANY;
            delete args;
            (yyval.node) = n;
        }
#line 2330 "parser.tab.cpp"
    break;

  case 27: /* expr: LPAREN OP_REL expr expr RPAREN  */
#line 832 "parser.y"
        {
            auto n = new ASTNode(NodeType::BINOP, yylineno);
            n->op = (yyvsp[-3].sval); free((yyvsp[-3].sval));
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-2].node), [](ASTNode*){}));
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-1].node), [](ASTNode*){}));
            n->dataType = DataType::BOOL;
            (yyval.node) = n;
        }
#line 2343 "parser.tab.cpp"
    break;

  case 28: /* expr: LPAREN BUILTIN_DISPLAY expr RPAREN  */
#line 843 "parser.y"
        {
            auto n = new ASTNode(NodeType::DISPLAY_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-1].node), [](ASTNode*){}));
            n->dataType = DataType::VOID;
            (yyval.node) = n;
        }
#line 2354 "parser.tab.cpp"
    break;

  case 29: /* expr: LPAREN BUILTIN_NEWLINE RPAREN  */
#line 852 "parser.y"
        {
            auto n = new ASTNode(NodeType::NEWLINE_EXPR, yylineno);
            n->dataType = DataType::VOID;
            (yyval.node) = n;
        }
#line 2364 "parser.tab.cpp"
    break;

  case 30: /* expr: LPAREN BUILTIN_CAR expr RPAREN  */
#line 860 "parser.y"
        {
            auto n = new ASTNode(NodeType::CAR_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-1].node), [](ASTNode*){}));
            n->dataType = DataType::ANY;
            (yyval.node) = n;
        }
#line 2375 "parser.tab.cpp"
    break;

  case 31: /* expr: LPAREN BUILTIN_CDR expr RPAREN  */
#line 869 "parser.y"
        {
            auto n = new ASTNode(NodeType::CDR_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-1].node), [](ASTNode*){}));
            n->dataType = DataType::LIST;
            (yyval.node) = n;
        }
#line 2386 "parser.tab.cpp"
    break;

  case 32: /* expr: LPAREN BUILTIN_CONS expr expr RPAREN  */
#line 878 "parser.y"
        {
            auto n = new ASTNode(NodeType::CONS_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-2].node), [](ASTNode*){}));
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-1].node), [](ASTNode*){}));
            n->dataType = DataType::LIST;
            (yyval.node) = n;
        }
#line 2398 "parser.tab.cpp"
    break;

  case 33: /* expr: LPAREN BUILTIN_LIST expr_list RPAREN  */
#line 888 "parser.y"
        {
            ASTNode* args = (yyvsp[-1].node);
            auto n = new ASTNode(NodeType::LIST_EXPR, yylineno);
            n->children = args->children;
            n->dataType = DataType::LIST;
            delete args;
            (yyval.node) = n;
        }
#line 2411 "parser.tab.cpp"
    break;

  case 34: /* expr: LPAREN BUILTIN_NULLP expr RPAREN  */
#line 899 "parser.y"
        {
            auto n = new ASTNode(NodeType::NULL_CHECK, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-1].node), [](ASTNode*){}));
            n->dataType = DataType::BOOL;
            (yyval.node) = n;
        }
#line 2422 "parser.tab.cpp"
    break;

  case 35: /* expr: LPAREN BUILTIN_PAIRP expr RPAREN  */
#line 908 "parser.y"
        {
            auto n = new ASTNode(NodeType::PAIR_CHECK, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-1].node), [](ASTNode*){}));
            n->dataType = DataType::BOOL;
            (yyval.node) = n;
        }
#line 2433 "parser.tab.cpp"
    break;

  case 36: /* expr: LPAREN expr expr_list RPAREN  */
#line 917 "parser.y"
        {
            auto n = new ASTNode(NodeType::CALL_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-2].node), [](ASTNode*){}));
            ASTNode* args = (yyvsp[-1].node);
            for (auto& c : args->children)
                n->children.push_back(c);
            // Verificação de aridade (se for identificador de função conhecida)
            if ((yyvsp[-2].node)->type == NodeType::IDENT) {
                SymbolInfo* si = symTable.lookup((yyvsp[-2].node)->sval);
                if (si && si->isFunction && si->arity >= 0) {
                    int given = (int)args->children.size();
                    if (given != si->arity)
                        typeError("Função '" + (yyvsp[-2].node)->sval + "' espera " +
                                  std::to_string(si->arity) + " argumento(s), mas recebeu " +
                                  std::to_string(given), yylineno);
                }
            }
            n->dataType = DataType::ANY;
            delete args;
            (yyval.node) = n;
        }
#line 2459 "parser.tab.cpp"
    break;

  case 37: /* body_list: expr  */
#line 943 "parser.y"
        {
            auto n = new ASTNode(NodeType::BEGIN_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[0].node), [](ASTNode*){}));
            (yyval.node) = n;
        }
#line 2469 "parser.tab.cpp"
    break;

  case 38: /* body_list: body_list expr  */
#line 949 "parser.y"
        {
            (yyvsp[-1].node)->children.push_back(std::shared_ptr<ASTNode>((yyvsp[0].node), [](ASTNode*){}));
            (yyval.node) = (yyvsp[-1].node);
        }
#line 2478 "parser.tab.cpp"
    break;

  case 39: /* body_list: body_list error RPAREN  */
#line 954 "parser.y"
        {
            yyerrok;
            (yyval.node) = (yyvsp[-2].node);
        }
#line 2487 "parser.tab.cpp"
    break;

  case 40: /* param_list: %empty  */
#line 963 "parser.y"
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            (yyval.node) = n;
        }
#line 2496 "parser.tab.cpp"
    break;

  case 41: /* param_list: param_list IDENT  */
#line 968 "parser.y"
        {
            (yyvsp[-1].node)->params.push_back((yyvsp[0].sval));
            free((yyvsp[0].sval));
            (yyval.node) = (yyvsp[-1].node);
        }
#line 2506 "parser.tab.cpp"
    break;

  case 42: /* binding_list: %empty  */
#line 978 "parser.y"
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            (yyval.node) = n;
        }
#line 2515 "parser.tab.cpp"
    break;

  case 43: /* binding_list: binding_list binding  */
#line 983 "parser.y"
        {
            auto pair = std::make_pair((yyvsp[0].node)->sval,
                            std::shared_ptr<ASTNode>((yyvsp[0].node)->children[0].get(), [](ASTNode*){}));
            // Copia o filho corretamente
            (yyvsp[-1].node)->bindings.push_back({(yyvsp[0].node)->sval, (yyvsp[0].node)->children[0]});
            delete (yyvsp[0].node);
            (yyval.node) = (yyvsp[-1].node);
        }
#line 2528 "parser.tab.cpp"
    break;

  case 44: /* binding_list: binding_list error RPAREN  */
#line 992 "parser.y"
        {
            yyerrok;
            (yyval.node) = (yyvsp[-2].node);
        }
#line 2537 "parser.tab.cpp"
    break;

  case 45: /* binding: LPAREN IDENT expr RPAREN  */
#line 1000 "parser.y"
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            n->sval = (yyvsp[-2].sval); free((yyvsp[-2].sval));
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-1].node), [](ASTNode*){}));
            (yyval.node) = n;
        }
#line 2548 "parser.tab.cpp"
    break;

  case 46: /* binding: LPAREN IDENT error RPAREN  */
#line 1007 "parser.y"
        {
            yyerrok;
            auto n = new ASTNode(NodeType::NIL, yylineno);
            n->sval = (yyvsp[-2].sval); free((yyvsp[-2].sval));
            n->children.push_back(std::shared_ptr<ASTNode>(makeRecoveryNode(yylineno)));
            (yyval.node) = n;
        }
#line 2560 "parser.tab.cpp"
    break;

  case 47: /* cond_clauses: %empty  */
#line 1019 "parser.y"
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            (yyval.node) = n;
        }
#line 2569 "parser.tab.cpp"
    break;

  case 48: /* cond_clauses: cond_clauses cond_clause  */
#line 1024 "parser.y"
        {
            for (auto& c : (yyvsp[0].node)->children)
                (yyvsp[-1].node)->children.push_back(c);
            delete (yyvsp[0].node);
            (yyval.node) = (yyvsp[-1].node);
        }
#line 2580 "parser.tab.cpp"
    break;

  case 49: /* cond_clauses: cond_clauses error RPAREN  */
#line 1031 "parser.y"
        {
            yyerrok;
            (yyval.node) = (yyvsp[-2].node);
        }
#line 2589 "parser.tab.cpp"
    break;

  case 50: /* cond_clause: LPAREN KW_ELSE expr RPAREN  */
#line 1039 "parser.y"
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-1].node), [](ASTNode*){}));
            (yyval.node) = n;
        }
#line 2599 "parser.tab.cpp"
    break;

  case 51: /* cond_clause: LPAREN expr expr RPAREN  */
#line 1045 "parser.y"
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-2].node), [](ASTNode*){}));
            n->children.push_back(std::shared_ptr<ASTNode>((yyvsp[-1].node), [](ASTNode*){}));
            (yyval.node) = n;
        }
#line 2610 "parser.tab.cpp"
    break;


#line 2614 "parser.tab.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 1053 "parser.y"


/* -------- Implementações C++ -------- */

void yyerror(const char* msg) {
    collectDiagnostic("[Erro de sintaxe]", translateSyntaxMessage(msg), yylineno, true);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.scm> [saida.py]\n", argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "Erro: não foi possível abrir '%s'\n", argv[1]);
        return 1;
    }
    yyin = f;

    // Cabeçalho Python
    pythonOutput << "# Código gerado automaticamente por scheme2python\n";
    pythonOutput << "# Fonte: " << argv[1] << "\n\n";

    int parseResult = yyparse();
    fclose(f);

    if (parseResult != 0 || errorCount > 0) {
        flushDiagnostics();
        fprintf(stderr, "Compilação falhou.\n");
        return 1;
    }

    // Validação semântica com escopos corretos
    {
        SymbolTable validateTable = symTable;
        validateNode(programRoot, validateTable);
    }

    if (errorCount > 0) {
        flushDiagnostics();
        fprintf(stderr, "Compilação falhou.\n");
        return 1;
    }

    // Gera código Python a partir da AST
    if (programRoot) {
        genStmt(programRoot);
    }

    // Saída
    std::string output = pythonOutput.str();
    if (argc >= 3) {
        FILE* out = fopen(argv[2], "w");
        if (!out) {
            fprintf(stderr, "Erro: não foi possível criar '%s'\n", argv[2]);
            return 1;
        }
        fprintf(out, "%s", output.c_str());
        fclose(out);
        fprintf(stderr, "Gerado: %s\n", argv[2]);
    } else {
        printf("%s", output.c_str());
    }

    flushDiagnostics();

    return 0;
}
