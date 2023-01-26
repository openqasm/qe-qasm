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

#include <qasm/AST/ASTScopeController.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTDeclarationBuilder.h>
#include <qasm/AST/ASTTypeSystemBuilder.h>
#include <qasm/AST/ASTExpressionValidator.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <iostream>
#include <sstream>
#include <cassert>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTScopeController ASTScopeController::SC;

static const std::set<std::string> CS = {
  { u8".creal" },
  { u8".cimag" },
};

static const std::set<std::string> FS = {
  { u8".freq" },
  { u8".frequency" },
  { u8".phase" },
  { u8".time" },
};

void
ASTScopeController::SetLocalScope(ASTDeclarationList* DL) const {
  assert(DL && "Invalid ASTDeclarationList argument!");

  if (DL && !DL->Empty()) {
    for (ASTDeclarationList::iterator I = DL->begin(); I != DL->end(); ++I) {
      if (ASTIdentifierNode* DId =
          const_cast<ASTIdentifierNode*>((*I)->GetIdentifier())) {
        if ((*I)->IsExpression() && CanHaveLocalScope(DId->GetSymbolType())) {
          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(DId, DId->GetBits(),
                                                DId->GetSymbolType())) {
            DId->SetLocalScope();
            STE->SetLocalScope();
          }
        }
      }
    }
  }
}

void
ASTScopeController::SetDeclarationContext(ASTDeclarationList* DL,
                                          const ASTDeclarationContext* CTX) const {
  assert(DL && "Invalid ASTDeclarationList argument!");
  assert(CTX && "Invalid ASTDeclarationContext argument!");

  bool LocalScope =
    !ASTDeclarationContextTracker::Instance().IsGlobalContext(CTX) &&
    !ASTDeclarationContextTracker::Instance().IsCalibrationContext(CTX);

  if (DL && !DL->Empty()) {
    for (ASTDeclarationList::iterator I = DL->begin(); I != DL->end(); ++I) {
      (*I)->SetDeclarationContext(CTX);

      if (ASTIdentifierNode* DId =
          const_cast<ASTIdentifierNode*>((*I)->GetIdentifier())) {
        if ((*I)->IsExpression()) {
          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(DId, DId->GetBits(),
                                                DId->GetSymbolType())) {
            if (LocalScope && CanHaveLocalScope(DId->GetSymbolType())) {
              DId->SetDeclarationContext(CTX);
              STE->SetContext(CTX);
              DId->SetLocalScope();
              STE->SetLocalScope();
            } else if (!LocalScope) {
              DId->SetDeclarationContext(CTX);
              STE->SetContext(CTX);
            }
          }
        }
      }
    }
  }
}

void
ASTScopeController::SetLocalScope(ASTStatementList* SL) const {
  assert(SL && "Invalid ASTStatementList argument!");

  if (SL && !SL->Empty()) {
    for (ASTStatementList::iterator I = SL->begin();
         I != SL->end(); ++I) {
      ASTStatementNode* SN = dynamic_cast<ASTStatementNode*>(*I);
      if (SN && SN->IsExpression()) {
        ASTIdentifierNode* SId = const_cast<ASTIdentifierNode*>(SN->GetIdentifier());
        if (SId && CanHaveLocalScope(SId->GetSymbolType())) {
          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(SId, SId->GetBits(),
                                                SId->GetSymbolType())) {
            SId->SetLocalScope();
            STE->SetLocalScope();
          }
        }
      }
    }
  }
}

void
ASTScopeController::SetDeclarationContext(ASTStatementList* SL,
                                          const ASTDeclarationContext* CTX) const {
  assert(SL && "Invalid ASTStatementList argument!");
  assert(CTX && "Invalid ASTDeclarationContext argument!");

  bool LocalScope =
    !ASTDeclarationContextTracker::Instance().IsGlobalContext(CTX) &&
    !ASTDeclarationContextTracker::Instance().IsCalibrationContext(CTX);

  if (SL && !SL->Empty()) {
    for (ASTStatementList::iterator I = SL->begin(); I != SL->end(); ++I) {
      if (ASTStatementNode* SN = dynamic_cast<ASTStatementNode*>(*I)) {
        SN->SetDeclarationContext(CTX);

        if (ASTDeclarationNode* DN = dynamic_cast<ASTDeclarationNode*>(SN)) {
          if (ASTIdentifierNode* DId =
              const_cast<ASTIdentifierNode*>(DN->GetIdentifier())) {
            if (SN->IsExpression()) {
              if (ASTSymbolTableEntry* STE =
                  ASTSymbolTable::Instance().Lookup(DId, DId->GetBits(),
                                                    DId->GetSymbolType())) {
                if (LocalScope && CanHaveLocalScope(DId->GetSymbolType())) {
                  DId->SetDeclarationContext(CTX);
                  DN->SetDeclarationContext(CTX);
                  STE->SetContext(CTX);
                  DId->SetLocalScope();
                  STE->SetLocalScope();
                } else if (!LocalScope) {
                  DId->SetDeclarationContext(CTX);
                  DN->SetDeclarationContext(CTX);
                  STE->SetContext(CTX);
                }
              }
            }
          }
        }
      }
    }
  }
}

