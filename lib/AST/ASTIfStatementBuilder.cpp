/* -*- coding: utf-8 -*-
 *
 * Copyright 2023 IBM RESEARCH. All Rights Reserved.
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

#include <qasm/AST/ASTIfStatementBuilder.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <string>
#include <sstream>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTIfStatementBuilder ASTIfStatementBuilder::CB;
std::map<unsigned, ASTStatementList*> ASTIfStatementBuilder::IfMap;
std::map<unsigned, const ASTToken*> ASTIfStatementBuilder::IfTokenMap;
std::map<unsigned, bool> ASTIfStatementBuilder::IfBraceMap;
unsigned ASTIfStatementBuilder::ISC = 0U;
unsigned ASTIfStatementBuilder::CISC = 0U;

ASTElseIfStatementBuilder ASTElseIfStatementBuilder::CB;
std::map<unsigned, ASTStatementList*> ASTElseIfStatementBuilder::ElseIfMap;
unsigned ASTElseIfStatementBuilder::ISC = 0U;
unsigned ASTElseIfStatementBuilder::CISC = 0U;

ASTElseStatementBuilder ASTElseStatementBuilder::CB;
std::map<unsigned, ASTStatementList*> ASTElseStatementBuilder::ElseMap;
unsigned ASTElseStatementBuilder::ISC = 0U;
unsigned ASTElseStatementBuilder::CISC = 0U;

void
ASTIfStatementBuilder::Push(const ASTToken* TK, bool HasBraces) {
  if (!IfTokenMap.insert(std::make_pair(ISC, TK)).second) {
    std::stringstream M;
    M << "IfStatement token map corruption. This is non-recoverable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
  }

  if (!IfBraceMap.insert(std::make_pair(ISC, HasBraces)).second) {
    std::stringstream M;
    M << "IfStatement graph corruption. This is non-recoverable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
  }
}

void
ASTIfStatementBuilder::Pop(unsigned SISC, const ASTToken* TK, bool HasBraces) {
  std::map<unsigned, const ASTToken*>::iterator TKI = IfTokenMap.find(SISC);
  if (TKI == IfTokenMap.end()) {
    std::stringstream M;
    M << "IfStatement token map corruption. This is non-recoverable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
  }

  if ((*TKI).second != TK) {
    std::stringstream M;
    M << "IfStatement token map corruption. This is non-recoverable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
  }

  std::map<unsigned, bool>::iterator MI = IfBraceMap.find(SISC);
  if (MI == IfBraceMap.end()) {
    std::stringstream M;
    M << "IfStatement graph corruption. This is non-recoverable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
  }

  if ((*MI).second != HasBraces) {
    std::stringstream M;
    M << "IfStatement graph corruption. This is non-recoverable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
  }

  IfTokenMap.erase(TKI);
  IfBraceMap.erase(MI);
}

} // namespace QASM

