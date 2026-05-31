%{
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

%}

/* -------- Declarações Bison -------- */
%union {
    int    ival;
    double fval;
    bool   bval;
    char*  sval;
    ASTNode* node;
}

/* Tokens */
%token KW_DEFINE KW_IF KW_COND KW_ELSE KW_LET KW_LETSTAR KW_LAMBDA
%token KW_BEGIN KW_AND KW_OR KW_NOT KW_QUOTE
%token BUILTIN_DISPLAY BUILTIN_NEWLINE BUILTIN_CAR BUILTIN_CDR
%token BUILTIN_CONS BUILTIN_LIST BUILTIN_NULLP BUILTIN_PAIRP
%token LPAREN RPAREN QUOTE NIL_LIT

%define parse.error verbose

%token <ival>  INT_LIT
%token <fval>  FLOAT_LIT
%token <bval>  BOOL_LIT
%token <sval>  STRING_LIT IDENT OP_ARITH OP_REL

%type  <node>  program expr expr_list
%type  <node>  param_list binding_list binding body_list
%type  <node>  cond_clauses cond_clause

%start program

%%

/* -------- Gramática -------- */

program
    : expr_list
        {
            auto prog = std::make_shared<ASTNode>(NodeType::PROGRAM, yylineno);
            // expr_list é um nó PROGRAM com children
            // Transfere os filhos
            ASTNode* raw = $1;
            auto wrapped = std::shared_ptr<ASTNode>(raw, [](ASTNode*){});
            prog->children = wrapped->children;
            programRoot = prog;
            $$ = prog.get();
        }
    ;

expr_list
    : /* vazio */
        {
            auto n = new ASTNode(NodeType::PROGRAM, yylineno);
            $$ = n;
        }
    | expr_list expr
        {
            $1->children.push_back(std::shared_ptr<ASTNode>($2, [](ASTNode*){}));
            $$ = $1;
        }
    | expr_list error RPAREN
        {
            yyerrok;
            $$ = $1;
        }
    ;

