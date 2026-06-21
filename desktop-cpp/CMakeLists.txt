cmake_minimum_required(VERSION 3.25)

project(TheoIDE VERSION 0.1.0 LANGUAGES CXX)

set(QT_VERSION 6.7)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_EXECUTABLES TheoIDEApp;TheoIDE)

# debian package settings
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libxcb-cursor0")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "")

# msi installer settings
set(CPACK_WIX_UPGRADE_GUID "A9C4B57E-8F8A-4765-B3B9-FD96DC587C01")

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

include(CPack)

find_package(
    Qt6
    ${QT_VERSION}
    COMPONENTS Core Quick QuickControls2 Sql Svg Concurrent Qml
    REQUIRED
)

qt_standard_project_setup(REQUIRES ${QT_VERSION})

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT
        )
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(
                FATAL_ERROR
                "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules"
            )
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/src/libtheo/CMakeLists.txt")
    message(
        FATAL_ERROR
        "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again."
    )
endif()

add_subdirectory(src/libtheo)
add_subdirectory(src/app)
