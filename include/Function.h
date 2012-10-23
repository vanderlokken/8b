#pragma once

#include <vector>

#include "LexicalAnalyser.h"
#include "Statement.h"
#include "Type.h"

namespace _8b {
namespace ast{

class Function {
public:

    struct Argument {
        std::string identifier;
        Type type;
    };
    
    Function( LexicalAnalyser& );
    
    const std::string& getIdentifier() const noexcept;
    const std::vector<Argument>& getArguments() const noexcept;
    TypePointer getReturnType() const noexcept;
    const BlockStatement& getBlockStatement() const;
    
private:
    std::string _identifier;
    std::vector<Argument> _arguments;
    TypePointer _returnType;
    BlockStatement _blockStatement;
};

}
}
