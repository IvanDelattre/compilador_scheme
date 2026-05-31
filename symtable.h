#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include "ast.h"

// Informação de um símbolo na tabela
struct SymbolInfo {
    std::string name;
    DataType    dataType;
    bool        isFunction = false;
    int         arity      = 0;       // número de parâmetros (se função)
    int         definedAt  = 0;       // linha onde foi definido
};

// Tabela de símbolos com escopos aninhados
class SymbolTable {
public:
    SymbolTable() {
        // Escopo global sempre existe
        pushScope();
        // Funções built-in do Scheme suportadas
        defineBuiltin("display",  DataType::VOID, 1);
        defineBuiltin("newline",  DataType::VOID, 0);
        defineBuiltin("car",      DataType::ANY,  1);
        defineBuiltin("cdr",      DataType::LIST, 1);
        defineBuiltin("cons",     DataType::LIST, 2);
        defineBuiltin("null?",    DataType::BOOL, 1);
        defineBuiltin("pair?",    DataType::BOOL, 1);
        defineBuiltin("list",     DataType::LIST, -1);
        defineBuiltin("not",      DataType::BOOL, 1);
        defineBuiltin("+",        DataType::ANY,  -1);
        defineBuiltin("-",        DataType::ANY,  -1);
        defineBuiltin("*",        DataType::ANY,  -1);
        defineBuiltin("/",        DataType::ANY,  -1);
        defineBuiltin("=",        DataType::BOOL, 2);
        defineBuiltin("<",        DataType::BOOL, 2);
        defineBuiltin(">",        DataType::BOOL, 2);
        defineBuiltin("<=",       DataType::BOOL, 2);
        defineBuiltin(">=",       DataType::BOOL, 2);
    }

    void defineBuiltin(const std::string& name, DataType dt, int arity) {
        SymbolInfo si;
        si.name = name; si.dataType = dt;
        si.isFunction = true; si.arity = arity; si.definedAt = 0;
        scopes.back()[name] = si;
    }

    // Abre novo escopo
    void pushScope() {
        scopes.push_back({});
    }

    // Fecha escopo atual
    void popScope() {
        if (scopes.size() <= 1)
            throw std::runtime_error("Tentativa de fechar o escopo global");
        scopes.pop_back();
    }

    // Define símbolo no escopo atual
    void define(const std::string& name, SymbolInfo info) {
        scopes.back()[name] = info;
    }

    // Busca símbolo nos escopos (do mais interno para o mais externo)
    SymbolInfo* lookup(const std::string& name) {
        for (int i = (int)scopes.size() - 1; i >= 0; --i) {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end())
                return &it->second;
        }
        return nullptr;
    }

    // Verifica se símbolo existe
    bool exists(const std::string& name) {
        return lookup(name) != nullptr;
    }

    bool existsInCurrentScope(const std::string& name) const {
        return scopes.back().find(name) != scopes.back().end();
    }

    int depth() const { return (int)scopes.size(); }

private:
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes;
};

#endif // SYMTABLE_H
