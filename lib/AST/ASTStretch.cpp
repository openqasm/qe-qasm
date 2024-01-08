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

#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTStretch.h>
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

void ASTStretchNode::ParseDuration(const std::string &Unit) {
  unsigned II = 0;
  const char *C = Unit.c_str();

  if (*C) {
    if (!std::isdigit(*C)) {
      std::stringstream M;
      M << "Length Duration is not expressed in numeric units!";
      Duration = static_cast<uint64_t>(~0x0);
      Units = LengthUnspecified;
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    while (isdigit(*C)) {
      ++II;
      ++C;
    }

    Duration = std::stoul(Unit.substr(0, II));
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
      std::stringstream M;
      M << "Parse error on Length Duration units!";
      Duration = static_cast<uint64_t>(~0x0);
      Units = LengthUnspecified;
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    }
  }
}

const char *ASTStretchNode::ParseUnits() const {
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

std::string ASTStretchNode::AsString() const {
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

void ASTStretchNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  std::string DS = std::to_string(Duration);
  DS += ParseUnits();
  M.Type(ASTTypeTimeUnit);
  M.StringValue(DS);

  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace QASM
