#pragma once

#include <functional>
#include <memory>
#include <string>

#include <llvm/IR/IRBuilder.h>

#include "Exception.h"

namespace _8b {

class _Value;
class _Type;

typedef std::shared_ptr<_Value> Value;
typedef std::shared_ptr<const _Type> Type;

class _Value {
public:

    typedef std::function<llvm::Value* (llvm::Value*)> Dereference;

    static Value create(
        llvm::Value *rawValue, Type type, const std::string &identifier = "" )
    {
        if( !identifier.empty() )
            rawValue->setName( identifier );
        return std::make_shared<_Value>( rawValue, type );
    }

    static Value createIndirect(
        llvm::Value *rawValue, Type type,
        llvm::IRBuilder<> &irBuilder, const std::string &identifier = "" )
    {
        if( !identifier.empty() )
            rawValue->setName( identifier );
        auto dereference = [&irBuilder] (llvm::Value *value) {
            return irBuilder.CreateLoad( value );
        };
        return std::make_shared<_Value>( rawValue, type, dereference );
    }

    _Value( llvm::Value *rawValue, Type type, Dereference dereference = nullptr )
        : _rawValue( rawValue ), _type( type ), _dereference( dereference ) {}

    Type getType() const {
        return _type;
    }

    llvm::Value* getRaw() const {
        return _dereference ? _dereference( _rawValue ) : _rawValue;
    }

    llvm::Value* getRawPointer() const {
        if( !_dereference )
            throw SemanticError( "Cannot reference a temporary value" );
        return _rawValue;
    }

    bool isIndirect() const {
        return _dereference != nullptr;
    }

    bool isInstanceOf( Type type ) const {
        return _type == type;
    }

protected:
    llvm::Value *_rawValue;
    Type _type;
    Dereference _dereference;
};

}
