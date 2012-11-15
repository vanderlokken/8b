#include "Expression.h"

#include <map>
#include <sstream>

#include "Exception.h"

namespace _8b {
namespace ast{

ExpressionPointer Expression::parse( LexicalAnalyser &lexicalAnalyser, int rightBindingPower ) {

    ExpressionPointer expression = Expression::nullDenotation( lexicalAnalyser );

    while( rightBindingPower < getLeftBindingPower(lexicalAnalyser.getCurrentToken().type) ) {

        expression = Expression::leftDenotation( lexicalAnalyser, expression );
    }

    return expression;
}

ExpressionPointer Expression::nullDenotation( LexicalAnalyser &lexicalAnalyser ) {

    const TokenType tokenType = lexicalAnalyser.getCurrentToken().type;

    if( tokenType == TokenType::KeywordInstance )
        return std::make_shared<InstanceExpression>( lexicalAnalyser );

    if( tokenType == TokenType::Identifier )
        return std::make_shared<IdentifierExpression>( lexicalAnalyser );

    if( tokenType == TokenType::ConstantInteger )
        return std::make_shared<IntegerConstantExpression>( lexicalAnalyser );

    if( tokenType == TokenType::PunctuatorOpeningParenthesis ) {
        lexicalAnalyser.extractToken( TokenType::PunctuatorOpeningParenthesis );
        ExpressionPointer expression = Expression::parse( lexicalAnalyser );
        lexicalAnalyser.extractToken( TokenType::PunctuatorClosingParenthesis );
        return expression;
    }

    if( tokenType == TokenType::KeywordTrue || tokenType == TokenType::KeywordFalse )
        return std::make_shared<BooleanConstantExpression>( lexicalAnalyser );

    throwRuntimeError( "Unexpected token" );
}

ExpressionPointer Expression::leftDenotation( LexicalAnalyser &lexicalAnalyser, ExpressionPointer expression ) {

    struct UnaryOperatorParsingRule {
        TokenType tokenType;
        UnaryOperation operation;
    };

    struct BinaryOperatorParsingRule {
        TokenType tokenType;
        BinaryOperation operation;
    };

    static const UnaryOperatorParsingRule unaryOperatorParsingRules[] = {
        {TokenType::OperatorIncrement, UnaryOperation::Increment},
        {TokenType::OperatorDecrement, UnaryOperation::Decrement}
    };

    static const BinaryOperatorParsingRule binaryOperatorParsingRules[] = {
        {TokenType::OperatorAssign,   BinaryOperation::Assignment},
        {TokenType::OperatorPlus,     BinaryOperation::Addition},
        {TokenType::OperatorMinus,    BinaryOperation::Subtraction},
        {TokenType::OperatorMultiply, BinaryOperation::Multiplication},
        {TokenType::OperatorDivide,   BinaryOperation::Division},
        {TokenType::KeywordAnd,       BinaryOperation::LogicAnd},
        {TokenType::KeywordOr,        BinaryOperation::LogicOr},
        {TokenType::OperatorLess,     BinaryOperation::LessComparison},
        {TokenType::OperatorGreater,  BinaryOperation::GreaterComparison}
    };

    const TokenType tokenType = lexicalAnalyser.getCurrentToken().type;

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

    if( tokenType == TokenType::PunctuatorOpeningParenthesis )
        return std::make_shared<CallExpression>( lexicalAnalyser, expression );

    if( tokenType == TokenType::PunctuatorDot )
        return std::make_shared<MemberAccessExpression>( lexicalAnalyser, expression );

    return expression;
}

InstanceExpression::InstanceExpression( LexicalAnalyser &lexicalAnalyser ) {
    lexicalAnalyser.extractToken( TokenType::KeywordInstance );
}

IdentifierExpression::IdentifierExpression( LexicalAnalyser &lexicalAnalyser )
    : identifier( lexicalAnalyser.extractToken(TokenType::Identifier).lexem )
{
}

MemberAccessExpression::MemberAccessExpression( LexicalAnalyser &lexicalAnalyser, ExpressionPointer operand )
    : operand( operand )
{
    lexicalAnalyser.extractToken( TokenType::PunctuatorDot );
    memberIdentifier = lexicalAnalyser.extractToken( TokenType::Identifier ).lexem;
}

IntegerConstantExpression::IntegerConstantExpression( LexicalAnalyser &lexicalAnalyser ) {
    std::stringstream stream;
    stream << lexicalAnalyser.extractToken( TokenType::ConstantInteger ).lexem;
    stream >> value;
}

BooleanConstantExpression::BooleanConstantExpression( LexicalAnalyser &lexicalAnalyser ) {

    if( lexicalAnalyser.getCurrentToken().type == TokenType::KeywordTrue )
        value = true;
    else if( lexicalAnalyser.getCurrentToken().type == TokenType::KeywordFalse )
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
    lexicalAnalyser.extractToken( TokenType::PunctuatorOpeningParenthesis );

    while( lexicalAnalyser.getCurrentToken().type != TokenType::PunctuatorClosingParenthesis ) {

        if( !arguments.empty() )
            lexicalAnalyser.extractToken( TokenType::PunctuatorComma );

        arguments.push_back( Expression::parse(lexicalAnalyser) );
    }

    lexicalAnalyser.extractToken( TokenType::PunctuatorClosingParenthesis );
}


int getLeftBindingPower( TokenType tokenType ) {

    if( tokenType == TokenType::OperatorAssign )
        return 5;

    if( tokenType == TokenType::KeywordAnd )
        return 10;

    if( tokenType == TokenType::KeywordOr )
        return 10;

    if( tokenType == TokenType::OperatorLess )
        return 15;

    if( tokenType == TokenType::OperatorGreater )
        return 15;

    if( tokenType == TokenType::OperatorPlus )
        return 20;

    if( tokenType == TokenType::OperatorMinus )
        return 20;

    if( tokenType == TokenType::OperatorMultiply )
        return 30;

    if( tokenType == TokenType::OperatorDivide )
        return 30;

    if( tokenType == TokenType::OperatorIncrement )
        return 40;

    if( tokenType == TokenType::OperatorDecrement )
        return 40;

    if( tokenType == TokenType::PunctuatorOpeningParenthesis )
        return 50;

    if( tokenType == TokenType::PunctuatorDot )
        return 60;

    return 0;
}

}
}
