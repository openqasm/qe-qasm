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

#include <qasm/AST/ASTIfConditionalsGraphController.h>
#include <qasm/AST/ASTIfStatementTracker.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <cassert>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTIfConditionalsGraphController ASTIfConditionalsGraphController::GC;

void
ASTIfConditionalsGraphController::ResolveIfChain(
                                  std::vector<ASTIfStatementNode*>& PV) const {
  if (!PV.empty()) {
    if (PV.size() == 1) {
      PV[0]->SetParentIf(nullptr);
      PV[0]->SetStackFrame(0);
    } else {
      uint32_t SFC = static_cast<uint32_t>(PV.size());
      for (std::vector<ASTIfStatementNode*>::reverse_iterator RI = PV.rbegin();
           RI != PV.rend(); ++RI) {
        if ((RI + 1) == PV.rend()) {
          (*RI)->SetParentIf(nullptr);
          (*RI)->SetStackFrame(--SFC);
          assert((static_cast<int32_t>(SFC) >= 0) &&
                 "IfConditional StackFrame overflow!");
        } else {
          (*RI)->SetParentIf(*(RI + 1));
          (*RI)->SetStackFrame(--SFC);
          assert((static_cast<int32_t>(SFC) >= 0) &&
                 "IfConditional StackFrame overflow!");
        }
      }
    }
  }
}

void
ASTIfConditionalsGraphController::ResolveIfEdges(ASTStatementList& SL,
                                  std::vector<ASTIfStatementNode*>& PV) const {
  for (ASTStatementList::iterator LI = SL.begin(); LI != SL.end(); ++LI) {
    if ((*LI)->GetASTType() == ASTTypeIfStatement) {
      ASTIfStatementNode* IFS = dynamic_cast<ASTIfStatementNode*>(*LI);
      assert(IFS && "Could not dynamic_cast to an ASTIfStatementNode!");

      PV.push_back(IFS);
      ASTStatementList* ISL = const_cast<ASTStatementList*>(IFS->GetOpList());
      assert(ISL && "Invalid ASTStatementList from ASTIfStatementNode!");

      if (SL.Size() > 2) {
        std::vector<ASTIfStatementNode*> IPV;
        IPV.push_back(IFS);
        ResolveIfEdges(*ISL, IPV);
      }
    }
  }
}

void
ASTIfConditionalsGraphController::RemoveOutOfScope(ASTStatementList& SL,
                                  const ASTDeclarationContext* DCX) const {
  assert(DCX && "Invalid ASTDeclarationContext argument!");

  if (DCX->GetContextType() == ASTTypeGlobal)
    return;

  if (!SL.Empty()) {
    for (ASTStatementList::iterator LI = SL.begin(); LI != SL.end(); ++LI) {
      switch ((*LI)->GetASTType()) {
      case ASTTypeIfStatement:
        if (ASTIfStatementNode* IFN = dynamic_cast<ASTIfStatementNode*>(*LI)) {
          ASTIfStatementTracker::Instance().RemoveOutOfScope(IFN);
          ASTElseIfStatementTracker::Instance().Erase(IFN);
          ASTElseStatementTracker::Instance().Erase(IFN);
        }
        break;
      case ASTTypeElseIfStatement:
        if (ASTElseIfStatementNode* EIN = dynamic_cast<ASTElseIfStatementNode*>(*LI)) {
          ASTElseIfStatementTracker::Instance().RemoveOutOfScope(EIN);
        }
        break;
      case ASTTypeElseStatement:
        if (ASTElseStatementNode* ESN = dynamic_cast<ASTElseStatementNode*>(*LI)) {
          ASTElseStatementTracker::Instance().RemoveOutOfScope(ESN);
        }
        break;
      default:
        break;
      }
    }
  }
}

} // namespace QASM

