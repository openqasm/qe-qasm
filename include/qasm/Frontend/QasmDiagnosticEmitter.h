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

#ifndef __QASM_DIAGNOSTIC_EMITTER_H
#define __QASM_DIAGNOSTIC_EMITTER_H

#include <qasm/AST/ASTBase.h>

#include <string>

namespace QASM {

class QasmDiagnosticEmitter {
public:
  enum DiagLevel : unsigned { Status = 0, Info, Warning, Error, ICE };

  typedef void (*QasmDiagnosticHandler)(const std::string &File,
                                        ASTLocation Loc, const std::string &Msg,
                                        DiagLevel DL);

private:
  static QasmDiagnosticEmitter QDE;
  static QasmDiagnosticHandler Handler;
  static unsigned ErrCounter;
  static unsigned WarnCounter;
  static unsigned MaxErrors;
  static unsigned ICECounter;

private:
  QasmDiagnosticEmitter() = default;

public:
  ~QasmDiagnosticEmitter() = default;
  static void DefaultHandler(const std::string &File, ASTLocation Loc,
                             const std::string &Msg, DiagLevel DL);

  static QasmDiagnosticEmitter &Instance() { return QDE; }

  static void SetMaxErrors(unsigned M) { MaxErrors = M; }

  static void SetHandler(QasmDiagnosticHandler handler) { Handler = handler; }

  bool HasErrors() const { return ErrCounter > 0; }

  bool HasWarnings() const { return WarnCounter > 0; }

  unsigned GetNumErrors() const { return ErrCounter; }

  unsigned GetNumWarnings() const { return WarnCounter; }

  bool CanEmit() const { return ICECounter < 1 && ErrCounter < MaxErrors; }

  void EmitDiagnostic(ASTLocation Loc, const std::string &Msg,
                      DiagLevel DL = DiagLevel::Status);
};

} // namespace QASM

#endif // __QASM_DIAGNOSTIC_EMITTER_H
