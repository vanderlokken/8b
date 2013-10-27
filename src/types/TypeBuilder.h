#pragma once

#include <map>
#include <tuple>
#include <vector>

#include <llvm/IR/IRBuilder.h>

#include "Type.h"

namespace _8b {

class TypeBuilder {
public:
    TypeBuilder( llvm::IRBuilder<> &irBuilder ) : _irBuilder( irBuilder ) {}

    llvm::IRBuilder<>& getIrBuilder() { return _irBuilder; }

    Type getVoidType();
    Type getBooleanType();
    Type getIntegerType( size_t bitWidth = 32 );
    Type getRealType( size_t bitWidth = 32 );
    Type getPointerType( Type targetType );
    Type getFunctionType(
        const std::vector<Type> &argumentTypes, Type returnType = nullptr );

private:
    llvm::IRBuilder<> &_irBuilder;

    Type _voidType;
    Type _booleanType;
    std::map<size_t, Type> _integerTypes;
    std::map<size_t, Type> _realTypes;
    std::map<Type, Type> _pointerTypes;
    std::map<std::tuple<std::vector<Type>, Type>, Type> _functionTypes;
};

}
