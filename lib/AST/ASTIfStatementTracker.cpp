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

#include <qasm/AST/ASTIfStatementTracker.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <sstream>

namespace QASM {

using DiagLevel = QasmDiagnosticEmitter::DiagLevel;

// If Tracker
ASTIfStatementList ASTIfStatementTracker::IL;
ASTIfStatementTracker ASTIfStatementTracker::TR;
ASTIfStatementNode* ASTIfStatementTracker::CIF = nullptr;
bool ASTIfStatementTracker::PendingElseIf = false;
bool ASTIfStatementTracker::PendingElse = false;
bool ASTIfStatementTracker::Braces = false;

// ElseIf Tracker
ASTIfStatementList ASTElseIfStatementTracker::IL;
ASTElseIfStatementTracker ASTElseIfStatementTracker::EITR;
std::deque<unsigned> ASTElseIfStatementTracker::ISCQ;
ASTIfStatementNode* ASTElseIfStatementTracker::CIF = nullptr;
const ASTElseIfStatementNode* ASTElseIfStatementTracker::CEI;
bool ASTElseIfStatementTracker::POP;
unsigned ASTElseIfStatementTracker::C;
bool ASTElseIfStatementTracker::PendingElseIf = false;
bool ASTElseIfStatementTracker::PendingElse = false;

// Else Tracker
ASTIfStatementList ASTElseStatementTracker::IL;
ASTElseStatementTracker ASTElseStatementTracker::ETR;
ASTIfStatementNode* ASTElseStatementTracker::CIF = nullptr;
std::deque<unsigned> ASTElseStatementTracker::ISCQ;
std::map<unsigned, ASTStatementList*> ASTElseStatementTracker::ESM;

void
ASTIfStatementTracker::CheckDeclarationContext() {
  const ASTDeclarationContext* CTX =
    ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(CTX && "Could not obtain a valid ASTDeclarationContext!");

  switch (CTX->GetContextType()) {
  case ASTTypeGlobal:
  case ASTTypeFunction:
  case ASTTypeForLoop:
  case ASTTypeWhileLoop:
  case ASTTypeDoWhileLoop:
  case ASTTypeForStatement:
  case ASTTypeWhileStatement:
  case ASTTypeDoWhileStatement:
  case ASTTypeCaseStatement:
  case ASTTypeDefaultStatement:
  case ASTTypeGate:
  case ASTTypeCXGate:
  case ASTTypeCCXGate:
  case ASTTypeCNotGate:
  case ASTTypeHadamardGate:
  case ASTTypeUGate:
  case ASTTypeDefcal:
  case ASTTypeKernel:
  case ASTTypeExtern:
    ASTIfStatementTracker::Instance().Clear();
    ASTElseIfStatementTracker::Instance().Clear();
    ASTElseStatementTracker::Instance().Clear();
    return;
    break;
  case ASTTypeIfStatement:
    PendingElseIf = false;
    PendingElse = false;
    Braces = false;
    return;
    break;
  case ASTTypeElseIfStatement:
  case ASTTypeElseStatement:
    return;
    break;
  default:
    break;
  }

  std::stringstream M;
  M << "An If Statement is not allowed here ("
    << PrintTypeEnum(CTX->GetContextType()) << ").";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
    DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
}

void
ASTElseIfStatementTracker::CheckDeclarationContext() {
  const ASTDeclarationContext* CTX =
    ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(CTX && "Could not obtain a valid ASTDeclarationContext!");

  switch (CTX->GetContextType()) {
  case ASTTypeGlobal:
  case ASTTypeFunction:
  case ASTTypeForLoop:
  case ASTTypeWhileLoop:
  case ASTTypeDoWhileLoop:
  case ASTTypeForStatement:
  case ASTTypeWhileStatement:
  case ASTTypeDoWhileStatement:
  case ASTTypeCaseStatement:
  case ASTTypeDefaultStatement:
  case ASTTypeGate:
  case ASTTypeCXGate:
  case ASTTypeCCXGate:
  case ASTTypeCNotGate:
  case ASTTypeHadamardGate:
  case ASTTypeUGate:
  case ASTTypeDefcal:
  case ASTTypeKernel:
  case ASTTypeExtern:
    return;
    break;
  case ASTTypeIfStatement:
  case ASTTypeElseIfStatement:
  case ASTTypeElseStatement:
    return;
    break;
  default:
    break;
  }

  std::stringstream M;
  M << "An ElseIf Statement is not allowed here ("
    << PrintTypeEnum(CTX->GetContextType()) << ").";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
    DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
}

void
ASTElseStatementTracker::CheckDeclarationContext() {
  const ASTDeclarationContext* CTX =
    ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(CTX && "Could not obtain a valid ASTDeclarationContext!");

  switch (CTX->GetContextType()) {
  case ASTTypeGlobal:
  case ASTTypeFunction:
  case ASTTypeForLoop:
  case ASTTypeWhileLoop:
  case ASTTypeDoWhileLoop:
  case ASTTypeForStatement:
  case ASTTypeWhileStatement:
  case ASTTypeDoWhileStatement:
  case ASTTypeCaseStatement:
  case ASTTypeDefaultStatement:
  case ASTTypeGate:
  case ASTTypeCXGate:
  case ASTTypeCCXGate:
  case ASTTypeCNotGate:
  case ASTTypeHadamardGate:
  case ASTTypeUGate:
  case ASTTypeDefcal:
  case ASTTypeKernel:
  case ASTTypeExtern:
    return;
    break;
  case ASTTypeIfStatement:
  case ASTTypeElseIfStatement:
  case ASTTypeElseStatement:
    return;
    break;
  default:
    break;
  }

  std::stringstream M;
  M << "An Else Statement is not allowed here ("
    << PrintTypeEnum(CTX->GetContextType()) << ").";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
    DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
}

void ASTIfStatementTracker::RemoveOutOfScope(ASTIfStatementNode* IFN) {
  assert(IFN && "Invalid ASTIfStatementNode argument!");

  const ASTDeclarationContext* CTX =
    ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(CTX && "Could not obtain a valid ASTDeclarationContext!");

  if (CTX->GetContextType() == ASTTypeGlobal)
    return;

  ASTStatementList* SL = IFN->GetOpList();
  if (SL && !SL->Empty()) {
    for (ASTStatementList::iterator LI = SL->begin(); LI != SL->end(); ++LI) {
      switch ((*LI)->GetASTType()) {
      case ASTTypeIfStatement:
        if (ASTIfStatementNode* ISN = dynamic_cast<ASTIfStatementNode*>(*LI)) {
          ASTIfStatementTracker::Instance().RemoveOutOfScope(ISN);
          ASTElseIfStatementTracker::Instance().Erase(ISN);
          ASTElseStatementTracker::Instance().Erase(ISN);
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

  ASTIfStatementTracker::Instance().Erase(IFN);
}

void ASTElseIfStatementTracker::RemoveOutOfScope(ASTElseIfStatementNode* EIN) {
  assert(EIN && "Invalid ASTElseIfStatementNode argument!");

  const ASTDeclarationContext* CTX =
    ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(CTX && "Could not obtain a valid ASTDeclarationContext!");

  if (CTX->GetContextType() == ASTTypeGlobal)
    return;

  ASTStatementList* SL = EIN->GetOpList();
  if (SL && !SL->Empty()) {
    for (ASTStatementList::iterator LI = SL->begin(); LI != SL->end(); ++LI) {
      switch ((*LI)->GetASTType()) {
      case ASTTypeIfStatement:
        if (ASTIfStatementNode* ISN = dynamic_cast<ASTIfStatementNode*>(*LI)) {
          ASTIfStatementTracker::Instance().RemoveOutOfScope(ISN);
          ASTElseIfStatementTracker::Instance().Erase(ISN);
          ASTElseStatementTracker::Instance().Erase(ISN);
        }
        break;
      case ASTTypeElseIfStatement:
        if (ASTElseIfStatementNode* EFN = dynamic_cast<ASTElseIfStatementNode*>(*LI)) {
          ASTElseIfStatementTracker::Instance().RemoveOutOfScope(EFN);
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

  ASTElseIfStatementTracker::SetCurrentElseIf(nullptr);
}

void ASTElseStatementTracker::RemoveOutOfScope(ASTElseStatementNode* ESN) {
  assert(ESN && "Invalid ASTElseStatementNode argument!");

  const ASTDeclarationContext* CTX =
    ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(CTX && "Could not obtain a valid ASTDeclarationContext!");

  if (CTX->GetContextType() == ASTTypeGlobal)
    return;

  ASTStatementList* SL = ESN->GetOpList();
  if (SL && !SL->Empty()) {
    for (ASTStatementList::iterator LI = SL->begin(); LI != SL->end(); ++LI) {
      switch ((*LI)->GetASTType()) {
      case ASTTypeIfStatement:
        if (ASTIfStatementNode* ISN = dynamic_cast<ASTIfStatementNode*>(*LI)) {
          ASTIfStatementTracker::Instance().RemoveOutOfScope(ISN);
          ASTElseIfStatementTracker::Instance().Erase(ISN);
          ASTElseStatementTracker::Instance().Erase(ISN);
        }
        break;
      case ASTTypeElseIfStatement:
        if (ASTElseIfStatementNode* EFN = dynamic_cast<ASTElseIfStatementNode*>(*LI)) {
          ASTElseIfStatementTracker::Instance().RemoveOutOfScope(EFN);
        }
        break;
      case ASTTypeElseStatement:
        if (ASTElseStatementNode* EN = dynamic_cast<ASTElseStatementNode*>(*LI)) {
          ASTElseStatementTracker::Instance().RemoveOutOfScope(EN);
        }
        break;
      default:
        break;
      }
    }
  }
}

} // namespace QASM

