set(BISON_FLAGS --verbose -t -d -l --language=c++ --feature=diagnostics-show-caret)
set(FLEX_FLAGS --yylineno --8bit -L --c++)

if(UPPERCASE_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
  set(BISON_FLAGS ${BISON_FLAGS} --debug)
  set(FLEX_FLAGS --debug ${FLEX_FLAGS})
endif()

message(STATUS "OPENQASM_BINARY_DIR: ${OPENQASM_BINARY_DIR}")
message(STATUS "OPENQASM_INCLUDE_DIR: ${OPENQASM_INCLUDE_DIR}")
message(STATUS "OPENQASM_LIB_DIR: ${OPENQASM_LIB_DIR}")
message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

set(BISON_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/QasmParser.tab.cpp")
set(BISON_DEFINES "${CMAKE_CURRENT_BINARY_DIR}/QasmParser.tab.h")
set(BISON_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/location.hh")
set(PARSER_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/QasmParser.output")
set(BISON_OUTPUT_DONE "${CMAKE_CURRENT_BINARY_DIR}/.QasmParser.tab.cpp.done")
set(BISON_DEFINES_DONE "${CMAKE_CURRENT_BINARY_DIR}/.QasmParser.tab.h.done")
set(BISON_LOCATION_DONE "${CMAKE_CURRENT_BINARY_DIR}/.location.hh.done")

set(BISON_FILE "${OPENQASM_LIB_DIR}/Parser/QasmParser.y")
set(BISON_ARGS --defines=${BISON_DEFINES} ${BISON_FILE} --output=${BISON_OUTPUT})

if (BISON_FOUND)
  if (UPPERCASE_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    add_custom_command(OUTPUT ${BISON_OUTPUT} ${BISON_DEFINES} ${BISON_LOCATION}
                              ${PARSER_OUTPUT}
                       COMMAND ${BISON_EXECUTABLE}
                       ARGS  ${BISON_FLAGS} ${BISON_ARGS}
                       BYPRODUCTS ${BISON_OUTPUT} ${BISON_DEFINES} ${BISON_LOCATION}
                                  ${PARSER_OUTPUT}
                       COMMAND ${OPENQASM_DIR}/utils/sed-bison-output.sh
                       ARGS ${CMAKE_CURRENT_BINARY_DIR}
                       VERBATIM
                       COMMENT "Generating QasmParser.tab.cpp")
  else()
    add_custom_command(OUTPUT ${BISON_OUTPUT} ${BISON_DEFINES} ${BISON_LOCATION}
                              ${PARSER_OUTPUT}
                       COMMAND ${BISON_EXECUTABLE}
                       ARGS  ${BISON_FLAGS} ${BISON_ARGS}
                       BYPRODUCTS ${BISON_OUTPUT} ${BISON_DEFINES} ${BISON_LOCATION}
                                  ${PARSER_OUTPUT}
                       VERBATIM
                       COMMENT "Generating QasmParser.tab.cpp")
  endif()
endif()

set(FLEX_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/QasmScanner.yy.cpp")
set(FLEX_HEADER "${CMAKE_CURRENT_BINARY_DIR}/QasmScanner.yy.h")
set(FLEX_OUTPUT_DONE "${CMAKE_CURRENT_BINARY_DIR}/.QasmScanner.yy.cpp.done")
set(FLEX_HEADER_DONE "${CMAKE_CURRENT_BINARY_DIR}/.QasmScanner.yy.h.done")

set(FLEX_FILE "${OPENQASM_LIB_DIR}/Parser/QasmScanner.l")
set(FLEX_ARGS --header-file=${FLEX_HEADER} --outfile=${FLEX_OUTPUT} ${FLEX_FILE})

if (FLEX_FOUND)
  if(UPPERCASE_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    add_custom_command(OUTPUT ${FLEX_OUTPUT} ${FLEX_HEADER}
                       COMMAND ${FLEX_EXECUTABLE}
                       ARGS ${FLEX_FLAGS} ${FLEX_ARGS}
                       BYPRODUCTS ${FLEX_OUTPUT} ${FLEX_HEADER}
                       COMMAND ${OPENQASM_DIR}/utils/sed-flex-output.sh
                       ARGS ${CMAKE_CURRENT_BINARY_DIR}
                       VERBATIM
                       COMMENT "Generating QasmScanner.yy.cpp.")
  else()
    add_custom_command(OUTPUT ${FLEX_OUTPUT} ${FLEX_HEADER}
                       COMMAND ${FLEX_EXECUTABLE}
                       ARGS ${FLEX_FLAGS} ${FLEX_ARGS}
                       BYPRODUCTS ${FLEX_OUTPUT} ${FLEX_HEADER}
                       VERBATIM
                       COMMENT "Generating QasmScanner.yy.cpp.")
  endif()
endif()

list(APPEND CMAKE_TARGETS ${BISON_TARGET})
list(APPEND CMAKE_TARGETS ${FLEX_TARGET})

include_directories(${OPENQASM_INCLUDE_DIR})
include_directories(${OPENQASM_BINARY_DIR})
include_directories(${OPENQASM_SOURCE_DIR})

set(OPENQASM_PARSER_SOURCES ${BISON_OUTPUT}
    ${BISON_DEFINES} ${BISON_LOCATION} ${PARSER_OUTPUT}
    ${FLEX_OUTPUT} ${FLEX_HEADER}
    ${BISON_OUTPUT_DONE}
    ${BISON_DEFINES_DONE}
    ${BISON_LOCATION_DONE}
    ${FLEX_OUTPUT_DONE}
    ${FLEX_HEADER_DONE}
    QasmScanner.cpp
)

set(SHARED_LIB_NAME "QasmParserShared")
set(STATIC_LIB_NAME "QasmParserStatic")
set(PHYSICAL_LIB_NAME "qasmParser")

set(BISON_LICENSE "${PROJECT_SOURCE_DIR}/licenses/bison-dual-license.txt")
set(FLEX_LICENSE  "${PROJECT_SOURCE_DIR}/licenses/flex-dual-license.txt")

if(OPENQASM_CMAKE_PACKAGE)
  set(INSTALL_EXPORT EXPORT OpenQASM)
endif()

message(STATUS "CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "OPENQASM_BINARY_DIR: ${OPENQASM_BINARY_DIR}")

if(BUILD_SHARED_LIBS)
  set(SHARED_LIB ${SHARED_LIB_NAME})
  add_library(${SHARED_LIB} SHARED ${OPENQASM_PARSER_SOURCES})
  set_target_properties(${SHARED_LIB} PROPERTIES
      OUTPUT_NAME ${PHYSICAL_LIB_NAME})
  set_target_properties(${SHARED_LIB} PROPERTIES
      VERSION ${OPENQASM_VERSION_TRIPLE})
  set_target_properties(${SHARED_LIB} PROPERTIES
      SOVERSION ${PROJECT_SOVERSION})
  set_target_properties(${SHARED_LIB} PROPERTIES
      POSITION_INDEPENDENT_CODE ON)

  add_custom_command(OUTPUT ${BISON_OUTPUT_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${BISON_OUTPUT} ${BISON_LICENSE} bison
                     DEPENDS ${BISON_OUTPUT}
                     VERBATIM
                     COMMENT "Adjusting license for ${BISON_OUTPUT}")
  add_custom_command(OUTPUT ${BISON_DEFINES_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${BISON_DEFINES} ${BISON_LICENSE} bison
                     DEPENDS ${BISON_DEFINES}
                     VERBATIM
                     COMMENT "Adjusting license for ${BISON_DEFINES}")
  add_custom_command(OUTPUT ${BISON_LOCATION_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${BISON_LOCATION} ${BISON_LICENSE} bison
                     DEPENDS ${BISON_LOCATION}
                     VERBATIM
                     COMMENT "Adjusting license for ${BISON_LOCATION}")
  add_custom_command(OUTPUT ${FLEX_OUTPUT_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${FLEX_OUTPUT} ${FLEX_LICENSE} flex
                     DEPENDS ${FLEX_OUTPUT}
                     VERBATIM
                     COMMENT "Adjusting license for ${FLEX_OUTPUT}")
  add_custom_command(OUTPUT ${FLEX_HEADER_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${FLEX_HEADER} ${FLEX_LICENSE} flex
                     DEPENDS ${FLEX_HEADER}
                     VERBATIM
                     COMMENT "Adjusting license for ${FLEX_HEADER}")

  target_include_directories(${SHARED_LIB} PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      $<BUILD_INTERFACE:${OPENQASM_INCLUDE_DIR}>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
      $<BUILD_INTERFACE:${OPENQASM_BINARY_DIR}/include>)
  target_link_libraries(QasmParserShared PUBLIC mpfr gmp)
  list(APPEND CMAKE_TARGETS ${SHARED_LIB})
  target_compile_features(${SHARED_LIB} PUBLIC ${REQUIRED_FEATURES})
endif()

if (BUILD_STATIC_LIBS)
  set(STATIC_LIB ${STATIC_LIB_NAME})
  add_library(${STATIC_LIB} STATIC ${OPENQASM_PARSER_SOURCES})
  set_target_properties(${STATIC_LIB} PROPERTIES
      OUTPUT_NAME ${PHYSICAL_LIB_NAME})
  set_target_properties(${STATIC_LIB} PROPERTIES
      POSITION_INDEPENDENT_CODE ON)

  add_custom_command(OUTPUT ${BISON_OUTPUT_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${BISON_OUTPUT} ${BISON_LICENSE} bison
                     DEPENDS ${BISON_OUTPUT}
                     VERBATIM
                     COMMENT "Adjusting license for ${BISON_OUTPUT}")
  add_custom_command(OUTPUT ${BISON_DEFINES_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${BISON_DEFINES} ${BISON_LICENSE} bison
                     DEPENDS ${BISON_DEFINES}
                     VERBATIM
                     COMMENT "Adjusting license for ${BISON_DEFINES}")
  add_custom_command(OUTPUT ${BISON_LOCATION_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${BISON_LOCATION} ${BISON_LICENSE} bison
                     DEPENDS ${BISON_LOCATION}
                     VERBATIM
                     COMMENT "Adjusting license for ${BISON_LOCATION}")
  add_custom_command(OUTPUT ${FLEX_OUTPUT_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${FLEX_OUTPUT} ${FLEX_LICENSE} flex
                     DEPENDS ${FLEX_OUTPUT}
                     VERBATIM
                     COMMENT "Adjusting license for ${FLEX_OUTPUT}")
  add_custom_command(OUTPUT ${FLEX_HEADER_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${FLEX_HEADER} ${FLEX_LICENSE} flex
                     DEPENDS ${FLEX_HEADER}
                     VERBATIM
                     COMMENT "Adjusting license for ${FLEX_HEADER}")

  target_include_directories(${STATIC_LIB} PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      $<BUILD_INTERFACE:${OPENQASM_INCLUDE_DIR}>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
      $<BUILD_INTERFACE:${OPENQASM_BINARY_DIR}/include>)
  list(APPEND CMAKE_TARGETS ${STATIC_LIB})
  target_compile_features(${STATIC_LIB} PUBLIC ${REQUIRED_FEATURES})
endif()

install(TARGETS ${CMAKE_TARGETS} ${INSTALL_EXPORT}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    OBJECTS DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

