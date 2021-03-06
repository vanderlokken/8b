#include "Parser.h"

#include <sstream>

#include "Exception.h"

namespace _8b {
namespace ast {

struct Parser : public NodeVisitor {

    explicit Parser( LexicalAnalyser &lexicalAnalyser )
        : _lexicalAnalyser( lexicalAnalyser ) {}

    template< class NodeType > std::shared_ptr< NodeType > parse() {
        auto node = std::make_shared< NodeType >();
        node->sourceLocation =
            _lexicalAnalyser.getCurrentToken().sourceLocation;
        node->acceptVisitor( this );
        return node;
    }

    std::string parseIdentifier() {
        return extractToken( TokenType::Identifier ).lexem;
    }

    // ------------------------------------------------------------------------
    //  Module level nodes
    // ------------------------------------------------------------------------

    boost::any visit( Module module ) {

        do
            if( currentTokenIs(TokenType::KeywordClass) )
                module->classDeclarations.emplace_back(
                    parse<_ClassDeclaration>() );
            else if( currentTokenIs(TokenType::KeywordFunction) )
                module->functionDeclarations.emplace_back(
                    parse<_FunctionDeclaration>() );
            else
                throw createSyntaxError(
                    "Expected class or function declaration" );
        while( !currentTokenIs(TokenType::Null) );

        return boost::any();
    }

    boost::any visit( ClassDeclaration classDeclaration ) {

        extractToken( TokenType::KeywordClass );

        classDeclaration->identifier = parseIdentifier();

        extractToken( TokenType::PunctuatorOpeningBrace );

        while( !currentTokenIs(TokenType::PunctuatorClosingBrace) ) {
            if( currentTokenIs(TokenType::KeywordVariable) )
                classDeclaration->memberDeclarations.emplace_back(
                    parse<_VariableDeclaration>() );
            else if( currentTokenIs(TokenType::KeywordFunction) )
                classDeclaration->methodDeclarations.emplace_back(
                    parse<_FunctionDeclaration>() );
            else
                throw createSyntaxError(
                    "Expected member or method declaration" );
        }

        extractToken( TokenType::PunctuatorClosingBrace );

        return boost::any();
    }

    boost::any visit( FunctionArgument functionArgument ) {
        functionArgument->identifier = parseIdentifier();
        functionArgument->type = parse<_Type>();
        return boost::any();
    }

    boost::any visit( FunctionDeclaration functionDeclaration ) {

        extractToken( TokenType::KeywordFunction );

        functionDeclaration->identifier = parseIdentifier();

        extractToken( TokenType::PunctuatorOpeningParenthesis );

        while( !currentTokenIs(TokenType::PunctuatorClosingParenthesis) ) {

            if( !functionDeclaration->arguments.empty() )
                extractToken( TokenType::PunctuatorComma );

            functionDeclaration->arguments.emplace_back(
                parse<_FunctionArgument>() );
        }

        extractToken( TokenType::PunctuatorClosingParenthesis );

        if( !currentTokenIs(TokenType::PunctuatorOpeningBrace) )
            functionDeclaration->returnType = parse<_Type>();

        functionDeclaration->block = parse<_Block>();

        return boost::any();
    }

    // ------------------------------------------------------------------------
    //  Statements
    // ------------------------------------------------------------------------

    template<> Statement parse<_Statement>() {

        const TokenType tokenType = _lexicalAnalyser.getCurrentToken().type;

        switch( tokenType ) {
        case TokenType::PunctuatorOpeningBrace:
            return parse<_Block>();
        case TokenType::KeywordIf:
            return parse<_IfStatement>();
        case TokenType::KeywordReturn:
            return parse<_ReturnStatement>();
        case TokenType::KeywordVariable:
            return parse<_VariableDeclaration>();
        case TokenType::KeywordWhile:
            return parse<_WhileStatement>();
        }

        Statement statement = parse<_Expression>();
        extractToken( TokenType::PunctuatorSemicolon );
        return statement;
    }

    boost::any visit( Block block ) {

        extractToken( TokenType::PunctuatorOpeningBrace );

        while( !currentTokenIs(TokenType::PunctuatorClosingBrace) )
            block->statements.emplace_back( parse<_Statement>() );

        extractToken( TokenType::PunctuatorClosingBrace );

        return boost::any();
    }

