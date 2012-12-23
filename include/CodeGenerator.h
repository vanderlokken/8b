#pragma once

#include <llvm/Module.h>

#include "Ast.h"

namespace _8b {

llvm::Module* generateCode( ast::Module );

}
