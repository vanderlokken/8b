#pragma once

#include <memory>

#include "ClassId.h"
#include "LexicalAnalyser.h"

namespace _8b {
namespace ast{


class Type;
typedef std::shared_ptr<Type> TypePointer;


class Type : public BaseIdClass {
public:
    static TypePointer parse( LexicalAnalyser& );
};

template<class T>
class ConcreteType : public DerivedIdClass<Type, T> {};


class IntegerType : public ConcreteType<IntegerType> {
public:
    IntegerType( LexicalAnalyser& );
};


class BooleanType : public ConcreteType<BooleanType> {
public:
    BooleanType( LexicalAnalyser& );
};


}
}
