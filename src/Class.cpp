#include "Class.h"

namespace _8b {
namespace ast{

Class::Class( LexicalAnalyser &lexicalAnalyser ) {

    lexicalAnalyser.extractToken( Token::Keyword_Class );

    _identifier = lexicalAnalyser.extractToken( Token::Identifier ).getLexem();

    lexicalAnalyser.extractToken( Token::Punctuator_OpeningBrace );

    while( lexicalAnalyser.getCurrentToken().getType() != Token::Punctuator_ClosingBrace ) {
        if( lexicalAnalyser.getCurrentToken().getType() == Token::Keyword_Variable )
            _variables.push_back( VariableDeclarationStatement(lexicalAnalyser) );
        else if( lexicalAnalyser.getCurrentToken().getType() == Token::Keyword_Function )
            _methods.push_back( Function(lexicalAnalyser) );
    }

    lexicalAnalyser.extractToken( Token::Punctuator_ClosingBrace );
}

const std::string& Class::getIdentifier() const {
    return _identifier;
}

const std::vector<Function>& Class::getMethods() const {
    return _methods;
}

const std::vector<VariableDeclarationStatement>& Class::getVariables() const {
    return _variables;
}

}
}