    boost::any visit( IfStatement statement ) {

        extractToken( TokenType::KeywordIf );

        extractToken( TokenType::PunctuatorOpeningParenthesis );
        statement->condition = parse<_Expression>();
        extractToken( TokenType::PunctuatorClosingParenthesis );

        statement->trueBlock = parse<_Block>();

        if( currentTokenIs(TokenType::KeywordElse) ) {
            extractToken();
            statement->falseBlock = parse<_Block>();
        } else
            // Creates a dummy empty block. This allows to handle "if"
            // statements with "false" blocks and without them uniformly
            statement->falseBlock = std::make_shared<_Block>();

        return boost::any();
    }

    boost::any visit( ReturnStatement statement ) {

        extractToken( TokenType::KeywordReturn );

        if( !currentTokenIs(TokenType::PunctuatorSemicolon) )
            statement->expression = parse<_Expression>();

        extractToken( TokenType::PunctuatorSemicolon );

        return boost::any();
    }

    boost::any visit( VariableDeclaration variableDeclaraion ) {

        extractToken( TokenType::KeywordVariable );

        variableDeclaraion->identifier = parseIdentifier();

        if( currentTokenIs(TokenType::OperatorAssign) ) {
            extractToken( TokenType::OperatorAssign );
            variableDeclaraion->initializer = parse<_Expression>();
        } else
            variableDeclaraion->type = parse<_Type>();

        extractToken( TokenType::PunctuatorSemicolon );

        return boost::any();
    }

    boost::any visit( WhileStatement statement ) {

        extractToken( TokenType::KeywordWhile );

        extractToken( TokenType::PunctuatorOpeningParenthesis );
        statement->condition = parse<_Expression>();
        extractToken( TokenType::PunctuatorClosingParenthesis );

        statement->block = parse<_Block>();

        return boost::any();
    }

    // ------------------------------------------------------------------------
    //  Expressions
    //
    //  Note: expressions are parsed using the "top down operator precedence"
    //        principle. See http://en.wikipedia.org/wiki/Pratt_parser
    // ------------------------------------------------------------------------

    int getLeftBindingPower() const {

        const TokenType tokenType = _lexicalAnalyser.getCurrentToken().type;

        switch( tokenType ) {
        case TokenType::OperatorAssign:
            return 4;
        case TokenType::KeywordAnd:
        case TokenType::KeywordOr:
            return 5;
        case TokenType::OperatorLess:
        case TokenType::OperatorLessOrEqual:
        case TokenType::OperatorGreater:
        case TokenType::OperatorGreaterOrEqual:
        case TokenType::OperatorEqual:
        case TokenType::OperatorNotEqual:
            return 6;
        case TokenType::OperatorPlus:
        case TokenType::OperatorMinus:
            return 7;
        case TokenType::OperatorMultiply:
        case TokenType::OperatorDivide:
            return 8;
        case TokenType::OperatorIncrement:
        case TokenType::OperatorDecrement:
        case TokenType::KeywordNot:
            return 9;
        case TokenType::PunctuatorOpeningParenthesis:
        case TokenType::PunctuatorDot:
            return 10;
        }

        return 0;
    }

    template<> Expression parse<_Expression>() {
        return parseExpression();
    }

    Expression parseExpression( int rightBindingPower = 0 ) {

        Expression expression = nullDenotation();

        while( rightBindingPower < getLeftBindingPower() )
            expression = leftDenotation( expression );

        return expression;
    }

