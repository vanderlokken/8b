#pragma once

#include "TypeBuilder.h"

namespace _8b {

class BooleanType : public _Type {
public:

    BooleanType( TypeBuilder &typeBuilder ) {
        _rawType = typeBuilder.getIrBuilder().getInt1Ty();
    }

    virtual Value generateBinaryOperatorCall(
        TypeBuilder &typeBuilder,
        Operator operator_, Value instance, Value argument ) const
    {
        if( operator_ == Operator::Assign )
            return generateDefaultAssignmentOperatorCall(
                typeBuilder, instance, argument );
        return nullptr;
    }

    virtual std::string getName() const {
        return "boolean";
    }
};

}
