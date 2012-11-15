#pragma once

#include <memory>
#include <vector>

#include "ClassId.h"
#include "LexicalAnalyser.h"
#include "Operation.h"

namespace _8b {
namespace ast {

struct Expression;
typedef std::shared_ptr<const Expression> ExpressionPointer;


struct Expression : public BaseIdClass {
    static ExpressionPointer parse( LexicalAnalyser&, int rightBindingPower = 0 );

protected:
    static ExpressionPointer nullDenotation( LexicalAnalyser& );
    static ExpressionPointer leftDenotation( LexicalAnalyser&, ExpressionPointer );
};

template<class T>
struct ExpressionType : public DerivedIdClass<Expression, T> {};


struct InstanceExpression : public ExpressionType<InstanceExpression> {
    InstanceExpression( LexicalAnalyser& );
};


struct IdentifierExpression : public ExpressionType<IdentifierExpression> {
    
    IdentifierExpression( LexicalAnalyser& );

    std::string identifier;
};


struct MemberAccessExpression : public ExpressionType<MemberAccessExpression> {
    
    MemberAccessExpression( LexicalAnalyser&, ExpressionPointer );

    ExpressionPointer operand;
    std::string memberIdentifier;
};


struct IntegerConstantExpression : public ExpressionType<IntegerConstantExpression> {
    
    IntegerConstantExpression( LexicalAnalyser& );

    int value;
};


struct BooleanConstantExpression : public ExpressionType<BooleanConstantExpression> {
    
    BooleanConstantExpression( LexicalAnalyser& );

    bool value;
};


struct BinaryOperationExpression : public ExpressionType< BinaryOperationExpression > {
    
    BinaryOperationExpression( BinaryOperation, ExpressionPointer leftOperand, ExpressionPointer rightOperand );

    BinaryOperation operation;
    ExpressionPointer leftOperand;
    ExpressionPointer rightOperand;
};


struct UnaryOperationExpression : public ExpressionType< UnaryOperationExpression > {
    
    UnaryOperationExpression( UnaryOperation, ExpressionPointer operand );

    UnaryOperation operation;
    ExpressionPointer operand;
};


struct CallExpression : public ExpressionType<CallExpression> {
    
    CallExpression( LexicalAnalyser&, ExpressionPointer callee );

    ExpressionPointer callee;
    std::vector<ExpressionPointer> arguments;
};


int getLeftBindingPower( Token::Type );

}
}
