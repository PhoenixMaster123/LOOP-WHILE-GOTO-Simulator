#include <iostream>

#include "VM/include/program.hpp"
#include "VM/include/vm.hpp"

/*
  this file implements (hand-compiled) the following program:
  PROGRAM + IN x0, x1 DO
    LOOP x1 DO
      x0 := x0 + 1
    END
  END
  PROGRAM * IN x0, x1 OUT x2 DO
    LOOP x1 DO
      x2 := +(x2, x0)
    END
  END
  x0 := 7
  x1 := 13
  x0 := *(x0, x1)
 */

using namespace Theo;

int main() {
  // creating the stack map for our program
  std::vector<Program::StackMap> sm = {{"main",
                                        {// "main" script
                                         {0, "x0"},
                                         {1, "x1"}}},
                                       {"+",
                                        {// + program
                                         {0, "x0"},
                                         {1, "x2"}}},
                                       {"*",
                                        {// * program
                                         {0, "x0"},
                                         {1, "x1"},
                                         {2, "x2"}}}};

  std::vector<Instruction> code = {
      // 3 local variables ( 2 user declared, 1 compiler generated )
      Instruction::PrepareExec(3, 0, 0), Instruction::Exec(2),

      // main
      Instruction::Add(0, 0, 7),          // x0 := x0 + 7 (x0 := 7)
      Instruction::Add(1, 1, 13),         // x1 := x1 + 13 (x1 := 13)
      Instruction::PrepareExec(4, 2, 0),  // begin *
      Instruction::Arg(0, 0), Instruction::Arg(1, 1),
      Instruction::Exec(15),  // jmp to mul
      Instruction::Halt(),

      // +
      Instruction::Add(2, 1, 0),   // loop_var := x1 (LOOP x1 DO)
      Instruction::JmpC(+4, 2),    // break loop if loop_var == 0
      Instruction::Add(0, 0, 1),   // x0 := x0 + 1
      Instruction::Add(2, 2, -1),  // loop_var --
      Instruction::Jmp(-3),        // jmp to loop beginning
      Instruction::Ret(0),         // OUT x0

      // *
      Instruction::Add(3, 1, 0),          // loop_var := x1 (LOOP x1 DO)
      Instruction::JmpC(7, 3),            // break loop if loop_var == 0
      Instruction::PrepareExec(3, 1, 2),  // begin +(x2, x0)
      Instruction::Arg(0, 2),             // x2 in *  is value of x0 in +
      Instruction::Arg(1, 0),             // x0 in * is value of x1 in +
      Instruction::Exec(9),               // jmp to +
      Instruction::Add(3, 3, -1),         // loop_var--
      Instruction::Jmp(-6),               // jmp to loop beginning
      Instruction::Ret(2),                // OUT x2
  };

  Program p = {.code = code,
               .stack_maps = {sm},
               .potential_breaks = {},
               .line_info = {}};

  VM v(p);

  v.execute();

  VM::Activation::Data res = v.getActivations().back().getActivationVariables();

  std::cout << "Variables after Execution:" << std::endl;
  for (auto const& ac : res) {
    std::cout << ac.first << " = " << ac.second << std::endl;
  }

  if (res.find("x0") == res.end() || res["x0"] != 91) {
    std::cout << "Execution 1 failed" << std::endl;
    return 1;
  }

  v.reset();
  v.execute();
  if (res.find("x0") == res.end() || res["x0"] != 91) {
    std::cout << "Execution 2 after reset failed" << std::endl;
    return 1;
  }

  return 0;
}
