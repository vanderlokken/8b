#include "Expression.h"

#include <map>
#include <sstream>

#include "Exception.h"

namespace _8b {
namespace ast{
    
Expression::Expression( size_t typeId )
    : _typeId ( typeId ) {}

size_t Expression::getTypeId() const {
    return _typeId;
}

ExpressionPointer Expression::parse( LexicalAnalyser &lexicalAnalyser, int rightBindingPower ) {

    ExpressionPointer expression = Expression::nullDenotation( lexicalAnalyser );

    while( rightBindingPower < getLeftBindingPower(lexicalAnalyser.getCurrentToken().getType()) ) {

        expression = Expression::leftDenotation( lexicalAnalyser, expression );
    }

    return expression;
}

ExpressionPointer Expression::nullDenotation( LexicalAnalyser &lexicalAnalyser ) {

    const Token &token = lexicalAnalyser.getCurrentToken();

    if( token.getType() == Token::Identifier )
        return std::make_shared<IdentifierExpression>( lexicalAnalyser );

    if( token.getType() == Token::Constant_Integer )
        return std::make_shared<IntegerConstantExpression>( lexicalAnalyser );

    if( token.getType() == Token::Punctuator_OpeningParenthesis ) {
        checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_OpeningParenthesis );
        ExpressionPointer expression = Expression::parse( lexicalAnalyser );
        checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_ClosingParenthesis );
        return expression;
    }

    throwRuntimeError( "Unexpected token" );
}

ExpressionPointer Expression::leftDenotation( LexicalAnalyser &lexicalAnalyser, ExpressionPointer expression ) {

    const Token::Type tokenType = lexicalAnalyser.getCurrentToken().getType();

    if( tokenType == Token::Operator_Assign )
        return std::make_shared<AssignmentExpression>( lexicalAnalyser, expression );
    if( tokenType == Token::Operator_Plus )
        return std::make_shared<AdditionExpression>( lexicalAnalyser, expression );
    if( tokenType == Token::Operator_Minus )
        return std::make_shared<SubtractionExpression>( lexicalAnalyser, expression );
    if( tokenType == Token::Operator_Multiply )
        return std::make_shared<MultiplicationExpression>( lexicalAnalyser, expression );
    if( tokenType == Token::Operator_Divide )
        return std::make_shared<DivisionExpression>( lexicalAnalyser, expression );
    if( tokenType == Token::Keyword_And )
        return std::make_shared<LogicAndExpression>( lexicalAnalyser, expression );
    if( tokenType == Token::Keyword_Or )
        return std::make_shared<LogicOrExpression>( lexicalAnalyser, expression );
    if( tokenType == Token::Operator_Less )
        return std::make_shared<LessExpression>( lexicalAnalyser, expression );
    if( tokenType == Token::Operator_Greater )
        return std::make_shared<GreaterExpression>( lexicalAnalyser, expression );

    if( tokenType == Token::Operator_Increment )
        return std::make_shared<IncrementExpression>( lexicalAnalyser, expression );
    if( tokenType == Token::Operator_Decrement )
        return std::make_shared<DecrementExpression>( lexicalAnalyser, expression );

    if( tokenType == Token::Punctuator_OpeningParenthesis )
        return std::make_shared<CallExpression>( lexicalAnalyser, expression );

    return expression;
}

IdentifierExpression::IdentifierExpression( LexicalAnalyser &lexicalAnalyser ) {

    Token token = lexicalAnalyser.extractToken();
    checkToken( token, Token::Identifier );

    _identifier = token.getLexem();
}

const std::string& IdentifierExpression::getIdentifier() const {
    return _identifier;
}

IntegerConstantExpression::IntegerConstantExpression( LexicalAnalyser &lexicalAnalyser ) {

    Token token = lexicalAnalyser.extractToken();
    checkToken( token, Token::Constant_Integer );

    std::stringstream stream;
    stream << token.getLexem();
    stream >> _value;
}

int IntegerConstantExpression::getValue() const {
    return _value;
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
    
    return 0;
}

CallExpression::CallExpression( LexicalAnalyser &lexicalAnalyser, ExpressionPointer callee )
    : _callee( callee )
{
    checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_OpeningParenthesis );

    while( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_ClosingParenthesis ) {

        if( !_arguments.empty() )
            checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_Comma );

        _arguments.push_back( Expression::parse(lexicalAnalyser) );
    }

    checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_ClosingParenthesis );
}

ExpressionPointer CallExpression::getCallee() const {
    return _callee;
}

const std::vector<ExpressionPointer>& CallExpression::getArguments() const {
    return _arguments;
}

}
}
