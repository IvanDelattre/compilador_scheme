#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>

// Tipos de nós da AST
enum class NodeType {
    // Literais
    INT_LIT,
    FLOAT_LIT,
    STRING_LIT,
    BOOL_LIT,
    NIL,

    // Identificador
    IDENT,

    // Expressões
    BINOP,
    UNOP,
    IF_EXPR,
    COND_EXPR,
    LET_EXPR,
    LETSTAR_EXPR,
    LAMBDA_EXPR,
    CALL_EXPR,
    DISPLAY_EXPR,
    NEWLINE_EXPR,
    BEGIN_EXPR,

    // Listas
    LIST_EXPR,
    CAR_EXPR,
    CDR_EXPR,
    CONS_EXPR,
    NULL_CHECK,
    PAIR_CHECK,

    // Definições
    DEFINE_VAR,
    DEFINE_FUNC,

    // Programa
    PROGRAM
};

// Tipos de dados para verificação de tipos
enum class DataType {
    INT,
    FLOAT,
    STRING,
    BOOL,
    LIST,
    FUNCTION,
    ANY,
    VOID,
    UNKNOWN
};

std::string dataTypeToString(DataType t);

// Nó da AST
struct ASTNode {
    NodeType type;
    DataType dataType = DataType::UNKNOWN;

    // Valores literais
    int         ival = 0;
    double      fval = 0.0;
    std::string sval;
    bool        bval = false;

    // Filhos
    std::vector<std::shared_ptr<ASTNode>> children;

    // Para operadores binários/unários
    std::string op;

    // Para define de função: nome + lista de parâmetros
    std::string funcName;
    std::vector<std::string> params;

    // Para let: lista de bindings (nome, expr)
    std::vector<std::pair<std::string, std::shared_ptr<ASTNode>>> bindings;

    // Linha para mensagens de erro
    int line = 0;

    ASTNode(NodeType t, int ln = 0) : type(t), line(ln) {}
};

using ASTNodePtr = std::shared_ptr<ASTNode>;

// Helpers para criar nós
ASTNodePtr makeInt(int v, int line = 0);
ASTNodePtr makeFloat(double v, int line = 0);
ASTNodePtr makeString(const std::string& v, int line = 0);
ASTNodePtr makeBool(bool v, int line = 0);
ASTNodePtr makeNil(int line = 0);
ASTNodePtr makeIdent(const std::string& name, int line = 0);

#endif // AST_H
