#pragma once

#include <memory>

#include "ClassId.h"
#include "Expression.h"
#include "LexicalAnalyser.h"
#include "Type.h"

namespace _8b {
namespace ast{


struct Statement;
typedef std::shared_ptr<Statement> StatementPointer;


struct Statement : public BaseIdClass {
    static StatementPointer parse( LexicalAnalyser& );
};

template<class T>
struct StatementType : public DerivedIdClass<Statement, T> {};


struct BlockStatement : public StatementType<BlockStatement> {
    
    BlockStatement() {};
    BlockStatement( LexicalAnalyser& );

    std::vector<StatementPointer> statements;
};


struct ExpressionStatement : public StatementType<ExpressionStatement> {
    
    ExpressionStatement( LexicalAnalyser& );

    ExpressionPointer expression;
};


struct IfStatement : public StatementType<IfStatement> {
    
    IfStatement( LexicalAnalyser& );

    ExpressionPointer conditionExpression;
    BlockStatement trueBlockStatement;
    BlockStatement falseBlockStatement;
};


struct ReturnStatement : public StatementType<ReturnStatement> {

    ReturnStatement( LexicalAnalyser& );

    ExpressionPointer expression;
};


struct VariableDeclarationStatement : public StatementType<VariableDeclarationStatement> {

    VariableDeclarationStatement( LexicalAnalyser& );

    std::string identifier;
    TypePointer type;
    ExpressionPointer initializerExpression;
};


struct WhileStatement : public StatementType<WhileStatement> {

    WhileStatement( LexicalAnalyser& );

    ExpressionPointer conditionExpression;
    BlockStatement blockStatement;
};


}
}
