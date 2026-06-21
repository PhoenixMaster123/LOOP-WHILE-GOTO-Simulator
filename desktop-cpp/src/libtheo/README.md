# libTheoC / libTheoVM

This repository contains the compilation API for the Theo-IDE languages (libTheoC) and a virtual machine to execute the resulting bytecode (libTheoVM). It contains an example application that shows how to make use of the APIs in the form of a command line interpreter and debugger (theo).

## Building

To build, you will need a C++20 capable compiler. Building has been verified to work with `gcc 13.2.1` and clang `17.0.1` but should work with any compliant compiler.

### Optional Dependencies

libTheoC includes a FLEX scanner. The repository includes the output of the Linux version of GNU flex. If you want to modify scanner code (`lexer.l`) or the output of the GNU/Linux version doesn't build on your platform, you will need to have flex installed on your system. We used `flex 2.6.4` to generate the included code. If CMake is able to locate your flex installation, it will automatically regenerate the scanner during the build process.

### Build Command

For an out-of-source build, just invoke CMake the usual way:

```
mkdir build
cd build
cmake ..
cmake --build .
```

Run the included tests with `ctest` from the same directory:

```
ctest
```

## Using

A complete user manual and additional documentation can / will be found in the main Theo-IDE repository. But as the API for the compilation / execution is rather simplistic, the documentation comments in the header files of the components will usually suffice.

## libTheoC

libTheoC is intended to be used through a single function found in `Compiler/include/compiler.hpp`, which will translate source code in the form of `std::string` into bytecode which will be accepted by libTheoVM. For example usage, you may study how the cli interpreter / debugger at `CLI/cli.cpp` utilizes the `Theo::compile` function.

## libTheoVM

libTheoVM exposes execution and debugging facilities through the `Theo::VM` class, found in `VM/include/vm.hpp`. VM objects are constructed with the output of libTheoC as parameters and expose methods altering the interpreter state. These methods may execute byte code up to the next breakpoint, modify the set of active breakpoints or give information about the memory contents of the VM, among other things. The feature set of the VM object is tailored to the use in an interactive debugger, such as the one supplied in this repository or the main graphical debugger included in the Theo-IDE. For example usage, you may study how the cli interpreter / debugger at `CLI/cli.cpp` utilizes the methods.

## theo / CLI

As mentioned, the project contains a text-mode interactive interpreter with debugging functionalities, the rather short source code of which can be found in `CLI/cli.cpp`. It is mainly intended to serve as an example for the usage of libTheoC and libTheoVM. After building, the executable is located in the `bin` subfolder of your build structure and may be used in the following way:

If you have some source files (`main.theo`, `add.theo`, where the first file is the main file and has an include directive targeting the second file) located in the same directory as the theo executable, use:

```
./theo main.theo add.theo
```

To execute the program. You will receive the final variable states as an output. Please note that any included file must be stated to the interpreter upon invocation like in the snippet above.

If you want to interactively debug your application, you can invoke the executable in debug mode like this:

```
./theo -d main.theo add.theo
```

You will then be landed at an interactive prompt from which you can enable breakpoints, step through single lines and query the memory state intermittently. Enter the command `h` at the prompt to receive a list of possible commands along with their explanation.
