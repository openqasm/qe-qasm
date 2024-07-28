%skeleton "lalr1.cc"
%require  "3.6"

/* -*- coding: utf-8 -*-
 *
 * Copyright 2023 IBM RESEARCH. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * =============================================================================
 */

%defines

%define api.namespace {QASM}
%define api.parser.class {Parser}
%define api.location.file "location.hh"
%define api.location.include {<location.hh>}

%define parse.error verbose
%define parse.lac full
%define parse.assert

%locations

%language "c++"

%code requires {
  namespace QASM {
    class ASTDriver;
    class ASTScanner;
  }

#ifndef YY_NULLPTR
#if defined(__cplusplus) && (201103L <= __cplusplus)
#define YY_NULLPTR nullptr
#else
#define YY_NULLPTR 0
#endif
#endif

#define YYERROR_VERBOSE 1

#include <qasm/AST/AST.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTQualifiers.h>
#include <qasm/AST/ASTStringList.h>
#include <qasm/AST/ASTGates.h>
#include <qasm/AST/ASTGateControl.h>
#include <qasm/AST/ASTAngleContextControl.h>
#include <qasm/AST/ASTReturn.h>
#include <qasm/AST/ASTDeclarationList.h>
#include <qasm/AST/ASTParameterList.h>
#include <qasm/AST/ASTPrimitives.h>
#include <qasm/AST/ASTIfConditionals.h>
#include <qasm/AST/ASTIfConditionalsGraphController.h>
#include <qasm/AST/ASTGates.h>
#include <qasm/AST/ASTQubitList.h>
#include <qasm/AST/ASTProgramBlock.h>
#include <qasm/AST/ASTTypeSpecifier.h>
#include <qasm/AST/ASTResult.h>
#include <qasm/AST/ASTLoops.h>
#include <qasm/AST/ASTKernel.h>
#include <qasm/AST/ASTDelay.h>
#include <qasm/AST/ASTDuration.h>
#include <qasm/AST/ASTStretch.h>
#include <qasm/AST/ASTPopcount.h>
#include <qasm/AST/ASTRotate.h>
#include <qasm/AST/ASTBox.h>
#include <qasm/AST/ASTExpressionNodeBuilder.h>
#include <qasm/AST/ASTInitializerListBuilder.h>
#include <qasm/AST/ASTGPhase.h>
#include <qasm/AST/ASTCallExpr.h>
#include <qasm/AST/ASTCastExpr.h>
#include <qasm/AST/ASTPragma.h>
#include <qasm/AST/ASTAnnotation.h>
#include <qasm/AST/ASTFunctionCallArgument.h>
#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTArray.h>
#include <qasm/AST/ASTArraySubscript.h>
#include <qasm/AST/ASTDirectiveStatementNode.h>
#include <qasm/AST/ASTFlowControl.h>
#include <qasm/AST/ASTSwitchStatement.h>
#include <qasm/AST/ASTSwitchStatementBuilder.h>
#include <qasm/AST/ASTIntegerSequenceBuilder.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTProductionFactory.h>
#include <qasm/AST/ASTTypeDiscovery.h>
#include <qasm/AST/ASTUtils.h>
#include <qasm/AST/ASTStatementBuilder.h>
#include <qasm/AST/ASTDeclarationBuilder.h>
#include <qasm/AST/ASTNamedTypeDeclarationBuilder.h>
#include <qasm/AST/ASTExpressionBuilder.h>
#include <qasm/AST/ASTParameterBuilder.h>
#include <qasm/AST/ASTAnyTypeBuilder.h>
#include <qasm/AST/ASTIdentifierIndexResolver.h>
#include <qasm/AST/ASTIdentifierBuilder.h>
#include <qasm/AST/ASTIdentifierTypeController.h>
#include <qasm/AST/ASTGateQubitParamBuilder.h>
#include <qasm/AST/ASTGateOpBuilder.h>
#include <qasm/AST/ASTAngleNodeBuilder.h>
#include <qasm/AST/ASTGateNodeBuilder.h>
#include <qasm/AST/ASTGateQubitTracker.h>
#include <qasm/AST/ASTArgumentNodeBuilder.h>
#include <qasm/AST/ASTQubitNodeBuilder.h>
#include <qasm/AST/ASTGateOpBuilder.h>
#include <qasm/AST/ASTIfStatementBuilder.h>
#include <qasm/AST/ASTIfStatementTracker.h>
#include <qasm/AST/ASTCBitNodeMap.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTLiveRangeChecker.h>
#include <qasm/AST/ASTTypeSystemBuilder.h>
#include <qasm/AST/ASTFunctionDefinitionBuilder.h>
#include <qasm/AST/ASTFunctionStatementBuilder.h>
#include <qasm/AST/ASTKernelStatementBuilder.h>
#include <qasm/AST/ASTBoxStatementBuilder.h>
#include <qasm/AST/ASTForStatementBuilder.h>
#include <qasm/AST/ASTWhileStatementBuilder.h>
#include <qasm/AST/ASTDoWhileStatementBuilder.h>
#include <qasm/AST/ASTIntegerListBuilder.h>
#include <qasm/AST/ASTDefcalBuilder.h>
#include <qasm/AST/ASTDefcalStatementBuilder.h>
#include <qasm/AST/ASTDefcalGrammarBuilder.h>
#include <qasm/AST/ASTDefcalParameterBuilder.h>
#include <qasm/AST/ASTCalContextBuilder.h>
#include <qasm/AST/ASTFunctionContextBuilder.h>
#include <qasm/AST/ASTKernelContextBuilder.h>
#include <qasm/AST/ASTPragmaContextBuilder.h>
#include <qasm/AST/ASTAnnotationContextBuilder.h>
#include <qasm/AST/ASTForRangeInitBuilder.h>
#include <qasm/AST/ASTCtrlAssocBuilder.h>
#include <qasm/AST/ASTInverseAssocBuilder.h>
#include <qasm/AST/ASTQubitConcatBuilder.h>
#include <qasm/AST/ASTBinaryOpAssignBuilder.h>
#include <qasm/AST/ASTObjectTracker.h>
#include <qasm/AST/ASTScopeController.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTOpenQASMVersionTracker.h>

// OpenPulse
#include <qasm/AST/OpenPulse/ASTOpenPulsePlay.h>
#include <qasm/AST/OpenPulse/ASTOpenPulsePort.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseFrame.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseWaveform.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseController.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseCalibration.h>

#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

}

%parse-param { QASM::ASTScanner& QASMScanner  }
%parse-param { QASM::ASTDriver&  QASMDriver  }
%lex-param { QASM::ASTScanner& QASMScanner  }
%locations

%code {

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <climits>
#include <cfloat>
#include <cassert>
#include <cstring>
#include <typeinfo>

#include <qasm/AST/AST.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTQualifiers.h>
#include <qasm/AST/ASTDeclaration.h>
#include <qasm/AST/ASTParameterList.h>
#include <qasm/AST/ASTPrimitives.h>
#include <qasm/AST/ASTProgramBlock.h>
#include <qasm/AST/ASTTypeSpecifier.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTProductionFactory.h>
#include <qasm/AST/ASTStatementBuilder.h>
#include <qasm/AST/ASTDeclarationBuilder.h>
#include <qasm/AST/ASTExpressionBuilder.h>
#include <qasm/AST/ASTParameterBuilder.h>
#include <qasm/AST/ASTAnyTypeBuilder.h>
#include <qasm/AST/ASTGateNodeBuilder.h>
#include <qasm/AST/ASTSymbolTable.h>

#include <qasm/Frontend/QasmDriver.h>
#include <qasm/Frontend/QasmScanner.h>
#include <qasm/Frontend/QasmFeatureTester.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/QPP/QasmPP.h>

#ifdef yylex
#undef yylex
#endif

#define yylex QASMScanner.yylex

#ifdef GET_TOKEN
#undef GET_TOKEN
#endif

#define GET_TOKEN(N) (yystack_[N].value.Tok)

QASM::ASTRoot* Root = nullptr;
QASM::ASTProgramBlock* ProgramBlock = nullptr;
QASM::ASTOpenQASMStatementNode* OQS = nullptr;
bool OpenQASMStated = false;
std::string FilePath;
std::ifstream InFile;
std::istream* InStream = nullptr;

extern int yylineno;

bool openstream(const char* Path) {
  if (!Path || !*Path)
    return false;

  FilePath = Path;

  if (FilePath != "-") {
    InFile.open(FilePath, std::fstream::in);
    if (!InFile.good() || InFile.eof())
      return false;
  }

  return true;
}

void yyerror(const char* msg) {
  if (msg) {
    std::string M(msg);

    QASM::ASTSyntaxErrorNode* SYX = new QASM::ASTSyntaxErrorNode(M);
    assert(SYX && "Could not create a valid ASTSyntaxErrorNode!");

    QASM::ASTStatementNode* STX =
      QASM::ASTStatementNode::StatementError(SYX->GetIdentifier(), SYX);
    assert(STX && "Could not create a valid ASTStatementNode!");

    QASM::ASTStatementBuilder::Instance().Append(STX);
    QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      QASM::DIAGLineCounter::Instance().GetLocation(), M, DiagLevel::Error);
  }
}

int yyparse() {
  QASM::ASTScanner S(InStream ? InStream : &InFile);
  QASM::ASTDriver D;
  QASM::Parser P(S, D);
  P.set_debug_stream(std::cerr);
  P.set_debug_level(3);

  QASM::ASTTypeSystemBuilder::Instance().Init();

  std::string OQ;
  std::string OV;
  std::stringstream SSF;
  std::string Line;

  bool SOQ = false;
  bool PS = false;
  bool OQR = false;

  if (QASM::QasmPreprocessor::Instance().IsTU()) {
    std::ifstream* IFS = dynamic_cast<std::ifstream*>(S.GetStream());

    if (!IFS) {
      std::stringstream M;
      M << "Could not obtain a valid file input stream.";
      QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                         DiagLevel::Error);
      return 1;
    }

    SSF << IFS->rdbuf();
    IFS->rdbuf()->pubseekoff(0, IFS->end, IFS->in);

    while (std::getline(SSF, Line)) {
      PS = QASM::ASTOpenQASMVersionTracker::Instance().ParseOpenQASMLine(Line,
                                                                OQ, OV, SOQ, OQR);

      if (PS && !SOQ)
        continue;
      else if ((PS && SOQ) || (!PS && !SOQ))
        break;
    }

    if (!SOQ && OQR) {
      std::stringstream M;
      M << "Error parsing the OPENQASM line directive.";
      QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                         DiagLevel::Error);
      return 1;
    }

    if (OQ != "OPENQASM" || OV.empty())
      QASM::ASTOpenQASMVersionTracker::Instance().SetVersion(3.0);

    if (!QASM::QasmPreprocessor::Instance().Preprocess(*IFS)) {
      std::stringstream M;
      M << "OpenQASM Preprocessor failure!";
      QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                         DiagLevel::Error);
      return 1;
    }

    QASM::QasmPreprocessor::Instance().SetFilePath(FilePath);
    return D.Parse(*IFS);
  } else {
    if (!QASM::QasmPreprocessor::Instance().IsIStream()) {
      std::stringstream M;
      M << "Translation Unit is neither file-based nor "
        << "stream-based!";
      QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                         DiagLevel::Error);
      return 1;
    }

    std::istream* IIS = dynamic_cast<std::istream*>(S.GetStream());
    assert(IIS && "Could not dynamic_cast to a valid Input Stream!");

    if (std::istringstream* ISS = dynamic_cast<std::istringstream*>(IIS)) {
      while (std::getline(*ISS, Line)) {
        PS = QASM::ASTOpenQASMVersionTracker::Instance().ParseOpenQASMLine(Line,
                                                         OQ, OV, SOQ, OQR);

        if (PS && !SOQ)
          continue;
        else if ((PS && SOQ) || (!PS && !SOQ))
          break;
      }

      if (!SOQ && OQR) {
        std::stringstream M;
        M << "Error parsing the OPENQASM line directive.";
        QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                           DiagLevel::Error);
        return 1;
      }
    } else {
      IIS->rdbuf()->pubseekoff(0, IIS->end, IIS->in);
      SSF << IIS->rdbuf();

      while (std::getline(SSF, Line)) {
        PS = QASM::ASTOpenQASMVersionTracker::Instance().ParseOpenQASMLine(Line,
                                                         OQ, OV, SOQ, OQR);

        if (PS && !SOQ)
          continue;
        else if ((PS && SOQ) || (!PS && !SOQ))
          break;
      }

      if (!SOQ && OQR) {
        std::stringstream M;
        M << "Error parsing the OPENQASM line directive.";
        QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                           DiagLevel::Error);
        return 1;
      }
    }

    if (OQ != "OPENQASM" || OV.empty())
      QASM::ASTOpenQASMVersionTracker::Instance().SetVersion(3.0);

    if (IIS) {
      IIS->clear();
      IIS->seekg(0, std::ios::beg);
    }

    if (!QASM::QasmPreprocessor::Instance().Preprocess(IIS)) {
      std::stringstream M;
      M << "OpenQASM Preprocessor failure!";
      QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                         DiagLevel::Error);
      return 1;
    }

    std::istream* PIS = QASM::QasmPreprocessor::Instance().GetIStream();

    if (!PIS) {
      std::stringstream M;
      M << "Invalid Input Stream obtained from the OpenQASM Preprocessor!";
      QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                         DiagLevel::Error);
      return 1;
    }

    if (IIS != PIS)
      IIS = PIS;

    QASM::QasmPreprocessor::Instance().SetFilePath(FilePath);
    return D.Parse(*IIS);
  }

  return 1;
}

int readinput() {
  int R = InFile.get();
  return (InFile.bad() || InFile.eof() || InFile.fail() || R == -1) ? 0 : R;
}

} // End top parser part.

%union {
  struct {
    QASM::ASTType Type;
    int64_t IntegerVal;
    double FPVal;
    bool Semicolon;
    bool Unsigned;
    int Token;

    const std::string* String;
    QASM::ASTToken* Tok;
    QASM::ASTStatementNode* StatementNode;
    QASM::ASTDeclarationNode* DeclarationNode;
    QASM::ASTDeclarationNode* NamedTypeDeclarationNode;
    QASM::ASTFunctionDeclarationNode* FuncDeclarationNode;
    QASM::ASTKernelDeclarationNode* KernelDeclarationNode;
    QASM::ASTDefcalDeclarationNode* DefcalDeclarationNode;
    QASM::ASTGateDeclarationNode* GateDeclarationNode;
    QASM::ASTIfStatementNode* IfStatementNode;
    QASM::ASTElseIfStatementNode* ElseIfStatementNode;
    QASM::ASTElseStatementNode* ElseStatementNode;
    QASM::ASTForStatementNode* ForStatementNode;
    QASM::ASTWhileStatementNode* WhileStatementNode;
    QASM::ASTDoWhileStatementNode* DoWhileStatementNode;
    QASM::ASTReturnStatementNode* ReturnStatementNode;
    QASM::ASTSwitchStatementNode* SwitchStatementNode;
    QASM::ASTCaseStatementNode* CaseStatementNode;
    QASM::ASTBreakStatementNode* BreakStatementNode;
    QASM::ASTContinueStatementNode* ContinueStatementNode;
    QASM::ASTDefaultStatementNode* DefaultStatementNode;
    QASM::ASTDelayStatementNode* DelayStatementNode;
    QASM::ASTStretchStatementNode* StretchStatementNode;
    QASM::ASTBinaryOpStatementNode* BinaryOpStatementNode;
    QASM::ASTDirectiveStatementNode* DirectiveStatementNode;
    QASM::ASTBoxStatementNode* BoxStatementNode;
    QASM::ASTFunctionCallStatementNode* FunctionCallStmtNode;
    QASM::ASTRotateStatementNode* RotateStmtNode;
    QASM::ASTPopcountStatementNode* PopcountStmtNode;
    QASM::ASTExpressionNode* ExpressionNode;
    QASM::ASTComplexExpressionNode* ComplexExprNode;
    QASM::ASTForLoopRangeExpressionNode* LoopRangeExpressionNode;
    QASM::ASTAssignmentNode* AssignmentNode;
    QASM::ASTArgumentNode* ArgumentNode;
    QASM::ASTAngleNode* AngleNode;
    QASM::ASTArraySubscriptNode* ArraySubscriptNode;
    QASM::ASTGateQOpNode* GPhaseOpNode;
    QASM::ASTGateQOpNode* GateQOpNode;
    QASM::ASTHGateOpNode* GateHOpNode;
    QASM::ASTGateQOpNode* GateEOpNode;
    QASM::ASTGateQOpNode* GateUOpNode;
    QASM::ASTCNotGateOpNode* GateCNotOpNode;
    QASM::ASTCXGateOpNode* GateCXOpNode;
    QASM::ASTCCXGateOpNode* GateCCXOpNode;
    QASM::ASTBarrierNode* BarrierNode;
    QASM::ASTResetNode* ResetNode;
    QASM::ASTKernelNode* KernelNode;
    QASM::ASTDelayNode* DelayNode;
    QASM::ASTStretchNode* StretchNode;
    QASM::ASTQubitNode* QubitNode;
    QASM::ASTQubitContainerNode* QubitContainerNode;
    QASM::ASTCBitNode* CBitNode;
    QASM::ASTArrayNode* ArrayNode;
    QASM::ASTMeasureNode* MeasureNode;
    QASM::ASTResultNode* ResultNode;
    QASM::ASTRotateNode* RotateNode;
    QASM::ASTPopcountNode* PopcountNode;
    QASM::ASTDefcalNode* DefcalNode;
    QASM::ASTDurationNode* DurationNode;
    QASM::ASTDurationOfNode* DurationOfNode;
    QASM::ASTCallExpressionNode* CallExprNode;
    QASM::ASTCastExpressionNode* CastExprNode;
    QASM::ASTFunctionCallArgumentNode* FunctionCallArgNode;
    QASM::ASTFunctionCallArgumentList* FunctionCallArgList;
    QASM::ASTFunctionCallNode* FunctionCallNode;

    QASM::ASTPragmaNode* PragmaExprNode;
    QASM::ASTPragmaStatementNode* PragmaStmtNode;

    QASM::ASTAnnotationNode* AnnotationExprNode;
    QASM::ASTAnnotationStatementNode* AnnotationStmtNode;

    QASM::ASTGateControlNode* GateControlNode;
    QASM::ASTGateNegControlNode* GateNegControlNode;
    QASM::ASTGateInverseNode* GateInverseNode;
    QASM::ASTGatePowerNode* GatePowerNode;
    QASM::ASTGPhaseExpressionNode* GPhaseNode;
    QASM::ASTGateGPhaseExpressionNode* GateGPhaseNode;

    QASM::ASTGateQOpNode* GateControlStmtNode;
    QASM::ASTGateQOpNode* GateNegControlStmtNode;
    QASM::ASTGateQOpNode* GateInverseStmtNode;
    QASM::ASTGateQOpNode* GatePowerStmtNode;

    QASM::ASTStatementList* StatementList;
    QASM::ASTStatementList* BoxStatementList;
    QASM::ASTStatementList* ForStatementList;
    QASM::ASTStatementList* WhileStatementList;
    QASM::ASTStatementList* DoWhileStatementList;
    QASM::ASTStatementList* FuncStatementList;

    QASM::ASTStatementNode* SwitchScopedStatement;
    QASM::ASTStatementNode* SwitchUnscopedStatement;
    QASM::ASTStatementList* SwitchStatementList;
    QASM::ASTStatementList* SwitchScopedStatementList;
    QASM::ASTStatementList* SwitchUnscopedStatementList;

    QASM::ASTExpressionNode* InitExpressionNode;
    QASM::ASTExpressionNodeList* InitExpressionNodeList;
    QASM::ASTInitializerList* InitializerList;

    QASM::ASTStringList* StringList;
    QASM::ASTStatementList* KernelStatementList;
    QASM::ASTStatementList* DefcalStatementList;
    QASM::ASTStatementList* IfStatementList;
    QASM::ASTStatementList* ElseIfStatementList;
    QASM::ASTStatementList* ElseStatementList;
    QASM::ASTIntegerList*   IntegerList;
    QASM::ASTDeclarationList* DeclarationList;
    QASM::ASTDeclarationList* NamedTypeDeclarationList;
    QASM::ASTDeclarationList* TypeDeclarationList;
    QASM::ASTDeclarationList* ParamTypeDeclarationList;
    QASM::ASTGateQOpList* GateOpList;
    QASM::ASTExpressionList* ExpressionList;
    QASM::ASTAnyTypeList* AnyList;
    QASM::ASTIdentifierList* IdentifierList;
    QASM::ASTIdentifierRefList* IdentifierRefList;
    QASM::ASTIdentifierList* GateQubitList;
    QASM::ASTIdentifierList* FunctionCallQArgsList;
    QASM::ASTParameterList* ParameterList;
    QASM::ASTArgumentNodeList* ArgumentList;
    QASM::ASTAngleNodeList* AngleList;
    QASM::ASTExpressionNodeList* ExpressionNodeList;
    QASM::ASTQubitNodeMap* QubitNodeMap;
    QASM::ASTIntegerSequence* IntegerSequence;
    QASM::ASTCtrlAssocList* CtrlAssocList;
    QASM::ASTInverseAssocList* InverseAssocList;
    QASM::ASTQubitConcatList* QubitConcatList;
    QASM::ASTBoundQubitList*  BoundQubitList;
    QASM::ASTArraySubscriptList* IndexedSubscriptList;

    QASM::ASTIdentifierNode* IdentifierNode;
    QASM::ASTIdentifierNode* IdentifierBinaryOpNode;
    QASM::ASTIdentifierRefNode* IdentifierRefNode;
    QASM::ASTIntNode* IntegerNode;
    QASM::ASTBoolNode* BoolNode;
    QASM::ASTFloatNode* FloatNode;
    QASM::ASTDoubleNode* RealNode;
    QASM::ASTEllipsisNode* EllipsisNode;
    QASM::ASTMPIntegerNode* MPIntegerNode;
    QASM::ASTMPDecimalNode* MPDecimalNode;
    QASM::ASTMPComplexNode* MPComplexNode;
    QASM::ASTCBitNode* BitsetNode;
    QASM::ASTStringNode* StringNode;
    QASM::ASTStringNode* TimeUnitNode;
    QASM::ASTPowNode* PowNode;
    QASM::ASTDefcalGrammarNode* DefcalGrammarNode;

    QASM::ASTUnaryOpNode* UnaryOpNode;
    QASM::ASTBinaryOpNode* BinaryOpNode;

    // OpenPulse.
    QASM::OpenPulse::ASTOpenPulseFrameNode* OPFrameNode;
    QASM::OpenPulse::ASTOpenPulsePortNode* OPPortNode;
    QASM::OpenPulse::ASTOpenPulseWaveformNode* OPWaveformNode;
    QASM::OpenPulse::ASTOpenPulsePlayNode* OPPlayNode;
    QASM::OpenPulse::ASTOpenPulseCalibration* OPCalNode;
  };
}

%token <String> TOK_IDENTIFIER TOK_INTEGER_CONSTANT TOK_FP_CONSTANT
%token <String> TOK_STRING_LITERAL TOK_FUNC_NAME TOK_BOOLEAN_CONSTANT
%token <String> TOK_TYPEDEF_NAME
%token <String> TOK_CREG TOK_QREG TOK_CNOT TOK_HADAMARD
%token <String> TOK_CCX TOK_CX TOK_QUBIT TOK_BOUND_QUBIT TOK_UNBOUND_QUBIT
%token <String> TOK_QUBITS TOK_U TOK_ANGLE TOK_FIXED
%token <String> TOK_DIRTY TOK_OPAQUE TOK_RESET
%token <String> TOK_IBMQASM

%token <Token> TOK_BOOL TOK_INT TOK_UINT TOK_CHAR TOK_SHORT TOK_LONG
%token <Token> TOK_SIGNED TOK_UNSIGNED TOK_FLOAT TOK_DOUBLE
%token <Token> TOK_LONG_DOUBLE TOK_VOID TOK_ARRAY
%token <Token> TOK_SIZEOF TOK_PRAGMA TOK_KERNEL TOK_ANNOTATION

%token <Token> TOK_COS TOK_EXP TOK_LN TOK_POW
%token <Token> TOK_SIN TOK_SQRT TOK_TAN
%token <Token> TOK_ARCCOS TOK_ARCSIN TOK_ARCTAN
%token <Token> TOK_ROTL TOK_ROTR TOK_POPCOUNT

%token <Token> TOK_ALPHA TOK_BETA TOK_GAMMA TOK_DELTA
%token <Token> TOK_EPSILON TOK_ZETA TOK_ETA TOK_THETA
%token <Token> TOK_IOTA TOK_KAPPA TOK_LAMBDA TOK_MU
%token <Token> TOK_NU TOK_XI TOK_OMICRON TOK_PI
%token <Token> TOK_RHO TOK_SIGMA TOK_TAU TOK_UPSILON
%token <Token> TOK_PHI TOK_CHI TOK_PSI TOK_OMEGA
%token <Token> TOK_EULER TOK_EULER_NUMBER

