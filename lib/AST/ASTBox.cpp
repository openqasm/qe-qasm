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

#include <qasm/AST/ASTBox.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cctype>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

void ASTBoxToExpressionNode::ParseDuration(const std::string& Timing) {
  unsigned I = 0;
  const char* C = Timing.c_str();

  if (*C) {
    if (!isdigit(*C)) {
      Duration = static_cast<uint64_t>(~0x0);
      Units = LengthUnspecified;
      std::stringstream M;
      M << "Box Duration is not expressed in numeric units.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
      return;
    }

    while (isdigit(*C)) {
      ++I;
      ++C;
    }

    Duration = std::stoul(Timing.substr(0, I));
    std::string U = C;

    if (U == u8"ns")
      Units = Nanoseconds;
    else if (U == u8"us" || U == u8"μs")
      Units = Microseconds;
    else if (U == u8"ms")
      Units = Milliseconds;
    else if (U == u8"s")
      Units = Seconds;
    else if (U == u8"dt")
      Units = DT;
    else {
      Duration = static_cast<uint64_t>(~0x0);
      Units = LengthUnspecified;

      std::stringstream M;
      M << "Parse error on Box Duration Timing.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    }
  }
}

const char* ASTBoxToExpressionNode::ParseUnits() const {
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

void
ASTBoxExpressionNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void
ASTBoxAsExpressionNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void
ASTBoxToExpressionNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetIdentifier()->GetPolymorphicName());
  std::string DS = std::to_string(Duration);
  DS += ParseUnits();
  M.Type(ASTTypeTimeUnit);
  M.StringValue(DS);
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace QASM

