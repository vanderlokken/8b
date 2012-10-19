#pragma once

#include <list>
#include <map>
#include <string>

#include "Value.h"

namespace _8b {

class SymbolTable {
public:

    void pushLexicalScope();
    void popLexicalScope();

    void addSymbol( const std::string&, ValuePointer );
    ValuePointer lookupSymbol( const std::string& ) const;

private:
    std::list< std::map<std::string, ValuePointer> > _scopes;
};

}
