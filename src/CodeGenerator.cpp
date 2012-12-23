#include "CodeGenerator.h"

#include <llvm/Support/IRBuilder.h>

#include "Exception.h"
#include "SymbolTable.h"
#include "Value.h"

namespace _8b {

extern llvm::IRBuilder<> irBuilder( llvm::getGlobalContext() );

struct CodeGenerator : ast::NodeVisitor {

    // This function runs an appropriate code generation routine for the
    // specified ast::Node object and returns the result of the specified type
    template< class ReturnType > ReturnType generate( ast::Node node ) {
        return boost::any_cast< ReturnType >( node->acceptVisitor(this) );
    }

    boost::any visit( ast::Module module ) {

        _currentModule = new llvm::Module( "module", irBuilder.getContext() );

        LexicalScope lexicalScope( _symbolTable );

        for( auto &declaration : module->classDeclarations )
            declaration->acceptVisitor( this );

        for( auto &declaration : module->functionDeclarations )
            declaration->acceptVisitor( this );

        return _currentModule;
    }

    boost::any visit( ast::ClassDeclaration classDeclaration ) {

        ClassType::Builder builder;

        for( auto &declaration : classDeclaration->memberDeclarations ) {

            if( declaration->initializer )
                throwRuntimeError( "Not implemented" );

            builder.addMember(
                declaration->identifier,
                generate< ValueTypePointer >( declaration->type ) );
        }

        _symbolTable.addType( classDeclaration->identifier, builder.build() );

        return boost::any();
    }

    boost::any visit( ast::FunctionArgument ) {
        // This function is not used
        return boost::any();
    }

    ValueTypePointer generateFunctionType(
        ast::FunctionDeclaration declaration )
    {

        FunctionType::Builder builder;

        for( auto &argument : declaration->arguments ) {
            ValueTypePointer type =
                generate< ValueTypePointer >( argument->type );
            builder.addArgument( argument->identifier, type );
        }

        builder.setReturnType(
            generate< ValueTypePointer >( declaration->returnType ) );

        return builder.build();
    }

    boost::any visit( ast::FunctionDeclaration declaration ) {

        ValueTypePointer type = generateFunctionType( declaration );

        _currentFunction = llvm::Function::Create(
            static_cast<llvm::FunctionType*>(type->toLlvm()),
            llvm::Function::ExternalLinkage, declaration->identifier,
            _currentModule );

        _symbolTable.addValue( declaration->identifier,
            Value::createSsaValue(type, _currentFunction) );

        LexicalScope lexicalScope( _symbolTable );

        // Add arguments to the symbol table

        for( size_t i = 0; i < declaration->arguments.size(); ++i ) {

            // TODO: do not generate ValueTypePointer from ast::Type but use
            // previously generated

            auto argument = _currentFunction->getArgumentList().begin();
            std::advance( argument, i );

            ValuePointer value = Value::createSsaValue(
                generate< ValueTypePointer >( declaration->arguments[i]->type ),
                argument );

            _symbolTable.addValue(
                declaration->arguments[i]->identifier, value );
        }

        declaration->block->acceptVisitor( this );

        return _currentFunction;
    }

    // ------------------------------------------------------------------------
    //  Statements
    // ------------------------------------------------------------------------

    boost::any visit( ast::Block block ) {

        llvm::BasicBlock *basicBlock = createBasicBlock();
        irBuilder.SetInsertPoint( basicBlock );

        LexicalScope lexicalScope( _symbolTable );

        for( auto &statement : block->statements )
            statement->acceptVisitor( this );

        return basicBlock;
    }

    boost::any visit( ast::IfStatement statement ) {

        // For "if" statements without "false" blocks a parser creates empty
        // dummy "false" blocks. So it's not neccessary to check whether a
        // "false" block exists or not.

        auto previous = irBuilder.GetInsertBlock();
        auto trueBranch =
            generate< llvm::BasicBlock* >( statement->trueBlock );
        auto falseBranch =
            generate< llvm::BasicBlock* >( statement->falseBlock );
        auto end = createBasicBlock();

        trueBranch->moveAfter( previous );
        falseBranch->moveAfter( trueBranch );
        end->moveAfter( falseBranch );

        irBuilder.SetInsertPoint( previous );
        ValuePointer condition =
            generate< ValuePointer >( statement->condition )->toBoolean();
        irBuilder.CreateCondBr( condition->toLlvm(), trueBranch, falseBranch );

        irBuilder.SetInsertPoint( trueBranch );
        irBuilder.CreateBr( end );

        irBuilder.SetInsertPoint( falseBranch );
        irBuilder.CreateBr( end );

        irBuilder.SetInsertPoint( end );

        return boost::any();
    }

