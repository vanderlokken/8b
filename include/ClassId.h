#pragma once


class BaseIdClass {
public:

    template<class T> bool instanceOf() const {
        return _typeId == T::typeId;
    }

protected:
        size_t _typeId;
};


template< class T, class U >
class DerivedIdClass : public T {
public:
    DerivedIdClass() { _typeId = typeId; }
    static const size_t typeId;
};

template< class T, class U >
const size_t DerivedIdClass<T, U>::typeId = (size_t)( typeid(U).raw_name() );
