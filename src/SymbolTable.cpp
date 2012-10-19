#include "SymbolTable.h"

#include "Exception.h"

namespace _8b {
    
void SymbolTable::pushLexicalScope() {
    _scopes.push_front( std::map< std::string, ValuePointer >() );
}

void SymbolTable::popLexicalScope() {
    _scopes.pop_front();
}

void SymbolTable::addSymbol( const std::string &name, ValuePointer value ) {
    (*_scopes.begin())[name] = value;
}

ValuePointer SymbolTable::lookupSymbol( const std::string &name ) const {
    for( const auto &scope : _scopes ) {
        auto it = scope.find( name );
        if( it != scope.end() )
            return it->second;
    }
    throwRuntimeError( "Undeclared identifier" );
}

}
