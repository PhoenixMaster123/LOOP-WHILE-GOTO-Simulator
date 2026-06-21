#ifndef __LIBTHEO_C_COMPILER_HPP__
#define __LIBTHEO_C_COMPILER_HPP__

#include "Compiler/include/ast.hpp"
#include "Compiler/include/gen.hpp"
#include "Compiler/include/parse.hpp"
namespace Theo {

/**
 * main compilation api;
 * @param files all valid files
 * @param main key of the main file in files
 * @return a codegen result which will contain a valid program or error messages
 */
CodegenResult compile(std::map<FileName, FileContent> files, FileName main);

};  // namespace Theo
#endif
