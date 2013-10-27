#pragma once

#include <boost/format.hpp>

namespace _8b {

template<class T>
std::string format( const std::string &formatString, const T& argument ) {
    return boost::str( boost::format(formatString) % argument );
}

template<class T, class U>
std::string format(
    const std::string &formatString, const T& first, const U& second )
{
    return boost::str( boost::format(formatString) % first % second );
}

}
