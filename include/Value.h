#pragma once

#include <llvm/Value.h>

namespace _8b {

class Value {
public:

    class Type {

    };

    Value();

    llvm::Value* llvm() const;

private:
    Value::Type _type;
    llvm::Value *_llvmValue;
};

}
