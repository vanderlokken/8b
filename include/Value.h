#pragma once

#include <memory>
#include <string>
#include <vector>

#include <llvm/Value.h>

#include "ClassId.h"
#include "Operation.h"

namespace _8b {


class Value;
typedef std::shared_ptr<const Value> ValuePointer;

class ValueType;
typedef std::shared_ptr<ValueType> ValueTypePointer;


class Value : public std::enable_shared_from_this<Value> {
public:

    static ValuePointer createVariable( ValueTypePointer type, const std::string &identifier );
    static ValuePointer createReference( ValueTypePointer type, llvm::Value* );
    static ValuePointer createSsaValue( ValueTypePointer type, llvm::Value* );
    static ValuePointer createUnusableValue();
    static ValuePointer createIntegerConstant( int );
    static ValuePointer createBooleanConstant( bool );
    static ValuePointer createStringConstant( const std::string& );

    Value( ValueTypePointer type, llvm::Value *llvmValue, bool assignable )
        : _type( type ), _llvmValue( llvmValue ), _assignable( assignable ) {}

    ValueTypePointer getType() const;
    llvm::Value* toLlvm() const;
    llvm::Value* toLlvmPointer() const;

    ValuePointer generateBinaryOperation( BinaryOperation, ValuePointer ) const;
    ValuePointer generateUnaryOperation( UnaryOperation ) const;
    ValuePointer generateCall( const std::vector<ValuePointer>& ) const;
    ValuePointer generateMemberAccess( const std::string &memberIdentifier ) const;

    ValuePointer toBoolean() const;
    ValuePointer toInteger() const;

protected:
    ValueTypePointer _type;
    llvm::Value *_llvmValue;
    bool _assignable;
};


class ValueType : public BaseIdClass {
public:
    virtual ~ValueType() {}

    virtual llvm::Type* toLlvm() const;

    virtual ValuePointer generateBinaryOperation( BinaryOperation, ValuePointer, ValuePointer ) const;
    virtual ValuePointer generateUnaryOperation( UnaryOperation, ValuePointer ) const;
    virtual ValuePointer generateCall( ValuePointer, const std::vector<ValuePointer>& ) const;
    virtual ValuePointer generateMemberAccess( ValuePointer, const std::string &memberIdentifier ) const;

protected:
    llvm::Type *_type;
};


template<class T>
class _ValueType : public DerivedIdClass<ValueType, T> {};


class IntegerType : public _ValueType<IntegerType> {
public:
    static ValueTypePointer get( int bitWidth = 32 );

    ValuePointer generateBinaryOperation( BinaryOperation, ValuePointer, ValuePointer ) const;
    ValuePointer generateUnaryOperation( UnaryOperation, ValuePointer ) const;

private:
    IntegerType( int bitWidth );

    static llvm::Value* integerOperand( ValuePointer );
};


class BooleanType : public _ValueType<BooleanType> {
public:
    static ValueTypePointer get();

    ValuePointer generateBinaryOperation( BinaryOperation, ValuePointer, ValuePointer ) const;
    ValuePointer generateUnaryOperation( UnaryOperation, ValuePointer ) const;

private:
    BooleanType();
};


class PointerType : public _ValueType<PointerType> {
public:
    PointerType( ValueTypePointer targetType );

    ValuePointer generateBinaryOperation( BinaryOperation, ValuePointer, ValuePointer ) const;
    ValuePointer generateUnaryOperation( UnaryOperation, ValuePointer ) const;
    ValuePointer generateMemberAccess( ValuePointer, const std::string &memberIdentifier ) const;
    
private:
    ValueTypePointer _targetType;
};


class StringType : public _ValueType<StringType> {
public:
    static ValueTypePointer get();
    
    ValuePointer generateMemberAccess( ValuePointer, const std::string &memberIdentifier ) const;
    
private:
    StringType();
};


class FunctionType : public _ValueType<FunctionType> {
public:
    FunctionType( const std::vector<ValueTypePointer>&, ValueTypePointer resultType = nullptr );

    ValuePointer generateCall( ValuePointer, const std::vector<ValuePointer>& ) const;

private:
    ValueTypePointer _resultType;
};


class ClassType : public _ValueType<ClassType> {
public:

    struct Member {
        std::string identifier;
        ValueTypePointer type;
    };

    ClassType( const std::vector<Member>& );

    ValuePointer generateMemberAccess( ValuePointer, const std::string &memberIdentifier ) const;

private:
    std::vector<Member> _members;
};


}
