#pragma once

#include <string>

namespace _8b {

enum class TokenType {
    Null,
    Whitespace,
    LineEnd,
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
    OperatorLessOrEqual,
    OperatorGreater,
    OperatorGreaterOrEqual,
    OperatorEqual,
    OperatorNotEqual,
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

struct SourceLocation {

    SourceLocation() : lineNumber( 1 ), columnNumber( 1 ),
        lineBeginningOffset( 0 ), tokenBeginningOffset( 0 ) {}

    size_t lineNumber;
    size_t columnNumber;
    size_t lineBeginningOffset;
    size_t tokenBeginningOffset;
};

struct Token {

    Token( SourceLocation sourceLocation, TokenType tokenType = TokenType::Null,
            const std::string &lexem = "" )
        : sourceLocation( sourceLocation ), type( tokenType ), lexem( lexem ) {}

    SourceLocation sourceLocation;
    TokenType type;
    std::string lexem;
};

}