%token <Token> TOK_GATE TOK_BARRIER TOK_MEASURE TOK_EXTERN
%token <Token> TOK_CAL TOK_DEFCAL TOK_DEFCAL_GRAMMAR

%token <Token> TOK_PTR_OP TOK_INC_OP TOK_DEC_OP
%token <Token> TOK_LEFT_SHIFT_OP TOK_RIGHT_SHIFT_OP
%token <Token> TOK_ADD_OP TOK_SUB_OP TOK_MUL_OP TOK_DIV_OP TOK_POW_OP
%token <Token> TOK_MOD_OP
%token <Token> TOK_LE_OP TOK_GE_OP TOK_EQ_OP TOK_NE_OP TOK_AND_OP
%token <Token> TOK_OR_OP TOK_MUL_ASSIGN TOK_DIV_ASSIGN TOK_MOD_ASSIGN
%token <Token> TOK_ADD_ASSIGN TOK_SUB_ASSIGN
%token <Token> TOK_AND_ASSIGN TOK_XOR_ASSIGN TOK_OR_ASSIGN
%token <Token> TOK_LEFT_SHIFT_ASSIGN TOK_RIGHT_SHIFT_ASSIGN

%token <Token> ';' '=' '+' '*' '-' '/' '%' '^' '|' '&' '[' ']'

%token <Token> TOK_RIGHT_ARROW TOK_LEFT_ARROW
%token <Token> TOK_CONST
%token <Token> TOK_STRETCH TOK_STRETCH_N TOK_STRETCHINF
%token <Token> TOK_LENGTH TOK_LENGTHOF TOK_DURATION TOK_DURATIONOF
%token <Token> TOK_GPHASE
%token <Token> TOK_LET TOK_INVERSE TOK_CTRL TOK_NEGCTRL TOK_ASSOCIATION_OP
%token <Token> TOK_DELAY TOK_IN TOK_BIT TOK_FUNCTION_DEFINITION
%token <Token> TOK_BOX TOK_BOXAS TOK_BOXTO TOK_VERBATIM TOK_IMPLEMENTS
%token <Token> TOK_INPUT TOK_OUTPUT

%token <Token> TOK_COMPLEX TOK_IMAGINARY TOK_CREAL TOK_CIMAG
%token <Token> TOK_STRUCT TOK_UNION TOK_ENUM
%token <Token> TOK_ELLIPSIS TOK_CASE TOK_IF TOK_ELSEIF TOK_ELSE TOK_SWITCH
%token <Token> TOK_WHILE TOK_DO TOK_FOR TOK_GOTO TOK_CONTINUE TOK_BREAK
%token <Token> TOK_RETURN TOK_DEFAULT
%token <Token> TOK_TIME_UNIT

%token <Token> TOK_LEFT_CURLY TOK_RIGHT_CURLY TOK_COMMA TOK_SEMICOLON
%token <Token> TOK_EQUAL_ASSIGN TOK_LEFT_PAREN TOK_RIGHT_PAREN
%token <Token> TOK_LEFT_BRACKET TOK_RIGHT_BRACKET TOK_PERIOD
%token <Token> TOK_AMPERSAND TOK_BANG TOK_TILDE TOK_MINUS TOK_PLUS
%token <Token> TOK_MUL TOK_DIV TOK_MOD TOK_LT_OP TOK_GT_OP TOK_XOR_OP
%token <Token> TOK_QUESTION TOK_INCLUDE

// OpenPulse specific tokens.
%token <Token> TOK_PORT TOK_FRAME TOK_NEWFRAME
%token <Token> TOK_FREQUENCY TOK_PHASE TOK_TIME
%token <Token> TOK_PLAY TOK_WAVEFORM

// Combine multiple grammars into the same Grammar Definition File.
%token TOK_START_OPENQASM TOK_START_OPENPULSE

%token <Token> TOK_NEWLINE TOK_HORIZONTAL_TAB TOK_VERTICAL_TAB
%token <Token> TOK_FILE TOK_LINE TOK_ERROR
%token <Token> TOK_END 0 "EOF"

%type <StatementList>               OpenQASMProgram OpenPulseProgram
%type <StatementNode>               Statement OpenPulseStatement
                                    OpenPulseStmt
%type <SwitchStatementList>         SwitchStmtList SwitchStmtListImpl
%type <SwitchScopedStatementList>   SwitchScopedStmtList SwitchScopedStmtListImpl
%type <SwitchUnscopedStatementList> SwitchUnscopedStmtList SwitchUnscopedStmtListImpl
%type <FuncDeclarationNode>         FuncDecl
%type <KernelDeclarationNode>       KernelDecl
%type <DefcalDeclarationNode>       DefcalDecl
%type <GateDeclarationNode>         GateDecl OpaqueDecl
%type <DeclarationNode>             Decl ConstDecl ParamTypeDecl
                                    ModifierDecl OpenPulseDecl
%type <NamedTypeDeclarationNode>    NamedTypeDecl
%type <IfStatementNode>             IfStmt
%type <ElseIfStatementNode>         ElseIfStmt
%type <ElseStatementNode>           ElseStmt
%type <ForStatementNode>            ForStmt
%type <WhileStatementNode>          WhileStmt
%type <DoWhileStatementNode>        DoWhileStmt
%type <ReturnStatementNode>         ReturnStmt
%type <BreakStatementNode>          BreakStmt
%type <ContinueStatementNode>       ContinueStmt
%type <SwitchScopedStatement>       SwitchScopedStatement
%type <SwitchUnscopedStatement>     SwitchUnscopedStatement
%type <SwitchStatementNode>         SwitchStmt
%type <CaseStatementNode>           SwitchCaseStmt
%type <DefaultStatementNode>        SwitchDefaultStmt
%type <DelayStatementNode>          DelayStmt
%type <StretchStatementNode>        StretchStmt
%type <BinaryOpStatementNode>       BinaryOpStmt
%type <DirectiveStatementNode>      LineDirective FileDirective
%type <MPIntegerNode>               MPIntegerType
%type <MPDecimalNode>               MPDecimalType
%type <MPComplexNode>               MPComplexDecl MPComplexType
                                    MPComplexFunctionCallDecl
%type <BitsetNode>                  BitType
%type <AngleNode>                   AngleType
%type <QubitContainerNode>          QubitType
%type <DurationNode>                DurationType
%type <GPhaseNode>                  GPhaseExpr
%type <GateGPhaseNode>              GateGPhaseExpr
%type <ArrayNode>                   ArrayExpr ArrayType InitArrayExpr
%type <ComplexExprNode>             ComplexInitializerExpr

%type <ExpressionNode>              Expr LogicalNotExpr
%type <FunctionCallNode>            FunctionCallExpr ParenFunctionCallExpr
%type <FunctionCallArgNode>         FunctionCallArg
%type <FunctionCallArgList>         FunctionCallArgExprList
                                    FunctionCallArgExprListImpl
%type <CastExprNode>                CastExpr
%type <GateControlNode>             GateCtrlExpr
%type <GateNegControlNode>          GateNegCtrlExpr
%type <GateInverseNode>             GateInvExpr
%type <GatePowerNode>               GatePowExpr
%type <BoxStatementNode>            BoxStmt
%type <GPhaseOpNode>                GPhaseStmt
%type <GateControlStmtNode>         GateCtrlExprStmt
%type <GateNegControlStmtNode>      GateNegCtrlExprStmt
%type <GateInverseStmtNode>         GateInvExprStmt
%type <GatePowerStmtNode>           GatePowExprStmt
%type <LoopRangeExpressionNode>     ForLoopRangeExpr

%type <GateQOpNode>                 GateQOp GateCtrlStmt GateNegCtrlStmt
                                    GateInvStmt GatePowStmt GateGPhaseStmt
%type <GateUOpNode>                 GateUOp
%type <GateEOpNode>                 GateEOp
%type <BarrierNode>                 Barrier
%type <ResetNode>                   Reset
%type <MeasureNode>                 Measure MeasureDecl
%type <DelayNode>                   Delay
%type <StretchNode>                 Stretch
%type <DurationNode>                DurationDecl ImplicitDuration
%type <DurationOfNode>              DurationOfDecl

%type <InitExpressionNode>          InitExpressionNode

%type <PragmaExprNode>              PragmaExpr
%type <PragmaStmtNode>              PragmaStmt

%type <AnnotationExprNode>          AnnotationExpr
%type <AnnotationStmtNode>          AnnotationStmt

%type <StatementList>               StmtList StmtListImpl
                                    OpenPulseStmtList OpenPulseStmtListImpl
%type <BoxStatementList>            BoxStmtList BoxStmtListImpl
%type <ForStatementList>            ForStmtList ForStmtListImpl
%type <WhileStatementList>          WhileStmtList WhileStmtListImpl
%type <DoWhileStatementList>        DoWhileStmtList DoWhileStmtListImpl
%type <FuncStatementList>           FuncStmtList FuncStmtListImpl
%type <InitExpressionNodeList>      InitExpressionNodeList InitExpressionNodeListImpl
%type <InitializerList>             InitializerList InitializerListImpl
%type <IfStatementList>             IfStmtList IfStmtListImpl
%type <ElseIfStatementList>         ElseIfStmtList ElseIfStmtListImpl
%type <ElseStatementList>           ElseStmtList ElseStmtListImpl
%type <KernelStatementList>         KernelStmtList KernelStmtListImpl
%type <DefcalStatementList>         DefcalStmtList DefcalStmtListImpl
%type <GateOpList>                  GateOpList
%type <ExpressionList>              ExprList ExprListImpl
%type <IntegerList>                 IntegerList IntegerListImpl
%type <QubitConcatList>             QubitConcatList QubitConcatListImpl
%type <AnyList>                     AnyList AnyListImpl
%type <IdentifierList>              IdentifierList IdentifierListImpl
%type <StringList>                  StringList StringListImpl
%type <GateQubitList>               GateQubitParamList GateQubitParamListImpl
%type <ArgumentList>                ArgsList
%type <NamedTypeDeclarationList>    NamedTypeDeclList NamedTypeDeclListImpl

%type <BoundQubitList>              QubitList QubitListImpl
%type <ArraySubscriptNode>          IndexedSubscriptExpr
%type <IndexedSubscriptList>        IndexedSubscriptList IndexedSubscriptListImpl

%type <IdentifierNode>              Identifier
%type <StringNode>                  String BoundQubit UnboundQubit
                                    IndexedBoundQubit IndexedUnboundQubit
                                    ComplexCReal ComplexCImag
                                    OpenPulseFramePhase OpenPulseFrameFrequency
                                    OpenPulseFrameTime
%type <TimeUnitNode>                TimeUnit
%type <BoolNode>                    BooleanConstant
%type <IntegerNode>                 Integer IntScalarType
%type <FloatNode>                   FloatScalarType
%type <RealNode>                    Real
%type <EllipsisNode>                Ellipsis
%type <ResultNode>                  FuncResult
%type <RotateNode>                  RotateOpExpr
%type <RotateStmtNode>              RotateOpStmt
%type <PopcountNode>                PopcountOpExpr
%type <PopcountStmtNode>            PopcountOpStmt
%type <FunctionCallStmtNode>        FunctionCallStmtExpr FunctionCallStmt
%type <DefcalGrammarNode>           DefcalGrammarDecl

%type <UnaryOpNode>                 UnaryOp
%type <BinaryOpNode>                BinaryOpSelfAssign BinaryOpAssign
                                    BinaryOpPrePost BinaryOp BinaryOpExpr
                                    ArithPowExpr

%type <OPFrameNode>                 OpenPulseFrame
%type <OPPortNode>                  OpenPulsePort
%type <OPWaveformNode>              OpenPulseWaveform
%type <OPPlayNode>                  OpenPulsePlay
%type <OPCalNode>                   OpenPulseCalibration

%right '='
%right TOK_OR_ASSIGN TOK_AND_ASSIGN TOK_XOR_ASSIGN
%right TOK_LEFT_SHIFT_ASSIGN TOK_RIGHT_SHIFT_ASSIGN
%right TOK_ADD_ASSIGN TOK_SUB_ASSIGN
%right TOK_MUL_ASSIGN TOK_DIV_ASSIGN TOK_MOD_ASSIGN

%left TOK_OR_OP
%left TOK_AND_OP
%left '|'
%left '^'
%left '&'
%left TOK_EQ_OP TOK_NE_OP
%left TOK_LEFT_SHIFT_OP TOK_RIGHT_SHIFT_OP
%left '<' '>' TOK_LE_OP TOK_GE_OP
%left TOK_ADD_OP TOK_SUB_OP '-'
%left TOK_MUL_OP TOK_DIV_OP TOK_MOD_OP
%right TOK_BANG TOK_TILDE
%left TOK_INC_OP TOK_DEC_OP TOK_RIGHT_ARROW TOK_LEFT_ARROW
%left '@' TOK_ASSOCIATION_OP
%left ';' TOK_SEMICOLON

%nonassoc TOK_NEG

%nonassoc XIF
%nonassoc XELSEIF
%nonassoc TOK_IF
%nonassoc TOK_ELSEIF
%nonassoc TOK_ELSE

%locations

%initial-action {
  /* @$.begin.filename = &OpenQASM::AST.TranslationUnit; */
  /* @$.end.filename = &OpenQASM:AST.TranslationUnit; */
}

%%

%start Start;

Start
  : TOK_START_OPENQASM OpenQASMProgram
  | TOK_START_OPENPULSE OpenPulseProgram
  ;

OpenPulseProgram
  : OpenPulseStmtList {
    $$ = $1;
  }
  | TOK_IBMQASM TOK_FP_CONSTANT ';' StmtListImpl {
    assert($1 && "Invalid OpenQASM Token argument!");
    assert($2 && "Invalid OpenQASM Version argument!");

    $$ = ASTStatementBuilder::Instance().List();

    if (!OQS || !OpenQASMStated) {
      OQS = ASTBuilder::Instance().CreateASTOpenQASMStatementNode(*$1, *$2);
      assert(OQS && "Could not create a valid ASTOpenQASMStatementNode!");
      ASTStatementBuilder::Instance().Prepend(OQS);
      ASTOpenQASMVersionTracker::Instance().SetVersion(std::stod(*$2));
      OpenQASMStated = true;
    }
  }
  | TOK_IBMQASM TOK_INTEGER_CONSTANT ';' StmtList {
    $$ = ASTStatementBuilder::Instance().List();

    if (!OQS || !OpenQASMStated) {
      OQS = ASTBuilder::Instance().CreateASTOpenQASMStatementNode(*$1, *$2);
      assert(OQS && "Could not create a valid ASTOpenQASMStatementNode!");
      ASTStatementBuilder::Instance().Prepend(OQS);
      ASTOpenQASMVersionTracker::Instance().SetVersion(std::stod(*$2));
      OpenQASMStated = true;
    }
  }
  | TOK_START_OPENQASM OpenQASMProgram {
  }
  ;

OpenQASMProgram
  : StmtList {
    $$ = $1;
  }
  | TOK_IBMQASM TOK_FP_CONSTANT ';' StmtListImpl {
    assert($1 && "Invalid OpenQASM Token argument!");
    assert($2 && "Invalid OpenQASM Version argument!");

    $$ = ASTStatementBuilder::Instance().List();

    if (!OQS || !OpenQASMStated) {
      OQS = ASTBuilder::Instance().CreateASTOpenQASMStatementNode(*$1, *$2);
      assert(OQS && "Could not create a valid ASTOpenQASMStatementNode!");
      ASTStatementBuilder::Instance().Prepend(OQS);
      ASTOpenQASMVersionTracker::Instance().SetVersion(std::stod(*$2));
      OpenQASMStated = true;
    }
  }
  | TOK_IBMQASM TOK_INTEGER_CONSTANT ';' StmtList {
    $$ = ASTStatementBuilder::Instance().List();

    if (!OQS || !OpenQASMStated) {
      OQS = ASTBuilder::Instance().CreateASTOpenQASMStatementNode(*$1, *$2);
      assert(OQS && "Could not create a valid ASTOpenQASMStatementNode!");
      ASTStatementBuilder::Instance().Prepend(OQS);
      ASTOpenQASMVersionTracker::Instance().SetVersion(std::stod(*$2));
      OpenQASMStated = true;
    }
  }
  | TOK_START_OPENPULSE OpenPulseProgram {
  }
  ;

StmtList
  : StmtListImpl TOK_END {
    $$ = ASTStatementBuilder::Instance().List();
  }
  ;

StmtListImpl
  : %empty {
    ASTStatementBuilder::Instance().Clear();
    $$ = ASTStatementBuilder::Instance().List();
  }
  | StmtListImpl Statement {
    if ($2 && !$2->IsDirective())
      ASTStatementBuilder::Instance().Append($2);
    $$ = ASTStatementBuilder::Instance().List();
  }
  | StmtListImpl OpenPulseStatement {
    if ($2 && !$2->IsDirective())
      ASTStatementBuilder::Instance().Append($2);
    $$ = ASTStatementBuilder::Instance().List();
  }
  ;

OpenPulseStmtList
  : OpenPulseStmtListImpl /* TOK_END */ {
    assert($$ && "OpenPulse StatementList is Invalid!");
    assert(!$$->Empty() && "OpenPulse StatementList is Empty!");
    $$ = $1;
  }
  ;

OpenPulseStmtListImpl
  : %empty {
    $$ = new ASTStatementList();
    assert($$ && "Could not create a valid OpenPulse StatementList!");
  }
  | OpenPulseStmtListImpl OpenPulseStatement {
    assert($$ && "OpenPulse StatementListImpl is Invalid!");
    assert($1 && "Invalid OpenPulseStmtListImpl argument!");
    assert($2 && "Invalid OpenPulseStatement argument!");

    if ($2 && !$2->IsDirective()) {
      $1->Append($2);
    }
  }
  | OpenPulseStmtListImpl Statement {
    assert($$ && "OpenPulse StatementListImpl is Invalid!");
    assert($1 && "Invalid OpenPulseStmtListImpl argument!");
    assert($2 && "Invalid Statement argument!");

    if ($2 && !$2->IsDirective()) {
      $1->Append($2);
    }
  }
  ;

ForStmtList
  : ForStmtListImpl {
    $$ = $1;
  }
  ;

ForStmtListImpl
  : %empty {
    $$ = ASTForStatementBuilder::Instance().NewList();
  }
  | ForStmtListImpl Statement {
    assert($2 && "Invalid ASTStatementNode argument!");
    if (!$2->IsDirective()) {
      $1->Append($2);
    }
  }
  ;

BoxStmtList
  : BoxStmtListImpl {
    $$ = ASTBoxStatementBuilder::Instance().List();
  }
  ;

BoxStmtListImpl
  : %empty {
    ASTBoxStatementBuilder::Instance().Clear();
    $$ = ASTBoxStatementBuilder::Instance().List();
  }
  | BoxStmtListImpl Statement {
    assert($1 && "Invalid BoxStmtListImpl argument!");
    assert($2 && "Invalid ASTStatementNode argument!");
    if (!$2->IsDirective())
      $1->Append($2);
  }
  ;

FuncStmtList
  : FuncStmtListImpl {
    $$ = ASTFunctionStatementBuilder::Instance().List();
  }
  ;

FuncStmtListImpl
  : %empty {
    ASTFunctionStatementBuilder::Instance().Clear();
    $$ = ASTFunctionStatementBuilder::Instance().List();
  }
  | FuncStmtListImpl Statement {
    assert($2 && "Invalid ASTStatementNode argument!");
    if (!$2->IsDirective())
      ASTFunctionStatementBuilder::Instance().Append($2);
    $$ = ASTFunctionStatementBuilder::Instance().List();
  }
  | FuncStmtListImpl Statement ',' {
    assert($2 && "Invalid ASTStatementNode argument!");
    if (!$2->IsDirective())
      ASTFunctionStatementBuilder::Instance().Append($2);
    $$ = ASTFunctionStatementBuilder::Instance().List();
  }
  ;

InitExpressionNodeList
  : InitExpressionNodeListImpl {
    $$ = $1;
  }
  ;

InitExpressionNodeListImpl
  : %empty {
    // Do not use NewList() here. New list allocations
    // are handled either in ASTTypeSystemBuilder or
    // in ASTExpressionNodeBuilder::Restart below.
    $$ = ASTExpressionNodeBuilder::Instance().List();
  }
  | InitExpressionNode {
    $$ = ASTExpressionNodeBuilder::Instance().List();
    $$->Append($1);
  }
  | InitExpressionNodeListImpl ',' InitExpressionNode {
    $1->Append($3);
  }
  ;

InitializerList
  : InitializerListImpl {
    $$ = $1;
  }
  ;

InitializerListImpl
  : %empty {
    $$ = ASTInitializerListBuilder::Instance().NewList();
  }
  | '{' InitExpressionNodeList '}' {
    $$ = ASTInitializerListBuilder::Instance().List();
    $$->Append($2);
    ASTExpressionNodeBuilder::Instance().Restart();
  }
  | InitializerListImpl ',' '{' InitExpressionNodeList '}' {
    $$ = ASTInitializerListBuilder::Instance().List();
    $$->Append($4);
    ASTExpressionNodeBuilder::Instance().Restart();
  }
  ;

QubitList
  : QubitListImpl {
    $$ = $1;
  }
  ;

QubitListImpl
  : %empty {
    $$ = new ASTBoundQubitList();
    assert($$ && "Could not create a valid ASTBoundQubitList!");
  }
  | QubitListImpl BoundQubit {
    assert($1 && "Invalid QubitListImpl argument!");
    assert($2 && "Invalid BoundQubit argument!");
    assert($$ == $1 && "Nested (recursive) QubitListImpl!");
    $$->Append($2);
  }
  | QubitListImpl IndexedBoundQubit {
    assert($1 && "Invalid QubitListImpl argument!");
    assert($2 && "Invalid BoundQubit argument!");
    assert($$ == $1 && "Nested (recursive) QubitListImpl!");
    $$->Append($2);
  }
  | QubitListImpl BoundQubit ',' {
    assert($1 && "Invalid QubitListImpl argument!");
    assert($2 && "Invalid BoundQubit argument!");
    assert($$ == $1 && "Nested (recursive) QubitListImpl!");
    $$->Append($2);
  }
  | QubitListImpl IndexedBoundQubit ',' {
    assert($1 && "Invalid QubitListImpl argument!");
    assert($2 && "Invalid BoundQubit argument!");
    assert($$ == $1 && "Nested (recursive) QubitListImpl!");
    $$->Append($2);
  }
  | QubitListImpl UnboundQubit {
    assert($1 && "Invalid QubitListImpl argument!");
    assert($2 && "Invalid UnboundQubit argument!");
    assert($$ == $1 && "Nested (recursive) QubitListImpl!");
    $$->Append($2);
  }
  | QubitListImpl IndexedUnboundQubit {
    assert($1 && "Invalid QubitListImpl argument!");
    assert($2 && "Invalid UnboundQubit argument!");
    assert($$ == $1 && "Nested (recursive) QubitListImpl!");
    $$->Append($2);
  }
  | QubitListImpl UnboundQubit ',' {
    assert($1 && "Invalid QubitListImpl argument!");
    assert($2 && "Invalid UnboundQubit argument!");
    assert($$ == $1 && "Nested (recursive) QubitListImpl!");
    $$->Append($2);
  }
  | QubitListImpl IndexedUnboundQubit ',' {
    assert($1 && "Invalid QubitListImpl argument!");
    assert($2 && "Invalid UnboundQubit argument!");
    assert($$ == $1 && "Nested (recursive) QubitListImpl!");
    $$->Append($2);
  }
  ;

IndexedSubscriptExpr
  : '[' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1520(GET_TOKEN(1), $2);
  }
  | '[' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1520(GET_TOKEN(1), $2);
  }
  | '[' BinaryOp ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1520(GET_TOKEN(1), $2);
  }
  | '[' UnaryOp ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1520(GET_TOKEN(1), $2);
  }
  ;

IndexedSubscriptList
  : IndexedSubscriptListImpl {
    $$ = $1;
  }
  ;

IndexedSubscriptListImpl
  : %empty {
    $$ = new ASTArraySubscriptList();
    assert($$ && "Could not create a valid ASTArraySubscriptList!");
  }
  | IndexedSubscriptListImpl IndexedSubscriptExpr {
    assert($1 && "Invalid IndexedSubscriptListImpl argument!");
    assert($2 && "Invalid ASTIntNode argument!");
    $1->Append($2);
  }
  ;

KernelStmtList
  : KernelStmtListImpl {
    $$ = ASTKernelStatementBuilder::Instance().List();
  }
  ;

