#include "Type.h"

#include "TypeBuilder.h"

namespace _8b {

Value _Type::generateDefaultAssignmentOperatorCall(
    TypeBuilder &typeBuilder, Value instance, Value argument ) const
{
    if( argument->isInstanceOf(shared_from_this()) )
        return typeBuilder.getVoidType()->createValue(
            typeBuilder.getIrBuilder().CreateStore(
                argument->getRaw(), instance->getRawPointer()) );
    else
        return nullptr;
}

}
