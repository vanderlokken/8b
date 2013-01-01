#pragma once

#include <stdexcept>

#include "Token.h"

namespace _8b {

struct NotImplementedError : std::exception {
    NotImplementedError() : std::exception( "Not implemented" ) {}
};

struct CompilationError : std::exception {
    CompilationError( const char *message, SourceLocation sourceLocation )
        : std::exception( message ), sourceLocation( sourceLocation ) {}
    SourceLocation sourceLocation;
};

struct LexicalError : CompilationError {
    LexicalError( const char *message, SourceLocation sourceLocation )
        : CompilationError( message, sourceLocation ) {}
};

struct SyntaxError : CompilationError {
    SyntaxError( const char *message, SourceLocation sourceLocation )
        : CompilationError( message, sourceLocation ) {}
};

}
