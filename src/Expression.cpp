#include "Expression.h"

#include <map>
#include <sstream>

#include "Exception.h"

namespace _8b {
namespace ast{

ExpressionPointer Expression::parse( LexicalAnalyser &lexicalAnalyser, int rightBindingPower ) {

    ExpressionPointer expression = Expression::nullDenotation( lexicalAnalyser );

    while( rightBindingPower < getLeftBindingPower(lexicalAnalyser.getCurrentToken().getType()) ) {

        expression = Expression::leftDenotation( lexicalAnalyser, expression );
    }

    return expression;
}

ExpressionPointer Expression::nullDenotation( LexicalAnalyser &lexicalAnalyser ) {

    const Token::Type tokenType = lexicalAnalyser.getCurrentToken().getType();

    if( tokenType == Token::Keyword_Instance )
        return std::make_shared<InstanceExpression>( lexicalAnalyser );

    if( tokenType == Token::Identifier )
        return std::make_shared<IdentifierExpression>( lexicalAnalyser );

    if( tokenType == Token::Constant_Integer )
        return std::make_shared<IntegerConstantExpression>( lexicalAnalyser );

    if( tokenType == Token::Punctuator_OpeningParenthesis ) {
        lexicalAnalyser.extractToken( Token::Punctuator_OpeningParenthesis );
        ExpressionPointer expression = Expression::parse( lexicalAnalyser );
        lexicalAnalyser.extractToken( Token::Punctuator_ClosingParenthesis );
        return expression;
    }

    if( tokenType == Token::Keyword_True || tokenType == Token::Keyword_False )
        return std::make_shared<BooleanConstantExpression>( lexicalAnalyser );

    throwRuntimeError( "Unexpected token" );
}

ExpressionPointer Expression::leftDenotation( LexicalAnalyser &lexicalAnalyser, ExpressionPointer expression ) {

    struct UnaryOperatorParsingRule {
        Token::Type tokenType;
        UnaryOperation operation;
    };

    struct BinaryOperatorParsingRule {
        Token::Type tokenType;
        BinaryOperation operation;
    };

    static const UnaryOperatorParsingRule unaryOperatorParsingRules[] = {
        {Token::Operator_Increment, UnaryOperation::Increment},
        {Token::Operator_Decrement, UnaryOperation::Decrement}
    };

    static const BinaryOperatorParsingRule binaryOperatorParsingRules[] = {
        {Token::Operator_Assign,   BinaryOperation::Assignment},
        {Token::Operator_Plus,     BinaryOperation::Addition},
        {Token::Operator_Minus,    BinaryOperation::Subtraction},
        {Token::Operator_Multiply, BinaryOperation::Multiplication},
        {Token::Operator_Divide,   BinaryOperation::Division},
        {Token::Keyword_And,       BinaryOperation::LogicAnd},
        {Token::Keyword_Or,        BinaryOperation::LogicOr},
        {Token::Operator_Less,     BinaryOperation::LessComparison},
        {Token::Operator_Greater,  BinaryOperation::GreaterComparison}
    };

    const Token::Type tokenType = lexicalAnalyser.getCurrentToken().getType();

    for( auto &rule : binaryOperatorParsingRules ) {
        if( tokenType == rule.tokenType ) {
            lexicalAnalyser.extractToken();
            ExpressionPointer leftOperand = expression;
            ExpressionPointer rightOperand = Expression::parse( lexicalAnalyser, getLeftBindingPower(tokenType) );
            return std::make_shared<BinaryOperationExpression>( rule.operation, leftOperand, rightOperand );
        }
    }

    for( auto &rule : unaryOperatorParsingRules ) {
        if( tokenType == rule.tokenType ) {
            lexicalAnalyser.extractToken();
            return std::make_shared<UnaryOperationExpression>( rule.operation, expression );
        }
    }

    if( tokenType == Token::Punctuator_OpeningParenthesis )
        return std::make_shared<CallExpression>( lexicalAnalyser, expression );

    if( tokenType == Token::Punctuator_Dot )
        return std::make_shared<MemberAccessExpression>( lexicalAnalyser, expression );

    return expression;
}

InstanceExpression::InstanceExpression( LexicalAnalyser &lexicalAnalyser ) {
    lexicalAnalyser.extractToken( Token::Keyword_Instance );
}

IdentifierExpression::IdentifierExpression( LexicalAnalyser &lexicalAnalyser )
    : identifier( lexicalAnalyser.extractToken(Token::Identifier).getLexem() )
{
}

MemberAccessExpression::MemberAccessExpression( LexicalAnalyser &lexicalAnalyser, ExpressionPointer operand )
    : operand( operand )
{
    lexicalAnalyser.extractToken( Token::Punctuator_Dot );
    memberIdentifier = lexicalAnalyser.extractToken( Token::Identifier ).getLexem();
}

IntegerConstantExpression::IntegerConstantExpression( LexicalAnalyser &lexicalAnalyser ) {
    std::stringstream stream;
    stream << lexicalAnalyser.extractToken( Token::Constant_Integer ).getLexem();
    stream >> value;
}

BooleanConstantExpression::BooleanConstantExpression( LexicalAnalyser &lexicalAnalyser ) {

    if( lexicalAnalyser.getCurrentToken().getType() == Token::Keyword_True )
        value = true;
    else if( lexicalAnalyser.getCurrentToken().getType() == Token::Keyword_False )
        value = false;
    else
        throwRuntimeError( "Unexpected token" );

    lexicalAnalyser.extractToken();
}

BinaryOperationExpression::BinaryOperationExpression(
    BinaryOperation operation,
    ExpressionPointer leftOperand,
    ExpressionPointer rightOperand )
      : operation( operation ),
        leftOperand( leftOperand ),
        rightOperand( rightOperand )
{
}

UnaryOperationExpression::UnaryOperationExpression( UnaryOperation operation, ExpressionPointer operand )
    : operation( operation ), operand( operand )
{
}

CallExpression::CallExpression( LexicalAnalyser &lexicalAnalyser, ExpressionPointer callee )
    : callee( callee )
{
    lexicalAnalyser.extractToken( Token::Punctuator_OpeningParenthesis );

    while( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_ClosingParenthesis ) {

        if( !arguments.empty() )
            lexicalAnalyser.extractToken( Token::Punctuator_Comma );

        arguments.push_back( Expression::parse(lexicalAnalyser) );
    }

    lexicalAnalyser.extractToken( Token::Punctuator_ClosingParenthesis );
}


int getLeftBindingPower( Token::Type tokenType ) {

    if( tokenType == Token::Operator_Assign )
        return 5;

    if( tokenType == Token::Keyword_And )
        return 10;

    if( tokenType == Token::Keyword_Or )
        return 10;

    if( tokenType == Token::Operator_Less )
        return 15;

    if( tokenType == Token::Operator_Greater )
        return 15;

    if( tokenType == Token::Operator_Plus )
        return 20;

    if( tokenType == Token::Operator_Minus )
        return 20;

    if( tokenType == Token::Operator_Multiply )
        return 30;

    if( tokenType == Token::Operator_Divide )
        return 30;

    if( tokenType == Token::Operator_Increment )
        return 40;

    if( tokenType == Token::Operator_Decrement )
        return 40;

    if( tokenType == Token::Punctuator_OpeningParenthesis )
        return 50;

    if( tokenType == Token::Punctuator_Dot )
        return 60;

    return 0;
}

}
}
