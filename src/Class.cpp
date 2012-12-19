#include "Class.h"

namespace _8b {
namespace ast{

Class::Class( LexicalAnalyser &lexicalAnalyser ) {

    lexicalAnalyser.extractToken( TokenType::KeywordClass );

    identifier = lexicalAnalyser.extractToken( TokenType::Identifier ).lexem;

    lexicalAnalyser.extractToken( TokenType::PunctuatorOpeningBrace );

    while( lexicalAnalyser.getCurrentToken().type != TokenType::PunctuatorClosingBrace ) {
        if( lexicalAnalyser.getCurrentToken().type == TokenType::KeywordVariable )
            variableDeclarations.push_back( VariableDeclarationStatement(lexicalAnalyser) );
        else if( lexicalAnalyser.getCurrentToken().type == TokenType::KeywordFunction )
            methods.push_back( Function(lexicalAnalyser) );
    }

    lexicalAnalyser.extractToken( TokenType::PunctuatorClosingBrace );
}

}
}
