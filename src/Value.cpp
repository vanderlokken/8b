#include "Value.h"

#include <llvm/Argument.h>
#include <llvm/Constants.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Type.h>

#include "Exception.h"

namespace _8b {

extern llvm::IRBuilder<> irBuilder;

// ----------------------------------------------------------------------------
//  Value
// ----------------------------------------------------------------------------

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
    static const Value value =
        _Value::createSsaValue( UnusableType::get(), nullptr );
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

Value _Value::generateBinaryOperation(
    BinaryOperation operation, Value second ) const
{
    Value first = shared_from_this();

    switch( operation ) {
    case BinaryOperation::Assignment:
        return _type->generateAssignment( first, second );
    case BinaryOperation::Addition:
        return _type->generateAddition( first, second );
    case BinaryOperation::Subtraction:
        return _type->generateSubtraction( first, second );
    case BinaryOperation::Multiplication:
        return _type->generateMultiplication( first, second );
    case BinaryOperation::Division:
        return _type->generateDivision( first, second );
    case BinaryOperation::LogicAnd:
        return _type->generateLogicAnd( first, second );
    case BinaryOperation::LogicOr:
        return _type->generateLogicOr( first, second );
    case BinaryOperation::LessComparison:
        return _type->generateLessComparison( first, second );
    case BinaryOperation::GreaterComparison:
        return _type->generateGreaterComparison( first, second );
    }

    throw NotImplementedError();
}

Value _Value::generateUnaryOperation( UnaryOperation operation ) const {

    Value operand = shared_from_this();

    switch( operation ) {
    case UnaryOperation::Increment:
        return _type->generateIncrement( operand );
    case UnaryOperation::Decrement:
        return _type->generateDecrement( operand );
    case UnaryOperation::BooleanConversion:
        return _type->generateBooleanConversion( operand );
    case UnaryOperation::IntegerConversion:
        return _type->generateIntegerConversion( operand );
    case UnaryOperation::RealConversion:
        return _type->generateRealConversion( operand );
    case UnaryOperation::PointerConversion:
        return _type->generatePointerConversion( operand );
    }

    throw NotImplementedError();
}

Value _Value::generateCall( const std::vector<Value> &arguments ) const {
    return _type->generateCall( shared_from_this(), arguments );
}

Value _Value::generateMemberAccess( const std::string &identifier ) const {
    return _type->generateMemberAccess( shared_from_this(), identifier );
}

Value _Value::toBoolean() const {
    return _type->generateBooleanConversion( shared_from_this() );
}

Value _Value::toInteger() const {
    return _type->generateIntegerConversion( shared_from_this() );
}

Value _Value::toReal() const {
    return _type->generateRealConversion( shared_from_this() );
}

// ----------------------------------------------------------------------------
//  ValueType
// ----------------------------------------------------------------------------

llvm::Type* _ValueType::toLlvm() const {
    return _type;
}

// Assignment operations

Value _ValueType::generateAssignment( Value, Value ) const {
    throw SemanticError( "An operation is not implemented" );
}

Value _ValueType::generateIncrement( Value ) const {
    throw SemanticError( "An operation is not implemented" );
}

Value _ValueType::generateDecrement( Value ) const {
    throw SemanticError( "An operation is not implemented" );
}

// Arithmetic operations

Value _ValueType::generateAddition( Value, Value ) const {
    throw SemanticError( "An operation is not implemented" );
}

Value _ValueType::generateSubtraction( Value, Value ) const {
    throw SemanticError( "An operation is not implemented" );
}

Value _ValueType::generateMultiplication( Value, Value ) const {
    throw SemanticError( "An operation is not implemented" );
}

Value _ValueType::generateDivision( Value, Value ) const {
    throw SemanticError( "An operation is not implemented" );
}

// Comparison operations

Value _ValueType::generateLessComparison( Value, Value ) const {
    throw SemanticError( "An operation is not implemented" );
}

Value _ValueType::generateGreaterComparison( Value, Value ) const {
    throw SemanticError( "An operation is not implemented" );
}

// Conversion operations

Value _ValueType::generateBooleanConversion( Value ) const {
    throw SemanticError( "An operand cannot be converted to a boolean value" );
}

Value _ValueType::generateIntegerConversion( Value ) const {
    throw SemanticError( "An operand cannot be converted to an integer value" );
}

Value _ValueType::generateRealConversion( Value ) const {
    throw SemanticError( "An operand cannot be converted to a real value" );
}

// Logic operations

Value _ValueType::generateLogicAnd( Value first, Value second ) const {
    llvm::Value *result = irBuilder.CreateAnd(
        first->toBoolean()->toLlvm(), second->toBoolean()->toLlvm() );
    return createBoolean( result );
}

Value _ValueType::generateLogicOr( Value first, Value second ) const {
    llvm::Value *result = irBuilder.CreateOr(
        first->toBoolean()->toLlvm(), second->toBoolean()->toLlvm() );
    return createBoolean( result );
}

// Other operations

Value _ValueType::generateCall( Value, const std::vector<Value>& ) const {
    throw SemanticError( "An operand is not callable" );
}

Value _ValueType::generateMemberAccess( Value, const std::string& ) const {
    throw SemanticError( "An operand has no such property" );
}

Value _ValueType::generatePointerConversion( Value operand ) const {
    ValueType resultType =
        std::make_shared< PointerType >( operand->getType() );
    return _Value::createSsaValue( resultType, operand->toLlvmPointer() );
}

bool _ValueType::isIntegerSubset() const {
    return false;
}

bool _ValueType::isRealSubset() const {
    return false;
}

Value _ValueType::createUnusable( llvm::Value *value ) {
    return _Value::createSsaValue( UnusableType::get(), value );
}

Value _ValueType::createBoolean( llvm::Value *value ) {
    return _Value::createSsaValue( BooleanType::get(), value );
}

Value _ValueType::createInteger( llvm::Value *value ) {
    return _Value::createSsaValue( IntegerType::get(), value );
}

Value _ValueType::createReal( llvm::Value *value ) {
    return _Value::createSsaValue( RealType::get(), value );
}

// ----------------------------------------------------------------------------
//  IntegerType
// ----------------------------------------------------------------------------

ValueType IntegerType::get( int bitWidth ) {

    if( bitWidth == 1 )
        throw SemanticError(
            "'integer[1]' is incorrect; 'boolean' should be used instead" );

    if( bitWidth != 32 )
        throw SemanticError( "Only 32-bit integer values are supported" );

    static const ValueType instance =
        std::make_shared< IntegerType >( bitWidth );
    return instance;
}

IntegerType::IntegerType( int bitWidth ) {

    switch( bitWidth ) {
    case 1:
        _type = irBuilder.getInt1Ty();
        break;

    case 32:
        _type = irBuilder.getInt32Ty();
        break;

    default:
        throw NotImplementedError();
    }
}

// Assignment operations

Value IntegerType::generateAssignment( Value first, Value second ) const {
    return createUnusable(
        irBuilder.CreateStore(convertOperand(second), first->toLlvmPointer()) );
}

Value IntegerType::generateIncrement( Value operand ) const {
    Value incremented =
        generateAddition( operand, _Value::createIntegerConstant(1) );
    return generateAssignment( operand, incremented );
}

Value IntegerType::generateDecrement( Value operand ) const {
    Value decremented =
        generateSubtraction( operand, _Value::createIntegerConstant(1) );
    return generateAssignment( operand, decremented );
}

// Arithmetic operations

Value IntegerType::generateAddition( Value first, Value second ) const {
    return createInteger(
        irBuilder.CreateAdd(convertOperand(first), convertOperand(second)) );
}

Value IntegerType::generateSubtraction( Value first, Value second ) const {
    return createInteger(
        irBuilder.CreateSub(convertOperand(first), convertOperand(second)) );
}

Value IntegerType::generateMultiplication( Value first, Value second ) const {
    return createInteger(
        irBuilder.CreateMul(convertOperand(first), convertOperand(second)) );
}

Value IntegerType::generateDivision( Value first, Value second ) const {
    return createInteger(
        irBuilder.CreateSDiv(convertOperand(first), convertOperand(second)) );
}

// Comparison operations

Value IntegerType::generateLessComparison( Value first, Value second ) const {
    return createBoolean(
        irBuilder.CreateICmpSLT(convertOperand(first), convertOperand(second))
    );
}

Value IntegerType::generateGreaterComparison( Value first, Value second ) const
{
    return createBoolean(
        irBuilder.CreateICmpSGT(convertOperand(first), convertOperand(second))
    );
}

// Conversion operations

Value IntegerType::generateBooleanConversion( Value operand ) const {
    return createBoolean( irBuilder.CreateIsNotNull(convertOperand(operand)) );
}

Value IntegerType::generateIntegerConversion( Value operand ) const {
    return operand;
}

Value IntegerType::generateRealConversion( Value operand ) const {
    llvm::Value *result = irBuilder.CreateSIToFP(
        convertOperand(operand), irBuilder.getFloatTy() );
    return createReal( result );
}

// Miscellaneous

bool IntegerType::isIntegerSubset() const {
    return true;
}

bool IntegerType::isRealSubset() const {
    return true;
}

llvm::Value* IntegerType::convertOperand( Value operand ) {
    if( !operand->getType()->isIntegerSubset() )
        throw SemanticError( "An operand is not an integer value" );
    return operand->toInteger()->toLlvm();
}

// ----------------------------------------------------------------------------
//  BooleanType
// ----------------------------------------------------------------------------

ValueType BooleanType::get() {
    static const ValueType instance = std::make_shared< BooleanType >();
    return instance;
}

BooleanType::BooleanType() : IntegerType( 1 ) {}

// Assignment operations

Value BooleanType::generateAssignment( Value first, Value second ) const {
    llvm::Value *result = irBuilder.CreateStore(
        second->toBoolean()->toLlvm(), first->toLlvmPointer() );
    return createUnusable( result );
}

Value BooleanType::generateIncrement( Value operand ) const {
    throw SemanticError( "A boolean value cannot be incremented" );
}

Value BooleanType::generateDecrement( Value operand ) const {
    throw SemanticError( "A boolean value cannot be decremented" );
}

// Conversion operations

Value BooleanType::generateBooleanConversion( Value operand ) const {
    return operand;
}

Value BooleanType::generateIntegerConversion( Value operand ) const {
    return createInteger(
        irBuilder.CreateZExt(operand->toLlvm(), irBuilder.getInt32Ty()) );
}

Value BooleanType::generateRealConversion( Value operand ) const {
    llvm::Value *result = irBuilder.CreateSIToFP(
        operand->toLlvm(), irBuilder.getFloatTy() );
    return createReal( result );
}

// ----------------------------------------------------------------------------
//  PointerType
// ----------------------------------------------------------------------------

PointerType::PointerType( ValueType targetType )
    : _targetType( targetType )
{
    _type = llvm::PointerType::getUnqual( targetType->toLlvm() );
}

ValueType PointerType::getTargetType() const {
    return _targetType;
}

// Assignment operations

Value PointerType::generateAssignment( Value first, Value second ) const {
    // TODO: check the type of 'second'
    llvm::Value *result =
        irBuilder.CreateStore( second->toLlvm(), first->toLlvmPointer() );
    return createUnusable( result );
}

// Conversion operations

Value PointerType::generateBooleanConversion( Value operand ) const {
    return createBoolean( irBuilder.CreateIsNotNull(operand->toLlvm()) );
}

Value PointerType::generateIntegerConversion( Value operand ) const {
    return createInteger(
        irBuilder.CreatePtrToInt(operand->toLlvm(), irBuilder.getInt32Ty()) );
}

// Other operations

Value PointerType::generateMemberAccess(
    Value operand, const std::string &identifier ) const
{
    if( identifier == "target" )
        return _Value::createReference( _targetType, operand->toLlvm() );
    return _ValueType::generateMemberAccess( operand, identifier );
}

// ----------------------------------------------------------------------------
//  RealType
// ----------------------------------------------------------------------------

ValueType RealType::get( int bitWidth ) {

    if( bitWidth != 32 )
        throw SemanticError( "Only 32-bit real values are supported" );

    static const ValueType instance = std::make_shared< RealType >( bitWidth );
    return instance;
}

RealType::RealType( int bitWidth ) {
    _type = irBuilder.getFloatTy();
}

// Assignment operations

Value RealType::generateAssignment( Value first, Value second ) const {
    return createUnusable(
        irBuilder.CreateStore(convertOperand(second), first->toLlvmPointer()) );
}

// Arithmetic operations

Value RealType::generateAddition( Value first, Value second ) const {
    return createReal(
        irBuilder.CreateFAdd(convertOperand(first), convertOperand(second)) );
}

Value RealType::generateSubtraction( Value first, Value second ) const {
    return createReal(
        irBuilder.CreateFSub(convertOperand(first), convertOperand(second)) );
}

Value RealType::generateMultiplication( Value first, Value second ) const {
    return createReal(
        irBuilder.CreateFMul(convertOperand(first), convertOperand(second)) );
}

Value RealType::generateDivision( Value first, Value second ) const {
    return createReal(
        irBuilder.CreateFDiv(convertOperand(first), convertOperand(second)) );
}

// Comparison operations

Value RealType::generateLessComparison( Value first, Value second ) const {
    return createBoolean(
        irBuilder.CreateFCmpOLT(convertOperand(first), convertOperand(second))
    );
}

Value RealType::generateGreaterComparison( Value first, Value second ) const {
    return createBoolean(
        irBuilder.CreateFCmpOGT(convertOperand(first), convertOperand(second))
    );
}

// Conversion operations

Value RealType::generateBooleanConversion( Value operand ) const {
    return createBoolean( irBuilder.CreateIsNotNull(convertOperand(operand)) );
}

Value RealType::generateIntegerConversion( Value operand ) const {
    llvm::Value *result = irBuilder.CreateFPToSI(
        convertOperand(operand), irBuilder.getInt32Ty() );
    return createInteger( result );
}

Value RealType::generateRealConversion( Value operand ) const {
    return operand;
}

// Miscellaneous

bool RealType::isRealSubset() const {
    return true;
}

llvm::Value* RealType::convertOperand( Value operand ) {
    if( !operand->getType()->isRealSubset() )
        throw SemanticError( "An operand is not a real value" );
    return operand->toReal()->toLlvm();
}

// ----------------------------------------------------------------------------
//  StringType
// ----------------------------------------------------------------------------

ValueType StringType::get() {
    static const ValueType instance = std::make_shared< StringType >();
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

// ----------------------------------------------------------------------------
//  FunctionType
// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------
//  BoundMethodType
// ----------------------------------------------------------------------------

class BoundMethodType : public _ValueType {
public:
    BoundMethodType( Value method, Value instance )
        : _methodValueType( method->getType() ), _instance( instance ) {}

    Value generateCall(
        Value callee, const std::vector<Value> &arguments ) const
    {
        std::vector< Value > extendedArguments( arguments.size() + 1 );
        extendedArguments[0] = _instance->generateUnaryOperation(
            UnaryOperation::PointerConversion );
        std::copy(
            arguments.begin(), arguments.end(), extendedArguments.begin() + 1 );
        return _methodValueType->generateCall( callee, extendedArguments );
    }

private:
    ValueType _methodValueType;
    Value _instance;
};

// ----------------------------------------------------------------------------
//  ClassType
// ----------------------------------------------------------------------------

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
