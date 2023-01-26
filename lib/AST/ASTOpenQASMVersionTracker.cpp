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

#include <qasm/AST/ASTOpenQASMVersionTracker.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <string>
#include <sstream>
#include <iostream>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTOpenQASMVersionTracker ASTOpenQASMVersionTracker::OQT;

namespace {

bool SCCL = false;
bool SCCR = false;

}

bool
ASTOpenQASMVersionTracker::ParseOpenQASMLine(std::string& L, std::string& OQ,
                                             std::string& OV, bool& SOQ,
                                             bool OQR) {
  if (L.empty()) {
    SOQ = false;
    return true;
  }

  std::string::size_type LP = L.find_first_not_of(u8' ');
  if (LP == std::string::npos && !SOQ) {
    if (OQR) {
      std::stringstream M;
      M << "The OPENQASM directive must be the first non-comment "
        << "line in a program.";
      QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                         DiagLevel::Error);
      return false;
    }

    OQ = "OPENQASM";
    OV = "3.0";
    SetVersion(3.0);
    return true;
  }

  if ((L[LP] == u8'/') && L[LP + 1] == u8'/') {
    SOQ = false;
    return true;
  }

  if (L.find(u8"/*") != std::string::npos && !SCCL) {
    SOQ = false;
    SCCL = true;
    return true;
  } else if (L.find(u8"*/") != std::string::npos && SCCL && !SCCR) {
    SOQ = false;
    SCCR = true;
    return true;
  } else if (SCCL && !SCCR) {
    SOQ = false;
    return true;
  }

  std::string::size_type F = L.find("OPENQASM");

  if (F == std::string::npos && !SOQ) {
    if (OQR) {
      std::stringstream M;
      M << "The OPENQASM directive must be the first non-comment "
        << "line in a program.";
      QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                         DiagLevel::Error);
      return false;
    }

    OQ = "OPENQASM";
    OV = "3.0";
    SetVersion(3.0);
    return true;
  }

  if (F != 0)
    L = L.substr(F);

  std::string SL = L;
  LP = L.find_first_not_of(u8' ');
  OQ = L.substr(LP, LP + 8);

  if (L[LP + 8] == u8' ') {
    L = L.substr(LP + 8);
    while (L[LP] == u8' ')
      ++LP;
  } else {
    std::stringstream M;
    M << "Malformed OPENQASM directive: '" << SL << "'.";
    QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Error);
    return false;
  }

  OV = L.substr(LP);

  if (OV[OV.length() - 1] != u8';') {
    std::stringstream M;
    M << "Malformed OPENQASM directive: '" << SL << "': "
      << "missing semicolon.";
    QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Error);
    return 1;
  } else if (!isdigit(OV[0])) {
    std::stringstream M;
    M << "Malformed OPENQASM directive: '" << SL << "': "
      << "version is not a number.";
    QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Error);
    return false;
  } else if (OV[0] != u8'2' && OV[0] != u8'3') {
    std::stringstream M;
    M << "Malformed OPENQASM directive: '" << SL << "': "
      << "unknown version number.";
    QASM::QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Error);
    return false;
  }

  OV = OV.substr(0, OV.length() - 1);
  SOQ = true;
  SetVersion(std::stod(OV));
  return true;
}

} // namespace QASM

