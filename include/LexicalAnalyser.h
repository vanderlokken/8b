#pragma once

#include <istream>
#include <string>

#include "Token.h"

namespace _8b {

class LexicalAnalyser {
public:
    LexicalAnalyser( std::istream &stream );
    
    Token getCurrentToken() const noexcept;
    Token getNextToken() const noexcept;
    
    Token extractToken();
    Token extractToken( Token::Type );

private:
    Token readToken();
    
    std::string _source;
    std::string::const_iterator _first;
    std::string::const_iterator _last;

    Token _currentToken;
    Token _nextToken;
};

}
