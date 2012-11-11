#include "SymbolTable.h"

#include "Exception.h"

namespace _8b {
    
void SymbolTable::enterLexicalScope() {
    _scopes.push_front( SymbolTable::LexicalScope() );
}

void SymbolTable::leaveLexicalScope() {
    _scopes.pop_front();
}

void SymbolTable::addValue( const std::string &name, ValuePointer value ) {
    (*_scopes.begin())._valueNames[name] = value;
}

void SymbolTable::addType( const std::string &name, ValueTypePointer type ) {
    (*_scopes.begin())._typeNames[name] = type;
}

ValuePointer SymbolTable::lookupValue( const std::string &name ) const {
    for( const auto &scope : _scopes ) {
        auto it = scope._valueNames.find( name );
        if( it != scope._valueNames.end() )
            return it->second;
    }
    throwRuntimeError( "Undeclared identifier" );
}

ValueTypePointer SymbolTable::lookupType( const std::string &name ) const {
    for( const auto &scope : _scopes ) {
        auto it = scope._typeNames.find( name );
        if( it != scope._typeNames.end() )
            return it->second;
    }
    throwRuntimeError( "Undeclared type" );
}

}
