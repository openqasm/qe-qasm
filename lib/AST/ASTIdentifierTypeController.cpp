/* -*- coding: utf-8 -*-
 *
 * Copyright 2022 IBM RESEARCH. All Rights Reserved.
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

#include <qasm/AST/ASTIdentifierTypeController.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseCalibration.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <iostream>
#include <sstream>
#include <cassert>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

void
ASTIdentifierTypeController::CheckIdentifier(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->GetSymbolType() == ASTTypeUndefined) {
    std::stringstream M;
    M << "An undefined Type is invalid in the current execution context.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }

  if (ASTIdentifierNode::InvalidBits(Id->GetBits())) {
    std::stringstream M;
    M << "Invalid number of bits for Identifier " << Id->GetName() << ".";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }

  if (!Id->GetSymbolTableEntry()) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " does not reference a valid "
      << "Symbol Table Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }

  switch (Id->GetSymbolType()) {
  case ASTTypeOpenPulseFrame:
  case ASTTypeOpenPulseFrameArray:
  case ASTTypeOpenPulsePlay:
  case ASTTypeOpenPulsePlayStatement:
  case ASTTypeOpenPulsePort:
  case ASTTypeOpenPulsePortArray:
  case ASTTypeOpenPulseWaveform:
    OpenPulse::ASTOpenPulseCalibrationBuilder::Instance().ValidateContext();
    break;
  default:
    break;
  }
}

void
ASTIdentifierTypeController::CheckMeasureTarget(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  switch (Id->GetSymbolType()) {
  case ASTTypeQubit:
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias:
    break;
  default: {
    std::stringstream M;
    M << "Measure target " << Id->GetName() << " is not a Qubit or a "
      << "Qubit register.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }
  }
}

void
ASTIdentifierTypeController::CheckMeasureResult(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  switch (Id->GetSymbolType()) {
  case ASTTypeBitset:
  case ASTTypeInt:
    break;
  case ASTTypeAngle: {
    std::stringstream M;
    M << "Measure result " << Id->GetName() << " is an angle implicit conversion.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Warning);
  }
    break;
  default: {
    std::stringstream M;
    M << "Measure result " << Id->GetName() << " is not a bitset nor an "
      << "integer.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }
    break;
  }
}

void
ASTIdentifierTypeController::CheckIdentifierType(const ASTIdentifierNode* Id,
                                                 ASTType Ty) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->GetSymbolType() != Ty) {
    std::stringstream M;
    M << "Identifier does not have the expected type ("
      << PrintTypeEnum(Id->GetSymbolType()) << " vs. "
      << PrintTypeEnum(Ty) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }
}

void
ASTIdentifierTypeController::CheckIdentifierType(const ASTIdentifierNode* Id,
                                                 ASTType Ty0, ASTType Ty1) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->GetSymbolType() != Ty0 && Id->GetSymbolType() != Ty1) {
    std::stringstream M;
    M << "Identifier does not have the expected type ("
      << PrintTypeEnum(Id->GetSymbolType()) << " vs. "
      << PrintTypeEnum(Ty0) << ", or " << PrintTypeEnum(Ty1)
      << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }
}

void
ASTIdentifierTypeController::CheckIdentifierType(const ASTIdentifierNode* Id,
                                                 ASTType Ty0, ASTType Ty1,
                                                 ASTType Ty2) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->GetSymbolType() != Ty0 && Id->GetSymbolType() != Ty1 &&
      Id->GetSymbolType() != Ty2) {
    std::stringstream M;
    M << "Identifier does not have the expected type ("
      << PrintTypeEnum(Id->GetSymbolType()) << " vs. "
      << PrintTypeEnum(Ty0) << ", or " << PrintTypeEnum(Ty1)
      << ", or " << PrintTypeEnum(Ty2) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }
}

void
ASTIdentifierTypeController::CheckIsCallable(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  switch (Id->GetSymbolType()) {
  case ASTTypeGate:
  case ASTTypeCNotGate:
  case ASTTypeCXGate:
  case ASTTypeCCXGate:
  case ASTTypeUGate:
  case ASTTypeHadamardGate:
  case ASTTypeFunction:
  case ASTTypeDefcal:
  case ASTTypeDefcalGroup:
  case ASTTypeKernel:
  case ASTTypeOpenPulseCalibration:
  case ASTTypeDefcalCallExpression:
  case ASTTypeFunctionCallExpression:
  case ASTTypeKernelCallExpression:
    return;
    break;
  default:
    break;
  }

  std::stringstream M;
  M << "Identifier " << Id->GetName() << " type ("
    << PrintTypeEnum(Id->GetSymbolType()) << ") is not callable.";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
    DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
}

void
ASTIdentifierTypeController::CheckIdentifierType(const ASTIdentifierNode* Id,
                                                 ASTType Ty0, ASTType Ty1,
                                                 ASTType Ty2, ASTType Ty3) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->GetSymbolType() != Ty0 && Id->GetSymbolType() != Ty1 &&
      Id->GetSymbolType() != Ty2 && Id->GetSymbolType() != Ty3) {
    std::stringstream M;
    M << "Identifier does not have the expected type ("
      << PrintTypeEnum(Id->GetSymbolType()) << " vs. "
      << PrintTypeEnum(Ty0) << ", or " << PrintTypeEnum(Ty1)
      << ", or " << PrintTypeEnum(Ty2) << ", or "
      << PrintTypeEnum(Ty3) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }
}

void
ASTIdentifierTypeController::CheckUndefinedType(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->GetSymbolType() == ASTTypeUndefined ||
      ASTIdentifierNode::InvalidBits(Id->GetBits())) {
    std::stringstream M;
    M << "Identifier '" << Id->GetName() << "' is not bound to a Type.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }
}

void
ASTIdentifierTypeController::CheckIsHardwareQubit(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->GetName()[0] != '$') {
    std::stringstream M;
    M << "Identifier must reference a hardware Qubit.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }

  if (Id->GetSymbolType() != ASTTypeQubit &&
      Id->GetSymbolType() != ASTTypeQubitContainer &&
      Id->GetSymbolType() != ASTTypeQubitContainerAlias) {
    std::stringstream M;
    M << "Identifier must reference a hardware Qubit.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }

}

void
ASTIdentifierTypeController::CheckGateQubitParamType(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  const ASTDeclarationContext* CTX =
    ASTDeclarationContextTracker::Instance().GetCurrentContext();

  if (CTX->GetContextType() != ASTTypeGate) {
    std::stringstream M;
    M << "Gate Qubit Parameters can only be declared inside a "
      << "Gate Declaration Context.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }

  if (Id->GetName()[0] == '$') {
    std::stringstream M;
    M << "Gate Qubit Parameter cannot be a Bound Qubit.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }

  if (Id->IsGlobalScope()) {
    if (Id->GetSymbolType() == ASTTypeGateQubitParam) {
      if (!ASTSymbolTable::Instance().TransferGateQubitParam(Id)) {
        std::stringstream M;
        M << "Transfer of Gate Qubit Parameter " << Id->GetName()
          << " failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      }

      return;
    } else if (Id->GetSymbolType() != ASTTypeUndefined) {
      std::stringstream M;
      M << "Symbol " << Id->GetName() << " already exists at Global Scope "
        << "with type " << PrintTypeEnum(Id->GetSymbolType()) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    }
  }

  if (Id->GetSymbolType() != ASTTypeUndefined &&
      Id->GetSymbolType() != ASTTypeGateQubitParam) {
      std::stringstream M;
      M << "Symbol " << Id->GetName() << " already exists with type "
        << PrintTypeEnum(Id->GetSymbolType()) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }

  CTX = Id->GetDeclarationContext();

  if (CTX->GetContextType() != ASTTypeGate) {
    std::stringstream M;
    M << "Gate Qubit Parameters can only be declared inside a "
      << "Gate Declaration Context.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }
}

void
ASTIdentifierTypeController::CheckGateQubitParamType(
                             const ASTIdentifierList& IL) const {
  if (!IL.Empty()) {
    for (ASTIdentifierList::const_iterator LI = IL.begin(); LI != IL.end(); ++LI)
      CheckGateQubitParamType(*LI);
  }
}

bool
ASTIdentifierTypeController::TypeScopeIsAlwaysGlobal(
                             const ASTSymbolTableEntry* STE) const {
  assert(STE && "Invalid ASTSymbolTableEntry argument!");
  return TypeScopeIsAlwaysGlobal(STE->GetValueType());
}

} // namespace QASM

