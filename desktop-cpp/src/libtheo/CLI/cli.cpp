#define CLI_VER "1.1.0"

#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#include "Compiler/include/compiler.hpp"
#include "Compiler/include/gen.hpp"
#include "VM/include/vm.hpp"

using namespace Theo;

void usage() {
  std::cout << "Usage: theo [OPTIONS...] [FILES...]" << std::endl
            << "Compiles and Executes Theo-IDE languages (commonly referred to "
               "as LOOP/WHILE/GOTO)"
            << std::endl
            << "First encountered file name will be treated as main file."
            << std::endl
            << std::endl
            << "OPTIONS:" << std::endl
            << "  -d, --debug\t\tenables interactive debug mode" << std::endl
            << "  -v, --version\t\treport version and license information"
            << std::endl
            << "  -h, --help\t\tproduce this help message" << std::endl;
}

void debug_usage() {
  std::cout << "Commands: " << std::endl
            << "e - execute until next breakpoint" << std::endl
            << "s - step to next line" << std::endl
            << "r - reset vm" << std::endl
            << "m - print memory contents of current program" << std::endl
            << "l - list current position in program" << std::endl
            << "i - like <l> followed by <m> " << std::endl
            << "b <file> <line> - set a breakpoint at specified position"
            << std::endl
            << "d <file> <line> - unset a breakpoint" << std::endl
            << "c - clear all breakpoints" << std::endl
            << "a - list active breakpoints" << std::endl
            << "o - list bytecode" << std::endl
            << "h - print this menu" << std::endl
            << "q - quit" << std::endl;
}

void sectionPrint(std::map<FileName, FileContent> &files, FileName file,
                  int around_line, int padding) {
  if (around_line == -1) return;

  int startLine = around_line - padding;
  int endLine = around_line + padding;

  std::string content = files[file];

  std::stringstream scontent(content);

  std::string c_line;

  int on_line = 1;

  while (on_line <= endLine && !scontent.eof()) {
    std::getline(scontent, c_line);
    if (on_line >= startLine) {
      if (on_line != around_line) {
        std::cout << on_line << " | " << c_line << std::endl;
      } else {
        std::cout << on_line << " | \033[1;32m" << c_line << "\033[0m"
                  << std::endl;
      }
    }
    on_line++;
  }
}

void debug_mode(VM &v, std::map<FileName, FileContent> &files,
                CodegenResult &cr) {
  std::cout << "debug mode, type 'h' and enter for a list of commands"
            << std::endl;
  bool running = true;
  while (running) {
    std::cout << ">>";
    std::string cmd;
    std::getline(std::cin, cmd);
    if (cmd == "h") debug_usage();
    if (cmd == "q") running = false;
    if (cmd == "e") v.execute();
    if (cmd == "r") v.reset();
    if (cmd == "l" || cmd == "i") {
      BreakPoint bp = v.getCurrentBreak();
      std::cout << "program is held on breakpoint '" << bp.file << ":"
                << bp.line << "'" << std::endl;
      sectionPrint(files, bp.file, bp.line, 5);
    }
    if (cmd == "m" || cmd == "i") {
      std::cout << "memory:" << std::endl;
      auto activations = v.getActivations();
      if (activations.size() == 0) {
        std::cout << "nothing to print, execution hasn't started" << std::endl;
        continue;
      }
      auto data = activations.back().getActivationVariables();
      for (auto p : data) {
        std::cout << p.first << ": " << p.second << std::endl;
      }
    }
    if (cmd == "s") {
      v.setSteppingMode(true);
      v.execute();
      v.setSteppingMode(false);
    }
    if (cmd[0] == 'b' || cmd[0] == 'd') {
      std::stringstream sstr(cmd);
      std::string t, _file, _line;
      sstr >> t >> _file >> _line;
      int _iline = -1;
      try {
        _iline = std::stoi(_line);
      } catch (std::exception &e) {
        std::cout << "invalid number as second argument" << std::endl;
      }
      bool mode = cmd[0] == 'b' ? true : false;
      bool res = v.setBreakPoint(_file, _iline, mode);
      if (!res)
        std::cout << "no possible breakpoint in file '" << _file << "' at line "
                  << _iline << std::endl;
    }
    if (cmd == "c") {
      v.clearBreakpoints();
      std::cout << "breakpoints cleared" << std::endl;
    }
    if (cmd == "a") {
      std::cout << "activated breakpoints: " << std::endl;
      for (auto b : v.getEnabledBreakPoints()) {
        std::cout << "- " << b.file << ":" << b.line << std::endl;
      }
    }
    if (cmd == "o") {
      cr.code.disassemble(std::cout);
    }
  }
}

void print_version() {
  std::cout
      << "Theo-IDE Command Line Interpreter / Debuger " << CLI_VER << std::endl
      << "Copyright (C) 2024 Peter Preinesberger" << std::endl
      << "License GPLv3: GNU GPL version 3" << std::endl
      << "This is free software: you are free to change and redistribute it."
      << std::endl
      << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc < 2) usage();

  for (int i = 1; i < argc; i++) {
    std::string cArg = argv[i];
    if (cArg == "-h" || cArg == "--help") {
      usage();
      return 0;
    }
    if (cArg == "-v" || cArg == "--version") {
      print_version();
      return 0;
    }
  }

  bool enable_debug = false;

  std::string mainFile = "";
  std::map<FileName, FileContent> files = {};

  for (int i = 1; i < argc; i++) {
    std::string cArg = argv[i];
    if (cArg == "-d" || cArg == "--debug") {
      enable_debug = true;
      continue;
    }

    if (cArg[0] == '-') continue;

    // read in file
    std::ifstream f(cArg);
    if (!f.is_open()) {
      std::cout << "Warning: Could'nt open file '" << cArg << "'" << std::endl;
      continue;
    }

    if (mainFile == "") mainFile = cArg;

    std::stringstream sbf;
    sbf << f.rdbuf();

    files[cArg] = sbf.str();
  }

  if (mainFile == "") {
    std::cout << "No files were opened, aborting." << std::endl;
    return 1;
  }

  CodegenResult cr = compile(files, mainFile);

  if (!cr.generated_correctly) {
    std::cout << "Compilation Errors: " << std::endl;
    for (auto e : cr.errors) {
      std::cout << "[" << (int)e.t << "] "
                << "in '" << e.file << "', line " << e.line << " '" << e.message
                << "'" << std::endl;
    }
    return 1;
  }

  VM v(cr.code);

  if (enable_debug) {
    debug_mode(v, files, cr);
  } else {
    v.execute();
    std::cout << "variables after execution:" << std::endl;
    auto data = v.getActivations().back().getActivationVariables();
    for (auto p : data) {
      std::cout << p.first << ": " << p.second << std::endl;
    }
  }

  return 0;
}
