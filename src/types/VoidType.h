#pragma once

#include "Type.h"

namespace _8b {

class VoidType : public _Type {
public:
    VoidType( TypeBuilder &typeBuilder ) {
        _rawType = typeBuilder.getIrBuilder().getVoidTy();
    }

    virtual std::string getName() const { return "\"not a value\""; }
};

}
