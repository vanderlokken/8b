#pragma once

#include <vector>

#include "Class.h"
#include "Function.h"
#include "LexicalAnalyser.h"

namespace _8b {
namespace ast{

class Module {
public:
    Module( LexicalAnalyser& );

    const std::vector<Class>& getClasses() const;
    const std::vector<Function>& getFunctions() const;
    
private:
    std::vector<Class> _classes;
    std::vector<Function> _functions;
};

}
}
