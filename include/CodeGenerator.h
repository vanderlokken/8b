#pragma once

#include <llvm/BasicBlock.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Value.h>

#include "Module.h"
#include "SymbolTable.h"

namespace _8b {

class CodeGenerator {
public:
    std::shared_ptr<llvm::Module> generate( const ast::Module &module );

private:
    void generate( const ast::Function&, llvm::Module* );

    void generate( ast::StatementPointer );
    void generate( const ast::BlockStatement& );
    void generate( const ast::BlockStatement&, llvm::BasicBlock* );
    void generate( const ast::ExpressionStatement& );
    void generate( const ast::IfStatement& );
    void generate( const ast::ReturnStatement& );
    void generate( const ast::VariableDeclarationStatement& );
    void generate( const ast::WhileStatement& );

    ValuePointer generate( ast::ExpressionPointer );
    ValuePointer generate( const ast::IdentifierExpression& );
    ValuePointer generate( const ast::IntegerConstantExpression& );
    ValuePointer generate( const ast::BooleanConstantExpression& );
    ValuePointer generate( const ast::AdditionExpression& );
    ValuePointer generate( const ast::SubtractionExpression& );
    ValuePointer generate( const ast::MultiplicationExpression& );
    ValuePointer generate( const ast::DivisionExpression& );
    ValuePointer generate( const ast::LogicAndExpression& );
    ValuePointer generate( const ast::LogicOrExpression& );
    ValuePointer generate( const ast::LessExpression& );
    ValuePointer generate( const ast::GreaterExpression& );
    ValuePointer generate( const ast::CallExpression& );

    void generateVoid( ast::ExpressionPointer );
    void generateVoid( const ast::AssignmentExpression& );
    void generateVoid( const ast::IncrementExpression& );
    void generateVoid( const ast::DecrementExpression& );

    llvm::BasicBlock* insertBasicBlock( const std::string& );

    llvm::Function *_llvmFunction;
    SymbolTable _symbolTable;
};

}