KernelStmtListImpl
  : %empty {
    ASTKernelStatementBuilder::Instance().Clear();
    $$ = ASTKernelStatementBuilder::Instance().List();
  }
  | KernelStmtListImpl Statement {
    assert($2 && "Invalid ASTStatementNode argument!");
    if (!$2->IsDirective())
      ASTKernelStatementBuilder::Instance().Append($2);
    $$ = ASTKernelStatementBuilder::Instance().List();
  }
  | KernelStmtListImpl Statement ',' {
    assert($2 && "Invalid ASTStatementNode argument!");
    if (!$2->IsDirective())
      ASTKernelStatementBuilder::Instance().Append($2);
    $$ = ASTKernelStatementBuilder::Instance().List();
  }
  ;

DefcalStmtList
  : DefcalStmtListImpl {
    $$ = ASTDefcalStatementBuilder::Instance().List();
  }
  ;

DefcalStmtListImpl
  : %empty {
    ASTDefcalStatementBuilder::Instance().Clear();
    $$ = ASTDefcalStatementBuilder::Instance().List();
  }
  | DefcalStmtListImpl Statement {
    assert($2 && "Invalid ASTStatementNode argument!");
    if (!$2->IsDirective())
      ASTDefcalStatementBuilder::Instance().Append($2);
    $$ = ASTDefcalStatementBuilder::Instance().List();
  }
  | DefcalStmtListImpl Statement ',' {
    assert($2 && "Invalid ASTStatementNode argument!");
    if (!$2->IsDirective())
      ASTDefcalStatementBuilder::Instance().Append($2);
    $$ = ASTDefcalStatementBuilder::Instance().List();
  }
  | DefcalStmtListImpl OpenPulseStatement {
    assert($2 && "Invalid OpenPulse StatementNode argument!");
    if (!$2->IsDirective())
      ASTDefcalStatementBuilder::Instance().Append($2);
    $$ = ASTDefcalStatementBuilder::Instance().List();
  }
  | DefcalStmtListImpl OpenPulseStatement ',' {
    assert($2 && "Invalid OpenPulse StatementNode argument!");
    if (!$2->IsDirective())
      ASTDefcalStatementBuilder::Instance().Append($2);
    $$ = ASTDefcalStatementBuilder::Instance().List();
  }
  ;

IfStmtList
  : IfStmtListImpl {
    $$ = $1;
  }
  ;

IfStmtListImpl
  : %empty {
    $$ = ASTIfStatementBuilder::Instance().NewList();
    assert($$ && "Could not create an IfStmtListImpl StatementList!");
  }
  | IfStmtListImpl Statement {
    assert($1 && "Invalid IfStmtListImpl argument!");
    assert($2 && "Invalid ASTStatementNode argument!");

    if (!$2->IsDirective())
      $1->Append($2);
  }
  ;

ElseIfStmtList
  : ElseIfStmtListImpl {
    $$ = $1;
  }
  ;

ElseIfStmtListImpl
  : %empty {
    $$ = ASTElseIfStatementBuilder::Instance().NewList();
    assert($$ && "Could not create an ElseIfStmtListImpl StatementList!");
  }
  | ElseIfStmtListImpl Statement {
    assert($1 && "Invalid ElseIfStmtListImpl argument!");
    assert($2 && "Invalid ASTStatementNode argument!");

    if (!$2->IsDirective())
      $1->Append($2);
  }
  ;

ElseStmtList
  : ElseStmtListImpl {
    $$ = $1;
  }
  ;

ElseStmtListImpl
  : %empty {
    $$ = ASTElseStatementBuilder::Instance().NewList();
    assert($$ && "Could not create an ElseStmtListImpl StatementList!");
  }
  | ElseStmtListImpl Statement {
    assert($1 && "Invalid ElseStmtListImpl argument!");
    assert($2 && "Invalid ASTStatementNode argument!");

    if (!$2->IsDirective())
      $1->Append($2);
  }
  ;

WhileStmtList
  : WhileStmtListImpl {
    $$ = $1;
  }
  ;

WhileStmtListImpl
  : %empty {
    $$ = ASTWhileStatementBuilder::Instance().NewList();
  }
  | WhileStmtListImpl Statement {
    assert($2 && "Invalid ASTStatementNode argument!");
    if (!$2->IsDirective()) {
      $1->Append($2);
    }
  }
  ;

DoWhileStmtList
  : DoWhileStmtListImpl {
    $$ = $1;
  }
  ;

DoWhileStmtListImpl
  : %empty {
    $$ = ASTDoWhileStatementBuilder::Instance().NewList();
  }
  | DoWhileStmtListImpl Statement {
    assert($2 && "Invalid ASTStatementNode argument!");
    if (!$2->IsDirective()) {
      $1->Append($2);
    }
  }
  ;

SwitchStmtList
  : SwitchStmtListImpl {
    $$ = $1;
  }
  ;

SwitchStmtListImpl
  : %empty {
    $$ = ASTSwitchStatementBuilder::Instance().NewList();
    assert($$ && "Could not create a valid SwitchStatementListImpl!");
  }
  | SwitchStmtListImpl SwitchCaseStmt {
    assert($1 && "Invalid SwitchStmtListImpl argument!");
    assert($2 && "Invalid ASTSwitchCaseStatement argument!");
    if (!$2->IsDirective())
      $1->Append($2);
  }
  | SwitchStmtListImpl SwitchDefaultStmt {
    assert($1 && "Invalid SwitchStmtListImpl argument!");
    assert($2 && "Invalid ASTSwitchDefaultStatement argument!");
    if (!$2->IsDirective())
      $1->Append($2);
  }
  ;

Statement
  : Decl {
    $$ = $1;
  }
  | ConstDecl {
    $$ = $1;
  }
  | FuncDecl   {
    $$ = $1;
  }
  | GateDecl   {
    $$ = $1;
  }
  | OpaqueDecl {
    $$ = $1;
  }
  | GateQOp   {
    $$ = $1;
  }
  | Barrier    {
    $$ = $1;
  }
  | IfStmt     {
    $$ = $1;
  }
  | ElseIfStmt {
    $$ = $1;
  }
  | ElseStmt   {
    $$ = $1;
  }
  | ForStmt    {
    $$ = $1;
  }
  | WhileStmt  {
    $$ = $1;
  }
  | DoWhileStmt  {
    $$ = $1;
  }
  | ReturnStmt {
    $$ = $1;
  }
  | SwitchStmt {
    $$ = $1;
  }
  | BreakStmt {
    $$ = $1;
  }
  | ContinueStmt {
    $$ = $1;
  }
  | DelayStmt {
    $$ = $1;
  }
  | StretchStmt {
    $$ = $1;
  }
  | BinaryOpStmt {
    $$ = $1;
  }
  | BoxStmt {
    $$ = $1;
  }
  | FunctionCallStmt {
    $$ = $1;
  }
  | IncludeStmt {
    // Ignored. Handled by the Preprocessor.
  }
  | RotateOpStmt {
    $$ = $1;
  }
  | PopcountOpStmt {
    $$ = $1;
  }
  | GateCtrlExprStmt {
    $$ = $1;
  }
  | GateNegCtrlExprStmt {
    $$ = $1;
  }
  | GateInvExprStmt {
    $$ = $1;
  }
  | GatePowExprStmt {
    $$ = $1;
  }
  | PragmaStmt {
    $$ = $1;
  }
  | AnnotationStmt {
    $$ = $1;
  }
  | LineDirective {
    $$ = $1;
  }
  | FileDirective {
    $$ = $1;
  }
  | Newline {
  }
  ;

OpenPulseStatement
  : OpenPulseDecl {
    assert($1 && "Invalid OpenPulseDecl argument!");
    $$ = $1;
  }
  | OpenPulseStmt  {
    assert($1 && "Invalid OpenPulseStmt argument!");
    $$ = $1;
  }
  ;

Decl
  : TOK_QREG Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1000(GET_TOKEN(2), $2);
  }
  | TOK_CREG Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1001(GET_TOKEN(2), $2);
  }
  | TOK_CREG Identifier '=' Integer ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1002(GET_TOKEN(4),
                                                              $2, $4);
  }
  | TOK_CREG '[' Integer ']' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1003(GET_TOKEN(5),
                                                              $5, $3);
  }
  | TOK_CREG '[' Integer ']' Identifier '=' Integer ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1004(GET_TOKEN(7),
                                                              $5, $3, $7);
  }
  | TOK_CREG '[' Integer ']' Identifier '=' TOK_STRING_LITERAL ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1005(GET_TOKEN(7),
                                                              $5, $3, *$7);
  }
  | TOK_BOOL Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_100(GET_TOKEN(2), $2);
  }
  | TOK_BOOL Identifier '=' TOK_BOOLEAN_CONSTANT ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_101(GET_TOKEN(4),
                                                             $2, *$4);
  }
  | TOK_BOOL Identifier '=' TOK_INTEGER_CONSTANT ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_102(GET_TOKEN(4),
                                                             $2, *$4);
  }
  | TOK_BOOL Identifier '=' BinaryOpExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_103(GET_TOKEN(4),
                                                             $2, $4);
  }
  | TOK_BOOL Identifier '=' UnaryOp ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_104(GET_TOKEN(4),
                                                             $2, $4);
  }
  | TOK_BOOL Identifier '=' LogicalNotExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_104(GET_TOKEN(4), $2,
                                          dynamic_cast<ASTUnaryOpNode*>($4));
  }
  | TOK_BOOL Identifier '=' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_105(GET_TOKEN(4),
                                                             $2, $4);
  }
  | TOK_BOOL Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_106(GET_TOKEN(3),
                                                             $2, $4);
  }
  | TOK_INT Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_120(GET_TOKEN(2), $2);
  }
  | TOK_INT Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_121(GET_TOKEN(4),
                                                             $2, $4);
  }
  | TOK_INT Identifier '=' Statement {
    $$ = ASTProductionFactory::Instance().ProductionRule_122(GET_TOKEN(3),
                                                             $2, $4);
  }
  | TOK_INT '[' Integer ']' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_220(GET_TOKEN(5),
                                                             $5, $3);
  }
  | TOK_INT '[' Identifier ']' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_221(GET_TOKEN(5),
                                                             $5, $3);
  }
  | TOK_INT '[' Integer ']' Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_222(GET_TOKEN(7),
                                                             $5, $3, $7);
  }
  | TOK_INT '[' Integer ']' Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_223(GET_TOKEN(6),
                                                             $5, $3, $7);
  }
  | TOK_INT '[' Identifier ']' Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_224(GET_TOKEN(7),
                                                             $5, $3, $7);
  }
  | TOK_INT '[' Identifier ']' Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_225(GET_TOKEN(6),
                                                             $5, $3, $7);
  }
  | TOK_UINT Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_120(GET_TOKEN(2),
                                                             $2, true);
  }
  | TOK_UINT Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_226(GET_TOKEN(4),
                                                             $2, $4);
  }
  | TOK_UINT Identifier '=' Statement {
    $$ = ASTProductionFactory::Instance().ProductionRule_227(GET_TOKEN(3),
                                                             $2, $4);
  }
  | TOK_UINT '[' Integer ']' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_220(GET_TOKEN(5),
                                                             $5, $3, true);
  }
  | TOK_UINT '[' Identifier ']' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_221(GET_TOKEN(5),
                                                             $5, $3, true);
  }
  | TOK_UINT '[' Integer ']' Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_222(GET_TOKEN(5),
                                                             $5, $3, $7, true);
  }
  | TOK_UINT '[' Integer ']' Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_223(GET_TOKEN(6),
                                                             $5, $3, $7, true);
  }
  | TOK_UINT '[' Identifier ']' Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_224(GET_TOKEN(7),
                                                             $5, $3, $7, true);
  }
  | TOK_UINT '[' Identifier ']' Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_225(GET_TOKEN(6),
                                                             $5, $3, $7, true);
  }
  | TOK_DOUBLE Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_150(GET_TOKEN(2), $2);
  }
  | TOK_DOUBLE Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_151(GET_TOKEN(4),
                                                             $2, $4);
  }
  | TOK_FLOAT Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_140(GET_TOKEN(2), $2);
  }
  | TOK_FLOAT Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_141(GET_TOKEN(4),
                                                             $2, $4);
  }
  | TOK_FLOAT Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_142(GET_TOKEN(3),
                                                             $2, $4);
  }
  | TOK_FLOAT '[' Integer ']' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_143(GET_TOKEN(5),
                                                             $5, $3);
  }
  | TOK_FLOAT '[' Identifier ']' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_144(GET_TOKEN(5),
                                                             $5, $3);
  }
  | TOK_FLOAT '[' Integer ']' Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_145(GET_TOKEN(7),
                                                             $5, $3, $7);
  }
  | TOK_FLOAT '[' Integer ']' Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_146(GET_TOKEN(6),
                                                             $5, $3, $7);
  }
  | TOK_FLOAT '[' Identifier ']' Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_147(GET_TOKEN(7),
                                                             $5, $3, $7);
  }
  | TOK_FLOAT '[' Identifier ']' Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_148(GET_TOKEN(6),
                                                             $5, $3, $7);
  }
  | TOK_CONST TOK_IDENTIFIER '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_170(GET_TOKEN(3),
                                                             *$2, $4);
  }
  | TOK_CONST TOK_IDENTIFIER '[' Integer ']' '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_171(GET_TOKEN(6),
                                                             *$2, $4, $7);
  }
  | TOK_CONST TOK_IDENTIFIER '[' Identifier ']' '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_172(GET_TOKEN(6),
                                                             *$2, $4, $7);
  }
  | TOK_ANGLE '[' Integer ']' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_240(GET_TOKEN(5),
                                                             $5, $3);
  }
  | TOK_ANGLE '[' Identifier ']' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_241(GET_TOKEN(5),
                                                             $5, $3);
  }
  | TOK_ANGLE '[' Integer ']' Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_242(GET_TOKEN(7),
                                                             $5, $3, $7);
  }
  | TOK_ANGLE '[' Integer ']' Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_243(GET_TOKEN(6),
                                                             $5, $3, $7);
  }
  | TOK_ANGLE Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_180(GET_TOKEN(2), $2);
  }
  | TOK_ANGLE Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_181(GET_TOKEN(4),
                                                             $2, $4);
  }
  | TOK_ANGLE Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_182(GET_TOKEN(3),
                                                             $2, $4);
  }
  | TOK_ANGLE '[' Identifier ']' Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_244(GET_TOKEN(7),
                                                             $5, $3, $7);
  }
  | TOK_ANGLE '[' Identifier ']' Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_245(GET_TOKEN(6),
                                                             $5, $3, $7);
  }
  | DurationOfDecl ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1207(GET_TOKEN(1), $1);
  }
  | DurationDecl ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1206(GET_TOKEN(1), $1);
  }
  | TOK_BIT Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_107(GET_TOKEN(2), $2);
  }
  | TOK_BIT Identifier '=' TOK_INTEGER_CONSTANT ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_108(GET_TOKEN(4),
                                                             $2, *$4);
  }
  | TOK_BIT Identifier '=' TOK_STRING_LITERAL ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_109(GET_TOKEN(4),
                                                             $2, *$4);
  }
  | TOK_BIT Identifier '=' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_110(GET_TOKEN(4),
                                                             $2, $4);
  }
  | TOK_BIT Identifier '=' GateQOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_111(GET_TOKEN(3),
                                                             $2, $4);
  }
  | TOK_BIT Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_112(GET_TOKEN(3),
                                                             $2, $4);
  }
  | TOK_BIT '[' Integer ']' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_200(GET_TOKEN(5),
                                                             $5, $3);
  }
  | TOK_BIT '[' Identifier ']' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_201(GET_TOKEN(5),
                                                             $5, $3);
  }
  | TOK_BIT '[' Integer ']' Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_202(GET_TOKEN(7),
                                                             $5, $3, $7);
  }
  | TOK_BIT '[' Integer ']' Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_203(GET_TOKEN(6),
                                                             $5, $3, $7);
  }
  | TOK_BIT '[' Identifier ']' Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_204(GET_TOKEN(7),
                                                             $5, $3, $7);
  }
  | TOK_BIT '[' Identifier ']' Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_205(GET_TOKEN(6),
                                                             $5, $3, $7);
  }
  | TOK_QUBIT Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1100(GET_TOKEN(2), $2);
  }
  | TOK_QUBIT '[' Integer ']' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1101(GET_TOKEN(5),
                                                              $5, $3);
  }
  | TOK_QUBIT '[' Identifier ']' Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1102(GET_TOKEN(5),
                                                              $5, $3);
  }
  | TOK_QUBIT Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1103(GET_TOKEN(4),
                                                              $2, $4);
  }
  | TOK_QUBIT Identifier '=' FunctionCallStmt {
    // Functions cannot return Qubits.
    $$ = ASTProductionFactory::Instance().ProductionRule_1104(GET_TOKEN(3),
                                                              $2, $4);
  }
  | TOK_QUBIT '[' Integer ']' Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1105(GET_TOKEN(7),
                                                              $5, $3, $7);
  }
  | TOK_QUBIT '[' Integer ']' Identifier '=' FunctionCallStmt {
    // Functions cannot return Qubits.
    $$ = ASTProductionFactory::Instance().ProductionRule_1104(GET_TOKEN(6),
                                                              $5, $7);
  }
  | TOK_QUBIT '[' Identifier ']' Identifier '=' Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1106(GET_TOKEN(7),
                                                              $5, $3, $7);
  }
  | TOK_QUBIT '[' Identifier ']' Identifier '=' FunctionCallStmt {
    // Functions cannot return Qubits.
    $$ = ASTProductionFactory::Instance().ProductionRule_1104(GET_TOKEN(6),
                                                              $5, $7);
  }
  | TOK_LET Identifier '=' TOK_IDENTIFIER '[' TOK_INTEGER_CONSTANT ','
                                              IntegerList ']' ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1150($2, $4, $6, $8);
  }
  | TOK_LET Identifier '=' TOK_IDENTIFIER '[' TOK_INTEGER_CONSTANT ':'
                                              IntegerList ']' ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1151($2, $4, $6, $8);
  }
  | TOK_LET Identifier '=' TOK_IDENTIFIER IndexedSubscriptExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1152($2, $4, $5);
  }
  | TOK_LET Identifier '=' Identifier TOK_OR_OP QubitConcatList ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1153($2, $4, $6);
  }
  | TOK_LET Identifier '=' Identifier TOK_INC_OP QubitConcatList ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1153($2, $4, $6);
  }
  | FuncResult {
    $$ = ASTProductionFactory::Instance().ProductionRule_2150(GET_TOKEN(0), $1);
  }
  | KernelDecl {
    $$ = $1;
  }
  | DefcalDecl {
    $$ = $1;
  }
  | DefcalGrammarDecl {
    $$ = ASTProductionFactory::Instance().ProductionRule_5000($1);
  }
  | ArrayExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3000($1);
  }
  | InitArrayExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3000($1);
  }
  | MPComplexDecl ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_9000($1);
  }
  | MPComplexFunctionCallDecl ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_9000($1);
  }
  | ModifierDecl {
    $$ = $1;
  }
  ;

ConstDecl
  : TOK_CONST Decl {
    $$ = ASTProductionFactory::Instance().ProductionRule_170($2);
  }
  ;

ModifierDecl
  : TOK_INPUT Decl {
    ASTDeclarationNode* D = $2;
    assert(D && "Invalid ASTDeclarationNode argument!");

    D->SetModifier(ASTInputModifierNode::Instance());
    $$ = D;
  }
  | TOK_OUTPUT Decl {
    ASTDeclarationNode* D = $2;
    assert(D && "Invalid ASTDeclarationNode argument!");

    D->SetModifier(ASTOutputModifierNode::Instance());
    $$ = D;
  }
  ;

OpenPulseDecl
  : OpenPulseFrame ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1750(GET_TOKEN(1), $1);
  }
  | OpenPulsePort ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1751(GET_TOKEN(1), $1);
  }
  | OpenPulseWaveform ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1752(GET_TOKEN(1), $1);
  }
  | OpenPulseWaveform {
    $$ = ASTProductionFactory::Instance().ProductionRule_1752(GET_TOKEN(0), $1);
  }
  ;

OpenPulseStmt
  : OpenPulsePlay ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1760(GET_TOKEN(1), $1);
  }
  | OpenPulseCalibration {
    $$ = ASTProductionFactory::Instance().ProductionRule_1761(GET_TOKEN(0), $1);
  }
  ;

OpenPulseFrame
  : TOK_FRAME Identifier '=' TOK_NEWFRAME '(' ExprList ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1700(GET_TOKEN(6), $2, $6);
  }
  | TOK_EXTERN TOK_FRAME Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_1701(GET_TOKEN(1), $3);
  }
  | TOK_NEWFRAME '(' ExprList ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1702(GET_TOKEN(3), $3);
  }
  ;

OpenPulsePort
  : TOK_EXTERN TOK_PORT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_1710(GET_TOKEN(1), $3);
  }
  ;

OpenPulseWaveform
  : TOK_WAVEFORM Identifier '=' '[' ExprList ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1720(GET_TOKEN(5), $2, $5);
  }
  | TOK_WAVEFORM Identifier '=' '{' ExprList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1720(GET_TOKEN(5), $2, $5);
  }

  | TOK_WAVEFORM Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_1721(GET_TOKEN(3), $2, $4);
  }
  ;

OpenPulsePlay
  : TOK_PLAY '(' '[' ExprList ']' ',' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1730(GET_TOKEN(7), $7, $4);
  }
  | TOK_PLAY '(' '[' ExprList ']' ',' OpenPulseFrame ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1731(GET_TOKEN(7), $4, $7);
  }
  | TOK_PLAY '(' OpenPulseWaveform ',' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1732(GET_TOKEN(5), $3, $5);
  }
  | TOK_PLAY '(' Identifier ',' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1733(GET_TOKEN(5), $3, $5);
  }
  | TOK_PLAY '(' Identifier ',' OpenPulseFrame ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1734(GET_TOKEN(5), $3, $5);
  }
  | TOK_PLAY '(' OpenPulseWaveform ',' OpenPulseFrame ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1735(GET_TOKEN(5), $3, $5);
  }
  | TOK_PLAY '(' ParenFunctionCallExpr ',' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1736(GET_TOKEN(5), $3, $5);
  }
  | TOK_PLAY '(' ParenFunctionCallExpr ',' OpenPulseFrame ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1737(GET_TOKEN(5), $3, $5);
  }
  ;

OpenPulseCalibration
  : TOK_CAL '{' OpenPulseStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1740(GET_TOKEN(3), $3);
  }
  ;

NamedTypeDeclList
  : NamedTypeDeclListImpl NamedTypeDecl {
    $1->Append($2);
  }
  | NamedTypeDeclListImpl ParamTypeDecl {
    $1->Append($2);
  }
  ;

NamedTypeDeclListImpl
  : %empty  {
    $$ = ASTNamedTypeDeclarationBuilder::Instance().List();
    $$->Clear();
  }
  | NamedTypeDeclListImpl NamedTypeDecl  {
    $1->Append($2);
  }
  | NamedTypeDeclListImpl NamedTypeDecl ',' {
    $1->Append($2);
  }
  | NamedTypeDeclListImpl ParamTypeDecl ',' {
    $1->Append($2);
  }
  ;

PragmaExpr
  : TOK_PRAGMA TOK_STRING_LITERAL {
    $$ = ASTProductionFactory::Instance().ProductionRule_2600(GET_TOKEN(1), $2);
  }
  ;

PragmaStmt
  : PragmaExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_2601($1);
  }
  ;

AnnotationExpr
  : TOK_ANNOTATION StringList {
    $$ = ASTProductionFactory::Instance().ProductionRule_2610(GET_TOKEN(1), $2);
  }
  ;

AnnotationStmt
  : AnnotationExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_2610(GET_TOKEN(0), $1);
  }
  ;

