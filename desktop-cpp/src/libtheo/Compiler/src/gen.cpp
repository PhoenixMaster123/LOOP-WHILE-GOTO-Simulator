#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <map>
#include <string>

#include "Compiler/include/gen.hpp"
#include "VM/include/instr.hpp"

using namespace Theo;

// Internal Parser Structs

struct VReg {
  bool in_use;
  bool is_temp;
  std::string name;
};

struct FunctionGenState {
  std::string name;
  std::vector<VReg> register_state = {};
  int argnum = 0;
  std::map<std::string, int> marks = {};

  // get a temporary register
  RegisterIndex fetchTemporary() {
    // linear search is inefficient, but practically bounded for useful programs
    for (std::size_t i = 0; i < this->register_state.size(); i++) {
      if (this->register_state[i].is_temp && !this->register_state[i].in_use) {
        this->register_state[i].in_use = true;
        return i;
      }
    }
    this->register_state.push_back({true, true, "Temporary Variable"});
    return this->register_state.size() - 1;
  }

  void releaseTemporary(int index) {
    if (this->register_state[index].is_temp)
      this->register_state[index].in_use = false;
  }

  // get a register for a variable
  RegisterIndex fetchVariableRegister(std::string varname) {
    for (std::size_t i = 0; i < this->register_state.size(); i++) {
      if (this->register_state[i].name == varname) return i;
    }
    this->register_state.push_back({true, false, varname});
    return this->register_state.size() - 1;
  }
};

struct FileState {
  std::string name;
  int line;
};

struct Prog {
  ProgramIndex ind;
  StackMapIndex mi;
  int argnum;
  int stack_size;
};

struct GenState {
  Theo::AST in;

  Program out;
  std::vector<CodegenResult::Error> errors;

  std::vector<FunctionGenState> symbols;  // symbol table stack

  std::map<std::string, Prog> funcAddrs;  // functionName -> Address

  std::vector<int> labels;  // index = label, value = label pos

  std::vector<ProgramIndex> backpatching_todo;

  int loops = 0;

  FileState fs;

  void err(CodegenResult::Error::Type t, std::string msg) {
    std::string in_file = "root";
    int on_line = 0;
    in_file = fs.name;
    on_line = fs.line;
    this->errors.push_back({t, msg, in_file, on_line});
  }

  void verr(CodegenResult::Error::Type t, std::string msg, std::string file,
            int line) {
    this->errors.push_back({t, msg, file, line});
  }

  ProgramIndex getNextPos() { return out.code.size(); }

  ProgramIndex getMarkPos() {
    if (out.code.back().op == OpCode::POTENTIAL_BREAK) {
      return this->getNextPos() - 1;
    }
    return this->getNextPos();
  }

  void removeTopPotBreak() {
    if (out.code.back().op == OpCode::POTENTIAL_BREAK) {
      BreakPoint bp = this->out.line_info[this->getNextPos() - 1];
      this->out.line_info.erase(
          this->out.line_info.find(this->getNextPos() - 1));
      this->out.potential_breaks.erase(this->out.potential_breaks.find(bp));
      out.code.pop_back();
    }
  }

  void emit(Instruction i) { this->out.code.push_back(i); }

  void breakpoint() {
    BreakPoint bp = {.file = fs.name, .line = fs.line};
    this->out.line_info[this->getNextPos()] = bp;
    this->out.potential_breaks[bp].push_back(this->getNextPos());
    this->emit(Instruction::PotentialBreak());
  }

  void pushSymbols(std::string activation_name) {
    this->symbols.push_back({activation_name});
  }

  FunctionGenState &getSymbols() { return this->symbols.back(); }

  // finish a function
  void popSymbols(ProgramIndex addr) {
    FunctionGenState fgs = this->getSymbols();

    for (auto e : fgs.marks) {
      if (this->labels[e.second] == -1) {
        this->err(CodegenResult::Error::Type::UNKNOWN_MARK,
                  "In program '" + fgs.name + "', mark '" + e.first +
                      "' is referenced but is never set");
      }
    }

    Program::StackMap sm;
    sm.func_name = fgs.name;
    sm.map = {};
    for (std::size_t i = 0; i < fgs.register_state.size(); i++) {
      if (!fgs.register_state[i].is_temp)
        sm.map[i] = fgs.register_state[i].name;
    }

    this->out.stack_maps.push_back(sm);

    Prog p = {.ind = addr,
              .mi = (int)(this->out.stack_maps.size() - 1),
              .argnum = fgs.argnum,
              .stack_size = (int)fgs.register_state.size()};

    this->funcAddrs[fgs.name] = p;

    this->symbols.pop_back();
  }

