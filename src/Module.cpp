#include "Module.h"

namespace _8b {
namespace ast{

Module::Module( LexicalAnalyser &lexicalAnalyser ) {
    do
        _functions.push_back( Function(lexicalAnalyser) );
    while( lexicalAnalyser.getCurrentToken().getType() != Token::Null );
}

const std::vector<Function>& Module::getFunctions() const {
    return _functions;
}

}
}
