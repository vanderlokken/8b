#include "Function.h"

#include "CheckToken.h"

namespace _8b {
namespace ast{

Function::Function( LexicalAnalyser &lexicalAnalyser ) {
    
    // Read "function" keyword
    
    checkToken( lexicalAnalyser.extractToken(), Token::Keyword_Function );
    
    // Read identifier
    
    checkToken( lexicalAnalyser.getCurrentToken(), Token::Identifier );
    _identifier = lexicalAnalyser.extractToken().getLexem();
    
    // Read arguments
    
    checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_OpeningParenthesis );

    while( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_ClosingParenthesis ) {

        if( !_arguments.empty() )
            checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_Comma );
        
        Token token = lexicalAnalyser.extractToken();
        checkToken( token, Token::Identifier );

        Argument argument = { token.getLexem(), Type(lexicalAnalyser) };
        _arguments.push_back( argument );
    }

    checkToken( lexicalAnalyser.extractToken(), Token::Punctuator_ClosingParenthesis );

    // Read return type

    if( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_OpeningBrace )
        _returnType = std::make_shared<Type>( lexicalAnalyser );

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
