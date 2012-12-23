#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/any.hpp>

#include "Operation.h"

namespace _8b {
namespace ast {

struct NodeVisitor;

struct _Node : std::enable_shared_from_this<_Node> {
    virtual ~_Node() {}
    virtual boost::any acceptVisitor( NodeVisitor* ) = 0;
};

#define _8b_visitable( className )                                     \
    boost::any acceptVisitor( NodeVisitor *visitor ) {                 \
        return visitor->visit(                                         \
            std::static_pointer_cast<className>(shared_from_this()) ); \
    }

// ----------------------------------------------------------------------------
//  Forward declarations
// ----------------------------------------------------------------------------

struct _BinaryOperationExpression;
struct _Block;
struct _BooleanConstant;
struct _BooleanType;
struct _CallExpression;
struct _ClassDeclaration;
struct _Expression;
struct _FunctionArgument;
struct _FunctionDeclaration;
struct _IdentifierExpression;
struct _IfStatement;
struct _InstanceExpression;
struct _IntegerConstant;
struct _IntegerType;
struct _MemberAccessExpression;
struct _Module;
struct _NamedType;
struct _PointerType;
struct _ReturnStatement;
struct _Statement;
struct _StringConstant;
struct _StringType;
struct _Type;
struct _UnaryOperationExpression;
struct _VariableDeclaration;
struct _WhileStatement;

typedef std::shared_ptr<_BinaryOperationExpression> BinaryOperationExpression;
typedef std::shared_ptr<_Block> Block;
typedef std::shared_ptr<_BooleanConstant> BooleanConstant;
typedef std::shared_ptr<_BooleanType> BooleanType;
typedef std::shared_ptr<_CallExpression> CallExpression;
typedef std::shared_ptr<_ClassDeclaration> ClassDeclaration;
typedef std::shared_ptr<_Expression> Expression;
typedef std::shared_ptr<_FunctionArgument> FunctionArgument;
typedef std::shared_ptr<_FunctionDeclaration> FunctionDeclaration;
typedef std::shared_ptr<_IdentifierExpression> IdentifierExpression;
typedef std::shared_ptr<_IfStatement> IfStatement;
typedef std::shared_ptr<_InstanceExpression> InstanceExpression;
typedef std::shared_ptr<_IntegerConstant> IntegerConstant;
typedef std::shared_ptr<_IntegerType> IntegerType;
typedef std::shared_ptr<_MemberAccessExpression> MemberAccessExpression;
typedef std::shared_ptr<_Module> Module;
typedef std::shared_ptr<_NamedType> NamedType;
typedef std::shared_ptr<_Node> Node;
typedef std::shared_ptr<_PointerType> PointerType;
typedef std::shared_ptr<_ReturnStatement> ReturnStatement;
typedef std::shared_ptr<_Statement> Statement;
typedef std::shared_ptr<_StringConstant> StringConstant;
typedef std::shared_ptr<_StringType> StringType;
typedef std::shared_ptr<_Type> Type;
typedef std::shared_ptr<_UnaryOperationExpression> UnaryOperationExpression;
typedef std::shared_ptr<_VariableDeclaration> VariableDeclaration;
typedef std::shared_ptr<_WhileStatement> WhileStatement;

// ----------------------------------------------------------------------------
//  NodeVisitor
// ----------------------------------------------------------------------------

struct NodeVisitor {
    virtual ~NodeVisitor() {}

    virtual boost::any visit( Module ) = 0;
    virtual boost::any visit( ClassDeclaration ) = 0;
    virtual boost::any visit( FunctionArgument ) = 0;
    virtual boost::any visit( FunctionDeclaration ) = 0;

    virtual boost::any visit( Block ) = 0;
    virtual boost::any visit( IfStatement ) = 0;
    virtual boost::any visit( ReturnStatement ) = 0;
    virtual boost::any visit( VariableDeclaration ) = 0;
    virtual boost::any visit( WhileStatement ) = 0;

