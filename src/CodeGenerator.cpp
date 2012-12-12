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

    _symbolTable.enterLexicalScope();

    for( auto &it : module.classes )
        generate( it );

    for( auto &it : module.functions )
        generate( it, llvmModule.get() );

    _symbolTable.leaveLexicalScope();

    return llvmModule;
}

void CodeGenerator::generate( const ast::Class &classDeclaration ) {

    std::vector<ClassType::Member> members( classDeclaration.variables.size() );

    std::transform(
        classDeclaration.variables.cbegin(),
        classDeclaration.variables.cend(),
        members.begin(),
        [this]( const ast::VariableDeclarationStatement &statement ) -> ClassType::Member {
            
            if( statement.initializerExpression )
                throwRuntimeError( "Not implemented" );

            ClassType::Member member;
            member.identifier = statement.identifier;
            member.type = valueTypeByAstType( statement.type );
            return member;
        });

    ValueTypePointer type = std::make_shared<ClassType>( members );

    _symbolTable.addType( classDeclaration.identifier, type );
}

void CodeGenerator::generate( const ast::Function &function, llvm::Module *module ) {

    std::vector<ValueTypePointer> argumentTypes( function.arguments.size() );
    
    std::transform(
        function.arguments.cbegin(),
        function.arguments.cend(),
        argumentTypes.begin(),
        [this]( const ast::Function::Argument &argument ) -> ValueTypePointer {
            return valueTypeByAstType( argument.type );
        });

    ValueTypePointer returnType =
        function.returnType ? valueTypeByAstType(function.returnType) : nullptr;

    ValueTypePointer functionType = std::make_shared<FunctionType>( argumentTypes, returnType );

    llvm::Function *llvmFunction = llvm::Function::Create(
        static_cast<llvm::FunctionType*>(functionType->toLlvm()), llvm::Function::ExternalLinkage, function.identifier, module );

    ValuePointer functionValue = Value::createSsaValue( functionType, llvmFunction );

    _symbolTable.addValue( function.identifier, functionValue );

    _symbolTable.enterLexicalScope();

    for( llvm::Function::arg_iterator it = llvmFunction->arg_begin(); it != llvmFunction->arg_end(); ++it ) {
        
        size_t index = std::distance( llvmFunction->arg_begin(), it );

        llvm::Value *value = it;
        value->setName( function.arguments[index].identifier );

        _symbolTable.addValue( function.arguments[index].identifier,
            Value::createSsaValue(argumentTypes[index], value) );
    }

    generate( function.blockStatement, llvm::BasicBlock::Create(globalLLVMContext, "entry", llvmFunction) );

    _symbolTable.leaveLexicalScope();
}

void CodeGenerator::generate( const ast::BlockStatement &blockStatement, llvm::BasicBlock *basicBlock ) {

    irBuilder.SetInsertPoint( basicBlock );

    _symbolTable.enterLexicalScope();

    for( auto &it : blockStatement.statements )
        generate( it );

    _symbolTable.leaveLexicalScope();
}

void CodeGenerator::generate( ast::StatementPointer statement ) {

#define _8b_generate_statement( TYPE )                                \
    if( statement->instanceOf<TYPE>() ) {                             \
        generate( *std::static_pointer_cast<const TYPE>(statement) ); \
        return;                                                       \
    }

    _8b_generate_statement( ast::ExpressionStatement );
    _8b_generate_statement( ast::IfStatement );
    _8b_generate_statement( ast::ReturnStatement );
    _8b_generate_statement( ast::WhileStatement );
    _8b_generate_statement( ast::VariableDeclarationStatement );

    throwRuntimeError( "Not implemented" );
}

void CodeGenerator::generate( const ast::ExpressionStatement &statement ) {
    generate( statement.expression );
}

void CodeGenerator::generate( const ast::IfStatement &statement ) {

    const bool hasFalseBlock = statement.falseBlockStatement.statements.empty();

    llvm::BasicBlock *trueBlock = insertBasicBlock( "ifTrue" );
    llvm::BasicBlock *falseBlock = hasFalseBlock ? insertBasicBlock( "ifFalse" ) : 0;
    llvm::BasicBlock *nextCodeBlock = insertBasicBlock( "endif" );

    llvm::Value *condition = generate( statement.conditionExpression )->toBoolean()->toLlvm();

    irBuilder.CreateCondBr(
        condition, trueBlock, hasFalseBlock ? falseBlock : nextCodeBlock );

    generate( statement.trueBlockStatement, trueBlock );
    irBuilder.CreateBr( nextCodeBlock );

    if( hasFalseBlock ) {
        generate( statement.falseBlockStatement, falseBlock );
        irBuilder.CreateBr( nextCodeBlock );
    }

    irBuilder.SetInsertPoint( nextCodeBlock );
}

