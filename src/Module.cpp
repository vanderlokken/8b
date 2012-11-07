#include "Module.h"

namespace _8b {
namespace ast{

Module::Module( LexicalAnalyser &lexicalAnalyser ) {
    do
        if( lexicalAnalyser.getCurrentToken().getType() == Token::Keyword_Class )
            _classes.push_back( Class(lexicalAnalyser) );
        else
            _functions.push_back( Function(lexicalAnalyser) );
    while( lexicalAnalyser.getCurrentToken().getType() != Token::Null );
}

const std::vector<Class>& Module::getClasses() const {
    return _classes;
}

const std::vector<Function>& Module::getFunctions() const {
    return _functions;
}

}
}
