#include "ast.h"

std::string dataTypeToString(DataType t) {
    switch(t) {
        case DataType::INT:      return "int";
        case DataType::FLOAT:    return "float";
        case DataType::STRING:   return "string";
        case DataType::BOOL:     return "bool";
        case DataType::LIST:     return "list";
        case DataType::FUNCTION: return "function";
        case DataType::ANY:      return "any";
        case DataType::VOID:     return "void";
        default:                 return "unknown";
    }
}

ASTNodePtr makeInt(int v, int line) {
    auto n = std::make_shared<ASTNode>(NodeType::INT_LIT, line);
    n->ival = v;
    n->dataType = DataType::INT;
    return n;
}

ASTNodePtr makeFloat(double v, int line) {
    auto n = std::make_shared<ASTNode>(NodeType::FLOAT_LIT, line);
    n->fval = v;
    n->dataType = DataType::FLOAT;
    return n;
}

ASTNodePtr makeString(const std::string& v, int line) {
    auto n = std::make_shared<ASTNode>(NodeType::STRING_LIT, line);
    n->sval = v;
    n->dataType = DataType::STRING;
    return n;
}

ASTNodePtr makeBool(bool v, int line) {
    auto n = std::make_shared<ASTNode>(NodeType::BOOL_LIT, line);
    n->bval = v;
    n->dataType = DataType::BOOL;
    return n;
}

ASTNodePtr makeNil(int line) {
    auto n = std::make_shared<ASTNode>(NodeType::NIL, line);
    n->dataType = DataType::LIST;
    return n;
}

ASTNodePtr makeIdent(const std::string& name, int line) {
    auto n = std::make_shared<ASTNode>(NodeType::IDENT, line);
    n->sval = name;
    return n;
}
