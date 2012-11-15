#include "Type.h"

#include "Exception.h"

namespace _8b {
namespace ast{

TypePointer Type::parse( LexicalAnalyser &lexicalAnalyser ) {

    const TokenType tokenType = lexicalAnalyser.getCurrentToken().type;
    
    if( tokenType == TokenType::KeywordInteger )
        return std::make_shared<IntegerType>( lexicalAnalyser );
    else if( tokenType == TokenType::KeywordBoolean )
        return std::make_shared<BooleanType>( lexicalAnalyser );
    else if( tokenType == TokenType::Identifier )
        return std::make_shared<NamedType>( lexicalAnalyser );

    throwRuntimeError( "Unexpected token or not supported" );
}

NamedType::NamedType( LexicalAnalyser &lexicalAnalyser )
    : identifier( lexicalAnalyser.extractToken(TokenType::Identifier).lexem )
{
}

IntegerType::IntegerType( LexicalAnalyser &lexicalAnalyser ) {
    lexicalAnalyser.extractToken( TokenType::KeywordInteger );
}

BooleanType::BooleanType( LexicalAnalyser &lexicalAnalyser ) {
    lexicalAnalyser.extractToken( TokenType::KeywordBoolean );
}

}
}
