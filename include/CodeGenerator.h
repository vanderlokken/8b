#pragma once

#include <map>

#include <llvm/BasicBlock.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Value.h>

#include "Module.h"

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

    llvm::Value* generate( ast::ExpressionPointer );
    llvm::Value* generate( const ast::IdentifierExpression& );
    llvm::Value* generate( const ast::IntegerConstantExpression& );
    llvm::Value* generate( const ast::AdditionExpression& );
    llvm::Value* generate( const ast::SubtractionExpression& );
    llvm::Value* generate( const ast::MultiplicationExpression& );
    llvm::Value* generate( const ast::DivisionExpression& );
    llvm::Value* generate( const ast::LogicAndExpression& );
    llvm::Value* generate( const ast::LogicOrExpression& );
    llvm::Value* generate( const ast::LessExpression& );
    llvm::Value* generate( const ast::GreaterExpression& );
    llvm::Value* generate( const ast::CallExpression& );

    void generateVoid( ast::ExpressionPointer );
    void generateVoid( const ast::AssignmentExpression& );
    void generateVoid( const ast::IncrementExpression& );
    void generateVoid( const ast::DecrementExpression& );

    llvm::Value* generateReference( ast::ExpressionPointer );
    llvm::Value* generateReference( const ast::IdentifierExpression& );

    llvm::Value* toBoolean( llvm::Value* );

    llvm::BasicBlock* insertBasicBlock( const std::string& );

    llvm::Function *_llvmFunction;
    std::map<std::string, llvm::Value*> _symbolTable;
};

}
