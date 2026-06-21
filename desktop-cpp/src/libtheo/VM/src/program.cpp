#include <algorithm>

#include "VM/include/program.hpp"

std::string op_to_str[] = {
    "<Potential Breakpoint>", "<Breakpoint>", "HALT", "ADD", "JMP",  "JMPC",
    "PREPARE_EXEC",           "ARG",          "EXEC", "RET", "CONST"};

using namespace Theo;
#include <ostream>
void Program::disassemble(std::ostream &o) {
  for (size_t line = 0; line < this->code.size(); line++) {
    Instruction i = this->code[line];
    o << std::to_string(line) << ":\t";
    switch (this->code[line].op) {
      case OpCode::TEST: {
        o << "r[" << i.parameters.test.target << "] = "
          << "r[" << i.parameters.test.op1 << "] == "
          << "r[" << i.parameters.test.op2 << "] ? 0 : 1" << std::endl;
        break;
      }
      case OpCode::POTENTIAL_BREAK: {
        BreakPoint bp = this->line_info[line];
        o << "--\t\t\t\t" << bp.file << ":" << bp.line << std::endl;
        break;
      }
      case OpCode::BREAK: {
        BreakPoint bp = this->line_info[line];
        o << "++\t\t\t\t" << bp.file << ":" << bp.line << std::endl;
        break;
      }
      case OpCode::HALT:
        o << "Halt!" << std::endl;
        break;
      case OpCode::ADD_CONST:
        o << "r[" << i.parameters.add.target << "] = "
          << "r[" << i.parameters.add.source << "] + "
          << i.parameters.add.constant << std::endl;
        break;
      case OpCode::JMP:
        o << "goto " << i.parameters.jmp.offset + line << std::endl;
        break;
      case OpCode::JMPC:
        o << "if r[" << i.parameters.jmpc.source << "] == 0 then goto "
          << i.parameters.jmpc.offset + line << std::endl;
        break;
      case OpCode::PREPARE_EXEC:
        o << "Prepare("
          << "count = " << i.parameters.prepare.count << ", "
          << "stackmap = " << i.parameters.prepare.index << ", "
          << "target = " << i.parameters.prepare.target << ")" << std::endl;
        break;
      case OpCode::ARG:
        o << "arg[" << i.parameters.arg.target << "] = "
          << "r[" << i.parameters.arg.source << "]" << std::endl;
        break;
      case OpCode::EXEC:
        o << "exec " << i.parameters.exec.entry << std::endl;
        break;
      case OpCode::RET:
        o << "return r[" << i.parameters.ret.source << "]" << std::endl;
        break;
      case OpCode::CONST:
        o << "r[" << i.parameters.constant.target
          << "] = " << i.parameters.constant.constant << std::endl;
        break;
    };
  }
}

bool Theo::operator<(const BreakPoint &bp1, const BreakPoint &bp2) {
  if (bp1.file < bp2.file) return true;
  if (bp2.file < bp1.file) return false;
  return bp1.line < bp2.line;
}

std::set<BreakPoint> Program::getAvailableBreakpoints() {
  std::set<BreakPoint> result = {};
  std::for_each(potential_breaks.begin(), potential_breaks.end(),
                [&result](auto &p) -> void { result.insert(p.first); });
  return result;
}
