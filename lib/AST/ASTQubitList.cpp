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

#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTQubitList.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <cctype>
#include <sstream>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

bool ASTBoundQubitList::ValidateQubits() const {
  if (SV.empty())
    return true;

  unsigned X = 0U;

  for (ASTBoundQubitList::const_iterator I = SV.begin(); I != SV.end(); ++I) {
    const std::string &QS = (*I)->GetValue();
    if (QS[0] != '$') {
      std::stringstream M;
      M << "Qubit at Index " << X << " (" << QS << ") is not a Bound Qubit.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return false;
    }

    const ASTIdentifierNode *QId = nullptr;
    const ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().FindQubit(QS);

    if (!STE) {
      unsigned Bits = 1U;
      if (ASTStringUtils::Instance().IsIndexed(QS)) {
        Bits = ASTStringUtils::Instance().GetIdentifierIndex(QS);
        std::string BQS = ASTStringUtils::Instance().GetIdentifierBase(QS);
        QId = ASTBuilder::Instance().CreateASTIdentifierNode(
            BQS, Bits, ASTTypeQubitContainer);
        assert(QId && "Could not create a valid ASTIdentifierNode!");

      } else {
        QId = ASTBuilder::Instance().CreateASTIdentifierNode(
            QS, Bits, ASTTypeQubitContainer);
        assert(QId && "Could not create a valid ASTIdentifierNode!");
      }

      ASTQubitContainerNode *QCN =
          ASTBuilder::Instance().CreateASTQubitContainerNode(QId, Bits);
      assert(QCN && "Could not create a valid ASTQubitContainerNode!");

      STE = ASTSymbolTable::Instance().FindQubit(QId);
      assert(STE && "Invalid SymbolTable Entry for ASTQubitContainerNode!");
    } else {
      QId = STE->GetIdentifier();
      assert(QId && "Invalid SymbolTable Entry without an ASTIdentifierNode!");
    }

    if (!QId) {
      std::stringstream M;
      M << "Unable to validate Bound Qubit " << QS << " in the SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return false;
    }

    ++X;
  }

  return true;
}

bool ASTBoundQubitList::ToASTIdentifierList(ASTIdentifierList &IL) const {
  IL.Clear();

  for (const_iterator I = SV.begin(); I != SV.end(); ++I) {
    const ASTSymbolTableEntry *QSTE =
        ASTSymbolTable::Instance().FindQubit((*I)->GetValue());
    if (!QSTE) {
      std::stringstream M;
      M << "Unknown Qubit " << (*I)->GetValue() << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(*I), M.str(),
          DiagLevel::Error);
      return false;
    }

    IL.Append(const_cast<ASTIdentifierNode *>(QSTE->GetIdentifier()));
  }

  return true;
}

bool ASTUnboundQubitList::ValidateQubits() const {
  if (SV.empty())
    return true;

  unsigned X = 0U;

  for (ASTBoundQubitList::const_iterator I = SV.begin(); I != SV.end(); ++I) {
    const std::string &QS = (*I)->GetValue();
    if (QS[0] == '$') {
      std::stringstream M;
      M << "Unbound Qubit at Index " << X << " (" << QS << ") is invalid.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return false;
    }

    const char *QD = QS.c_str();

    while (*QD) {
      if (std::isdigit(*QD)) {
        std::stringstream M;
        M << "Unbound Qubit at Index " << X << " (" << QS << ") is invalid.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
            DiagLevel::Error);
        return false;
      }

      ++QD;
    }

    ++X;
  }

  return true;
}

} // namespace QASM