void
ASTScopeController::SetLocalScope(ASTExpressionList* EL) const {
  assert(EL && "Invalid ASTExpressionList argument!");

  if (EL && !EL->Empty()) {
    for (ASTExpressionList::iterator I = EL->begin();
         I != EL->end(); ++I) {
      if (ASTExpression* EX = dynamic_cast<ASTExpression*>(*I)) {
        ASTIdentifierNode* Id = const_cast<ASTIdentifierNode*>(EX->GetIdentifier());
        if (Id && CanHaveLocalScope(Id->GetSymbolType())) {
          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                                Id->GetSymbolType())) {
            Id->SetLocalScope();
            STE->SetLocalScope();
          }
        }
      }
    }
  }
}

void
ASTScopeController::SetDeclarationContext(ASTExpressionList* EL,
                                          const ASTDeclarationContext* CTX) const {
  assert(EL && "Invalid ASTExpressionList argument!");
  assert(CTX && "Invalid ASTDeclarationContext argument!");

  bool LocalScope =
    !ASTDeclarationContextTracker::Instance().IsGlobalContext(CTX) &&
    !ASTDeclarationContextTracker::Instance().IsCalibrationContext(CTX);

  if (EL && !EL->Empty()) {
    for (ASTExpressionList::iterator I = EL->begin(); I != EL->end(); ++I) {
      if (ASTIdentifierNode* Id = dynamic_cast<ASTIdentifierNode*>(*I)) {
        if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()))
          continue;
        else if (LocalScope && !CanHaveLocalScope(Id->GetSymbolType()))
          continue;
        else if (ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id->GetName())) {
          Id->SetDeclarationContext(CTX);

          if (LocalScope)
            Id->SetLocalScope();

          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                                Id->GetSymbolType())) {
            STE->SetContext(CTX);

            if (LocalScope)
              STE->SetLocalScope();
          }
        }
      } else if (ASTExpressionNode* EXN = dynamic_cast<ASTExpressionNode*>(*I)) {
        if (ASTBinaryOpNode* BOP = dynamic_cast<ASTBinaryOpNode*>(EXN)) {
          ASTIdentifierNode* BId = BOP->GetIdentifier();
          BOP->SetDeclarationContext(CTX);
          BId->SetDeclarationContext(CTX);
          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(BId, BId->GetBits(),
                                                BId->GetSymbolType())) {
            STE->SetContext(CTX);

            if (LocalScope)
              STE->SetLocalScope();
          }
        } else if (ASTUnaryOpNode* UOP = dynamic_cast<ASTUnaryOpNode*>(EXN)) {
          ASTIdentifierNode* UId = UOP->GetIdentifier();
          UOP->SetDeclarationContext(CTX);
          UId->SetDeclarationContext(CTX);
          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(UId, UId->GetBits(),
                                                UId->GetSymbolType())) {
            STE->SetContext(CTX);

            if (LocalScope)
              STE->SetLocalScope();
          }
        } else if (EXN->GetASTType() == ASTTypeIdentifier) {
          ASTIdentifierNode* EId = EXN->GetIdentifier();
          if (ASTTypeSystemBuilder::Instance().IsReservedAngle(EId->GetName()))
            continue;
          else if (LocalScope && !CanHaveLocalScope(EId->GetSymbolType()))
            continue;
          else if (ASTTypeSystemBuilder::Instance().IsImplicitAngle(EId->GetName())) {
            EId->SetDeclarationContext(CTX);

            if (LocalScope)
              EId->SetLocalScope();

            if (ASTSymbolTableEntry* STE =
                ASTSymbolTable::Instance().Lookup(EId, EId->GetBits(),
                                                  EId->GetSymbolType())) {
              STE->SetContext(CTX);

              if (LocalScope)
                STE->SetLocalScope();
            }
          }
        }
      }
    }
  }
}

void
ASTScopeController::SetLocalScope(ASTExpressionNodeList* EL) const {
  if (EL && !EL->Empty()) {
    for (ASTExpressionNodeList::iterator I = EL->begin();
         I != EL->end(); ++I) {
      ASTExpressionNode* EX = *I;
      ASTIdentifierNode* Id = const_cast<ASTIdentifierNode*>(EX->GetIdentifier());
      if (Id && CanHaveLocalScope(Id->GetSymbolType())) {
        if (ASTSymbolTableEntry* STE =
            ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                              Id->GetSymbolType())) {
          Id->SetLocalScope();
          STE->SetLocalScope();
        }
      }
    }
  }
}

