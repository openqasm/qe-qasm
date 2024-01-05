cmake_minimum_required(VERSION 3.17.0)

if(POLICY CMP0068)
  cmake_policy(SET CMP0068 NEW)
  set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON)
endif()
# Allow target_link_libraries() from other directories (since 3.13):
#   https://cmake.org/cmake/help/v3.13/policy/CMP0079.html
if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0075)
  cmake_policy(SET CMP0075 NEW)
endif()

if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

find_package(mpc REQUIRED)
find_package(mpfr REQUIRED)
find_package(gmp REQUIRED)

# Version information
if(DEFINED ENV{VERSION_STRING})
    set(VERSION_STRING $ENV{VERSION_STRING})
else()
  set(VERSION_FILE "${CMAKE_SOURCE_DIR}/VERSION.txt")
  if (EXISTS "${VERSION_FILE}")
    file(READ ${VERSION_FILE} VERSION_STRING)
  else()
    set(VERSION_STRING "0.0.1")
    message("Could not parse version file from: ${VERSION_FILE}")
    message("Defaulting to ${VERSION_STRING}")
  endif()

  string(REGEX MATCHALL "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" _ ${VERSION_STRING})
  set(OPENQASM_VERSION_MAJOR ${CMAKE_MATCH_1})
  set(OPENQASM_VERSION_MINOR ${CMAKE_MATCH_2})
  set(OPENQASM_VERSION_PATCH ${CMAKE_MATCH_3})
  set(OPENQASM_VERSION_TRIPLE ${OPENQASM_VERSION_MAJOR}.${OPENQASM_VERSION_MINOR}.${OPENQASM_VERSION_PATCH})
endif()

message(STATUS "Raw Version: ${VERSION_STRING}")

# Version information
string(REGEX MATCHALL "^([0-9]+)\\.([0-9]+)\\.([0-9]+)([a-zA-Z0-9\+_.-])*" _ ${VERSION_STRING})
set(OPENQASM_VERSION_MAJOR ${CMAKE_MATCH_1})
set(OPENQASM_VERSION_MINOR ${CMAKE_MATCH_2})
set(OPENQASM_VERSION_PATCH ${CMAKE_MATCH_3})
set(OPENQASM_VERSION_TRIPLE ${OPENQASM_VERSION_MAJOR}.${OPENQASM_VERSION_MINOR}.${OPENQASM_VERSION_PATCH})
message(STATUS "Parsed version: ${OPENQASM_VERSION_TRIPLE}")


# Build type
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

enable_language(C CXX ASM)

# Compiler options
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/apple-clang.cmake)
    else()
        include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/llvm-clang.cmake)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      message(FATAL_ERROR "Building with GCC on MacOS is not supported.")
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      include(${CMAKE_SOURCE_DIR}/cmake/linux-gcc.cmake)
    else()
      message(FATAL_ERROR "Unsupported operating system.")
    endif()
endif()

project(OPENQASM VERSION "${OPENQASM_VERSION_TRIPLE}")
set(CMAKE_C_STANDARD 99 CACHE STRING "Building for C99")
set(CMAKE_CXX_STANDARD 17 CACHE STRING "Building for C++17")
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckIncludeFileCXX)
include(CheckTypeSize)
include(CheckStructHasMember)
include(CheckCXXSymbolExists)
include(GNUInstallDirs)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, defaulting to Debug")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type Debug (default)." FORCE)
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" UPPERCASE_CMAKE_BUILD_TYPE)
message(STATUS "Building in ${UPPERCASE_CMAKE_BUILD_TYPE} mode.")

set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
                      ${CMAKE_MODULE_PATH})

message(STATUS "OpenQASM Version: ${OPENQASM_VERSION_TRIPLE}")
set(PROJECT_SOVERSION 0)

# Find GMP
find_package(gmp 5.0.0 REQUIRED)
include_directories(BEFORE SYSTEM ${gmp_INCLUDES})