void CodeGenerator::generate( const ast::ReturnStatement &statement ) {
    if( !statement.expression )
        irBuilder.CreateRetVoid();
    else
        irBuilder.CreateRet( generate(statement.expression)->toLlvm() );
}

void CodeGenerator::generate( const ast::VariableDeclarationStatement &statement ) {

    ValuePointer variable;

    if( statement.type ) {

        variable = Value::createVariable(
            valueTypeByAstType(statement.type), statement.identifier );
    
    } else if( statement.initializerExpression ) {

        ValuePointer initializerValue = generate( statement.initializerExpression );
        variable = Value::createVariable( initializerValue->getType(), statement.identifier );
        variable->generateBinaryOperation( BinaryOperation::Assignment, initializerValue );

    } else {
        throwRuntimeError( "A variable declaration statement doesn't contain"
            " neither type identifier nor initializer expression" );
    }

    _symbolTable.addValue( statement.identifier, variable );

    return;
}

void CodeGenerator::generate( const ast::WhileStatement &statement ) {

    llvm::BasicBlock *loopStart = insertBasicBlock( "loopStart" );
    llvm::BasicBlock *loopCode = insertBasicBlock( "loopCode" );
    llvm::BasicBlock *loopEnd = insertBasicBlock( "loopEnd" );

    irBuilder.CreateBr( loopStart );

    generate( statement.blockStatement, loopCode );
    irBuilder.CreateBr( loopStart );

    irBuilder.SetInsertPoint( loopStart );
    llvm::Value *condition = generate( statement.conditionExpression )->toBoolean()->toLlvm();
    irBuilder.CreateCondBr( condition, loopCode, loopEnd );

    irBuilder.SetInsertPoint( loopEnd );
}

ValuePointer CodeGenerator::generate( ast::ExpressionPointer expression ) {

#define _8b_generate_expression( TYPE )                                       \
    if( expression->instanceOf<TYPE>() ) {                                    \
        return generate( *std::static_pointer_cast<const TYPE>(expression) ); \
    }

    _8b_generate_expression( ast::IdentifierExpression );
    _8b_generate_expression( ast::MemberAccessExpression );
    _8b_generate_expression( ast::IntegerConstantExpression );
    _8b_generate_expression( ast::BooleanConstantExpression );
    _8b_generate_expression( ast::UnaryOperationExpression );
    _8b_generate_expression( ast::BinaryOperationExpression );
    _8b_generate_expression( ast::CallExpression );

    throwRuntimeError( "Not implemented" );
}

ValuePointer CodeGenerator::generate( const ast::IdentifierExpression &expression ) {
    return _symbolTable.lookupValue( expression.identifier );
}

ValuePointer CodeGenerator::generate( const ast::MemberAccessExpression &expression ) {
    return generate( expression.operand )->generateMemberAccess( expression.memberIdentifier );
}

ValuePointer CodeGenerator::generate( const ast::IntegerConstantExpression &expression ) {
    return Value::createIntegerConstant( expression.value );
}

ValuePointer CodeGenerator::generate( const ast::BooleanConstantExpression &expression ) {
    return Value::createBooleanConstant( expression.value );
}

ValuePointer CodeGenerator::generate( const ast::UnaryOperationExpression &expression ) {
    ValuePointer operand = generate( expression.operand );
    return operand->generateUnaryOperation( expression.operation );
}

ValuePointer CodeGenerator::generate( const ast::BinaryOperationExpression &expression ) {
    ValuePointer leftValue = generate( expression.leftOperand );
    ValuePointer rightValue = generate( expression.rightOperand );
    return leftValue->generateBinaryOperation( expression.operation, rightValue );
}

ValuePointer CodeGenerator::generate( const ast::CallExpression &expression ) {

    ValuePointer callee = generate( expression.callee );

    std::vector<ValuePointer> arguments( expression.arguments.size() );

    std::transform(
        expression.arguments.cbegin(),
        expression.arguments.cend(),
        arguments.begin(),
        [this]( ast::ExpressionPointer expression ) -> ValuePointer {
            return CodeGenerator::generate( expression );
        });

    return callee->generateCall( arguments );
}

llvm::BasicBlock* CodeGenerator::insertBasicBlock( const std::string &name ) {
    return llvm::BasicBlock::Create( globalLLVMContext, name, irBuilder.GetInsertBlock()->getParent() );
}

ValueTypePointer CodeGenerator::valueTypeByAstType( ast::TypePointer astType ) {
    if( astType->instanceOf<ast::IntegerType>() )
        return IntegerType::get();
    if( astType->instanceOf<ast::BooleanType>() )
        return BooleanType::get();
    if( astType->instanceOf<ast::PointerType>() )
        return std::make_shared<PointerType>(valueTypeByAstType(
            std::static_pointer_cast<const ast::PointerType>(astType)->targetType));
    if( astType->instanceOf<ast::NamedType>() )
        return _symbolTable.lookupType(
            std::static_pointer_cast<const ast::NamedType>(astType)->identifier );
}

}
