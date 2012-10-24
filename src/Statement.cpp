#include "Statement.h"

#include "CheckToken.h"

namespace _8b {
namespace ast{


StatementPointer Statement::parse( LexicalAnalyser &lexicalAnalyser ) {
    
    const Token::Type tokenType = lexicalAnalyser.getCurrentToken().getType();

    if( tokenType == Token::Punctuator_OpeningBrace )
        return std::make_shared<BlockStatement>( lexicalAnalyser );
    if( tokenType == Token::Keyword_If )
        return std::make_shared<IfStatement>( lexicalAnalyser );
    if( tokenType == Token::Keyword_Return )
        return std::make_shared<ReturnStatement>( lexicalAnalyser );
    if( tokenType == Token::Keyword_Variable )
        return std::make_shared<VariableDeclarationStatement>( lexicalAnalyser );
    if( tokenType == Token::Keyword_While )
        return std::make_shared<WhileStatement>( lexicalAnalyser );

    return std::make_shared<ExpressionStatement>( lexicalAnalyser );
}


BlockStatement::BlockStatement( LexicalAnalyser &lexicalAnalyser ) {

    checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_OpeningBrace );

    while( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_ClosingBrace ) {
        _statements.push_back( Statement::parse(lexicalAnalyser) );
    }

    checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_ClosingBrace );
}

const std::vector<StatementPointer>& BlockStatement::getStatements() const {
    return _statements;
}


ExpressionStatement::ExpressionStatement( LexicalAnalyser &lexicalAnalyser ) {
    _expression = Expression::parse( lexicalAnalyser );
    checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_Semicolon );
}

ExpressionPointer ExpressionStatement::getExpression() const {
    return _expression;
}


IfStatement::IfStatement( LexicalAnalyser &lexicalAnalyser ) {

    checkToken( lexicalAnalyser.extractToken(), Token::Keyword_If );
    checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_OpeningParenthesis );
    _conditionExpression = Expression::parse( lexicalAnalyser );
    checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_ClosingParenthesis );
    _trueBlockStatement = BlockStatement( lexicalAnalyser );

    if( lexicalAnalyser.getCurrentToken().getType() == Token::Keyword_Else ) {

        lexicalAnalyser.extractToken();
        _falseBlockStatement = BlockStatement( lexicalAnalyser );
    }
}

ExpressionPointer IfStatement::getConditionExpression() const {
    return _conditionExpression;
}

const BlockStatement& IfStatement::getTrueBlockStatement() const {
    return _trueBlockStatement;
}

const BlockStatement& IfStatement::getFalseBlockStatement() const {
    return _falseBlockStatement;
}


ReturnStatement::ReturnStatement( LexicalAnalyser &lexicalAnalyser ) {
    
    checkToken( lexicalAnalyser.extractToken(), Token::Keyword_Return );

    if( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_Semicolon )
        _expression = Expression::parse( lexicalAnalyser );

    checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_Semicolon );
}

ExpressionPointer ReturnStatement::getExpression() const {
    return _expression;
}


VariableDeclarationStatement::VariableDeclarationStatement( LexicalAnalyser &lexicalAnalyser ) {

    checkToken( lexicalAnalyser.extractToken(), Token::Keyword_Variable );

    Token token = lexicalAnalyser.extractToken();
    checkToken( token, Token::Identifier );
    _identifier = token.getLexem();

    if( lexicalAnalyser.getCurrentToken().getType() == Token::Operator_Assign ) {
        checkToken( lexicalAnalyser.extractToken(), Token::Operator_Assign );
        _initializerExpression = Expression::parse( lexicalAnalyser );
    } else {
        _type = Type::parse( lexicalAnalyser );
    }

    checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_Semicolon );
}

const std::string& VariableDeclarationStatement::getIdentifier() const {
    return _identifier;
}

TypePointer VariableDeclarationStatement::getType() const {
    return _type;
}

ExpressionPointer VariableDeclarationStatement::getInitializerExpression() const {
    return _initializerExpression;
}


WhileStatement::WhileStatement( LexicalAnalyser &lexicalAnalyser ) {

    checkToken( lexicalAnalyser.extractToken(), Token::Keyword_While );
    checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_OpeningParenthesis );
    _conditionExpression = Expression::parse( lexicalAnalyser );
    checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_ClosingParenthesis );
    _blockStatement = BlockStatement( lexicalAnalyser );
}

ExpressionPointer WhileStatement::getConditionExpression() const {
    return _conditionExpression;
}

const BlockStatement& WhileStatement::getBlockStatement() const {
    return _blockStatement;
}


}
}
