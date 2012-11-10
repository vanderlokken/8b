#include "Function.h"

namespace _8b {
namespace ast{

Function::Function( LexicalAnalyser &lexicalAnalyser ) {
    
    // Read "function" keyword
    
    lexicalAnalyser.extractToken( Token::Keyword_Function );
    
    // Read identifier
    
    _identifier = lexicalAnalyser.extractToken( Token::Identifier ).getLexem();
    
    // Read arguments
    
    lexicalAnalyser.extractToken( Token::Punctuator_OpeningParenthesis );

    while( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_ClosingParenthesis ) {

        if( !_arguments.empty() )
            lexicalAnalyser.extractToken( Token::Punctuator_Comma );
        
        Argument argument = {
            lexicalAnalyser.extractToken( Token::Identifier ).getLexem(),
            Type::parse(lexicalAnalyser)
        };
        _arguments.push_back( argument );
    }

    lexicalAnalyser.extractToken( Token::Punctuator_ClosingParenthesis );

    // Read return type

    if( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_OpeningBrace )
        _returnType = Type::parse( lexicalAnalyser );

    // Read statement block

    _blockStatement = BlockStatement( lexicalAnalyser );
    
    return;
}

const std::string& Function::getIdentifier() const noexcept {
    return _identifier;
}

const std::vector<Function::Argument>& Function::getArguments() const noexcept {
    return _arguments;
}

TypePointer Function::getReturnType() const {
    return _returnType;
}

const BlockStatement& Function::getBlockStatement() const {
    return _blockStatement;
}

}
}
