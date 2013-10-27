#pragma once

#include "Format.h"
#include "TypeBuilder.h"

namespace _8b {

class IntegerType : public _Type {
public:

    IntegerType( int bitWidth, TypeBuilder &typeBuilder )
        : _bitWidth( bitWidth )
    {
        llvm::IRBuilder<> &irBuilder = typeBuilder.getIrBuilder();

        switch( bitWidth ) {
        case 1:
            throw SemanticError(
                "'integer[1]' is incorrect; 'boolean' should be used instead" );
        case 8:
            _rawType = irBuilder.getInt8Ty();
            break;
        case 16:
            _rawType = irBuilder.getInt16Ty();
            break;
        case 32:
            _rawType = irBuilder.getInt32Ty();
            break;
        case 64:
            throw NotImplementedError();
        default:
            throw SemanticError(
                format("Invalid integer bit width: %1%", _bitWidth) );
        }
    }

    virtual Value generateUnaryOperatorCall(
        TypeBuilder &typeBuilder, Operator operator_, Value instance ) const
    {
        llvm::IRBuilder<> &irBuilder = typeBuilder.getIrBuilder();

        switch( operator_ ) {
        case Operator::Increment:
        case Operator::Decrement:
            {
                llvm::Value *one = llvm::ConstantInt::get( _rawType, 1 );

                llvm::Value *modified = operator_ == Operator::Increment ?
                    irBuilder.CreateAdd( instance->getRaw(), one ) :
                    irBuilder.CreateSub( instance->getRaw(), one );

                return typeBuilder.getVoidType()->createValue(
                    irBuilder.CreateStore(modified, instance->getRawPointer()) );
            }
        case Operator::ToBoolean:
            return typeBuilder.getBooleanType()->createValue(
                irBuilder.CreateIsNotNull(instance->getRaw()) );
        }

        return nullptr;
    }

    virtual Value generateBinaryOperatorCall(
        TypeBuilder &typeBuilder,
        Operator operator_, Value instance, Value argument ) const
    {
        if( !argument->isInstanceOf(typeBuilder.getBooleanType()) &&
            !argument->isInstanceOf(typeBuilder.getIntegerType(8)) &&
            !argument->isInstanceOf(typeBuilder.getIntegerType(16)) &&
            !argument->isInstanceOf(typeBuilder.getIntegerType(32)) )
                return nullptr;

        switch( operator_ ) {
        case Operator::Add:
            return generateArithmeticalOperation(
                typeBuilder, llvm::Instruction::Add, instance, argument );
        case Operator::Subtract:
            return generateArithmeticalOperation(
                typeBuilder, llvm::Instruction::Sub, instance, argument );
        case Operator::Multiply:
            return generateArithmeticalOperation(
                typeBuilder, llvm::Instruction::Mul, instance, argument );
        case Operator::Divide:
            return generateArithmeticalOperation(
                typeBuilder, llvm::Instruction::SDiv, instance, argument );
        case Operator::Less:
            return generateComparisonOperation(
                typeBuilder, llvm::CmpInst::ICMP_SLT, instance, argument );
        case Operator::LessOrEqual:
            return generateComparisonOperation(
                typeBuilder, llvm::CmpInst::ICMP_SLE, instance, argument );
        case Operator::Greater:
            return generateComparisonOperation(
                typeBuilder, llvm::CmpInst::ICMP_SGT, instance, argument );
        case Operator::GreaterOrEqual:
            return generateComparisonOperation(
                typeBuilder, llvm::CmpInst::ICMP_SGE, instance, argument );
        case Operator::Equal:
            return generateComparisonOperation(
                typeBuilder, llvm::CmpInst::ICMP_EQ, instance, argument );
        case Operator::NotEqual:
            return generateComparisonOperation(
                typeBuilder, llvm::CmpInst::ICMP_NE, instance, argument );
        case Operator::Assign:
            return generateDefaultAssignmentOperatorCall(
                typeBuilder, instance, argument );
        }

        return nullptr;
    }

    virtual std::string getName() const {
        return format( "integer[%1%]", _bitWidth );
    }

private:
    static llvm::Value* toInteger( TypeBuilder &typeBuilder, Value value ) {
        llvm::IRBuilder<> &irBuilder = typeBuilder.getIrBuilder();

        Type integerType = typeBuilder.getIntegerType();

        if( value->isInstanceOf(integerType) )
            return value->getRaw();

        if( value->isInstanceOf(typeBuilder.getBooleanType()) )
            return irBuilder.CreateZExt(
                value->getRaw(), integerType->getRaw() );

        return irBuilder.CreateSExt( value->getRaw(), integerType->getRaw() );
    }

    static Value generateArithmeticalOperation(
        TypeBuilder &typeBuilder,
        llvm::Instruction::BinaryOps operation, Value left, Value right )
    {
        llvm::Value *result = typeBuilder.getIrBuilder().CreateBinOp(
            operation,
            toInteger(typeBuilder, left),
            toInteger(typeBuilder, right) );
        return typeBuilder.getIntegerType()->createValue( result );
    }

    static Value generateComparisonOperation(
        TypeBuilder &typeBuilder,
        llvm::CmpInst::Predicate predicate, Value left, Value right )
    {
        llvm::Value *result = typeBuilder.getIrBuilder().CreateICmp(
            predicate,
            toInteger(typeBuilder, left),
            toInteger(typeBuilder, right) );
        return typeBuilder.getBooleanType()->createValue( result );
    }

    int _bitWidth;
};

}
