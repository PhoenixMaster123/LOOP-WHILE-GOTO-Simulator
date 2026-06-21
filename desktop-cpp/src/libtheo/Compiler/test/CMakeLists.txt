include_directories(PUBLIC ${PROJECT_SOURCE_DIR})

link_directories(
    PUBLIC
    ${PROJECT_BINARY_DIR}/Compiler/
    ${PROJECT_BINARY_DIR}/VM/
)

link_libraries(TheoC TheoVM)

# parse test
add_executable(parse_test parse_test.cpp)
add_test(NAME parse_test COMMAND parse_test)

# codegen test
add_executable(gen_test gen_test.cpp)
add_test(NAME gen_test COMMAND gen_test)

# scan test
add_executable(scan_test scan_test.cpp)
add_test(NAME scan_test COMMAND scan_test)

# macro extraction test
add_executable(macro_extraction_test macro_extraction_test.cpp)
add_test(NAME macro_extraction_test COMMAND macro_extraction_test)

# grammar test
add_executable(grammar_test grammar_test.cpp)
add_test(NAME grammar_test COMMAND grammar_test)

# test of the hull of LR(1) Elements
add_executable(hull_test hull_test.cpp)
add_test(NAME hull_test COMMAND hull_test)

# test of Jump of set LR(1) Elements
add_executable(jump_test jump_test.cpp)
add_test(NAME jump_test COMMAND jump_test)

# test of sets-of-elements construction
add_executable(elements_test elements_test.cpp)
add_test(NAME elements_test COMMAND elements_test)

# test of LRParser with closed end grammars
add_executable(lrparser_test lrparser_test.cpp)
add_test(NAME lrparser_test COMMAND lrparser_test)

# test if LRParser generates for actual macros
add_executable(macro_compilation_test macro_compilation_test.cpp)
add_test(NAME macro_compilation_test COMMAND macro_compilation_test)

# macro application text
add_executable(macro_application_test macro_application_test.cpp)
add_test(NAME macro_application_test COMMAND macro_application_test)
