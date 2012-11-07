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

    std::vector<ValueTypePointer> argumentTypes( arguments.size() );
    
    std::transform(
        arguments.cbegin(),
        arguments.cend(),
        argumentTypes.begin(),
        []( const ast::Function::Argument &argument ) -> ValueTypePointer {
            return CodeGenerator::valueTypeByAstType( argument.type );
        });

    ValueTypePointer functionType;

    if( function.getReturnType() )
        functionType = std::make_shared<FunctionType>(
            argumentTypes, CodeGenerator::valueTypeByAstType(function.getReturnType()) );
    else
        functionType = std::make_shared<FunctionType>( argumentTypes );

    _llvmFunction = llvm::Function::Create(
        static_cast<llvm::FunctionType*>(functionType->toLlvm()), llvm::Function::ExternalLinkage, function.getIdentifier(), module );

    ValuePointer functionValue = Value::createSsaValue( functionType, _llvmFunction );

    _symbolTable.addSymbol( function.getIdentifier(), functionValue );

    _symbolTable.pushLexicalScope();

    for( llvm::Function::arg_iterator it = _llvmFunction->arg_begin(); it != _llvmFunction->arg_end(); ++it ) {
        
        size_t index = std::distance( _llvmFunction->arg_begin(), it );

        llvm::Value *value = it;
        value->setName( arguments[index].identifier );

        _symbolTable.addSymbol( arguments[index].identifier,
            Value::createSsaValue(argumentTypes[index], value) );
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
    generate( statement.getExpression() );
}

void CodeGenerator::generate( const ast::IfStatement &statement ) {

    const bool hasFalseBlock = statement.getFalseBlockStatement().getStatements().empty();

    llvm::BasicBlock *trueBlock = insertBasicBlock( "ifTrue" );
    llvm::BasicBlock *falseBlock = hasFalseBlock ? insertBasicBlock( "ifFalse" ) : 0;
    llvm::BasicBlock *nextCodeBlock = insertBasicBlock( "endif" );

    llvm::Value *condition = generate( statement.getConditionExpression() )->toBoolean()->getLlvmValue();

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

    ValuePointer variable;

    ast::TypePointer type = statement.getType();
    ast::ExpressionPointer initializerExpression = statement.getInitializerExpression();

    const std::string &identifier = statement.getIdentifier();

    if( type ) {

        variable = Value::createVariable( valueTypeByAstType(type), identifier );
    
    } else if( initializerExpression ) {

        ValuePointer initializerValue = generate( initializerExpression );
        variable = Value::createVariable( initializerValue->getType(), identifier );
        variable->generateBinaryOperation( BinaryOperation::Assignment, initializerValue );

    } else {
        throwRuntimeError( "A variable declaration statement doesn't contain"
            " neither type identifier nor initializer expression" );
    }

    _symbolTable.addSymbol( identifier, variable );

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
    llvm::Value *condition = generate( statement.getConditionExpression() )->toBoolean()->getLlvmValue();
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
    _8b_generate_expression( ast::BooleanConstantExpression );
    _8b_generate_expression( ast::UnaryOperationExpression );
    _8b_generate_expression( ast::BinaryOperationExpression );
    _8b_generate_expression( ast::CallExpression );

    throwRuntimeError( "Not implemented" );
}

ValuePointer CodeGenerator::generate( const ast::IdentifierExpression &expression ) {
    return _symbolTable.lookupSymbol( expression.getIdentifier() );
}

ValuePointer CodeGenerator::generate( const ast::IntegerConstantExpression &expression ) {
    return Value::createIntegerConstant( expression.getValue() );
}

ValuePointer CodeGenerator::generate( const ast::BooleanConstantExpression &expression ) {
    return Value::createBooleanConstant( expression.getValue() );
}

ValuePointer CodeGenerator::generate( const ast::UnaryOperationExpression &expression ) {
    ValuePointer operand = generate( expression.getOperand() );
    return operand->generateUnaryOperation( expression.getOperation() );
}

ValuePointer CodeGenerator::generate( const ast::BinaryOperationExpression &expression ) {
    ValuePointer leftValue = generate( expression.getLeftOperand() );
    ValuePointer rightValue = generate( expression.getRightOperand() );
    return leftValue->generateBinaryOperation( expression.getOperation(), rightValue );
}

ValuePointer CodeGenerator::generate( const ast::CallExpression &expression ) {

    ValuePointer callee = generate( expression.getCallee() );

    std::vector<ValuePointer> arguments( expression.getArguments().size() );

    std::transform(
        expression.getArguments().cbegin(),
        expression.getArguments().cend(),
        arguments.begin(),
        [&]( ast::ExpressionPointer expression ) -> ValuePointer {
            return CodeGenerator::generate( expression );
        });

    return callee->generateCall( arguments );
}

llvm::BasicBlock* CodeGenerator::insertBasicBlock( const std::string &name ) {
    return llvm::BasicBlock::Create( globalLLVMContext, name, _llvmFunction );
}

ValueTypePointer CodeGenerator::valueTypeByAstType( ast::TypePointer astType ) {
    if( astType->instanceOf<ast::IntegerType>() )
        return IntegerType::get();
    if( astType->instanceOf<ast::BooleanType>() )
        return BooleanType::get();
}

}