FuncResult
  : TOK_RIGHT_ARROW TOK_ANGLE {
    $$ = ASTProductionFactory::Instance().ProductionRule_2100(GET_TOKEN(1));
  }
  | TOK_RIGHT_ARROW TOK_ANGLE '[' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2101($4, GET_TOKEN(3));
  }
  | TOK_RIGHT_ARROW TOK_ANGLE '[' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2102($4, GET_TOKEN(3));
  }
  | TOK_RIGHT_ARROW TOK_ANGLE '[' BinaryOpExpr ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2103($4, GET_TOKEN(3));
  }
  | TOK_RIGHT_ARROW TOK_BIT {
    $$ = ASTProductionFactory::Instance().ProductionRule_2104(GET_TOKEN(1));
  }
  | TOK_RIGHT_ARROW TOK_BIT '[' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2105($4, GET_TOKEN(3));
  }
  | TOK_RIGHT_ARROW TOK_BIT '[' BinaryOpExpr ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2106($4, GET_TOKEN(3));
  }
  | TOK_RIGHT_ARROW TOK_BIT '[' TOK_INTEGER_CONSTANT ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2107($4, GET_TOKEN(3));
  }
  | TOK_RIGHT_ARROW TOK_QUBIT {
    $$ = ASTProductionFactory::Instance().ProductionRule_2108(GET_TOKEN(0));
  }
  | TOK_RIGHT_ARROW TOK_QUBIT '[' Expr ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2108(GET_TOKEN(3));
  }
  | TOK_RIGHT_ARROW TOK_BOOL {
    $$ = ASTProductionFactory::Instance().ProductionRule_2109(GET_TOKEN(0));
  }
  | TOK_RIGHT_ARROW TOK_INT {
    $$ = ASTProductionFactory::Instance().ProductionRule_2110(GET_TOKEN(0));
  }
  | TOK_RIGHT_ARROW TOK_UINT {
    $$ = ASTProductionFactory::Instance().ProductionRule_2111(GET_TOKEN(0));
  }
  | TOK_RIGHT_ARROW MPIntegerType {
    $$ = ASTProductionFactory::Instance().ProductionRule_2112($2);
  }
  | TOK_RIGHT_ARROW TOK_FLOAT {
    $$ = ASTProductionFactory::Instance().ProductionRule_2113(GET_TOKEN(0));
  }
  | TOK_RIGHT_ARROW TOK_DOUBLE {
    $$ = ASTProductionFactory::Instance().ProductionRule_2114(GET_TOKEN(0));
  }
  | TOK_RIGHT_ARROW MPDecimalType {
    $$ = ASTProductionFactory::Instance().ProductionRule_2115($2);
  }
  | TOK_RIGHT_ARROW MPComplexType {
    $$ = ASTProductionFactory::Instance().ProductionRule_2116($2);
  }
  | TOK_RIGHT_ARROW TOK_WAVEFORM {
    $$ = ASTProductionFactory::Instance().ProductionRule_2117(GET_TOKEN(0));
  }
  | TOK_RIGHT_ARROW TOK_FRAME {
    $$ = ASTProductionFactory::Instance().ProductionRule_2118(GET_TOKEN(0));
  }
  ;

FuncDecl
  : TOK_FUNCTION_DEFINITION Identifier '(' NamedTypeDeclList ')' FuncResult
                                                      '{' FuncStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2300(GET_TOKEN(8),
                                                              $2, $4, $6, $8);
  }
  | TOK_FUNCTION_DEFINITION Identifier '(' ')' FuncResult '{' FuncStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2301(GET_TOKEN(7),
                                                              $2, $5, $7);
  }
  | TOK_FUNCTION_DEFINITION Identifier FuncResult '{' FuncStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2301(GET_TOKEN(5),
                                                              $2, $3, $5);
  }
  | TOK_FUNCTION_DEFINITION Identifier '(' NamedTypeDeclList ')' '{' FuncStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2302(GET_TOKEN(7),
                                                              $2, $4, $7);
  }
  | TOK_FUNCTION_DEFINITION Identifier '(' ')' '{' FuncStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2303(GET_TOKEN(6),
                                                              $2, $6);
  }
  | TOK_FUNCTION_DEFINITION Identifier '{' FuncStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2303(GET_TOKEN(4),
                                                              $2, $4);
  }
  ;

GateDecl
  : TOK_GATE Identifier '(' NamedTypeDeclList ')' GateQubitParamList '{' GateOpList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1430(GET_TOKEN(8),
                                                              $2, $4, $6, $8);
  }
  | TOK_GATE Identifier GateQubitParamList '{' GateOpList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1431(GET_TOKEN(5),
                                                              $2, $3, $5);
  }
  | TOK_GATE TOK_CX GateQubitParamList '{' GateOpList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1432(GET_TOKEN(5), $3,
                                          GET_TOKEN(4)->GetLocation(), $5);
  }
  | TOK_GATE TOK_HADAMARD GateQubitParamList '{' GateOpList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1433(GET_TOKEN(5), $2,
                                          GET_TOKEN(4)->GetLocation(), $3, $5);
  }
  | TOK_GATE TOK_HADAMARD '(' NamedTypeDeclList ')' GateQubitParamList '{' GateOpList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1434(GET_TOKEN(8), $2,
                                          GET_TOKEN(7)->GetLocation(),
                                          $4, $6, $8);
  }
  ;

OpaqueDecl
  : TOK_OPAQUE Identifier '(' NamedTypeDeclList ')' GateQubitParamList ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1435(GET_TOKEN(6),
                                                              $2, $4, $6);
  }
  | TOK_OPAQUE Identifier GateQubitParamList ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1436(GET_TOKEN(3),
                                                              $2, $3);
  }
  ;

BinaryOpAssign
  : Identifier '=' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_450(GET_TOKEN(2), $1, $3);
  }
  | Identifier '=' FunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_451(GET_TOKEN(2), $1, $3);
  }
  | Identifier '=' ComplexInitializerExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_452(GET_TOKEN(2), $1, $3);
  }
  | Identifier '=' BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_453(GET_TOKEN(2), $1, $3);
  }
  | Identifier '=' BinaryOpAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_453(GET_TOKEN(2), $1, $3);
  }
  | Identifier '=' BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_453(GET_TOKEN(2), $1, $3);
  }
  | Identifier '=' RotateOpExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_454(GET_TOKEN(2), $1, $3);
  }
  | Identifier '=' PopcountOpExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_455(GET_TOKEN(2), $1, $3);
  }
  | Identifier '=' TimeUnit {
    $$ = ASTProductionFactory::Instance().ProductionRule_456(GET_TOKEN(2), $1, $3);
  }
  | Identifier '=' BooleanConstant {
    $$ = ASTProductionFactory::Instance().ProductionRule_457(GET_TOKEN(2), $1, $3);
  }
  | TOK_INTEGER_CONSTANT '=' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_7001(GET_TOKEN(2), $3);
  }
  | TOK_FP_CONSTANT '=' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_7001(GET_TOKEN(2), $3);
  }
  | TOK_STRING_LITERAL '=' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_7001(GET_TOKEN(2), $3);
  }
  | BinaryOp '=' BinaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | BinaryOp '=' BinaryOpAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | BinaryOp '=' BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | BinaryOp '=' BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | BinaryOp '=' UnaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | BinaryOp '=' RotateOpExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | BinaryOp '=' PopcountOpExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | BinaryOp '=' FunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | BinaryOp '=' ComplexInitializerExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | BinaryOp '=' TimeUnit {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | BinaryOp '=' BooleanConstant {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | BinaryOpSelfAssign '=' BinaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | BinaryOpPrePost '=' BinaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | UnaryOp '=' BinaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | UnaryOp '=' BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  | UnaryOp '=' BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_7002(GET_TOKEN(2));
  }
  ;

BinaryOpSelfAssign
  : Expr TOK_RIGHT_SHIFT_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeRightShiftAssign);
  }
  | Expr TOK_RIGHT_SHIFT_ASSIGN ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeRightShiftAssign);
  }
  | BinaryOpSelfAssign TOK_RIGHT_SHIFT_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeRightShiftAssign);
  }
  | BinaryOpPrePost TOK_RIGHT_SHIFT_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeRightShiftAssign);
  }
  | Expr TOK_RIGHT_SHIFT_ASSIGN BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeRightShiftAssign);
  }
  | Expr TOK_RIGHT_SHIFT_ASSIGN BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeRightShiftAssign);
  }
  | Expr TOK_LEFT_SHIFT_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeLeftShiftAssign);
  }
  | Expr TOK_LEFT_SHIFT_ASSIGN ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeLeftShiftAssign);
  }
  | BinaryOpSelfAssign TOK_LEFT_SHIFT_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeLeftShiftAssign);
  }
  | BinaryOpPrePost TOK_LEFT_SHIFT_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeLeftShiftAssign);
  }
  | Expr TOK_LEFT_SHIFT_ASSIGN BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeLeftShiftAssign);
  }
  | Expr TOK_LEFT_SHIFT_ASSIGN BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeLeftShiftAssign);
  }
  | Expr TOK_ADD_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeAddAssign);
  }
  | Expr TOK_ADD_ASSIGN ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeAddAssign);
  }
  | BinaryOpSelfAssign TOK_ADD_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeAddAssign);
  }
  | BinaryOpPrePost TOK_ADD_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeAddAssign);
  }
  | Expr TOK_ADD_ASSIGN BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeAddAssign);
  }
  | Expr TOK_ADD_ASSIGN BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeAddAssign);
  }
  | Expr TOK_SUB_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeSubAssign);
  }
  | Expr TOK_SUB_ASSIGN ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeSubAssign);
  }
  | BinaryOpSelfAssign TOK_SUB_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeSubAssign);
  }
  | BinaryOpPrePost TOK_SUB_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeSubAssign);
  }
  | Expr TOK_SUB_ASSIGN BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeSubAssign);
  }
  | Expr TOK_SUB_ASSIGN BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeSubAssign);
  }
  | Expr TOK_MUL_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeMulAssign);
  }
  | Expr TOK_MUL_ASSIGN ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeMulAssign);
  }
  | BinaryOpSelfAssign TOK_MUL_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeMulAssign);
  }
  | BinaryOpPrePost TOK_MUL_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeMulAssign);
  }
  | Expr TOK_MUL_ASSIGN BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeMulAssign);
  }
  | Expr TOK_MUL_ASSIGN BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeMulAssign);
  }
  | Expr TOK_DIV_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeDivAssign);
  }
  | Expr TOK_DIV_ASSIGN ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeDivAssign);
  }
  | BinaryOpSelfAssign TOK_DIV_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeDivAssign);
  }
  | BinaryOpPrePost TOK_DIV_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeDivAssign);
  }
  | Expr TOK_DIV_ASSIGN BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeDivAssign);
  }
  | Expr TOK_DIV_ASSIGN BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeDivAssign);
  }
  | Expr TOK_MOD_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeModAssign);
  }
  | Expr TOK_MOD_ASSIGN ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeModAssign);
  }
  | BinaryOpSelfAssign TOK_MOD_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeModAssign);
  }
  | BinaryOpPrePost TOK_MOD_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeModAssign);
  }
  | Expr TOK_MOD_ASSIGN BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeModAssign);
  }
  | Expr TOK_MOD_ASSIGN BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeModAssign);
  }
  | Expr TOK_AND_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeBitAndAssign);
  }
  | Expr TOK_AND_ASSIGN ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeBitAndAssign);
  }
  | BinaryOpSelfAssign TOK_AND_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeBitAndAssign);
  }
  | BinaryOpPrePost TOK_AND_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeBitAndAssign);
  }
  | Expr TOK_AND_ASSIGN BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeBitAndAssign);
  }
  | Expr TOK_AND_ASSIGN BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeBitAndAssign);
  }
  | Expr TOK_OR_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeBitOrAssign);
  }
  | Expr TOK_OR_ASSIGN ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeBitOrAssign);
  }
  | BinaryOpSelfAssign TOK_OR_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeBitOrAssign);
  }
  | BinaryOpPrePost TOK_OR_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeBitOrAssign);
  }
  | Expr TOK_OR_ASSIGN BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeBitOrAssign);
  }
  | Expr TOK_OR_ASSIGN BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeBitOrAssign);
  }
  | Expr TOK_XOR_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeXorAssign);
  }
  | Expr TOK_XOR_ASSIGN ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeXorAssign);
  }
  | BinaryOpSelfAssign TOK_XOR_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeXorAssign);
  }
  | BinaryOpPrePost TOK_XOR_ASSIGN Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeXorAssign);
  }
  | Expr TOK_XOR_ASSIGN BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeXorAssign);
  }
  | Expr TOK_XOR_ASSIGN BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                          $3, ASTOpTypeXorAssign);
  }
  ;

BinaryOpPrePost
  : TOK_DEC_OP Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_600(GET_TOKEN(1), $2,
                                                             ASTOpTypePreDec);
  }
  | TOK_DEC_OP '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_601(GET_TOKEN(3), $3,
                                                             ASTOpTypePreDec);
  }
  | TOK_INC_OP Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_600(GET_TOKEN(1), $2,
                                                             ASTOpTypePreInc);
  }
  | TOK_INC_OP '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_601(GET_TOKEN(3), $3,
                                                             ASTOpTypePreInc);
  }
  | Identifier TOK_DEC_OP {
    $$ = ASTProductionFactory::Instance().ProductionRule_602(GET_TOKEN(1), $1,
                                                             ASTOpTypePostDec);
  }
  | '(' Expr ')' TOK_DEC_OP {
    $$ = ASTProductionFactory::Instance().ProductionRule_603(GET_TOKEN(3), $2,
                                                             ASTOpTypePostDec);
  }
  | Identifier TOK_INC_OP {
    $$ = ASTProductionFactory::Instance().ProductionRule_602(GET_TOKEN(1), $1,
                                                             ASTOpTypePostInc);
  }
  | '(' Expr ')' TOK_INC_OP {
    $$ = ASTProductionFactory::Instance().ProductionRule_603(GET_TOKEN(3), $2,
                                                             ASTOpTypePostInc);
  }
  ;

BinaryOp
  : Expr TOK_ADD_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeAdd);
  }
  | Expr TOK_ADD_OP ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeAdd);
  }
  | Expr TOK_ADD_OP BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeAdd);
  }
  | Expr TOK_ADD_OP BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeAdd);
  }
  | BinaryOpPrePost TOK_ADD_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeAdd);
  }
  | BinaryOpSelfAssign TOK_ADD_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeAdd);
  }
  | Expr '-' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeSub);
  }
  | Expr '-' ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeSub);
  }
  | Expr '-' BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeSub);
  }
  | Expr '-' BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeSub);
  }
  | BinaryOpPrePost '-' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeSub);
  }
  | BinaryOpSelfAssign '-' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeSub);
  }
  | Expr TOK_MUL_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeMul);
  }
  | Expr TOK_MUL_OP ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeMul);
  }
  | Expr TOK_MUL_OP BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeMul);
  }
  | Expr TOK_MUL_OP BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeMul);
  }
  | BinaryOpPrePost TOK_MUL_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeMul);
  }
  | BinaryOpSelfAssign TOK_MUL_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeMul);
  }
  | Expr TOK_MUL_OP TOK_MUL_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(3), $1,
                                                             $4, ASTOpTypePow);
  }
  | Expr TOK_MUL_OP TOK_MUL_OP ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(3), $1,
                                                             $4, ASTOpTypeMul);
  }
  | Expr TOK_MUL_OP TOK_MUL_OP BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(3), $1,
                                                             $4, ASTOpTypePow);
  }
  | Expr TOK_MUL_OP TOK_MUL_OP BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(3), $1,
                                                             $4, ASTOpTypePow);
  }
  | BinaryOpPrePost TOK_MUL_OP TOK_MUL_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(3), $1,
                                                             $4, ASTOpTypePow);
  }
  | BinaryOpSelfAssign TOK_MUL_OP TOK_MUL_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(3), $1,
                                                             $4, ASTOpTypePow);
  }
  | Expr TOK_DIV_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeDiv);
  }
  | Expr TOK_DIV_OP ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeDiv);
  }
  | Expr TOK_DIV_OP BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeDiv);
  }
  | Expr TOK_DIV_OP BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeDiv);
  }
  | BinaryOpPrePost TOK_DIV_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeDiv);
  }
  | BinaryOpSelfAssign TOK_DIV_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeDiv);
  }
  | Expr TOK_MOD_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeMod);
  }
  | Expr TOK_MOD_OP ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeMod);
  }
  | Expr TOK_MOD_OP BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeMod);
  }
  | Expr TOK_MOD_OP BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeMod);
  }
  | BinaryOpPrePost TOK_MOD_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeMod);
  }
  | BinaryOpSelfAssign TOK_MOD_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeMod);
  }
  | Expr '^' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeXor);
  }
  | Expr '^' ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeXor);
  }
  | Expr '^' BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeXor);
  }
  | BinaryOpPrePost '^' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeXor);
  }
  | Expr '^' BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeXor);
  }
  | BinaryOpSelfAssign '^' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeXor);
  }
  | Expr '|' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeBitOr);
  }
  | Expr '|' ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeBitOr);
  }
  | Expr '|' BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeBitOr);
  }
  | BinaryOpPrePost '|' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeBitOr);
  }
  | Expr '|' BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeBitOr);
  }
  | BinaryOpSelfAssign '|' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeBitOr);
  }
  | Expr '&' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeBitAnd);
  }
  | Expr '&' ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeBitAnd);
  }
  | Expr '&' BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeBitAnd);
  }
  | BinaryOpPrePost '&' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeBitAnd);
  }
  | Expr '&' BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeBitAnd);
  }
  | BinaryOpSelfAssign '&' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeBitAnd);
  }
  | Expr '<' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLT);
  }
  | Expr '<' ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLT);
  }
  | Expr '<' BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLT);
  }
  | BinaryOpPrePost '<' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLT);
  }
  | Expr '<' BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLT);
  }
  | BinaryOpSelfAssign '<' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLT);
  }
  | Expr '>' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeGT);
  }
  | Expr '>' ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeGT);
  }
  | Expr '>' BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeGT);
  }
  | BinaryOpPrePost '>' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeGT);
  }
  | Expr '>' BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeGT);
  }
  | BinaryOpSelfAssign '>' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeGT);
  }
  | Expr TOK_LE_OP Expr  {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLE);
  }
  | Expr TOK_LE_OP ParenFunctionCallExpr  {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLE);
  }
  | Expr TOK_LE_OP BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLE);
  }
  | BinaryOpPrePost TOK_LE_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLE);
  }
  | Expr TOK_LE_OP BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLE);
  }
  | BinaryOpSelfAssign TOK_LE_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLE);
  }
  | Expr TOK_GE_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeGE);
  }
  | Expr TOK_GE_OP ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeGE);
  }
  | Expr TOK_GE_OP BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeGE);
  }
  | BinaryOpPrePost TOK_GE_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeGE);
  }
  | Expr TOK_GE_OP BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeGE);
  }
  | BinaryOpSelfAssign TOK_GE_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeGE);
  }
  | Expr TOK_OR_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLogicalOr);
  }
  | Expr TOK_OR_OP ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLogicalOr);
  }
  | Expr TOK_OR_OP BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLogicalOr);
  }
  | BinaryOpPrePost TOK_OR_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLogicalOr);
  }
  | Expr TOK_OR_OP BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLogicalOr);
  }
  | BinaryOpSelfAssign TOK_OR_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLogicalOr);
  }
  | Expr TOK_AND_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLogicalAnd);
  }
  | Expr TOK_AND_OP ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLogicalAnd);
  }
  | Expr TOK_AND_OP BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLogicalAnd);
  }
  | BinaryOpPrePost TOK_AND_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLogicalAnd);
  }
  | Expr TOK_AND_OP BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLogicalAnd);
  }
  | BinaryOpSelfAssign TOK_AND_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLogicalAnd);
  }
  | Expr TOK_EQ_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeCompEq);
  }
  | Expr TOK_EQ_OP ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeCompEq);
  }
  | Expr TOK_EQ_OP BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeCompEq);
  }
  | BinaryOpPrePost TOK_EQ_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeCompEq);
  }
  | Expr TOK_EQ_OP BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeCompEq);
  }
  | BinaryOpSelfAssign TOK_EQ_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeCompEq);
  }
  | Expr TOK_NE_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeCompNeq);
  }
  | Expr TOK_NE_OP ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeCompNeq);
  }
  | Expr TOK_NE_OP BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeCompNeq);
  }
  | BinaryOpPrePost TOK_NE_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeCompNeq);
  }
  | Expr TOK_NE_OP BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeCompNeq);
  }
  | BinaryOpSelfAssign TOK_NE_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeCompNeq);
  }
  | Expr TOK_LEFT_SHIFT_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLeftShift);
  }
  | Expr TOK_LEFT_SHIFT_OP ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLeftShift);
  }
  | Expr TOK_LEFT_SHIFT_OP BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLeftShift);
  }
  | BinaryOpPrePost TOK_LEFT_SHIFT_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLeftShift);
  }
  | Expr TOK_LEFT_SHIFT_OP BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLeftShift);
  }
  | BinaryOpSelfAssign TOK_LEFT_SHIFT_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeLeftShift);
  }
  | Expr TOK_RIGHT_SHIFT_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeRightShift);
  }
  | Expr TOK_RIGHT_SHIFT_OP ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeRightShift);
  }
  | Expr TOK_RIGHT_SHIFT_OP BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeRightShift);
  }
  | BinaryOpPrePost TOK_RIGHT_SHIFT_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeRightShift);
  }
  | Expr TOK_RIGHT_SHIFT_OP BinaryOpSelfAssign {
    $$ = ASTProductionFactory::Instance().ProductionRule_588(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeRightShift);
  }
  | BinaryOpSelfAssign TOK_RIGHT_SHIFT_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_594(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeRightShift);
  }
  | Expr TOK_ASSOCIATION_OP Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_580(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeAssociate);
  }
  | Expr TOK_ASSOCIATION_OP ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_584(GET_TOKEN(2), $1,
                                                             $3, ASTOpTypeAssociate);
  }
  ;

BinaryOpExpr
  : BinaryOp {
    $$ = $1;
  }
  | BinaryOpAssign {
    $$ = $1;
  }
  | BinaryOpSelfAssign {
    $$ = $1;
  }
  | BinaryOpPrePost {
    $$ = $1;
  }
  | ArithPowExpr {
    $$ = $1;
  }
  ;

BinaryOpStmt
  : BinaryOpAssign ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_605(GET_TOKEN(1), $1);
  }
  | BinaryOpSelfAssign ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_605(GET_TOKEN(1), $1);
  }
  | BinaryOpPrePost ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_605(GET_TOKEN(1), $1);
  }
  ;

Barrier
  : TOK_BARRIER IdentifierList ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1450(GET_TOKEN(2), $2);
  }
  | TOK_BARRIER ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1451(GET_TOKEN(1));
  }
  ;

Reset
  : TOK_RESET Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1480(GET_TOKEN(2), $2);
  }
  ;

Measure
  : MeasureDecl ';' {
    $$ = $1;
  }
  | Identifier '=' TOK_MEASURE Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1461(GET_TOKEN(2),
                                                              $4, $1);
  }
  | TOK_IDENTIFIER '[' Integer ':' Integer ']' '=' TOK_MEASURE TOK_IDENTIFIER '[' Integer ':' Integer ']' ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1463(GET_TOKEN(7), *$9,
                                                              GET_TOKEN(6), $11,
                                                              $13, *$1, GET_TOKEN(14),
                                                              $3, $5);
  }
  | TOK_MEASURE Identifier ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1462(GET_TOKEN(2), $2);
  }
  ;

MeasureDecl
  : TOK_MEASURE Identifier TOK_RIGHT_ARROW Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_1461(GET_TOKEN(3), $2, $4);
  }
  | TOK_MEASURE Identifier TOK_RIGHT_ARROW TOK_BIT {
    $$ = ASTProductionFactory::Instance().ProductionRule_1462(GET_TOKEN(3), $2);
  }
  | TOK_MEASURE TOK_IDENTIFIER '[' Integer ':' Integer ']' TOK_RIGHT_ARROW TOK_IDENTIFIER '[' Integer ':' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1463(GET_TOKEN(13), *$2,
                                                              GET_TOKEN(12), $4,
                                                              $6, *$9, GET_TOKEN(5),
                                                              $11, $13);
  }
  | TOK_MEASURE Identifier TOK_RIGHT_ARROW TOK_BIT '[' TOK_INTEGER_CONSTANT ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1464(GET_TOKEN(6), $2, *$6);
  }
  ;

DefcalDecl
  : TOK_DEFCAL Identifier '(' ExprList ')' QubitList '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1440(GET_TOKEN(8),
                                                              $2, $4, $6, $8);
  }
  | TOK_DEFCAL Identifier QubitList '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1441(GET_TOKEN(5),
                                                              $2, $3, $5);
  }
  | TOK_DEFCAL String Identifier '(' ExprList ')' QubitList '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1440(GET_TOKEN(9),
                                                              $3, $5, $7, $9, $2);
  }
  | TOK_DEFCAL String Identifier QubitList '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1441(GET_TOKEN(6),
                                                              $3, $4, $6, $2);
  }
  | TOK_DEFCAL MeasureDecl '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1442($2, GET_TOKEN(4),
                                                              $4);
  }
  | TOK_DEFCAL String MeasureDecl '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1442($3, GET_TOKEN(5),
                                                              $5, $2);
  }
  | TOK_DEFCAL TOK_RESET Identifier '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1443(GET_TOKEN(5),
                                                              GET_TOKEN(4), $3, $5);
  }
  | TOK_DEFCAL String TOK_RESET Identifier '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1443(GET_TOKEN(6),
                                                              GET_TOKEN(4), $4,
                                                              $6, $2);
  }
  | TOK_DEFCAL TOK_DELAY '[' TimeUnit ']' QubitList '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1444(GET_TOKEN(8),
                                                              GET_TOKEN(7),
                                                              $4, $6, $8);
  }
  | TOK_DEFCAL String TOK_DELAY '[' TimeUnit ']' QubitList '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1444(GET_TOKEN(9),
                                                              GET_TOKEN(7),
                                                              $5, $7, $9, $2);
  }
  | TOK_DEFCAL TOK_DELAY '[' DurationOfDecl ']' QubitList '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1445(GET_TOKEN(8),
                                                              GET_TOKEN(7),
                                                              $4, $6, $8);
  }
  | TOK_DEFCAL TOK_DELAY '[' Identifier ']' QubitList '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1445(GET_TOKEN(8),
                                                              GET_TOKEN(7),
                                                              $4, $6, $8);
  }
  | TOK_DEFCAL String TOK_DELAY '[' DurationOfDecl ']' QubitList '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1445(GET_TOKEN(9),
                                                              GET_TOKEN(7),
                                                              $5, $7, $9, $2);
  }
  | TOK_DEFCAL String TOK_DELAY '[' Identifier ']' QubitList '{' DefcalStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1445(GET_TOKEN(9),
                                                              GET_TOKEN(7),
                                                              $5, $7, $9, $2);
  }
  ;