  void advanceLine(int new_lineno, std::string file) {
    if (file == "__standards__")
      return;  // standard macros for (id + int, id - int are not visible)
    if (fs.name == file && new_lineno != fs.line) {
      fs.line = new_lineno;
      this->breakpoint();
    }
    if (fs.name != file) {
      fs.name = file;
      fs.line = new_lineno;
      this->breakpoint();
    }
  }

  int createLabel() {
    this->labels.push_back(-1);
    return this->labels.size() - 1;
  }

  void setLabel(int label, ProgramIndex i) { this->labels[label] = i; }

  void emitBackpatched(Instruction i) {
    this->emit(i);
    this->backpatching_todo.push_back(this->getNextPos() - 1);
  }

  // performs backpatching on the code
  void backpatchErr() {
    this->err(CodegenResult::Error::Type::UNKNOWN_MARK,
              "backpatching failed because of unknown mark");
  }

  void backpatch() {
    for (auto loc : this->backpatching_todo) {
      if (this->out.code[loc].op == OpCode::JMP) {
        int label = this->out.code[loc].parameters.jmp.offset;
        ProgramIndex tgt = this->labels[label];
        if (tgt == -1) this->backpatchErr();
        this->out.code[loc].parameters.jmp.offset = tgt - loc;
      } else if (this->out.code[loc].op == OpCode::JMPC) {
        int label = this->out.code[loc].parameters.jmpc.offset;
        ProgramIndex tgt = this->labels[label];
        if (tgt == -1) this->backpatchErr();
        this->out.code[loc].parameters.jmpc.offset = tgt - loc;
      } else {
        this->err(
            CodegenResult::Error::Type::INTERNAL_ERROR,
            "attempted to backpatch non-jmp at loc " + std::to_string(loc));
      }
    }

    this->backpatching_todo.clear();
  }
};

/*recursive AST traversal funcs*/
void dispatchVoid(GenState &gs, Node *c);
void dispatchValue(GenState &gs, Node *c, RegisterIndex tgt);
void gen_ast(GenState &gs, std::string fname);

void dispatchIf(GenState &gs, Node *c) {
  RegisterIndex cond = gs.getSymbols().fetchTemporary(),
                op1 = gs.getSymbols().fetchTemporary(),
                op2 = gs.getSymbols().fetchTemporary();

  dispatchValue(gs, c->left->left, op1);
  dispatchValue(gs, c->left->right, op2);

  gs.emit(Instruction::Test(cond, op1, op2));

  std::string name = std::string(c->right->left->tok);

  if (gs.getSymbols().marks.find(name) == gs.getSymbols().marks.end()) {
    gs.getSymbols().marks[name] = gs.createLabel();
  }

  gs.emitBackpatched(Instruction::JmpC(gs.getSymbols().marks[name], cond));

  gs.getSymbols().releaseTemporary(cond);
  gs.getSymbols().releaseTemporary(op1);
  gs.getSymbols().releaseTemporary(op2);
}

void dispatchGoto(GenState &gs, Node *c) {
  std::string name = std::string(c->left->tok);

  if (gs.getSymbols().marks.find(name) == gs.getSymbols().marks.end())
    gs.getSymbols().marks[name] = gs.createLabel();

  gs.emitBackpatched(Instruction::Jmp(gs.getSymbols().marks[name]));
}

// create a mark in the current function
void dispatchMark(GenState &gs, Node *c) {
  std::string name = std::string(c->left->tok);
  if (gs.getSymbols().marks.find(name) == gs.getSymbols().marks.end()) {
    gs.getSymbols().marks[name] = gs.createLabel();
  }

  gs.setLabel(gs.getSymbols().marks[name], gs.getMarkPos());
}

// dispatch while construct
void dispatchWhile(GenState &gs, Node *c) {
  int startLabel = gs.createLabel(), endLabel = gs.createLabel();

  RegisterIndex cond_reg = gs.getSymbols().fetchTemporary();

  // WHILE:
  gs.setLabel(startLabel, gs.getNextPos());

  // calculate condition
  dispatchValue(gs, c->left, cond_reg);

  // if cond == 0 GOTO END
  gs.emitBackpatched(Instruction::JmpC(endLabel, cond_reg));

  dispatchVoid(gs, c->right);  // while body

  gs.emitBackpatched(Instruction::Jmp(startLabel));  // goto WHILE

  // END:
  gs.setLabel(endLabel, gs.getNextPos());

  gs.getSymbols().releaseTemporary(cond_reg);
}

