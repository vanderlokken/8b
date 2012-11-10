#include "Value.h"

#include <llvm/Argument.h>
#include <llvm/Constants.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Type.h>

#include "Exception.h"

namespace _8b {

extern llvm::LLVMContext &globalLLVMContext;
extern llvm::IRBuilder<> irBuilder;

// Value

ValuePointer Value::createVariable( ValueTypePointer type, const std::string &identifier ) {
    llvm::Value *llvmValue = irBuilder.CreateAlloca( type->toLlvm() );
    llvmValue->setName( identifier );
    return std::make_shared<Value>( type, llvmValue );
}

ValuePointer Value::createSsaValue( ValueTypePointer type, llvm::Value *value ) {
    return std::make_shared<Value>( type, value );
}

ValuePointer Value::createUnusableValue() {
    class UnusableType : public _ValueType<UnusableType> {};
    static const ValueTypePointer type( new UnusableType() );
    static const ValuePointer value( Value::createSsaValue(type, nullptr) );
    return value;
}

ValuePointer Value::createIntegerConstant( int value ) {
    return Value::createSsaValue(
        IntegerType::get(),
        llvm::ConstantInt::get(IntegerType::get()->toLlvm(), value) );
}

ValuePointer Value::createBooleanConstant( bool value ) {
    return Value::createSsaValue(
        BooleanType::get(),
        llvm::ConstantInt::get(BooleanType::get()->toLlvm(), value) );
}

ValueTypePointer Value::getType() const {
    return _type;
}

llvm::Value* Value::getLlvmValue() const {

    if( llvm::AllocaInst::classof(_llvmValue) )
        return irBuilder.CreateLoad( _llvmValue );
    else
        return _llvmValue;
}

llvm::Value* Value::getRawLlvmValue() const {
    return _llvmValue;
}

ValuePointer Value::generateBinaryOperation( BinaryOperation operation, ValuePointer rightOperand ) const {
    return _type->generateBinaryOperation( operation, shared_from_this(), rightOperand );
}

ValuePointer Value::generateUnaryOperation( UnaryOperation operation ) const {
    return _type->generateUnaryOperation( operation, shared_from_this() );
}

ValuePointer Value::generateCall( const std::vector<ValuePointer> &arguments ) const {
    return _type->generateCall( shared_from_this(), arguments );
}

ValuePointer Value::generateMemberAccess( const std::string &memberIdentifier ) const {
    return _type->generateMemberAccess( shared_from_this(), memberIdentifier );
}

ValuePointer Value::toBoolean() const {
    return _type->generateUnaryOperation( UnaryOperation::BooleanConversion, shared_from_this() );
}

ValuePointer Value::toInteger() const {
    return _type->generateUnaryOperation( UnaryOperation::IntegerConversion, shared_from_this() );
}


// ValueType

llvm::Type* ValueType::toLlvm() const {
    return _type;
}

ValuePointer ValueType::generateBinaryOperation( BinaryOperation operation, ValuePointer first, ValuePointer second ) const {

    if( operation == BinaryOperation::LogicOr ||
        operation == BinaryOperation::LogicAnd )
        return first->toBoolean()->generateBinaryOperation( operation, second->toBoolean() );

    throwRuntimeError( "This type doesn't implement specified operation" );
    return 0;
}

ValuePointer ValueType::generateUnaryOperation( UnaryOperation operation, ValuePointer operand ) const {
    throwRuntimeError( "This type doesn't implement specified operation" );
    return 0;
}

ValuePointer ValueType::generateCall( ValuePointer, const std::vector<ValuePointer>& ) const {
    throwRuntimeError( "This type doesn't implement invokation" );
    return 0;
}

ValuePointer ValueType::generateMemberAccess( ValuePointer, const std::string& ) const {
    throwRuntimeError( "Not implemented or this type doesn't have specified property" );
    return 0;
}


// IntegerType

ValueTypePointer IntegerType::get( int bitWidth ) {

    if( bitWidth != 32 )
        throwRuntimeError( "Not supported" );

    static const ValueTypePointer instance( new IntegerType(bitWidth) );
    return instance;
}

IntegerType::IntegerType( int bitWidth ) {
    _type = llvm::IntegerType::get( globalLLVMContext, bitWidth );
}

ValuePointer IntegerType::generateBinaryOperation( BinaryOperation operation, ValuePointer first, ValuePointer second ) const {

    if( operation == BinaryOperation::Assignment ) {

        if( !llvm::AllocaInst::classof(first->getRawLlvmValue()) )
            throwRuntimeError( "An assignment to a temporary value is not possible" );

        irBuilder.CreateStore( integerOperand(second), first->getRawLlvmValue() );

        return Value::createUnusableValue();
    }

    if( operation == BinaryOperation::Addition )
        return Value::createSsaValue( IntegerType::get(),
            irBuilder.CreateAdd(first->getLlvmValue(), integerOperand(second)) );

    if( operation == BinaryOperation::Subtraction )
        return Value::createSsaValue( IntegerType::get(),
            irBuilder.CreateSub(first->getLlvmValue(), integerOperand(second)) );

    if( operation == BinaryOperation::Multiplication )
        return Value::createSsaValue( IntegerType::get(),
            irBuilder.CreateMul(first->getLlvmValue(), integerOperand(second)) );

    if( operation == BinaryOperation::Division )
        return Value::createSsaValue( IntegerType::get(),
            irBuilder.CreateSDiv(first->getLlvmValue(), integerOperand(second)) );

    if( operation == BinaryOperation::LessComparison )
        return Value::createSsaValue( BooleanType::get(),
            irBuilder.CreateICmpSLT(first->getLlvmValue(), integerOperand(second)) );

    if( operation == BinaryOperation::GreaterComparison )
        return Value::createSsaValue( BooleanType::get(),
            irBuilder.CreateICmpSGT(first->getLlvmValue(), integerOperand(second)) );
    
    return ValueType::generateBinaryOperation( operation, first, second );
}

ValuePointer IntegerType::generateUnaryOperation( UnaryOperation operation, ValuePointer operand ) const {

    if( operation == UnaryOperation::BooleanConversion )
        return Value::createSsaValue( BooleanType::get(),
            irBuilder.CreateIsNotNull(integerOperand(operand)) );

    if( operation == UnaryOperation::IntegerConversion )
        return operand;

    if( operation == UnaryOperation::Increment ) {

        ValuePointer incremented = generateBinaryOperation(
            BinaryOperation::Addition, operand, Value::createIntegerConstant(1) );

        return generateBinaryOperation(
            BinaryOperation::Assignment, operand, incremented );
    }

    if( operation == UnaryOperation::Decrement ) {

        ValuePointer decremented = generateBinaryOperation(
            BinaryOperation::Subtraction, operand, Value::createIntegerConstant(1) );

        return generateBinaryOperation(
            BinaryOperation::Assignment, operand, decremented );
    }

    return ValueType::generateUnaryOperation( operation, operand );
}

llvm::Value* IntegerType::integerOperand( ValuePointer operand ) {
    
    if( operand->getType()->instanceOf<BooleanType>() )
        operand = operand->toInteger();
    
    if( !operand->getType()->instanceOf<IntegerType>() )
        throwRuntimeError( "Not implemented or not supported" );
    
    return operand->getLlvmValue();
}


// BooleanType

ValueTypePointer BooleanType::get() {
    static const ValueTypePointer instance( new BooleanType() );
    return instance;
}

BooleanType::BooleanType() {
    _type = llvm::Type::getInt1Ty( globalLLVMContext );
}

ValuePointer BooleanType::generateBinaryOperation( BinaryOperation operation, ValuePointer first, ValuePointer second ) const {

    if( operation == BinaryOperation::Assignment ) {

        if( !llvm::AllocaInst::classof(first->getRawLlvmValue()) )
            throwRuntimeError( "An assignment to a temporary value is not possible" );

        irBuilder.CreateStore( second->toBoolean()->getLlvmValue(), first->getRawLlvmValue() );

        return Value::createUnusableValue();
    }

    if( operation == BinaryOperation::LogicOr )
        return Value::createSsaValue( BooleanType::get(),
            irBuilder.CreateOr(first->getLlvmValue(), second->toBoolean()->getLlvmValue()) );

    if( operation == BinaryOperation::LogicAnd )
        return Value::createSsaValue( BooleanType::get(),
            irBuilder.CreateAnd(first->getLlvmValue(), second->toBoolean()->getLlvmValue()) );

    if( operation == BinaryOperation::Addition ||
        operation == BinaryOperation::Subtraction ||
        operation == BinaryOperation::Multiplication ||
        operation == BinaryOperation::Division ||
        operation == BinaryOperation::LessComparison ||
        operation == BinaryOperation::GreaterComparison )
    {
        return first->toInteger()->generateBinaryOperation( operation, second );
    }

    return ValueType::generateBinaryOperation( operation, first, second );
}

ValuePointer BooleanType::generateUnaryOperation( UnaryOperation operation, ValuePointer operand ) const {

    if( operation == UnaryOperation::BooleanConversion )
        return operand;

    if( operation == UnaryOperation::IntegerConversion )
        return Value::createSsaValue( IntegerType::get(),
            irBuilder.CreateZExt(operand->getLlvmValue(), IntegerType::get()->toLlvm()) );

    return ValueType::generateUnaryOperation( operation, operand );
}


// FunctionType

FunctionType::FunctionType( const std::vector<ValueTypePointer> &argumentTypes, ValueTypePointer resultType )
    : _resultType( resultType )
{
    const bool isVariableArgument = false;
    
    std::vector<llvm::Type*> llvmArgumentTypes( argumentTypes.size() );

    std::transform(
        argumentTypes.cbegin(),
        argumentTypes.cend(),
        llvmArgumentTypes.begin(),
        []( ValueTypePointer type ) -> llvm::Type* {
            return type->toLlvm();
        });

    llvm::Type *llvmResultType = _resultType ? _resultType->toLlvm() : llvm::Type::getVoidTy( globalLLVMContext );

    _type = llvm::FunctionType::get( llvmResultType, llvmArgumentTypes, isVariableArgument );
}

ValuePointer FunctionType::generateCall( ValuePointer callee, const std::vector<ValuePointer> &arguments ) const {

    std::vector<llvm::Value*> llvmArguments( arguments.size() );

    std::transform(
        arguments.cbegin(),
        arguments.cend(),
        llvmArguments.begin(),
        []( ValuePointer value ) -> llvm::Value* {
            return value->getLlvmValue();
        });

    llvm::Value *llvmResultValue = irBuilder.CreateCall( callee->getLlvmValue(), llvmArguments );

    if( _resultType )
        return Value::createSsaValue( _resultType, llvmResultValue );
    else
        return Value::createUnusableValue();
}

}
