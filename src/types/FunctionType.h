#pragma once

#include <algorithm>
#include <vector>

#include "Format.h"
#include "TypeBuilder.h"

namespace _8b {

class FunctionType : public _Type {
public:

    FunctionType( const std::vector<Type> &argumentTypes, Type resultType,
        TypeBuilder &typeBuilder )
            : _argumentTypes( argumentTypes ), _resultType( resultType )
    {
        llvm::IRBuilder<> &irBuilder = typeBuilder.getIrBuilder();

        std::vector<llvm::Type*> rawArgumentTypes( argumentTypes.size() );
        std::transform(
            argumentTypes.begin(),
            argumentTypes.end(),
            rawArgumentTypes.begin(),
            []( Type type ) { return type->getRaw(); });

        llvm::Type *rawResultType =
            resultType ? resultType->getRaw() : irBuilder.getVoidTy();

        const bool isVariableArgument = false;
        _rawType = llvm::FunctionType::get(
            rawResultType, rawArgumentTypes, isVariableArgument );
    }

    const std::vector<Type>& getArgumentTypes() const { return _argumentTypes; }

    virtual Value generateGeneralOperatorCall(
        TypeBuilder &typeBuilder, Operator operator_,
        Value instance, const std::vector<Value> &arguments ) const
    {
        if( operator_ == Operator::Call ) {

            if( arguments.size() != _argumentTypes.size() )
                return nullptr;

            for( size_t index = 0; index < arguments.size(); ++index )
                if( arguments[index]->getType() != _argumentTypes[index] )
                    return nullptr;

            llvm::IRBuilder<> &irBuilder = typeBuilder.getIrBuilder();

            std::vector<llvm::Value*> rawArgumentValues( arguments.size() );
            std::transform(
                arguments.begin(),
                arguments.end(),
                rawArgumentValues.begin(),
                []( Value value ) { return value->getRaw(); });

            llvm::Value *result = irBuilder.CreateCall(
                instance->getRaw(), rawArgumentValues );

            if( _resultType )
                return _resultType->createValue( result );
            else
                return typeBuilder.getVoidType()->createValue( nullptr );
        }

        return nullptr;
    }

    virtual std::string getName() const {
        std::string argumentList;

        for( Type argumentType : _argumentTypes ) {
            if( !argumentList.empty() )
                argumentList =
                    format( "%1%, %2%", argumentList, argumentType->getName() );
            else
                argumentList = argumentType->getName();
        }

        if( _resultType )
            return format( "function( %1% ) %2%", argumentList,
                _resultType->getName() );
        else
            return format( "function( %1% )", argumentList );
    }

private:
    std::vector<Type> _argumentTypes;
    Type _resultType;
};

}
