#pragma once

#include <vector>

#include "LexicalAnalyser.h"
#include "Statement.h"
#include "Type.h"

namespace _8b {
namespace ast{

struct Function {
    
    struct Argument {
        std::string identifier;
        TypePointer type;
    };
    
    Function( LexicalAnalyser& );
    
    std::string identifier;
    std::vector<Argument> arguments;
    TypePointer returnType;
    BlockStatement blockStatement;
};

}
}
