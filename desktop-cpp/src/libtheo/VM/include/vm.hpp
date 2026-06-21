#ifndef _LIBTHEO_VM_VM_HPP_
#define _LIBTHEO_VM_VM_HPP_

#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "VM/include/instr.hpp"
#include "program.hpp"

namespace Theo {

class VM {
 public:
  typedef int Word, WordIndex;

  class Activation {
    VM* vm;

    WordIndex data_start;
    WordIndex seg_size;
    RegisterIndex ret_target;
    ProgramIndex ret_addr;
    StackMapIndex debug_info;

    Activation(VM* vm, WordIndex data_start, WordIndex seg_size,
               RegisterIndex ret_target, ProgramIndex ret_addr,
               StackMapIndex debug_info);

   public:
    typedef std::map<std::string, Word> Data;

    /**
     * get a list of variable names and corresponding current values
     */
    VM::Activation::Data getActivationVariables();

    friend class VM;
  };

 private:
  bool stepping_mode_enabled;
  ProgramIndex instruction_pointer;
  Program code;
  std::vector<Word> data;
  std::vector<Activation> stack;
  std::set<BreakPoint> enabled_breakpoints;

 public:
  VM(Program code);

  /**
   * get reference to activation stack (for debug purposes)
   */
  std::vector<VM::Activation>& getActivations();

  /**
   * returns the line number of the breakpoint the VM
   * is currently on
   * @return line number != -1 only if the VM is currently halted on a
   * breakpoint, -1 otherwise and file = "none"
   */
  BreakPoint getCurrentBreak();

  /**
   * in stepping mode, potential breakpoints get treated as breakpoints
   * @param mode true enables stepping mode
   */
  void setSteppingMode(bool mode);

  bool isSteppingModeEnabled();

  /**
   * enable a breakpoint
   * @param line is the actual source code line to break on
   * @param value true to enable the breakpoint, false to disable
   * @return true if operation was successfull (false if e.g. line breakpoint
   * does not exist)
   */
  bool setBreakPoint(std::string file, int line, bool value);

  /**
   * disable all breakpoints
   */
  void clearBreakpoints();

  /**
   * reference to the set of currently enabled breakpoints
   */
  std::set<BreakPoint>& getEnabledBreakPoints();

  /**
   * resets the VM to its initial state;
   * any references held to activations will become invalid after this call;
   */
  void reset();

  /**
   * executes code until it runs into BREAK or HALT;
   * upon reaching HALT, all subsequent calls to this method
   * will remain without effect
   */
  void execute();

  /**
   * execute a single instruction and return;
   * use this method if you want to control the interpreter
   * loop yourself (e.g. after executing a instruction you're
   * able to check if execution was forcefully terminated)
   * You may construct a loop that can be externally terminated
   * and also breaks on breakpoints like this:
   *   bool terminated = false;
   *   while(!terminated && !vm.executeSingle()) ;
   * @return true  : a breakpoint (or the end of the prorgram) was reached
   *         false : no breakpoint was reached
   */
  bool executeSingle();

  /**
   * query if the VM has reached the end of the program
   * @return true if the end has been reached
   */
  bool isDone();
};

}  // namespace Theo

#endif
