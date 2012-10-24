#include "Type.h"

#include "CheckToken.h"
#include "Exception.h"

namespace _8b {
namespace ast{

TypePointer Type::parse( LexicalAnalyser &lexicalAnalyser ) {

    const Token::Type tokenType = lexicalAnalyser.getCurrentToken().getType();
    
    if( tokenType == Token::Keyword_Integer )
        return std::make_shared<IntegerType>( lexicalAnalyser );
    else if( tokenType == Token::Keyword_Boolean )
        return std::make_shared<BooleanType>( lexicalAnalyser );
    
    throwRuntimeError( "Unexpected token or not supported" );
}

IntegerType::IntegerType( LexicalAnalyser &lexicalAnalyser ) {
    checkToken( lexicalAnalyser.extractToken(), Token::Keyword_Integer );
}

BooleanType::BooleanType( LexicalAnalyser &lexicalAnalyser ) {
    checkToken( lexicalAnalyser.extractToken(), Token::Keyword_Boolean );
}

}
}
