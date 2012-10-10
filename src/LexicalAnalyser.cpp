#include "LexicalAnalyser.h"

#include <algorithm>
#include <array>
#include <map>

#include "Exception.h"

namespace _8b {

LexicalAnalyser::LexicalAnalyser( std::istream &stream )
    : _stream( stream )
{
    if( !stream )
        throwInvalidArgument( "'stream' is invalid" );
    
    _currentToken = readToken();
    _nextToken = readToken();
    return;
}

Token LexicalAnalyser::getCurrentToken() const noexcept {
    return _currentToken;
}

Token LexicalAnalyser::getNextToken() const noexcept {
    return _nextToken;
}

Token LexicalAnalyser::extractToken() {
    Token result = _currentToken;
    
    _currentToken = _nextToken;
    _nextToken = readToken();
    
    return result;
}

Token LexicalAnalyser::readToken() {
    char character = _stream.peek();
    
    // Omit characters and then read token
    
    if( isCommentBeginning(character) )
        return readTokenAfterComment();
    
    if( isWhiteSpace(character) )
        return readTokenAfterSpace();
    
    // Read token
    
    if( isWordBeginning(character) )
        return readIdentifierOrKeyword();
    
    if( isDigit(character) )
        return readNumber();
    
    if( isOperator(character) )
        return readOperator();
    
    // Throw an exception when an invalid character is encountered
    
    if( _stream.good() )
        throwRuntimeError( "The source code contains an invalid character" );
    
    // Throw an exception when an internal input error occurs
    
    if( _stream.fail() && !_stream.eof() )
        throwRuntimeError( "An input error has occured during the source code parsing" );
    
    return Token();
}

Token LexicalAnalyser::readTokenAfterComment() {
    while( _stream.peek() != '\r' && _stream.peek() != '\n' && _stream )
        _stream.get();
    return readToken();
}

Token LexicalAnalyser::readTokenAfterSpace() {
    while( isWhiteSpace(_stream.peek()) && _stream )
        _stream.get();
    return readToken();
}

Token LexicalAnalyser::readIdentifierOrKeyword() {

    static const class TokenByLexem {
    public:
        
        TokenByLexem() {
            _keywords["and"] = Token( Token::Keyword_And );
            _keywords["boolean"] = Token( Token::Keyword_Boolean );
            _keywords["constant"] = Token( Token::Keyword_Constant );
            _keywords["if"] = Token( Token::Keyword_If );
            _keywords["in"] = Token( Token::Keyword_In );
            _keywords["integer"] = Token( Token::Keyword_Integer );
            _keywords["else"] = Token( Token::Keyword_Else );
            _keywords["false"] = Token( Token::Keyword_False );
            _keywords["for"] = Token( Token::Keyword_For );
            _keywords["function"] = Token( Token::Keyword_Function );
            _keywords["or"] = Token( Token::Keyword_Or );
            _keywords["real"] = Token( Token::Keyword_Real );
            _keywords["return"] = Token( Token::Keyword_Return );
            _keywords["true"] = Token( Token::Keyword_True );
            _keywords["variable"] = Token( Token::Keyword_Variable );
            _keywords["while"] = Token( Token::Keyword_While );
        }

        Token operator() ( const std::string &lexem ) const {
            auto result = _keywords.find( lexem );
            return result != _keywords.end() ? result->second : Token( Token::Identifier, lexem );
        }

    private:
        std::map< std::string, Token > _keywords;

    } tokenByLexem;

    std::string lexem;

    do {
        lexem += _stream.get();
    } while( isWordContinuation(_stream.peek()) );

    return tokenByLexem( lexem );
}

Token LexicalAnalyser::readNumber() {

    Token::Type tokenType = Token::Constant_Integer;

    std::string lexem;
    
    // Read an integer part
    
    while( isDigit(_stream.peek()) )
        lexem += _stream.get();
    
    // Read an optional fractional part if it exists
    
    if( _stream.peek() == '.' ) {
    
        lexem += _stream.get();
        
        while( isDigit(_stream.peek()) )
            lexem += _stream.get();

        tokenType = Token::Constant_Real;
    }
    
    // Read an optional exponent part if it exists
    
    if( _stream.peek() == 'E' || _stream.peek() == 'e' ) {
        
        lexem += _stream.get();
        
        if( _stream.peek() != '+' && _stream.peek() != '-' )
            throwRuntimeError( "A floating point number has no sign after the exponent character" );
        
        lexem += _stream.get();
        
        if( !isDigit(_stream.peek()) )
            throwRuntimeError( "A floating point number has no digits after the sign" );
        
        while( isDigit(_stream.peek()) )
            lexem += _stream.get();
    }

    return Token( tokenType, lexem );
}

Token LexicalAnalyser::readOperator() {

    char character = _stream.get();
    char nextCharacter = _stream.peek();

    switch( character ) {

        case ';':
            return Token( Token::Punctuator_Semicolon );
            break;

        case '.':
            return Token( Token::Punctuator_Dot );
            break;
            
        case '(':
            return Token( Token::Punctuator_OpeningParenthesis );
            break;
            
        case ')':
            return Token( Token::Punctuator_ClosingParenthesis );
            break;
            
        case '{':
            return Token( Token::Punctuator_OpeningBrace );
            break;
            
        case '}':
            return Token( Token::Punctuator_ClosingBrace );
            break;
            
        case ',':
            return Token( Token::Punctuator_Comma );
            break;

        case '+':
            if( nextCharacter == '+' ) {
                _stream.get();
                return Token( Token::Operator_Increment );
            }
            return Token( Token::Operator_Plus );
            break;
        
        case '-':
            if( nextCharacter == '-' ) {
                _stream.get();
                return Token( Token::Operator_Decrement );
            }
            return Token( Token::Operator_Minus );
            break;

        case '*':
            return Token( Token::Operator_Multiply );
            break;
        
        case '/':
            return Token( Token::Operator_Divide );
            break;

        case '<':
            return Token( Token::Operator_Less );
            break;

        case '>':
            return Token( Token::Operator_Greater );
            break;

        case '=':
            return Token( Token::Operator_Assign );
            break;
    }

    throwRuntimeError( "Not implemented" );
}


bool LexicalAnalyser::isCommentBeginning( char character ) noexcept {
    return character == '#';
}

bool LexicalAnalyser::isWhiteSpace( char character ) noexcept {
    return character == ' '  ||
           character == '\t' ||
           character == '\n' ||
           character == '\r';
}

bool LexicalAnalyser::isWordBeginning( char character ) noexcept {
    return isLetter(character) || character == '_';
}

bool LexicalAnalyser::isWordContinuation( char character ) noexcept {
    return isLetter(character) || isDigit(character) || character == '_';
}

bool LexicalAnalyser::isLetter( char character ) noexcept {
    return
        (character >= 'A' && character <= 'Z') ||
        (character >= 'a' && character <= 'z');
}

bool LexicalAnalyser::isDigit( char character ) noexcept {
    return character >= '0' && character <= '9';
}

bool LexicalAnalyser::isOperator( char character ) {
    static const std::string punctuators( ";.[](){},=!*/+-<>" );
    return punctuators.find( character ) != std::string::npos;
}

}