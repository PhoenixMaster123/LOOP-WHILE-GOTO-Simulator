#include "Compiler/include/compiler.hpp"
#include "Compiler/include/gen.hpp"

using namespace Theo;

CodegenResult Theo::compile(std::map<FileName, FileContent> files,
                            FileName main) {
  ParseResult intermediate = parse(files, main);
  CodegenResult result = gen(intermediate.a);

  intermediate.a.clear();
  result.file_requests = intermediate.missing_files;

  return result;
}
