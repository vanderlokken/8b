#pragma once

#include <memory>

#include "ClassId.h"
#include "Expression.h"
#include "LexicalAnalyser.h"
#include "Type.h"

namespace _8b {
namespace ast{


class Statement;
typedef std::shared_ptr<Statement> StatementPointer;


class Statement : public BaseIdClass {
public:
    static StatementPointer parse( LexicalAnalyser& );
};

template<class T>
class StatementType : public DerivedIdClass<Statement, T> {};


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
    TypePointer getType() const;
    ExpressionPointer getInitializerExpression() const;

private:
    std::string _identifier;
    TypePointer _type;
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
