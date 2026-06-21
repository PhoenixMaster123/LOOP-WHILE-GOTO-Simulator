qt_add_library(TheoIDEModels)

qt_add_qml_module(
    TheoIDEModels
    VERSION 1.0
    URI TheoIDE.Models
    SOURCES
        compilerservice.cpp include/compilerservice.hpp
        dialogbutton.cpp include/dialogbutton.hpp
        dialogservice.cpp include/dialogservice.hpp
        filesystemservice.cpp include/filesystemservice.hpp
        messagedialogdto.cpp include/messagedialogdto.hpp
        virtualmachineservice.cpp include/virtualmachineservice.hpp
)

target_include_directories(
    TheoIDEModels
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/
)

target_link_libraries(
    TheoIDEModels
    PRIVATE Qt::Core Qt::Qml Qt::Quick Qt::Concurrent
    PUBLIC TheoVM TheoC
)
