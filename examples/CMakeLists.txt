set(OPENQASM_EXAMPLES QasmParser QDem)

if(OPENQASM_BUILD_EXAMPLES)
  foreach(program ${OPENQASM_EXAMPLES})
    add_executable(${program} ${program}/${program}.cpp)
    message(STATUS "Building ${program} ...")
    target_include_directories(${program} PRIVATE ${OPENQASM_INCLUDE_DIR})
    target_include_directories(${program} PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
    target_link_libraries(${program} PUBLIC ${OPENQASM_LIBRARIES} mpc::mpc mpfr::mpfr gmp::gmp)
  endforeach()

  add_custom_target(examples ALL DEPENDS
    ${OPENQASM_LIBRARIES}
    ${OPENQASM_EXAMPLES}
  )
endif()

if (OPENQASM_BUILD_EXAMPLES)
  install(TARGETS ${OPENQASM_EXAMPLES} ${INSTALL_EXPORT}
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
         )
endif()