void
ASTScopeController::SetDeclarationContext(ASTExpressionNodeList* EL,
                                          const ASTDeclarationContext* CTX) const {
  assert(EL && "Invalid ASTExpressionList argument!");
  assert(CTX && "Invalid ASTDeclarationContext argument!");

  bool LocalScope =
    !ASTDeclarationContextTracker::Instance().IsGlobalContext(CTX) &&
    !ASTDeclarationContextTracker::Instance().IsCalibrationContext(CTX);

  if (EL && !EL->Empty()) {
    for (ASTExpressionNodeList::iterator I = EL->begin(); I != EL->end(); ++I) {
      ASTExpressionNode* EX = *I;
      if (ASTIdentifierNode* Id =
          const_cast<ASTIdentifierNode*>(EX->GetIdentifier())) {
        if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()))
          continue;
        else if (LocalScope && !CanHaveLocalScope(Id->GetSymbolType()))
          continue;
        else if (ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id->GetName())) {
          Id->SetDeclarationContext(CTX);

          if (LocalScope)
            Id->SetLocalScope();

          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                                Id->GetSymbolType())) {
            STE->SetContext(CTX);

            if (LocalScope)
              STE->SetLocalScope();
          }
        } else if (ASTExpressionValidator::Instance().IsNumericType(EX->GetASTType())) {
          continue;
        } else if (ASTBinaryOpNode* BOP = dynamic_cast<ASTBinaryOpNode*>(EX)) {
          ASTIdentifierNode* BId = BOP->GetIdentifier();
          BOP->SetDeclarationContext(CTX);
          BId->SetDeclarationContext(CTX);
          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(BId, BId->GetBits(),
                                                BId->GetSymbolType())) {
            STE->SetContext(CTX);

            if (LocalScope)
              STE->SetLocalScope();
          }
        } else if (ASTUnaryOpNode* UOP = dynamic_cast<ASTUnaryOpNode*>(EX)) {
          ASTIdentifierNode* UId = UOP->GetIdentifier();
          UOP->SetDeclarationContext(CTX);
          UId->SetDeclarationContext(CTX);
          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(UId, UId->GetBits(),
                                                UId->GetSymbolType())) {
            STE->SetContext(CTX);

            if (LocalScope)
              STE->SetLocalScope();
          }
        }
      }
    }
  }
}

void
ASTScopeController::SetGlobalScope(ASTDeclarationList* DL) const {
  if (DL && !DL->Empty()) {
    const ASTDeclarationContext* CTX =
      ASTDeclarationContextTracker::Instance().GetGlobalContext();
    assert(CTX && "Could not obtain a valid Global Declaration Context!");

    for (ASTDeclarationList::iterator I = DL->begin(); I != DL->end(); ++I) {
      (*I)->SetDeclarationContext(CTX);

      ASTIdentifierNode* DId =
        const_cast<ASTIdentifierNode*>((*I)->GetIdentifier());

      if (DId) {
        if ((*I)->IsExpression() && CanHaveLocalScope(DId->GetSymbolType())) {
          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(DId, DId->GetBits(),
                                                DId->GetSymbolType())) {
            DId->SetGlobalScope();
            STE->SetGlobalScope();
          }
        }
      }
    }
  }
}

void
ASTScopeController::SetGlobalScope(ASTStatementList* SL) const {
  if (SL && !SL->Empty()) {
    const ASTDeclarationContext* CTX =
      ASTDeclarationContextTracker::Instance().GetGlobalContext();
    assert(CTX && "Could not obtain a valid Global Declaration Context!");

    for (ASTStatementList::iterator I = SL->begin(); I != SL->end(); ++I) {
      ASTStatementNode* SN = dynamic_cast<ASTStatementNode*>(*I);
      if (SN && SN->IsExpression()) {
        SN->SetDeclarationContext(CTX);

        if (ASTIdentifierNode* SId =
            const_cast<ASTIdentifierNode*>(SN->GetIdentifier())) {
          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(SId, SId->GetBits(),
                                                SId->GetSymbolType())) {
            SId->SetGlobalScope();
            STE->SetGlobalScope();
          }
        }
      }
    }
  }
}

void
ASTScopeController::SetGlobalScope(ASTExpressionList* EL) const {
  if (EL && !EL->Empty()) {
    const ASTDeclarationContext* CTX =
      ASTDeclarationContextTracker::Instance().GetGlobalContext();
    assert(CTX && "Could not obtain a valid Global Declaration Context!");

    for (ASTExpressionList::iterator I = EL->begin(); I != EL->end(); ++I) {
      if (ASTExpression* EX = dynamic_cast<ASTExpression*>(*I)) {
        if (ASTIdentifierNode* Id =
            const_cast<ASTIdentifierNode*>(EX->GetIdentifier())) {
          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                                Id->GetSymbolType())) {
            Id->SetGlobalScope();
            STE->SetGlobalScope();
          }
        }
      }
    }
  }
}

