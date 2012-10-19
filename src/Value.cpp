#include "Value.h"

#include <llvm/Constants.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Type.h>

#include "Exception.h"

namespace _8b {

extern llvm::LLVMContext &globalLLVMContext;
extern llvm::IRBuilder<> irBuilder;

// Value

llvm::Value* Value::getLlvmValue() const {

    if( llvm::AllocaInst::classof(_llvmValue) )
        return irBuilder.CreateLoad( _llvmValue );
    else
        return _llvmValue;
}

ValuePointer Value::generateAdd( ValuePointer ) const {
    throwRuntimeError( "Unsupported operation" );
    return 0;
}

ValuePointer Value::generateSubtract( ValuePointer ) const {
    throwRuntimeError( "Unsupported operation" );
    return 0;
}

ValuePointer Value::generateMultiply( ValuePointer ) const {
    throwRuntimeError( "Unsupported operation" );
    return 0;
}

ValuePointer Value::generateDivide( ValuePointer ) const {
    throwRuntimeError( "Unsupported operation" );
    return 0;
}

BooleanValuePointer Value::generateAnd( ValuePointer operand ) const {
    llvm::Value *left =
        instanceOf<BooleanValue>() ? getLlvmValue() : generateToBoolean()->getLlvmValue();
    llvm::Value *right =
        operand->instanceOf<BooleanValue>() ? operand->getLlvmValue() : operand->generateToBoolean()->getLlvmValue();
    return BooleanValue::create( irBuilder.CreateAnd(left, right) );
}

BooleanValuePointer Value::generateOr( ValuePointer operand ) const {
    llvm::Value *left =
        instanceOf<BooleanValue>() ? getLlvmValue() : generateToBoolean()->getLlvmValue();
    llvm::Value *right =
        operand->instanceOf<BooleanValue>() ? operand->getLlvmValue() : operand->generateToBoolean()->getLlvmValue();
    return BooleanValue::create( irBuilder.CreateOr(left, right) );
}

BooleanValuePointer Value::generateLess( ValuePointer ) const {
    throwRuntimeError( "Unsupported operation" );
    return 0;
}

BooleanValuePointer Value::generateGreater( ValuePointer ) const {
    throwRuntimeError( "Unsupported operation" );
    return 0;
}

void Value::generateIncrement() const {
    throwRuntimeError( "Unsupported operation" );
}

void Value::generateDecrement() const {
    throwRuntimeError( "Unsupported operation" );
}

BooleanValuePointer Value::generateToBoolean() const {
    throwRuntimeError( "The value cannot be represented as a boolean value" );
    return 0;
}

IntegerValuePointer Value::generateToInteger() const {
    throwRuntimeError( "The value cannot be represented as an integer value" );
    return 0;
}

ValuePointer Value::generateCall( const std::vector<ValuePointer>& ) const {
    throwRuntimeError( "The value cannot be called" );
    return 0;
}


// IntegerValue

IntegerValuePointer IntegerValue::create( const std::string &name, int bitWidth ) {
    return std::make_shared<IntegerValue>( name, bitWidth );
}

IntegerValuePointer IntegerValue::create( int value ) {
    return std::make_shared<IntegerValue>( value );
}

IntegerValuePointer IntegerValue::create( llvm::Value *value ) {
    return std::make_shared<IntegerValue>( value );
}

IntegerValue::IntegerValue( const std::string &name, int bitWidth ) {
    llvm::Type *type = llvm::IntegerType::get( globalLLVMContext, bitWidth );
    _llvmValue = irBuilder.CreateAlloca( type );
    _llvmValue->setName( name );
}

IntegerValue::IntegerValue( int value ) {
    _llvmValue = llvm::ConstantInt::get(
        globalLLVMContext, llvm::APInt(32, value, false) );
}

ValuePointer IntegerValue::generateAdd( ValuePointer operand ) const {
    return IntegerValue::create( irBuilder.CreateAdd(getLlvmValue(), integerOperand(operand)) );
}

ValuePointer IntegerValue::generateSubtract( ValuePointer operand ) const {
    return IntegerValue::create( irBuilder.CreateSub(getLlvmValue(), integerOperand(operand)) );
}

ValuePointer IntegerValue::generateMultiply( ValuePointer operand ) const {
    return IntegerValue::create( irBuilder.CreateMul(getLlvmValue(), integerOperand(operand)) );
}

ValuePointer IntegerValue::generateDivide( ValuePointer operand ) const {
    return IntegerValue::create( irBuilder.CreateSDiv(getLlvmValue(), integerOperand(operand)) );
}

BooleanValuePointer IntegerValue::generateLess( ValuePointer operand ) const {
    return BooleanValue::create( irBuilder.CreateICmpSLT(getLlvmValue(), integerOperand(operand)) );
}

BooleanValuePointer IntegerValue::generateGreater( ValuePointer operand ) const {
    return BooleanValue::create( irBuilder.CreateICmpSGT(getLlvmValue(), integerOperand(operand)) );
}

void IntegerValue::generateAssignment( ValuePointer operand ) const {
    if( !llvm::AllocaInst::classof(_llvmValue) )
        throwRuntimeError( "An assignment to a temporary value is not possible" );
    irBuilder.CreateStore( integerOperand(operand), _llvmValue );
}

void IntegerValue::generateIncrement() const {
    generateAssignment( generateAdd(IntegerValue::create(1)) );
}

void IntegerValue::generateDecrement() const {
    generateAssignment( generateSubtract(IntegerValue::create(1)) );
}

BooleanValuePointer IntegerValue::generateToBoolean() const {
    return BooleanValue::create( irBuilder.CreateIsNotNull(getLlvmValue()) );
}

llvm::Value* IntegerValue::integerOperand( ValuePointer operand ) {
    if( operand->instanceOf<BooleanValue>() )
        operand = operand->generateToInteger();
    if( !operand->instanceOf<IntegerValue>() )
        throwRuntimeError( "Not implemented or not supported" );
    return operand->getLlvmValue();
}


// BooleanValue

BooleanValuePointer BooleanValue::create( const std::string &name ) {
    return std::make_shared<BooleanValue>( name );
}

BooleanValuePointer BooleanValue::create( bool value ) {
    return std::make_shared<BooleanValue>( value );
}

BooleanValuePointer BooleanValue::create( llvm::Value *value ) {
    return std::make_shared<BooleanValue>( value );
}

BooleanValue::BooleanValue( const std::string &name ) {
    llvm::Type *type = llvm::Type::getInt1Ty( globalLLVMContext );
    _llvmValue = irBuilder.CreateAlloca( type );
    _llvmValue->setName( name );
}

BooleanValue::BooleanValue( bool value ) {
    llvm::Type *type = llvm::Type::getInt1Ty( globalLLVMContext );
    if( value )
        _llvmValue = llvm::ConstantInt::getTrue( type );
    else
        _llvmValue = llvm::ConstantInt::getFalse( type );
}

void BooleanValue::generateAssignment( ValuePointer operand ) const {
    if( !llvm::AllocaInst::classof(_llvmValue) )
        throwRuntimeError( "An assignment to a temporary value is not possible" );
    irBuilder.CreateStore( operand->generateToBoolean()->getLlvmValue(), _llvmValue );
}

IntegerValuePointer BooleanValue::generateToInteger() const {
    llvm::Value *converted = irBuilder.CreateZExt(
        getLlvmValue(), llvm::Type::getInt32Ty(globalLLVMContext) );
    return IntegerValue::create( converted );
}


// FunctionValue

FunctionValuePointer FunctionValue::create( llvm::Value *value ) {
    return std::make_shared<FunctionValue>( value );
}

void FunctionValue::generateAssignment( ValuePointer operand ) const {
    throwRuntimeError( "Not implemented or not supported" );
}

ValuePointer FunctionValue::generateCall( const std::vector<ValuePointer> &arguments ) const {

    std::vector<llvm::Value*> llvmArguments( arguments.size() );

    for( size_t i = 0; i < llvmArguments.size(); ++i ) {
        llvmArguments[i] = arguments[i]->getLlvmValue();
    }

    return IntegerValue::create( irBuilder.CreateCall(getLlvmValue(), llvmArguments) );
}

}