DefcalGrammarDecl
  : TOK_DEFCAL_GRAMMAR String ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1445(GET_TOKEN(2), $2);
  }
  ;

DurationOfDecl
  : TOK_DURATIONOF '(' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1250(GET_TOKEN(3), $3);
  }
  | TOK_DURATIONOF '(' '{' GateEOp ';' '}' ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1251(GET_TOKEN(6), $4);
  }
  ;

DurationDecl
  : TOK_DURATION Identifier '=' TimeUnit  {
    $$ = ASTProductionFactory::Instance().ProductionRule_1200(GET_TOKEN(3),
                                                              $2, $4);
  }
  | TOK_DURATION Identifier '=' DurationOfDecl {
    $$ = ASTProductionFactory::Instance().ProductionRule_1201(GET_TOKEN(3),
                                                              $2, $4);
  }
  | TOK_DURATION '[' TimeUnit ']' Identifier '=' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_1202(GET_TOKEN(6),
                                                              $5, $7, $3);
  }
  | TOK_DURATION Identifier '=' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_1203(GET_TOKEN(3),
                                                              $2, $4);
  }
  | TOK_DURATION Identifier '=' BinaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_1204(GET_TOKEN(3),
                                                              $2, $4);
  }
  | TOK_DURATION Identifier '=' FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_1205(GET_TOKEN(3),
                                                              $2, $4);
  }
  ;

ImplicitDuration
  : TimeUnit {
    $$ = ASTProductionFactory::Instance().ProductionRule_1209(GET_TOKEN(0), $1);
  }
  ;

IfStmt
  : TOK_IF '(' Expr ')' Statement %prec XIF {
    $$ = ASTProductionFactory::Instance().ProductionRule_3000(GET_TOKEN(4),
                                                              $3, $5);
  }
  | TOK_IF '(' Expr ')' '{' IfStmtList '}' %prec XIF {
    $$ = ASTProductionFactory::Instance().ProductionRule_3001(GET_TOKEN(6),
                                                              $3, $6, true);
  }
  ;

ElseIfStmt
  : TOK_ELSEIF '(' Expr ')' Statement %prec XELSEIF {
    $$ = ASTProductionFactory::Instance().ProductionRule_3010(GET_TOKEN(4),
                                                              $3, $5);
  }
  | TOK_ELSEIF '(' Expr ')' '{' ElseIfStmtList '}' %prec XELSEIF {
    $$ = ASTProductionFactory::Instance().ProductionRule_3011(GET_TOKEN(6),
                                                              $3, $6, true);
  }
  ;

ElseStmt
  : TOK_ELSE Statement {
    $$ = ASTProductionFactory::Instance().ProductionRule_3020(GET_TOKEN(1), $2);
  }
  | TOK_ELSE '{' ElseStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3021(GET_TOKEN(3),
                                                              $3, true);
  }
  ;

ForStmt
  : TOK_FOR Identifier TOK_IN '[' IntegerList ']' '{' ForStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3200(GET_TOKEN(8),
                                                              $2, $5, $8);
  }
  | TOK_FOR Identifier TOK_IN '[' IntegerList ']' Statement  {
    $$ = ASTProductionFactory::Instance().ProductionRule_3201(GET_TOKEN(6),
                                                              $2, $5, $7);
  }
  | TOK_FOR Identifier TOK_IN '{' IntegerList '}' Statement {
    $$ = ASTProductionFactory::Instance().ProductionRule_3202(GET_TOKEN(6),
                                                              $2, $5, $7);
  }
  | TOK_FOR Identifier TOK_IN '{' IntegerList '}' '{' ForStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3203(GET_TOKEN(8),
                                                              $2, $5, $8);
  }
  | TOK_FOR Identifier TOK_IN '[' ForLoopRangeExpr ']' '{' ForStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3204(GET_TOKEN(8),
                                                              $2, $5, $8);
  }
  | TOK_FOR Identifier TOK_IN '[' ForLoopRangeExpr ']' Statement {
    $$ = ASTProductionFactory::Instance().ProductionRule_3205(GET_TOKEN(6),
                                                              $2, $5, $7);
  }
  | TOK_FOR IntScalarType Identifier TOK_IN '[' IntegerList ']' '{' ForStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3206(GET_TOKEN(9),
                                                              $2, $3, $6, $9);
  }
  | TOK_FOR IntScalarType Identifier TOK_IN '[' IntegerList ']' Statement  {
    $$ = ASTProductionFactory::Instance().ProductionRule_3207(GET_TOKEN(7),
                                                              $2, $3, $6, $8);
  }
  | TOK_FOR IntScalarType Identifier TOK_IN '{' IntegerList '}' '{' ForStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3208(GET_TOKEN(9),
                                                              $2, $3, $6, $9);
  }
  | TOK_FOR IntScalarType Identifier TOK_IN '{' IntegerList '}' Statement  {
    $$ = ASTProductionFactory::Instance().ProductionRule_3209(GET_TOKEN(7),
                                                              $2, $3, $6, $8);
  }
  | TOK_FOR IntScalarType Identifier TOK_IN '[' ForLoopRangeExpr ']' '{' ForStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3210(GET_TOKEN(9),
                                                              $2, $3, $6, $9);
  }
  | TOK_FOR IntScalarType Identifier TOK_IN '[' ForLoopRangeExpr ']' Statement {
    $$ = ASTProductionFactory::Instance().ProductionRule_3211(GET_TOKEN(7),
                                                              $2, $3, $6, $8);
  }
  ;

KernelDecl
  : TOK_EXTERN Identifier '(' NamedTypeDeclList ')' FuncResult ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2500(GET_TOKEN(6),
                                                              $2, $4, $6,
                                                              GET_TOKEN(1));
  }
  | TOK_EXTERN Identifier '(' NamedTypeDeclList ')' ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2501(GET_TOKEN(5),
                                                              $2, $4,
                                                              GET_TOKEN(0));
  }
  | TOK_EXTERN Identifier '(' ')' ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2502(GET_TOKEN(4),
                                                              $2, GET_TOKEN(0));
  }
  | TOK_EXTERN Identifier '(' ')' FuncResult ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2503(GET_TOKEN(5),
                                                              $2, $5,
                                                              GET_TOKEN(1));
  }
  | TOK_EXTERN Identifier FuncResult ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2503(GET_TOKEN(3),
                                                              $2, $3,
                                                              GET_TOKEN(1));
  }
  | TOK_EXTERN Identifier '(' NamedTypeDeclList ')' FuncResult '{' KernelStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2504(GET_TOKEN(8),
                                                              $2, $4, $8, $6,
                                                              GET_TOKEN(3));
  }
  ;

WhileStmt
  : TOK_WHILE '(' Expr ')' '{' WhileStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3300(GET_TOKEN(6),
                                                              $3, $6);
  }
  ;

DoWhileStmt
  : TOK_DO '{' DoWhileStmtList '}' TOK_WHILE '(' Expr ')' ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3400(GET_TOKEN(8),
                                                              $7, $3);
  }
  ;

SwitchScopedStatement
  : IfStmt {
    $$ = $1;
  }
  | ElseIfStmt {
    $$ = $1;
  }
  | ElseStmt {
    $$ = $1;
  }
  | ForStmt {
    $$ = $1;
  }
  | ContinueStmt {
    $$ = $1;
  }
  | WhileStmt {
    $$ = $1;
  }
  | DoWhileStmt {
    $$ = $1;
  }
  | SwitchStmt {
    $$ = $1;
  }
  | ReturnStmt {
    $$ = $1;
  }
  | FunctionCallStmt {
    $$ = $1;
  }
  | BinaryOpStmt {
    $$ = $1;
  }
  | Decl {
    $$ = $1;
  }
  | Barrier {
    $$ = $1;
  }
  | DelayStmt {
    $$ = $1;
  }
  | StretchStmt {
    $$ = $1;
  }
  | BoxStmt {
    $$ = $1;
  }
  | GateQOp   {
    $$ = $1;
  }
  | RotateOpStmt {
    $$ = $1;
  }
  | PopcountOpStmt {
    $$ = $1;
  }
  | IncludeStmt {
    // Ignored.
  }
  | LineDirective {
    $$ = $1;
  }
  | FileDirective {
    $$ = $1;
  }
  | Newline {
    $$ = nullptr;
  }
  ;

SwitchUnscopedStatement
  : IfStmt {
    $$ = $1;
  }
  | ElseIfStmt {
    $$ = $1;
  }
  | ElseStmt {
    $$ = $1;
  }
  | ForStmt {
    $$ = $1;
  }
  | ContinueStmt {
    $$ = $1;
  }
  | WhileStmt {
    $$ = $1;
  }
  | DoWhileStmt {
    $$ = $1;
  }
  | SwitchStmt {
    $$ = $1;
  }
  | ReturnStmt {
    $$ = $1;
  }
  | FunctionCallStmt {
    $$ = $1;
  }
  | BinaryOpStmt {
    $$ = $1;
  }
  | Decl {
    $$ = ASTProductionFactory::Instance().ProductionRule_7000($1);
  }
  | Barrier {
    $$ = $1;
  }
  | DelayStmt {
    $$ = $1;
  }
  | StretchStmt {
    $$ = $1;
  }
  | BoxStmt {
    $$ = $1;
  }
  | GateQOp   {
    $$ = $1;
  }
  | RotateOpStmt {
    $$ = $1;
  }
  | PopcountOpStmt {
    $$ = $1;
  }
  | IncludeStmt {
    // Ignored.
  }
  | LineDirective {
    $$ = $1;
  }
  | FileDirective {
    $$ = $1;
  }
  | Newline {
    $$ = nullptr;
  }
  ;
SwitchScopedStmtList
  : SwitchScopedStmtListImpl {
    $$ = ASTSwitchScopedStatementBuilder::Instance().List();
    assert($$ && "Could not obtain a valid SwitchScopedStmtList!");
    $$ = $1;
  }
  ;

SwitchScopedStmtListImpl
  : %empty {
    $$ = ASTSwitchScopedStatementBuilder::Instance().NewList();
    assert($$ && "Could not obtain a valid SwitchScopeStmtList!");
  }
  | SwitchScopedStmtListImpl SwitchScopedStatement {
    assert($1 && "Invalid SwitchScopedStmtListImpl argument!");
    assert($2 && "Invalid SwitchScopedStatement argument!");
    if ($2 && !$2->IsDirective())
      $1->Append($2);
  }
  ;

SwitchUnscopedStmtList
  : SwitchUnscopedStmtListImpl {
    $$ = ASTSwitchUnscopedStatementBuilder::Instance().List();
    assert($$ && "Could not obtain a valid SwitchUnscopedStmtList!");
    $$ = $1;
  }
  ;

SwitchUnscopedStmtListImpl
  : %empty {
    $$ = ASTSwitchUnscopedStatementBuilder::Instance().NewList();
    assert($$ && "Could not obtain a valid SwitchUnscopeStmtList!");
  }
  | SwitchUnscopedStmtListImpl SwitchUnscopedStatement {
    assert($1 && "Invalid SwitchUnscopedStmtListImpl argument!");
    assert($2 && "Invalid SwitchUnscopedStatement argument!");
    if ($2 && !$2->IsDirective())
      $1->Append($2);
  }
  ;

SwitchStmt
  : TOK_SWITCH '(' Integer ')' '{' SwitchStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3100(GET_TOKEN(6),
                                                              $3, $6);
  }
  | TOK_SWITCH '(' BinaryOp ')' '{' SwitchStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3101(GET_TOKEN(6),
                                                              $3, $6);
  }
  | TOK_SWITCH '(' UnaryOp ')' '{' SwitchStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3102(GET_TOKEN(6),
                                                              $3, $6);
  }
  | TOK_SWITCH '(' Identifier ')' '{' SwitchStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3103(GET_TOKEN(6),
                                                              $3, $6);
  }
  | TOK_SWITCH '(' ParenFunctionCallExpr ')' '{' SwitchStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3104(GET_TOKEN(6),
                                                              $3, $6);
  }
  ;

ReturnStmt
  : TOK_RETURN Expr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2000(GET_TOKEN(2), $2);
  }
  | TOK_RETURN BinaryOpAssign ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2002(GET_TOKEN(2), $2);
  }
  | TOK_RETURN BinaryOpSelfAssign ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2002(GET_TOKEN(2), $2);
  }
  | TOK_RETURN BinaryOpPrePost ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2002(GET_TOKEN(2), $2);
  }
  | TOK_RETURN FunctionCallStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_2003(GET_TOKEN(1), $2);
  }
  | TOK_RETURN Measure {
    $$ = ASTProductionFactory::Instance().ProductionRule_2004(GET_TOKEN(1), $2);
  }
  | TOK_RETURN ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2005(GET_TOKEN(1));
  }
  | TOK_RETURN TOK_BOOLEAN_CONSTANT ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2006(GET_TOKEN(2), *$2);
  }
  ;

BreakStmt
  : TOK_BREAK ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3150(GET_TOKEN(1));
  }
  ;

ContinueStmt
  : TOK_CONTINUE ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3151(GET_TOKEN(1));
  }
  ;

GateOpList
  : %empty {
    $$ = ASTGateOpBuilder::Instance().NewList();
  }
  | GateOpList GateUOp {
    // No need to append to the GateOpBuilder, the Gate already exists.
    $$ = ASTGateOpBuilder::Instance().List();
  }
  | GateOpList Barrier {
    ASTGateOpBuilder::Instance().Append($2);
  }
  | GateOpList Measure {
    ASTGateOpBuilder::Instance().Append($2);
  }
  | GateOpList GPhaseStmt {
    ASTGateOpBuilder::Instance().Append($2);
  }
  | GateOpList GateCtrlExpr {
    ASTGateControlNode* GCN = $2;
    assert(GCN && "Invalid ASTGateControlNode argument!");

    ASTGateQOpNode* GQN = new ASTGateQOpNode(GCN);
    assert(GQN && "Could not create a valid ASTGateQOpNode!");

    ASTGateOpBuilder::Instance().Append(GQN);
  }
  | GateOpList GateCtrlStmt {
    ASTGateQOpNode* GCS = $2;
    assert(GCS && "Invalid GateCtrlStmt argument!");

    ASTGateOpBuilder::Instance().Append(GCS);
  }
  | GateOpList GateNegCtrlExpr {
    ASTGateNegControlNode* GNCN = $2;
    assert(GNCN && "Invalid ASTGateNegControlNode argument!");

    ASTGateQOpNode* GQN = new ASTGateQOpNode(GNCN);
    assert(GQN && "Could not create a valid ASTGateQOpNode!");

    ASTGateOpBuilder::Instance().Append(GQN);
  }
  | GateOpList GateNegCtrlStmt {
    ASTGateQOpNode* GCS = $2;
    assert(GCS && "Invalid GateNegCtrlStmt argument!");

    ASTGateOpBuilder::Instance().Append(GCS);
  }
  | GateOpList GateInvExpr {
    ASTGateInverseNode* GIN = $2;
    assert(GIN && "Invalid ASTGateInverseNode argument!");

    ASTGateQOpNode* GQN = new ASTGateQOpNode(GIN);
    assert(GQN && "Could not create a valid ASTGateQOpNode!");

    ASTGateOpBuilder::Instance().Append(GQN);
  }
  | GateOpList GateInvStmt {
    ASTGateQOpNode* GCS = $2;
    assert(GCS && "Invalid GateInvStmt argument!");

    ASTGateOpBuilder::Instance().Append(GCS);
  }
  | GateOpList GatePowExpr {
    ASTGatePowerNode* GPN = $2;
    assert(GPN && "Invalid ASTGatePowerNode argument!");

    ASTGateQOpNode* GQN = new ASTGateQOpNode(GPN);
    assert(GQN && "Could not create a valid ASTGateQOpNode!");

    ASTGateOpBuilder::Instance().Append(GQN);
  }
  | GateOpList GatePowStmt {
    ASTGateQOpNode* GCS = $2;
    assert(GCS && "Invalid GatePowStmt argument!");

    ASTGateOpBuilder::Instance().Append(GCS);
  }
  | GateOpList GateGPhaseExpr {
    ASTGateGPhaseExpressionNode* GPE = $2;
    assert(GPE && "Invalid ASTGPhaseExpressionNode argument!");

    ASTGateQOpNode* GQN = new ASTGateQOpNode(GPE);
    assert(GQN && "Could not create a valid ASTGateQOpNode!");

    ASTGateOpBuilder::Instance().Append(GQN);
  }
  | GateOpList GateGPhaseStmt {
    ASTGateOpBuilder::Instance().Append($2);
  }
  | GateOpList GPhaseExpr {
    ASTGPhaseExpressionNode* GPE = $2;
    assert(GPE && "Invalid ASTGPhaseExpressionNode argument!");

    ASTGateQOpNode* GQN = new ASTGateQOpNode(GPE);
    assert(GQN && "Could not create a valid ASTGateQOpNode!");

    ASTGateOpBuilder::Instance().Append(GQN);
  }
  | GateOpList LineDirective {
    // Ignored. Only used by the Preprocessor and DIAGLineCounter.
  }
  | GateOpList FileDirective {
    // Ignored. Only used by the Preprocessor and DIAGLineCounter.
  }
  ;

GateQOp
  : GateUOp {
    $$ = $1;
  }
  | Measure {
    $$ = $1;
  }
  | Reset {
    $$ = $1;
  }
  | GPhaseStmt {
    $$ = $1;
  }
  ;

GateEOp
  : Identifier ArgsList AnyList {
    $$ = ASTProductionFactory::Instance().ProductionRule_3500(GET_TOKEN(2),
                                                              $1, $2, $3);
  }
  | TOK_U ArgsList AnyList {
    $$ = ASTProductionFactory::Instance().ProductionRule_3502(GET_TOKEN(2),
                                                              $2, $3);
  }
  | TOK_CX IdentifierList {
    $$ = ASTProductionFactory::Instance().ProductionRule_3503(GET_TOKEN(1), $2);
  }
  | TOK_CCX ArgsList AnyList {
    $$ = ASTProductionFactory::Instance().ProductionRule_3504(GET_TOKEN(2),
                                                              $2, $3);
  }
  | TOK_CNOT ArgsList AnyList {
    $$ = ASTProductionFactory::Instance().ProductionRule_3505(GET_TOKEN(2),
                                                              $2, $3);
  }
  | TOK_HADAMARD ArgsList AnyList {
    $$ = ASTProductionFactory::Instance().ProductionRule_3506(GET_TOKEN(2),
                                                              $2, $3);
  }
  ;

GateUOp
  : GateEOp ';' {
    $$ = $1;
  }
  ;

ArgsList
  : %empty {
    $$ = ASTArgumentNodeBuilder::Instance().NewList();
  }
  | '(' ExprList ')' {
    $$ = ASTArgumentNodeBuilder::Instance().List();
    *$$ = $2;
  }
  ;

IdentifierList
  : IdentifierListImpl Identifier {
    $1->Append($2);
  }
  ;

IdentifierListImpl
  : %empty {
    $$ = ASTIdentifierBuilder::Instance().NewList();
  }
  | IdentifierListImpl Identifier ',' {
    $1->Append($2);
  }
  ;

StringList
  : StringListImpl String {
    $$ = $1;
    $1->Append($2);
    ASTAnnotationContextBuilder::Instance().AddDirective($2->GetValue());
  }
  | StringListImpl TOK_ANNOTATION {
    assert($1 && "Invalid StringListImpl argument!");
    $1->Append(GET_TOKEN(0)->GetString());
  }
  ;

StringListImpl
  : %empty {
    $$ = ASTStringListBuilder::Instance().NewList();
  }
  | StringListImpl TOK_IDENTIFIER {
    assert($1 && "Invalid StringListImpl argument!");
    $1->Append(GET_TOKEN(0)->GetString());
  }
  | StringListImpl TOK_INTEGER_CONSTANT {
    assert($1 && "Invalid StringListImpl argument!");
    $1->Append(GET_TOKEN(0)->GetString());
  }
  | StringListImpl TOK_FP_CONSTANT {
    assert($1 && "Invalid StringListImpl argument!");
    $1->Append(GET_TOKEN(0)->GetString());
  }
  | StringListImpl '(' {
    assert($1 && "Invalid StringListImpl argument!");
    $1->Append(GET_TOKEN(0)->GetString());
  }
  | StringListImpl ')' {
    assert($1 && "Invalid StringListImpl argument!");
    $1->Append(GET_TOKEN(0)->GetString());
  }
  | StringListImpl '[' {
    assert($1 && "Invalid StringListImpl argument!");
    $1->Append(GET_TOKEN(0)->GetString());
  }
  | StringListImpl ']' {
    assert($1 && "Invalid StringListImpl argument!");
    $1->Append(GET_TOKEN(0)->GetString());
  }
  | StringListImpl '{' {
    assert($1 && "Invalid StringListImpl argument!");
    $1->Append(GET_TOKEN(0)->GetString());
  }
  | StringListImpl '}' {
    assert($1 && "Invalid StringListImpl argument!");
    $1->Append(GET_TOKEN(0)->GetString());
  }
  | StringListImpl ',' {
    assert($1 && "Invalid StringListImpl argument!");
    $1->Append(GET_TOKEN(0)->GetString());
  }
  | StringListImpl ':' {
    assert($1 && "Invalid StringListImpl argument!");
    $1->Append(GET_TOKEN(0)->GetString());
  }
  ;

GateQubitParamList
  : GateQubitParamListImpl Identifier {
    assert($1 && "Invalid GateQubitParamListImpl!");
    ASTIdentifierNode* Id = $2;
    assert(Id && "Invalid ASTIdentifierNode argument!");

    Id->SetPolymorphicType(Id->GetSymbolType());
    Id->SetSymbolType(ASTTypeGateQubitParam);
    Id->SetBits(1U);
    Id->SetLocalScope();
    $1->Append(Id);
  }
  ;

GateQubitParamListImpl
  : %empty {
    $$ = ASTGateQubitParamBuilder::Instance().NewList();
  }
  | GateQubitParamListImpl Identifier ',' {
    assert($1 && "Invalid GateQubitParamListImpl!");
    ASTIdentifierNode* Id = $2;
    assert(Id && "Invalid ASTIdentifierNode argument!");

    Id->SetPolymorphicType(Id->GetSymbolType());
    Id->SetSymbolType(ASTTypeGateQubitParam);
    Id->SetBits(1U);
    Id->SetLocalScope();
    $1->Append(Id);
  }
  | GateQubitParamListImpl Identifier {
    assert($1 && "Invalid GateQubitParamListImpl!");
    ASTIdentifierNode* Id = $2;
    assert(Id && "Invalid ASTIdentifierNode argument!");

    Id->SetPolymorphicType(Id->GetSymbolType());
    Id->SetSymbolType(ASTTypeGateQubitParam);
    Id->SetBits(1U);
    Id->SetLocalScope();
    $1->Append($2);
  }
  ;

AnyList
  : AnyListImpl Identifier {
    $$ = ASTAnyTypeBuilder::Instance().List();
    ASTIdentifierNode* Id = $2;
    assert(Id && "Invalid ASTIdentifierNode argument!");

    bool VB = Id->GetBits() != static_cast<unsigned>(~0x0);
    if (Id->IsRValue() && Id->IsIndexed() && VB) {
      ASTIdentifierRefNode* RId = dynamic_cast<ASTIdentifierRefNode*>(Id);
      assert(RId && "Could not dynamic_cast to a valid ASTIdentifierRefNode!");

      ASTAnyType ATI(RId, ASTTypeIdentifierRef);
      ASTAnyTypeBuilder::Instance().Append(ATI);
    } else {
      ASTAnyType ATI(Id, ASTTypeIdentifier);
      ASTAnyTypeBuilder::Instance().Append(ATI);
    }
  }
  ;

