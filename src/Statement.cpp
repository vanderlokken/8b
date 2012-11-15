#include "Statement.h"

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

    lexicalAnalyser.extractToken( Token::Punctuator_OpeningBrace );

    while( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_ClosingBrace ) {
        statements.push_back( Statement::parse(lexicalAnalyser) );
    }

    lexicalAnalyser.extractToken( Token::Punctuator_ClosingBrace);
}


ExpressionStatement::ExpressionStatement( LexicalAnalyser &lexicalAnalyser )
    : expression( Expression::parse(lexicalAnalyser) )
{
    lexicalAnalyser.extractToken( Token::Punctuator_Semicolon );
}


IfStatement::IfStatement( LexicalAnalyser &lexicalAnalyser ) {

    lexicalAnalyser.extractToken( Token::Keyword_If );
    lexicalAnalyser.extractToken( Token::Punctuator_OpeningParenthesis );
    conditionExpression = Expression::parse( lexicalAnalyser );
    lexicalAnalyser.extractToken( Token::Punctuator_ClosingParenthesis );
    trueBlockStatement = BlockStatement( lexicalAnalyser );

    if( lexicalAnalyser.getCurrentToken().getType() == Token::Keyword_Else ) {

        lexicalAnalyser.extractToken();
        falseBlockStatement = BlockStatement( lexicalAnalyser );
    }
}


ReturnStatement::ReturnStatement( LexicalAnalyser &lexicalAnalyser ) {
    
    lexicalAnalyser.extractToken( Token::Keyword_Return );

    if( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_Semicolon )
        expression = Expression::parse( lexicalAnalyser );

    lexicalAnalyser.extractToken( Token::Punctuator_Semicolon );
}


VariableDeclarationStatement::VariableDeclarationStatement( LexicalAnalyser &lexicalAnalyser ) {

    lexicalAnalyser.extractToken( Token::Keyword_Variable );

    identifier = lexicalAnalyser.extractToken( Token::Identifier ).getLexem();

    if( lexicalAnalyser.getCurrentToken().getType() == Token::Operator_Assign ) {
        lexicalAnalyser.extractToken( Token::Operator_Assign );
        initializerExpression = Expression::parse( lexicalAnalyser );
    } else {
        type = Type::parse( lexicalAnalyser );
    }

    lexicalAnalyser.extractToken( Token::Punctuator_Semicolon );
}


WhileStatement::WhileStatement( LexicalAnalyser &lexicalAnalyser ) {

    lexicalAnalyser.extractToken( Token::Keyword_While );
    lexicalAnalyser.extractToken( Token::Punctuator_OpeningParenthesis );
    conditionExpression = Expression::parse( lexicalAnalyser );
    lexicalAnalyser.extractToken( Token::Punctuator_ClosingParenthesis );
    blockStatement = BlockStatement( lexicalAnalyser );
}


}
}
