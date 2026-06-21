#include <iostream>

#include "Compiler/include/ast.hpp"
#include "Compiler/include/parse.hpp"

int main() {
  std::string math_theo =
      "\
DEFINE PRIO 30\n\
   <ID>(<ARGS>)\n\
AS \n\
   RUN $0 WITH $1 END\n\
END DEFINE\n\
DEFINE PRIO 20\n\
   <V> + <V>\n\
AS \n\
   add($0, $1)\n\
END DEFINE \n\
DEFINE PRIO 25\n\
   <V> * <V>\n\
AS \n\
   mul($0, $1)\n\
END DEFINE \n\
    // OUT is optional and defaults to x0 if not specified\n\
PROGRAM add IN x0, x1 DO\n\
	LOOP x1 DO\n\
		x0 := x0 + 1\n\
	END\n\
	// x0 implicitly treated as return value\n\
END\n\
\n\
// this is a function where a different variable (x2) is used to return values\n\
PROGRAM mul IN x0, x1 OUT x2 DO\n\
	LOOP x1 DO\n\
		// \"+\" is used as infix operator (defined by above function)\n\
		// c-style syntax would look like: x2 := +(x2, x0)\n\
		x2 := x2 + x0 \n\
	END\n\
END\n\
  ";

  std::string main_theo =
      "\n\
INCLUDE \"math.theo\" // works like copy-and-paste, or include in C/C++\n\
\n\
// language-supported instructions\n\
x0 := 10 ;	// constant assignment is a permissive extension\n\
x1 := 5  ;\n\
x0 := x0 + 1 ;	\n\
\n\
// using programs defined in math.theo\n\
x2 := x0 + x1 ;	// infix\n\
x2 := add(x0, x1) ; // c-style\n\
\n\
// note that x3 := x0 + x1 + x2 would NOT be valid syntax; infix calls can only appear top level in expressions to avoid operator-hierachy confusion\n\
x3 := x2 + mul(x0, x1) //this works, as infix is top level and second arg is c-style call						  \n\
						  ";

  Theo::ParseResult pres = Theo::parse(
      {{"main.theo", main_theo}, {"math.theo", math_theo}}, "main.theo");

  bool all_clear = true;

  Theo::AST res = pres.a;

  auto p = res;
  if (!res.parsed_correctly) {
    std::cout << "Syntax Errors:" << std::endl;
    for (auto e : res.errors) {
      std::cout << "File " << e.file << " Line " << e.line << ": " << e.msg
                << std::endl;
    }
    all_clear = false;
  } else {
    std::cout << "AST: " << std::endl;
    res.visualize(std::cout);
  }

  // do not forget to free
  res.clear();

  return all_clear ? 0 : 1;
}
