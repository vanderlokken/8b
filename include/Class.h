#pragma once

#include <vector>

#include "Function.h"
#include "LexicalAnalyser.h"
#include "Statement.h"

namespace _8b {
namespace ast {
    
class Class {
public:
    Class( LexicalAnalyser& );

    const std::string& getIdentifier() const;
    const std::vector<Function>& getMethods() const;
    const std::vector<VariableDeclarationStatement>& getVariables() const;

private:
    std::string _identifier;
    std::vector<Function> _methods;
    std::vector<VariableDeclarationStatement> _variables;
};

}
}
