#pragma once

#include <vector>

#include "Function.h"
#include "LexicalAnalyser.h"
#include "Statement.h"

namespace _8b {
namespace ast {
    
struct Class {

    Class( LexicalAnalyser& );

    std::string identifier;
    std::vector<Function> methods;
    std::vector<VariableDeclarationStatement> variables;
};

}
}
