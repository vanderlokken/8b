#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <llvm/Value.h>

#include "Operation.h"

namespace _8b {


struct SemanticError : std::exception {
    SemanticError( const char *message ) : std::exception( message ) {}
};


class _Value;
typedef std::shared_ptr<const _Value> Value;

class _ValueType;
typedef std::shared_ptr<_ValueType> ValueType;


class _Value : public std::enable_shared_from_this<_Value> {
public:

    static Value createVariable( ValueType type, const std::string &identifier );
    static Value createReference( ValueType type, llvm::Value* );
    static Value createSsaValue( ValueType type, llvm::Value* );
    static Value createUnusableValue();
    static Value createIntegerConstant( int );
    static Value createBooleanConstant( bool );
    static Value createStringConstant( const std::string& );

    _Value( ValueType type, llvm::Value *llvmValue, bool assignable )
        : _type( type ), _llvmValue( llvmValue ), _assignable( assignable ) {}

    ValueType getType() const;
    llvm::Value* toLlvm() const;
    llvm::Value* toLlvmPointer() const;

    bool isAssignable() const;

    Value generateBinaryOperation( BinaryOperation, Value ) const;
    Value generateUnaryOperation( UnaryOperation ) const;
    Value generateCall( const std::vector<Value>& ) const;
    Value generateMemberAccess( const std::string &memberIdentifier ) const;

    Value toBoolean() const;
    Value toInteger() const;
    Value toReal() const;

protected:
    ValueType _type;
    llvm::Value *_llvmValue;
    bool _assignable;
};

// ----------------------------------------------------------------------------
//  _ValueType
// ----------------------------------------------------------------------------

class _ValueType {
public:
    virtual ~_ValueType() {}

    virtual llvm::Type* toLlvm() const;

    // Assignment operations

    virtual Value generateAssignment( Value, Value ) const;
    virtual Value generateIncrement( Value ) const;
    virtual Value generateDecrement( Value ) const;

    // Arithmetic operations

    virtual Value generateAddition( Value, Value ) const;
    virtual Value generateSubtraction( Value, Value ) const;
    virtual Value generateMultiplication( Value, Value ) const;
    virtual Value generateDivision( Value, Value ) const;

    // Comparison operations

    virtual Value generateLessComparison( Value, Value ) const;
    virtual Value generateGreaterComparison( Value, Value ) const;

    // Conversion operations

    virtual Value generateBooleanConversion( Value ) const;
    virtual Value generateIntegerConversion( Value ) const;
    virtual Value generateRealConversion( Value ) const;

    // Logic operations

    Value generateLogicAnd( Value, Value ) const;
    Value generateLogicOr( Value, Value ) const;

    // Other operations

    virtual Value generateCall( Value, const std::vector<Value>& ) const;
    virtual Value generateMemberAccess( Value, const std::string& ) const;

    Value generatePointerConversion( Value ) const;

    virtual bool isIntegerSubset() const;
    virtual bool isRealSubset() const;

protected:
    static Value createUnusable( llvm::Value* );
    static Value createBoolean( llvm::Value* );
    static Value createInteger( llvm::Value* );
    static Value createReal( llvm::Value* );

    llvm::Type *_type;
};

// ----------------------------------------------------------------------------
//  UnusableType
// ----------------------------------------------------------------------------

class UnusableType : public _ValueType {
public:
    static ValueType get() {
        static const ValueType instance = std::make_shared< UnusableType >();
        return instance;
    }
};

// ----------------------------------------------------------------------------
//  IntegerType
// ----------------------------------------------------------------------------

class IntegerType : public _ValueType {
public:

    IntegerType( int bitWidth );

    static ValueType get( int bitWidth = 32 );

    // Assignment operations

    virtual Value generateAssignment( Value, Value ) const;
    virtual Value generateIncrement( Value ) const;
    virtual Value generateDecrement( Value ) const;

    // Arithmetic operations

    virtual Value generateAddition( Value, Value ) const;
    virtual Value generateSubtraction( Value, Value ) const;
    virtual Value generateMultiplication( Value, Value ) const;
    virtual Value generateDivision( Value, Value ) const;

    // Comparison operations

    virtual Value generateLessComparison( Value, Value ) const;
    virtual Value generateGreaterComparison( Value, Value ) const;

