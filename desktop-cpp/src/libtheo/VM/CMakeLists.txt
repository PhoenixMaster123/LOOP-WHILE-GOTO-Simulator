set(LIBTHEO_VM_HEADERS include/instr.hpp include/vm.hpp include/program.hpp)

set(LIBTHEO_VM_SOURCES src/instr.cpp src/vm.cpp src/program.cpp)

add_library(TheoVM ${LIBTHEO_VM_HEADERS} ${LIBTHEO_VM_SOURCES})

target_include_directories(TheoVM PUBLIC ${PROJECT_SOURCE_DIR})

add_subdirectory(test)
