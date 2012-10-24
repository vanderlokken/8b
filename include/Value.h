#pragma once

#include <memory>
#include <vector>

#include <llvm/Value.h>

#include "ClassId.h"

namespace _8b {


class Value;
typedef std::shared_ptr<Value> ValuePointer;

class IntegerValue;
typedef std::shared_ptr<IntegerValue> IntegerValuePointer;

class BooleanValue;
typedef std::shared_ptr<BooleanValue> BooleanValuePointer;

class FunctionValue;
typedef std::shared_ptr<FunctionValue> FunctionValuePointer;


class Value : public BaseIdClass {
public:

    Value() : _llvmValue( 0 ) {}

    virtual ~Value() {}

    llvm::Value* getLlvmValue() const;

    virtual ValuePointer generateAdd( ValuePointer ) const;
    virtual ValuePointer generateSubtract( ValuePointer ) const;
    virtual ValuePointer generateMultiply( ValuePointer ) const;
    virtual ValuePointer generateDivide( ValuePointer ) const;

    virtual BooleanValuePointer generateOr( ValuePointer ) const;
    virtual BooleanValuePointer generateAnd( ValuePointer ) const;
    virtual BooleanValuePointer generateLess( ValuePointer ) const;
    virtual BooleanValuePointer generateGreater( ValuePointer ) const;

    virtual void generateAssignment( ValuePointer ) const = 0;
    virtual void generateIncrement() const;
    virtual void generateDecrement() const;

    virtual BooleanValuePointer generateToBoolean() const;
    virtual IntegerValuePointer generateToInteger() const;

    virtual ValuePointer generateCall( const std::vector<ValuePointer>& ) const;
    
protected:
    llvm::Value *_llvmValue;
};

template<class T>
class ValueType : public DerivedIdClass<Value, T> {};


class IntegerValue : public ValueType<IntegerValue> {
public:

    static IntegerValuePointer create( const std::string &name, int bitWidth = 32 );
    static IntegerValuePointer create( int value );
    static IntegerValuePointer create( llvm::Value *value );

    ValuePointer generateAdd( ValuePointer ) const;
    ValuePointer generateSubtract( ValuePointer ) const;
    ValuePointer generateMultiply( ValuePointer ) const;
    ValuePointer generateDivide( ValuePointer ) const;

    BooleanValuePointer generateLess( ValuePointer ) const;
    BooleanValuePointer generateGreater( ValuePointer ) const;

    void generateAssignment( ValuePointer ) const;
    void generateIncrement() const;
    void generateDecrement() const;

    BooleanValuePointer generateToBoolean() const;

    IntegerValue( const std::string &name, int bitWidth );
    IntegerValue( int value );
    IntegerValue( llvm::Value *value ) { _llvmValue = value; }

private:
    static llvm::Value* integerOperand( ValuePointer );
};


class BooleanValue : public ValueType<BooleanValue> {
public:

    static BooleanValuePointer create( const std::string &name );
    static BooleanValuePointer create( bool );
    static BooleanValuePointer create( llvm::Value *value );

    void generateAssignment( ValuePointer ) const;

    IntegerValuePointer generateToInteger() const;

    BooleanValue( const std::string &name );
    BooleanValue( bool );
    BooleanValue( llvm::Value *value ) { _llvmValue = value; }
};


class FunctionValue : public ValueType<FunctionValue> {
public:

    static FunctionValuePointer create( llvm::Value *value );

    void generateAssignment( ValuePointer ) const;

    ValuePointer generateCall( const std::vector<ValuePointer>& ) const;
    
    FunctionValue( llvm::Value *value ) { _llvmValue = value; }
};

}
