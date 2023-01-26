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

#include <qasm/AST/ASTStatementBuilder.h>
#include <qasm/AST/ASTIfConditionals.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTDeclarationBuilder.h>
#include <qasm/AST/ASTScopeController.h>
#include <qasm/AST/ASTReturn.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTStatementList ASTStatementBuilder::SL;
ASTStatementBuilder ASTStatementBuilder::B;
std::map<uintptr_t, const ASTStatement*> ASTStatementBuilder::SM;

void ASTStatementList::SetLocalScope() {
  for (ASTStatementList::iterator I = List.begin();
       I != List.end(); ++I) {
    if (ASTStatementNode* ASN = dynamic_cast<ASTStatementNode*>(*I)) {
      if (const ASTIdentifierNode* Id = ASN->GetIdentifier()) {
        const ASTDeclarationContext* DCX = Id->GetDeclarationContext();

        if (!ASTDeclarationContextTracker::Instance().IsGlobalContext(DCX)) {
          const_cast<ASTIdentifierNode*>(Id)->SetLocalScope();
          if (ASTSymbolTableEntry* STE =
              ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                                Id->GetSymbolType()))
            STE->SetLocalScope();
        }
      }
    }
  }
}

void ASTStatementList::SetDeclarationContext(const ASTDeclarationContext* DCX) {
  assert(DCX && "Invalid ASTDeclarationContext argument!");
  bool IsGlobal = ASTDeclarationContextTracker::Instance().IsGlobalContext(DCX);

  for (ASTStatementList::iterator I = List.begin(); I != List.end(); ++I) {
    if (ASTStatementNode* ASN = dynamic_cast<ASTStatementNode*>(*I)) {
      if (ASTDeclarationNode* DN = dynamic_cast<ASTDeclarationNode*>(ASN)) {
        if (!ASTScopeController::Instance().CanHaveLocalScope(DN->GetASTType()) &&
            !IsGlobal) {
          const ASTIdentifierNode* Id = DN->GetIdentifier();
          std::stringstream M;
          M << "A Declaration of Type " << PrintTypeEnum(DN->GetASTType())
            << " cannot have Local Scope.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::Warning);
          continue;
        }

        DN->SetDeclarationContext(DCX);

        if (const ASTIdentifierNode* Id = DN->GetIdentifier()) {
          Id->SetDeclarationContext(DCX);
          if (IsGlobal)
            const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
          else
            const_cast<ASTIdentifierNode*>(Id)->SetLocalScope();
          ASTSymbolTableEntry* STE =
            ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                              Id->GetSymbolType());
          if (STE) {
            const ASTDeclarationContext* STCX = STE->GetContext();

            STE->SetContext(DCX);
            if (IsGlobal)
              STE->SetGlobalScope();
            else
              STE->SetLocalScope(DCX);
            if (ASTDeclarationContextTracker::Instance().IsGlobalContext(STCX)) {
              if (!ASTSymbolTable::Instance().TransferGlobalSymbolToLocal(Id, STE)) {
                std::stringstream M;
                M << "Failure transfering Global Symbol " << Id->GetName()
                  << " to the Local Symbol Table.";
                QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                  DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                               DiagLevel::ICE);
              }
            }
          }
        }
      }
    }
  }
}

bool ASTStatementList::TransferDeclarations(std::map<std::string,
                                            const ASTSymbolTableEntry*>& MM) const {
  if (List.empty())
    return true;

  for (ASTStatementList::const_iterator I = List.begin(); I != List.end(); ++I) {
    if (ASTStatementNode* SN = dynamic_cast<ASTStatementNode*>(*I)) {
      if (ASTDeclarationNode* DN = dynamic_cast<ASTDeclarationNode*>(SN)) {
        const ASTIdentifierNode* DId = DN->GetIdentifier();
        assert(DId && "Could not obtain a valid ASTIdentifierNode!");

        if (!ASTScopeController::Instance().CanHaveLocalScope(DN->GetASTType())) {
          std::stringstream M;
          M << "A Declaration of Type " << PrintTypeEnum(DN->GetASTType())
            << " cannot have Local Scope.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(DId), M.str(),
                                                          DiagLevel::Warning);
          continue;
        }

        if (DId->GetSymbolScope() == ASTSymbolScope::Global ||
            ASTDeclarationContextTracker::Instance().IsGlobalContext(
                                                     DN->GetDeclarationContext()))
          continue;

        if (!ASTDeclarationBuilder::Instance().TransferSymbol(DId, MM))
          return false;
      }
    }
  }

  return true;
}

bool
ASTStatementList::TransferStatements(std::map<std::string,
                                     const ASTSymbolTableEntry*>& MM) const {
  if (List.empty())
    return true;

  uint64_t LS = List.size();

  for (ASTStatementList::const_iterator I = List.begin(); I != List.end(); ++I) {
    if (ASTStatementNode* SN = dynamic_cast<ASTStatementNode*>(*I)) {
      if (dynamic_cast<ASTDeclarationNode*>(SN))
        continue;

      if (ASTReturnStatementNode* RSN = dynamic_cast<ASTReturnStatementNode*>(SN)) {
        if (LS > 1UL) {
          if (!ASTDeclarationBuilder::Instance().TransferReturn(RSN, MM))
            return false;
        }

        continue;
      }

      if (!ASTStatementBuilder::Instance().TransferStatement(SN, MM))
        return false;
    }
  }

  return true;
}

bool
ASTStatementBuilder::TransferStatement(ASTStatementNode* SN,
                                       std::map<std::string,
                                       const ASTSymbolTableEntry*>& MM) const {
  assert(SN && "Invalid ASTStatementNode argument!");

  const ASTIdentifierNode* Id = SN->GetIdentifier();
  assert(Id && "Could not obtain a valid ASTIdentifierNode!");

  if (const ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().FindLocal(Id)) {
    if (MM.insert(std::make_pair(Id->GetName(), STE)).second) {
      ASTSymbolTable::Instance().EraseLocal(Id, Id->GetBits(),
                                            Id->GetSymbolType());
    } else {
      std::stringstream M;
      M << "Could not transfer Statement " << Id->GetName()
        << " Type " << PrintTypeEnum(Id->GetSymbolType()) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }
  }

  return true;
}

} // namespace QASM

