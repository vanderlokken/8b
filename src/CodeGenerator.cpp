#include "CodeGenerator.h"

#include <boost/optional/optional.hpp>

#include <llvm/IR/IRBuilder.h>

#include "Exception.h"
#include "Format.h"
#include "SymbolTable.h"
#include "types/ClassType.h"
#include "types/FunctionType.h"
#include "types/TypeBuilder.h"
#include "Value.h"

namespace _8b {

struct CodeGenerator : ast::NodeVisitor {

    CodeGenerator() :
        irBuilder( llvm::getGlobalContext() ),
        _typeBuilder( irBuilder ),
        _currentModule( nullptr ),
        _currentFunction( nullptr )
    {}

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
        } catch( NotImplementedError &error ) {
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

        generateFunctionImplementations();

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
                generate< Type >( declaration->type ) );
        }

        auto classType = std::make_shared< ClassType >(
            classDeclaration->identifier, members, _typeBuilder );

        _symbolTable.addType( classDeclaration->identifier, classType );

        // Generate methods

        for( auto &declaration : classDeclaration->methodDeclarations ) {
            Value value = generateFunctionHeader( declaration, classType );
            classType->addMethod( declaration->identifier, value );
        }

        return boost::any();
    }

    boost::any visit( ast::FunctionArgument ) {
        // This function is not used
        return boost::any();
    }

    Type generateFunctionType(
        ast::FunctionDeclaration declaration, Type classType = nullptr )
    {
        std::vector< Type > argumentTypes;
        argumentTypes.reserve( declaration->arguments.size() );

        if( classType )
            argumentTypes.emplace_back(
                _typeBuilder.getPointerType(classType) );

        for( auto &argument : declaration->arguments )
            argumentTypes.emplace_back( generate< Type >( argument->type ) );

        Type returnType;

        if( declaration->returnType )
            returnType = generate< Type >( declaration->returnType );

        return std::make_shared< FunctionType >(
            argumentTypes, returnType, _typeBuilder );
    }

    static std::string generateFunctionIdentifier(
        ast::FunctionDeclaration declaration, Type classType = nullptr )
    {
        if( !classType )
            return declaration->identifier;

        return std::static_pointer_cast< const ClassType >(classType)->
            getIdentifier() + "." + declaration->identifier;
    }

    Value generateFunctionHeader(
        ast::FunctionDeclaration declaration, Type classType = nullptr )
    {
        Type type = generateFunctionType( declaration, classType );

        llvm::FunctionType *rawType =
            static_cast< llvm::FunctionType* >( type->getRaw() );

        std::string identifier =
            generateFunctionIdentifier( declaration, classType );

        llvm::Function *rawValue = llvm::Function::Create( rawType,
            llvm::Function::ExternalLinkage, identifier, _currentModule );

        Value value = type->createValue( rawValue );
        _symbolTable.addValue( identifier, value );

        // Add a task to generate function implementation later
        _functionGenerationTasks.emplace_back( declaration, classType );

        return value;
    }

    void generateFunctionImplementation(
        ast::FunctionDeclaration declaration, Type classType = nullptr )
    {
        std::string identifier =
            generateFunctionIdentifier( declaration, classType );

        auto value = _symbolTable.lookupValue( identifier );
        auto type = value->getType();

        _currentFunction = static_cast< llvm::Function* >( value->getRaw() );

        // Add function arguments to the symbol table
        LexicalScope lexicalScope( _symbolTable );

        const std::vector<Type> &argumentTypes = std::static_pointer_cast<
            const FunctionType >( type )->getArgumentTypes();

        for( size_t index = 0; index < argumentTypes.size(); ++index ) {
            auto argumentIterator = _currentFunction->arg_begin();
            std::advance( argumentIterator, index );
            llvm::Value *rawArgumentValue = argumentIterator;

            if( classType && index == 0 )
                _symbolTable.addValue( "instance", _Value::createIndirect(
                    rawArgumentValue, classType, irBuilder, "instance") );
            else
                _symbolTable.addValue(
                   declaration->arguments[
                        index - (classType ? 1 : 0)]->identifier,
                   argumentTypes[index]->createValue(rawArgumentValue) );
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
    }

    void generateFunctionImplementations() {

        // It's necessary to generate function implementations after the
        // generation of function headers. To understand the reason look at the
        // following example:

        //    function x() integer{ return y(); }
        //    function y() integer{ return x(); }

        // It's not possible to generate any function before the generation of
        // the header of the other one.

        for( const auto &task : _functionGenerationTasks ) {
            generateFunctionImplementation( task.declaration, task.classType );
        }
    }

    boost::any visit( ast::FunctionDeclaration declaration ) {
        return generateFunctionHeader( declaration );
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
        Value condition = toBoolean( statement->condition );
        irBuilder.CreateCondBr( condition->getRaw(), trueBranch, falseBranch );

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
                generate< Value >( statement->expression )->getRaw() );
        else
            irBuilder.CreateRetVoid();
        return boost::any();
    }

    boost::any visit( ast::VariableDeclaration declaration ) {

        Value initializerValue;
        Type type;

        if( declaration->type ) {

            type = generate< Type >( declaration->type );

        } else if( declaration->initializer ) {

            initializerValue = generate< Value >( declaration->initializer );
            type = initializerValue->getType();

        } else {
            throw CompilationError(
                "A variable declaration statement doesn't contain neither type "
                "identifier nor initializer expression",
                declaration->sourceLocation );
        }

        llvm::Value *rawValue = irBuilder.CreateAlloca( type->getRaw() );
        Value variable = _Value::createIndirect(
            rawValue, type, irBuilder, declaration->identifier );

        if( initializerValue )
            type->generateBinaryOperatorCall( _typeBuilder,
                _Type::Operator::Assign, variable, initializerValue );

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
        Value condition = toBoolean( statement->condition );
        irBuilder.CreateCondBr( condition->getRaw(), loop, end );

        createBranchIfNeeded( loop, start );

        irBuilder.SetInsertPoint( end );

        return boost::any();
    }

    // ------------------------------------------------------------------------
    //  Expressions
    // ------------------------------------------------------------------------

    Value generateLogicOperation( ast::BinaryOperationExpression expression ) {
        // The following code implements specific operand evaluation for the
        // BinaryOperation::LogicAnd and BinaryOperation::LogicOr

        llvm::BasicBlock *evaluateRight = createBasicBlock();
        llvm::BasicBlock *end = createBasicBlock();

        llvm::Value *leftValue =
            toBoolean( expression->leftOperand )->getRaw();
        llvm::BasicBlock *leftParent = irBuilder.GetInsertBlock();

        if( expression->operation == BinaryOperation::LogicAnd )
            irBuilder.CreateCondBr( leftValue, evaluateRight, end );
        else // operation == BinaryOperation::LogicOr
            irBuilder.CreateCondBr( leftValue, end, evaluateRight );

        irBuilder.SetInsertPoint( evaluateRight );
        llvm::Value *rightValue =
            toBoolean( expression->rightOperand )->getRaw();
        llvm::BasicBlock *rightParent = irBuilder.GetInsertBlock();
        irBuilder.CreateBr( end );

        irBuilder.SetInsertPoint( end );

        const unsigned incomingBranchCount = 2;

        llvm::PHINode *resultValue =
            irBuilder.CreatePHI( irBuilder.getInt1Ty(), incomingBranchCount );

        if( expression->operation == BinaryOperation::LogicAnd )
            resultValue->addIncoming( irBuilder.getFalse(), leftParent );
        else // operation == BinaryOperation::LogicOr
            resultValue->addIncoming( irBuilder.getTrue(), leftParent );

        resultValue->addIncoming( rightValue, rightParent );

        return _typeBuilder.getBooleanType()->createValue( resultValue );
    }

    boost::any visit( ast::BinaryOperationExpression expression ) {
        _Type::Operator operator_;
        boost::optional<_Type::Operator> reverseOperator;

        switch( expression->operation ) {
        case BinaryOperation::LogicAnd:
        case BinaryOperation::LogicOr:
            return generateLogicOperation( expression );
        case BinaryOperation::Assignment:
            operator_ = _Type::Operator::Assign;
            break;
        case BinaryOperation::Addition:
            operator_ = _Type::Operator::Add;
            reverseOperator = _Type::Operator::AddRight;
            break;
        case BinaryOperation::Subtraction:
            operator_ = _Type::Operator::Subtract;
            reverseOperator = _Type::Operator::SubtractRight;
            break;
        case BinaryOperation::Multiplication:
            operator_ = _Type::Operator::Multiply;
            reverseOperator = _Type::Operator::MultiplyRight;
            break;
        case BinaryOperation::Division:
            operator_ = _Type::Operator::Divide;
            reverseOperator = _Type::Operator::DivideRight;
            break;
        case BinaryOperation::LessComparison:
            operator_ = _Type::Operator::Less;
            reverseOperator = _Type::Operator::Greater;
            break;
        case BinaryOperation::LessOrEqualComparison:
            operator_ = _Type::Operator::LessOrEqual;
            reverseOperator = _Type::Operator::GreaterOrEqual;
            break;
        case BinaryOperation::GreaterComparison:
            operator_ = _Type::Operator::Greater;
            reverseOperator = _Type::Operator::Less;
            break;
        case BinaryOperation::GreaterOrEqualComparison:
            operator_ = _Type::Operator::GreaterOrEqual;
            reverseOperator = _Type::Operator::LessOrEqual;
            break;
        case BinaryOperation::EqualComparison:
            operator_ = _Type::Operator::Equal;
            reverseOperator = _Type::Operator::Equal;
            break;
        case BinaryOperation::NotEqualComparison:
            operator_ = _Type::Operator::NotEqual;
            reverseOperator = _Type::Operator::NotEqual;
            break;
        default:
            throw NotImplementedError();
        }

        auto leftValue = generate< Value >( expression->leftOperand );
        auto rightValue = generate< Value >( expression->rightOperand );

        auto result = leftValue->getType()->generateBinaryOperatorCall(
            _typeBuilder, operator_, leftValue, rightValue );

        if( result )
            return result;

        if( reverseOperator ) {

            auto result = rightValue->getType()->generateBinaryOperatorCall(
                _typeBuilder, *reverseOperator, rightValue, leftValue );

            if( result )
                return result;
        }

        throw CompilationError(
            "No suitable binary operator found", expression->sourceLocation );
    }

    boost::any visit( ast::BooleanConstant constant ) {
        return _typeBuilder.getBooleanType()->createValue(
            irBuilder.getInt1(constant->value) );
    }

    boost::any visit( ast::CallExpression expression ) {
        auto value = generate< Value >( expression->callee );

        std::vector< Value > arguments;
        arguments.reserve( expression->arguments.size() );

        for( const auto &argumentExpression : expression->arguments )
            arguments.emplace_back( generate< Value >( argumentExpression ) );

        Value result = value->getType()->generateGeneralOperatorCall(
            _typeBuilder, _Type::Operator::Call, value, arguments );

        if( result )
            return result;
        else
            throw CompilationError( "No suitable call operator found",
                expression->sourceLocation );
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
        return _typeBuilder.getIntegerType()->createValue(
            irBuilder.getInt32(constant->value) );
    }

    boost::any visit( ast::MemberAccessExpression expression ) {
        Value value = generate< Value >( expression->object );

        Value result = value->getType()->generateMemberAccess(
            _typeBuilder, value, expression->memberIdentifier );

        if( result )
            return result;
        else
            throw CompilationError(
                format("A value of type '%1%' has no member '%2%'",
                    value->getType()->getName(), expression->memberIdentifier),
                expression->sourceLocation );
    }

    boost::any visit( ast::StringConstant constant ) {
        throw NotImplementedError();
    }

    boost::any visit( ast::UnaryOperationExpression expression ) {

        if( expression->operation == UnaryOperation::LogicInversion )
            return _typeBuilder.getBooleanType()->createValue(
                irBuilder.CreateNot(toBoolean(expression->operand)->getRaw()) );

        auto value = generate< Value >( expression->operand );
        Type type = value->getType();

        Value result;

        switch( expression->operation ) {
        case UnaryOperation::Increment:
            result = type->generateUnaryOperatorCall(
                _typeBuilder, _Type::Operator::Increment, value );
            break;
        case UnaryOperation::Decrement:
            result = type->generateUnaryOperatorCall(
                _typeBuilder, _Type::Operator::Decrement, value );
            break;
        case UnaryOperation::BooleanConversion:
            result = type->generateUnaryOperatorCall(
                _typeBuilder, _Type::Operator::ToBoolean, value );
            break;
        case UnaryOperation::PointerConversion:
            return _typeBuilder.getPointerType( type )->createValue(
                value->getRawPointer() );
        default:
            throw NotImplementedError();
        }

        if( result )
            return result;

        if( expression->operation == UnaryOperation::BooleanConversion )
            throw CompilationError(
                "A boolean conversion operator is not implemented",
                expression->sourceLocation );
        else
            throw CompilationError( "No suitable unary operator found",
                expression->sourceLocation );
    }

    // ------------------------------------------------------------------------
    //  Types
    // ------------------------------------------------------------------------

    boost::any visit( ast::BooleanType ) {
        return _typeBuilder.getBooleanType();
    }

    boost::any visit( ast::IntegerType ) {
        return _typeBuilder.getIntegerType();
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
        return _typeBuilder.getPointerType(
            generate< Type >( pointerType->targetType ) );
    }

    boost::any visit( ast::RealType ) {
        return _typeBuilder.getRealType();
    }

    boost::any visit( ast::StringType ) {
        throw NotImplementedError();
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

    Value toBoolean( ast::Expression expression ) {
        Value value = generate< Value >( expression );

        if( value->isInstanceOf(_typeBuilder.getBooleanType()) )
            return value;

        Value result = value->getType()->generateUnaryOperatorCall(
            _typeBuilder, _Type::Operator::ToBoolean, value );

        if( result )
            return result;
        else
            throw CompilationError(
                "A boolean conversion operator is not implemented",
                expression->sourceLocation );
    }

    llvm::IRBuilder<> irBuilder;

    TypeBuilder _typeBuilder;

    SymbolTable _symbolTable;

    llvm::Module *_currentModule;
    llvm::Function *_currentFunction;

    struct FunctionGenerationTask {
        FunctionGenerationTask(
                ast::FunctionDeclaration declaration, Type classType )
            : declaration( declaration ), classType( classType ) {}

        ast::FunctionDeclaration declaration;
        Type classType;
    };

    std::vector< FunctionGenerationTask > _functionGenerationTasks;
};

llvm::Module* generateCode( ast::Module module ) {
    return CodeGenerator().generate< llvm::Module* >( module );
}

}
