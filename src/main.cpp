#include <fstream>
#include <iostream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/TargetSelect.h>

#include "LexicalAnalyser.h"
#include "Parser.h"
#include "CodeGenerator.h"

using namespace _8b;

int main() {
    std::ifstream input( "G:/Users/Lokken/Desktop/test.8b", std::ios::binary );

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

    } catch( std::exception &exception ) {
        std::cout << "Exception: " << exception.what();
    }

    std::cin.get();

    return 0;
}
