#ifndef _LIBTHEO_VM_PROGRAM_HPP_
#define _LIBTHEO_VM_PROGRAM_HPP_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "VM/include/instr.hpp"
namespace Theo {

struct BreakPoint {
  std::string file;
  int line;
};

bool operator<(const BreakPoint &bp1, const BreakPoint &bp2);

struct Program {
  struct StackMap {
    std::string func_name;
    std::map<RegisterIndex, std::string> map;
  };

  std::vector<Instruction> code;
  std::vector<StackMap> stack_maps;

  // map from {filename, linenumber} to "bytecode position of potential
  // breakpoint"
  std::map<BreakPoint, std::vector<ProgramIndex>> potential_breaks;
  // map from "bytecode position of potential breakpoint" to {filename,
  // linenumber}
  std::map<ProgramIndex, BreakPoint> line_info;

  /* disassemble the program into triplet code*/
  void disassemble(std::ostream &o);

  /**
   * get a list of available breakpoints
   */
  std::set<BreakPoint> getAvailableBreakpoints();
};

}  // namespace Theo

#endif
