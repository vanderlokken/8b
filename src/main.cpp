#include <fstream>
#include <iostream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/TargetSelect.h>

#include "Exception.h"
#include "LexicalAnalyser.h"
#include "Parser.h"
#include "CodeGenerator.h"

using namespace _8b;

const std::string filename = "G:/Users/Lokken/Desktop/test.8b";

void printSourceLine( SourceLocation sourceLocation ) {
    // TODO: trim indentation
    std::ifstream input( filename, std::ios::binary );
    input.seekg( sourceLocation.lineBeginningOffset );
    std::string line;
    std::getline( input, line );
    std::cout << line << std::endl;
    for( size_t index = 0; index < sourceLocation.tokenBeginningOffset -
            sourceLocation.lineBeginningOffset; ++index )
        std::cout << " ";
    std::cout << "^~~~" << std::endl;
}

void printError( const char *message, SourceLocation sourceLocation ) {
    std::cout << "<filename>("
              << sourceLocation.lineNumber
              << ", "
              << sourceLocation.columnNumber
              << "): error: "
              << message
              << std::endl;
    std::cout << std::endl;
    printSourceLine( sourceLocation );
    std::cout << std::endl;
}

int main() {
    std::ifstream input( filename, std::ios::binary );

    try {
        LexicalAnalyser lexicalAnalyser( input );
        ast::Module moduleAst = parse( lexicalAnalyser );
        llvm::Module *module = generateCode( moduleAst );

        module->dump();

        llvm::InitializeNativeTarget();

        llvm::ExecutionEngine *executionEngine = llvm::ExecutionEngine::createJIT( module );
        llvm::Function *main = executionEngine->FindFunctionNamed( "main" );

        std::vector<llvm::GenericValue> arguments;
        llvm::GenericValue result = executionEngine->runFunction( main, arguments );

        std::cout << "Result is: " << result.IntVal.getLimitedValue();

    } catch( CompilationError &error ) {

        printError( error.what(), error.sourceLocation );

    } catch( std::exception &exception ) {
        std::cout << "Exception: " << exception.what();
    }

    std::cin.get();

    return 0;
}
