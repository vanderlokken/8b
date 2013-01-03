#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/TargetSelect.h>

#include "Exception.h"
#include "LexicalAnalyser.h"
#include "Parser.h"
#include "CodeGenerator.h"

using namespace _8b;

struct CommandLineArguments {
    std::string filename;
};

CommandLineArguments parseCommandLineArguments( int argc, char **argv ) {

    namespace options = boost::program_options;

    options::options_description optional;
    optional.add_options()
        ( "help", "displays help" )
        ( "filename", "file to compile" );

    options::positional_options_description positional;
    positional.add( "filename", -1 );

    options::variables_map variables;
    options::store(
        options::command_line_parser(argc, argv).options(optional).
            positional(positional).run(), variables );
    options::notify( variables );

    if( variables.count("help") ) {
        std::cout << optional << std::endl;
        std::exit( 1 );
    }

    if( !variables.count("filename") ) {
        std::cerr << "No input file specified" << std::endl;
        std::exit( 1 );
    }

    CommandLineArguments arguments;
    arguments.filename = variables["filename"].as< std::string >();
    return arguments;
}

void printSourceLine(
    const std::string &filename, SourceLocation sourceLocation )
{

    std::ifstream input( filename, std::ios::binary );

    input.seekg( sourceLocation.lineBeginningOffset );

    std::string line;
    std::getline( input, line );

    // Trim indentation
    size_t lineLength = line.size();
    boost::trim_left( line );
    size_t indentationLength = lineLength - line.size();

    std::cerr << line << std::endl;

    std::string spaces( sourceLocation.tokenBeginningOffset -
        sourceLocation.lineBeginningOffset - indentationLength, ' ' );

    std::cerr << spaces + "^~~~" << std::endl;
}

int main( int argc, char **argv ) {

    auto commandLineArguments = parseCommandLineArguments( argc, argv );

    std::ifstream input( commandLineArguments.filename, std::ios::binary );

    try {
        LexicalAnalyser lexicalAnalyser( input );
        ast::Module moduleAst = parse( lexicalAnalyser );
        llvm::Module *module = generateCode( moduleAst );

        module->dump();

        llvm::InitializeNativeTarget();

        llvm::ExecutionEngine *executionEngine =
            llvm::ExecutionEngine::createJIT( module );
        llvm::Function *main = executionEngine->FindFunctionNamed( "main" );

        std::vector< llvm::GenericValue > arguments;
        llvm::GenericValue result =
            executionEngine->runFunction( main, arguments );

        std::cout << "Exit code: " << result.IntVal.getLimitedValue();

    } catch( CompilationError &error ) {

        SourceLocation sourceLocation = error.sourceLocation;

        std::cerr << boost::format( "%s(%d, %d): error: %s\n" ) %
            commandLineArguments.filename % sourceLocation.lineNumber %
            sourceLocation.columnNumber % error.what();
        std::cerr << std::endl;

        printSourceLine( commandLineArguments.filename, sourceLocation );
        std::cerr << std::endl;

    } catch( std::exception &exception ) {
        std::cerr << "Exception: " << exception.what();
    }

    std::cin.get();

    return 0;
}
