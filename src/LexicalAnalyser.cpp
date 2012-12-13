#include "LexicalAnalyser.h"

#include <regex>

#include "CheckToken.h"
#include "Exception.h"

namespace _8b {

LexicalAnalyser::LexicalAnalyser( std::istream &stream )
    : _source( (std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>() ),
      _first( _source.begin() ),
      _last( _source.end() )
{
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

Token LexicalAnalyser::extractToken( TokenType tokenType ) {
    checkToken( getCurrentToken(), tokenType );
    return extractToken();
}

Token LexicalAnalyser::readToken() {

    struct Rule {
        std::regex regex;
        TokenType tokenType;
        bool storeLexem;
    };

    static const Rule rules[] = {
        { std::regex("\\s+"), TokenType::Whitespace, false },
        { std::regex("and(?![_0-9a-zA-Z])"), TokenType::KeywordAnd, false },
        { std::regex("boolean(?![_0-9a-zA-Z])"), TokenType::KeywordBoolean, false },
        { std::regex("class(?![_0-9a-zA-Z])"), TokenType::KeywordClass, false },
        { std::regex("constant(?![_0-9a-zA-Z])"), TokenType::KeywordConstant, false },
        { std::regex("if(?![_0-9a-zA-Z])"), TokenType::KeywordIf, false },
        { std::regex("in(?![_0-9a-zA-Z])"), TokenType::KeywordIn, false },
        { std::regex("instance(?![_0-9a-zA-Z])"), TokenType::KeywordInstance, false },
        { std::regex("integer(?![_0-9a-zA-Z])"), TokenType::KeywordInteger, false },
        { std::regex("else(?![_0-9a-zA-Z])"), TokenType::KeywordElse, false },
        { std::regex("false(?![_0-9a-zA-Z])"), TokenType::KeywordFalse, false },
        { std::regex("for(?![_0-9a-zA-Z])"), TokenType::KeywordFor, false },
        { std::regex("function(?![_0-9a-zA-Z])"), TokenType::KeywordFunction, false },
        { std::regex("or(?![_0-9a-zA-Z])"), TokenType::KeywordOr, false },
        { std::regex("pointerTo(?![_0-9a-zA-Z])"), TokenType::KeywordPointerTo, false },
        { std::regex("pointer(?![_0-9a-zA-Z])"), TokenType::KeywordPointer, false },
        { std::regex("real(?![_0-9a-zA-Z])"), TokenType::KeywordReal, false },
        { std::regex("return(?![_0-9a-zA-Z])"), TokenType::KeywordReturn, false },
        { std::regex("string(?![_0-9a-zA-Z])"), TokenType::KeywordString, false },
        { std::regex("true(?![_0-9a-zA-Z])"), TokenType::KeywordTrue, false },
        { std::regex("variable(?![_0-9a-zA-Z])"), TokenType::KeywordVariable, false },
        { std::regex("while(?![_0-9a-zA-Z])"), TokenType::KeywordWhile, false },
        { std::regex("[_a-zA-Z][_0-9a-zA-Z]*"), TokenType::Identifier, true },        
        { std::regex(";"), TokenType::PunctuatorSemicolon, false },
        { std::regex(","), TokenType::PunctuatorComma, false },
        { std::regex("\\."), TokenType::PunctuatorDot, false },
        { std::regex("\\("), TokenType::PunctuatorOpeningParenthesis, false },
        { std::regex("\\)"), TokenType::PunctuatorClosingParenthesis, false },
        { std::regex("\\{"), TokenType::PunctuatorOpeningBrace, false },
        { std::regex("\\}"), TokenType::PunctuatorClosingBrace, false },
        { std::regex("\\["), TokenType::PunctuatorOpeningBracket, false },
        { std::regex("\\]"), TokenType::PunctuatorClosingBracket, false },
        { std::regex("<="), TokenType::OperatorLessOrEqual, false },
        { std::regex(">="), TokenType::OperatorGreaterOrEqual, false },
        { std::regex("=="), TokenType::OperatorEqual, false },
        { std::regex("\\+\\+"), TokenType::OperatorIncrement, false },
        { std::regex("--"), TokenType::OperatorDecrement, false },
        { std::regex("\\+"), TokenType::OperatorPlus, false },
        { std::regex("-"), TokenType::OperatorMinus, false },
        { std::regex("\\*"), TokenType::OperatorMultiply, false },
        { std::regex("/"), TokenType::OperatorDivide, false },
        { std::regex("="), TokenType::OperatorAssign, false },
        { std::regex("<"), TokenType::OperatorLess, false },
        { std::regex(">"), TokenType::OperatorGreater, false },
        { std::regex("\\d+"), TokenType::ConstantInteger, true },
        { std::regex("\".*\""), TokenType::ConstantString, true },
        { std::regex("#.*"), TokenType::Comment, false }
	};

	std::smatch matchResult;

    for( auto &rule : rules ) {

        if( std::regex_search(_first, _last, matchResult, rule.regex, std::regex_constants::match_continuous) ) {
            
            _first += matchResult.length();
            
            if( rule.tokenType == TokenType::Whitespace || rule.tokenType == TokenType::Comment )
                return readToken();

            return Token( rule.tokenType, rule.storeLexem ? matchResult.str() : "" );
        }

    }

    if( _first == _last )
        return Token();
    
    throwRuntimeError( "Unknown token" );
}

}
