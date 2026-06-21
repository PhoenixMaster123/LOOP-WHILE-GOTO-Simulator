#ifndef _LIBTHEO_VM_INSTR_HPP_
#define _LIBTHEO_VM_INSTR_HPP_

namespace Theo {
enum class OpCode {
  POTENTIAL_BREAK,
  BREAK,
  HALT,
  ADD_CONST,
  JMP,
  JMPC,
  PREPARE_EXEC,
  ARG,
  EXEC,
  RET,
  CONST,
  TEST
};

typedef int RegisterIndex, JumpOffset, Constant, ProgramIndex, RegisterCount,
    StackMapIndex;

struct Instruction {
  OpCode op;

  union {
    struct {
      RegisterIndex target;
      RegisterIndex op1;
      RegisterIndex op2;
    } test;
    struct {
      RegisterIndex target;
      RegisterIndex source;
      Constant constant;
    } add;
    struct {
      RegisterIndex target;
      Constant constant;
    } constant;
    struct {
      JumpOffset offset;
    } jmp;
    struct {
      JumpOffset offset;
      RegisterIndex source;
    } jmpc;
    struct {
      RegisterCount count;
      StackMapIndex index;
      RegisterIndex target;
    } prepare;
    struct {
      RegisterIndex target;
      RegisterIndex source;
    } arg;
    struct {
      ProgramIndex entry;
    } exec;
    struct {
      RegisterIndex source;
    } ret;
  } parameters;

  static Instruction PotentialBreak();

  static Instruction Halt();

  static Instruction Break();

  /**
   * <target> := (<op1> == <op2>) ? 0 : 1
   */
  static Instruction Test(RegisterIndex target, RegisterIndex op1,
                          RegisterIndex op2);

  /**
   * <target> := <source> + <constant>
   */
  static Instruction Add(RegisterIndex target, RegisterIndex source,
                         Constant constant);

  /**
   * InstructionPointer := InstructionPointer + <offset>
   */
  static Instruction Jmp(JumpOffset offset);

  /**
   * IF <source> == 0 THEN InstructionPointer := InstructionPointer + <offset>
   */
  static Instruction JmpC(JumpOffset offset, RegisterIndex source);

  /**
   * Creates new stack frame after the current one with <count> registers
   * and initializes those to zero;
   * The new activation will have return register set to target;
   * StackMapIndex is the index to a stack map assigning names of variables
   * to virtual registers
   */
  static Instruction PrepareExec(RegisterCount count, StackMapIndex index,
                                 RegisterIndex target);

  /**
   * copies from register <source> of second to last stack frame
   * to register <target> of last stack frame
   */
  static Instruction Arg(RegisterIndex target, RegisterIndex source);

  /**
   * InstructionPointer := <entry>;
   * used for calling functions:
   * PREPARE 3
   * ARG 1 13
   * ARG 2 14
   * EXEC <entry>
   */
  static Instruction Exec(ProgramIndex entry);

  /**
   * copies from register <source> of the current stack frame
   * to the return register specified for the second to last stack frame
   * and resets InstructionPointer to the return address
   */
  static Instruction Ret(RegisterIndex source);

  /**
   * <target> = c
   */
  static Instruction LoadConstant(RegisterIndex target, Constant c);
};
}  // namespace Theo

#endif
