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

#ifndef __QASM_AST_SCOPE_CONTROLLER_H
#define __QASM_AST_SCOPE_CONTROLLER_H

#include <qasm/AST/ASTDeclarationList.h>
#include <qasm/AST/ASTStatement.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTExpressionNodeList.h>
#include <qasm/AST/ASTSymbolTable.h>

#include <iostream>
#include <string>
#include <cassert>

namespace QASM {

class ASTDeclarationContext;

class ASTScopeController {
private:
  static ASTScopeController SC;

protected:
  ASTScopeController() = default;

public:
  static ASTScopeController& Instance() {
    return SC;
  }

  void SetLocalScope(ASTDeclarationList* DL) const;

  void SetDeclarationContext(ASTDeclarationList* DL,
                             const ASTDeclarationContext* CTX) const;

  void SetLocalScope(ASTStatementList* SL) const;

  void SetDeclarationContext(ASTStatementList* SL,
                             const ASTDeclarationContext* CTX) const;

  void SetLocalScope(ASTExpressionList* EL) const;

  void SetDeclarationContext(ASTExpressionList* EL,
                             const ASTDeclarationContext* CTX) const;

  void SetLocalScope(ASTExpressionNodeList* EL) const;

  void SetDeclarationContext(ASTExpressionNodeList* EL,
                             const ASTDeclarationContext* CTX) const;

  void SetCalBlockScope(ASTStatementList* SL,
                        const std::string& CBN) const;

  void SetCalBlockScope(ASTExpressionList* EL,
                        const std::string& CBN) const;

  void SetLocalScope(const ASTIdentifierNode* Id) const {
    const_cast<ASTIdentifierNode*>(Id)->SetLocalScope();
    if (Id->HasSymbolTableEntry())
      const_cast<ASTIdentifierNode*>(Id)->GetSymbolTableEntry()->SetLocalScope();

  }

  void SetLocalScope(ASTIdentifierNode* Id) const {
    Id->SetLocalScope();
    if (Id->HasSymbolTableEntry())
      Id->GetSymbolTableEntry()->SetLocalScope();
  }

  void SetLocalScope(const ASTIdentifierRefNode* IdR) const {
    const_cast<ASTIdentifierRefNode*>(IdR)->SetLocalScope();
    if (IdR->HasSymbolTableEntry())
      const_cast<ASTIdentifierRefNode*>(IdR)->GetSymbolTableEntry()->SetLocalScope();
  }

  void SetLocalScope(ASTIdentifierRefNode* IdR) const {
    IdR->SetLocalScope();
    if (IdR->HasSymbolTableEntry())
      IdR->GetSymbolTableEntry()->SetLocalScope();
  }

  void SetLocalScope(const ASTSymbolTableEntry* STE) const {
    const_cast<ASTSymbolTableEntry*>(STE)->SetLocalScope();
  }

  void SetLocalScope(ASTSymbolTableEntry* STE) const {
    STE->SetLocalScope();
  }

  void SetGlobalScope(ASTDeclarationList* DL) const;

  void SetGlobalScope(ASTStatementList* SL) const;

  void SetGlobalScope(ASTExpressionList* EL) const;

  void SetGlobalScope(ASTExpressionNodeList* EL) const;

  void SetGlobalScope(const ASTIdentifierNode* Id) const {
    const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  }

  void SetGlobalScope(ASTIdentifierNode* Id) const {
    Id->SetGlobalScope();
  }

  void SetGlobalScope(const ASTIdentifierRefNode* IdR) const {
    const_cast<ASTIdentifierRefNode*>(IdR)->SetGlobalScope();
  }

  void SetGlobalScope(ASTIdentifierRefNode* IdR) const {
    IdR->SetGlobalScope();
  }

  void SetGlobalScope(const ASTSymbolTableEntry* STE) const {
    const_cast<ASTSymbolTableEntry*>(STE)->SetGlobalScope();
  }

  void SetGlobalScope(ASTSymbolTableEntry* STE) const {
    STE->SetGlobalScope();
  }

  bool CheckScope(const ASTIdentifierNode* Id) const;

  bool CheckScope(const ASTIdentifierRefNode* IdR) const;

  bool CheckScope(const ASTSymbolTableEntry* STE) const;

  void CheckOutOfScope(const ASTIdentifierNode* Id) const;

  void CheckOutOfScope(const ASTIdentifierRefNode* IdR) const;

  void CheckOutOfScope(const ASTSymbolTableEntry* STE) const;

  void CheckUndefined(const ASTIdentifierNode* Id) const;

  void CheckUndefined(const ASTIdentifierRefNode* IdR) const;

  void CheckDeclaration(const ASTIdentifierNode* Id) const;

  void CheckDeclaration(const ASTIdentifierRefNode* IdR) const;

  void CheckIdentifier(const ASTIdentifierNode* Id) const {
    CheckUndefined(Id);
    CheckDeclaration(Id);
    CheckOutOfScope(Id);
  }

  void CheckScopeAndUndefined(const ASTIdentifierNode* Id) const {
    CheckUndefined(Id);
    CheckOutOfScope(Id);
  }

  bool CheckConstantInitializer(const ASTIdentifierNode* Id) const;

  bool CheckConstantInitializer(const ASTDeclarationNode* DN) const;

  void CheckReservedSuffix(const ASTIdentifierNode* Id) const;

  void CheckReservedSuffix(const ASTIdentifierRefNode* IdR) const;

  void CheckIdentifier(const ASTIdentifierRefNode* IdR) const {
    CheckUndefined(IdR);
    CheckDeclaration(IdR);
    CheckOutOfScope(IdR);
  }

  void CheckScopeAndUndefined(const ASTIdentifierRefNode* IdR) const {
    CheckUndefined(IdR);
    CheckOutOfScope(IdR);
  }

  bool CanHaveLocalScope(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeGate:
    case ASTTypeGateDeclaration:
    case ASTTypeCXGate:
    case ASTTypeCCXGate:
    case ASTTypeCNotGate:
    case ASTTypeHadamardGate:
    case ASTTypeUGate:
    case ASTTypeDefcal:
    case ASTTypeDefcalDeclaration:
    case ASTTypeDefcalGrammar:
    case ASTTypeDefcalGroup:
    case ASTTypeDefcalMeasure:
    case ASTTypeDefcalReset:
    case ASTTypeFunction:
    case ASTTypeFunctionDeclaration:
    case ASTTypeExtern:
    case ASTTypeKernel:
    case ASTTypeEulerAngle:
    case ASTTypePi:
    case ASTTypePiAngle:
    case ASTTypeTauAngle:
      return false;
      break;
    default:
      return true;
      break;
    }
  }

  bool CheckArrayContextType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeGate:
    case ASTTypeGateDeclaration:
    case ASTTypeCXGate:
    case ASTTypeCCXGate:
    case ASTTypeCNotGate:
    case ASTTypeHadamardGate:
    case ASTTypeUGate:
    case ASTTypeDefcal:
    case ASTTypeDefcalDeclaration:
    case ASTTypeDefcalGrammar:
    case ASTTypeDefcalGroup:
    case ASTTypeDefcalMeasure:
    case ASTTypeDefcalReset:
    case ASTTypeFunction:
    case ASTTypeFunctionDeclaration:
      return false;
        break;
    default:
      return true;
      break;
    }
  }
};

} // namespace QASM

#endif // __QASM_AST_SCOPE_CONTROLLER_H

