#pragma once

#include <memory>
#include <vector>

#include "CheckToken.h"
#include "LexicalAnalyser.h"

namespace _8b {
namespace ast {

class Expression;
typedef std::shared_ptr<Expression> ExpressionPointer;


class Expression {
public:

    Expression( size_t typeId );

    size_t getTypeId() const;

    static ExpressionPointer parse( LexicalAnalyser&, int rightBindingPower = 0 );

protected:
    static ExpressionPointer nullDenotation( LexicalAnalyser& );
    static ExpressionPointer leftDenotation( LexicalAnalyser&, ExpressionPointer );

    size_t _typeId;
};


template< class T >
class ExpressionType : public Expression {
public:
    ExpressionType() : Expression( typeId ) {}
    static const size_t typeId;
};

template< class T >
const size_t ExpressionType<T>::typeId = (size_t)( typeid(T).raw_name() );


class IdentifierExpression : public ExpressionType<IdentifierExpression> {
public:
    IdentifierExpression( LexicalAnalyser& );

    const std::string& getIdentifier() const;

private:
    std::string _identifier;
};


class IntegerConstantExpression : public ExpressionType<IntegerConstantExpression> {
public:
    IntegerConstantExpression( LexicalAnalyser& );

    int getValue() const;

private:
    int _value;
};


int getLeftBindingPower( Token::Type );


template<Token::Type tokenType>
class BinaryOperatorExpression : public ExpressionType< BinaryOperatorExpression<tokenType> > {
public:

    BinaryOperatorExpression( LexicalAnalyser &lexicalAnalyser, ExpressionPointer leftOperand )
        : _leftOperand( leftOperand )
    {
        checkToken( lexicalAnalyser.extractToken(), tokenType );
        _rightOperand = Expression::parse( lexicalAnalyser, getLeftBindingPower(tokenType) );
    }

    ExpressionPointer getLeftOperand() const {
        return _leftOperand;
    }

    ExpressionPointer getRightOperand() const {
        return _rightOperand;
    }

protected:
    ExpressionPointer _leftOperand;
    ExpressionPointer _rightOperand;
};


typedef BinaryOperatorExpression< Token::Operator_Assign > AssignmentExpression;
typedef BinaryOperatorExpression< Token::Keyword_And > LogicAndExpression;
typedef BinaryOperatorExpression< Token::Keyword_Or > LogicOrExpression;
typedef BinaryOperatorExpression< Token::Operator_Plus > AdditionExpression;
typedef BinaryOperatorExpression< Token::Operator_Minus > SubtractionExpression;
typedef BinaryOperatorExpression< Token::Operator_Multiply > MultiplicationExpression;
typedef BinaryOperatorExpression< Token::Operator_Divide > DivisionExpression;
typedef BinaryOperatorExpression< Token::Operator_Less > LessExpression;
typedef BinaryOperatorExpression< Token::Operator_Greater > GreaterExpression;


template<Token::Type tokenType>
class PostfixUnaryOperatorExpression : public ExpressionType< PostfixUnaryOperatorExpression<tokenType> > {
public:

    PostfixUnaryOperatorExpression( LexicalAnalyser &lexicalAnalyser, ExpressionPointer operand )
        : _operand( operand )
    {
        _operand = operand;
        checkToken( lexicalAnalyser.extractToken(), tokenType );
    }

    ExpressionPointer getOperand() const {
        return _operand;
    }

protected:
    ExpressionPointer _operand;
};


typedef PostfixUnaryOperatorExpression< Token::Operator_Increment > IncrementExpression;
typedef PostfixUnaryOperatorExpression< Token::Operator_Decrement > DecrementExpression;


class CallExpression : public ExpressionType<CallExpression> {
public:
    CallExpression( LexicalAnalyser&, ExpressionPointer callee );

    ExpressionPointer getCallee() const;
    const std::vector<ExpressionPointer>& getArguments() const;

private:
    ExpressionPointer _callee;
    std::vector<ExpressionPointer> _arguments;
};


}
}