    Expression nullDenotation() {

        const TokenType tokenType = _lexicalAnalyser.getCurrentToken().type;

        switch( tokenType ) {
        case TokenType::KeywordInstance:
            return parse<_InstanceExpression>();
        case TokenType::Identifier:
            return parse<_IdentifierExpression>();
        case TokenType::ConstantInteger:
            return parse<_IntegerConstant>();
        case TokenType::ConstantString:
            return parse<_StringConstant>();
        case TokenType::KeywordTrue:
        case TokenType::KeywordFalse:
            return parse<_BooleanConstant>();

        case TokenType::PunctuatorOpeningParenthesis: {

            extractToken();

            Expression expression = parse<_Expression>();

            extractToken( TokenType::PunctuatorClosingParenthesis );

            return expression;
        }

        case TokenType::KeywordBoolean:
        case TokenType::KeywordInteger:
        case TokenType::KeywordReal:
        case TokenType::KeywordPointerTo:
        {
            UnaryOperation operation;

            if( tokenType == TokenType::KeywordBoolean )
                operation =  UnaryOperation::BooleanConversion;
            else if( tokenType == TokenType::KeywordInteger )
                operation =  UnaryOperation::IntegerConversion;
            else if( tokenType == TokenType::KeywordReal )
                operation =  UnaryOperation::RealConversion;
            else if( tokenType == TokenType::KeywordPointerTo )
                operation =  UnaryOperation::PointerConversion;

            auto sourceLocation = extractToken().sourceLocation;
            extractToken( TokenType::PunctuatorOpeningParenthesis );

            auto expression = std::make_shared<_UnaryOperationExpression>();
            expression->sourceLocation = sourceLocation;
            expression->operation = operation;
            expression->operand = parse<_Expression>();

            extractToken( TokenType::PunctuatorClosingParenthesis );

            return expression;

        }

        case TokenType::KeywordNot:
        {
            const int bindingPower = getLeftBindingPower();

            auto expression = std::make_shared<_UnaryOperationExpression>();
            expression->sourceLocation = extractToken().sourceLocation;
            expression->operation = UnaryOperation::LogicInversion;
            expression->operand = parseExpression( bindingPower );

            return expression;
        }

        }

        throw createSyntaxError( "Unexpected token" );
    }

    Expression leftDenotation( Expression expression ) {

        // TODO: use 'parse<_BinaryOperatorExpression>()' etc. instead of
        // explicit parsing in this function

        struct UnaryOperatorParsingRule {
            TokenType tokenType;
            UnaryOperation operation;
        };

        struct BinaryOperatorParsingRule {
            TokenType tokenType;
            BinaryOperation operation;
        };

        static const UnaryOperatorParsingRule unaryOperatorParsingRules[] = {
            {TokenType::OperatorIncrement, UnaryOperation::Increment},
            {TokenType::OperatorDecrement, UnaryOperation::Decrement}
        };

        static const BinaryOperatorParsingRule binaryOperatorParsingRules[] = {
            {TokenType::OperatorAssign,   BinaryOperation::Assignment},
            {TokenType::OperatorPlus,     BinaryOperation::Addition},
            {TokenType::OperatorMinus,    BinaryOperation::Subtraction},
            {TokenType::OperatorMultiply, BinaryOperation::Multiplication},
            {TokenType::OperatorDivide,   BinaryOperation::Division},
            {TokenType::KeywordAnd,       BinaryOperation::LogicAnd},
            {TokenType::KeywordOr,        BinaryOperation::LogicOr},
            {TokenType::OperatorLess,     BinaryOperation::LessComparison},
            {TokenType::OperatorGreater,  BinaryOperation::GreaterComparison},
            {TokenType::OperatorEqual,    BinaryOperation::EqualComparison},
            {TokenType::OperatorNotEqual, BinaryOperation::NotEqualComparison},
            {TokenType::OperatorLessOrEqual,
                BinaryOperation::LessOrEqualComparison},
            {TokenType::OperatorGreaterOrEqual,
                BinaryOperation::GreaterOrEqualComparison}
        };

        const TokenType tokenType = _lexicalAnalyser.getCurrentToken().type;

        const int bindingPower = getLeftBindingPower();

        for( auto &rule : binaryOperatorParsingRules ) {
            if( tokenType == rule.tokenType ) {
                auto sourceLocation = extractToken().sourceLocation;
                auto result = std::make_shared<_BinaryOperationExpression>();
                result->sourceLocation = sourceLocation;
                result->operation = rule.operation;
                result->leftOperand = expression;
                result->rightOperand = parseExpression( bindingPower );
                return result;
            }
        }

        for( auto &rule : unaryOperatorParsingRules ) {
            if( tokenType == rule.tokenType ) {
                auto sourceLocation = extractToken().sourceLocation;
                auto result = std::make_shared<_UnaryOperationExpression>();
                result->sourceLocation = sourceLocation;
                result->operation = rule.operation;
                result->operand = expression;
                return result;
            }
        }

        if( tokenType == TokenType::PunctuatorOpeningParenthesis ) {
            auto sourceLocation = extractToken().sourceLocation;
            auto result = std::make_shared<_CallExpression>();
            result->sourceLocation = sourceLocation;
            result->callee = expression;
            while( !currentTokenIs(TokenType::PunctuatorClosingParenthesis) ) {
                if( !result->arguments.empty() )
                    extractToken( TokenType::PunctuatorComma );
                result->arguments.emplace_back( parse<_Expression>() );
            }
            extractToken( TokenType::PunctuatorClosingParenthesis );
            return result;
        }

        if( tokenType == TokenType::PunctuatorDot ) {
            auto sourceLocation = extractToken().sourceLocation;
            auto result = std::make_shared<_MemberAccessExpression>();
            result->sourceLocation = sourceLocation;
            result->object = expression;
            result->memberIdentifier = parseIdentifier();
            return result;
        }

        return expression;
    }

