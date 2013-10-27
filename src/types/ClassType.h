#pragma once

#include <algorithm>

#include "TypeBuilder.h"

namespace _8b {

namespace {

class BoundMethodType : public _Type {
public:
    BoundMethodType( Value method, Value instance )
        : _methodType( method->getType() ), _instance( instance ) {}

    virtual Value generateGeneralOperatorCall(
        TypeBuilder &typeBuilder, Operator operator_,
        Value instance, const std::vector<Value> &arguments ) const
    {
        if( operator_ == Operator::Call ) {
            std::vector< Value > extendedArguments( arguments.size() + 1 );
            extendedArguments[0] =
                typeBuilder.getPointerType( _instance->getType() )->
                    createValue( _instance->getRawPointer() );
            std::copy(
                arguments.begin(),
                arguments.end(),
                extendedArguments.begin() + 1 );
            return _methodType->generateGeneralOperatorCall(
                typeBuilder, operator_, instance, extendedArguments );
        }
        return nullptr;
    }

    virtual std::string getName() const { return "\"bound method\""; }

private:
    Type _methodType;
    Value _instance;
};

}

class ClassType : public _Type {
public:

    struct Member {
        Member( const std::string &identifier, Type type ) :
            identifier( identifier ), type( type ) {}
        std::string identifier;
        Type type;
    };

    struct Method {
        Method( const std::string &identifier, Value value ) :
            identifier( identifier ), value( value ) {}
        std::string identifier;
        Value value;
    };

    ClassType(
        const std::string &identifier,
        const std::vector<Member> &members, TypeBuilder &typeBuilder )
            : _identifier( identifier ), _members( members )
    {
        std::vector<llvm::Type*> rawMemberTypes( _members.size() );
        std::transform(
            _members.begin(),
            _members.end(),
            rawMemberTypes.begin(),
            []( const Member &member ) { return member.type->getRaw(); });

        _rawType = llvm::StructType::create( rawMemberTypes );
    }

    void addMethod( const std::string &identifier, Value value ) {
        _methods.emplace_back( identifier, value );
    }

    const std::string& getIdentifier() const { return _identifier; }
    const std::vector<Member>& getMembers() const { return _members; }
    const std::vector<Method>& getMethods() const { return _methods; }

    virtual Value generateMemberAccess(
        TypeBuilder &typeBuilder,
        Value instance, const std::string &identifier ) const
    {
        llvm::IRBuilder<> &irBuilder = typeBuilder.getIrBuilder();

        auto memberIterator = std::find_if(
            _members.cbegin(),
            _members.cend(),
            [&identifier]( const Member &member ) {
                return member.identifier == identifier;
            });

        if( memberIterator != _members.end() ) {

            const size_t memberIndex =
                std::distance( _members.begin(), memberIterator );

            if( instance->isIndirect() )
                return _Value::createIndirect(
                    irBuilder.CreateStructGEP(
                        instance->getRawPointer(), memberIndex),
                    memberIterator->type, typeBuilder.getIrBuilder() );
            else {
                const unsigned indices[] = { memberIndex };
                return memberIterator->type->createValue(
                    irBuilder.CreateExtractValue(instance->getRaw(), indices) );
            }
        }

        auto methodIterator = std::find_if(
            _methods.cbegin(),
            _methods.cend(),
            [&identifier]( const Method &method ) {
                return method.identifier == identifier;
            });

        if( methodIterator != _methods.end() ) {
            Type type = std::make_shared< BoundMethodType >(
                methodIterator->value, instance );
            return type->createValue( methodIterator->value->getRaw() );
        }

        return nullptr;
    }

    virtual std::string getName() const {
        return _identifier;
    }

private:
    std::string _identifier;
    std::vector<Member> _members;
    std::vector<Method> _methods;
};

}
