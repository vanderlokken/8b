#pragma once

#include "Noexcept.h"

#include <string>

namespace _8b {
    
enum class TokenType {
    Null,
    Whitespace,
    Comment,
    Identifier,
    KeywordAnd,
    KeywordBoolean,
    KeywordClass,
    KeywordConstant,
    KeywordIf,
    KeywordIn,
    KeywordInstance,
    KeywordInteger,
    KeywordElse,
    KeywordFalse,
    KeywordFor,
    KeywordFunction,
    KeywordOr,
    KeywordPointer,
    KeywordPointerTo,
    KeywordReal,
    KeywordReturn,
    KeywordString,
    KeywordTrue,
    KeywordVariable,
    KeywordWhile,
    ConstantInteger,
    ConstantReal,
    ConstantString,
    OperatorPlus,
    OperatorMinus,
    OperatorMultiply,
    OperatorDivide,
    OperatorAssign,
    OperatorLess,
    OperatorGreater,
    OperatorLessOrEqual,
    OperatorGreaterOrEqual,
    OperatorEqual,
    OperatorIncrement,
    OperatorDecrement,
    PunctuatorSemicolon,
    PunctuatorComma,
    PunctuatorDot,
    PunctuatorOpeningParenthesis,
    PunctuatorClosingParenthesis,
    PunctuatorOpeningBrace,
    PunctuatorClosingBrace,
    PunctuatorOpeningBracket,
    PunctuatorClosingBracket
};

struct Token {
    
    Token( TokenType type = TokenType::Null, const std::string &lexem = "" );
    
    bool operator == ( const Token& ) const noexcept;
    bool operator != ( const Token& ) const noexcept;
    
    TokenType type;
    std::string lexem;
};

}
