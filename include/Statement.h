#pragma once

#include <memory>

#include "Expression.h"
#include "LexicalAnalyser.h"
#include "Type.h"

namespace _8b {
namespace ast{


class Statement;
typedef std::shared_ptr<Statement> StatementPointer;


class Statement {
public:

    Statement( size_t typeId );

    size_t getTypeId() const;

    static StatementPointer parse( LexicalAnalyser& );

protected:
    size_t _typeId;
};


template< class T >
class StatementType : public Statement {
public:
    StatementType() : Statement( typeId ) {}
    static const size_t typeId;
};

template< class T >
const size_t StatementType<T>::typeId = (size_t)( typeid(T).raw_name() );


class BlockStatement : public StatementType<BlockStatement> {
public:
    BlockStatement() {};
    BlockStatement( LexicalAnalyser& );

    const std::vector<StatementPointer>& getStatements() const;

private:
    std::vector<StatementPointer> _statements;
};


class ExpressionStatement : public StatementType<ExpressionStatement> {
public:
    ExpressionStatement( LexicalAnalyser& );

    ExpressionPointer getExpression() const;

private:
    ExpressionPointer _expression;
};


class IfStatement : public StatementType<IfStatement> {
public:
    IfStatement( LexicalAnalyser& );

    ExpressionPointer getConditionExpression() const;
    const BlockStatement& getTrueBlockStatement() const;
    const BlockStatement& getFalseBlockStatement() const;

private:
    ExpressionPointer _conditionExpression;
    BlockStatement _trueBlockStatement;
    BlockStatement _falseBlockStatement;
};


class ReturnStatement : public StatementType<ReturnStatement> {
public:
    ReturnStatement( LexicalAnalyser& );

    ExpressionPointer getExpression() const;

private:
    ExpressionPointer _expression;
};


class VariableDeclarationStatement : public StatementType<VariableDeclarationStatement> {
public:
    VariableDeclarationStatement( LexicalAnalyser& );

    const std::string& getIdentifier() const;
    ExpressionPointer getInitializerExpression() const;

private:
    std::string _identifier;
    std::shared_ptr<Type> _type;
    ExpressionPointer _initializerExpression;
};


class WhileStatement : public StatementType<WhileStatement> {
public:
    WhileStatement( LexicalAnalyser& );

    ExpressionPointer getConditionExpression() const;
    const BlockStatement& getBlockStatement() const;

private:
    ExpressionPointer _conditionExpression;
    BlockStatement _blockStatement;
};


}
}
