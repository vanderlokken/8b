#pragma once

#include "Format.h"
#include "TypeBuilder.h"

namespace _8b {

class PointerType : public _Type {
public:

    PointerType( Type targetType, TypeBuilder &typeBuilder )
        : _targetType( targetType )
    {
        _rawType = llvm::PointerType::getUnqual( targetType->getRaw() );
    }

    virtual Value generateUnaryOperatorCall(
        TypeBuilder &typeBuilder, Operator operator_, Value instance ) const
    {
        if( operator_ == Operator::ToBoolean ) {
            llvm::IRBuilder<> &irBuilder = typeBuilder.getIrBuilder();
            llvm::Value *result = irBuilder.CreateIsNotNull(
                irBuilder.CreatePtrToInt(
                    instance->getRaw(), irBuilder.getInt32Ty()) );
            return typeBuilder.getBooleanType()->createValue( result );
        }
        return nullptr;
    }

    virtual Value generateBinaryOperatorCall(
        TypeBuilder &typeBuilder,
        Operator operator_, Value instance, Value argument ) const
    {
        if( operator_ == Operator::Assign )
            return generateDefaultAssignmentOperatorCall(
                typeBuilder, instance, argument );
        return nullptr;
    }

    virtual Value generateMemberAccess(
        TypeBuilder &typeBuilder,
        Value instance, const std::string &member ) const
    {
        if( member == "target" )
            return _Value::createIndirect(
                instance->getRaw(), _targetType, typeBuilder.getIrBuilder() );
        return nullptr;
    }

    virtual std::string getName() const {
        return format( "pointer[%1%]", _targetType->getName() );
    }

private:
    Type _targetType;
};

}
