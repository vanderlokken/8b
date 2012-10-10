#pragma once

#include <vector>

#include "Function.h"
#include "LexicalAnalyser.h"

namespace _8b {
namespace ast{

class Module {
public:
    Module( LexicalAnalyser& );

    const std::vector<Function>& getFunctions() const;
    
private:
    std::vector<Function> _functions;
};

}
}
