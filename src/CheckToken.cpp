#include "CheckToken.h"

#include <map>
#include <sstream>

#include "Exception.h"

namespace _8b {

const std::string & tokenTypeName( TokenType tokenType ) {

    static std::map<TokenType, std::string> names;

    if( names.empty() ) {
        names[TokenType::Null] = "end of file";
        names[TokenType::Identifier] = "identifier";
        names[TokenType::KeywordAnd] = "and";
        names[TokenType::KeywordBoolean] = "boolean";
        names[TokenType::KeywordConstant] = "constant";
        names[TokenType::KeywordIf] = "if";
        names[TokenType::KeywordIn] = "in";
        names[TokenType::KeywordInteger] = "integer";
        names[TokenType::KeywordElse] = "else";
        names[TokenType::KeywordFalse] = "false";
        names[TokenType::KeywordFor] = "for";
        names[TokenType::KeywordFunction] = "function";
        names[TokenType::KeywordOr] = "or";
        names[TokenType::KeywordReal] = "real";
        names[TokenType::KeywordReturn] = "return";
        names[TokenType::KeywordTrue] = "true";
        names[TokenType::KeywordVariable] = "variable";
        names[TokenType::KeywordWhile] = "while";
        names[TokenType::ConstantInteger] = "integer constant";
        names[TokenType::ConstantReal] = "real constant";
        names[TokenType::OperatorPlus] = "+";
        names[TokenType::OperatorMinus] = "-";
        names[TokenType::OperatorMultiply] = "*";
        names[TokenType::OperatorDivide] = "/";
        names[TokenType::OperatorAssign] = "=";
        names[TokenType::OperatorLess] = "<";
        names[TokenType::OperatorGreater] = ">";
        names[TokenType::OperatorLessOrEqual] = "<=";
        names[TokenType::OperatorGreaterOrEqual] = ">=";
        names[TokenType::OperatorEqual] = "Operator_Equal";
        names[TokenType::OperatorIncrement] = "++";
        names[TokenType::OperatorDecrement] = "--";
        names[TokenType::PunctuatorSemicolon] = ";";
        names[TokenType::PunctuatorComma] = ",";
        names[TokenType::PunctuatorDot] = ".";
        names[TokenType::PunctuatorOpeningParenthesis] = "(";
        names[TokenType::PunctuatorClosingParenthesis] = ")";
        names[TokenType::PunctuatorOpeningBrace] = "{";
        names[TokenType::PunctuatorClosingBrace] = "}";
    }

    return names[tokenType];
}

void checkToken( const Token &token, TokenType tokenType ) {

    if( token.type == tokenType )
        return;

    std::stringstream stream;

    stream << "Unexpected token: '"
           << tokenTypeName(token.type) << "' instead of '"
           << tokenTypeName(tokenType) << "'";

    throwRuntimeError( stream.str().c_str() );
}

}
