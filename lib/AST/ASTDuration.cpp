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

#include <qasm/AST/ASTDuration.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <cctype>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

namespace QASM {

using DiagLevel = QasmDiagnosticEmitter::DiagLevel;

void ASTDurationNode::ParseDuration(const std::string &Unit) {
  if (Unit.empty() || Unit == "dt" || Unit == "DT") {
    Duration = 0UL;
    Units = DT;
    return;
  }

  const char *C = Unit.c_str();

  if (!std::isdigit(*C)) {
    std::stringstream M;
    M << "Duration must be expressed in numeric units.";
    Duration = static_cast<uint64_t>(~0x0);
    Units = LengthUnspecified;
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(this), M.str(),
        DiagLevel::Error);
  }

  Duration = std::stoul(Unit);

  while (*C && std::isdigit(*C++))
    ;

  std::string_view U = --C;

  if (U == u8"ns")
    Units = Nanoseconds;
  else if (U == u8"us" || U == u8"μs")
    Units = Microseconds;
  else if (U == u8"ms")
    Units = Milliseconds;
  else if (U == u8"s")
    Units = Seconds;
  else if (U == u8"dt" || U == u8"DT")
    Units = DT;
  else {
    std::stringstream M;
    M << "Parse error on Length Duration units!";
    Duration = static_cast<uint64_t>(~0x0);
    Units = LengthUnspecified;
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(this), M.str(),
        DiagLevel::Error);
  }
}

const char *ASTDurationNode::ParseUnits() const {
  switch (Units) {
  case Nanoseconds:
    return "ns";
    break;
  case Microseconds:
    return "us";
    break;
  case Milliseconds:
    return "ms";
    break;
  case Seconds:
    return "s";
    break;
  case DT:
    return "dt";
    break;
  default:
    break;
  }

  return "dt";
}

std::string ASTDurationNode::AsString() const {
  std::stringstream R;
  if (Duration == static_cast<uint64_t>(~0x0) || Units == LengthUnspecified) {
    R << "";
    std::stringstream M;
    M << "Invalid Duration value or Duration value is expressed in "
      << "invalid units!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);

    return R.str();
  }

  R << Duration;
  R << ParseUnits();

  return R.str();
}

void ASTDurationNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());

  if (HasOperand()) {
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
        GetOperand()->GetMangledName()));
  } else if (HasFunctionCall()) {
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
        GetFunctionCall()->GetMangledName()));
  } else if (IsBinaryOp()) {
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
        GetBinaryOp()->GetMangledName()));
  } else {
    std::string DS = std::to_string(Duration);
    DS += ParseUnits();
    M.Type(ASTTypeTimeUnit);
    M.StringValue(DS);
  }

  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTDurationOfNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetIdentifier()->GetPolymorphicName());

  if (HasOpTarget()) {
    switch (QOP->GetTargetType()) {
    case ASTTypeGate:
    case ASTTypeCXGate:
    case ASTTypeCCXGate:
    case ASTTypeCNotGate:
    case ASTTypeHadamardGate:
    case ASTTypeUGate:
      assert(QOP->GetGateNode() && "GateQOpNode has no Gate Target!");
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
          QOP->GetGateNode()->GetMangledName()));
      break;
    case ASTTypeDefcal:
      assert(QOP->GetDefcalNode() && "GateQOpNode has no Defcal Target!");
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
          QOP->GetDefcalNode()->GetMangledName()));
      break;
    case ASTTypeDefcalGroup:
      assert(QOP->GetDefcalGroupNode() &&
             "GateQOpNode has no Defcal Group Target!");
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
          QOP->GetDefcalGroupNode()->GetMangledName()));
      break;
    default: {
      std::stringstream DM;
      DM << "A durtationof target operand can only be a gate or a defcal.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), DM.str(),
          DiagLevel::Error);
      return;
    } break;
    }
  } else {
    M.TypeIdentifier(GetTarget()->GetSymbolType(), GetTarget()->GetName());
    if (!Args.empty()) {
      M.EndExpression();
      for (std::vector<ASTOperandNode *>::const_iterator I = Args.begin();
           I != Args.end(); ++I) {
        if ((*I)->IsExpression()) {
          M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
              (*I)->GetExpression()->GetMangledName()));
        } else if ((*I)->GetASTType() == ASTTypeOpndTy) {
          if (const ASTOperandNode *OPN =
                  dynamic_cast<const ASTOperandNode *>(*I)) {
            if (OPN->IsIdentifier())
              M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                  OPN->GetTargetIdentifier()->GetMangledName()));
            else
              M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                  OPN->GetExpression()->GetIdentifier()->GetMangledName()));
          }
        } else if ((*I)->GetASTType() == ASTTypeOpTy) {
          if (const ASTOperatorNode *OPN =
                  dynamic_cast<const ASTOperatorNode *>(*I)) {
            if (OPN->IsIdentifier())
              M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                  OPN->GetTargetIdentifier()->GetMangledName()));
            else
              M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                  OPN->GetTargetExpression()
                      ->GetIdentifier()
                      ->GetMangledName()));
          }
        } else {
          M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
              (*I)->GetIdentifier()->GetMangledName()));
        }
      }
    }

    if (!Qubits.Empty()) {
      M.EndExpression();
      for (ASTIdentifierList::const_iterator I = Qubits.begin();
           I != Qubits.end(); ++I) {
        M.TypeIdentifier((*I)->GetSymbolType(), (*I)->GetBits(),
                         (*I)->GetName());
      }
    }
  }

  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace QASM
