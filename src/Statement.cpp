#include "Statement.h"

namespace _8b {
namespace ast{


StatementPointer Statement::parse( LexicalAnalyser &lexicalAnalyser ) {
    
    const TokenType tokenType = lexicalAnalyser.getCurrentToken().type;

    if( tokenType == TokenType::PunctuatorOpeningBrace )
        return std::make_shared<BlockStatement>( lexicalAnalyser );
    if( tokenType == TokenType::KeywordIf )
        return std::make_shared<IfStatement>( lexicalAnalyser );
    if( tokenType == TokenType::KeywordReturn )
        return std::make_shared<ReturnStatement>( lexicalAnalyser );
    if( tokenType == TokenType::KeywordVariable )
        return std::make_shared<VariableDeclarationStatement>( lexicalAnalyser );
    if( tokenType == TokenType::KeywordWhile )
        return std::make_shared<WhileStatement>( lexicalAnalyser );

    return std::make_shared<ExpressionStatement>( lexicalAnalyser );
}


BlockStatement::BlockStatement( LexicalAnalyser &lexicalAnalyser ) {

    lexicalAnalyser.extractToken( TokenType::PunctuatorOpeningBrace );

    while( lexicalAnalyser.getCurrentToken().type != TokenType::PunctuatorClosingBrace ) {
        statements.push_back( Statement::parse(lexicalAnalyser) );
    }

    lexicalAnalyser.extractToken( TokenType::PunctuatorClosingBrace);
}


ExpressionStatement::ExpressionStatement( LexicalAnalyser &lexicalAnalyser )
    : expression( Expression::parse(lexicalAnalyser) )
{
    lexicalAnalyser.extractToken( TokenType::PunctuatorSemicolon );
}


IfStatement::IfStatement( LexicalAnalyser &lexicalAnalyser ) {

    lexicalAnalyser.extractToken( TokenType::KeywordIf );
    lexicalAnalyser.extractToken( TokenType::PunctuatorOpeningParenthesis );
    conditionExpression = Expression::parse( lexicalAnalyser );
    lexicalAnalyser.extractToken( TokenType::PunctuatorClosingParenthesis );
    trueBlockStatement = BlockStatement( lexicalAnalyser );

    if( lexicalAnalyser.getCurrentToken().type == TokenType::KeywordElse ) {

        lexicalAnalyser.extractToken();
        falseBlockStatement = BlockStatement( lexicalAnalyser );
    }
}


ReturnStatement::ReturnStatement( LexicalAnalyser &lexicalAnalyser ) {
    
    lexicalAnalyser.extractToken( TokenType::KeywordReturn );

    if( lexicalAnalyser.getCurrentToken().type != TokenType::PunctuatorSemicolon )
        expression = Expression::parse( lexicalAnalyser );

    lexicalAnalyser.extractToken( TokenType::PunctuatorSemicolon );
}


VariableDeclarationStatement::VariableDeclarationStatement( LexicalAnalyser &lexicalAnalyser ) {

    lexicalAnalyser.extractToken( TokenType::KeywordVariable );

    identifier = lexicalAnalyser.extractToken( TokenType::Identifier ).lexem;

    if( lexicalAnalyser.getCurrentToken().type == TokenType::OperatorAssign ) {
        lexicalAnalyser.extractToken( TokenType::OperatorAssign );
        initializerExpression = Expression::parse( lexicalAnalyser );
    } else {
        type = Type::parse( lexicalAnalyser );
    }

    lexicalAnalyser.extractToken( TokenType::PunctuatorSemicolon );
}


WhileStatement::WhileStatement( LexicalAnalyser &lexicalAnalyser ) {

    lexicalAnalyser.extractToken( TokenType::KeywordWhile );
    lexicalAnalyser.extractToken( TokenType::PunctuatorOpeningParenthesis );
    conditionExpression = Expression::parse( lexicalAnalyser );
    lexicalAnalyser.extractToken( TokenType::PunctuatorClosingParenthesis );
    blockStatement = BlockStatement( lexicalAnalyser );
}


}
}