void
ASTScopeController::SetGlobalScope(ASTExpressionNodeList* EL) const {
  if (EL && !EL->Empty()) {
    const ASTDeclarationContext* CTX =
      ASTDeclarationContextTracker::Instance().GetGlobalContext();
    assert(CTX && "Could not obtain a valid Global Declaration Context!");

    for (ASTExpressionNodeList::iterator I = EL->begin(); I != EL->end(); ++I) {
      ASTExpressionNode* EX = *I;
      assert(EX && "Invalid ASTExpressionNode!");

      EX->SetDeclarationContext(CTX);

      if (ASTIdentifierNode* Id =
          const_cast<ASTIdentifierNode*>(EX->GetIdentifier())) {
        if (ASTSymbolTableEntry* STE =
            ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                              Id->GetSymbolType())) {
          Id->SetLocalScope();
          STE->SetLocalScope();
        }
      }
    }
  }
}

void
ASTScopeController::SetCalBlockScope(ASTStatementList* SL,
                                     const std::string& CBN) const {
  assert(SL && "Invalid ASTStatementList argument!");

  if (SL && !SL->Empty()) {
    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().FindCalibrationBlock(CBN);
    if (!STE) {
      std::stringstream M;
      M << "Invalid calibration block " << CBN << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    const ASTDeclarationContext* CTX =
      ASTDeclarationContextTracker::Instance().GetDefaultCalibrationContext();
    assert(CTX && "Invalid ASTDeclarationContext!");

    for (ASTStatementList::iterator I = SL->begin(); I != SL->end(); ++I) {
      ASTStatementNode* SN = dynamic_cast<ASTStatementNode*>(*I);
      if (SN && SN->IsExpression()) {
        if (ASTIdentifierNode* SId =
            const_cast<ASTIdentifierNode*>(SN->GetIdentifier())) {
          if ((STE = ASTSymbolTable::Instance().Lookup(SId, SId->GetBits(),
                                                      SId->GetSymbolType()))) {
            SId->SetDeclarationContext(CTX);
            STE->SetContext(CTX);
          }
        }
      }
    }
  }
}

void
ASTScopeController::SetCalBlockScope(ASTExpressionList* EL,
                                     const std::string& CBN) const {
  assert(EL && "Invalid ASTExpressionList argument!");

  if (EL && !EL->Empty()) {
    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().FindCalibrationBlock(CBN);
    if (!STE) {
      std::stringstream M;
      M << "Invalid calibration block " << CBN << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    const ASTDeclarationContext* CTX =
      ASTDeclarationContextTracker::Instance().GetDefaultCalibrationContext();
    assert(CTX && "Invalid ASTDeclarationContext!");

    for (ASTExpressionList::iterator I = EL->begin();
         I != EL->end(); ++I) {
      if (ASTExpression* EX = dynamic_cast<ASTExpression*>(*I)) {
        if (ASTIdentifierNode* Id =
            const_cast<ASTIdentifierNode*>(EX->GetIdentifier())) {
          if ((STE = ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                                      Id->GetSymbolType()))) {
            Id->SetDeclarationContext(CTX);
            STE->SetContext(CTX);
          }
        }
      }
    }
  }
}

bool
ASTScopeController::CheckScope(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->IsReference()) {
    if (const ASTIdentifierRefNode* IdR =
        dynamic_cast<const ASTIdentifierRefNode*>(Id)) {
      return CheckScope(IdR);
    }
  }

  if (ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id->GetName()))
    return true;
  else if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()))
    return true;
  else if (ASTIdentifierTypeController::Instance().IsGateType(Id->GetSymbolType()))
    return true;
  else if (ASTUtils::Instance().IsQubitType(Id->GetSymbolType()))
    return true;
  else if (ASTExpressionValidator::Instance().IsFunctionType(Id->GetSymbolType()))
    return true;

  const ASTDeclarationContext* CX = Id->GetDeclarationContext();
  if (CX->IsDead())
    return false;

  if (ASTDeclarationContextTracker::Instance().IsGlobalContext(CX) ||
      ASTDeclarationContextTracker::Instance().GetCurrentContext() == CX)
    return true;

  const ASTDeclarationContext* DCX =
    ASTDeclarationContextTracker::Instance().GetDefaultCalibrationContext();

  if (Id->GetDeclarationContext() == DCX &&
      (ASTCalContextBuilder::Instance().InOpenContext() ||
       ASTDefcalContextBuilder::Instance().InOpenContext()))
    return true;

  CX = ASTDeclarationContextTracker::Instance().GetCurrentContext();

  while (CX) {
    if (CX == Id->GetDeclarationContext()) {
      return true;
      break;
    }

    CX = CX->GetParentContext();
  }

  return false;
}