AnyListImpl
  : %empty {
    $$ = ASTAnyTypeBuilder::Instance().NewList();
  }
  | AnyListImpl Identifier ','  {
    $$ = ASTAnyTypeBuilder::Instance().List();
    ASTIdentifierNode* Id = $2;
    assert(Id && "Invalid ASTIdentifierNode argument!");

    bool VB = Id->GetBits() != static_cast<unsigned>(~0x0);
    if (Id->IsRValue() && Id->IsIndexed() && VB) {
      ASTIdentifierRefNode* RId = dynamic_cast<ASTIdentifierRefNode*>(Id);
      assert(RId && "Could not dynamic_cast to a valid ASTIdentifierRefNode!");

      ASTAnyType ATI(RId, ASTTypeIdentifierRef);
      ASTAnyTypeBuilder::Instance().Append(ATI);
    } else {
      ASTAnyType ATI(Id, ASTTypeIdentifier);
      ASTAnyTypeBuilder::Instance().Append(ATI);
    }
  }
  | AnyListImpl LineDirective {
    // Ignored. Only used by the Preprocessor and DIAGLineCounter.
  }
  | AnyListImpl FileDirective {
    // Ignored. Only used by the Preprocessor and DIAGLineCounter.
  }
  ;

FunctionCallArgExprList
  : %empty {
    $$ = new ASTFunctionCallArgumentList();
    assert($$ && "Could not create a valid ASTFunctionCallArgumentList!");
  }
  | FunctionCallArgExprListImpl Expr {
    assert($1 && "FunctionCallArgExprListImpl argument!");
    assert($$ && "Invalid ASTFunctionCallArgumentList!");
    ASTExpressionNode* E = $2;
    assert(E && "Invalid ASTExpressionNode argument!");
    $1->Append(E);
  }
  ;

FunctionCallArgExprListImpl
  : %empty {
    $$ = new ASTFunctionCallArgumentList();
    assert($$ && "Could not create a valid ASTFunctionCallArgumentList!");
  }
  | FunctionCallArgExprListImpl Expr ',' {
    assert($1 && "FunctionCallArgExprListImpl argument!");
    assert($$ && "Invalid ASTFunctionCallArgumentList!");
    ASTExpressionNode* E = $2;
    assert(E && "Invalid ASTExpressionNode argument!");
    $1->Append(E);
  }
  ;

FunctionCallArg
  : Identifier '(' FunctionCallArgExprList ')' {
    ASTIdentifierNode* Id = $1;
    assert(Id && "Invalid ASTIdentifierNode argument!");

    ASTFunctionCallArgumentList* FAL = $3;
    assert(FAL && "Invalid ASTFunctionCallArgumentList argument!");

    $$ = new ASTFunctionCallArgumentNode(Id, FAL);
    assert($$ && "Could not create a valid ASTFunctionCallArgumentNode!");

    if (!$$->ResolveFunctionCall()) {
        std::stringstream M;
        M << "Could not resolve the function call argument to "
          << Id->GetName() << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Warning);
    }
  }
  ;

ExprList
  : %empty  {
    $$ = ASTExpressionBuilder::Instance().NewList();
    assert($$ && "Could not create a valid ASTExpressionList!");
  }
  | ExprListImpl Expr {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl BinaryOpAssign {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl '(' BinaryOpAssign ')' {
    $3->AddParens();
    $1->Append($3);
    $$ = $1;
  }
  | ExprListImpl BinaryOpSelfAssign {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl '(' BinaryOpSelfAssign ')' {
    $3->AddParens();
    $1->Append($3);
    $$ = $1;
  }
  | ExprListImpl BinaryOpPrePost {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl '(' BinaryOpPrePost ')' {
    $3->AddParens();
    $1->Append($3);
    $$ = $1;
  }
  | ExprListImpl ComplexInitializerExpr {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl FunctionCallArg {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl ImplicitDuration {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl LineDirective {
    // Ignored. Only used by the Preprocessor and DIAGLineCounter.
  }
  | ExprListImpl FileDirective {
    // Ignored. Only used by the Preprocessor and DIAGLineCounter.
  }
  ;

ExprListImpl
  : %empty {
    $$ = ASTExpressionBuilder::Instance().NewList();
  }
  | ExprListImpl Expr ','  {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl BinaryOpAssign ','  {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl '(' BinaryOpAssign ')' ','  {
    $3->AddParens();
    $1->Append($3);
    $$ = $1;
  }
  | ExprListImpl BinaryOpSelfAssign ','  {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl '(' BinaryOpSelfAssign ')' ','  {
    $3->AddParens();
    $1->Append($3);
    $$ = $1;
  }
  | ExprListImpl BinaryOpPrePost ','  {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl '(' BinaryOpPrePost ')' ','  {
    $3->AddParens();
    $1->Append($3);
    $$ = $1;
  }
  | ExprListImpl Expr ':' {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl ComplexInitializerExpr {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl ComplexInitializerExpr ',' {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl FunctionCallArg {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl FunctionCallArg ',' {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl ImplicitDuration {
    $1->Append($2);
    $$ = $1;
  }
  | ExprListImpl ImplicitDuration ',' {
    $1->Append($2);
    $$ = $1;
  }
  ;

Expr
  : Real {
    $$ = $1;
  }
  | Integer  {
    $$ = $1;
  }
  | String {
    $$ = $1;
  }
  | Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_8001(GET_TOKEN(0), $1);
  }
  | UnaryOp {
    $$ = $1;
  }
  | BinaryOp {
    $$ = $1;
  }
  | ArithPowExpr {
    $$ = $1;
  }
  | '(' Expr ')'         {
    $$ = ASTProductionFactory::Instance().ProductionRule_8000(GET_TOKEN(1), $2);
  }
  | LogicalNotExpr {
    $$ = $1;
  }
  | CastExpr {
    $$ = $1;
  }
  | '-' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(1), $2,
                                                             ASTOpTypeNegative,
                                                             false);
  }
  | '+' Identifier       {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(1), $2,
                                                             ASTOpTypePositive,
                                                             false);
  }
  | '-' '(' BinaryOpSelfAssign ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(3), $3,
                                                             ASTOpTypeNegative,
                                                             true);
  }
  | '-' '(' BinaryOpAssign ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(3), $3,
                                                             ASTOpTypeNegative,
                                                             true);
  }
  | '-' '(' BinaryOpPrePost ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(3), $3,
                                                             ASTOpTypeNegative,
                                                             true);
  }
  | '+' '(' BinaryOpSelfAssign ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(3), $3,
                                                             ASTOpTypePositive,
                                                             true);
  }
  | '+' '(' BinaryOpAssign ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(3), $3,
                                                             ASTOpTypeNegative,
                                                             true);
  }
  | '+' '(' BinaryOpPrePost ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(3), $3,
                                                             ASTOpTypePositive,
                                                             true);
  }
  | '-' UnaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(1), $2,
                                                             ASTOpTypeNegative,
                                                             false);
  }
  | '+' UnaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(1), $2,
                                                             ASTOpTypePositive,
                                                             false);
  }
  | '-' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(3), $3,
                                                             ASTOpTypeNegative,
                                                             true);
  }
  | '+' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(3), $3,
                                                             ASTOpTypePositive,
                                                             true);
  }
  | '-' FunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(1), $2,
                                                             ASTOpTypeNegative,
                                                             false);
  }
  | '+' FunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(1), $2,
                                                             ASTOpTypePositive,
                                                             false);
  }
  | '-' '(' FunctionCallExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(3), $3,
                                                             ASTOpTypeNegative,
                                                             true);
  }
  | '+' '(' FunctionCallExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_350(GET_TOKEN(3), $3,
                                                             ASTOpTypePositive,
                                                             true);
  }
  ;

InitExpressionNode
  : '{' Real '}' {
    $$ = $2;
  }
  | Real {
    $$ = $1;
  }
  | '{' Integer '}' {
    $$ = $2;
  }
  | Integer {
    $$ = $1;
  }
  | '{' String '}' {
    $$ = $2;
  }
  | String {
    $$ = $1;
  }
  | '{' MPIntegerType '}' {
    $$ = $2;
  }
  | MPIntegerType {
    $$ = $1;
  }
  | '{' MPDecimalType '}' {
    $$ = $2;
  }
  | MPDecimalType {
    $$ = $1;
  }
  | '{' MPComplexType '}' {
    $$ = $2;
  }
  | MPComplexType {
    $$ = $1;
  }
  | '{' ComplexInitializerExpr '}' {
    $$ = $2;
  }
  | ComplexInitializerExpr {
    $$ = $1;
  }
  | '{' CastExpr '}' {
    $$ = $2;
  }
  | CastExpr {
    $$ = $1;
  }
  | '{' BinaryOpExpr '}' {
    $$ = $2;
  }
  | BinaryOpExpr {
    $$ = $1;
  }
  | '{' UnaryOp '}' {
    $$ = $2;
  }
  | UnaryOp {
    $$ = $1;
  }
  ;

LogicalNotExpr
  : TOK_BANG '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_340(GET_TOKEN(3), $3);
  }
  | TOK_BANG Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_340(GET_TOKEN(1), $2);
  }
  | TOK_BANG UnaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_340(GET_TOKEN(1), $2);
  }
  | TOK_BANG BinaryOpPrePost {
    $$ = ASTProductionFactory::Instance().ProductionRule_340(GET_TOKEN(1), $2);
  }
  | TOK_BANG '(' BinaryOpPrePost ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_340(GET_TOKEN(3), $3);
  }
  | TOK_BANG '(' BinaryOpSelfAssign ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_340(GET_TOKEN(3), $3);
  }
  | TOK_BANG '(' BinaryOpAssign ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_340(GET_TOKEN(3), $3);
  }
  | TOK_BANG LogicalNotExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_340(GET_TOKEN(1), $2);
  }
  | TOK_BANG ArithPowExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_340(GET_TOKEN(1), $2);
  }
  | TOK_BANG FunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_340(GET_TOKEN(1), $2);
  }
  | TOK_BANG '(' FunctionCallExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_340(GET_TOKEN(3), $3);
  }
  ;

PopcountOpExpr
  : TOK_POPCOUNT '(' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_320(GET_TOKEN(3), $3);
  }
  | TOK_POPCOUNT '(' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_320(GET_TOKEN(3), $3);
  }
  ;

PopcountOpStmt
  : PopcountOpExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_320(GET_TOKEN(1), $1);
  }
  ;

RotateOpExpr
  : TOK_ROTL '(' Identifier ',' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5,
                                                             ASTRotationTypeLeft);
  }
  | TOK_ROTL '(' Identifier ',' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5,
                                                             ASTRotationTypeLeft);
  }
  | TOK_ROTR '(' Identifier ',' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5,
                                                             ASTRotationTypeRight);
  }
  | TOK_ROTR '(' Identifier ',' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5,
                                                             ASTRotationTypeRight);
  }
  ;

RotateOpStmt
  : RotateOpExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(1), $1);
  }
  ;

UnaryOp
  : TOK_SIN '(' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeSin);
  }
  | TOK_SIN '(' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeSin);
  }
  | TOK_SIN '(' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeSin);
  }
  | TOK_SIN '(' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeSin);
  }
  | TOK_SIN '(' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeSin);
  }
  | TOK_COS '(' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeCos);
  }
  | TOK_COS '(' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeCos);
  }
  | TOK_COS '(' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeCos);
  }
  | TOK_COS '(' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeCos);
  }
  | TOK_COS '(' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeCos);
  }
  | TOK_TAN '(' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeTan);
  }
  | TOK_TAN '(' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeTan);
  }
  | TOK_TAN '(' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeTan);
  }
  | TOK_TAN '(' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeTan);
  }
  | TOK_TAN '(' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeTan);
  }
  | TOK_ARCSIN '(' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcSin);
  }
  | TOK_ARCSIN '(' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcSin);
  }
  | TOK_ARCSIN '(' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcSin);
  }
  | TOK_ARCSIN '(' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcSin);
  }
  | TOK_ARCSIN '(' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcSin);
  }
  | TOK_ARCCOS '(' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcCos);
  }
  | TOK_ARCCOS '(' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcCos);
  }
  | TOK_ARCCOS '(' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcCos);
  }
  | TOK_ARCCOS '(' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcCos);
  }
  | TOK_ARCCOS '(' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcCos);
  }
  | TOK_ARCTAN '(' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcTan);
  }
  | TOK_ARCTAN '(' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcTan);
  }
  | TOK_ARCTAN '(' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcTan);
  }
  | TOK_ARCTAN '(' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcTan);
  }
  | TOK_ARCTAN '(' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeArcTan);
  }
  | TOK_EXP '(' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeExp);
  }
  | TOK_EXP '(' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeExp);
  }
  | TOK_EXP '(' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeExp);
  }
  | TOK_EXP '(' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeExp);
  }
  | TOK_EXP '(' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeExp);
  }
  | TOK_LN '(' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeLn);
  }
  | TOK_LN '(' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeLn);
  }
  | TOK_LN '(' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeLn);
  }
  | TOK_LN '(' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeLn);
  }
  | TOK_LN '(' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeLn);
  }
  | TOK_SQRT '(' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeSqrt);
  }
  | TOK_SQRT '(' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeSqrt);
  }
  | TOK_SQRT '(' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeSqrt);
  }
  | TOK_SQRT '(' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeSqrt);
  }
  | TOK_SQRT '(' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(3), $3,
                                                             ASTOpTypeSqrt);
  }
  | '~' Expr {
    $$ = ASTProductionFactory::Instance().ProductionRule_300(GET_TOKEN(2), $2,
                                                             ASTOpTypeBitNot);
  }
  ;

ArithPowExpr
  : TOK_POW '(' Integer ',' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Integer ',' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Real ',' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Real ',' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Identifier ',' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Identifier ',' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Integer ',' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Real ',' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Identifier ',' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Integer ',' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Real ',' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Identifier ',' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Integer ',' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Real ',' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' Identifier ',' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' BinaryOpExpr ',' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' BinaryOpExpr ',' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' BinaryOpExpr ',' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' UnaryOp ',' Integer ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' UnaryOp ',' Real ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' UnaryOp ',' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' BinaryOpExpr ',' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' UnaryOp ',' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' BinaryOpExpr ',' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  | TOK_POW '(' UnaryOp ',' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_310(GET_TOKEN(5),
                                                             $3, $5);
  }
  ;

Identifier
  : TOK_IDENTIFIER {
    $$ = ASTProductionFactory::Instance().ProductionRule_1500(GET_TOKEN(0), *$1);
  }
  | UnboundQubit {
    $$ = ASTProductionFactory::Instance().ProductionRule_1502(GET_TOKEN(0), $1);
  }
  | BoundQubit {
    $$ = ASTProductionFactory::Instance().ProductionRule_1501(GET_TOKEN(0), $1);
  }
  | ComplexCReal {
    $$ = ASTProductionFactory::Instance().ProductionRule_1503(GET_TOKEN(0), $1,
                                                              "real");
  }
  | ComplexCImag {
    $$ = ASTProductionFactory::Instance().ProductionRule_1503(GET_TOKEN(0), $1,
                                                              "imag");
  }
  | OpenPulseFramePhase {
    $$ = ASTProductionFactory::Instance().ProductionRule_1504(GET_TOKEN(0), $1);
  }
  | OpenPulseFrameFrequency {
    $$ = ASTProductionFactory::Instance().ProductionRule_1504(GET_TOKEN(0), $1);
  }
  | OpenPulseFrameTime {
    $$ = ASTProductionFactory::Instance().ProductionRule_1504(GET_TOKEN(0), $1);
  }
  | TOK_IDENTIFIER IndexedSubscriptExpr IndexedSubscriptList {
    $$ = ASTProductionFactory::Instance().ProductionRule_1505(GET_TOKEN(2),
                                                              $2, $3, *$1);
  }
  | BoundQubit IndexedSubscriptExpr IndexedSubscriptList {
    $$ = ASTProductionFactory::Instance().ProductionRule_1506(GET_TOKEN(2), $1,
                                                              $2, $3);
  }
  | UnboundQubit IndexedSubscriptExpr IndexedSubscriptList {
    $$ = ASTProductionFactory::Instance().ProductionRule_1506(GET_TOKEN(2), $1,
                                                              $2, $3);
  }
  | TOK_IDENTIFIER '[' TimeUnit ']' {
    // FIXME: IMPLEMENT.
  }
  ;

Integer
  : TOK_INTEGER_CONSTANT {
    $$ = ASTProductionFactory::Instance().ProductionRule_800(GET_TOKEN(0), *$1);
  }
  ;

IntegerList
  : IntegerListImpl {
    $$ = ASTIntegerListBuilder::Instance().List();
  }
  ;

IntegerListImpl
  : %empty {
    $$ = ASTIntegerListBuilder::Instance().NewList();
  }
  | IntegerListImpl Integer {
    assert($1 && "Invalid IntegerListImpl argument!");
    assert($2 && "Invalid Integer argument!");
    $1->Append($2);
  }
  | IntegerListImpl Integer ',' {
    assert($1 && "Invalid IntegerListImpl argument!");
    assert($2 && "Invalid Integer argument!");
    $$->SetSeparator(',');
    $1->Append($2->GetSignedValue());
  }
  | IntegerListImpl Integer ':' {
    assert($1 && "Invalid IntegerListImpl argument!");
    assert($2 && "Invalid Integer argument!");
    $$->SetSeparator(':');
    $1->Append($2->GetSignedValue());
  }
  | IntegerListImpl LineDirective {
    // Ignored. Only used by the Preprocessor and DIAGLineCounter.
  }
  | IntegerListImpl FileDirective {
    // Ignored. Only used by the Preprocessor and DIAGLineCounter.
  }
  ;

QubitConcatList
  : QubitConcatListImpl {
    $$ = $1;
  }
  ;

QubitConcatListImpl
  : %empty {
    $$ = ASTQubitConcatListBuilder::Instance().NewList();
  }
  | QubitConcatListImpl Identifier {
    assert($1 && "Invalid QubitConcatListImpl argument!");
    ASTIdentifierNode* Id = $2;
    assert(Id && "Invalid ASTIdentifierNode argument!");

    $1->Append(Id);
  }
  | QubitConcatListImpl Identifier TOK_OR_OP {
    assert($1 && "Invalid QubitConcatListImpl argument!");
    ASTIdentifierNode* Id = $2;
    assert(Id && "Invalid ASTIdentifierNode argument!");

    $1->Append(Id);
  }
  | QubitConcatListImpl Identifier TOK_INC_OP {
    assert($1 && "Invalid QubitConcatListImpl argument!");
    ASTIdentifierNode* Id = $2;
    assert(Id && "Invalid ASTIdentifierNode argument!");

    $1->Append(Id);
  }
  ;

Real
  : TOK_FP_CONSTANT {
    $$ = ASTProductionFactory::Instance().ProductionRule_801(GET_TOKEN(0), *$1);
  }
  ;

Ellipsis
  : TOK_ELLIPSIS {
    $$ = ASTProductionFactory::Instance().ProductionRule_810(GET_TOKEN(0));
  }
  ;

String
  : TOK_STRING_LITERAL {
    $$ = ASTProductionFactory::Instance().ProductionRule_811(GET_TOKEN(0),
                                                    GET_TOKEN(0)->GetString());
  }
  ;

TimeUnit
  : TOK_TIME_UNIT {
    $$ = ASTProductionFactory::Instance().ProductionRule_812(GET_TOKEN(0),
                                                    GET_TOKEN(0)->GetString());
  }
  ;

BooleanConstant
  : TOK_BOOLEAN_CONSTANT {
    $$ = ASTProductionFactory::Instance().ProductionRule_813(GET_TOKEN(0));
  }
  ;

BoundQubit
  : TOK_BOUND_QUBIT {
    $$ = ASTProductionFactory::Instance().ProductionRule_814(GET_TOKEN(0),
                                                             ASTTypeBoundQubit);
  }
  ;

IndexedBoundQubit
  : TOK_BOUND_QUBIT IndexedSubscriptExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_815(GET_TOKEN(1), $2,
                                                             ASTTypeBoundQubit);
  }
  ;

UnboundQubit
  : TOK_UNBOUND_QUBIT {
    $$ = ASTProductionFactory::Instance().ProductionRule_814(GET_TOKEN(0),
                                                             ASTTypeUnboundQubit);
  }
  ;

IndexedUnboundQubit
  : TOK_UNBOUND_QUBIT IndexedSubscriptExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_815(GET_TOKEN(1), $2,
                                                             ASTTypeUnboundQubit);
  }
  ;

ComplexCReal
  : TOK_IDENTIFIER '.' TOK_CREAL {
    $$ = ASTProductionFactory::Instance().ProductionRule_816(GET_TOKEN(2),
                                                             GET_TOKEN(0));
  }
  ;

ComplexCImag
  : TOK_IDENTIFIER '.' TOK_CIMAG {
    $$ = ASTProductionFactory::Instance().ProductionRule_816(GET_TOKEN(2),
                                                             GET_TOKEN(0));
  }
  ;

OpenPulseFramePhase
  : TOK_IDENTIFIER TOK_PHASE {
    $$ = ASTProductionFactory::Instance().ProductionRule_816(GET_TOKEN(1),
                                                             GET_TOKEN(0),
                                                             false);
  }
  ;

OpenPulseFrameFrequency
  : TOK_IDENTIFIER TOK_FREQUENCY {
    $$ = ASTProductionFactory::Instance().ProductionRule_816(GET_TOKEN(1),
                                                             GET_TOKEN(0),
                                                             false);
  }
  ;

OpenPulseFrameTime
  : TOK_IDENTIFIER TOK_TIME {
    $$ = ASTProductionFactory::Instance().ProductionRule_816(GET_TOKEN(1),
                                                             GET_TOKEN(0),
                                                             false);
  }
  ;

IntScalarType
  : TOK_INT {
    $$ = ASTProductionFactory::Instance().ProductionRule_802(GET_TOKEN(0),
                                                             ASTTypeInt);
  }
  | TOK_UINT {
    $$ = ASTProductionFactory::Instance().ProductionRule_802(GET_TOKEN(0),
                                                             ASTTypeUInt);
  }
  ;

FloatScalarType
  : TOK_FLOAT {
    $$ = ASTProductionFactory::Instance().ProductionRule_803(GET_TOKEN(0));
  }
  ;

MPIntegerType
  : TOK_INT '[' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_804(GET_TOKEN(3),
                                                             $3, ASTTypeMPInteger);
  }
  | TOK_INT '[' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_805(GET_TOKEN(3),
                                                             $3, ASTTypeMPInteger);
  }
  | TOK_INT '[' BinaryOpExpr ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_7004(GET_TOKEN(3));
  }
  | TOK_UINT '[' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_804(GET_TOKEN(3),
                                                             $3, ASTTypeMPUInteger);
  }
  | TOK_UINT '[' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_805(GET_TOKEN(3),
                                                             $3, ASTTypeMPUInteger);
  }
  | TOK_UINT '[' BinaryOpExpr ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_7004(GET_TOKEN(3));
  }
  ;

MPDecimalType
  : TOK_FLOAT '[' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_806(GET_TOKEN(3), $3);
  }
  | TOK_FLOAT '[' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_807(GET_TOKEN(3), $3);
  }
  | TOK_FLOAT '[' BinaryOpExpr ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_7005(GET_TOKEN(3));
  }
  ;

MPComplexDecl
  : TOK_COMPLEX Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_250(GET_TOKEN(1), $2);
  }
  | TOK_COMPLEX '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_251(GET_TOKEN(4),
                                                             $5, $3);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_251(GET_TOKEN(7),
                                                             $8, $5);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_252(GET_TOKEN(7),
                                                             $8, $5);
  }
  | TOK_COMPLEX '[' Integer ']' Identifier '=' ComplexInitializerExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_253(GET_TOKEN(6),
                                                             $5, $3, $7);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' Identifier '=' ComplexInitializerExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_253(GET_TOKEN(9),
                                                             $8, $5, $10);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' Identifier '=' BinaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_254(GET_TOKEN(9),
                                                             $8, $5, $10);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' Identifier '=' UnaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_255(GET_TOKEN(9),
                                                             $8, $5, $10);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' Identifier '=' ComplexInitializerExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_256(GET_TOKEN(9),
                                                             $8, $5, $10);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' Identifier '=' BinaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_257(GET_TOKEN(9),
                                                             $8, $5, $10);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' Identifier '=' UnaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_258(GET_TOKEN(9),
                                                             $8, $5, $10);

  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' Identifier '=' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_259(GET_TOKEN(9),
                                                             $8, $5, $10);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' Identifier '=' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_260(GET_TOKEN(9),
                                                             $8, $5, $10);
  }
  ;

MPComplexFunctionCallDecl
  : TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' Identifier '=' FunctionCallStmtExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_261(GET_TOKEN(9),
                                                             $8, $5, $10);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' Identifier '=' FunctionCallStmtExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_262(GET_TOKEN(9),
                                                             $8, $5, $10);
  }
  ;

MPComplexType
  : TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_808(GET_TOKEN(6), $5);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_809(GET_TOKEN(6), $5);
  }
  ;

