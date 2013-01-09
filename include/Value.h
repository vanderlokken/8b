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

protected:
    ValueType _type;
    llvm::Value *_llvmValue;
    bool _assignable;
};


class _ValueType {
public:
    virtual ~_ValueType() {}

    virtual llvm::Type* toLlvm() const;

    virtual Value generateBinaryOperation( BinaryOperation, Value, Value ) const;
    virtual Value generateUnaryOperation( UnaryOperation, Value ) const;
    virtual Value generateCall( Value, const std::vector<Value>& ) const;
    virtual Value generateMemberAccess( Value, const std::string &memberIdentifier ) const;

    virtual bool isIntegerSubset() const;
    virtual bool isRealSubset() const;

protected:
    llvm::Type *_type;
};


class IntegerType : public _ValueType {
public:
    static ValueType get( int bitWidth = 32 );

    Value generateBinaryOperation( BinaryOperation, Value, Value ) const;
    Value generateUnaryOperation( UnaryOperation, Value ) const;

private:
    IntegerType( int bitWidth );

    bool isIntegerSubset() const;
    bool isRealSubset() const;
};


class BooleanType : public _ValueType {
public:
    static ValueType get();

    Value generateBinaryOperation( BinaryOperation, Value, Value ) const;
    Value generateUnaryOperation( UnaryOperation, Value ) const;

private:
    BooleanType();

    bool isIntegerSubset() const;
    bool isRealSubset() const;
};


class PointerType : public _ValueType {
public:
    PointerType( ValueType targetType );

    ValueType getTargetType() const;

    Value generateBinaryOperation( BinaryOperation, Value, Value ) const;
    Value generateUnaryOperation( UnaryOperation, Value ) const;
    Value generateMemberAccess( Value, const std::string &memberIdentifier ) const;

private:
    ValueType _targetType;
};


class StringType : public _ValueType {
public:
    static ValueType get();

    Value generateMemberAccess( Value, const std::string &memberIdentifier ) const;

private:
    StringType();
};


class FunctionType : public _ValueType {
public:

    class Builder {
    public:
        void addArgument(
            const std::string &identifier, ValueType type );
        void setReturnType( ValueType type );

        ValueType build() const;

    private:
        std::vector< ValueType > _argumentTypes;
        ValueType _resultType;
    };

    FunctionType( const std::vector<ValueType>&, ValueType resultType = nullptr );

    Value generateCall( Value, const std::vector<Value>& ) const;

private:
    ValueType _resultType;
};


class ClassType : public _ValueType {
public:

    struct Member {
        std::string identifier;
        ValueType type;
    };

    struct Method {
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
