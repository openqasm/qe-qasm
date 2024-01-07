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

set(BISON_TARGET BisonQasmParser)
set(BISON_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/QasmParser.tab.cpp")
set(BISON_DEFINES "${CMAKE_CURRENT_BINARY_DIR}/QasmParser.tab.h")
set(BISON_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/location.hh")
set(PARSER_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/QasmParser.output")
set(BISON_OUTPUT_DONE "${CMAKE_CURRENT_BINARY_DIR}/.QasmParser.tab.cpp.done")
set(BISON_DEFINES_DONE "${CMAKE_CURRENT_BINARY_DIR}/.QasmParser.tab.h.done")
set(BISON_LOCATION_DONE "${CMAKE_CURRENT_BINARY_DIR}/.location.hh.done")
set(BISON_LICENSE "${PROJECT_SOURCE_DIR}/licenses/bison-dual-license.txt")

set(BISON_FILE "${OPENQASM_LIB_DIR}/Parser/QasmParser.y")
set(BISON_ARGS --defines=${BISON_DEFINES} ${BISON_FILE} --output=${BISON_OUTPUT})

if (BISON_FOUND)
  if (UPPERCASE_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    add_custom_command(OUTPUT ${BISON_OUTPUT} ${BISON_DEFINES} ${BISON_LOCATION}
                              ${PARSER_OUTPUT}
                       COMMAND ${CMAKE_COMMAND} -E env "M4=${M4}" ${BISON_EXECUTABLE}
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
                       COMMAND ${CMAKE_COMMAND} -E env "M4=${M4}" ${BISON_EXECUTABLE}
                       ARGS  ${BISON_FLAGS} ${BISON_ARGS}
                       BYPRODUCTS ${BISON_OUTPUT} ${BISON_DEFINES} ${BISON_LOCATION}
                                  ${PARSER_OUTPUT}
                       VERBATIM
                       COMMENT "Generating QasmParser.tab.cpp")
  endif()
else()
  message(WARNING "Bison not found. Skipping generation of QasmParser.tab.cpp.")
endif()

add_custom_target(${BISON_TARGET} DEPENDS ${BISON_OUTPUT} ${BISON_DEFINES} ${BISON_LOCATION} ${PARSER_OUTPUT})

set(FLEX_TARGET FlexQasmScanner)
set(FLEX_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/QasmScanner.yy.cpp")
set(FLEX_HEADER "${CMAKE_CURRENT_BINARY_DIR}/QasmScanner.yy.h")
set(FLEX_OUTPUT_DONE "${CMAKE_CURRENT_BINARY_DIR}/.QasmScanner.yy.cpp.done")
set(FLEX_HEADER_DONE "${CMAKE_CURRENT_BINARY_DIR}/.QasmScanner.yy.h.done")
set(FLEX_LICENSE "${PROJECT_SOURCE_DIR}/licenses/flex-dual-license.txt")

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
else()
  message(WARNING "Flex not found. Skipping generation of QasmScanner.yy.cpp.")
endif()

add_custom_target(${FLEX_TARGET} DEPENDS ${FLEX_OUTPUT} ${FLEX_HEADER})

include_directories(${OPENQASM_INCLUDE_DIR})
include_directories(${OPENQASM_BINARY_DIR})
include_directories(${OPENQASM_SOURCE_DIR})

set(OPENQASM_PARSER_SOURCES ${BISON_OUTPUT}
    ${BISON_DEFINES} ${BISON_LOCATION} ${PARSER_OUTPUT}
    ${FLEX_OUTPUT} ${FLEX_HEADER}
    QasmScanner.cpp
)

if(BUILD_SHARED_LIBS OR BUILD_STATIC_LIBS)
  add_custom_command(OUTPUT ${BISON_OUTPUT_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${BISON_OUTPUT} ${BISON_LICENSE} bison
                     DEPENDS ${BISON_TARGET}
                     VERBATIM
                     COMMENT "Adjusting license for ${BISON_OUTPUT}")
  add_custom_command(OUTPUT ${BISON_DEFINES_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${BISON_DEFINES} ${BISON_LICENSE} bison
                     DEPENDS ${BISON_TARGET}
                     VERBATIM
                     COMMENT "Adjusting license for ${BISON_DEFINES}")
  add_custom_command(OUTPUT ${BISON_LOCATION_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${BISON_LOCATION} ${BISON_LICENSE} bison
                     DEPENDS ${BISON_TARGET}
                     VERBATIM
                     COMMENT "Adjusting license for ${BISON_LOCATION}")
  add_custom_command(OUTPUT ${FLEX_OUTPUT_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${FLEX_OUTPUT} ${FLEX_LICENSE} flex
                     DEPENDS ${FLEX_TARGET}
                     VERBATIM
                     COMMENT "Adjusting license for ${FLEX_OUTPUT}")
  add_custom_command(OUTPUT ${FLEX_HEADER_DONE}
                     COMMAND ${PROJECT_SOURCE_DIR}/utils/update-license.sh
                     ARGS ${FLEX_HEADER} ${FLEX_LICENSE} flex
                     DEPENDS ${FLEX_TARGET}
                     VERBATIM
                     COMMENT "Adjusting license for ${FLEX_HEADER}")

  add_custom_target(${OPENQASM_TARGET_LICENSING} DEPENDS
                    ${BISON_OUTPUT_DONE}
                    ${BISON_DEFINES_DONE}
                    ${BISON_LOCATION_DONE}
                    ${FLEX_OUTPUT_DONE}
                    ${FLEX_HEADER_DONE})
endif()

set(PHYSICAL_LIB_NAME "qasmParser")

if(OPENQASM_CMAKE_PACKAGE)
  set(INSTALL_EXPORT EXPORT OpenQASM)
endif()

message(STATUS "CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "OPENQASM_BINARY_DIR: ${OPENQASM_BINARY_DIR}")

if(BUILD_SHARED_LIBS)
  set(SHARED_LIB ${OPENQASM_TARGET_PARSER_SHARED})
  add_library(${SHARED_LIB} SHARED ${OPENQASM_PARSER_SOURCES})
  add_dependencies(${SHARED_LIB} ${OPENQASM_TARGET_LICENSING} ${BISON_TARGET} ${FLEX_TARGET})
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
      $<BUILD_INTERFACE:${OPENQASM_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${FLEX_INCLUDE_DIRS}>)
  target_link_libraries(${SHARED_LIB} PUBLIC ${OPENQASM_TARGET_DIAG_SHARED} mpc::mpc mpfr::mpfr gmp::gmp)
  list(APPEND CMAKE_TARGETS ${SHARED_LIB})
  target_compile_features(${SHARED_LIB} PUBLIC ${REQUIRED_FEATURES})
endif()

if (BUILD_STATIC_LIBS)
  set(STATIC_LIB ${OPENQASM_TARGET_PARSER_STATIC})
  add_library(${STATIC_LIB} STATIC ${OPENQASM_PARSER_SOURCES})
  add_dependencies(${STATIC_LIB} ${OPENQASM_TARGET_LICENSING} ${BISON_TARGET} ${FLEX_TARGET})
  set_target_properties(${STATIC_LIB} PROPERTIES
      OUTPUT_NAME ${PHYSICAL_LIB_NAME})
  set_target_properties(${STATIC_LIB} PROPERTIES
      POSITION_INDEPENDENT_CODE ON)

  target_include_directories(${STATIC_LIB} PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      $<BUILD_INTERFACE:${OPENQASM_INCLUDE_DIR}>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
      $<BUILD_INTERFACE:${OPENQASM_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${FLEX_INCLUDE_DIRS}>)
  target_link_libraries(${STATIC_LIB} PUBLIC ${OPENQASM_TARGET_DIAG_STATIC} mpc::mpc mpfr::mpfr gmp::gmp)
  list(APPEND CMAKE_TARGETS ${STATIC_LIB})
  target_compile_features(${STATIC_LIB} PUBLIC ${REQUIRED_FEATURES})
endif()

install(TARGETS ${CMAKE_TARGETS} ${INSTALL_EXPORT}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    OBJECTS DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
