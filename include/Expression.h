#pragma once

#include <memory>
#include <vector>

#include "ClassId.h"
#include "LexicalAnalyser.h"
#include "Operation.h"

namespace _8b {
namespace ast {

class Expression;
typedef std::shared_ptr<Expression> ExpressionPointer;


class Expression : public BaseIdClass {
public:
    static ExpressionPointer parse( LexicalAnalyser&, int rightBindingPower = 0 );

protected:
    static ExpressionPointer nullDenotation( LexicalAnalyser& );
    static ExpressionPointer leftDenotation( LexicalAnalyser&, ExpressionPointer );
};

template<class T>
class ExpressionType : public DerivedIdClass<Expression, T> {};


class IdentifierExpression : public ExpressionType<IdentifierExpression> {
public:
    IdentifierExpression( LexicalAnalyser& );

    const std::string& getIdentifier() const;

private:
    std::string _identifier;
};


class MemberAccessExpression : public ExpressionType<MemberAccessExpression> {
public:
    MemberAccessExpression( LexicalAnalyser&, ExpressionPointer );

    ExpressionPointer getOperand() const;
    const std::string& getMemberIdentifier() const;

private:
    ExpressionPointer _operand;
    std::string _identifier;
};



class IntegerConstantExpression : public ExpressionType<IntegerConstantExpression> {
public:
    IntegerConstantExpression( LexicalAnalyser& );

    int getValue() const;

private:
    int _value;
};


class BooleanConstantExpression : public ExpressionType<BooleanConstantExpression> {
public:
    BooleanConstantExpression( LexicalAnalyser& );

    bool getValue() const;

private:
    bool _value;
};


class BinaryOperationExpression : public ExpressionType< BinaryOperationExpression > {
public:

    BinaryOperationExpression( BinaryOperation, ExpressionPointer leftOperand, ExpressionPointer rightOperand );

    BinaryOperation getOperation() const;
    ExpressionPointer getLeftOperand() const;
    ExpressionPointer getRightOperand() const;

protected:
    BinaryOperation _operation;
    ExpressionPointer _leftOperand;
    ExpressionPointer _rightOperand;
};


class UnaryOperationExpression : public ExpressionType< UnaryOperationExpression > {
public:

    UnaryOperationExpression( UnaryOperation, ExpressionPointer operand );

    UnaryOperation getOperation() const;
    ExpressionPointer getOperand() const;

protected:
    UnaryOperation _operation;
    ExpressionPointer _operand;
};


class CallExpression : public ExpressionType<CallExpression> {
public:
    CallExpression( LexicalAnalyser&, ExpressionPointer callee );

    ExpressionPointer getCallee() const;
    const std::vector<ExpressionPointer>& getArguments() const;

private:
    ExpressionPointer _callee;
    std::vector<ExpressionPointer> _arguments;
};


int getLeftBindingPower( Token::Type );

}
}