bool
ASTScopeController::CheckScope(const ASTIdentifierRefNode* IdR) const {
  assert(IdR && "Invalid ASTIdentifierRefNode argument!");

  if (ASTTypeSystemBuilder::Instance().IsImplicitAngle(
                                       IdR->GetIdentifier()->GetName()))
    return true;

  const ASTDeclarationContext* CX = IdR->GetDeclarationContext();
  if (CX->IsDead())
    return false;

  if (ASTDeclarationContextTracker::Instance().IsGlobalContext(CX) ||
      ASTDeclarationContextTracker::Instance().GetCurrentContext() == CX)
    return true;

  const ASTDeclarationContext* DCX =
    ASTDeclarationContextTracker::Instance().GetDefaultCalibrationContext();

  if (IdR->GetIdentifier()->GetDeclarationContext() == DCX &&
      (ASTCalContextBuilder::Instance().InOpenContext() ||
       ASTDefcalContextBuilder::Instance().InOpenContext()))
    return true;

  CX = ASTDeclarationContextTracker::Instance().GetCurrentContext();

  while (CX) {
    if (CX == IdR->GetIdentifier()->GetDeclarationContext()) {
      return true;
      break;
    }

    CX = CX->GetParentContext();
  }

  return false;
}

bool
ASTScopeController::CheckScope(const ASTSymbolTableEntry* STE) const {
  assert(STE && "Invalid ASTSymbolTableEntry argument!");

  if (ASTTypeSystemBuilder::Instance().IsImplicitAngle(
                                       STE->GetIdentifier()->GetName()))
    return true;

  const ASTDeclarationContext* CX = STE->GetContext();
  if (ASTDeclarationContextTracker::Instance().IsGlobalContext(CX) ||
      ASTDeclarationContextTracker::Instance().GetCurrentContext() == CX)
    return true;

  const ASTDeclarationContext* DCX =
    ASTDeclarationContextTracker::Instance().GetDefaultCalibrationContext();

  if (CX == DCX && (ASTCalContextBuilder::Instance().InOpenContext() ||
                    ASTDefcalContextBuilder::Instance().InOpenContext()))
    return true;

  CX = ASTDeclarationContextTracker::Instance().GetCurrentContext();

  while (CX) {
    if (CX == STE->GetIdentifier()->GetDeclarationContext()) {
      return true;
      break;
    }

    CX = CX->GetParentContext();
  }

  return false;
}

void
ASTScopeController::CheckUndefined(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->GetSymbolType() == ASTTypeUndefined) {
    std::stringstream M;
    M << "Unknown Identifier '" << Id->GetName() << "' at current scope.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }
}

void
ASTScopeController::CheckUndefined(const ASTIdentifierRefNode* IdR) const {
  assert(IdR && "Invalid ASTIdentifierRefNode argument!");

  if (IdR->GetSymbolType() == ASTTypeUndefined) {
    std::stringstream M;
    M << "Unknown Identifier '" << IdR->GetName() << "' at current scope.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(IdR), M.str(), DiagLevel::Error);
  }
}

void
ASTScopeController::CheckOutOfScope(const ASTIdentifierNode* Id) const {
  if (!CheckScope(Id)) {
    std::stringstream M;
    M << "Unknown Identifier '" << Id->GetName() << "' at current scope.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }
}

void
ASTScopeController::CheckOutOfScope(const ASTIdentifierRefNode* IdR) const {
  if (!CheckScope(IdR)) {
    std::stringstream M;
    M << "Unknown Identifier '" << IdR->GetIdentifier()->GetName()
      << "' at current scope.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(IdR->GetIdentifier()),
                                              M.str(), DiagLevel::Error);
  }
}

void
ASTScopeController::CheckOutOfScope(const ASTSymbolTableEntry* STE) const {
  if (!CheckScope(STE)) {
    std::stringstream M;
    M << "Unknown Identifier '" << STE->GetIdentifier()->GetName()
      << "' at current scope.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(STE->GetIdentifier()),
                                              M.str(), DiagLevel::Error);
  }
}

void
ASTScopeController::CheckReservedSuffix(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  std::string S = ASTStringUtils::Instance().GetReservedSuffix(Id->GetName());
  bool E = false;

  if (!S.empty()) {
    switch (Id->GetSymbolType()) {
    case ASTTypeMPComplex:
      E = CS.find(S) == CS.end();
      break;
    case ASTTypeOpenPulseFrame:
      E = FS.find(S) == FS.end();
      break;
    default:
      break;
    }

    if (E) {
      std::stringstream M;
      M << "Invalid reserved suffix '" << S << "' for type "
        << PrintTypeEnum(Id->GetSymbolType()) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return;
    }
  }
}

