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


extern llvm::LLVMContext &globalLLVMContext = llvm::getGlobalContext();
extern llvm::IRBuilder<> irBuilder( globalLLVMContext );

llvm::Type *int32 = llvm::IntegerType::get( globalLLVMContext, 32 );


std::shared_ptr<llvm::Module> CodeGenerator::generate( const ast::Module &module ) {

    std::shared_ptr<llvm::Module> llvmModule = std::make_shared<llvm::Module>(
        "test_module", globalLLVMContext );

    _symbolTable.pushLexicalScope();

    for( auto &it : module.getFunctions() ) {
        generate( it, llvmModule.get() );
    }

    _symbolTable.popLexicalScope();

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

    _symbolTable.addSymbol( function.getIdentifier(), FunctionValue::create(_llvmFunction) );

    _symbolTable.pushLexicalScope();

    for( llvm::Function::arg_iterator it = _llvmFunction->arg_begin(); it != _llvmFunction->arg_end(); ++it ) {
        size_t index = std::distance( _llvmFunction->arg_begin(), it );

        llvm::Value *value = it;
        value->setName( arguments[index].identifier );
        _symbolTable.addSymbol( arguments[index].identifier, IntegerValue::create(value) );
    }

    generate( function.getBlockStatement() );

    _symbolTable.popLexicalScope();
}

void CodeGenerator::generate( const ast::BlockStatement &blockStatement ) {

    llvm::BasicBlock *basicBlock = insertBasicBlock( "entry" );

    generate( blockStatement, basicBlock );
}

void CodeGenerator::generate( const ast::BlockStatement &blockStatement, llvm::BasicBlock *basicBlock ) {

    irBuilder.SetInsertPoint( basicBlock );

    _symbolTable.pushLexicalScope();

    for( auto &it : blockStatement.getStatements() )
        generate( it );

    _symbolTable.popLexicalScope();
}


void CodeGenerator::generate( ast::StatementPointer statement ) {

#define _8b_generate_statement( TYPE )                          \
    if( statement->instanceOf<TYPE>() ) {                       \
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

    llvm::Value *condition = generate( statement.getConditionExpression() )->generateToBoolean()->getLlvmValue();

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
        irBuilder.CreateRet( generate(expression)->getLlvmValue() );
}

void CodeGenerator::generate( const ast::VariableDeclarationStatement &statement ) {

    ValuePointer variable = IntegerValue::create( statement.getIdentifier() );

    _symbolTable.addSymbol( statement.getIdentifier(), variable );

    ast::ExpressionPointer initializerExpression = statement.getInitializerExpression();

    if( initializerExpression )
        variable->generateAssignment( generate(initializerExpression) );

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
    llvm::Value *condition = generate( statement.getConditionExpression() )->generateToBoolean()->getLlvmValue();
    irBuilder.CreateCondBr( condition, loopCode, loopEnd );

    irBuilder.SetInsertPoint( loopEnd );
}

ValuePointer CodeGenerator::generate( ast::ExpressionPointer expression ) {

#define _8b_generate_expression( TYPE )                                 \
    if( expression->instanceOf<TYPE>() ) {                              \
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

ValuePointer CodeGenerator::generate( const ast::IdentifierExpression &expression ) {
    return _symbolTable.lookupSymbol( expression.getIdentifier() );
}

ValuePointer CodeGenerator::generate( const ast::IntegerConstantExpression &expression ) {
    return IntegerValue::create( expression.getValue() );
}

ValuePointer CodeGenerator::generate( const ast::AdditionExpression &expression ) {
    ValuePointer leftValue = generate( expression.getLeftOperand() );
    ValuePointer rightValue = generate( expression.getRightOperand() );
    return leftValue->generateAdd( rightValue );
}

ValuePointer CodeGenerator::generate( const ast::SubtractionExpression &expression ) {
    ValuePointer leftValue = generate( expression.getLeftOperand() );
    ValuePointer rightValue = generate( expression.getRightOperand() );
    return leftValue->generateSubtract( rightValue );
}

ValuePointer CodeGenerator::generate( const ast::MultiplicationExpression &expression ) {
    ValuePointer leftValue = generate( expression.getLeftOperand() );
    ValuePointer rightValue = generate( expression.getRightOperand() );
    return leftValue->generateMultiply( rightValue );
}

ValuePointer CodeGenerator::generate( const ast::DivisionExpression &expression ) {
    ValuePointer leftValue = generate( expression.getLeftOperand() );
    ValuePointer rightValue = generate( expression.getRightOperand() );
    return leftValue->generateDivide( rightValue );
}

ValuePointer CodeGenerator::generate( const ast::LogicAndExpression &expression ) {
    ValuePointer leftValue = generate( expression.getLeftOperand() );
    ValuePointer rightValue = generate( expression.getRightOperand() );
    return leftValue->generateAnd( rightValue );
}

ValuePointer CodeGenerator::generate( const ast::LogicOrExpression &expression ) {
    ValuePointer leftValue = generate( expression.getLeftOperand() );
    ValuePointer rightValue = generate( expression.getRightOperand() );
    return leftValue->generateOr( rightValue );
}

ValuePointer CodeGenerator::generate( const ast::LessExpression &expression ) {
    ValuePointer leftValue = generate( expression.getLeftOperand() );
    ValuePointer rightValue = generate( expression.getRightOperand() );
    return leftValue->generateLess( rightValue );
}

ValuePointer CodeGenerator::generate( const ast::GreaterExpression &expression ) {
    ValuePointer leftValue = generate( expression.getLeftOperand() );
    ValuePointer rightValue = generate( expression.getRightOperand() );
    return leftValue->generateGreater( rightValue );
}

ValuePointer CodeGenerator::generate( const ast::CallExpression &expression ) {

    ast::ExpressionPointer callee = expression.getCallee();

    std::vector<ValuePointer> arguments( expression.getArguments().size() );

    for( size_t i = 0; i < arguments.size(); ++i ) {
        arguments[i] = generate( expression.getArguments()[i] );
    }

    return generate( callee )->generateCall( arguments );
}

void CodeGenerator::generateVoid( ast::ExpressionPointer expression ) {

#define _8b_generate_void_expression( TYPE )                         \
    if( expression->instanceOf<TYPE>() ) {                           \
        generateVoid( *std::static_pointer_cast<TYPE>(expression) ); \
        return;                                                      \
    }

    _8b_generate_void_expression( ast::AssignmentExpression );
    _8b_generate_void_expression( ast::IncrementExpression );
    _8b_generate_void_expression( ast::DecrementExpression );

    generate( expression );
}

void CodeGenerator::generateVoid( const ast::AssignmentExpression &expression ) {
    ValuePointer leftValue = generate( expression.getLeftOperand() );
    ValuePointer rightValue = generate( expression.getRightOperand() );
    leftValue->generateAssignment( rightValue );
}

void CodeGenerator::generateVoid( const ast::IncrementExpression &expression ) {
    ValuePointer leftValue = generate( expression.getOperand() );
    leftValue->generateIncrement();
}

void CodeGenerator::generateVoid( const ast::DecrementExpression &expression ) {
    ValuePointer leftValue = generate( expression.getOperand() );
    leftValue->generateDecrement();
}

llvm::BasicBlock* CodeGenerator::insertBasicBlock( const std::string &name ) {
    return llvm::BasicBlock::Create( globalLLVMContext, name, _llvmFunction );
}

}