// dispatch loop construct
void dispatchLoop(GenState &gs, Node *c) {
  gs.loops++;
  std::string loop_counter = "Loop Variable " + gs.fs.name + ":" +
                             std::to_string(gs.fs.line) + "[" +
                             std::to_string(gs.loops) + "]";

  RegisterIndex counter = gs.getSymbols().fetchVariableRegister(loop_counter);

  // initialize counter
  dispatchValue(gs, c->left, counter);

  int startLabel = gs.createLabel(), endLabel = gs.createLabel();

  // LOOP:
  gs.setLabel(startLabel, gs.getNextPos());

  // IF counter == 0 GOTO END
  gs.emitBackpatched(Instruction::JmpC(endLabel, counter));

  dispatchVoid(gs, c->right);  // body of loop

  gs.emit(Instruction::Add(counter, counter, -1));

  gs.emitBackpatched(Instruction::Jmp(startLabel));  // GOTO LOOP

  // END:
  gs.setLabel(endLabel, gs.getNextPos());
}

// allocated registers for program arguments
void dispatchArgs(GenState &gs, Node *c) {
  if (c == NULL) return;

  if (c->t == Node::Type::SPLIT) {
    dispatchArgs(gs, c->left);
    dispatchArgs(gs, c->right);
    return;
  }

  gs.getSymbols().argnum++;
  gs.getSymbols().fetchVariableRegister(std::string(c->tok));
}

// dispatch a function definition
void dispatchProgram(GenState &gs, Node *c) {
  // create code to jump over the function code
  int after_label = gs.createLabel();
  gs.emitBackpatched(Instruction::Jmp(after_label));

  // generate program code
  Node *name_node = c->left->left, *args_node = c->left->right->left,
       *out_node = c->left->right->right, *body_node = c->right;

  std::string name = std::string(name_node->tok);
  gs.pushSymbols(name);

  // dispatch args so that they occupy the first regs
  dispatchArgs(gs, args_node);

  std::string out_name = "x0";
  if (out_node != NULL) out_name = std::string(out_node->tok);

  ProgramIndex i = gs.getNextPos();
  // generate prog body
  dispatchVoid(gs, body_node);

  // generate return instruction
  RegisterIndex ret_val = gs.getSymbols().fetchVariableRegister(out_name);
  gs.emit(Instruction::Ret(ret_val));

  gs.popSymbols(i);
  // set label addr
  gs.setLabel(after_label, gs.getNextPos());
}

void dispatchCallArgs(GenState &gs, Node *c,
                      std::vector<RegisterIndex> &arglocs) {
  if (c == NULL) return;

  if (c->t == Node::Type::SPLIT) {
    dispatchCallArgs(gs, c->left, arglocs);
    dispatchCallArgs(gs, c->right, arglocs);
    return;
  }

  RegisterIndex tmp = gs.getSymbols().fetchTemporary();
  dispatchValue(gs, c, tmp);
  arglocs.push_back(tmp);
}

