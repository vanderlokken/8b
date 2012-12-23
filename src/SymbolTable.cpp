#include "SymbolTable.h"

#include "Exception.h"

namespace _8b {

void SymbolTable::enterLexicalScope() {
    _scopes.push_front( SymbolTable::LexicalScope() );
}

void SymbolTable::leaveLexicalScope() {
    _scopes.pop_front();
}

void SymbolTable::addValue( const std::string &name, Value value ) {
    _scopes.begin()->_values[name] = value;
}

void SymbolTable::addType( const std::string &name, ValueType type ) {
    _scopes.begin()->_types[name] = type;
}

Value SymbolTable::lookupValue( const std::string &name ) const {
    for( const auto &scope : _scopes ) {
        auto it = scope._values.find( name );
        if( it != scope._values.end() )
            return it->second;
    }
    throwRuntimeError( "Undeclared identifier" );
}

ValueType SymbolTable::lookupType( const std::string &name ) const {
    for( const auto &scope : _scopes ) {
        auto it = scope._types.find( name );
        if( it != scope._types.end() )
            return it->second;
    }
    throwRuntimeError( "Undeclared type" );
}

}