BitType
  : TOK_BIT '[' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_817(GET_TOKEN(3), $3);
  }
  | TOK_BIT '[' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_817(GET_TOKEN(3), $3);
  }
  | TOK_BIT {
    $$ = ASTProductionFactory::Instance().ProductionRule_817(GET_TOKEN(0));
  }
  ;

AngleType
  : TOK_ANGLE {
    $$ = ASTProductionFactory::Instance().ProductionRule_818(GET_TOKEN(0));
  }
  | TOK_ANGLE '[' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_818(GET_TOKEN(3), $3);
  }
  | TOK_ANGLE '[' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_818(GET_TOKEN(3), $3);
  }
  ;

QubitType
  : TOK_QUBIT {
    $$ = ASTProductionFactory::Instance().ProductionRule_819(GET_TOKEN(0));
  }
  | TOK_QUBIT '[' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_819(GET_TOKEN(3), $3);
  }
  | TOK_QUBIT '[' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_819(GET_TOKEN(3), $3);
  }
  ;

DurationType
  : TOK_DURATION {
    $$ = ASTProductionFactory::Instance().ProductionRule_820(GET_TOKEN(0));
  }
  | TOK_DURATION '[' TimeUnit ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_820(GET_TOKEN(3), $3);
  }
  ;

ArrayType
  : TOK_ARRAY '[' TOK_BIT ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_BIT ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_BIT '[' Integer ']' ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8),
                                                             $8, $5,
                                                             ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_BIT '[' Integer ']' ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $5, $10,
                                                             ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT '[' Integer ']' ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8),
                                                             $8, $5,
                                                             ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT '[' Integer ']' ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $5, $10,
                                                             ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE '[' Integer ']' ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8),
                                                             $8, $5,
                                                             ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE '[' Integer ']' ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $5, $10,
                                                             ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeBoolArray);
  }
  | TOK_ARRAY '[' TOK_INT ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeIntArray);
  }
  | TOK_ARRAY '[' TOK_INT ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeIntArray);
  }
  | TOK_ARRAY '[' TOK_INT '[' Integer ']' ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8),
                                                             $8, $5,
                                                             ASTTypeMPIntegerArray);
  }
  | TOK_ARRAY '[' TOK_INT '[' Integer ']' ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $5, $10,
                                                             ASTTypeMPIntegerArray);
  }
  | TOK_ARRAY '[' TOK_UINT ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeIntArray, true);
  }
  | TOK_ARRAY '[' TOK_UINT ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeIntArray, true);
  }
  | TOK_ARRAY '[' TOK_UINT '[' Integer ']' ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8),
                                                             $8, $5,
                                                             ASTTypeMPIntegerArray,
                                                             true);
  }
  | TOK_ARRAY '[' TOK_UINT '[' Integer ']' ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $5, $10,
                                                             ASTTypeMPIntegerArray,
                                                             true);
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeFloatArray);
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeFloatArray);
  }
  | TOK_ARRAY '[' TOK_FLOAT '[' Integer ']' ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8),
                                                             $8, $5,
                                                             ASTTypeMPDecimalArray);
  }
  | TOK_ARRAY '[' TOK_FLOAT '[' Integer ']' ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $5, $10,
                                                             ASTTypeMPDecimalArray);
  }
  | TOK_ARRAY '[' TOK_DURATION ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_DURATION '[' TimeUnit ']' ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8), $8, $5,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_DURATION '[' TimeUnit ']' ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $10, $5,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_BIT ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_BIT ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_BIT '[' Identifier ']' ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8),
                                                             $8, $5,
                                                             ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_BIT '[' Identifier ']' ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $5, $10,
                                                             ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT '[' Identifier ']' ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8),
                                                             $8, $5,
                                                             ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT '[' Identifier ']' ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $5, $10,
                                                             ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE '[' Identifier ']' ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8),
                                                             $8, $5,
                                                             ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE '[' Identifier ']' ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $5, $10,
                                                             ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeBoolArray);
  }
  | TOK_ARRAY '[' TOK_INT ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeIntArray);
  }
  | TOK_ARRAY '[' TOK_INT ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeIntArray);
  }
  | TOK_ARRAY '[' TOK_INT '[' Identifier ']' ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8), $8, $5,
                                                             ASTTypeMPIntegerArray);
  }
  | TOK_ARRAY '[' TOK_INT '[' Identifier ']' ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $5, $10,
                                                             ASTTypeMPIntegerArray);
  }
  | TOK_ARRAY '[' TOK_UINT ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeIntArray, true);
  }
  | TOK_ARRAY '[' TOK_UINT ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeIntArray, true);
  }
  | TOK_ARRAY '[' TOK_UINT '[' Identifier ']' ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8), $8, $5,
                                                             ASTTypeMPIntegerArray,
                                                             true);
  }
  | TOK_ARRAY '[' TOK_UINT '[' Identifier ']' ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $5, $10,
                                                             ASTTypeMPIntegerArray,
                                                             true);
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeFloatArray);
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeFloatArray);
  }
  | TOK_ARRAY '[' TOK_FLOAT '[' Identifier ']' ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8), $8, $5,
                                                             ASTTypeMPDecimalArray);
  }
  | TOK_ARRAY '[' TOK_FLOAT '[' Identifier ']' ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $5, $10,
                                                             ASTTypeMPDecimalArray);
  }
  | TOK_ARRAY '[' TOK_DURATION ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_DURATION '[' TimeUnit ']' ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(8), $8, $5,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_DURATION '[' TimeUnit ']' ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(10),
                                                             $8, $10, $5,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(11),
                                                             $11, $7,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(11),
                                                             $11, $7,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(11),
                                                             $11, $7,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(11),
                                                             $11, $7,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(13),
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }

  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(13),
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Identifier ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(13),
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(13),
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Identifier ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(13),
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(13),
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Integer ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(13),
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' DurationOfDecl ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5),
                                                             $5, $3,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' DurationOfDecl ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5, $7, $3,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' DurationOfDecl ',' Integer ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5, $7, $3,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' DurationOfDecl ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5),
                                                             $5, $3,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' DurationOfDecl ',' Identifier ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5, $7, $3,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' DurationOfDecl ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5, $7, $3,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_FRAME ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                    ASTTypeOpenPulseFrameArray);
  }
  | TOK_ARRAY '[' TOK_FRAME ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulseFrameArray);
  }
  | TOK_ARRAY '[' TOK_FRAME ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                    ASTTypeOpenPulseFrameArray);
  }
  | TOK_ARRAY '[' TOK_FRAME ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulseFrameArray);
  }
  | TOK_ARRAY '[' TOK_FRAME ',' Integer ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulseFrameArray);
  }
  | TOK_ARRAY '[' TOK_FRAME ',' Identifier ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulseFrameArray);
  }
  | TOK_ARRAY '[' TOK_PORT ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                    ASTTypeOpenPulsePortArray);
  }
  | TOK_ARRAY '[' TOK_PORT ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulsePortArray);
  }
  | TOK_ARRAY '[' TOK_PORT ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                    ASTTypeOpenPulsePortArray);
  }
  | TOK_ARRAY '[' TOK_PORT ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulsePortArray);
  }
  | TOK_ARRAY '[' TOK_PORT ',' Integer ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulsePortArray);
  }
  | TOK_ARRAY '[' TOK_PORT ',' Identifier ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                    ASTProductionFactory::EVX, $7,
                                                    ASTTypeOpenPulsePortArray);
  }
  | TOK_ARRAY '[' TOK_WAVEFORM ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                    ASTTypeOpenPulseWaveformArray);
  }
  | TOK_ARRAY '[' TOK_WAVEFORM ',' Integer ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                    ASTProductionFactory::EVX, $7,
                                                    ASTTypeOpenPulseWaveformArray);
  }
  | TOK_ARRAY '[' TOK_WAVEFORM ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                    ASTTypeOpenPulseWaveformArray);
  }
  | TOK_ARRAY '[' TOK_WAVEFORM ',' Identifier ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                    ASTProductionFactory::EVX, $7,
                                                    ASTTypeOpenPulseWaveformArray);
  }
  | TOK_ARRAY '[' TOK_WAVEFORM ',' Integer ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                    ASTProductionFactory::EVX, $7,
                                                    ASTTypeOpenPulseWaveformArray);
  }
  | TOK_ARRAY '[' TOK_WAVEFORM ',' Identifier ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7), $5,
                                                    ASTProductionFactory::EVX, $7,
                                                    ASTTypeOpenPulseWaveformArray);
  }
  | TOK_ARRAY '[' TOK_GATE ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeGate);
  }
  | TOK_ARRAY '[' TOK_GATE '[' Integer ']' ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $8, ASTTypeGate);
  }
  | TOK_ARRAY '[' TOK_GATE ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeGate);
  }
  | TOK_ARRAY '[' TOK_GATE '[' Identifier ']' ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $8, ASTTypeGate);
  }
  | TOK_ARRAY '[' TOK_DEFCAL ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeDefcal);
  }
  | TOK_ARRAY '[' TOK_DEFCAL '[' Integer ']' ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $8, ASTTypeDefcal);
  }
  | TOK_ARRAY '[' TOK_DEFCAL ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeDefcal);
  }
  | TOK_ARRAY '[' TOK_DEFCAL '[' Identifier ']' ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $8, ASTTypeDefcal);
  }
  | TOK_ARRAY '[' TOK_BARRIER ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeBarrier);
  }
  | TOK_ARRAY '[' TOK_BARRIER '[' Integer ']' ',' Integer ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $8, ASTTypeBarrier);
  }
  | TOK_ARRAY '[' TOK_BARRIER ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(5), $5,
                                                             ASTTypeBarrier);
  }
  | TOK_ARRAY '[' TOK_BARRIER '[' Identifier ']' ',' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_821(GET_TOKEN(7),
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $8, ASTTypeBarrier);
  }
  ;

ParamTypeDecl
  : IntScalarType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $1, ASTTypeInt);
  }
  | TOK_CONST IntScalarType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $2, ASTTypeInt,
                                                              true);
  }
  | FloatScalarType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $1, ASTTypeFloat);
  }
  | TOK_CONST FloatScalarType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $2, ASTTypeFloat,
                                                              true);
  }
  | MPDecimalType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $1, ASTTypeMPDecimal);
  }
  | TOK_CONST MPDecimalType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $2, ASTTypeMPDecimal,
                                                              true);
  }
  | MPIntegerType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $1, ASTTypeMPInteger);
  }
  | TOK_CONST MPIntegerType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $2, ASTTypeMPInteger,
                                                              true);
  }
  | MPComplexType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $1, ASTTypeMPComplex);
  }
  | TOK_CONST MPComplexType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $2, ASTTypeMPComplex,
                                                              true);
  }
  | BitType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $1, ASTTypeBitset);
  }
  | TOK_CONST BitType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $2, ASTTypeBitset,
                                                              true);
  }
  | AngleType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $1, ASTTypeAngle);
  }
  | TOK_CONST AngleType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $2, ASTTypeAngle,
                                                              true);
  }
  | QubitType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $1, ASTTypeQubitContainer);
  }
  | TOK_CONST QubitType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $2, ASTTypeQubitContainer,
                                                              true);
  }
  | DurationType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $1, ASTTypeDuration);
  }
  | TOK_CONST DurationType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $2, ASTTypeDuration,
                                                              true);
  }
  | ArrayType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $1, $1->GetASTType());
  }
  | TOK_CONST ArrayType {
    $$ = ASTProductionFactory::Instance().ProductionRule_3800(GET_TOKEN(0),
                                                              $2, $2->GetASTType(),
                                                              true);
  }
  ;

CastExpr
  : TOK_BOOL '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(3), $3,
                                                              ASTTypeBool);
  }
  | TOK_INT '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(3), $3,
                                                              ASTTypeInt);
  }
  | TOK_UINT '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(3), $3,
                                                              ASTTypeUInt);
  }
  | TOK_FLOAT '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(3), $3,
                                                              ASTTypeFloat);
  }
  | TOK_DOUBLE '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(3), $3,
                                                              ASTTypeDouble);
  }
  | TOK_BIT '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(3), $3,
                                                              ASTTypeBitset);
  }
  | TOK_BIT '[' Integer ']' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(6), $3,
                                                              $6, ASTTypeBitset);
  }
  | TOK_BIT '[' Identifier ']' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(6), $3,
                                                              $6, ASTTypeBitset);
  }
  | TOK_ANGLE '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(3), $3,
                                                              ASTTypeAngle);
  }
  | TOK_ANGLE '[' Integer ']' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(6), $3,
                                                              $6, ASTTypeAngle);
  }
  | TOK_ANGLE '[' Identifier ']' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(6), $3,
                                                              $6, ASTTypeAngle);
  }
  | TOK_INT '[' Integer ']' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(6), $3,
                                                              $6, ASTTypeMPInteger);
  }
  | TOK_INT '[' Identifier ']' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(6), $3,
                                                              $6, ASTTypeMPInteger);
  }
  | TOK_UINT '[' Integer ']' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(6), $3,
                                                              $6, ASTTypeMPUInteger);
  }
  | TOK_UINT '[' Identifier ']' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(6), $3,
                                                              $6, ASTTypeMPUInteger);
  }
  | TOK_FLOAT '[' Integer ']' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(6), $3,
                                                              $6, ASTTypeMPDecimal);
  }
  | TOK_FLOAT '[' Identifier ']' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(6), $3,
                                                              $6, ASTTypeMPDecimal);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(9), $5,
                                                              $9, ASTTypeMPComplex);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' '(' Expr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3600(GET_TOKEN(9), $5,
                                                              $9, ASTTypeMPComplex);
  }
  ;

NamedTypeDecl
  : TOK_INT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $2,
                                                             ASTTypeInt);
  }
  | TOK_CONST TOK_INT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $3,
                                                             ASTTypeInt, true);
  }
  | TOK_INT '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeMPInteger);
  }
  | TOK_CONST TOK_INT '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeMPInteger,
                                                             true);
  }
  | TOK_INT '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeMPInteger);
  }
  | TOK_CONST TOK_INT '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeMPInteger,
                                                             true);
  }
  | TOK_UINT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $2,
                                                             ASTTypeUInt);
  }
  | TOK_CONST TOK_UINT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $3,
                                                             ASTTypeUInt, true);
  }
  | TOK_UINT '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeMPUInteger);
  }
  | TOK_CONST TOK_UINT '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeMPUInteger,
                                                             true);
  }
  | TOK_UINT '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeMPUInteger);
  }
  | TOK_CONST TOK_UINT '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeMPUInteger,
                                                             true);
  }
  | TOK_FLOAT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $2,
                                                             ASTTypeFloat);
  }
  | TOK_CONST TOK_FLOAT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $3,
                                                             ASTTypeFloat,
                                                             true);
  }
  | TOK_FLOAT '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeMPDecimal);
  }
  | TOK_CONST TOK_FLOAT '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeMPDecimal,
                                                             true);
  }
  | TOK_FLOAT '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeMPDecimal);
  }
  | TOK_CONST TOK_FLOAT '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeMPDecimal,
                                                             true);
  }
  | TOK_DOUBLE Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $2,
                                                             ASTTypeDouble);
  }
  | TOK_CONST TOK_DOUBLE Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $3,
                                                             ASTTypeDouble, true);
  }
  | TOK_DOUBLE '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeMPDecimal);
  }
  | TOK_CONST TOK_DOUBLE '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeMPDecimal,
                                                             true);
  }
  | TOK_DOUBLE '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeMPDecimal);
  }
  | TOK_CONST TOK_DOUBLE '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeMPDecimal,
                                                             true);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(7), $8,
                                                             $5, ASTTypeMPComplex);
  }
  | TOK_CONST TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(7), $9,
                                                             $6, ASTTypeMPComplex,
                                                             true);
  }
  | TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(7), $8,
                                                             $5, ASTTypeMPComplex);
  }
  | TOK_CONST TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(7), $9,
                                                             $6, ASTTypeMPComplex,
                                                             true);
  }
  | TOK_DURATION Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $2,
                                                             ASTTypeDuration);
  }
  | TOK_CONST TOK_DURATION Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $3,
                                                             ASTTypeDuration,
                                                             true);
  }
  | TOK_DURATION '[' TimeUnit ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeDuration);
  }
  | TOK_CONST TOK_DURATION '[' TimeUnit ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeDuration,
                                                             true);
  }
  | TOK_BOOL Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $2,
                                                             ASTTypeBool);
  }
  | TOK_CONST TOK_BOOL Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $3,
                                                             ASTTypeBool,
                                                             true);
  }
  | TOK_QUBIT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $2,
                                                             ASTTypeQubitContainer);
  }
  | TOK_CONST TOK_QUBIT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $3,
                                                             ASTTypeQubitContainer,
                                                             true);
  }
  | TOK_QUBIT '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeQubitContainer);
  }
  | TOK_CONST TOK_QUBIT '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeQubitContainer,
                                                             true);
  }
  | TOK_QUBIT '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeQubitContainer);
  }
  | TOK_CONST TOK_QUBIT '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeQubitContainer,
                                                             true);
  }
  | TOK_BIT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $2,
                                                             ASTTypeBitset);
  }
  | TOK_CONST TOK_BIT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $3,
                                                             ASTTypeBitset, true);
  }
  | TOK_BIT '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeBitset);
  }
  | TOK_CONST TOK_BIT '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeBitset,
                                                             true);
  }
  | TOK_BIT '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeBitset);
  }
  | TOK_CONST TOK_BIT '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeBitset,
                                                             true);
  }
  | TOK_ANGLE Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $2,
                                                             ASTTypeAngle);
  }
  | TOK_CONST TOK_ANGLE Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $3,
                                                             ASTTypeAngle, true);
  }
  | TOK_ANGLE '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeAngle);
  }
  | TOK_CONST TOK_ANGLE '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeAngle,
                                                             true);
  }
  | TOK_ANGLE '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $5,
                                                             $3, ASTTypeAngle);
  }
  | TOK_CONST TOK_ANGLE '[' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(4), $6,
                                                             $4, ASTTypeAngle,
                                                             true);
  }
  | Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(0), $1);
  }
  | Ellipsis {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(0), $1);
  }
  | ArrayExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(0),
                                                             $1->GetIdentifier(),
                                                             $1->Size(),
                                                             $1, $1->GetASTType());
  }
  | TOK_CONST ArrayExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(0),
                                                             $2->GetIdentifier(),
                                                             $2->Size(),
                                                             $2, $2->GetASTType(),
                                                             true);
  }
  | TOK_WAVEFORM Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $2,
                                                             ASTTypeOpenPulseWaveform);
  }
  | TOK_CONST TOK_WAVEFORM Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $3,
                                                             ASTTypeOpenPulseWaveform,
                                                             true);
  }
  | TOK_FRAME Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $2,
                                                             ASTTypeOpenPulseFrame);
  }
  | TOK_CONST TOK_FRAME Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $3,
                                                             ASTTypeOpenPulseFrame,
                                                             true);
  }
  | TOK_PORT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $2,
                                                             ASTTypeOpenPulsePort);
  }
  | TOK_CONST TOK_PORT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $3,
                                                             ASTTypeOpenPulsePort,
                                                             true);
  }
  | TOK_EXTERN TOK_PORT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $3,
                                                             ASTTypeOpenPulsePort);
  }
  | TOK_CONST TOK_EXTERN TOK_PORT Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_850(GET_TOKEN(1), $4,
                                                             ASTTypeOpenPulsePort,
                                                             true);
  }
  ;

ArrayExpr
  : TOK_ARRAY '[' TOK_BIT ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6),
                                                             $7, $5,
                                                             ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_BIT ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8),
                                                             $9, $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_BIT '[' Integer ']' ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9),
                                                             $10, $8, $5,
                                                             ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_BIT '[' Integer ']' ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $5, $10,
                                                             ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT '[' Integer ']' ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10,
                                                             $8, $5,
                                                             ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT '[' Integer ']' ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $5, $10,
                                                             ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE '[' Integer ']' ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10,
                                                             $8, $5,
                                                             ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE '[' Integer ']' ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $5, $10,
                                                             ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeBoolArray);
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeBoolArray);
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeBoolArray);
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Integer ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeBoolArray);
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Identifier ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeBoolArray);
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeBoolArray);
  }
  | TOK_ARRAY '[' TOK_INT ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeIntArray);
  }
  | TOK_ARRAY '[' TOK_INT ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeIntArray);
  }
  | TOK_ARRAY '[' TOK_INT ',' Integer ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeIntArray);
  }
  | TOK_ARRAY '[' TOK_INT ',' Identifier ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeIntArray);
  }
  | TOK_ARRAY '[' TOK_INT ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeIntArray);
  }
  | TOK_ARRAY '[' TOK_INT '[' Integer ']' ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10,
                                                             $8, $5,
                                                             ASTTypeMPIntegerArray);
  }
  | TOK_ARRAY '[' TOK_INT '[' Integer ']' ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $5, $10,
                                                             ASTTypeMPIntegerArray);
  }
  | TOK_ARRAY '[' TOK_UINT ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeIntArray, true);
  }
  | TOK_ARRAY '[' TOK_UINT ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeIntArray, true);
  }
  | TOK_ARRAY '[' TOK_UINT ',' Integer ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeIntArray, true);
  }
  | TOK_ARRAY '[' TOK_UINT ',' Identifier ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeIntArray, true);
  }
  | TOK_ARRAY '[' TOK_UINT ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeIntArray, true);
  }
  | TOK_ARRAY '[' TOK_UINT '[' Integer ']' ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10,
                                                             $8, $5,
                                                             ASTTypeMPIntegerArray,
                                                             true);
  }
  | TOK_ARRAY '[' TOK_UINT '[' Integer ']' ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $5, $10,
                                                             ASTTypeMPIntegerArray,
                                                             true);
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeFloatArray);
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeFloatArray);
  }
  | TOK_ARRAY '[' TOK_FLOAT '[' Integer ']' ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10,
                                                             $8, $5,
                                                             ASTTypeMPDecimalArray);
  }
  | TOK_ARRAY '[' TOK_FLOAT '[' Integer ']' ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $5, $10,
                                                             ASTTypeMPDecimalArray);
  }
  | TOK_ARRAY '[' TOK_DURATION ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_DURATION '[' TimeUnit ']' ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10,
                                                             $8, $5,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_DURATION '[' TimeUnit ']' ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $10, $5,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_BIT ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_BIT ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_BIT '[' Identifier ']' ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10,
                                                             $8, $5,
                                                             ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_BIT '[' Identifier ']' ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $5, $10,
                                                             ASTTypeCBitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT '[' Identifier ']' ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10,
                                                             $8, $5,
                                                             ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_QUBIT '[' Identifier ']' ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $5, $10,
                                                             ASTTypeQubitArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE '[' Identifier ']' ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10,
                                                             $8, $5,
                                                             ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_ANGLE '[' Identifier ']' ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $5, $10,
                                                             ASTTypeAngleArray);
  }
  | TOK_ARRAY '[' TOK_INT ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeIntArray);
  }
  | TOK_ARRAY '[' TOK_INT '[' Identifier ']' ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10,
                                                             $8, $5,
                                                             ASTTypeMPIntegerArray);
  }
  | TOK_ARRAY '[' TOK_INT '[' Identifier ']' ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $5, $10,
                                                             ASTTypeMPIntegerArray);
  }
  | TOK_ARRAY '[' TOK_UINT ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeIntArray, true);
  }
  | TOK_ARRAY '[' TOK_UINT '[' Identifier ']' ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10,
                                                             $8, $5,
                                                             ASTTypeMPIntegerArray,
                                                             true);
  }
  | TOK_ARRAY '[' TOK_UINT '[' Identifier ']' ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $5, $10,
                                                             ASTTypeMPIntegerArray,
                                                             true);
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeFloatArray);
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9, $5,
                                                             ASTProductionFactory::EVX,
                                                             $7, ASTTypeFloatArray);
  }
  | TOK_ARRAY '[' TOK_FLOAT '[' Identifier ']' ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10,
                                                             $8, $5,
                                                             ASTTypeMPDecimalArray);
  }
  | TOK_ARRAY '[' TOK_FLOAT '[' Identifier ']' ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $5, $10,
                                                             ASTTypeMPDecimalArray);
  }
  | TOK_ARRAY '[' TOK_DURATION ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_DURATION '[' TimeUnit ']' ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10,
                                                             $8, $5,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_DURATION '[' TimeUnit ']' ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(11), $12,
                                                             $8, $10, $5,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(12), $13,
                                                             $11, $7,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(12), $13,
                                                             $11, $7,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(12), $13,
                                                             $11, $7,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(12), $13,
                                                             $11, $7,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(14), $15,
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }

  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(14), $15,
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Identifier ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(14), $15,
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(14), $15,
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Identifier ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(14), $15,
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(14), $15,
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Integer ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(14), $15,
                                                             $11, $7, $13,
                                                             ASTTypeMPComplexArray);
  }
  | TOK_ARRAY '[' DurationOfDecl ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7,
                                                             $5, $3,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' DurationOfDecl ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5, $7, $3,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' DurationOfDecl ',' Integer ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5, $7, $3,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' DurationOfDecl ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7,
                                                             $5, $3,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' DurationOfDecl ',' Identifier ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5, $7, $3,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' DurationOfDecl ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9,
                                                             $5, $7, $3,
                                                             ASTTypeDurationArray);
  }
  | TOK_ARRAY '[' TOK_FRAME ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                    ASTTypeOpenPulseFrameArray);
  }
  | TOK_ARRAY '[' TOK_FRAME ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9, $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulseFrameArray);
  }
  | TOK_ARRAY '[' TOK_FRAME ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                    ASTTypeOpenPulseFrameArray);
  }
  | TOK_ARRAY '[' TOK_FRAME ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9, $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulseFrameArray);
  }
  | TOK_ARRAY '[' TOK_FRAME ',' Integer ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9, $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulseFrameArray);
  }
  | TOK_ARRAY '[' TOK_FRAME ',' Identifier ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9, $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulseFrameArray);
  }
  | TOK_ARRAY '[' TOK_PORT ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                    ASTTypeOpenPulsePortArray);
  }
  | TOK_ARRAY '[' TOK_PORT ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9, $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulsePortArray);
  }
  | TOK_ARRAY '[' TOK_PORT ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                    ASTTypeOpenPulsePortArray);
  }
  | TOK_ARRAY '[' TOK_PORT ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9, $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulsePortArray);
  }
  | TOK_ARRAY '[' TOK_PORT ',' Integer ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9, $5,
                                                    ASTProductionFactory::EVX,
                                                    $7, ASTTypeOpenPulsePortArray);
  }
  | TOK_ARRAY '[' TOK_PORT ',' Identifier ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9, $5,
                                                    ASTProductionFactory::EVX, $7,
                                                    ASTTypeOpenPulsePortArray);
  }
  | TOK_ARRAY '[' TOK_WAVEFORM ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                    ASTTypeOpenPulseWaveformArray);
  }
  | TOK_ARRAY '[' TOK_WAVEFORM ',' Integer ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9, $5,
                                                    ASTProductionFactory::EVX, $7,
                                                    ASTTypeOpenPulseWaveformArray);
  }
  | TOK_ARRAY '[' TOK_WAVEFORM ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                    ASTTypeOpenPulseWaveformArray);
  }
  | TOK_ARRAY '[' TOK_WAVEFORM ',' Identifier ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9, $5,
                                                    ASTProductionFactory::EVX, $7,
                                                    ASTTypeOpenPulseWaveformArray);
  }
  | TOK_ARRAY '[' TOK_WAVEFORM ',' Integer ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9, $5,
                                                    ASTProductionFactory::EVX, $7,
                                                    ASTTypeOpenPulseWaveformArray);
  }
  | TOK_ARRAY '[' TOK_WAVEFORM ',' Identifier ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(8), $9, $5,
                                                    ASTProductionFactory::EVX, $7,
                                                    ASTTypeOpenPulseWaveformArray);
  }
  | TOK_ARRAY '[' TOK_GATE ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeGate);
  }
  | TOK_ARRAY '[' TOK_GATE '[' Integer ']' ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10, $5,
                                                             ASTProductionFactory::EVX,
                                                             $8, ASTTypeGate);
  }
  | TOK_ARRAY '[' TOK_GATE ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeGate);
  }
  | TOK_ARRAY '[' TOK_GATE '[' Identifier ']' ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10, $5,
                                                             ASTProductionFactory::EVX,
                                                             $8, ASTTypeGate);
  }
  | TOK_ARRAY '[' TOK_DEFCAL ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeDefcal);
  }
  | TOK_ARRAY '[' TOK_DEFCAL '[' Integer ']' ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10, $5,
                                                             ASTProductionFactory::EVX,
                                                             $8, ASTTypeDefcal);
  }
  | TOK_ARRAY '[' TOK_DEFCAL ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeDefcal);
  }
  | TOK_ARRAY '[' TOK_DEFCAL '[' Identifier ']' ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10, $5,
                                                             ASTProductionFactory::EVX,
                                                             $8, ASTTypeDefcal);
  }
  | TOK_ARRAY '[' TOK_BARRIER ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeBarrier);
  }
  | TOK_ARRAY '[' TOK_BARRIER '[' Integer ']' ',' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10, $5,
                                                             ASTProductionFactory::EVX,
                                                             $8, ASTTypeBarrier);
  }
  | TOK_ARRAY '[' TOK_BARRIER ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(6), $7, $5,
                                                             ASTTypeBarrier);
  }
  | TOK_ARRAY '[' TOK_BARRIER '[' Identifier ']' ',' Identifier ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_822(GET_TOKEN(9), $10, $5,
                                                             ASTProductionFactory::EVX,
                                                             $8, ASTTypeBarrier);

  }
  ;

