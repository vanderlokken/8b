#include "CodeGenerator.h"

#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Type.h>

#include "Exception.h"

namespace _8b {


static llvm::LLVMContext &globalLLVMContext = llvm::getGlobalContext();
static llvm::IRBuilder<> irBuilder( globalLLVMContext );

llvm::Type *int32 = llvm::IntegerType::get( globalLLVMContext, 32 );


std::shared_ptr<llvm::Module> CodeGenerator::generate( const ast::Module &module ) {

    std::shared_ptr<llvm::Module> llvmModule = std::make_shared<llvm::Module>(
        "test_module", globalLLVMContext );

    for( auto &it : module.getFunctions() ) {
        generate( it, llvmModule.get() );
    }

    return llvmModule;
}

void CodeGenerator::generate( const ast::Function &function, llvm::Module *module ) {

    const std::vector<ast::Function::Argument> &arguments = function.getArguments();

    const std::vector<llvm::Type*> argumentTypes( arguments.size(), int32 );
    llvm::Type *returnType = int32;

    const bool isVariableArgument = false;

    llvm::FunctionType *type = llvm::FunctionType::get(
        returnType, argumentTypes, isVariableArgument);

    _llvmFunction = llvm::Function::Create(
        type, llvm::Function::ExternalLinkage, function.getIdentifier(), module );

    _symbolTable.clear();

    for( llvm::Function::arg_iterator it = _llvmFunction->arg_begin(); it != _llvmFunction->arg_end(); ++it ) {
        size_t index = std::distance( _llvmFunction->arg_begin(), it );

        llvm::Value *value = it;
        value->setName( arguments[index].identifier );
        _symbolTable[arguments[index].identifier] = value;
    }

    generate( function.getBlockStatement() );
}

void CodeGenerator::generate( const ast::BlockStatement &blockStatement ) {

    llvm::BasicBlock *basicBlock = insertBasicBlock( "entry" );

    generate( blockStatement, basicBlock );
}

void CodeGenerator::generate( const ast::BlockStatement &blockStatement, llvm::BasicBlock *basicBlock ) {

    irBuilder.SetInsertPoint( basicBlock );

    std::map<std::string, llvm::Value*> storedSymbolTable( _symbolTable );

    for( auto &it : blockStatement.getStatements() )
        generate( it );

    _symbolTable = storedSymbolTable;
}


void CodeGenerator::generate( ast::StatementPointer statement ) {

    const size_t typeId = statement->getTypeId();

#define _8b_generate_statement( TYPE )                          \
    if( typeId == TYPE::typeId ) {                              \
        generate( *std::static_pointer_cast<TYPE>(statement) ); \
        return;                                                 \
    }

    _8b_generate_statement( ast::ExpressionStatement );
    _8b_generate_statement( ast::IfStatement );
    _8b_generate_statement( ast::ReturnStatement );
    _8b_generate_statement( ast::WhileStatement );
    _8b_generate_statement( ast::VariableDeclarationStatement );

    throwRuntimeError( "Not implemented" );
}

void CodeGenerator::generate( const ast::ExpressionStatement &statement ) {
    generateVoid( statement.getExpression() );
}

void CodeGenerator::generate( const ast::IfStatement &statement ) {

    const bool hasFalseBlock = statement.getFalseBlockStatement().getStatements().empty();

    llvm::BasicBlock *trueBlock = insertBasicBlock( "ifTrue" );
    llvm::BasicBlock *falseBlock = hasFalseBlock ? insertBasicBlock( "ifFalse" ) : 0;
    llvm::BasicBlock *nextCodeBlock = insertBasicBlock( "endif" );

    llvm::Value *condition = toBoolean( generate(statement.getConditionExpression()) );

    irBuilder.CreateCondBr(
        condition, trueBlock, hasFalseBlock ? falseBlock : nextCodeBlock );

    generate( statement.getTrueBlockStatement(), trueBlock );
    irBuilder.CreateBr( nextCodeBlock );

    if( hasFalseBlock ) {
        generate( statement.getFalseBlockStatement(), falseBlock );
        irBuilder.CreateBr( nextCodeBlock );
    }

    irBuilder.SetInsertPoint( nextCodeBlock );
}

void CodeGenerator::generate( const ast::ReturnStatement &statement ) {
    
    ast::ExpressionPointer expression = statement.getExpression();

    if( !expression )
        irBuilder.CreateRetVoid();
    else
        irBuilder.CreateRet( generate(expression) );
}

void CodeGenerator::generate( const ast::VariableDeclarationStatement &statement ) {
    llvm::Value *variable = irBuilder.CreateAlloca( int32 );

    variable->setName( statement.getIdentifier() );

    _symbolTable[statement.getIdentifier()] = variable;

    ast::ExpressionPointer initializerExpression = statement.getInitializerExpression();

    if( initializerExpression )
        irBuilder.CreateStore( generate(initializerExpression), variable );

    return;
}

void CodeGenerator::generate( const ast::WhileStatement &statement ) {

    llvm::BasicBlock *loopStart = insertBasicBlock( "loopStart" );
    llvm::BasicBlock *loopCode = insertBasicBlock( "loopCode" );
    llvm::BasicBlock *loopEnd = insertBasicBlock( "loopEnd" );

    irBuilder.CreateBr( loopStart );

    generate( statement.getBlockStatement(), loopCode );
    irBuilder.CreateBr( loopStart );

    irBuilder.SetInsertPoint( loopStart );
    llvm::Value *condition = toBoolean( generate(statement.getConditionExpression()) );
    irBuilder.CreateCondBr( condition, loopCode, loopEnd );

    irBuilder.SetInsertPoint( loopEnd );
}

llvm::Value* CodeGenerator::generate( ast::ExpressionPointer expression ) {

    const auto typeId = expression->getTypeId();

#define _8b_generate_expression( TYPE )                                 \
    if( typeId == TYPE::typeId ) {                                      \
        return generate( *std::static_pointer_cast<TYPE>(expression) ); \
    }

    _8b_generate_expression( ast::IdentifierExpression );
    _8b_generate_expression( ast::IntegerConstantExpression );
    _8b_generate_expression( ast::AdditionExpression );
    _8b_generate_expression( ast::SubtractionExpression );
    _8b_generate_expression( ast::MultiplicationExpression );
    _8b_generate_expression( ast::DivisionExpression );
    _8b_generate_expression( ast::LogicAndExpression );
    _8b_generate_expression( ast::LogicOrExpression );
    _8b_generate_expression( ast::LessExpression );
    _8b_generate_expression( ast::GreaterExpression );
    _8b_generate_expression( ast::CallExpression );

    throwRuntimeError( "An expression has no return value or not implemented" );
}

llvm::Value* CodeGenerator::generate( const ast::IdentifierExpression &expression ) {
    
    llvm::Value *reference = generateReference( expression );

    if( llvm::AllocaInst::classof(reference) )
        return irBuilder.CreateLoad( reference );
    else
        return reference;
}

llvm::Value* CodeGenerator::generate( const ast::IntegerConstantExpression &expression ) {
    return llvm::ConstantInt::get( globalLLVMContext, llvm::APInt(32, expression.getValue(), false) );
}

llvm::Value* CodeGenerator::generate( const ast::AdditionExpression &expression ) {
    return irBuilder.CreateAdd(
        generate(expression.getLeftOperand()),
        generate(expression.getRightOperand()) );
}

llvm::Value* CodeGenerator::generate( const ast::SubtractionExpression &expression ) {
    return irBuilder.CreateSub(
        generate(expression.getLeftOperand()),
        generate(expression.getRightOperand()) );
}

llvm::Value* CodeGenerator::generate( const ast::MultiplicationExpression &expression ) {
    return irBuilder.CreateMul(
        generate(expression.getLeftOperand()),
        generate(expression.getRightOperand()) );
}

llvm::Value* CodeGenerator::generate( const ast::DivisionExpression &expression ) {
    return irBuilder.CreateSDiv(
        generate(expression.getLeftOperand()),
        generate(expression.getRightOperand()) );
}

llvm::Value* CodeGenerator::generate( const ast::LogicAndExpression &expression ) {
    return irBuilder.CreateAnd(
        toBoolean(generate(expression.getLeftOperand())),
        toBoolean(generate(expression.getRightOperand())) );
}

llvm::Value* CodeGenerator::generate( const ast::LogicOrExpression &expression ) {
    return irBuilder.CreateOr(
        toBoolean(generate(expression.getLeftOperand())),
        toBoolean(generate(expression.getRightOperand())) );
}

llvm::Value* CodeGenerator::generate( const ast::LessExpression &expression ) {
    return irBuilder.CreateICmpULT(
        generate(expression.getLeftOperand()),
        generate(expression.getRightOperand()) );
}

llvm::Value* CodeGenerator::generate( const ast::GreaterExpression &expression ) {
    return irBuilder.CreateICmpUGT(
        generate(expression.getLeftOperand()),
        generate(expression.getRightOperand()) );
}

llvm::Value* CodeGenerator::generate( const ast::CallExpression &expression ) {

    ast::ExpressionPointer callee = expression.getCallee();

    if( callee->getTypeId() != ast::IdentifierExpression::typeId )
        throwRuntimeError( "Not implemented" );

    llvm::Function *calleeFunction = _llvmFunction->getParent()->getFunction(
        std::static_pointer_cast<ast::IdentifierExpression>(callee)->getIdentifier() );

    std::vector<llvm::Value*> arguments( expression.getArguments().size() );
    for( size_t i = 0; i < arguments.size(); ++i ) {
        arguments[i] = generate( expression.getArguments()[i] );
    }

    return irBuilder.CreateCall( calleeFunction, arguments );
}

void CodeGenerator::generateVoid( ast::ExpressionPointer expression ) {

    const auto typeId = expression->getTypeId();

#define _8b_generate_void_expression( TYPE )                         \
    if( typeId == TYPE::typeId ) {                                   \
        generateVoid( *std::static_pointer_cast<TYPE>(expression) ); \
        return;                                                      \
    }

    _8b_generate_void_expression( ast::AssignmentExpression );
    _8b_generate_void_expression( ast::IncrementExpression );
    _8b_generate_void_expression( ast::DecrementExpression );

    generate( expression );
}

void CodeGenerator::generateVoid( const ast::AssignmentExpression &expression ) {
    irBuilder.CreateStore(
        generate(expression.getRightOperand()),
        generateReference(expression.getLeftOperand()));
}

void CodeGenerator::generateVoid( const ast::IncrementExpression &expression ) {
    llvm::Value *operand = generateReference( expression.getOperand() );
    llvm::Value *operandValue = irBuilder.CreateLoad( operand );
    llvm::Value *one = llvm::ConstantInt::get( operandValue->getType(), 1 );
    irBuilder.CreateStore( irBuilder.CreateAdd(operandValue, one), operand );
}

void CodeGenerator::generateVoid( const ast::DecrementExpression &expression ) {
    llvm::Value *operand = generateReference( expression.getOperand() );
    llvm::Value *operandValue = irBuilder.CreateLoad( operand );
    llvm::Value *one = llvm::ConstantInt::get( operandValue->getType(), 1 );
    irBuilder.CreateStore( irBuilder.CreateSub(operandValue, one), operand );
}

llvm::Value* CodeGenerator::generateReference( ast::ExpressionPointer expression ) {

    if( expression->getTypeId() == ast::IdentifierExpression::typeId )
        return generateReference( *std::static_pointer_cast<ast::IdentifierExpression>(expression) );

    throwRuntimeError( "Cannot reference expression or not implemented" );
}

llvm::Value* CodeGenerator::generateReference( const ast::IdentifierExpression &expression ) {

    auto it = _symbolTable.find( expression.getIdentifier() );

    if( it == _symbolTable.end() )
        throwRuntimeError( "Undeclared identifier" );

    return it->second;
}

llvm::Value* CodeGenerator::toBoolean( llvm::Value *value ) {

    if( value->getType()->isIntegerTy(1) )
        return value;

    if( value->getType()->isIntegerTy() )
        return irBuilder.CreateIsNotNull( value );

    throwRuntimeError( "A value cannot be represented as boolean or not implemented" );
}

llvm::BasicBlock* CodeGenerator::insertBasicBlock( const std::string &name ) {
    return llvm::BasicBlock::Create( globalLLVMContext, name, _llvmFunction );
}

}
