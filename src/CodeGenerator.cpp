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
        return boost::any_cast< ReturnType >( _generate(node) );
    }

    boost::any _generate( ast::Node node ) {
        try {
            return node->acceptVisitor( this );

        } catch( SemanticError &error ) {
            throw CompilationError( error.what(), node->sourceLocation );
        }
    }

    boost::any visit( ast::Module module ) {

        _currentModule = new llvm::Module( "module", irBuilder.getContext() );

        LexicalScope lexicalScope( _symbolTable );

        for( auto &declaration : module->classDeclarations )
            _generate( declaration );

        for( auto &declaration : module->functionDeclarations )
            _generate( declaration );

        return _currentModule;
    }

    boost::any visit( ast::ClassDeclaration classDeclaration ) {

        ClassType::Builder builder;

        for( auto &declaration : classDeclaration->memberDeclarations ) {

            if( declaration->initializer )
                throw NotImplementedError();

            builder.addMember(
                declaration->identifier,
                generate< ValueType >( declaration->type ) );
        }

        _symbolTable.addType( classDeclaration->identifier, builder.build() );

        return boost::any();
    }

    boost::any visit( ast::FunctionArgument ) {
        // This function is not used
        return boost::any();
    }

    ValueType generateFunctionType( ast::FunctionDeclaration declaration ) {

        FunctionType::Builder builder;

        for( auto &argument : declaration->arguments ) {
            ValueType type = generate< ValueType >( argument->type );
            builder.addArgument( argument->identifier, type );
        }

        builder.setReturnType(
            generate< ValueType >( declaration->returnType ) );

        return builder.build();
    }

    boost::any visit( ast::FunctionDeclaration declaration ) {

        ValueType type = generateFunctionType( declaration );

        _currentFunction = llvm::Function::Create(
            static_cast<llvm::FunctionType*>(type->toLlvm()),
            llvm::Function::ExternalLinkage, declaration->identifier,
            _currentModule );

        _symbolTable.addValue( declaration->identifier,
            _Value::createSsaValue(type, _currentFunction) );

        LexicalScope lexicalScope( _symbolTable );

        // Add arguments to the symbol table

        for( size_t i = 0; i < declaration->arguments.size(); ++i ) {

            // TODO: do not generate ValueType from ast::Type but use
            // previously generated

            auto argument = _currentFunction->getArgumentList().begin();
            std::advance( argument, i );

            Value value = _Value::createSsaValue(
                generate< ValueType >( declaration->arguments[i]->type ),
                argument );

            _symbolTable.addValue(
                declaration->arguments[i]->identifier, value );
        }

        _generate( declaration->block );

        return _currentFunction;
    }

    // ------------------------------------------------------------------------
    //  Statements
    // ------------------------------------------------------------------------

    boost::any visit( ast::Block block ) {

        // FIXME: a standalone block statement will cause an incorrect result

        llvm::BasicBlock *basicBlock = createBasicBlock();
        irBuilder.SetInsertPoint( basicBlock );

        LexicalScope lexicalScope( _symbolTable );

        for( auto &statement : block->statements )
            _generate( statement );

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
        Value condition =
            generate< Value >( statement->condition )->toBoolean();
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
                generate< Value >( statement->expression )->toLlvm() );
        else
            irBuilder.CreateRetVoid();
        return boost::any();
    }

    boost::any visit( ast::VariableDeclaration declaration ) {

        Value variable;

        if( declaration->type ) {

            auto type = generate< ValueType >( declaration->type );
            variable = _Value::createVariable( type, declaration->identifier );

        } else if( declaration->initializer ) {

            Value initializerValue =
                generate< Value >( declaration->initializer );
            variable = _Value::createVariable(
                initializerValue->getType(), declaration->identifier );
            variable->generateBinaryOperation(
                BinaryOperation::Assignment, initializerValue );

        } else {
            throw CompilationError(
                "A variable declaration statement doesn't contain neither type "
                "identifier nor initializer expression",
                declaration->sourceLocation );
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
        Value condition =
            generate< Value >( statement->condition )->toBoolean();
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
        auto leftValue = generate< Value >( expression->leftOperand );
        auto rightValue = generate< Value >( expression->rightOperand );
        return leftValue->generateBinaryOperation(
            expression->operation, rightValue );
    }

    boost::any visit( ast::BooleanConstant constant ) {
        return _Value::createBooleanConstant( constant->value );
    }

    boost::any visit( ast::CallExpression expression ) {
        auto value = generate< Value >( expression->callee );
        std::vector< Value > arguments( expression->arguments.size() );
        std::transform(
            expression->arguments.cbegin(),
            expression->arguments.cend(),
            arguments.begin(),
            [this]( ast::Expression expression ) -> Value {
                return generate< Value >( expression );
            }
        );
        return value->generateCall( arguments );
    }

    boost::any visit( ast::IdentifierExpression expression ) {
        try {
            return _symbolTable.lookupValue( expression->identifier );

        } catch( SymbolLookupError& ) {
            throw CompilationError(
                "Undeclared variable", expression->sourceLocation );
        }
    }

    boost::any visit( ast::InstanceExpression ) {
        throw NotImplementedError();
    }

    boost::any visit( ast::IntegerConstant constant ) {
        return _Value::createIntegerConstant( constant->value );
    }

    boost::any visit( ast::MemberAccessExpression expression ) {
        auto value = generate< Value >( expression->object );
        return value->generateMemberAccess(
            expression->memberIdentifier );
    }

    boost::any visit( ast::StringConstant constant ) {
        return _Value::createStringConstant( constant->value );
    }

    boost::any visit( ast::UnaryOperationExpression expression ) {
        auto value = generate< Value >( expression->operand );
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
        try {
            return _symbolTable.lookupType( namedType->identifier );

        } catch( SymbolLookupError& ) {
            throw CompilationError(
                "Undeclared type", namedType->sourceLocation );
        }
    }

    boost::any visit( ast::PointerType pointerType ) {
        auto type = std::make_shared< PointerType >(
            generate< ValueType >( pointerType->targetType ) );
        return std::static_pointer_cast< _ValueType >( type );
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
