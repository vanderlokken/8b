#pragma once

#include <list>
#include <map>
#include <string>

#include "Value.h"

namespace _8b {

class SymbolTable {
public:

    void enterLexicalScope();
    void leaveLexicalScope();

    void addValue( const std::string&, ValuePointer );
    void addType( const std::string&, ValueTypePointer );

    ValuePointer lookupValue( const std::string& ) const;
    ValueTypePointer lookupType( const std::string& ) const;

private:
    struct LexicalScope {
        std::map<std::string, ValuePointer> _values;
        std::map<std::string, ValueTypePointer> _types;
    };
    std::list< LexicalScope > _scopes;
};

}
