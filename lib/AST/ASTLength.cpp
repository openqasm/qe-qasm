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

#include <qasm/AST/ASTLength.h>
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

void ASTLengthNode::ParseDuration(const std::string &Unit) {
  unsigned I = 0;
  const char *C = Unit.c_str();

  if (*C) {
    if (!isdigit(*C)) {
      std::stringstream M;
      M << "Length Duration is not expressed in numeric units!";
      Duration = static_cast<uint64_t>(~0x0);
      Units = LengthUnspecified;
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    while (isdigit(*C)) {
      ++I;
      ++C;
    }

    Duration = std::stoul(Unit.substr(0, I));
    std::string U = C;

    if (U == "ns")
      Units = Nanoseconds;
    else if (U == "us" || U == u8"μs")
      Units = Microseconds;
    else if (U == "ms")
      Units = Milliseconds;
    else if (U == "s")
      Units = Seconds;
    else if (U == "dt")
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

std::string ASTLengthNode::AsString() const {
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

  switch (Units) {
  case Nanoseconds:
    R << "ns";
    break;
  case Microseconds:
    R << "us";
    break;
  case Milliseconds:
    R << "ms";
    break;
  case Seconds:
    R << "s";
    break;
  case DT:
    R << "dt";
    break;
  default:
    R << "dt";
    break;
  }

  return R.str();
}

void ASTLengthNode::Mangle() {
  std::stringstream M;
  M << "Type " << PrintTypeEnum(GetASTType()) << " cannot be "
    << "used or mangled.";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
}

void ASTLengthOfNode::Mangle() {
  std::stringstream M;
  M << "Type " << PrintTypeEnum(GetASTType()) << " cannot be "
    << "used or mangled.";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
}

} // namespace QASM