expr
    /* Literais */
    : INT_LIT
        {
            auto n = new ASTNode(NodeType::INT_LIT, yylineno);
            n->ival = $1; n->dataType = DataType::INT;
            $$ = n;
        }
    | FLOAT_LIT
        {
            auto n = new ASTNode(NodeType::FLOAT_LIT, yylineno);
            n->fval = $1; n->dataType = DataType::FLOAT;
            $$ = n;
        }
    | STRING_LIT
        {
            auto n = new ASTNode(NodeType::STRING_LIT, yylineno);
            n->sval = $1; n->dataType = DataType::STRING;
            $$ = n;
        }
    | BOOL_LIT
        {
            auto n = new ASTNode(NodeType::BOOL_LIT, yylineno);
            n->bval = $1; n->dataType = DataType::BOOL;
            $$ = n;
        }
    | NIL_LIT
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            n->dataType = DataType::LIST;
            $$ = n;
        }
    | IDENT
        {
            auto n = new ASTNode(NodeType::IDENT, yylineno);
            n->sval = $1;
            $$ = n;
        }

    /* Forma parênteseada malformada */
    | LPAREN error RPAREN
        {
            yyerrok;
            $$ = makeRecoveryNode(yylineno);
        }

    /* Define variável: (define x expr) */
    | LPAREN KW_DEFINE IDENT expr RPAREN
        {
            auto n = new ASTNode(NodeType::DEFINE_VAR, yylineno);
            n->sval = $3;
            auto child = std::shared_ptr<ASTNode>($4, [](ASTNode*){});
            n->children.push_back(child);
            n->dataType = DataType::VOID;
            // Registra na tabela de símbolos
            SymbolInfo si;
            si.name = $3; si.dataType = child->dataType;
            si.isFunction = false; si.definedAt = yylineno;
            symTable.define($3, si);
            $$ = n;
        }

    /* Define função: (define (f p1 p2 ...) body...) */
    | LPAREN KW_DEFINE LPAREN IDENT param_list RPAREN
        {
            // Registra a função ANTES do corpo (permite recursão)
            SymbolInfo fsi;
            fsi.name = $4; fsi.dataType = DataType::FUNCTION;
            fsi.isFunction = true; fsi.arity = (int)$5->params.size();
            fsi.definedAt = yylineno;
            symTable.define($4, fsi);
            // Abre escopo para o corpo da função
            symTable.pushScope();
            // Registra parâmetros
            ASTNode* plist = $5;
            for (auto& p : plist->params) {
                SymbolInfo si;
                si.name = p; si.dataType = DataType::ANY;
                si.isFunction = false; si.definedAt = yylineno;
                symTable.define(p, si);
            }
        }
      body_list RPAREN
        {
            auto n = new ASTNode(NodeType::DEFINE_FUNC, yylineno);
            n->funcName = $4;
            n->params = $5->params;
            // Filhos = body
            ASTNode* body = $8;
            for (auto& c : body->children)
                n->children.push_back(c);
            n->dataType = DataType::FUNCTION;
            symTable.popScope();
            // Atualiza aridade correta agora que params foram lidos
            if (SymbolInfo* si = symTable.lookup($4))
                si->arity = (int)n->params.size();
            delete $5; delete body;
            $$ = n;
        }

    /* if */
    | LPAREN KW_IF expr expr expr RPAREN
        {
            auto n = new ASTNode(NodeType::IF_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>($3, [](ASTNode*){}));
            n->children.push_back(std::shared_ptr<ASTNode>($4, [](ASTNode*){}));
            n->children.push_back(std::shared_ptr<ASTNode>($5, [](ASTNode*){}));
            $$ = n;
        }
    | LPAREN KW_IF expr expr RPAREN
        {
            auto n = new ASTNode(NodeType::IF_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>($3, [](ASTNode*){}));
            n->children.push_back(std::shared_ptr<ASTNode>($4, [](ASTNode*){}));
            $$ = n;
        }

    /* cond */
    | LPAREN KW_COND cond_clauses RPAREN
        {
            $3->type = NodeType::COND_EXPR;
            $$ = $3;
        }

    /* let */
    | LPAREN KW_LET LPAREN binding_list RPAREN
        {
            symTable.pushScope();
            ASTNode* bl = $4;
            for (auto& b : bl->bindings) {
                SymbolInfo si;
                si.name = b.first; si.dataType = b.second->dataType;
                si.definedAt = yylineno;
                symTable.define(b.first, si);
            }
        }
      body_list RPAREN
        {
            auto n = new ASTNode(NodeType::LET_EXPR, yylineno);
            n->bindings = $4->bindings;
            ASTNode* body = $7;
            // Corpo: se múltiplos, último é o valor
            if (body->children.size() == 1)
                n->children.push_back(body->children[0]);
            else {
                auto bg = std::make_shared<ASTNode>(NodeType::BEGIN_EXPR, yylineno);
                bg->children = body->children;
                n->children.push_back(bg);
            }
            symTable.popScope();
            delete $4; delete body;
            $$ = n;
        }

    /* let* */
    | LPAREN KW_LETSTAR LPAREN binding_list RPAREN
        {
            symTable.pushScope();
            ASTNode* bl = $4;
            for (auto& b : bl->bindings) {
                SymbolInfo si;
                si.name = b.first; si.dataType = b.second->dataType;
                si.definedAt = yylineno;
                symTable.define(b.first, si);
            }
        }
      body_list RPAREN
        {
            auto n = new ASTNode(NodeType::LETSTAR_EXPR, yylineno);
            n->bindings = $4->bindings;
            ASTNode* body = $7;
            if (body->children.size() == 1)
                n->children.push_back(body->children[0]);
            else {
                auto bg = std::make_shared<ASTNode>(NodeType::BEGIN_EXPR, yylineno);
                bg->children = body->children;
                n->children.push_back(bg);
            }
            symTable.popScope();
            delete $4; delete body;
            $$ = n;
        }

    /* lambda */
    | LPAREN KW_LAMBDA LPAREN param_list RPAREN
        {
            symTable.pushScope();
            ASTNode* pl = $4;
            for (auto& p : pl->params) {
                SymbolInfo si;
                si.name = p; si.dataType = DataType::ANY;
                si.definedAt = yylineno;
                symTable.define(p, si);
            }
        }
      body_list RPAREN
        {
            auto n = new ASTNode(NodeType::LAMBDA_EXPR, yylineno);
            n->params = $4->params;
            ASTNode* body = $7;
            if (body->children.size() == 1)
                n->children.push_back(body->children[0]);
            else {
                auto bg = std::make_shared<ASTNode>(NodeType::BEGIN_EXPR, yylineno);
                bg->children = body->children;
                n->children.push_back(bg);
            }
            n->dataType = DataType::FUNCTION;
            symTable.popScope();
            delete $4; delete body;
            $$ = n;
        }

    /* begin */
    | LPAREN KW_BEGIN body_list RPAREN
        {
            $3->type = NodeType::BEGIN_EXPR;
            $$ = $3;
        }

    /* Operadores aritméticos: (+ a b ...) */
    | LPAREN OP_ARITH expr_list RPAREN
        {
            auto n = new ASTNode(NodeType::BINOP, yylineno);
            n->op = $2; free($2);
            ASTNode* args = $3;
            n->children = args->children;
            // Inferência de tipo simples
            n->dataType = DataType::ANY;
            delete args;
            $$ = n;
        }

    /* Operadores relacionais: (= a b) */
    | LPAREN OP_REL expr expr RPAREN
        {
            auto n = new ASTNode(NodeType::BINOP, yylineno);
            n->op = $2; free($2);
            n->children.push_back(std::shared_ptr<ASTNode>($3, [](ASTNode*){}));
            n->children.push_back(std::shared_ptr<ASTNode>($4, [](ASTNode*){}));
            n->dataType = DataType::BOOL;
            $$ = n;
        }

    /* display */
    | LPAREN BUILTIN_DISPLAY expr RPAREN
        {
            auto n = new ASTNode(NodeType::DISPLAY_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>($3, [](ASTNode*){}));
            n->dataType = DataType::VOID;
            $$ = n;
        }

    /* newline */
    | LPAREN BUILTIN_NEWLINE RPAREN
        {
            auto n = new ASTNode(NodeType::NEWLINE_EXPR, yylineno);
            n->dataType = DataType::VOID;
            $$ = n;
        }

    /* car */
    | LPAREN BUILTIN_CAR expr RPAREN
        {
            auto n = new ASTNode(NodeType::CAR_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>($3, [](ASTNode*){}));
            n->dataType = DataType::ANY;
            $$ = n;
        }

    /* cdr */
    | LPAREN BUILTIN_CDR expr RPAREN
        {
            auto n = new ASTNode(NodeType::CDR_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>($3, [](ASTNode*){}));
            n->dataType = DataType::LIST;
            $$ = n;
        }

    /* cons */
    | LPAREN BUILTIN_CONS expr expr RPAREN
        {
            auto n = new ASTNode(NodeType::CONS_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>($3, [](ASTNode*){}));
            n->children.push_back(std::shared_ptr<ASTNode>($4, [](ASTNode*){}));
            n->dataType = DataType::LIST;
            $$ = n;
        }

    /* list */
    | LPAREN BUILTIN_LIST expr_list RPAREN
        {
            ASTNode* args = $3;
            auto n = new ASTNode(NodeType::LIST_EXPR, yylineno);
            n->children = args->children;
            n->dataType = DataType::LIST;
            delete args;
            $$ = n;
        }

    /* null? */
    | LPAREN BUILTIN_NULLP expr RPAREN
        {
            auto n = new ASTNode(NodeType::NULL_CHECK, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>($3, [](ASTNode*){}));
            n->dataType = DataType::BOOL;
            $$ = n;
        }

    /* pair? */
    | LPAREN BUILTIN_PAIRP expr RPAREN
        {
            auto n = new ASTNode(NodeType::PAIR_CHECK, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>($3, [](ASTNode*){}));
            n->dataType = DataType::BOOL;
            $$ = n;
        }

    /* Chamada de função genérica: (f arg1 arg2 ...) */
    | LPAREN expr expr_list RPAREN
        {
            auto n = new ASTNode(NodeType::CALL_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>($2, [](ASTNode*){}));
            ASTNode* args = $3;
            for (auto& c : args->children)
                n->children.push_back(c);
            // Verificação de aridade (se for identificador de função conhecida)
            if ($2->type == NodeType::IDENT) {
                SymbolInfo* si = symTable.lookup($2->sval);
                if (si && si->isFunction && si->arity >= 0) {
                    int given = (int)args->children.size();
                    if (given != si->arity)
                        typeError("Função '" + $2->sval + "' espera " +
                                  std::to_string(si->arity) + " argumento(s), mas recebeu " +
                                  std::to_string(given), yylineno);
                }
            }
            n->dataType = DataType::ANY;
            delete args;
            $$ = n;
        }
    ;

