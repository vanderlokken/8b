#pragma once

#include <istream>

#include "Token.h"

namespace _8b {

class LexicalAnalyser {
public:
    LexicalAnalyser( std::istream &stream );
    
    Token getCurrentToken() const noexcept;
    Token getNextToken() const noexcept;
    
    Token extractToken();
    
private:
    Token readToken();
    
    Token readTokenAfterComment();
    Token readTokenAfterSpace();
    
    Token readIdentifierOrKeyword();
    Token readNumber();
    Token readOperator();
    
    static bool isCommentBeginning( char ) noexcept;
    static bool isWhiteSpace( char ) noexcept;
    static bool isWordBeginning( char ) noexcept;
    static bool isWordContinuation( char ) noexcept;
    static bool isLetter( char ) noexcept;
    static bool isDigit( char ) noexcept;
    static bool isOperator( char );
    
    std::istream &_stream;
    
    Token _currentToken;
    Token _nextToken;
};

}
