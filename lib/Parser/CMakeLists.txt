set(BISON_FLAGS --verbose -t -d -l --language=c++ --feature=diagnostics-show-caret)
set(FLEX_FLAGS --yylineno --8bit -L --c++)

if(UPPERCASE_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
  set(BISON_FLAGS ${BISON_FLAGS} --debug)
  set(FLEX_FLAGS --debug ${FLEX_FLAGS})
endif()

message(STATUS "OPENQASM_BINARY_DIR: ${OPENQASM_BINARY_DIR}")
message(STATUS "OPENQASM_INCLUDE_DIR: ${OPENQASM_INCLUDE_DIR}")
message(STATUS "OPENQASM_LIB_DIR: ${OPENQASM_LIB_DIR}")

set(BISON_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/QasmParser.tab.cpp")
set(BISON_DEFINES "${CMAKE_CURRENT_BINARY_DIR}/QasmParser.tab.h")
set(BISON_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/location.hh")
set(BISON_FILE "${OPENQASM_LIB_DIR}/Parser/QasmParser.y")
set(BISON_ARGS --defines=${BISON_DEFINES} ${BISON_FILE} --output=${BISON_OUTPUT})
if (BISON_FOUND)
  if(UPPERCASE_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    add_custom_command(OUTPUT ${BISON_OUTPUT} ${BISON_DEFINES} ${BISON_LOCATION}
                      COMMAND ${BISON_EXECUTABLE}
                      ARGS  ${BISON_FLAGS} ${BISON_ARGS}
                      BYPRODUCTS ${BISON_OUTPUT} ${BISON_DEFINES} ${BISON_LOCATION}
                      COMMAND ${OPENQASM_DIR}/utils/sed-bison-output.sh
                      ARGS ${CMAKE_CURRENT_BINARY_DIR}
                      VERBATIM
                      COMMENT "Generating QasmParser.tab.cpp")
  else()
    add_custom_command(OUTPUT ${BISON_OUTPUT} ${BISON_DEFINES} ${BISON_LOCATION}
                      COMMAND ${BISON_EXECUTABLE}
                      ARGS  ${BISON_FLAGS} ${BISON_ARGS}
                      BYPRODUCTS ${BISON_OUTPUT} ${BISON_DEFINES} ${BISON_LOCATION}
                      VERBATIM
                      COMMENT "Generating QasmParser.tab.cpp")
  endif()
endif()

set(FLEX_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/QasmScanner.yy.cpp")
set(FLEX_HEADER "${CMAKE_CURRENT_BINARY_DIR}/QasmScanner.yy.h")
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
    ${FLEX_OUTPUT}
    QasmScanner.cpp
)

set(SHARED_LIB_NAME "QasmParserShared")
set(STATIC_LIB_NAME "QasmParserStatic")
set(PHYSICAL_LIB_NAME "qasmParser")

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

