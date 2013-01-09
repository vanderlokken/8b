#include "Value.h"

#include <llvm/Argument.h>
#include <llvm/Constants.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Type.h>

#include "Exception.h"

namespace _8b {

extern llvm::IRBuilder<> irBuilder;

// Value

Value _Value::createVariable( ValueType type, const std::string &identifier ) {
    llvm::Value *llvmValue = irBuilder.CreateAlloca( type->toLlvm() );
    llvmValue->setName( identifier );
    return std::make_shared<_Value>( type, llvmValue, true );
}

Value _Value::createReference( ValueType type, llvm::Value *value ) {
    return std::make_shared<_Value>( type, value, true );
}

Value _Value::createSsaValue( ValueType type, llvm::Value *value ) {
    return std::make_shared<_Value>( type, value, false );
}

Value _Value::createUnusableValue() {
    class UnusableType : public _ValueType {};
    static const ValueType type( new UnusableType() );
    static const Value value( _Value::createSsaValue(type, nullptr) );
    return value;
}

Value _Value::createIntegerConstant( int value ) {
    return _Value::createSsaValue( IntegerType::get(), irBuilder.getInt32(value) );
}

Value _Value::createBooleanConstant( bool value ) {
    return _Value::createSsaValue( BooleanType::get(), irBuilder.getInt1(value) );
}

Value _Value::createStringConstant( const std::string &value ) {
    return _Value::createSsaValue( StringType::get(), irBuilder.CreateGlobalString(value) );
}

ValueType _Value::getType() const {
    return _type;
}

llvm::Value* _Value::toLlvm() const {

    if( _assignable )
        return irBuilder.CreateLoad( _llvmValue );
    else
        return _llvmValue;
}

llvm::Value* _Value::toLlvmPointer() const {

    if( _assignable )
        return _llvmValue;
    else
        throw SemanticError( "Cannot reference a temporary value" );
}

bool _Value::isAssignable() const {
    return _assignable;
}

Value _Value::generateBinaryOperation( BinaryOperation operation, Value rightOperand ) const {
    return _type->generateBinaryOperation( operation, shared_from_this(), rightOperand );
}

Value _Value::generateUnaryOperation( UnaryOperation operation ) const {
    return _type->generateUnaryOperation( operation, shared_from_this() );
}

Value _Value::generateCall( const std::vector<Value> &arguments ) const {
    return _type->generateCall( shared_from_this(), arguments );
}

Value _Value::generateMemberAccess( const std::string &memberIdentifier ) const {
    return _type->generateMemberAccess( shared_from_this(), memberIdentifier );
}

Value _Value::toBoolean() const {
    return _type->generateUnaryOperation( UnaryOperation::BooleanConversion, shared_from_this() );
}

Value _Value::toInteger() const {
    return _type->generateUnaryOperation( UnaryOperation::IntegerConversion, shared_from_this() );
}


// ValueType

llvm::Type* _ValueType::toLlvm() const {
    return _type;
}

Value _ValueType::generateBinaryOperation( BinaryOperation operation, Value first, Value second ) const {

    if( operation == BinaryOperation::LogicOr ||
        operation == BinaryOperation::LogicAnd )
        return first->toBoolean()->generateBinaryOperation( operation, second->toBoolean() );

    throw SemanticError( "This type doesn't implement specified operation" );
}

Value _ValueType::generateUnaryOperation( UnaryOperation operation, Value operand ) const {

    if( operation == UnaryOperation::Addressing )
        return _Value::createSsaValue(
            std::make_shared<PointerType>(operand->getType()), operand->toLlvmPointer() );

    throw SemanticError( "This type doesn't implement specified operation" );
}

Value _ValueType::generateCall( Value, const std::vector<Value>& ) const {
    throw SemanticError( "This type doesn't implement invokation" );
}

Value _ValueType::generateMemberAccess( Value, const std::string& ) const {
    throw SemanticError(
        "Not implemented or this type doesn't have specified property" );
}

bool _ValueType::isIntegerSubset() const {
    return false;
}

bool _ValueType::isRealSubset() const {
    return false;
}

// IntegerType

ValueType IntegerType::get( int bitWidth ) {

    if( bitWidth != 32 )
        throw NotImplementedError();

    static const ValueType instance( new IntegerType(bitWidth) );
    return instance;
}

IntegerType::IntegerType( int bitWidth ) {
    _type = irBuilder.getInt32Ty();
}

Value IntegerType::generateBinaryOperation(
    BinaryOperation operation, Value first, Value second ) const
{
    if( !second->getType()->isIntegerSubset() )
        return _ValueType::generateBinaryOperation( operation, first, second );

    llvm::Value *secondValue = second->toInteger()->toLlvm();

    switch( operation ) {
    case BinaryOperation::Assignment:
        {
            irBuilder.CreateStore( secondValue, first->toLlvmPointer() );
            return _Value::createUnusableValue();
        }

    case BinaryOperation::Addition:
        return _Value::createSsaValue( IntegerType::get(),
            irBuilder.CreateAdd(first->toLlvm(), secondValue) );

    case BinaryOperation::Subtraction:
        return _Value::createSsaValue( IntegerType::get(),
            irBuilder.CreateSub(first->toLlvm(), secondValue) );

    case BinaryOperation::Multiplication:
        return _Value::createSsaValue( IntegerType::get(),
            irBuilder.CreateMul(first->toLlvm(), secondValue) );

    case BinaryOperation::Division:
        return _Value::createSsaValue( IntegerType::get(),
            irBuilder.CreateSDiv(first->toLlvm(), secondValue) );

    case BinaryOperation::LessComparison:
        return _Value::createSsaValue( BooleanType::get(),
            irBuilder.CreateICmpSLT(first->toLlvm(), secondValue) );

    case BinaryOperation::GreaterComparison:
        return _Value::createSsaValue( BooleanType::get(),
            irBuilder.CreateICmpSGT(first->toLlvm(), secondValue) );
    }

    return _ValueType::generateBinaryOperation( operation, first, second );
}

Value IntegerType::generateUnaryOperation( UnaryOperation operation, Value operand ) const {

    if( operation == UnaryOperation::BooleanConversion )
        return _Value::createSsaValue( BooleanType::get(),
            irBuilder.CreateIsNotNull(operand->toLlvm()) );

    if( operation == UnaryOperation::IntegerConversion )
        return operand;

    if( operation == UnaryOperation::Increment ) {

        Value incremented = generateBinaryOperation(
            BinaryOperation::Addition, operand, _Value::createIntegerConstant(1) );

        return generateBinaryOperation(
            BinaryOperation::Assignment, operand, incremented );
    }

    if( operation == UnaryOperation::Decrement ) {

        Value decremented = generateBinaryOperation(
            BinaryOperation::Subtraction, operand, _Value::createIntegerConstant(1) );

        return generateBinaryOperation(
            BinaryOperation::Assignment, operand, decremented );
    }

    return _ValueType::generateUnaryOperation( operation, operand );
}

bool IntegerType::isIntegerSubset() const {
    return true;
}

bool IntegerType::isRealSubset() const {
    return true;
}

// BooleanType

ValueType BooleanType::get() {
    static const ValueType instance( new BooleanType() );
    return instance;
}

BooleanType::BooleanType() {
    _type = irBuilder.getInt1Ty();
}

Value BooleanType::generateBinaryOperation( BinaryOperation operation, Value first, Value second ) const {

    if( operation == BinaryOperation::Assignment ) {
        irBuilder.CreateStore( second->toBoolean()->toLlvm(), first->toLlvmPointer() );
        return _Value::createUnusableValue();
    }

    if( operation == BinaryOperation::LogicOr )
        return _Value::createSsaValue( BooleanType::get(),
            irBuilder.CreateOr(first->toLlvm(), second->toBoolean()->toLlvm()) );

    if( operation == BinaryOperation::LogicAnd )
        return _Value::createSsaValue( BooleanType::get(),
            irBuilder.CreateAnd(first->toLlvm(), second->toBoolean()->toLlvm()) );

    if( operation == BinaryOperation::Addition ||
        operation == BinaryOperation::Subtraction ||
        operation == BinaryOperation::Multiplication ||
        operation == BinaryOperation::Division ||
        operation == BinaryOperation::LessComparison ||
        operation == BinaryOperation::GreaterComparison )
    {
        return first->toInteger()->generateBinaryOperation( operation, second );
    }

    return _ValueType::generateBinaryOperation( operation, first, second );
}

Value BooleanType::generateUnaryOperation( UnaryOperation operation, Value operand ) const {

    if( operation == UnaryOperation::BooleanConversion )
        return operand;

    if( operation == UnaryOperation::IntegerConversion )
        return _Value::createSsaValue( IntegerType::get(),
            irBuilder.CreateZExt(operand->toLlvm(), IntegerType::get()->toLlvm()) );

    return _ValueType::generateUnaryOperation( operation, operand );
}

bool BooleanType::isIntegerSubset() const {
    return true;
}

bool BooleanType::isRealSubset() const {
    return true;
}

// PointerType

PointerType::PointerType( ValueType targetType )
    : _targetType( targetType )
{
    _type = llvm::PointerType::getUnqual( targetType->toLlvm() );
}

ValueType PointerType::getTargetType() const {
    return _targetType;
}

Value PointerType::generateUnaryOperation( UnaryOperation operation, Value operand ) const {

    if( operation == UnaryOperation::BooleanConversion )
        return _Value::createSsaValue( BooleanType::get(),
            irBuilder.CreateIsNotNull(operand->toLlvm()) );

    return _ValueType::generateUnaryOperation( operation, operand );
}

Value PointerType::generateBinaryOperation( BinaryOperation operation, Value first, Value second ) const {

    if( operation == BinaryOperation::Assignment ) {
        irBuilder.CreateStore( second->toLlvm(), first->toLlvmPointer() );
        return _Value::createUnusableValue();
    }

    if( operation == BinaryOperation::LessComparison )
        return _Value::createSsaValue( BooleanType::get(),
            irBuilder.CreateICmpSLT(first->toLlvm(), second->toLlvm()) );

    if( operation == BinaryOperation::GreaterComparison )
        return _Value::createSsaValue( BooleanType::get(),
            irBuilder.CreateICmpSGT(first->toLlvm(), second->toLlvm()) );

    return _ValueType::generateBinaryOperation( operation, first, second );
}

Value PointerType::generateMemberAccess( Value operand, const std::string &memberIdentifier ) const {

    if( memberIdentifier == "target" )
        return _Value::createReference( _targetType, operand->toLlvm() );

    return _ValueType::generateMemberAccess( operand, memberIdentifier );
}

// StringType

ValueType StringType::get() {
    static const ValueType instance( new StringType() );
    return instance;
}

StringType::StringType() {
    _type = irBuilder.getInt8PtrTy();
}

Value StringType::generateMemberAccess( Value operand, const std::string &memberIdentifier ) const {

    if( memberIdentifier == "data" )
        return _Value::createSsaValue(
            std::make_shared<PointerType>(IntegerType::get()),
            irBuilder.CreateStructGEP(operand->toLlvm(), 0) );

    return _ValueType::generateMemberAccess( operand, memberIdentifier );
}

// FunctionType

void FunctionType::Builder::addArgument(
    const std::string &identifier, ValueType type )
{
    _argumentTypes.push_back( type );
}

void FunctionType::Builder::setReturnType( ValueType type ) {
    _resultType = type;
}

ValueType FunctionType::Builder::build() const {
    return std::make_shared< FunctionType >( _argumentTypes, _resultType );
}

FunctionType::FunctionType( const std::vector<ValueType> &argumentTypes, ValueType resultType )
    : _resultType( resultType )
{
    const bool isVariableArgument = false;

    std::vector<llvm::Type*> llvmArgumentTypes( argumentTypes.size() );

    std::transform(
        argumentTypes.cbegin(),
        argumentTypes.cend(),
        llvmArgumentTypes.begin(),
        []( ValueType type ) -> llvm::Type* {
            return type->toLlvm();
        });

    llvm::Type *llvmResultType = _resultType ? _resultType->toLlvm() : irBuilder.getVoidTy();

    _type = llvm::FunctionType::get( llvmResultType, llvmArgumentTypes, isVariableArgument );
}

Value FunctionType::generateCall( Value callee, const std::vector<Value> &arguments ) const {

    std::vector<llvm::Value*> llvmArguments( arguments.size() );

    std::transform(
        arguments.cbegin(),
        arguments.cend(),
        llvmArguments.begin(),
        []( Value value ) -> llvm::Value* {
            return value->toLlvm();
        });

    llvm::Value *llvmResultValue = irBuilder.CreateCall( callee->toLlvm(), llvmArguments );

    if( _resultType )
        return _Value::createSsaValue( _resultType, llvmResultValue );
    else
        return _Value::createUnusableValue();
}

// BoundMethodType

class BoundMethodType : public _ValueType {
public:
    BoundMethodType( Value method, Value instance )
        : _methodValueType( method->getType() ), _instance( instance ) {}

