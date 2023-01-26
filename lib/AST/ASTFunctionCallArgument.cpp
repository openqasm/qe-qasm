/* -*- coding: utf-8 -*-
 *
 * Copyright 2021 IBM RESEARCH. All Rights Reserved.
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

#include <qasm/AST/ASTFunctionCallArgument.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <sstream>

namespace QASM {

using DiagLevel = QasmDiagnosticEmitter::DiagLevel;

bool ASTFunctionCallArgumentNode::ResolveFunctionCall() {
  FDN = ASTFunctionDeclarationMap::Instance().Find(FId->GetName());
  if (!FDN) {
    std::stringstream M;
    M << "Function " << FId->GetName() << " does not exist.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return false;
  }

  const ASTFunctionDefinitionNode* FDEF = FDN->GetDefinition();
  if (!FDEF) {
    std::stringstream M;
    M << "Function Declaration " << FId->GetName() << " without a function "
      << "definition!?";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return false;
  }

  if (FDEF->HasEllipsis()) {
    if (FAL->Size() < FDEF->GetNumParameters() - 1) {
      std::stringstream M;
      M << "A varargs function expects at least the number "
        << "of formal parameters (excluding ellipsis).";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return false;
    }
  } else {
    if (FAL->Size() != FDEF->GetNumParameters()) {
      std::stringstream M;
      M << "The number of function arguments is inconsistent with "
        << "the number of expected function parameters.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return false;
    }
  }

  Result = FDEF->GetResult();

  if (!Result) {
    std::stringstream M;
    M << "Function Definition has an invalid return statement!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return false;
  }

  EType = Result->GetResultType();
  bool InvalidType = false;

  switch (EType) {
  case ASTTypeBool:
    B = Result->GetBoolNode();
    break;
  case ASTTypeBitset:
    C = Result->GetCBitNode();
    break;
  case ASTTypeInt:
    I = Result->GetIntNode();
    break;
  case ASTTypeFloat:
    F = Result->GetFloatNode();
    break;
  case ASTTypeDouble:
    D = Result->GetDoubleNode();
    break;
  case ASTTypeMPInteger:
    MPI = Result->GetMPInteger();
    break;
  case ASTTypeMPDecimal:
    MPD = Result->GetMPDecimal();
    break;
  case ASTTypeAngle:
    A = Result->GetAngleNode();
    break;
  default:
    InvalidType = true;
    break;
  }

  if (InvalidType) {
    std::stringstream M;
    M << "Invalid argument type " << PrintTypeEnum(EType) << ".";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return false;
  }

  return true;
}

} // namespace QASM

