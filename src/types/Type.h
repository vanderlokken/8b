#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Value.h"

namespace _8b{

class TypeBuilder;

class _Type : public std::enable_shared_from_this<_Type> {
public:
    enum class Operator {
        Create,
        Destroy,
        Increment,
        Decrement,
        Assign,
        Add,
        Subtract,
        Multiply,
        Divide,
        AddRight,
        SubtractRight,
        MultiplyRight,
        DivideRight,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual,
        Equal,
        NotEqual,
        ToBoolean,
        Call
    };

    virtual ~_Type() {}

    virtual std::string getName() const = 0;

    virtual Value generateUnaryOperatorCall(
        TypeBuilder &typeBuilder, Operator operator_, Value instance ) const
            { return nullptr; }

    virtual Value generateBinaryOperatorCall(
        TypeBuilder &typeBuilder,
        Operator operator_, Value instance, Value argument ) const
            { return nullptr; }

    virtual Value generateGeneralOperatorCall(
        TypeBuilder &typeBuilder, Operator operator_,
        Value instance, const std::vector<Value> &arguments ) const
            { return nullptr; }

    virtual Value generateMemberAccess(
        TypeBuilder &typeBuilder,
        Value instance, const std::string &member ) const
            { return nullptr; }

    Value createValue( llvm::Value *value ) const {
        return _Value::create( value, shared_from_this() );
    }

    llvm::Type* getRaw() const {
        return _rawType;
    }

protected:
    Value generateDefaultAssignmentOperatorCall(
        TypeBuilder &typeBuilder, Value instance, Value argument ) const;

    llvm::Type *_rawType;
};

}
