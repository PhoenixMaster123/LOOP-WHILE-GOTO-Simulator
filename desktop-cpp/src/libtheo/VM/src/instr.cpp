#include "VM/include/instr.hpp"

using namespace Theo;

Instruction Instruction::PotentialBreak() {
  return {.op = OpCode::POTENTIAL_BREAK};
}

Instruction Instruction::Break() { return {.op = OpCode::BREAK}; }

Instruction Instruction::Test(RegisterIndex target, RegisterIndex op1,
                              RegisterIndex op2) {
  return {.op = OpCode::TEST,
          .parameters = {.test = {.target = target, .op1 = op1, .op2 = op2}}};
}

Instruction Instruction::Add(RegisterIndex target, RegisterIndex source,
                             ::Constant constant) {
  return {
      .op = OpCode::ADD_CONST,
      .parameters = {
          .add = {.target = target, .source = source, .constant = constant}}};
}

Instruction Instruction::Halt() { return {.op = OpCode::HALT}; }

Instruction Instruction::Jmp(JumpOffset offset) {
  return {.op = OpCode::JMP,
          .parameters = {.jmp = {
                             .offset = offset,
                         }}};
}

Instruction Instruction::JmpC(JumpOffset offset, RegisterIndex source) {
  return {.op = OpCode::JMPC,
          .parameters = {.jmpc = {.offset = offset, .source = source}}};
}

Instruction Instruction::PrepareExec(RegisterCount count, StackMapIndex index,
                                     RegisterIndex target) {
  return {.op = OpCode::PREPARE_EXEC,
          .parameters = {
              .prepare = {.count = count, .index = index, .target = target}}};
}

Instruction Instruction::Arg(RegisterIndex target, RegisterIndex source) {
  return {.op = OpCode::ARG,
          .parameters = {.arg = {.target = target, .source = source}}};
}

Instruction Instruction::Exec(ProgramIndex entry) {
  return {.op = OpCode::EXEC, .parameters = {.exec = {.entry = entry}}};
}

Instruction Instruction::Ret(RegisterIndex source) {
  return {.op = OpCode::RET, .parameters = {.ret = {.source = source}}};
}

Instruction Instruction::LoadConstant(RegisterIndex target, Constant c) {
  return {.op = OpCode::CONST,
          .parameters = {.constant = {.target = target, .constant = c}}};
}
