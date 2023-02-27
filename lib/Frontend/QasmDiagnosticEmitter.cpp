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

#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <iostream>
#include <cstdlib>

namespace QASM {

QasmDiagnosticEmitter QasmDiagnosticEmitter::QDE;
QasmDiagnosticEmitter::QasmDiagnosticHandler QasmDiagnosticEmitter::Handler =
  QasmDiagnosticEmitter::DefaultHandler;
unsigned QasmDiagnosticEmitter::ErrCounter = 0;
unsigned QasmDiagnosticEmitter::WarnCounter = 0;
unsigned QasmDiagnosticEmitter::MaxErrors = 1;
unsigned QasmDiagnosticEmitter::ICECounter = 0;

void
QasmDiagnosticEmitter::DefaultHandler(const std::string& File,
                                      const ASTLocation& Loc,
                                      const std::string& Msg,
                                      DiagLevel DL) {
  static const char* DiagPrefix[] = { "Status: ", "Info: ",
                                      "Warning: ", "Error: ",
                                      "===> ICE [Internal Compiler Error]: " };

  std::cerr << DiagPrefix[DL] << "File: " << File
            << ", Line: " << Loc.LineNo << ", Col: " << Loc.ColNo
            << ":\n" << "    " << Msg << std::endl;

  if (ICECounter) {
    std::cerr << "Compilation terminated abnormally.\n" << std::endl;
    exit(1);
  }

  if (ErrCounter == MaxErrors) {
    std::cerr << "Maximum number of errors (" << MaxErrors
              << ") has been reached. Exiting now.\n" << std::endl;
    exit(1);
  }
}

void
QasmDiagnosticEmitter::EmitDiagnostic(const ASTLocation& Loc,
                                      const std::string& Msg,
                                      DiagLevel DL) {
  switch (DL) {
  case DiagLevel::Error:
    ++ErrCounter;
    break;
  case DiagLevel::Warning:
    ++WarnCounter;
    break;
  case DiagLevel::ICE:
    ++ICECounter;
    break;
  default:
    break;
  }

  if (ErrCounter > MaxErrors)
    return;

  if (ICECounter > 1)
    return;

  Handler(DIAGLineCounter::Instance().GetTranslationUnit(), Loc, Msg, DL);
}

} // namespace QASM