# Find MPC
find_package(mpc 1.2.0 REQUIRED)
include_directories(BEFORE SYSTEM ${mpc_INCLUDES})

# Find MPFR
find_package(mpfr 4.0.0 REQUIRED)
include_directories(BEFORE SYSTEM ${mpfr_INCLUDES})

# Find BISON
find_package(BISON 3.6.0 REQUIRED)
include_directories(BEFORE SYSTEM ${bison_INCLUDE_DIRS})
if (NOT DEFINED BISON_EXECUTABLE)
  find_program(BISON_EXECUTABLE bison)
endif()
message(STATUS "Bison executable: ${BISON_EXECUTABLE}")

# Find M4
find_program(M4 m4)

# Find FLEX
find_package(FLEX 2.6.0 REQUIRED)

if (NOT DEFINED FLEX_EXECUTABLE)
  find_program(FLEX_EXECUTABLE flex)
endif()

# Workaround for the conan flex package not providing
if (FLEX_INCLUDE_DIR STREQUAL "FLEX_INCLUDE_DIR-NOTFOUND")
  # bin
  get_filename_component(FLEX_PATH ${FLEX_EXECUTABLE} DIRECTORY)
  # conan folder
  get_filename_component(FLEX_PATH ${FLEX_PATH} DIRECTORY)
  set(FLEX_INCLUDE_DIR ${FLEX_PATH}/include)
endif()

include_directories(BEFORE SYSTEM ${FLEX_INCLUDE_DIR})

message(STATUS "Flex include dir: ${FLEX_INCLUDE_DIR}")
message(STATUS "Flex executable: ${FLEX_EXECUTABLE}")

set(OPENQASM_CCACHE_BUILD OFF CACHE BOOL "Set to ON for a ccache enabled build")
if(OPENQASM_CCACHE_BUILD)
  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
    set(OPENQASM_CCACHE_MAXSIZE "" CACHE STRING "Size of ccache")
    set(OPENQASM_CCACHE_DIR "" CACHE STRING "Directory to keep ccached data")
    set(OPENQASM_CCACHE_PARAMS "CCACHE_CPP2=yes CCACHE_HASHDIR=yes"
        CACHE STRING "Parameters to pass through to ccache")

    set(CCACHE_PROGRAM "${OPENQASM_CCACHE_PARAMS} ${CCACHE_PROGRAM}")
    if (OPENQASM_CCACHE_MAXSIZE)
      set(CCACHE_PROGRAM
          "CCACHE_MAXSIZE=${OPENQASM_CCACHE_MAXSIZE} ${CCACHE_PROGRAM}")
    endif()
    if (OPENQASM_CCACHE_DIR)
      set(CCACHE_PROGRAM "CCACHE_DIR=${OPENQASM_CCACHE_DIR} ${CCACHE_PROGRAM}")
    endif()
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
  else()
    message(FATAL_ERROR "Unable to find the program ccache. Set OPENQASM_CCACHE_BUILD to OFF")
  endif()
endif()

option(OPENQASM_CONAN_BUILD "Building qss-qasm with Conan" OFF)
option(OPENQASM_RUN_UNITTEST "Automatically run unit-tests as a post build step" OFF)
option(OPENQASM_WERROR "Force compilation to fail if a warning occurs" OFF)
option(OPENQASM_STRICT_ISO "Issue all the warnings demanded by strict ISO C and ISO C++" ON)
option(OPENQASM_PKGCONFIG_SUPPORT "Generate and install .pc files" ON)
option(OPENQASM_CMAKE_PACKAGE "Generate and install cmake package files" ON)
option(OPENQASM_BUILD_EXAMPLES "Compile OpenQASM examples." ON)
option(BUILD_SHARED_LIBS "Build shared libraries." ON)

if (OPENQASM_CONAN_BUILD)
  option(BUILD_STATIC_LIBS "Build static libraries." OFF)
else()
  option(BUILD_STATIC_LIBS "Build static libraries." ON)
endif()

