#include "VM/include/program.hpp"
#include "VM/include/vm.hpp"

using namespace Theo;

VM::Activation::Activation(VM *vm, VM::WordIndex data_start,
                           VM::WordIndex seg_size, RegisterIndex ret_target,
                           ProgramIndex ret_addr, StackMapIndex debug_info) {
  this->vm = vm;
  this->data_start = data_start;
  this->seg_size = seg_size;
  this->ret_target = ret_target;
  this->ret_addr = ret_addr;
  this->debug_info = debug_info;
}

VM::Activation::Data VM::Activation::getActivationVariables() {
  VM::Activation::Data res;
  Program::StackMap stack_map = this->vm->code.stack_maps[this->debug_info];
  for (int offset = 0; offset < this->seg_size; offset++) {
    for (auto &entry : stack_map.map) {
      res[entry.second] = this->vm->data[this->data_start + entry.first];
    }
  }
  return res;
}

VM::VM(Program code) {
  this->stepping_mode_enabled = false;
  this->instruction_pointer = 0;
  this->code = code;
  this->enabled_breakpoints = {};
  this->stack = {};
  this->data = {};
}

std::vector<VM::Activation> &VM::getActivations() { return this->stack; }

BreakPoint VM::getCurrentBreak() {
  auto itr = this->code.line_info.find(this->instruction_pointer - 1);
  return (itr == this->code.line_info.end()) ? (BreakPoint{"none", -1})
                                             : itr->second;
}

void VM::setSteppingMode(bool mode) { this->stepping_mode_enabled = mode; }

bool VM::setBreakPoint(std::string file, int line, bool value) {
  BreakPoint bp = {file, line};
  auto itr = this->code.potential_breaks.find(bp);
  if (itr == this->code.potential_breaks.end()) return false;
  if (!value) {
    this->enabled_breakpoints.erase(bp);
    for (auto ind : itr->second)
      this->code.code[ind].op = OpCode::POTENTIAL_BREAK;
  } else {
    this->enabled_breakpoints.insert(bp);
    for (auto ind : itr->second) this->code.code[ind].op = OpCode::BREAK;
  }
  return true;
}

void VM::clearBreakpoints() {
  for (auto const &bp : this->enabled_breakpoints) {
    for (auto ind : this->code.potential_breaks[bp])
      this->code.code[ind].op = OpCode::POTENTIAL_BREAK;
  }
  this->enabled_breakpoints.clear();
}

std::set<BreakPoint> &VM::getEnabledBreakPoints() {
  return this->enabled_breakpoints;
}

bool VM::isSteppingModeEnabled() { return this->stepping_mode_enabled; }

void VM::reset() {
  this->stepping_mode_enabled = false;
  this->instruction_pointer = 0;
  this->clearBreakpoints();
  this->data.clear();
  this->stack.clear();
}

bool VM::isDone() {
  return this->code.code[this->instruction_pointer].op == OpCode::HALT;
}

bool VM::executeSingle() {
  Instruction i = this->code.code[this->instruction_pointer];
  switch (i.op) {
  case OpCode::POTENTIAL_BREAK: {
    // std::cout << "Potential Break" << std::endl;
    this->instruction_pointer++;
    if (this->stepping_mode_enabled) {
      return true;
    }
    break;
  }
  case OpCode::BREAK: {
    // std::cout << "Break" << std::endl;
    this->instruction_pointer++;
    return true;
    break;
  }
  case OpCode::HALT: {
    // std::cout << "Halt" << std::endl;
    return true;
    break;
  }
  case OpCode::ADD_CONST: {
    // std::cout << "Add Const x" << i.parameters.add.target << " := x" <<
    // i.parameters.add.source << " + " << i.parameters.add.constant <<
    // std::endl;
    WordIndex base = this->stack.back().data_start;
    this->data[base + i.parameters.add.target] = std::max(
      this->data[base + i.parameters.add.source] +
      i.parameters.add.constant, 0);
    this->instruction_pointer++;
    break;
  }
  case OpCode::TEST: {
    WordIndex base = this->stack.back().data_start;
    this->data[base + i.parameters.test.target] =
      (this->data[base + i.parameters.test.op1] ==
       this->data[base + i.parameters.test.op2])
      ? 0
      : 1;
    this->instruction_pointer++;
    break;
  }
  case OpCode::CONST: {
    // std::cout << "Const load" << std::endl;
    WordIndex base = this->stack.back().data_start;
    this->data[base + i.parameters.constant.target] =
      i.parameters.constant.constant;
    this->instruction_pointer++;
    break;
  }
  case OpCode::JMP: {
    // std::cout << "Jmp" << std::endl;
    this->instruction_pointer += i.parameters.jmp.offset;
    break;
  }
  case OpCode::JMPC: {
    // std::cout << "JmpC" << std::endl;
    WordIndex base = this->stack.back().data_start;
    if (this->data[base + i.parameters.jmpc.source] == 0)
      this->instruction_pointer += i.parameters.jmpc.offset;
    else
      this->instruction_pointer++;
    break;
  }
  case OpCode::PREPARE_EXEC: {
    // std::cout << "Prepare Exec" << std::endl;
    int next_offset = this->data.size();
    int next_len = i.parameters.prepare.count;
    RegisterIndex ret_targ = i.parameters.prepare.target;
    StackMapIndex ind = i.parameters.prepare.index;
    for (int k = 0; k < next_len; k++) {
      this->data.push_back(0);
    }
    // return address filled by EXEC
    this->stack.push_back(
			  Activation(this, next_offset, next_len, ret_targ, -1, ind));
    this->instruction_pointer++;
    break;
  }
  case OpCode::ARG: {
    // std::cout << "Arg" << std::endl;
    int source_off = (*(this->stack.end() - 2)).data_start;
    int target_off = this->stack.back().data_start;
    this->data[target_off + i.parameters.arg.target] =
      this->data[source_off + i.parameters.arg.source];
    this->instruction_pointer++;
    break;
  }
  case OpCode::EXEC: {
    // std::cout << "Exec" << std::endl;
    this->stack.back().ret_addr = this->instruction_pointer + 1;
    this->instruction_pointer = i.parameters.exec.entry;
    break;
  }
  case OpCode::RET: {
    // std::cout << "Ret" << std::endl;
    RegisterIndex ret_target = this->stack.back().ret_target;
    WordIndex target_off = (*(this->stack.end() - 2)).data_start;
    RegisterIndex ret_source = i.parameters.ret.source;
    WordIndex source_off = this->stack.back().data_start;
    this->data[target_off + ret_target] =
      this->data[source_off + ret_source];
    this->instruction_pointer = this->stack.back().ret_addr;
    this->stack.pop_back();
    break;
  }
  }
  return false;
}

void VM::execute() {
  while(!executeSingle());
}