void dispatchValue(GenState &gs, Node *c, RegisterIndex tgt) {
  if (c == NULL) return;
  gs.advanceLine(c->line, c->file);

  switch (c->t) {
    case Node::Type::NAME: {  // value copying : target = source + 0
      RegisterIndex src =
          gs.getSymbols().fetchVariableRegister(std::string(c->tok));
      gs.emit(Instruction::Add(tgt, src, 0));
      break;
    }
    case Node::Type::NUMBER: {
      int cs = std::stoi(std::string(c->tok));
      gs.emit(Instruction::LoadConstant(tgt, cs));
      break;
    }
    case Node::Type::CALL: {
      std::vector<RegisterIndex> arglocs;
      dispatchCallArgs(gs, c->right, arglocs);

      std::string funcname = std::string(c->left->tok);

      // check if these are inbuilt operations (add constant, sub constant):
      bool register_constant_operation =
          arglocs.size() == 2 && c->right->left->t == Node::Type::NAME &&
          c->right->right->left->t == Node::Type::NUMBER;

      if ((funcname == "__INC__" || funcname == "__DEC__") &&
          register_constant_operation) {
        int cs = std::stoi(std::string(c->right->right->left->tok));
        if (funcname == "__INC__")
          gs.emit(Instruction::Add(tgt, arglocs[0], cs));
        else
          gs.emit(Instruction::Add(tgt, arglocs[0], -cs));
        break;
      }

      if (gs.funcAddrs.find(funcname) ==
          gs.funcAddrs.end()) {  // is there such a function?
        gs.err(CodegenResult::Error::Type::UNKNOWN_PROGRAM_NAME,
               "unknown name " + funcname);
        return;
      }

      Prog p = gs.funcAddrs[funcname];

      if (p.argnum != arglocs.size()) {
        gs.err(CodegenResult::Error::Type::ARGSIZE_MISMATCH,
               "expected " + std::to_string(p.argnum) + " arguments but got " +
                   std::to_string(arglocs.size()));
        return;
      }

      // call sequence
      gs.emit(Instruction::PrepareExec(p.stack_size, p.mi, tgt));
      for (size_t arg = 0; arg < arglocs.size(); arg++) {
        gs.emit(Instruction::Arg(arg, arglocs[arg]));
        gs.getSymbols().releaseTemporary(arglocs[arg]);
      }
      gs.emit(Instruction::Exec(p.ind));

      break;
    }
    default: {
      gs.err(CodegenResult::Error::Type::MALFORMED_AST,
             "node type " + std::to_string((int)c->t) +
                 " unimplemented in dispatchValue()");
      break;
    }
  };
}

void dispatchAssign(GenState &gs, Node *c) {
  std::string target = std::string(c->left->tok);
  RegisterIndex tind = gs.getSymbols().fetchVariableRegister(target);

  dispatchValue(gs, c->right, tind);
}

// dispatch an AST node that corresponds to a <element> in the source
void dispatchVoid(GenState &gs, Node *c) {
  if (c == NULL) return;

  gs.advanceLine(c->line, c->file);

  switch (c->t) {
    case Node::Type::SPLIT: {
      dispatchVoid(gs, c->left);
      dispatchVoid(gs, c->right);
      break;
    }
    case Node::Type::PROGRAM: {
      gs.removeTopPotBreak();
      dispatchProgram(gs, c);
      break;
    }
    case Node::Type::ASSIGN: {
      dispatchAssign(gs, c);
      break;
    }
    case Node::Type::LOOP: {
      dispatchLoop(gs, c);
      break;
    }
    case Node::Type::WHILE: {
      dispatchWhile(gs, c);
      break;
    }
    case Node::Type::MARK: {
      dispatchMark(gs, c);
      break;
    }
    case Node::Type::GOTO: {
      dispatchGoto(gs, c);
      break;
    }
    case Node::Type::IF: {
      dispatchIf(gs, c);
      break;
    }
    case Node::Type::STOP: {
      gs.emit(Instruction::Halt());
      break;
    }
    default: {
      gs.err(CodegenResult::Error::Type::MALFORMED_AST,
             "node type " + std::to_string((int)c->t) +
                 " unimplemented in dispatchVoid()");
      break;
    }
  }
}

void gen_ast(GenState &gs) {
  if (!gs.in.parsed_correctly) {  // file loaded but parsed with errors
    for (auto e : gs.in.errors) {
      gs.verr(CodegenResult::Error::Type::PARSE_ERROR, e.msg, e.file, e.line);
    }
    return;
  }
  dispatchVoid(gs, gs.in.root);
}

CodegenResult Theo::gen(Theo::AST in) {
  GenState gs = {
      .in = in,
      .out =
          {
              .code = {},
              .stack_maps = {},
              .potential_breaks = {},
              .line_info = {},
          },
      .errors = {},
      .symbols = {},
      .funcAddrs = {},
      .fs =
          {
              .name = "#root_file_context",
              .line = 0,
          },
  };

  // first prep instruction, args determined later
  gs.emit(Instruction::PrepareExec(-1, -1, 0));

  gs.pushSymbols("#root");  // symbol table of root script
  gen_ast(gs);
  gs.popSymbols(0);  // finish root function

  Prog p = gs.funcAddrs["#root"];
  gs.out.code[0].parameters.prepare.count = p.stack_size;
  gs.out.code[0].parameters.prepare.index = p.mi;

  gs.emit(Instruction::Halt());
  gs.backpatch();

  return {
      .generated_correctly = gs.errors.size() == 0,
      .errors = gs.errors,
      .code = gs.out,
  };
}
