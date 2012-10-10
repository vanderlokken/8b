#pragma once

#include "LexicalAnalyser.h"

namespace _8b {
namespace ast{
    
class Type {
public:
    Type( LexicalAnalyser& );

private:
    std::string _identifier;    
};

}
}
