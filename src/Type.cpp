#include "Type.h"

#include "Exception.h"

namespace _8b {
namespace ast{

TypePointer Type::parse( LexicalAnalyser &lexicalAnalyser ) {

    const Token::Type tokenType = lexicalAnalyser.getCurrentToken().getType();
    
    if( tokenType == Token::Keyword_Integer )
        return std::make_shared<IntegerType>( lexicalAnalyser );
    else if( tokenType == Token::Keyword_Boolean )
        return std::make_shared<BooleanType>( lexicalAnalyser );
    else if( tokenType == Token::Identifier )
        return std::make_shared<NamedType>( lexicalAnalyser );

    throwRuntimeError( "Unexpected token or not supported" );
}

NamedType::NamedType( LexicalAnalyser &lexicalAnalyser )
    : _identifier( lexicalAnalyser.extractToken(Token::Identifier).getLexem() )
{
}

const std::string& NamedType::getIdentifier() const {
    return _identifier;
}

IntegerType::IntegerType( LexicalAnalyser &lexicalAnalyser ) {
    lexicalAnalyser.extractToken( Token::Keyword_Integer );
}

BooleanType::BooleanType( LexicalAnalyser &lexicalAnalyser ) {
    lexicalAnalyser.extractToken( Token::Keyword_Boolean );
}

}
}
