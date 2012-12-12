#pragma once

#include <memory>

#include "ClassId.h"
#include "LexicalAnalyser.h"

namespace _8b {
namespace ast{


struct Type;
typedef std::shared_ptr<Type> TypePointer;


struct Type : public BaseIdClass {
    static TypePointer parse( LexicalAnalyser& );
};

template<class T>
struct ConcreteType : public DerivedIdClass<Type, T> {};


struct NamedType : public ConcreteType<NamedType> {
    
    NamedType( LexicalAnalyser& );

    std::string identifier;
};


struct IntegerType : public ConcreteType<IntegerType> {
    IntegerType( LexicalAnalyser& );
};


struct BooleanType : public ConcreteType<BooleanType> {
    BooleanType( LexicalAnalyser& );
};


struct PointerType : public ConcreteType<PointerType> {

    PointerType( LexicalAnalyser& );

    TypePointer targetType;
};


}
}
