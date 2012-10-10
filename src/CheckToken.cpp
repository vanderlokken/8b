#include "CheckToken.h"

#include <map>
#include <sstream>

#include "Exception.h"

namespace _8b {

const std::string & tokenTypeName( Token::Type tokenType ) {

    static std::map<Token::Type, std::string> names;

    if( names.empty() ) {
        names[Token::Null] = "end of file";
        names[Token::Identifier] = "identifier";
        names[Token::Keyword_And] = "and";
        names[Token::Keyword_Boolean] = "boolean";
        names[Token::Keyword_Constant] = "constant";
        names[Token::Keyword_If] = "if";
        names[Token::Keyword_In] = "in";
        names[Token::Keyword_Integer] = "integer";
        names[Token::Keyword_Else] = "else";
        names[Token::Keyword_False] = "false";
        names[Token::Keyword_For] = "for";
        names[Token::Keyword_Function] = "function";
        names[Token::Keyword_Or] = "or";
        names[Token::Keyword_Real] = "real";
        names[Token::Keyword_Return] = "return";
        names[Token::Keyword_True] = "true";
        names[Token::Keyword_Variable] = "variable";
        names[Token::Keyword_While] = "while";
        names[Token::Constant_Integer] = "integer constant";
        names[Token::Constant_Real] = "real constant";
        names[Token::Operator_Plus] = "+";
        names[Token::Operator_Minus] = "-";
        names[Token::Operator_Multiply] = "*";
        names[Token::Operator_Divide] = "/";
        names[Token::Operator_Assign] = "=";
        names[Token::Operator_Less] = "<";
        names[Token::Operator_Greater] = ">";
        names[Token::Operator_LessOrEqual] = "<=";
        names[Token::Operator_GreaterOrEqual] = ">=";
        names[Token::Operator_Equal] = "Operator_Equal";
        names[Token::Operator_Increment] = "++";
        names[Token::Operator_Decrement] = "--";
        names[Token::Punctuator_Semicolon] = ";";
        names[Token::Punctuator_Comma] = ",";
        names[Token::Punctuator_Dot] = ".";
        names[Token::Punctuator_OpeningParenthesis] = "(";
        names[Token::Punctuator_ClosingParenthesis] = ")";
        names[Token::Punctuator_OpeningBrace] = "{";
        names[Token::Punctuator_ClosingBrace] = "}";
    }

    return names[tokenType];
}

void checkToken( const Token &token, Token::Type tokenType ) {

    if( token.getType() == tokenType )
        return;

    std::stringstream stream;

    stream << "Unexpected token: '"
           << tokenTypeName(token.getType()) << "' instead of '"
           << tokenTypeName(tokenType) << "'";

    throwRuntimeError( stream.str().c_str() );
}

}
