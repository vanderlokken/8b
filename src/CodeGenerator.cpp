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

        // Generate a class type

        std::vector< ClassType::Member > members;
        members.reserve( classDeclaration->memberDeclarations.size() );

        for( auto &declaration : classDeclaration->memberDeclarations ) {

            if( declaration->initializer )
                throw NotImplementedError();

            members.emplace_back( declaration->identifier,
                generate< ValueType >( declaration->type ) );
        }

        auto classType = std::make_shared< ClassType >(
            classDeclaration->identifier, members );

        _symbolTable.addType( classDeclaration->identifier, classType );

        // Generate methods

        for( auto &declaration : classDeclaration->methodDeclarations ) {
            Value value = generateFunction( declaration, classType );
            classType->addMethod( declaration->identifier, value );
        }

        return boost::any();
    }

    boost::any visit( ast::FunctionArgument ) {
        // This function is not used
        return boost::any();
    }

    ValueType generateFunctionType(
        ast::FunctionDeclaration declaration, ValueType classType = nullptr )
    {
        std::vector< FunctionType::Argument > arguments;
        arguments.reserve( declaration->arguments.size() );

        if( classType )
            arguments.emplace_back( "instance", PointerType::get(classType) );

        for( auto &argument : declaration->arguments )
            arguments.emplace_back(
                argument->identifier, generate< ValueType >( argument->type ) );

        ValueType returnType;

        if( declaration->returnType )
            returnType = generate< ValueType >( declaration->returnType );

        return std::make_shared< FunctionType >( arguments, returnType );
    }

    Value generateFunction(
        ast::FunctionDeclaration declaration, ValueType classType = nullptr )
    {
        // Genrate type
        ValueType type = generateFunctionType( declaration, classType );

        llvm::FunctionType *llvmType =
            static_cast< llvm::FunctionType* >( type->toLlvm() );

        // Generate identifier
        std::string identifier = declaration->identifier;
        if( classType )
            identifier = std::static_pointer_cast< ClassType >(classType)->
                getIdentifier() + "." + identifier;

        // Generate value
        _currentFunction = llvm::Function::Create( llvmType,
            llvm::Function::ExternalLinkage, identifier, _currentModule );

        Value value = _Value::createSsaValue( type, _currentFunction );
        _symbolTable.addValue( declaration->identifier, value );

        // Add function arguments to the symbol table
        LexicalScope lexicalScope( _symbolTable );

        size_t argumentIndex = 0;

        for( const auto &argument :
                std::static_pointer_cast< FunctionType >(type)->getArguments() )
        {
            auto argumentIterator = _currentFunction->arg_begin();
            std::advance( argumentIterator, argumentIndex );

            llvm::Value *llvmArgumentValue = argumentIterator;

            if( classType && argumentIndex == 0 )
                _symbolTable.addValue( "instance",
                    _Value::createReference(classType, llvmArgumentValue) );
            else
                _symbolTable.addValue( argument.identifier,
                    _Value::createSsaValue(argument.type, llvmArgumentValue) );

            argumentIndex++;
        }

        generateBlock( declaration->block );

        // Check for the 'return' statement
        if( !irBuilder.GetInsertBlock()->getTerminator() )
            if( declaration->returnType )
                throw CompilationError(
                    "A function ends without the 'return' statement",
                    declaration->sourceLocation );
            else
                irBuilder.CreateRetVoid();

        return value;
    }

    boost::any visit( ast::FunctionDeclaration declaration ) {
        return generateFunction( declaration );
    }

    // ------------------------------------------------------------------------
    //  Statements
    // ------------------------------------------------------------------------

    llvm::BasicBlock* generateBlock(
        ast::Block block, bool insertBasicBlock = true )
    {
        llvm::BasicBlock *basicBlock =
            insertBasicBlock ? createBasicBlock() : irBuilder.GetInsertBlock();

        if( insertBasicBlock )
            irBuilder.SetInsertPoint( basicBlock );

        LexicalScope lexicalScope( _symbolTable );

        for( auto &statement : block->statements ) {
            if( irBuilder.GetInsertBlock()->getTerminator() )
                throw CompilationError(
                    "Unreachable code", statement->sourceLocation );
            _generate( statement );
        }

        return basicBlock;
    }

    boost::any visit( ast::Block block ) {
        // This method is called only for standalone explicit code blocks. In
        // such cases there's no need to create a new basic block.

        // Example: "statement; { statement; } statement;"

        const bool insertBasicBlock = false;
        return generateBlock( block, insertBasicBlock );
    }

    boost::any visit( ast::IfStatement statement ) {

        // For "if" statements without "false" blocks a parser creates empty
        // dummy "false" blocks. So it's not neccessary to check whether a
        // "false" block exists or not.

        auto previous = irBuilder.GetInsertBlock();
        auto trueBranch = generateBlock( statement->trueBlock );
        auto falseBranch = generateBlock( statement->falseBlock );
        auto end = createBasicBlock();

        trueBranch->moveAfter( previous );
        falseBranch->moveAfter( trueBranch );
        end->moveAfter( falseBranch );

        irBuilder.SetInsertPoint( previous );
        Value condition =
            generate< Value >( statement->condition )->toBoolean();
        irBuilder.CreateCondBr( condition->toLlvm(), trueBranch, falseBranch );

        createBranchIfNeeded( trueBranch, end );
        createBranchIfNeeded( falseBranch, end );

        if( end->getNumUses() )
            irBuilder.SetInsertPoint( end );
        else
            end->eraseFromParent();

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
        auto     loop = generateBlock( statement->block );

        start->moveAfter( previous );
        loop->moveAfter( start );
        end->moveAfter( loop );

        irBuilder.SetInsertPoint( previous );
        irBuilder.CreateBr( start );

        irBuilder.SetInsertPoint( start );
        Value condition =
            generate< Value >( statement->condition )->toBoolean();
        irBuilder.CreateCondBr( condition->toLlvm(), loop, end );

        createBranchIfNeeded( loop, start );

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

        std::vector< Value > arguments;
        arguments.reserve( expression->arguments.size() );

        for( const auto &argumentExpression : expression->arguments )
            arguments.emplace_back( generate< Value >( argumentExpression ) );

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

    boost::any visit( ast::InstanceExpression expression ) {
        try {
            return _symbolTable.lookupValue( "instance" );

        } catch( SymbolLookupError& ) {
            throw CompilationError( "'instance' is used outside a method",
                expression->sourceLocation );
        }
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
        auto type = PointerType::get(
            generate< ValueType >( pointerType->targetType ) );
        return std::static_pointer_cast< _ValueType >( type );
    }

    boost::any visit( ast::RealType ) {
        return RealType::get();
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

    void createBranchIfNeeded( llvm::BasicBlock *from, llvm::BasicBlock *to ) {
        if( !from->getTerminator() ) {
            irBuilder.SetInsertPoint( from );
            irBuilder.CreateBr( to );
        }
    }

    SymbolTable _symbolTable;

    llvm::Module *_currentModule;
    llvm::Function *_currentFunction;
};

llvm::Module* generateCode( ast::Module module ) {
    return CodeGenerator().generate< llvm::Module* >( module );
}

}
