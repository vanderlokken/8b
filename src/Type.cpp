#include "Type.h"

#include "CheckToken.h"

namespace _8b {
namespace ast{

Type::Type( LexicalAnalyser &lexicalAnalyser ) {
    checkToken( lexicalAnalyser.getCurrentToken().getType(), Token::Keyword_Integer );
    Token token = lexicalAnalyser.extractToken();
    _identifier = token.getLexem();
}

}
}
