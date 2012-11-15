#include "Token.h"

namespace _8b {

Token::Token( TokenType type, const std::string &lexem )
    : type( type ), lexem( lexem ) {}

bool Token::operator == ( const Token &token ) const {
    return type == token.type && lexem == token.lexem;
}

bool Token::operator != ( const Token &token ) const {
    return type != token.type || lexem != token.lexem;
}

}
