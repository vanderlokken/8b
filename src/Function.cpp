#include "Function.h"

namespace _8b {
namespace ast{

Function::Function( LexicalAnalyser &lexicalAnalyser ) {
    
    // Read "function" keyword
    
    lexicalAnalyser.extractToken( TokenType::KeywordFunction );
    
    // Read identifier
    
    identifier = lexicalAnalyser.extractToken( TokenType::Identifier ).lexem;
    
    // Read arguments
    
    lexicalAnalyser.extractToken( TokenType::PunctuatorOpeningParenthesis );

    while( lexicalAnalyser.getCurrentToken().type != TokenType::PunctuatorClosingParenthesis ) {

        if( !arguments.empty() )
            lexicalAnalyser.extractToken( TokenType::PunctuatorComma );
        
        Argument argument = {
            lexicalAnalyser.extractToken( TokenType::Identifier ).lexem,
            Type::parse(lexicalAnalyser)
        };
        arguments.push_back( argument );
    }

    lexicalAnalyser.extractToken( TokenType::PunctuatorClosingParenthesis );

    // Read return type

    if( lexicalAnalyser.getCurrentToken().type != TokenType::PunctuatorOpeningBrace )
        returnType = Type::parse( lexicalAnalyser );

    // Read statement block

    blockStatement = BlockStatement( lexicalAnalyser );
    
    return;
}

}
}