    Value generateCall(
        Value callee, const std::vector<Value> &arguments ) const
    {
        std::vector< Value > extendedArguments( arguments.size() + 1 );
        extendedArguments[0] = _instance->generateUnaryOperation(
            UnaryOperation::Addressing );
        std::copy(
            arguments.begin(), arguments.end(), extendedArguments.begin() + 1 );
        return _methodValueType->generateCall( callee, extendedArguments );
    }

private:
    ValueType _methodValueType;
    Value _instance;
};

// ClassType

ClassType::ClassType(
    const std::string &identifier, const std::vector<Member> &members )
        : _identifier( identifier ), _members( members )
{
    std::vector< llvm::Type* > memberTypes( _members.size() );
    std::transform(
        _members.cbegin(),
        _members.cend(),
        memberTypes.begin(),
        []( const Member &member ) -> llvm::Type* {
            return member.type->toLlvm();
        });
    _type = llvm::StructType::create( memberTypes );
}

void ClassType::addMethod( const std::string &identifier, Value value ) {
    Method method = { identifier, value };
    _methods.push_back( method );
}

const std::string& ClassType::getIdentifier() const {
    return _identifier;
}

const std::vector<ClassType::Member>& ClassType::getMembers() const {
    return _members;
}

const std::vector<ClassType::Method>& ClassType::getMethods() const {
    return _methods;
}

Value ClassType::generateMemberAccess(
    Value classInstance, const std::string &identifier ) const
{
    auto memberIterator = std::find_if(
        _members.cbegin(),
        _members.cend(),
        [&identifier]( const Member &member ) -> bool {
            return member.identifier == identifier;
        });

    if( memberIterator != _members.end() ) {

        const size_t memberIndex =
            std::distance( _members.begin(), memberIterator );

        if( classInstance->isAssignable() )
            return _Value::createReference(
                memberIterator->type, irBuilder.CreateStructGEP(
                    classInstance->toLlvmPointer(), memberIndex) );
        else {
            unsigned indices[] = { memberIndex };
            return _Value::createSsaValue(
                memberIterator->type, irBuilder.CreateExtractValue(
                classInstance->toLlvm(), indices) );
        }
    }

    auto methodIterator = std::find_if(
        _methods.cbegin(),
        _methods.cend(),
        [&identifier]( const Method &method ) -> bool {
            return method.identifier == identifier;
        });

    if( methodIterator != _methods.end() ) {
        Value value = methodIterator->value;
        ValueType type = std::make_shared< BoundMethodType >(
            value, classInstance );
        return _Value::createSsaValue( type, value->toLlvm() );
    }

    return _ValueType::generateMemberAccess( classInstance, identifier );
}

}
