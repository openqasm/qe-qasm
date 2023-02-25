execute_process(COMMAND uname -m OUTPUT_VARIABLE OS_UNAME_MACHINE)
string(STRIP ${OS_UNAME_MACHINE} OS_UNAME_MACHINE)

execute_process(COMMAND uname -s OUTPUT_VARIABLE OS_UNAME_SYSTEM)
string(STRIP ${OS_UNAME_SYSTEM} OS_UNAME_SYSTEM)

message(STATUS "Building on ${OS_UNAME_SYSTEM} ${OS_UNAME_MACHINE} architecture.")

string (REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)" _
        ${CMAKE_CXX_COMPILER_VERSION})

set(CLANG_VERSION_MAJOR ${CMAKE_MATCH_1})
set(CLANG_VERSION_MINOR ${CMAKE_MATCH_2})
set(CLANG_VERSION_MICRO ${CMAKE_MATCH_3})
set(CLANG_VERSION_PATCH ${CMAKE_MATCH_4})

set (CLANG_MIN_VERSION_MAJOR 14)
set (CLANG_MIN_VERSION_MINOR 0)
set (CLANG_MIN_VERSION_MICRO 0)

set (CLANG_MIN_REQUIRED_VERSION "14.0.0.14000029")

if (${OS_UNAME_MACHINE} STREQUAL "arm64")
  set (CLANG_MIN_VERSION_PATCH 14000029)
elseif (${OS_UNAME_MACHINE} STREQUAL "x86_64")
  set (CLANG_MIN_VERSION_PATCH 14000029)
else()
  message(FATAL_ERROR "Unrecognized machine type.")
endif()

message(STATUS "Building with AppleClang ${CMAKE_CXX_COMPILER_VERSION}")

if (${OS_UNAME_MACHINE} STREQUAL "arm64")
  set (HOMEBREW_PREFIX "/opt/homebrew")
elseif (${OS_UNAME_MACHINE} STREQUAL "x86_64")
  set (HOMEBREW_PREFIX "/usr/local/opt/llvm@14")
else()
  message(FATAL_ERROR "Unrecognized machine type.")
endif()

message(STATUS "HOMEBREW_PREFIX: ${HOMEBREW_PREFIX}")

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -std=c99 -fno-strict-aliasing -Wall -Wextra -Wshadow")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstack-protector -ffunction-sections -fdata-sections")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fkeep-static-consts -fstack-protector-all -fno-omit-frame-pointer")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -finput-charset=UTF-8 -fextended-identifiers")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstandalone-debug -Wno-deprecated")

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${HOMEBREW_PREFIX}/include")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17 -fno-strict-aliasing -fexceptions -frtti")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -Wall -Wextra -Wshadow -Wno-redundant-move")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstack-protector -fkeep-static-consts  -fdata-sections  -ffunction-sections")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finput-charset=UTF-8 -fextended-identifiers")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstandalone-debug -Wno-deprecated")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-redundant-move -Wno-microsoft-anon-tag")
set (CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS} -Wno-vla -Wno-vla-extension")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${HOMEBREW_PREFIX}/include")

if (${OS_UNAME_MACHINE} STREQUAL "arm64")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target arm64-apple-macos11")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=11.0")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target arm64-apple-macos11")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=11.0")
endif()

set (CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_C_FLAGS} -dynamiclib -undefined dynamic_lookup -L${HOMEBREW_PREFIX}/lib -Wl,-rpath,${HOMEBREW_PREFIX}/lib")
set (CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -dynamiclib -undefined dynamic_lookup -L${HOMEBREW_PREFIX}/lib -Wl,-rpath,${HOMEBREW_PREFIX}/lib")

add_definitions (-D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
add_definitions (-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)

set (CMAKE_C_FLAGS_DEBUG "-g3 -O0")
set (CMAKE_C_FLAGS_RELEASE "-g -O3")

set (CMAKE_CXX_FLAGS_DEBUG "-g3 -O0")
set (CMAKE_CXX_FLAGS_RELEASE "-g -O3")

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIE -O2 -L${HOMEBREW_PREFIX}/lib")

set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC -O2")

set (CMAKE_SUPPRESS_REGENERATION OFF)

