#pragma once

#include "Noexcept.h"

#include <string>

namespace _8b {

class Token {
public:
    
    enum Type {
        Null,
        Whitespace,
        Comment,
        Identifier,
        Keyword_And,
        Keyword_Boolean,
        Keyword_Constant,
        Keyword_If,
        Keyword_In,
        Keyword_Integer,
        Keyword_Else,
        Keyword_False,
        Keyword_For,
        Keyword_Function,
        Keyword_Or,
        Keyword_Real,
        Keyword_Return,
        Keyword_True,
        Keyword_Variable,
        Keyword_While,
        Constant_Integer,
        Constant_Real,
        Operator_Plus,
        Operator_Minus,
        Operator_Multiply,
        Operator_Divide,
        Operator_Assign,
        Operator_Less,
        Operator_Greater,
        Operator_LessOrEqual,
        Operator_GreaterOrEqual,
        Operator_Equal,
        Operator_Increment,
        Operator_Decrement,
        Punctuator_Semicolon,
        Punctuator_Comma,
        Punctuator_Dot,
        Punctuator_OpeningParenthesis,
        Punctuator_ClosingParenthesis,
        Punctuator_OpeningBrace,
        Punctuator_ClosingBrace
    };
    
    Token( Type type = Null, const std::string &lexem = "" );
    
    bool operator == ( const Token& ) const noexcept;
    bool operator != ( const Token& ) const noexcept;
    
    Type getType() const noexcept;
    const std::string& getLexem() const noexcept;
    
private:
    Type _type;
    std::string _lexem;
};

}
