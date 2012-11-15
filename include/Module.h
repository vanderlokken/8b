#pragma once

#include <vector>

#include "Class.h"
#include "Function.h"
#include "LexicalAnalyser.h"

namespace _8b {
namespace ast{

struct Module {

    Module( LexicalAnalyser& );

    std::vector<Class> classes;
    std::vector<Function> functions;
};

}
}
