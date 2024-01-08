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
#include <qasm/AST/OpenPulse/ASTOpenPulsePlay.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <cassert>
#include <sstream>

namespace QASM {
namespace OpenPulse {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

void ASTOpenPulsePlayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());

  if (WN)
    M.Identifier(WN->GetMangledName());
  if (FC)
    M.TypeIdentifier(ASTTypeFunctionCall, FC->GetMangledName());
  if (FN)
    M.Identifier(FN->GetMangledName());

  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace OpenPulse
} // namespace QASM