void
ASTScopeController::CheckReservedSuffix(const ASTIdentifierRefNode* IdR) const {
  assert(IdR && "Invalid ASTIdentifierRefNode argument!");

  std::string S = ASTStringUtils::Instance().GetReservedSuffix(IdR->GetName());
  bool E = false;

  if (!S.empty()) {
    switch (IdR->GetSymbolType()) {
    case ASTTypeMPComplex:
      E = CS.find(S) == CS.end();
      break;
    case ASTTypeOpenPulseFrame:
      E = FS.find(S) == FS.end();
      break;
    default:
      break;
    }

    if (E) {
      std::stringstream M;
      M << "Invalid reserved suffix '" << S << "' for type "
        << PrintTypeEnum(IdR->GetSymbolType()) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(IdR->GetIdentifier()),
                                                M.str(), DiagLevel::Error);
      return;
    }
  }
}

void
ASTScopeController::CheckDeclaration(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (!ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal()) {
    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()) ||
        ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id->GetName()))
      return;

    const ASTDeclarationContext* CCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
    assert(CCX && "Could not obtain a valid ASTDeclarationContext!");

    const ASTDeclarationContext* ICX = Id->GetDeclarationContext();
    assert(ICX && "Could not obtain a valid ASTDeclarationContext!");

    if (ASTDeclarationContextTracker::Instance().IsGlobalContext(ICX)) {
      CheckUndefined(Id);

      if (ASTStringUtils::Instance().IsIndexed(Id->GetName())) {
        ASTType ITy = ASTTypeUndefined;
        const ASTSymbolTableEntry* STE = nullptr;
        const ASTIdentifierNode* IId = nullptr;

        switch (Id->GetSymbolType()) {
        case ASTTypeUndefined: {
          std::stringstream M;
          M << "Unknown Identifier '" << Id->GetName() << "' at current scope.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
          return;
        }
          break;
        case ASTTypeBitset: {
          std::string BIS =
            ASTStringUtils::Instance().GetIdentifierBase(Id->GetName());
          STE = ASTSymbolTable::Instance().FindGlobalSymbol(BIS, ASTTypeBitset);
          if (STE) {
            IId = STE->GetIdentifier();
            ITy = IId->GetSymbolType();
          } else {
            STE = ASTSymbolTable::Instance().FindLocalSymbol(BIS, ASTTypeBitset);
            if (STE) {
              IId = STE->GetIdentifier();
              ITy = IId->GetSymbolType();
            }
          }
        }
          break;
        case ASTTypeQubitContainer:
        case ASTTypeQubitContainerAlias: {
          STE = ASTSymbolTable::Instance().FindQubit(Id->GetName());
          if (STE) {
            IId = STE->GetIdentifier();
            ITy = IId->GetSymbolType();
          }
        }
          break;
        case ASTTypeAngle:
        case ASTTypeLambdaAngle:
        case ASTTypePhiAngle:
        case ASTTypeThetaAngle: {
          std::string BIS =
            ASTStringUtils::Instance().GetIdentifierBase(Id->GetName());
          STE = ASTSymbolTable::Instance().FindAngle(BIS);
          if (STE) {
            IId = STE->GetIdentifier();
            ITy = IId->GetSymbolType();
          } else {
            STE = ASTSymbolTable::Instance().FindLocalSymbol(BIS, ASTIntNode::IntBits,
                                                             ASTTypeAngle);
            if (STE) {
              IId = STE->GetIdentifier();
              ITy = IId->GetSymbolType();
            }
          }
        }
          break;
        default:
          STE = ASTSymbolTable::Instance().FindGlobal(Id->GetName());
          if (STE) {
            IId = STE->GetIdentifier();
            ITy = IId->GetSymbolType();
          }
          break;
        }

        if (IId && STE) {
          return;
        }
      }

      std::vector<ASTDeclarationNode*> DV =
        ASTDeclarationBuilder::Instance().FindRange(Id);

      if (DV.empty()) {
        std::stringstream M;
        M << "Unknown Identifier '" << Id->GetName() << "' at current scope.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
        return;
      }

      bool Found = false;
      for (std::vector<ASTDeclarationNode*>::const_iterator DI = DV.begin();
           DI != DV.end(); ++DI) {
        const ASTIdentifierNode* DId = (*DI)->GetIdentifier();
        assert(DId && "Could not obtain a valid ASTIdentifierNode!");

        if (DId->IsReference()) {
          const ASTIdentifierRefNode* DIdR =
            dynamic_cast<const ASTIdentifierRefNode*>(DId);
          if (!DIdR) {
            std::stringstream M;
            M << "Unknown Identifier '" << Id->GetName() << "' at current scope.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
            return;
          } else {
            DId = DIdR->GetIdentifier();
            assert(DId && "Could not obtain a valid ASTIdentifierNode!");
          }
        }

        if (DId == Id &&
            DId->GetDeclarationContext()->IsAlive() &&
            DId->GetDeclarationContext()->GetIndex() <=
            Id->GetDeclarationContext()->GetIndex() &&
            DId->GetSymbolType() == Id->GetSymbolType() &&
            Id->GetSymbolType() != ASTTypeUndefined) {
          Found = true;
          break;
        }
      }

      if (!Found) {
        std::stringstream M;
        M << "Unknown Identifier '" << Id->GetName() << "' at current scope.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
        return;
      } else {
        return;
      }
    } else if (ASTDeclarationContextTracker::Instance().IsCalibrationContext(ICX)) {
      if (!ASTCalContextBuilder::Instance().InOpenContext() &&
          !ASTDefcalContextBuilder::Instance().InOpenContext()) {
        std::stringstream M;
        M << "Not in Calibration or Defcal Context.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
        return;
      }

      CheckUndefined(Id);

      const ASTSymbolTableEntry* STE =
        ASTSymbolTable::Instance().FindCalibrationSymbol(Id);
      if (STE && STE->GetValueType() == Id->GetSymbolType() &&
          STE->GetIdentifier() && STE->GetIdentifier() == Id &&
          STE->GetIdentifier()->GetBits() == Id->GetBits() &&
          Id->GetBits() > 0U) {
        return;
      } else if (ASTStringUtils::Instance().IsReservedSuffix(Id->GetName())) {
        std::string BId = ASTStringUtils::Instance().GetReservedBase(Id->GetName());
        STE = ASTSymbolTable::Instance().FindCalibrationSymbol(BId, Id->GetBits(),
                                                               Id->GetSymbolType());
        if (STE && STE->GetValueType() == Id->GetSymbolType() &&
            STE->GetIdentifier() &&
            STE->GetIdentifier()->GetBits() == Id->GetBits() &&
            Id->GetBits() > 0U)
          return;
      }

      std::stringstream M;
      M << "Unknown Calibration Block Symbol " << Id->GetName()
        << " at current scope.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return;
    }

    CheckUndefined(Id);

    if (!CanHaveLocalScope(Id->GetSymbolType())) {
      std::stringstream M;
      M << "A Symbol of Type " << PrintTypeEnum(Id->GetSymbolType())
        << " cannot have Local Scope." << std::endl;
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return;
    }

    if (!CheckScope(Id)) {
      std::stringstream M;
      M << "Unknown Identifier '" << Id->GetName() << "' at current scope.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return;
    }

    if (ASTStringUtils::Instance().IsReservedSuffix(Id->GetName())) {
      CheckReservedSuffix(Id);

      std::string BId = ASTStringUtils::Instance().GetReservedBase(Id->GetName());
      const ASTSymbolTableEntry* STE =
        ASTSymbolTable::Instance().FindLocalSymbol(Id);

      if (!STE) {
        STE = ASTSymbolTable::Instance().FindLocalSymbol(BId, Id->GetBits(),
                                                         Id->GetSymbolType());
        if (STE && STE->GetIdentifier() &&
            STE->GetIdentifier()->GetBits() == Id->GetBits() &&
            Id->GetBits() > 0U &&
            STE->GetIdentifier()->GetSymbolType() == Id->GetSymbolType() &&
            STE->GetValueType() == Id->GetSymbolType() &&
            STE->GetContext() == Id->GetDeclarationContext())
          return;
      } else {
        if (STE->GetIdentifier()->GetName() == BId &&
            STE->GetIdentifier()->GetBits() == Id->GetBits() &&
            Id->GetBits() > 0U &&
            STE->GetIdentifier()->GetSymbolType() == STE->GetValueType() &&
            STE->GetContext() == Id->GetDeclarationContext())
          return;
      }
    }

    const ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().FindLocalSymbol(Id);
    if (!STE) {
      std::stringstream M;
      M << "Unknown Identifier '" << Id->GetName() << "' at current scope.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return;
    } else {
      if (STE->GetIdentifier() == Id &&
          STE->GetValueType() == Id->GetSymbolType() &&
          Id->GetSymbolType() != ASTTypeUndefined &&
          STE->GetIdentifier()->GetBits() == Id->GetBits() &&
          Id->GetBits() > 0U && STE->GetIdentifier()->GetBits() > 0U &&
          STE->GetContext()->IsAlive() &&
          Id->GetDeclarationContext()->IsAlive() &&
          STE->GetContext()->GetIndex() <= Id->GetDeclarationContext()->GetIndex())
        return;
    }

    std::vector<ASTDeclarationNode*> DV =
      ASTDeclarationBuilder::Instance().FindRange(Id);

    if (DV.empty()) {
      std::stringstream M;
      M << "Unknown Identifier '" << Id->GetName() << "' at current scope.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return;
    }

    bool Found = false;
    for (std::vector<ASTDeclarationNode*>::const_iterator DI = DV.begin();
         DI != DV.end(); ++DI) {
      const ASTIdentifierNode* DId = (*DI)->GetIdentifier();
      assert(DId && "Could not obtain a valid ASTIdentifierNode!");

      if (DId->IsReference()) {
        const ASTIdentifierRefNode* DIdR =
          dynamic_cast<const ASTIdentifierRefNode*>(DId);
        if (!DIdR) {
          std::stringstream M;
          M << "Unknown Identifier '" << Id->GetName() << "' at current scope.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
          return;
        } else {
          DId = DIdR->GetIdentifier();
          assert(DId && "Could not obtain a valid ASTIdentifierNode!");
        }
      }

      if (DId == Id &&
          DId->GetDeclarationContext()->IsAlive() &&
          DId->GetDeclarationContext()->GetIndex() <=
          Id->GetDeclarationContext()->GetIndex() &&
          DId->GetSymbolType() == Id->GetSymbolType() &&
          Id->GetSymbolType() != ASTTypeUndefined) {
        Found = true;
        break;
      }
    }

    if (!Found) {
      std::stringstream M;
      M << "Unknown Identifier '" << Id->GetName() << "' at current scope.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return;
    } else {
      return;
    }
  }

  if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()) ||
      ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id->GetName()))
    return;

  if (ASTStringUtils::Instance().IsReservedSuffix(Id->GetName())) {
    CheckReservedSuffix(Id);

    std::string BId = ASTStringUtils::Instance().GetReservedBase(Id->GetName());
    const ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().FindGlobalSymbol(Id);

    if (!STE) {
      STE = ASTSymbolTable::Instance().FindGlobalSymbol(BId, Id->GetBits(),
                                                        Id->GetSymbolType());
      if (STE && STE->GetIdentifier() &&
          STE->GetIdentifier()->GetBits() == Id->GetBits() &&
          Id->GetBits() > 0U &&
          STE->GetIdentifier()->GetSymbolType() == Id->GetSymbolType() &&
          STE->GetValueType() == Id->GetSymbolType() &&
          STE->GetContext() == Id->GetDeclarationContext())
        return;
    } else {
      if (STE->GetIdentifier()->GetName() == BId &&
          STE->GetIdentifier()->GetBits() == Id->GetBits() &&
          Id->GetBits() > 0U &&
          STE->GetIdentifier()->GetSymbolType() == STE->GetValueType() &&
          STE->GetContext() == Id->GetDeclarationContext())
        return;
    }
  }

  if (ASTStringUtils::Instance().IsIndexed(Id->GetName())) {
    if (const ASTIdentifierRefNode* IdR =
      dynamic_cast<const ASTIdentifierRefNode*>(Id)) {
      CheckDeclaration(IdR->GetIdentifier());
      return;
    }
  } else if (Id->IsReference()) {
    if (const ASTIdentifierRefNode* IdR =
      dynamic_cast<const ASTIdentifierRefNode*>(Id)) {
      CheckDeclaration(IdR->GetIdentifier());
      return;
    }
  }

  std::vector<ASTDeclarationNode*> DV =
    ASTDeclarationBuilder::Instance().FindRange(Id);

  if (DV.empty()) {
    std::stringstream M;
    M << "Unknown Identifier '" << Id->GetName() << "' at current scope.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }

  for (std::vector<ASTDeclarationNode*>::const_iterator I = DV.begin();
       I != DV.end(); ++I) {
    if ((*I)->GetIdentifier()->GetSymbolType() == Id->GetSymbolType() &&
        (*I)->GetIdentifier()->GetSymbolTableEntry() &&
        (*I)->GetIdentifier()->GetSymbolTableEntry()->GetValueType() ==
                                                      Id->GetSymbolType())
      return;
  }

  std::stringstream M;
  M << "Unknown Identifier '" << Id->GetName() << "' at current scope.";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
    DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
}

bool
ASTScopeController::CheckConstantInitializer(const ASTIdentifierNode* Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (!ASTDeclarationBuilder::Instance().IsConstDeclaration(Id)) {
    std::stringstream M;
    M << "'" << Id->GetName() << "' is not a constant initializer.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return false;
  }

  return true;
}

bool
ASTScopeController::CheckConstantInitializer(const ASTDeclarationNode* DN) const {
  assert(DN && "Invalid ASTDeclarationNode argument!");
  if (!DN->IsConst()) {
    std::stringstream M;
    M << "'" << DN->GetIdentifier()->GetName() << "' is not "
      << "a constant initializer.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(DN), M.str(), DiagLevel::Error);
    return false;
  }

  return true;
}

void
ASTScopeController::CheckDeclaration(const ASTIdentifierRefNode* IdR) const {
  assert(IdR && "Invalid ASTIdentifierRefNode argument!");
  CheckDeclaration(IdR->GetIdentifier());
}

} // namespace QASM

