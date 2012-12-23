#pragma once

#include "Ast.h"
#include "LexicalAnalyser.h"

namespace _8b {

ast::Module parse( LexicalAnalyser& );

}
