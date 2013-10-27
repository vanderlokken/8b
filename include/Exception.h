#pragma once

#include <stdexcept>

#include "Token.h"

namespace _8b {

struct Exception : public std::exception {
    Exception( const std::string &message ) : _message( message ) {}

    virtual const char* what() const {
        return _message.c_str();
    }
private:
    std::string _message;
};

struct NotImplementedError : public Exception {
    NotImplementedError() : Exception( "Not implemented" ) {}
};

struct CompilationError : public Exception {
    CompilationError(
        const std::string &message, SourceLocation sourceLocation )
            : Exception( message ), sourceLocation( sourceLocation ) {}
    SourceLocation sourceLocation;
};

struct LexicalError : public CompilationError {
    LexicalError( const std::string &message, SourceLocation sourceLocation )
        : CompilationError( message, sourceLocation ) {}
};

struct SyntaxError : public CompilationError {
    SyntaxError( const std::string &message, SourceLocation sourceLocation )
        : CompilationError( message, sourceLocation ) {}
};

// When exceptions of this type are thrown a source location is not known.
struct SemanticError : public Exception {
    SemanticError( const std::string &message ) : Exception( message ) {}
};

}
