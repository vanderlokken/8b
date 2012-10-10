#include "Token.h"

namespace _8b {

Token::Token( Type type, const std::string &lexem )
    : _type( type ), _lexem( lexem ) {}

Token::Type Token::getType() const {
    return _type;
}

const std::string& Token::getLexem() const {
    return _lexem;
}

bool Token::operator == ( const Token &token ) const {
    return
        _type == token._type &&
        _lexem == token._lexem;
}

bool Token::operator != ( const Token &token ) const {
    return
        _type != token._type ||
        _lexem != token._lexem;
}

}
