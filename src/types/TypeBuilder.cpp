#include "TypeBuilder.h"

#include "BooleanType.h"
#include "FunctionType.h"
#include "IntegerType.h"
#include "PointerType.h"
#include "RealType.h"
#include "VoidType.h"

namespace {

template<class T, class U, class V>
U getOrCreateType( std::map<T, U> &types, const T &key, V createType ) {
    auto result = types.find( key );
    if( result == types.end() )
        result = types.insert( std::make_pair(key, createType()) ).first;
    return result->second;
}

}

namespace _8b {

Type TypeBuilder::getVoidType() {
    if( !_voidType )
        _voidType = std::make_shared<VoidType>( *this );
    return _voidType;
}

Type TypeBuilder::getBooleanType() {
    if( !_booleanType )
        _booleanType = std::make_shared<BooleanType>( *this );
    return _booleanType;
}

Type TypeBuilder::getIntegerType( size_t bitWidth ) {
    return getOrCreateType( _integerTypes, bitWidth, [this, bitWidth]() {
        return std::make_shared<IntegerType>( bitWidth, *this );
    });
}

// Type TypeBuilder::getUnsignedType( size_t bitWidth ) {
//     return getOrCreateType( _unsignedTypes, bitWidth, [this, bitWidth]() {
//         return std::make_shared<UnsignedType>( bitWidth, *this );
//     });
// }

Type TypeBuilder::getRealType( size_t bitWidth ) {
    return getOrCreateType( _realTypes, bitWidth, [this, bitWidth]() {
        return std::make_shared<RealType>( bitWidth, *this );
    });
}

Type TypeBuilder::getPointerType( Type targetType ) {
    return getOrCreateType( _pointerTypes, targetType, [this, &targetType]() {
        return std::make_shared<PointerType>( targetType, *this );
    });
}

Type TypeBuilder::getFunctionType(
    const std::vector<Type> &argumentTypes, Type returnType )
{
    const auto key = std::make_tuple( argumentTypes, returnType );
    return getOrCreateType(
        _functionTypes, key, [this, &argumentTypes, &returnType]() {
            return std::make_shared<FunctionType>(
                argumentTypes, returnType, *this );
        });
}

}
