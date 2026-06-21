include_directories(PUBLIC ${PROJECT_SOURCE_DIR})

link_directories(PUBLIC ${PROJECT_BINARY_DIR}/VM/)

link_libraries(TheoVM)

# instr test
add_executable(instr_test instr_test.cpp)
add_test(NAME instr_test COMMAND instr_test)
