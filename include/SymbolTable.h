#pragma once

#include <list>
#include <map>
#include <stdexcept>
#include <string>

#include <boost/noncopyable.hpp>

#include "Value.h"

namespace _8b {

class SymbolTable {
public:

    void enterLexicalScope();
    void leaveLexicalScope();

    void addValue( const std::string&, Value );
    void addType( const std::string&, ValueType );

    Value lookupValue( const std::string& ) const;
    ValueType lookupType( const std::string& ) const;

private:
    struct LexicalScope {
        std::map<std::string, Value> _values;
        std::map<std::string, ValueType> _types;
    };
    std::list< LexicalScope > _scopes;
};

struct SymbolLookupError : std::exception {
    SymbolLookupError() : std::exception( "Undeclared identifier" ) {}
};

class LexicalScope : boost::noncopyable {
public:
    LexicalScope( SymbolTable &symbolTable ) : _symbolTable( symbolTable ) {
        _symbolTable.enterLexicalScope();
    }

    ~LexicalScope() {
        _symbolTable.leaveLexicalScope();
    }

private:
    SymbolTable &_symbolTable;
};

}
