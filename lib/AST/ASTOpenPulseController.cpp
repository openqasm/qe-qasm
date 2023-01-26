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

#include <qasm/AST/OpenPulse/ASTOpenPulseController.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <vector>
#include <cassert>

namespace QASM {
namespace OpenPulse {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTOpenPulseController ASTOpenPulseController::OPC;

ASTOpenPulseFrameNode*
ASTOpenPulseController::GetFrameNode(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), Id->GetSymbolType());
  if (!STE) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " does not exist.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  return STE->GetValue()->GetValue<OpenPulse::ASTOpenPulseFrameNode*>();
}

ASTOpenPulseWaveformNode*
ASTOpenPulseController::GetWaveformNode(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), Id->GetSymbolType());
  if (!STE) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " does not exist.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  return STE->GetValue()->GetValue<OpenPulse::ASTOpenPulseWaveformNode*>();
}

} // namespace OpenPulse
} // namespace QASM

