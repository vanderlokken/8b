#include "Module.h"

namespace _8b {
namespace ast{

Module::Module( LexicalAnalyser &lexicalAnalyser ) {
    do
        if( lexicalAnalyser.getCurrentToken().type == TokenType::KeywordClass )
            classes.push_back( Class(lexicalAnalyser) );
        else
            functions.push_back( Function(lexicalAnalyser) );
    while( lexicalAnalyser.getCurrentToken().type != TokenType::Null );
}

}
}
