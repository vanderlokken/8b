#pragma once

#include "Format.h"
#include "TypeBuilder.h"

namespace _8b {

class RealType : public _Type {
public:

    RealType( int bitWidth, TypeBuilder &typeBuilder )
        : _bitWidth( bitWidth )
    {
        llvm::IRBuilder<> &irBuilder = typeBuilder.getIrBuilder();

        switch( bitWidth ) {
        case 32:
            _rawType = irBuilder.getFloatTy();
            break;
        case 64:
            throw NotImplementedError();
        default:
            throw SemanticError(
                format("Invalid real bit width: %1%", _bitWidth) );
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
                llvm::Value *one = llvm::ConstantFP::get( _rawType, 1 );

                llvm::Value *modified = operator_ == Operator::Increment ?
                    irBuilder.CreateFAdd( instance->getRaw(), one ) :
                    irBuilder.CreateFSub( instance->getRaw(), one );

                return typeBuilder.getVoidType()->createValue(
                    irBuilder.CreateStore(modified, instance->getRawPointer()) );
            }
        case Operator::ToBoolean:
            return typeBuilder.getBooleanType()->createValue(
                irBuilder.CreateFCmpONE(
                    instance->getRaw(), llvm::ConstantFP::get(_rawType, 0)) );
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
            !argument->isInstanceOf(typeBuilder.getIntegerType(32)) &&
            !argument->isInstanceOf(typeBuilder.getRealType(32)) )
                return nullptr;

        switch( operator_ ) {
        case Operator::Add:
            return generateArithmeticalOperation(
                typeBuilder, llvm::Instruction::FAdd, instance, argument );
        case Operator::Subtract:
            return generateArithmeticalOperation(
                typeBuilder, llvm::Instruction::FSub, instance, argument );
        case Operator::Multiply:
            return generateArithmeticalOperation(
                typeBuilder, llvm::Instruction::FMul, instance, argument );
        case Operator::Divide:
            return generateArithmeticalOperation(
                typeBuilder, llvm::Instruction::FDiv, instance, argument );
        case Operator::AddRight:
            return generateArithmeticalOperation(
                typeBuilder, llvm::Instruction::FAdd, argument, instance );
        case Operator::SubtractRight:
            return generateArithmeticalOperation(
                typeBuilder, llvm::Instruction::FSub, argument, instance );
        case Operator::MultiplyRight:
            return generateArithmeticalOperation(
                typeBuilder, llvm::Instruction::FMul, argument, instance );
        case Operator::DivideRight:
            return generateArithmeticalOperation(
                typeBuilder, llvm::Instruction::FDiv, argument, instance );
        case Operator::Less:
            return generateComparisonOperation(
                typeBuilder, llvm::CmpInst::FCMP_OLT, instance, argument );
        case Operator::LessOrEqual:
            return generateComparisonOperation(
                typeBuilder, llvm::CmpInst::FCMP_OLE, instance, argument );
        case Operator::Greater:
            return generateComparisonOperation(
                typeBuilder, llvm::CmpInst::FCMP_OGT, instance, argument );
        case Operator::GreaterOrEqual:
            return generateComparisonOperation(
                typeBuilder, llvm::CmpInst::FCMP_OGE, instance, argument );
        case Operator::Equal:
            return generateComparisonOperation(
                typeBuilder, llvm::CmpInst::FCMP_OEQ, instance, argument );
        case Operator::NotEqual:
            return generateComparisonOperation(
                typeBuilder, llvm::CmpInst::FCMP_ONE, instance, argument );
        case Operator::Assign:
            return generateDefaultAssignmentOperatorCall(
                typeBuilder, instance, argument );
        }

        return nullptr;
    }

    virtual std::string getName() const {
        return format( "real[%1%]", _bitWidth );
    }

private:
    static llvm::Value* toReal( TypeBuilder &typeBuilder, Value value ) {
        llvm::IRBuilder<> &irBuilder = typeBuilder.getIrBuilder();

        Type realType = typeBuilder.getRealType();

        if( value->isInstanceOf(realType) )
            return value->getRaw();

        if( value->isInstanceOf(typeBuilder.getBooleanType()) )
            return irBuilder.CreateUIToFP(
                value->getRaw(), realType->getRaw() );

        return irBuilder.CreateSIToFP( value->getRaw(), realType->getRaw() );
    }

    static Value generateArithmeticalOperation(
        TypeBuilder &typeBuilder,
        llvm::Instruction::BinaryOps operation, Value left, Value right )
    {
        llvm::Value *result = typeBuilder.getIrBuilder().CreateBinOp(
            operation,
            toReal(typeBuilder, left),
            toReal(typeBuilder, right) );
        return typeBuilder.getRealType()->createValue( result );
    }

    static Value generateComparisonOperation(
        TypeBuilder &typeBuilder,
        llvm::CmpInst::Predicate predicate, Value left, Value right )
    {
        llvm::Value *result = typeBuilder.getIrBuilder().CreateFCmp(
            predicate,
            toReal(typeBuilder, left),
            toReal(typeBuilder, right) );
        return typeBuilder.getBooleanType()->createValue( result );
    }

    int _bitWidth;
};

}
