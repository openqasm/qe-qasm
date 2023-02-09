include_directories(${OPENQASM_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(OPENQASM_AST_SOURCES
  ASTAnnotation.cpp
  ASTAngleNodeBuilder.cpp
  ASTArray.cpp
  ASTArraySubscript.cpp
  ASTAnyTypeBuilder.cpp
  ASTArgumentNodeBuilder.cpp
  ASTBase.cpp
  ASTBinaryOp.cpp
  ASTBinaryOpNode.cpp
  ASTBox.cpp
  ASTBoxStatementBuilder.cpp
  ASTBraceMatcher.cpp
  ASTBuilder.cpp
  ASTBuiltinFunctionsBuilder.cpp
  ASTCallExpr.cpp
  ASTCastExpr.cpp
  ASTCBit.cpp
  ASTCBitNodeMap.cpp
  ASTCtrlAssocBuilder.cpp
  ASTDeclarationBuilder.cpp
  ASTDeclarationContext.cpp
  ASTDefcal.cpp
  ASTDelay.cpp
  ASTDuration.cpp
  ASTExpressionBuilder.cpp
  ASTExpressionNodeList.cpp
  ASTFunctionCallExpr.cpp
  ASTForRangeInitBuilder.cpp
  ASTFunctions.cpp
  ASTFunctionCallArgument.cpp
  ASTFunctionDefinitionBuilder.cpp
  ASTGateNodeBuilder.cpp
  ASTGateOpBuilder.cpp
  ASTGateOps.cpp
  ASTGates.cpp
  ASTGateQubitParamBuilder.cpp
  ASTGateQubitTracker.cpp
  ASTHeapSizeController.cpp
  ASTIdentifierBuilder.cpp
  ASTIdentifier.cpp
  ASTIdentifierIndexResolver.cpp
  ASTIdentifierTypeController.cpp
  ASTInitializerNode.cpp
  ASTIntegerConstantExpression.cpp
  ASTInverseAssocBuilder.cpp
  ASTIfConditionals.cpp
  ASTIfConditionalsGraphController.cpp
  ASTIfStatementBuilder.cpp
  ASTIfStatementTracker.cpp
  ASTImplicitConversionExpr.cpp
  ASTIntegerSequenceBuilder.cpp
  ASTKernel.cpp
  ASTLength.cpp
  ASTLiveRangeChecker.cpp
  ASTLoops.cpp
  ASTLoopStatementBuilder.cpp
  ASTMangler.cpp
  ASTMPComplex.cpp
  ASTMPComplexList.cpp
  ASTMPDecimal.cpp
  ASTMPInteger.cpp
  ASTNamedTypeDeclarationBuilder.cpp
  ASTObjectTracker.cpp
  ASTOpenQASMVersionTracker.cpp
  ASTOpenPulseCalibration.cpp
  ASTOpenPulseController.cpp
  ASTOpenPulseFrame.cpp
  ASTOpenPulsePlay.cpp
  ASTOpenPulseWaveform.cpp
  ASTOperatorPrecedenceController.cpp
  ASTScopeController.cpp
  ASTTypeCastController.cpp
  ASTParameterBuilder.cpp
  ASTPragma.cpp
  ASTProductionFactory.cpp
  ASTQubit.cpp
  ASTQubitConcatBuilder.cpp
  ASTQubitList.cpp
  ASTQubitNodeBuilder.cpp
  ASTResult.cpp
  ASTScannerLineContext.cpp
  ASTStatementBuilder.cpp
  ASTStretch.cpp
  ASTStringList.cpp
  ASTSwitchStatement.cpp
  ASTSwitchStatementBuilder.cpp
  ASTSymbolTable.cpp
  ASTTypes.cpp
  ASTTypeDiscovery.cpp
  ASTTypeEnums.cpp
  ASTTypeSpecifier.cpp
  ASTTypeSystemBuilder.cpp
  ASTUtils.cpp
  ASTExpressionEvaluator.cpp
  ASTExpressionValidator.cpp)

set(PHYSICAL_LIB_NAME "qasmAST")

if(OPENQASM_CMAKE_PACKAGE)
  set(INSTALL_EXPORT EXPORT OpenQASM)
endif()

if(BUILD_SHARED_LIBS)
  set(SHARED_LIB ${OPENQASM_TARGET_AST_SHARED})
  add_library(${SHARED_LIB} SHARED ${OPENQASM_AST_SOURCES})
  set_target_properties(${SHARED_LIB} PROPERTIES
      OUTPUT_NAME ${PHYSICAL_LIB_NAME})
  set_target_properties(${SHARED_LIB} PROPERTIES
      VERSION ${OPENQASM_VERSION_TRIPLE})
  set_target_properties(${SHARED_LIB} PROPERTIES
      SOVERSION ${PROJECT_SOVERSION})
  set_target_properties(${SHARED_LIB} PROPERTIES
      POSITION_INDEPENDENT_CODE ON)
  set_target_properties(QasmParserShared PROPERTIES
      IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/lib)
  add_dependencies(${SHARED_LIB} QasmParserShared)

  target_include_directories(${SHARED_LIB} PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
      $<BUILD_INTERFACE:${OPENQASM_BINARY_DIR}/lib/Parser>
      $<BUILD_INTERFACE:${OPENQASM_BINARY_DIR}/include>)
  target_link_libraries(${SHARED_LIB} PUBLIC ${OPENQASM_TARGET_PARSER_SHARED} mpc::mpc mpfr::mpfr gmp::gmp)
  list(APPEND CMAKE_TARGETS ${SHARED_LIB})
  target_compile_features(${SHARED_LIB} PUBLIC ${REQUIRED_FEATURES})
endif()

if (BUILD_STATIC_LIBS)
  set(STATIC_LIB ${OPENQASM_TARGET_AST_STATIC})
  add_library(${STATIC_LIB} STATIC ${OPENQASM_AST_SOURCES})
  set_target_properties(${STATIC_LIB} PROPERTIES
      OUTPUT_NAME ${PHYSICAL_LIB_NAME})
  set_target_properties(${STATIC_LIB} PROPERTIES
      POSITION_INDEPENDENT_CODE ON)
  set_target_properties(QasmParserStatic PROPERTIES
      IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/lib)
  add_dependencies(${STATIC_LIB} QasmParserStatic)

  target_include_directories(${STATIC_LIB} PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
      $<BUILD_INTERFACE:${OPENQASM_BINARY_DIR}/lib/Parser>
      $<BUILD_INTERFACE:${OPENQASM_BINARY_DIR}/include>)
  target_link_libraries(${STATIC_LIB} PUBLIC ${OPENQASM_TARGET_PARSER_STATIC} mpc::mpc mpfr::mpfr gmp::gmp)
  list(APPEND CMAKE_TARGETS ${STATIC_LIB})
  target_compile_features(${STATIC_LIB} PUBLIC ${REQUIRED_FEATURES})
endif()

install(TARGETS ${CMAKE_TARGETS} ${INSTALL_EXPORT}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    OBJECTS DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

