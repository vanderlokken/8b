#pragma once

#include <istream>
#include <string>

#include "Token.h"

namespace _8b {

class LexicalAnalyser {
public:
    LexicalAnalyser( std::istream &stream );

    const Token& getCurrentToken() const;
    const Token& getNextToken() const;

    Token extractToken();

private:
    Token readToken();

    std::string _source;
    std::string::const_iterator _first;
    std::string::const_iterator _last;

    SourceLocation _sourceLocation;

    Token _currentToken;
    Token _nextToken;
};

}
