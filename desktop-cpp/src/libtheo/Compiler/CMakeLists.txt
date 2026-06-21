find_package(FLEX)

set(GENERATED_OUTPUTS ${CMAKE_CURRENT_SOURCE_DIR}/src/lex.yy.c)

# if the environment has flex installed, regenerate the initial scanner on build
# the files are also pregenerated in the source tree to avoid having flex
# as a dependency on every platform
if(FLEX_FOUND)
    add_custom_command(
        OUTPUT ${GENERATED_OUTPUTS}
        COMMAND
            flex --outfile="./src/lex.yy.c" --header-file="./include/lex.yy.h"
            --noline --nounistd "./src/lexer.l"
        DEPENDS src/lexer.l
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

set(LIBTHEO_C_HEADERS
    include/ast.hpp
    include/parse.hpp
    include/lexer.hpp
    include/gen.hpp
    include/compiler.hpp
    include/scanner_info.hpp
    include/token.hpp
    include/scan.hpp
    include/macro.hpp
    include/ParserGenerator/grammar.hpp
    include/ParserGenerator/lrdea.hpp
    include/ParserGenerator/lrparser.hpp
)

set(LIBTHEO_C_SOURCES
    src/ast.cpp
    src/parse.cpp
    src/gen.cpp
    src/compiler.cpp
    src/scan.cpp
    src/macro.cpp
    src/ParserGenerator/grammar.cpp
    src/ParserGenerator/lrdea.cpp
)

set_source_files_properties(${GENERATED_OUTPUTS} PROPERTIES LANGUAGE CXX)
add_library(
    TheoC
    ${LIBTHEO_C_HEADERS}
    ${LIBTHEO_C_SOURCES}
    ${GENERATED_OUTPUTS}
)

target_include_directories(
    TheoC
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/Compiler/include # needed for bison output files to find each other
)

target_link_directories(TheoC PUBLIC ${PROJECT_BINARY_DIR}/VM/)

target_link_libraries(TheoC PUBLIC TheoVM)

add_subdirectory(test)
