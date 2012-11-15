#include "Function.h"

namespace _8b {
namespace ast{

Function::Function( LexicalAnalyser &lexicalAnalyser ) {
    
    // Read "function" keyword
    
    lexicalAnalyser.extractToken( Token::Keyword_Function );
    
    // Read identifier
    
    identifier = lexicalAnalyser.extractToken( Token::Identifier ).getLexem();
    
    // Read arguments
    
    lexicalAnalyser.extractToken( Token::Punctuator_OpeningParenthesis );

    while( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_ClosingParenthesis ) {

        if( !arguments.empty() )
            lexicalAnalyser.extractToken( Token::Punctuator_Comma );
        
        Argument argument = {
            lexicalAnalyser.extractToken( Token::Identifier ).getLexem(),
            Type::parse(lexicalAnalyser)
        };
        arguments.push_back( argument );
    }

    lexicalAnalyser.extractToken( Token::Punctuator_ClosingParenthesis );

    // Read return type

    if( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_OpeningBrace )
        returnType = Type::parse( lexicalAnalyser );

    // Read statement block

    blockStatement = BlockStatement( lexicalAnalyser );
    
    return;
}

}
}