option(OPENQASM_WITH_TESTS "Enable CTest harness." ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  option(OPENQASM_BISON_DEBUG "Compile Bison files in debug mode." ON)
  option(OPENQASM_FLEX_DEBUG "Compile Flex files in debug mode." ON)
else()
  option(OPENQASM_BISON_DEBUG "Compile Bison files in release mode." OFF)
  option(OPENQASM_FLEX_DEBUG "Compile Flex files in release mode." OFF)
endif()

set(OPENQASM_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(OPENQASM_INCLUDE_DIR "${OPENQASM_DIR}/include"
    CACHE PATH "" FORCE)
set(OPENQASM_LIB_DIR "${OPENQASM_DIR}/lib"
    CACHE PATH "" FORCE)

# Compute binary directory from base path.
file(RELATIVE_PATH
  REL_BASE_PATH
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(OPENQASM_BINARY_DIR ${CMAKE_BINARY_DIR}/${REL_BASE_PATH})
message(STATUS "OPENQASM_BINARY_DIR: ${OPENQASM_BINARY_DIR}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OPENQASM_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OPENQASM_BINARY_DIR}/lib")
set(CMAKE_PDB_OUTPUT_DIRECTORY     "${OPENQASM_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OPENQASM_BINARY_DIR}/bin")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wshadow)
  if(OPENQASM_STRICT_ISO)
    add_compile_options(-Wpedantic)
    add_compile_options(-Wno-vla)
    add_compile_options(-Wno-vla-extension)
  endif()
  if(OPENQASM_WERROR)
    add_compile_options(-Werror)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-Wall -Wextra -Wshadow)
  if(OPENQASM_STRICT_ISO)
    add_compile_options(-Wpedantic)
    add_compile_options(-Wno-vla)
  endif()
  if(OPENQASM_WERROR)
    add_compile_options(-Werror)
  endif()
endif()

if(OPENQASM_CMAKE_PACKAGE)
  include(CMakePackageConfigHelpers)
  install(EXPORT OpenQASM
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OpenQASM
          FILE        OpenQASMConfig.cmake)
  write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/OpenQASMConfigVersion.cmake"
                                   VERSION "${OPENQASM_VERSION_TRIPLE}"
                                   COMPATIBILITY SameMajorVersion)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/OpenQASMConfigVersion.cmake
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OpenQASM)
endif()

if(OPENQASM_WITH_TESTS)
  enable_testing()
  include(CTest)
endif()

set(OPENQASM_TARGET_LICENSING QasmLicensing)

set(OPENQASM_TARGET_PARSER_SHARED QasmParserShared)
set(OPENQASM_TARGET_PARSER_STATIC QasmParserStatic)

set(OPENQASM_TARGET_AST_SHARED QasmASTShared)
set(OPENQASM_TARGET_AST_STATIC QasmASTStatic)

set(OPENQASM_TARGET_FRONTEND_SHARED QasmFrontendShared)
set(OPENQASM_TARGET_FRONTEND_STATIC QasmFrontendStatic)

set(OPENQASM_TARGET_DIAG_SHARED QasmDiagShared)
set(OPENQASM_TARGET_DIAG_STATIC QasmDiagStatic)

if(BUILD_SHARED_LIBS)
  set(OPENQASM_LIBRARIES
    ${OPENQASM_TARGET_PARSER_SHARED}
    ${OPENQASM_TARGET_AST_SHARED}
    ${OPENQASM_TARGET_FRONTEND_SHARED}
    ${OPENQASM_TARGET_DIAG_SHARED}
  )
elseif(BUILD_STATIC_LIBS)
  set(OPENQASM_LIBRARIES
    ${OPENQASM_TARGET_PARSER_STATIC}
    ${OPENQASM_TARGET_AST_STATIC}
    ${OPENQASM_TARGET_FRONTEND_STATIC}
    ${OPENQASM_TARGET_DIAG_STATIC}
  )
else()
  # Don't add examples target if no libraries are being built.
  set(OPENQASM_BUILD_EXAMPLES OFF)
endif()

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(examples)
add_subdirectory(tests)