    // Conversion operations

    virtual Value generateBooleanConversion( Value ) const;
    virtual Value generateIntegerConversion( Value ) const;
    virtual Value generateRealConversion( Value ) const;

protected:
    bool isIntegerSubset() const;
    bool isRealSubset() const;

    static llvm::Value* convertOperand( Value );
};

// ----------------------------------------------------------------------------
//  BooleanType
// ----------------------------------------------------------------------------

class BooleanType : public IntegerType {
public:

    BooleanType();

    static ValueType get();

    // Assignment operations

    virtual Value generateAssignment( Value, Value ) const;
    virtual Value generateIncrement( Value ) const;
    virtual Value generateDecrement( Value ) const;

    // Conversion operations

    virtual Value generateBooleanConversion( Value ) const;
    virtual Value generateIntegerConversion( Value ) const;
    virtual Value generateRealConversion( Value ) const;
};

// ----------------------------------------------------------------------------
//  PointerType
// ----------------------------------------------------------------------------

class PointerType : public _ValueType {
public:

    static ValueType get( ValueType targetType );

    PointerType( ValueType targetType );

    ValueType getTargetType() const;

    // Assignment operations

    virtual Value generateAssignment( Value, Value ) const;

    // Conversion operations

    virtual Value generateBooleanConversion( Value ) const;
    virtual Value generateIntegerConversion( Value ) const;

    // Other operations

    virtual Value generateMemberAccess( Value, const std::string& ) const;

private:
    ValueType _targetType;
};

// ----------------------------------------------------------------------------
//  RealType
// ----------------------------------------------------------------------------

class RealType : public _ValueType {
public:

    RealType( int bitWidth );

    static ValueType get( int bitWidth = 32 );

    // Assignment operations

    virtual Value generateAssignment( Value, Value ) const;

    // Arithmetic operations

    virtual Value generateAddition( Value, Value ) const;
    virtual Value generateSubtraction( Value, Value ) const;
    virtual Value generateMultiplication( Value, Value ) const;
    virtual Value generateDivision( Value, Value ) const;

    // Comparison operations

    virtual Value generateLessComparison( Value, Value ) const;
    virtual Value generateGreaterComparison( Value, Value ) const;

    // Conversion operations

    virtual Value generateBooleanConversion( Value ) const;
    virtual Value generateIntegerConversion( Value ) const;
    virtual Value generateRealConversion( Value ) const;

private:
    bool isRealSubset() const;

    static llvm::Value* convertOperand( Value );
};

// ----------------------------------------------------------------------------
//  StringType
// ----------------------------------------------------------------------------

class StringType : public _ValueType {
public:

    StringType();

    static ValueType get();

    Value generateMemberAccess( Value, const std::string& ) const;
};

// ----------------------------------------------------------------------------
//  FunctionType
// ----------------------------------------------------------------------------

class FunctionType : public _ValueType {
public:

    struct Argument {

        Argument( const std::string &identifier, ValueType type )
            : identifier( identifier ), type( type ) {}

        std::string identifier;
        ValueType type;
    };

    FunctionType(
        const std::vector<Argument>&, ValueType returnType = nullptr );

    Value generateCall( Value, const std::vector<Value>& ) const;

    const std::vector<Argument>& getArguments() const;
    ValueType getReturnType() const;

private:
    std::vector<Argument> _arguments;
    ValueType _returnType;
};

// ----------------------------------------------------------------------------
//  ClassType
// ----------------------------------------------------------------------------

class ClassType : public _ValueType {
public:

    struct Member {

        Member( const std::string &identifier, ValueType type )
            : identifier( identifier ), type( type ) {}

        std::string identifier;
        ValueType type;
    };

    struct Method {

        Method( const std::string &identifier, Value value )
            : identifier( identifier ), value( value ) {}

        std::string identifier;
        Value value;
    };

    ClassType(
        const std::string &identifier, const std::vector<Member> &members );

    void addMethod( const std::string&, Value );

    const std::string& getIdentifier() const;
    const std::vector<Member>& getMembers() const;
    const std::vector<Method>& getMethods() const;

    Value generateMemberAccess( Value, const std::string& ) const;

private:
    std::string _identifier;
    std::vector< Member > _members;
    std::vector< Method > _methods;
};


}
