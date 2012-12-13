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
    void generate( const ast::Class& );
    void generate( const ast::Function&, llvm::Module* );

    void generate( ast::StatementPointer );
    void generate( const ast::BlockStatement&, llvm::BasicBlock* );
    void generate( const ast::ExpressionStatement& );
    void generate( const ast::IfStatement& );
    void generate( const ast::ReturnStatement& );
    void generate( const ast::VariableDeclarationStatement& );
    void generate( const ast::WhileStatement& );

    ValuePointer generate( ast::ExpressionPointer );
    ValuePointer generate( const ast::IdentifierExpression& );
    ValuePointer generate( const ast::MemberAccessExpression& );
    ValuePointer generate( const ast::IntegerConstantExpression& );
    ValuePointer generate( const ast::BooleanConstantExpression& );
    ValuePointer generate( const ast::StringConstantExpression& );
    ValuePointer generate( const ast::UnaryOperationExpression& );
    ValuePointer generate( const ast::BinaryOperationExpression& );
    ValuePointer generate( const ast::CallExpression& );

    llvm::BasicBlock* insertBasicBlock( const std::string& );

    ValueTypePointer valueTypeByAstType( ast::TypePointer );

    SymbolTable _symbolTable;
};

}