    boost::any visit( BinaryOperationExpression ) {
        return boost::any();
    }

    boost::any visit( BooleanConstant booleanConstant ) {

        if( currentTokenIs(TokenType::KeywordTrue) )
            booleanConstant->value = true;
        else if( currentTokenIs(TokenType::KeywordFalse) )
            booleanConstant->value = false;
        else
            throw createSyntaxError( "Expected 'true' or 'false'" );

        extractToken();

        return boost::any();
    }

    boost::any visit( CallExpression ) {
        return boost::any();
    }

    boost::any visit( IdentifierExpression expression ) {
        expression->identifier = parseIdentifier();
        return boost::any();
    }

    boost::any visit( InstanceExpression expression ) {
        extractToken( TokenType::KeywordInstance );
        return boost::any();
    }

    boost::any visit( IntegerConstant integerConstant ) {
        std::stringstream stream;
        stream << extractToken( TokenType::ConstantInteger ).lexem;
        stream >> integerConstant->value;
        return boost::any();
    }

    boost::any visit( MemberAccessExpression ) {
        return boost::any();
    }

    boost::any visit( StringConstant stringConstant ) {
        const auto &lexem = extractToken( TokenType::ConstantString ).lexem;
        // Remove surrounding '"' symbols
        stringConstant->value = lexem.substr( 1, lexem.size() - 2 );
        return boost::any();
    }

    boost::any visit( UnaryOperationExpression ) {
        return boost::any();
    }

    // ------------------------------------------------------------------------
    //  Types
    // ------------------------------------------------------------------------

    template<> Type parse<_Type>() {

        const TokenType tokenType = _lexicalAnalyser.getCurrentToken().type;

        switch( tokenType ) {
        case TokenType::KeywordBoolean:
            return parse<_BooleanType>();
        case TokenType::Identifier:
            return parse<_NamedType>();
        case TokenType::KeywordInteger:
            return parse<_IntegerType>();
        case TokenType::KeywordPointer:
            return parse<_PointerType>();
        case TokenType::KeywordReal:
            return parse<_RealType>();
        case TokenType::KeywordString:
            return parse<_StringType>();
        }

        throw createSyntaxError( "Expected type" );
    }

    boost::any visit( BooleanType type ) {
        extractToken( TokenType::KeywordBoolean );
        return boost::any();
    }

    boost::any visit( IntegerType type ) {
        extractToken( TokenType::KeywordInteger );
        return boost::any();
    }

    boost::any visit( NamedType type ) {
        type->identifier = parseIdentifier();
        return boost::any();
    }

    boost::any visit( PointerType type ) {
        extractToken( TokenType::KeywordPointer );
        extractToken( TokenType::PunctuatorOpeningBracket );
        type->targetType = parse<_Type>();
        extractToken( TokenType::PunctuatorClosingBracket );
        return boost::any();
    }

    boost::any visit( RealType type ) {
        extractToken( TokenType::KeywordReal );
        return boost::any();
    }

    boost::any visit( StringType type ) {
        extractToken( TokenType::KeywordString );
        return boost::any();
    }

    // ------------------------------------------------------------------------
    //  Shortcuts
    // ------------------------------------------------------------------------

    bool currentTokenIs( TokenType tokenType ) const {
        return _lexicalAnalyser.getCurrentToken().type == tokenType;
    }

    Token extractToken() {
        return _lexicalAnalyser.extractToken();
    }

    Token extractToken( TokenType tokenType ) {
        if( !currentTokenIs(tokenType) )
            throw createSyntaxError( "Unexpected token" );
        return _lexicalAnalyser.extractToken();
    }

    SyntaxError createSyntaxError( const char *message ) const {
        return SyntaxError(
            message, _lexicalAnalyser.getCurrentToken().sourceLocation );
    }

    LexicalAnalyser &_lexicalAnalyser;
};

} // namespace ast

ast::Module parse( LexicalAnalyser &lexicalAnalyser ) {
    return ast::Parser( lexicalAnalyser ).parse< ast::_Module >();
}

} // namespace _8b
