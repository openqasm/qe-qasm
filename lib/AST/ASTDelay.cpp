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

#include <qasm/AST/ASTDelay.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <cctype>
#include <cstring>
#include <iostream>
#include <string>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

void ASTDelayNode::ParseDuration(const std::string &Time) {
  if (Time.empty())
    return;

  if (Time == u8"dt" || Time == u8"DT") {
    Units = DT;
    Duration = 0UL;
    DType = ASTTypeQubit;
    return;
  }

  unsigned I = 0;
  const char *C = Time.c_str();

  if (*C) {
    if (!std::isdigit(*C)) {
      Duration = static_cast<uint64_t>(~0x0);
      Units = LengthUnspecified;

      std::stringstream M;
      M << "Delay Duration is not expressed in numeric units!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    while (std::isdigit(*C)) {
      ++I;
      ++C;
    }

    Duration = std::stoul(Time.substr(0, I));
    std::string U = C;

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
      Duration = static_cast<uint64_t>(~0x0);
      Units = LengthUnspecified;

      std::stringstream M;
      M << "Parse error on Delay Duration Unit!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    }
  }
}

const char *ASTDelayNode::ParseUnits() const {
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

void ASTDelayNode::SetDelayType(const ASTIdentifierNode *TId) {
  assert(TId && "Invalid ASTIdentifierNode argument!");

  const std::string &S = TId->GetName();
  const char *C = S.c_str();

  if (S == "dt") {
    ParseDuration(S);
    TId->SetBits(64U);
    return;
  } else if (std::isdigit(C[0])) {
    if (TId->GetSymbolType() != ASTTypeTimeUnit) {
      std::stringstream M;
      M << "Invalid Symbol Type for Delay Target ASTIdentifierNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(TId), M.str(),
          DiagLevel::Error);
      return;
    }

    ParseDuration(S);
    TId->SetBits(64U);
    return;
  }

  ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().Lookup(
      TId, TId->GetBits(), TId->GetSymbolType());
  if (!STE) {
    DType = ASTTypeUndefined;
    std::stringstream M;
    M << "Identifier " << TId->GetName() << " has no SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return;
  }

  DType = STE->GetValueType();

  if (DType == ASTTypeStretch) {
    STR = STE->GetValue()->GetValue<ASTStretchNode *>();
    if (!STR) {
      std::stringstream M;
      M << "Failed to locate ASTStretchNode Identifier " << TId->GetName()
        << " in the SymbolTable!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(TId), M.str(),
          DiagLevel::Error);
      return;
    }

    Duration = STR->GetDuration();
    Units = STR->GetLengthUnit();
  } else if (DType == ASTTypeDuration) {
    DN = STE->GetValue()->GetValue<ASTDurationNode *>();
    if (!DN) {
      std::stringstream M;
      M << "Failed to locate ASTDurationNode Identifier " << TId->GetName()
        << " in the SymbolTable!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(TId), M.str(),
          DiagLevel::Error);
      return;
    }

    Duration = DN->GetDuration();
    Units = DN->GetLengthUnit();
  }
}

void ASTDelayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetIdentifier()->GetPolymorphicName());

  switch (DType) {
  case ASTTypeIdentifier:
    assert(Id && "Invalid ASTIdentifierNode!");
    M.Type(ASTTypeTimeUnit);
    M.NumericLiteral(Duration);

    switch (Units) {
    case DT:
      M.TypeIdentifier(ASTTypeEnum, "dt");
      break;
    case Nanoseconds:
      M.TypeIdentifier(ASTTypeEnum, "ns");
      break;
    case Microseconds:
      M.TypeIdentifier(ASTTypeEnum, "us");
      break;
    case Milliseconds:
      M.TypeIdentifier(ASTTypeEnum, "ms");
      break;
    case Seconds:
      M.TypeIdentifier(ASTTypeEnum, "s");
      break;
    default:
      M.TypeIdentifier(ASTTypeEnum, "dt");
      break;
    }
    break;
  case ASTTypeStretch:
    assert(STR && "Invalid ASTStretchNode!");
    M.TypeIdentifier(ASTTypeStretch, STR->GetName());
    break;
  case ASTTypeQubit:
    assert(Id && "Invalid ASTIdentifierNode!");

    M.Type(ASTTypeTimeUnit);
    M.NumericLiteral(Duration);

    switch (Units) {
    case DT:
      M.TypeIdentifier(ASTTypeEnum, "dt");
      break;
    case Nanoseconds:
      M.TypeIdentifier(ASTTypeEnum, "ns");
      break;
    case Microseconds:
      M.TypeIdentifier(ASTTypeEnum, "us");
      break;
    case Milliseconds:
      M.TypeIdentifier(ASTTypeEnum, "ms");
      break;
    case Seconds:
      M.TypeIdentifier(ASTTypeEnum, "s");
      break;
    default:
      M.TypeIdentifier(ASTTypeEnum, "dt");
      break;
    }
    break;
  case ASTTypeDuration:
    assert(DN && "Invalid ASTDurationNode!");
    if (DN->GetDuration()) {
      M.Type(ASTTypeTimeUnit);
      M.NumericLiteral(Duration);

      switch (DN->GetLengthUnit()) {
      case DT:
        M.TypeIdentifier(ASTTypeEnum, "dt");
        break;
      case Nanoseconds:
        M.TypeIdentifier(ASTTypeEnum, "ns");
        break;
      case Microseconds:
        M.TypeIdentifier(ASTTypeEnum, "us");
        break;
      case Milliseconds:
        M.TypeIdentifier(ASTTypeEnum, "ms");
        break;
      case Seconds:
        M.TypeIdentifier(ASTTypeEnum, "s");
        break;
      default:
        M.TypeIdentifier(ASTTypeEnum, "dt");
        break;
      }
    } else {
      M.TypeIdentifier(ASTTypeDuration, DN->GetName());
    }
    break;
  case ASTTypeDurationOf:
    assert(DON && "Invalid ASTDurationOfNode!");
    M.TypeIdentifier(ASTTypeDurationOf, DON->GetName());
    break;
  case ASTTypeBinaryOp:
    assert(BOP && "Invalid ASTBinaryOpNode!");
    M.TypeIdentifier(ASTTypeBinaryOp, BOP->GetMangledName());
    break;
  case ASTTypeUnaryOp:
    assert(UOP && "Invalid ASTUnaryOpNode!");
    M.TypeIdentifier(ASTTypeUnaryOp, UOP->GetMangledName());
    break;
  default:
    break;
  }

  M.EndExpression();

  if (!IL.Empty()) {
    unsigned IX = 0U;
    for (ASTIdentifierList::const_iterator LI = IL.begin(); LI != IL.end();
         ++LI) {
      const std::string &QN = (*LI)->GetName();
      if (ASTStringUtils::Instance().IsIndexedQubit(QN)) {
        std::stringstream QS;
        QS << ASTStringUtils::Instance().GetBaseQubitName(QN) << '['
           << ASTStringUtils::Instance().GetQubitIndex(QN) << ']';
        M.QubitTarget(IX++, QS.str());
      } else {
        M.QubitTarget(IX++, (*LI)->GetName());
      }
    }
  }

  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace QASM