    virtual boost::any visit( BinaryOperationExpression ) = 0;
    virtual boost::any visit( BooleanConstant ) = 0;
    virtual boost::any visit( CallExpression ) = 0;
    virtual boost::any visit( IdentifierExpression ) = 0;
    virtual boost::any visit( InstanceExpression ) = 0;
    virtual boost::any visit( IntegerConstant ) = 0;
    virtual boost::any visit( MemberAccessExpression ) = 0;
    virtual boost::any visit( StringConstant ) = 0;
    virtual boost::any visit( UnaryOperationExpression ) = 0;

    virtual boost::any visit( BooleanType ) = 0;
    virtual boost::any visit( IntegerType ) = 0;
    virtual boost::any visit( NamedType ) = 0;
    virtual boost::any visit( PointerType ) = 0;
    virtual boost::any visit( StringType ) = 0;
};

// ----------------------------------------------------------------------------
//  Module level nodes
// ----------------------------------------------------------------------------

struct _Module : _Node {
    _8b_visitable( _Module );
    std::vector<FunctionDeclaration> functionDeclarations;
    std::vector<ClassDeclaration> classDeclarations;
};

struct _ClassDeclaration : _Node {
    _8b_visitable( _ClassDeclaration );
    std::string identifier;
    std::vector<VariableDeclaration> memberDeclarations;
    std::vector<FunctionDeclaration> methodDeclarations;
};

struct _FunctionArgument : _Node {
    _8b_visitable( _FunctionArgument );
    std::string identifier;
    Type type;
};

struct _FunctionDeclaration : _Node {
    _8b_visitable( _FunctionDeclaration );
    std::string identifier;
    std::vector<FunctionArgument> arguments;
    Type returnType;
    Block block;
};

// ----------------------------------------------------------------------------
//  Statements
// ----------------------------------------------------------------------------

struct _Statement : _Node {};

struct _Block : _Statement {
    _8b_visitable( _Block );
    std::vector<Statement> statements;
};

struct _IfStatement : _Statement {
    _8b_visitable( _IfStatement );
    Expression condition;
    Block trueBlock;
    Block falseBlock;
};

struct _ReturnStatement : _Statement {
    _8b_visitable( _ReturnStatement );
    Expression expression;
};

struct _VariableDeclaration : _Statement {
    _8b_visitable( _VariableDeclaration );
    std::string identifier;
    Type type;
    Expression initializer;
};

struct _WhileStatement : _Statement {
    _8b_visitable( _WhileStatement );
    Expression condition;
    Block block;
};

// ----------------------------------------------------------------------------
//  Expressions
// ----------------------------------------------------------------------------

struct _Expression : _Statement {};

struct _BinaryOperationExpression : _Expression {
    _8b_visitable( _BinaryOperationExpression );
    BinaryOperation operation;
    Expression leftOperand;
    Expression rightOperand;
};

struct _BooleanConstant : _Expression {
    _8b_visitable( _BooleanConstant );
    bool value;
};

struct _CallExpression : _Expression {
    _8b_visitable( _CallExpression );
    Expression callee;
    std::vector<Expression> arguments;
};

struct _IdentifierExpression : _Expression {
    _8b_visitable( _IdentifierExpression );
    std::string identifier;
};

struct _InstanceExpression : _Expression {
    _8b_visitable( _InstanceExpression );
};

struct _IntegerConstant : _Expression {
    _8b_visitable( _IntegerConstant );
    int value;
};

struct _MemberAccessExpression : _Expression {
    _8b_visitable( _MemberAccessExpression );
    Expression object;
    std::string memberIdentifier;
};

struct _StringConstant : _Expression {
    _8b_visitable( _StringConstant );
    std::string value;
};

struct _UnaryOperationExpression : _Expression {
    _8b_visitable( _UnaryOperationExpression );
    UnaryOperation operation;
    Expression operand;
};

// ----------------------------------------------------------------------------
//  Types
// ----------------------------------------------------------------------------

struct _Type : _Node {};

struct _BooleanType : _Type {
    _8b_visitable( _BooleanType );
};

struct _IntegerType : _Type {
    _8b_visitable( _IntegerType );
};

struct _NamedType : _Type {
    _8b_visitable( _NamedType );
    std::string identifier;
};

struct _PointerType : _Type {
    _8b_visitable( _PointerType );
    Type targetType;
};

struct _StringType : _Type {
    _8b_visitable( _StringType );
};

}
}
