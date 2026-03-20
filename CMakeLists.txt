#############################################################################
#Copyright (c) 2026,
#
#Distributed under the terms of the BSD 3-Clause License.
#
#The full license is in the file LICENSE, distributed with this software.
#############################################################################

cmake_minimum_required(VERSION 3.24.0)
project(xeus-brainfuck)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

set(XEUS_BRAINFUCK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Versionning
# ===========

file(STRINGS "${XEUS_BRAINFUCK_INCLUDE_DIR}/xeus-brainfuck/xeus_brainfuck_config.hpp" version_defines
     REGEX "#define XEUS_BRAINFUCK_VERSION_(MAJOR|MINOR|PATCH)")
foreach (ver ${version_defines})
    if (ver MATCHES "#define XEUS_BRAINFUCK_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XEUS_BRAINFUCK_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif ()
endforeach ()
set(${PROJECT_NAME}_VERSION
    ${XEUS_BRAINFUCK_VERSION_MAJOR}.${XEUS_BRAINFUCK_VERSION_MINOR}.${XEUS_BRAINFUCK_VERSION_PATCH})
message(STATUS "Building xeus-brainfuck v${${PROJECT_NAME}_VERSION}")

# Configuration
# =============

include(GNUInstallDirs)

if (NOT DEFINED XEUS_BRAINFUCK_KERNELSPEC_PATH)
    set(XEUS_BRAINFUCK_KERNELSPEC_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/")
endif ()

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xbrainfuck/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xbrainfuck/kernel.json"
)

# Build options
# =============

option(XEUS_BRAINFUCK_BUILD_STATIC "Build xeus-brainfuck static library" ON)
OPTION(XEUS_BRAINFUCK_BUILD_SHARED "Split xbrainfuck build into executable and library" ON)
OPTION(XEUS_BRAINFUCK_BUILD_EXECUTABLE "Build the xbrainfuck executable" ON)

OPTION(XEUS_BRAINFUCK_USE_SHARED_XEUS "Link xbrainfuck  with the xeus shared library (instead of the static library)" ON)
OPTION(XEUS_BRAINFUCK_USE_SHARED_XEUS_BRAINFUCK "Link xbrainfuck  with the xeus shared library (instead of the static library)" ON)

if(EMSCRIPTEN)
    add_compile_definitions(XEUS_BRAINFUCK_EMSCRIPTEN_WASM_BUILD)
    message("Build with emscripten")
    set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
    SET(XEUS_BRAINFUCK_BUILD_STATIC ON)
    SET(XEUS_BRAINFUCK_BUILD_SHARED OFF)
    SET(XEUS_BRAINFUCK_BUILD_EXECUTABLE OFF)
    SET(XEUS_BRAINFUCK_USE_SHARED_XEUS OFF)
    SET(XEUS_BRAINFUCK_USE_SHARED_XEUS_BRAINFUCK OFF)
    SET(XEUS_BRAINFUCK_BUILD_TESTS OFF)
endif()


# Dependencies
# ============

set(xeus_zmq_REQUIRED_VERSION 4.0.0)

set(xeus_lite_REQUIRED_VERSION 5.0.0)
if (EMSCRIPTEN)
    if (NOT TARGET xeus-lite)
        find_package(xeus-lite ${xeus_lite_REQUIRED_VERSION} REQUIRED)
    endif()
else()
    if (NOT TARGET xeus-zmq AND NOT TARGET xeus-zmq-static)
        find_package(xeus-zmq ${xeus_zmq_REQUIRED_VERSION} REQUIRED)
    endif()
endif()


# Source files
# ============

set(XEUS_BRAINFUCK_HEADERS
    include/xeus-brainfuck/xbrainfuck.hpp
    include/xeus-brainfuck/xeus_brainfuck_config.hpp
    include/xeus-brainfuck/xinterpreter.hpp
)

set(XEUS_BRAINFUCK_SRC
    src/xbrainfuck.cpp
    src/xinterpreter.cpp
)

set(XEUS_BRAINFUCK_MAIN_SRC
    src/main.cpp
)

set(XEUS_BRAINFUCK_MAIN_WASM_SRC
    src/main_emscripten_kernel.cpp
)


# Targets and link - Macros
# =========================

include(CheckCXXCompilerFlag)

string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib; ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

macro(xeus_brainfuck_set_common_options target_name)
    if (MSVC)
        target_compile_options(${target_name} PUBLIC /wd4251 /wd4141)
        target_compile_options(${target_name} PUBLIC /wd4018 /wd4267 /wd4715 /wd4146 /wd4129)
    endif ()

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
        CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        CMAKE_CXX_COMPILER_ID MATCHES "Intel")

        target_compile_options(${target_name} PUBLIC -Wunused-parameter -Wextra -Wreorder)

        # C++17 flag
        CHECK_CXX_COMPILER_FLAG("-std=c++17" HAS_CPP_17_FLAG)
        if (HAS_CPP_17_FLAG)
            target_compile_features(${target_name} PRIVATE cxx_std_17)
        else ()
            message(FATAL_ERROR "Unsupported compiler -- xeus-brainfuck requires C++17 support!")
        endif ()
    endif ()

    if (APPLE)
        set_target_properties(${target_name} PROPERTIES
            MACOSX_RPATH ON
        )
    else ()
        set_target_properties(${target_name} PROPERTIES
            BUILD_WITH_INSTALL_RPATH 1
            SKIP_BUILD_RPATH FALSE
        )
    endif ()

    set_target_properties(${target_name} PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endmacro()

# Common macro kernels (xbrainfuck)
macro(xeus_brainfuck_set_kernel_options target_name)
    if (XEUS_BRAINFUCK_USE_SHARED_XEUS_BRAINFUCK)
        target_link_libraries(${target_name} PRIVATE xeus-brainfuck)
        if(CMAKE_DL_LIBS)
            target_link_libraries(${target_name} PRIVATE ${CMAKE_DL_LIBS} util)
        endif()
    else ()
        target_link_libraries(${target_name} PRIVATE xeus-brainfuck-static)
    endif()

    if(NOT EMSCRIPTEN)
        find_package(Threads)
        target_link_libraries(${target_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    endif()
    
endmacro()

# Common macro for shared and static library
macro(xeus_brainfuck_create_target target_name linkage output_name)
    string(TOUPPER "${linkage}" linkage_upper)

    if (NOT ${linkage_upper} MATCHES "^(SHARED|STATIC)$")
        message(FATAL_ERROR "Invalid library linkage: ${linkage}")
    endif ()

    add_library(${target_name} ${linkage_upper} ${XEUS_BRAINFUCK_SRC} ${XEUS_BRAINFUCK_HEADERS})
    xeus_brainfuck_set_common_options(${target_name})

    set_target_properties(${target_name} PROPERTIES
                          PUBLIC_HEADER "${XEUS_BRAINFUCK_HEADERS}"
                          PREFIX ""
                          VERSION ${${PROJECT_NAME}_VERSION}
                          SOVERSION ${XEUS_BRAINFUCK_VERSION_MAJOR}
                          OUTPUT_NAME "lib${output_name}")

    target_compile_definitions(${target_name} PUBLIC "XEUS_BRAINFUCK_EXPORTS")

    target_compile_features(${target_name} PRIVATE cxx_std_17)

    target_include_directories(${target_name}
                               PUBLIC
                               $<BUILD_INTERFACE:${XEUS_BRAINFUCK_INCLUDE_DIR}>
                               $<INSTALL_INTERFACE:include>)

    if (XEUS_BRAINFUCK_USE_SHARED_XEUS)
        set(XEUS_BRAINFUCK_XEUS_TARGET xeus)
    else ()
        set(XEUS_BRAINFUCK_XEUS_TARGET xeus-static)
    endif ()

    target_link_libraries(${target_name} PUBLIC ${XEUS_BRAINFUCK_XEUS_TARGET})
    if (WIN32 OR CYGWIN)
        #
    elseif (APPLE)
        target_link_libraries(${target_name} PRIVATE "-undefined dynamic_lookup")
    endif ()
    if(NOT EMSCRIPTEN)
        find_package(Threads) # TODO: add Threads as a dependence of xeus-static?
        target_link_libraries(${target_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    endif()
    
endmacro()

# xeus-brainfuck
# ==============

set(XEUS_BRAINFUCK_TARGETS "")

if (XEUS_BRAINFUCK_BUILD_SHARED)
    # Build libraries
    xeus_brainfuck_create_target(xeus-brainfuck SHARED xeus-brainfuck)
    list(APPEND XEUS_BRAINFUCK_TARGETS xeus-brainfuck)
endif ()

if (XEUS_BRAINFUCK_BUILD_STATIC)
    # On Windows, a static library should use a different output name
    # to avoid the conflict with the import library of a shared one.
    if (CMAKE_HOST_WIN32)
        xeus_brainfuck_create_target(xeus-brainfuck-static STATIC xeus-brainfuck-static)
    else ()
        xeus_brainfuck_create_target(xeus-brainfuck-static STATIC xeus-brainfuck)
    endif ()
    if(EMSCRIPTEN)
        target_compile_options(xeus-brainfuck-static PRIVATE -fPIC)
    endif()
    list(APPEND XEUS_BRAINFUCK_TARGETS xeus-brainfuck-static)
endif ()

# xbrainfuck
# =======
if (XEUS_BRAINFUCK_BUILD_EXECUTABLE)
    find_package(xeus-zmq 1.0.2 REQUIRED)
    add_executable(xbrainfuck ${XEUS_BRAINFUCK_MAIN_SRC})
    target_compile_features(xbrainfuck PRIVATE cxx_std_17)
    xeus_brainfuck_set_common_options(xbrainfuck)
    xeus_brainfuck_set_kernel_options(xbrainfuck)
    target_link_libraries(xbrainfuck PRIVATE xeus-zmq)
endif()

if(EMSCRIPTEN)
    include(WasmBuildOptions)
    find_package(xeus-lite REQUIRED)
    add_executable(xbrainfuck src/main_emscripten_kernel.cpp )
    target_link_libraries(xbrainfuck PRIVATE xeus-lite)
    target_compile_options(xbrainfuck PRIVATE -fPIC)
    XEUS_BRAINFUCK_set_kernel_options(xbrainfuck)
    xeus_wasm_compile_options(xbrainfuck)
    xeus_wasm_link_options(xbrainfuck "web,worker")
endif()


# Installation
# ============
include(CMakePackageConfigHelpers)

set(XEUS_BRAINFUCK_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for xeus-brainfuckConfig.cmake")

# Install xeus-brainfuck and xeus-brainfuck-static
if (XEUS_BRAINFUCK_BUILD_SHARED)
    install(TARGETS ${XEUS_BRAINFUCK_TARGETS}
            EXPORT ${PROJECT_NAME}-targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xeus-brainfuck)

    # Makes the project importable from the build directory
    export(EXPORT ${PROJECT_NAME}-targets
           FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
endif ()

# Install xbrainfuck
if (XEUS_BRAINFUCK_BUILD_EXECUTABLE)
    install(TARGETS xbrainfuck
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if (XEUS_BRAINFUCK_BUILD_EXECUTABLE OR EMSCRIPTEN)
    # Configuration and data directories for jupyter and xeus-brainfuck
    set(XJUPYTER_DATA_DIR "share/jupyter"    CACHE STRING "Jupyter data directory")

    # Install xbrainfuck Jupyter kernelspec
    set(KERNELSPEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels)
    install(DIRECTORY ${KERNELSPEC_DIR}
            DESTINATION ${XJUPYTER_DATA_DIR}
            PATTERN "*.in" EXCLUDE)


    # Extra path for installing Jupyter kernelspec
    if (XEXTRA_JUPYTER_DATA_DIR)
        install(DIRECTORY ${KERNELSPEC_DIR}
                DESTINATION ${XEXTRA_JUPYTER_DATA_DIR}
                PATTERN "*.in" EXCLUDE)
    endif ()
endif ()

# Configure 'xeus-brainfuckConfig.cmake' for a build tree
set(XEUS_BRAINFUCK_CONFIG_CODE "####### Expanded from \@XEUS_BRAINFUCK_CONFIG_CODE\@ #######\n")
set(XEUS_BRAINFUCK_CONFIG_CODE "${XEUS_BRAINFUCK_CONFIG_CODE}set(CMAKE_MODULE_PATH \"${CMAKE_CURRENT_SOURCE_DIR}/cmake;\${CMAKE_MODULE_PATH}\")\n")
set(XEUS_BRAINFUCK_CONFIG_CODE "${XEUS_BRAINFUCK_CONFIG_CODE}##################################################")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${PROJECT_BINARY_DIR})

# Configure 'xeus-brainfuckConfig.cmake.in for an install tree
set(XEUS_BRAINFUCK_CONFIG_CODE "")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XEUS_BRAINFUCK_CMAKECONFIG_INSTALL_DIR})

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              DESTINATION ${XEUS_BRAINFUCK_CMAKECONFIG_INSTALL_DIR})

if (XEUS_BRAINFUCK_BUILD_SHARED)
    install(EXPORT ${PROJECT_NAME}-targets
            FILE ${PROJECT_NAME}Targets.cmake
            DESTINATION ${XEUS_BRAINFUCK_CMAKECONFIG_INSTALL_DIR})
endif ()

if(EMSCRIPTEN)
    install(TARGETS xbrainfuck
            ARCHIVE DESTINATION ${XEUS_BRAINFUCK_CMAKECONFIG_INSTALL_DIR})

    install(FILES
            "$<TARGET_FILE_DIR:xbrainfuck>/xbrainfuck.js"
            "$<TARGET_FILE_DIR:xbrainfuck>/xbrainfuck.wasm"
            DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