    boost::any visit( ast::ReturnStatement statement ) {
        if( statement->expression )
            irBuilder.CreateRet(
                generate< ValuePointer >( statement->expression )->toLlvm() );
        else
            irBuilder.CreateRetVoid();
        return boost::any();
    }

    boost::any visit( ast::VariableDeclaration declaration ) {

        ValuePointer variable;

        if( declaration->type ) {

            auto type = generate< ValueTypePointer >( declaration->type );
            variable = Value::createVariable( type, declaration->identifier );

        } else if( declaration->initializer ) {

            ValuePointer initializerValue =
                generate< ValuePointer >( declaration->initializer );
            variable = Value::createVariable(
                initializerValue->getType(), declaration->identifier );
            variable->generateBinaryOperation(
                BinaryOperation::Assignment, initializerValue );

        } else {
            throwRuntimeError(
                "A variable declaration statement doesn't contain neither type "
                "identifier nor initializer expression" );
        }

        _symbolTable.addValue( declaration->identifier, variable );

        return boost::any();
    }

    boost::any visit( ast::WhileStatement statement ) {

        auto previous = irBuilder.GetInsertBlock();
        auto    start = createBasicBlock();
        auto      end = createBasicBlock();
        auto     loop = generate< llvm::BasicBlock* >( statement->block );

        start->moveAfter( previous );
        loop->moveAfter( start );
        end->moveAfter( loop );

        irBuilder.SetInsertPoint( previous );
        irBuilder.CreateBr( start );

        irBuilder.SetInsertPoint( start );
        ValuePointer condition =
            generate< ValuePointer >( statement->condition )->toBoolean();
        irBuilder.CreateCondBr( condition->toLlvm(), loop, end );

        irBuilder.SetInsertPoint( loop );
        irBuilder.CreateBr( start );

        irBuilder.SetInsertPoint( end );

        return boost::any();
    }

    // ------------------------------------------------------------------------
    //  Expressions
    // ------------------------------------------------------------------------

    boost::any visit( ast::BinaryOperationExpression expression ) {
        auto leftValue = generate< ValuePointer >( expression->leftOperand );
        auto rightValue = generate< ValuePointer >( expression->rightOperand );
        return leftValue->generateBinaryOperation(
            expression->operation, rightValue );
    }

    boost::any visit( ast::BooleanConstant constant ) {
        return Value::createBooleanConstant( constant->value );
    }

    boost::any visit( ast::CallExpression expression ) {
        auto value = generate< ValuePointer >( expression->callee );
        std::vector< ValuePointer > arguments( expression->arguments.size() );
        std::transform(
            expression->arguments.cbegin(),
            expression->arguments.cend(),
            arguments.begin(),
            [this]( ast::Expression expression ) -> ValuePointer {
                return generate< ValuePointer >( expression );
            }
        );
        return value->generateCall( arguments );
    }

    boost::any visit( ast::IdentifierExpression expression ) {
        return _symbolTable.lookupValue( expression->identifier );
    }

    boost::any visit( ast::InstanceExpression ) {
        throwRuntimeError( "Not implemented" );
        return boost::any();
    }

    boost::any visit( ast::IntegerConstant constant ) {
        return Value::createIntegerConstant( constant->value );
    }

    boost::any visit( ast::MemberAccessExpression expression ) {
        auto value = generate< ValuePointer >( expression->object );
        return value->generateMemberAccess(
            expression->memberIdentifier );
    }

    boost::any visit( ast::StringConstant constant ) {
        return Value::createStringConstant( constant->value );
    }

    boost::any visit( ast::UnaryOperationExpression expression ) {
        auto value = generate< ValuePointer >( expression->operand );
        return value->generateUnaryOperation( expression->operation );
    }

    // ------------------------------------------------------------------------
    //  Types
    // ------------------------------------------------------------------------

    boost::any visit( ast::BooleanType ) {
        return BooleanType::get();
    }

    boost::any visit( ast::IntegerType ) {
        return IntegerType::get();
    }

    boost::any visit( ast::NamedType namedType ) {
        return _symbolTable.lookupType( namedType->identifier );
    }

    boost::any visit( ast::PointerType pointerType ) {
        auto type = std::make_shared< PointerType >(
            generate< ValueTypePointer >( pointerType->targetType ) );
        return std::static_pointer_cast< ValueType >( type );
    }

    boost::any visit( ast::StringType ) {
        return StringType::get();
    }

    // ------------------------------------------------------------------------
    //  Shortcuts
    // ------------------------------------------------------------------------

    llvm::BasicBlock* createBasicBlock() {
        return llvm::BasicBlock::Create(
            irBuilder.getContext(), "", _currentFunction );
    }

    SymbolTable _symbolTable;

    llvm::Module *_currentModule;
    llvm::Function *_currentFunction;
};

llvm::Module* generateCode( ast::Module module ) {
    return CodeGenerator().generate< llvm::Module* >( module );
}

}