/* Lista de expressões (usada em corpo e args) */
body_list
    : expr
        {
            auto n = new ASTNode(NodeType::BEGIN_EXPR, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>($1, [](ASTNode*){}));
            $$ = n;
        }
    | body_list expr
        {
            $1->children.push_back(std::shared_ptr<ASTNode>($2, [](ASTNode*){}));
            $$ = $1;
        }
    | body_list error RPAREN
        {
            yyerrok;
            $$ = $1;
        }
    ;

/* Lista de parâmetros formais */
param_list
    : /* vazio */
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            $$ = n;
        }
    | param_list IDENT
        {
            $1->params.push_back($2);
            free($2);
            $$ = $1;
        }
    ;

/* Bindings do let */
binding_list
    : /* vazio */
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            $$ = n;
        }
    | binding_list binding
        {
            auto pair = std::make_pair($2->sval,
                            std::shared_ptr<ASTNode>($2->children[0].get(), [](ASTNode*){}));
            // Copia o filho corretamente
            $1->bindings.push_back({$2->sval, $2->children[0]});
            delete $2;
            $$ = $1;
        }
    | binding_list error RPAREN
        {
            yyerrok;
            $$ = $1;
        }
    ;

binding
    : LPAREN IDENT expr RPAREN
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            n->sval = $2; free($2);
            n->children.push_back(std::shared_ptr<ASTNode>($3, [](ASTNode*){}));
            $$ = n;
        }
    | LPAREN IDENT error RPAREN
        {
            yyerrok;
            auto n = new ASTNode(NodeType::NIL, yylineno);
            n->sval = $2; free($2);
            n->children.push_back(std::shared_ptr<ASTNode>(makeRecoveryNode(yylineno)));
            $$ = n;
        }
    ;

/* Cláusulas do cond */
cond_clauses
    : /* vazio */
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            $$ = n;
        }
    | cond_clauses cond_clause
        {
            for (auto& c : $2->children)
                $1->children.push_back(c);
            delete $2;
            $$ = $1;
        }
    | cond_clauses error RPAREN
        {
            yyerrok;
            $$ = $1;
        }
    ;

cond_clause
    : LPAREN KW_ELSE expr RPAREN
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>($3, [](ASTNode*){}));
            $$ = n;
        }
    | LPAREN expr expr RPAREN
        {
            auto n = new ASTNode(NodeType::NIL, yylineno);
            n->children.push_back(std::shared_ptr<ASTNode>($2, [](ASTNode*){}));
            n->children.push_back(std::shared_ptr<ASTNode>($3, [](ASTNode*){}));
            $$ = n;
        }
    ;

%%

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
