#include "LexicalAnalyser.h"

#include <regex>

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

Token LexicalAnalyser::readToken() {

    struct Rule {
        std::regex regex;
        Token::Type tokenType;
        bool storeLexem;
    };

    static const Rule rules[] = {
        { std::regex("\\s+"), Token::Whitespace, false },
        { std::regex("and(?![_0-9a-zA-Z])"), Token::Keyword_And, false },
        { std::regex("boolean(?![_0-9a-zA-Z])"), Token::Keyword_Boolean, false },
        { std::regex("class(?![_0-9a-zA-Z])"), Token::Keyword_Class, false },
        { std::regex("constant(?![_0-9a-zA-Z])"), Token::Keyword_Constant, false },
        { std::regex("if(?![_0-9a-zA-Z])"), Token::Keyword_If, false },
        { std::regex("in(?![_0-9a-zA-Z])"), Token::Keyword_In, false },
        { std::regex("integer(?![_0-9a-zA-Z])"), Token::Keyword_Integer, false },
        { std::regex("else(?![_0-9a-zA-Z])"), Token::Keyword_Else, false },
        { std::regex("false(?![_0-9a-zA-Z])"), Token::Keyword_False, false },
        { std::regex("for(?![_0-9a-zA-Z])"), Token::Keyword_For, false },
        { std::regex("function(?![_0-9a-zA-Z])"), Token::Keyword_Function, false },
        { std::regex("or(?![_0-9a-zA-Z])"), Token::Keyword_Or, false },
        { std::regex("real(?![_0-9a-zA-Z])"), Token::Keyword_Real, false },
        { std::regex("return(?![_0-9a-zA-Z])"), Token::Keyword_Return, false },
        { std::regex("true(?![_0-9a-zA-Z])"), Token::Keyword_True, false },
        { std::regex("variable(?![_0-9a-zA-Z])"), Token::Keyword_Variable, false },
        { std::regex("while(?![_0-9a-zA-Z])"), Token::Keyword_While, false },
        { std::regex("[_a-zA-Z][_0-9a-zA-Z]*"), Token::Identifier, true },        
        { std::regex(";"), Token::Punctuator_Semicolon, false },
        { std::regex(","), Token::Punctuator_Comma, false },
        { std::regex("\\."), Token::Punctuator_Dot, false },
        { std::regex("\\("), Token::Punctuator_OpeningParenthesis, false },
        { std::regex("\\)"), Token::Punctuator_ClosingParenthesis, false },
        { std::regex("\\{"), Token::Punctuator_OpeningBrace, false },
        { std::regex("\\}"), Token::Punctuator_ClosingBrace, false },
        { std::regex("<="), Token::Operator_LessOrEqual, false },
        { std::regex(">="), Token::Operator_GreaterOrEqual, false },
        { std::regex("=="), Token::Operator_Equal, false },
        { std::regex("\\+\\+"), Token::Operator_Increment, false },
        { std::regex("--"), Token::Operator_Decrement, false },
        { std::regex("\\+"), Token::Operator_Plus, false },
        { std::regex("-"), Token::Operator_Minus, false },
        { std::regex("\\*"), Token::Operator_Multiply, false },
        { std::regex("/"), Token::Operator_Divide, false },
        { std::regex("="), Token::Operator_Assign, false },
        { std::regex("<"), Token::Operator_Less, false },
        { std::regex(">"), Token::Operator_Greater, false },
        { std::regex("\\d+"), Token::Constant_Integer, true },
        { std::regex("#.*"), Token::Comment, false }
	};

	std::smatch matchResult;

    for( auto &rule : rules ) {

        if( std::regex_search(_first, _last, matchResult, rule.regex, std::regex_constants::match_continuous) ) {
            
            _first += matchResult.length();
            
            if( rule.tokenType == Token::Whitespace || rule.tokenType == Token::Comment )
                return readToken();

            return Token( rule.tokenType, rule.storeLexem ? matchResult.str() : "" );
        }

    }

    if( _first == _last )
        return Token();
    
    throwRuntimeError( "Unknown token" );
}

}