InitArrayExpr
  : TOK_ARRAY '[' TOK_BIT ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_BIT ',' Integer ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_BIT ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_BIT ',' Identifier ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_BIT ',' Integer ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_BIT ',' Identifier ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Integer ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Identifier ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Integer ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_QUBIT ',' Identifier ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Integer ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Identifier ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Integer ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_ANGLE ',' Identifier ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Integer ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Identifier ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Integer ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_BOOL ',' Identifier ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_INT ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_INT ',' Integer ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_INT ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_INT ',' Identifier ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_INT ',' Integer ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_INT ',' Identifier ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_UINT ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_UINT ',' Integer ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_UINT ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_UINT ',' Identifier ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_UINT ',' Integer ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_UINT ',' Identifier ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Integer ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Identifier ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Integer ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_FLOAT ',' Identifier ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_DOUBLE ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_DOUBLE ',' Integer ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_DOUBLE ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_DOUBLE ',' Identifier ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_DOUBLE ',' Integer ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_DOUBLE ',' Identifier ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_DURATION ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(10));
  }
  | TOK_ARRAY '[' TOK_DURATION ',' Integer ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_DURATION ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(12));
  }
  | TOK_ARRAY '[' TOK_DURATION ',' Identifier ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(13));
  }
  | TOK_ARRAY '[' TOK_DURATION ',' Integer ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(13));
  }
  | TOK_ARRAY '[' TOK_DURATION ',' Identifier ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(13));
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(16));
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Integer ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(18));
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(18));
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Identifier ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(18));
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Integer ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(18));
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Integer ']' ']' ',' Identifier ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(18));
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(18));
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Integer ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(18));
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(18));
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Identifier ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(18));
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Integer ',' Identifier ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(18));
  }
  | TOK_ARRAY '[' TOK_COMPLEX '[' TOK_FLOAT '[' Identifier ']' ']' ',' Identifier ',' Integer ']' Identifier '=' '{' InitializerList '}' {
    // FIXME: IMPLEMENT.
    $$ = ASTProductionFactory::Instance().ProductionRule_7006(GET_TOKEN(18));
  }
  ;

ForLoopRangeExpr
  : IntegerList Identifier '+' Integer {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4, ASTOpTypeAdd);
  }
  | IntegerList Identifier '+' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4, ASTOpTypeAdd);
  }
  | IntegerList Identifier '-' Integer {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4, ASTOpTypeSub);
  }
  | IntegerList Identifier '-' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4, ASTOpTypeSub);
  }
  | IntegerList Identifier '*' Integer {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4, ASTOpTypeMul);
  }
  | IntegerList Identifier '*' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4, ASTOpTypeMul);
  }
  | IntegerList Identifier '/' Integer {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4, ASTOpTypeDiv);
  }
  | IntegerList Identifier '/' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4, ASTOpTypeDiv);
  }
  | IntegerList Identifier '%' Integer {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4, ASTOpTypeMod);
  }
  | IntegerList Identifier '%' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4, ASTOpTypeMod);
  }
  | IntegerList Identifier TOK_LEFT_SHIFT_OP Integer {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4,
                                                             ASTOpTypeLeftShift);
  }
  | IntegerList Identifier TOK_LEFT_SHIFT_OP Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4,
                                                             ASTOpTypeLeftShift);
  }
  | IntegerList Identifier TOK_RIGHT_SHIFT_OP Integer {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4,
                                                             ASTOpTypeRightShift);
  }
  | IntegerList Identifier TOK_RIGHT_SHIFT_OP Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4,
                                                             ASTOpTypeRightShift);
  }
  | IntegerList Identifier TOK_LEFT_SHIFT_ASSIGN Integer {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4,
                                                             ASTOpTypeLeftShiftAssign);
  }
  | IntegerList Identifier TOK_LEFT_SHIFT_ASSIGN Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4,
                                                             ASTOpTypeLeftShiftAssign);
  }
  | IntegerList Identifier TOK_RIGHT_SHIFT_ASSIGN Integer {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4,
                                                             ASTOpTypeRightShiftAssign);
  }
  | IntegerList Identifier TOK_RIGHT_SHIFT_ASSIGN Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_860(GET_TOKEN(3), $2,
                                                             $1, $4,
                                                             ASTOpTypeRightShiftAssign);
  }
  ;

GPhaseExpr
  : TOK_GPHASE '(' Identifier ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_870(GET_TOKEN(3), $3);
  }
  | TOK_GPHASE '(' BinaryOpExpr ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_870(GET_TOKEN(3), $3);
  }
  | TOK_GPHASE '(' UnaryOp ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_870(GET_TOKEN(3), $3);
  }
  ;

GPhaseStmt
  : GPhaseExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3507(GET_TOKEN(1), $1);
  }
  ;

GateCtrlExpr
  : TOK_CTRL TOK_ASSOCIATION_OP GateEOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(2), $3);
  }
  | TOK_CTRL '(' Integer ')' TOK_ASSOCIATION_OP GateEOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_CTRL TOK_ASSOCIATION_OP GateCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(2), $3);
  }
  | TOK_CTRL '(' Integer ')' TOK_ASSOCIATION_OP GateCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_CTRL TOK_ASSOCIATION_OP GateNegCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(2), $3);
  }
  | TOK_CTRL '(' Integer ')' TOK_ASSOCIATION_OP GateNegCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_CTRL TOK_ASSOCIATION_OP GateGPhaseExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(2), $3);
  }
  | TOK_CTRL '(' Integer ')' TOK_ASSOCIATION_OP GateGPhaseExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_CTRL TOK_ASSOCIATION_OP GateInvExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(2), $3);
  }
  | TOK_CTRL '(' Integer ')' TOK_ASSOCIATION_OP GateInvExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_CTRL TOK_ASSOCIATION_OP GatePowExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(2), $3);
  }
  | TOK_CTRL '(' Integer ')' TOK_ASSOCIATION_OP GatePowExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(5),
                                                              $6, $3);
  }
  ;

GateCtrlStmt
  : GateCtrlExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(1),
                                                              $1, true);
  }
  ;

GateCtrlExprStmt
  : GateCtrlExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3850(GET_TOKEN(1),
                                                              $1, false);
  }
  ;

GateNegCtrlExpr
  : TOK_NEGCTRL TOK_ASSOCIATION_OP GateEOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_3851(GET_TOKEN(2), $3);
  }
  | TOK_NEGCTRL '(' Integer ')' TOK_ASSOCIATION_OP GateEOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_3851(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_NEGCTRL TOK_ASSOCIATION_OP GateNegCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3851(GET_TOKEN(2), $3);
  }
  | TOK_NEGCTRL '(' Integer ')' TOK_ASSOCIATION_OP GateNegCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3851(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_NEGCTRL TOK_ASSOCIATION_OP GateGPhaseExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3851(GET_TOKEN(2), $3);
  }
  | TOK_NEGCTRL '(' Integer ')' TOK_ASSOCIATION_OP GateGPhaseExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3851(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_NEGCTRL TOK_ASSOCIATION_OP GateInvExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3851(GET_TOKEN(2), $3);
  }
  | TOK_NEGCTRL '(' Integer ')' TOK_ASSOCIATION_OP GateInvExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3851(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_NEGCTRL TOK_ASSOCIATION_OP GatePowExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3851(GET_TOKEN(2), $3);
  }
  | TOK_NEGCTRL '(' Integer ')' TOK_ASSOCIATION_OP GatePowExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3851(GET_TOKEN(5),
                                                              $6, $3);
  }
  ;

GateNegCtrlStmt
  : GateNegCtrlExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3851(GET_TOKEN(1),
                                                              $1, true);
  }
  ;

GateNegCtrlExprStmt
  : GateNegCtrlExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3851(GET_TOKEN(1),
                                                              $1, false);
  }
  ;

GateInvExpr
  : TOK_INVERSE TOK_ASSOCIATION_OP GateEOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_3853(GET_TOKEN(2), $3);
  }
  | TOK_INVERSE TOK_ASSOCIATION_OP GateCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3853(GET_TOKEN(2), $3);
  }
  | TOK_INVERSE TOK_ASSOCIATION_OP GateNegCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3853(GET_TOKEN(2), $3);
  }
  | TOK_INVERSE TOK_ASSOCIATION_OP GateGPhaseExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3853(GET_TOKEN(2), $3);
  }
  | TOK_INVERSE TOK_ASSOCIATION_OP GateInvExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3853(GET_TOKEN(2), $3);
  }
  | TOK_INVERSE TOK_ASSOCIATION_OP GatePowExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3853(GET_TOKEN(2), $3);
  }
  ;

GateInvStmt
  : GateInvExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3853(GET_TOKEN(1),
                                                              $1, true);
  }
  ;

GateInvExprStmt
  : GateInvExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3853(GET_TOKEN(1),
                                                              $1, false);
  }
  ;

GatePowExpr
  : TOK_POW '(' Integer ')' TOK_ASSOCIATION_OP GateEOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' Identifier ')' TOK_ASSOCIATION_OP GateEOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' BinaryOpExpr ')' TOK_ASSOCIATION_OP GateEOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' UnaryOp ')' TOK_ASSOCIATION_OP GateEOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' Integer ')' TOK_ASSOCIATION_OP GateCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' BinaryOpExpr ')' TOK_ASSOCIATION_OP GateCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' UnaryOp ')' TOK_ASSOCIATION_OP GateCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' Integer ')' TOK_ASSOCIATION_OP GateNegCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' BinaryOpExpr ')' TOK_ASSOCIATION_OP GateNegCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' UnaryOp ')' TOK_ASSOCIATION_OP GateNegCtrlExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' Integer ')' TOK_ASSOCIATION_OP GateGPhaseExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' BinaryOpExpr ')' TOK_ASSOCIATION_OP GateGPhaseExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' UnaryOp ')' TOK_ASSOCIATION_OP GateGPhaseExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' Integer ')' TOK_ASSOCIATION_OP GateInvExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' BinaryOpExpr ')' TOK_ASSOCIATION_OP GateInvExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' UnaryOp ')' TOK_ASSOCIATION_OP GateInvExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' Integer ')' TOK_ASSOCIATION_OP GatePowExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' BinaryOpExpr ')' TOK_ASSOCIATION_OP GatePowExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  | TOK_POW '(' UnaryOp ')' TOK_ASSOCIATION_OP GatePowExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(5),
                                                              $6, $3);
  }
  ;

GatePowStmt
  : GatePowExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(1),
                                                              $1, true);
  }
  ;

GatePowExprStmt
  : GatePowExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3852(GET_TOKEN(1),
                                                              $1, false);
  }
  ;

GateGPhaseExpr
  : TOK_CTRL TOK_ASSOCIATION_OP GPhaseExpr IdentifierList {
    $$ = ASTProductionFactory::Instance().ProductionRule_3854(GET_TOKEN(3),
                                                              $3, $4);
  }
  ;

GateGPhaseStmt
  : GateGPhaseExpr ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3854(GET_TOKEN(1), $1);
  }
  ;

SwitchCaseStmt
  : TOK_CASE Expr ':' '{' SwitchScopedStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3110(GET_TOKEN(5), $2,
                                                              $5, true);
  }
  | TOK_CASE Expr ':' '{' SwitchScopedStmtList '}' BreakStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_3111(GET_TOKEN(6), $2,
                                                              $5, $7, true);
  }
  | TOK_CASE Expr ':' SwitchUnscopedStmtList {
    $$ = ASTProductionFactory::Instance().ProductionRule_3110(GET_TOKEN(3), $2,
                                                              $4, false);
  }
  | TOK_CASE Expr ':' SwitchUnscopedStmtList BreakStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_3111(GET_TOKEN(4), $2,
                                                              $4, $5, false);
  }
  ;

SwitchDefaultStmt
  : TOK_DEFAULT ':' '{' SwitchScopedStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_3120(GET_TOKEN(4),
                                                              $4, true);
  }
  | TOK_DEFAULT ':' '{' SwitchScopedStmtList '}' BreakStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_3121(GET_TOKEN(5),
                                                              $4, $6, true);
  }
  | TOK_DEFAULT ':' SwitchUnscopedStmtList {
    $$ = ASTProductionFactory::Instance().ProductionRule_3120(GET_TOKEN(2),
                                                              $3, false);
  }
  | TOK_DEFAULT ':' SwitchUnscopedStmtList BreakStmt {
    $$ = ASTProductionFactory::Instance().ProductionRule_3121(GET_TOKEN(3),
                                                              $3, $4, false);
  }
  ;

/* CallExpr */

FunctionCallStmtExpr
  : ParenFunctionCallExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_2700(GET_TOKEN(0), $1);
  }
  ;

FunctionCallStmt
  : FunctionCallStmtExpr ';' {
    $$ = $1;
  }
  ;

FunctionCallExpr
  : ParenFunctionCallExpr {
    $$ = $1;
  }
  ;

ParenFunctionCallExpr
  : Identifier '(' ExprList ')' {
    $$ = ASTProductionFactory::Instance().ProductionRule_2700(GET_TOKEN(3),
                                                              $1, $3);
  }
  ;

ComplexInitializerExpr
  : BinaryOp TOK_IMAGINARY {
    $$ = ASTProductionFactory::Instance().ProductionRule_880(GET_TOKEN(1), $1);
  }
  | BinaryOpSelfAssign TOK_IMAGINARY {
    $$ = ASTProductionFactory::Instance().ProductionRule_880(GET_TOKEN(1), $1);
  }
  | BinaryOpPrePost TOK_IMAGINARY {
    $$ = ASTProductionFactory::Instance().ProductionRule_880(GET_TOKEN(1), $1);
  }
  | UnaryOp TOK_IMAGINARY {
    $$ = ASTProductionFactory::Instance().ProductionRule_880(GET_TOKEN(1), $1);
  }
  ;

Delay
  : TOK_DELAY '[' Identifier ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1300(GET_TOKEN(3), $3);
  }
  | TOK_DELAY '[' TimeUnit ']' IdentifierList {
    $$ = ASTProductionFactory::Instance().ProductionRule_1300(GET_TOKEN(4),
                                                              $3, $5);
  }
  | TOK_DELAY '[' TimeUnit ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1300(GET_TOKEN(3), $3);
  }
  | TOK_DELAY '[' DurationOfDecl ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1300(GET_TOKEN(3), $3);
  }
  | TOK_DELAY '[' DurationOfDecl ']' IdentifierList {
    $$ = ASTProductionFactory::Instance().ProductionRule_1300(GET_TOKEN(4),
                                                              $3, $5);
  }
  | TOK_DELAY '[' Identifier ']' IdentifierList {
    $$ = ASTProductionFactory::Instance().ProductionRule_1300(GET_TOKEN(4),
                                                              $3, $5);
  }
  | TOK_DELAY '[' BinaryOp ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1300(GET_TOKEN(3), $3);
  }
  | TOK_DELAY '[' UnaryOp ']' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1300(GET_TOKEN(3), $3);
  }
  | TOK_DELAY '(' Identifier ')' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_1300(GET_TOKEN(4),
                                                              $3, $5);
  }
  ;

DelayStmt
  : Delay ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1300(GET_TOKEN(1), $1);
  }
  ;

Stretch
  : TOK_STRETCH Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_1350(GET_TOKEN(1), $2);
  }
  | TOK_STRETCH Identifier '=' Integer {
    $$ = ASTProductionFactory::Instance().ProductionRule_1350(GET_TOKEN(3),
                                                              $2, $4);
  }
  | TOK_STRETCH Identifier '=' Real {
    $$ = ASTProductionFactory::Instance().ProductionRule_1350(GET_TOKEN(3),
                                                              $2, $4);
  }
  | TOK_STRETCH Identifier '=' BinaryOpExpr {
    $$ = ASTProductionFactory::Instance().ProductionRule_1350(GET_TOKEN(3),
                                                              $2, $4);
  }
  | TOK_STRETCH Identifier '=' UnaryOp {
    $$ = ASTProductionFactory::Instance().ProductionRule_1350(GET_TOKEN(3),
                                                              $2, $4);
  }
  | TOK_STRETCH '[' Integer ']' Identifier {
    $$ = ASTProductionFactory::Instance().ProductionRule_1350(GET_TOKEN(4),
                                                              $5, $3);
  }
  ;

StretchStmt
  : Stretch ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1350(GET_TOKEN(1), $1);
  }
  ;

BoxStmt
  : TOK_BOX Identifier '{' BoxStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1400(GET_TOKEN(4),
                                                              $2, $4);
  }
  | TOK_BOXAS Identifier '{' BoxStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1401(GET_TOKEN(4),
                                                              $2, $4);
  }
  | TOK_BOXTO TimeUnit '{' BoxStmtList '}' {
    $$ = ASTProductionFactory::Instance().ProductionRule_1402(GET_TOKEN(4),
                                                              $2, $4);
  }
  ;

Newline
  : TOK_NEWLINE {
    DIAGLineCounter::Instance().SetLineCount(QASMScanner.lineno() + 1);
  }
  ;

IncludeStmt
  : TOK_INCLUDE TOK_STRING_LITERAL {
    // Ignored. Handled by the Preprocessor.
  }
  | TOK_INCLUDE TOK_STRING_LITERAL ';' {
    // Ignored. Handled by the Preprocessor.
  }
  ;

LineDirective
  : TOK_LINE ':' TOK_INTEGER_CONSTANT {
    $$ = ASTProductionFactory::Instance().ProductionRule_5300(GET_TOKEN(2), $3);
  }
  | TOK_LINE ':' TOK_INTEGER_CONSTANT ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_5300(GET_TOKEN(3), $3);
  }
  ;

FileDirective
  : TOK_FILE ':' TOK_STRING_LITERAL {
    $$ = ASTProductionFactory::Instance().ProductionRule_5301(GET_TOKEN(2), $3);
  }
  | TOK_FILE ':' TOK_STRING_LITERAL ';' {
    $$ = ASTProductionFactory::Instance().ProductionRule_5301(GET_TOKEN(3), $3);
  }
  ;

%%

QASM::ASTRoot* QASM::ASTParser::ParseAST(std::istream* IS) {
  Root = new QASM::ASTRoot();
  if (!Root) {
    std::stringstream M;
    M << "Could not create the AST Root Entry Point!";
    QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  QasmPreprocessor::Instance().SetTranslationUnit(IS);

  if (QasmPreprocessor::Instance().IsTU()) {
    InStream = nullptr;
    const std::string& TU = QasmPreprocessor::Instance().Resolver().GetTU();

    if (!openstream(TU.c_str())) {
      std::stringstream M;
      M << "Could not open translation unit " << TU.c_str();
      QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return nullptr;
    } else {
      DIAGLineCounter::Instance().SetTranslationUnit(
                    QasmPreprocessor::Instance().Resolver().GetNormalizedTU());
      if (yyparse() != 0) {
        std::stringstream M;
        M << "yyparse() failed!";
        QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                     DiagLevel::Error);
        return nullptr;
      }
    }
  } else {
    FilePath = "-";
    InStream = IS ? IS : &std::cin;
    QasmPreprocessor::Instance().SetTranslationUnit(InStream);
    DIAGLineCounter::Instance().SetTranslationUnit("-");
    if (yyparse() != 0) {
      std::stringstream M;
      M << "yyparse() failed!";
      QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }
  }

  InFile.close();
  return Root;
}

QASM::ASTRoot* QASM::ASTParser::ParseAST(const std::string& IS) {
  Root = new QASM::ASTRoot();
  if (!Root) {
    std::stringstream M;
    M << "Could not create the AST Root Entry Point!";
    QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  // Double-parens are required here. Horrible C++ parsing rule.
  std::istringstream ISS((IS));

  FilePath = "";
  InStream = &ISS;
  QasmPreprocessor::Instance().ClearTU();
  QasmPreprocessor::Instance().SetTranslationUnit(InStream);
  DIAGLineCounter::Instance().SetTranslationUnit("-");

  if (yyparse() != 0) {
    std::stringstream M;
    M << "yyparse() failed!";
    QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ISS.clear();
  return Root;
}

void QASM::Parser::error(const QASM::location& Loc, const std::string& Msg) {
  (void) Loc;
  QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      QASM::DIAGLineCounter::Instance().GetLocation(), Msg,
                                                       DiagLevel::Error);
}
