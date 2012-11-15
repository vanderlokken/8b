#include "Class.h"

namespace _8b {
namespace ast{

Class::Class( LexicalAnalyser &lexicalAnalyser ) {

    lexicalAnalyser.extractToken( Token::Keyword_Class );

    identifier = lexicalAnalyser.extractToken( Token::Identifier ).getLexem();

    lexicalAnalyser.extractToken( Token::Punctuator_OpeningBrace );

    while( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_ClosingBrace ) {
        if( lexicalAnalyser.getCurrentToken().getType() == Token::Keyword_Variable )
            variables.push_back( VariableDeclarationStatement(lexicalAnalyser) );
        else if( lexicalAnalyser.getCurrentToken().getType() == Token::Keyword_Function )
            methods.push_back( Function(lexicalAnalyser) );
    }

    lexicalAnalyser.extractToken( Token::Punctuator_ClosingBrace );
}

}
}
