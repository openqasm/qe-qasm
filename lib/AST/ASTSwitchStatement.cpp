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

#include <qasm/AST/ASTExpressionEvaluator.h>
#include <qasm/AST/ASTExpressionValidator.h>
#include <qasm/AST/ASTSwitchStatement.h>
#include <qasm/AST/ASTUtils.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <cassert>
#include <sstream>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

bool ASTCaseStatementNode::CheckDeclarations() const {
  if (!HS) {
    for (ASTStatementList::const_iterator I = SL->begin(); I != SL->end();
         ++I) {
      const ASTStatementNode *ST = dynamic_cast<const ASTStatementNode *>(*I);
      assert(ST && "Could not dynamic_cast to a valid ASTStatementNode!");

      if (ST && ST->IsDeclaration()) {
        const ASTDeclarationNode *DN =
            dynamic_cast<const ASTDeclarationNode *>(ST);
        assert(DN && "Could not dynamic_cast to a valid ASTDeclarationNode!");

        const ASTIdentifierNode *DId =
            dynamic_cast<const ASTDeclarationNode *>(ST)->GetIdentifier();
        assert(DId && "Could not dynamic_cast to a valid ASTIdentifierNode!");

        std::stringstream M;
        M << "A Declaration at switch case label scope without block "
          << "enclosing braces is invalid.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(DId), M.str(),
            DiagLevel::Error);
        return false;
      }
    }
  }

  return true;
}

bool ASTDefaultStatementNode::CheckDeclarations() const {
  if (!HS) {
    for (ASTStatementList::const_iterator I = SL->begin(); I != SL->end();
         ++I) {
      const ASTStatementNode *ST = dynamic_cast<const ASTStatementNode *>(*I);
      assert(ST && "Could not dynamic_cast to a valid ASTStatementNode!");

      if (ST && ST->IsDeclaration()) {
        const ASTDeclarationNode *DN =
            dynamic_cast<const ASTDeclarationNode *>(ST);
        assert(DN && "Could not dynamic_cast to a valid ASTDeclarationNode!");

        const ASTIdentifierNode *DId =
            dynamic_cast<const ASTDeclarationNode *>(ST)->GetIdentifier();
        assert(DId && "Could not dynamic_cast to a valid ASTIdentifierNode!");

        std::stringstream M;
        M << "A Declaration at switch default label scope without block "
          << "enclosing braces is invalid.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(DId), M.str(),
            DiagLevel::Error);
        return false;
      }
    }
  }

  return true;
}

void ASTSwitchStatementNode::ResolveDefaultStatement(
    const ASTStatementList *SL) {
  assert(SL && "Invalid ASTStatementList argument!");

  unsigned ID = 0U;
  unsigned DCT = 0U;

  for (ASTStatementList::const_iterator LI = SL->begin(); LI != SL->end();
       ++LI) {
    if ((*LI)->GetASTType() == ASTTypeDefaultStatement) {
      if (const ASTDefaultStatementNode *DS =
              dynamic_cast<const ASTDefaultStatementNode *>(*LI)) {
        if (DCT == 1U) {
          std::stringstream M;
          M << "Only one default case label per switch statement is allowed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(*LI), M.str(),
              DiagLevel::Error);
          break;
        }

        DSN = DS;
        const_cast<ASTDefaultStatementNode *>(DSN)->SetStatementIndex(ID);
        DCT += 1U;
      }
    }

    ID += 1U;
  }

  if (!DSN || DCT == 0U) {
    std::stringstream M;
    M << "Switch statement does not have a default case label.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(this), M.str(),
        DiagLevel::Warning);
  }
}

void ASTSwitchStatementNode::ResolveCaseStatements(const ASTStatementList *SL) {
  assert(SL && "Invalid ASTStatementList argument!");

  unsigned ID = 0U;

  for (ASTStatementList::const_iterator LI = SL->begin(); LI != SL->end();
       ++LI) {
    if ((*LI)->GetASTType() == ASTTypeCaseStatement) {
      if (const ASTCaseStatementNode *CS =
              dynamic_cast<const ASTCaseStatementNode *>(*LI)) {
        const_cast<ASTCaseStatementNode *>(CS)->SetStatementIndex(ID);

        if (!CSM.insert(std::make_pair(ID, CS)).second) {
          std::stringstream M;
          M << "Failure inserting switch case statement into the "
            << "switch case statement map.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(CS), M.str(),
              DiagLevel::ICE);
        }
      }
    }

    ID += 1U;
  }
}

void ASTSwitchStatementNode::VerifyStatements(const ASTStatementList *SL) {
  assert(SL && "Invalid ASTStatementList argument!");

  std::set<int64_t> CLS;

  for (ASTStatementList::const_iterator LI = SL->begin(); LI != SL->end();
       ++LI) {
    if ((*LI)->GetASTType() == ASTTypeCaseStatement) {
      if (const ASTCaseStatementNode *CS =
              dynamic_cast<const ASTCaseStatementNode *>(*LI)) {
        if (!CLS.insert(CS->GetCaseIndex()).second) {
          std::stringstream M;
          M << "Duplicate case label: " << CS->GetCaseIndex() << '.';
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(*LI), M.str(),
              DiagLevel::Error);
          return;
        }
      }
    } else if ((*LI)->GetASTType() == ASTTypeDefaultStatement) {
      continue;
    } else {
      std::stringstream M;
      M << "Only case and default label statements are allowed in "
        << "a switch statement.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(*LI), M.str(),
          DiagLevel::Error);
      return;
    }
  }
}

void ASTSwitchStatementNode::ResolveQuantity(const ASTIntNode *DI) {
  assert(DI && "Invalid ASTIntNode argument!");

  if (DI->IsString()) {
    const ASTDeclarationContext *DCX =
        ASTDeclarationContextTracker::Instance().GetCurrentContext();
    assert(DCX && "Could not obtain a valid ASTDeclarationContext!");

    ASTIdentifierNode *MPId = ASTIdentifierNode::MPInt.Clone();
    assert(MPId && "Could not obtain a valid ASTIdentifierNode!");

    MPId->SetDeclarationContext(DCX);
    MPId->SetLocalScope();

    ASTMPIntegerNode *DMPI =
        new ASTMPIntegerNode(MPId, DI->IsSigned() ? Signed : Unsigned, 128U,
                             DI->GetString().c_str());
    assert(DMPI && "Could not create a valid ASTMPIntegerNode!");

    DMPI->SetLocation(DI->GetLocation());
    DMPI->SetDeclarationContext(DCX);
    MPI = DMPI;
    DTy = DMPI->GetASTType();
  } else {
    I = DI;
    DTy = DI->GetASTType();
  }
}

void ASTSwitchStatementNode::ResolveQuantity(const ASTMPIntegerNode *DI) {
  assert(DI && "Invalid ASTMPIntegerNode argument!");

  MPI = DI;
  DTy = DI->GetASTType();
}

void ASTSwitchStatementNode::ResolveQuantity(const ASTBinaryOpNode *DI) {
  assert(DI && "Invalid ASTBinaryOpNode argument!");

  ASTType BTy = ASTExpressionEvaluator::Instance().EvaluatesTo(DI);
  if (!ASTUtils::Instance().IsUnpromotedIntegralType(BTy)) {
    BOP = nullptr;
    DTy = ASTTypeUndefined;

    std::stringstream M;
    M << "Switch Quantity is not an integer.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(DI), M.str(), DiagLevel::Error);
  } else {
    BOP = DI;
    DTy = DI->GetASTType();
  }
}

void ASTSwitchStatementNode::ResolveQuantity(const ASTUnaryOpNode *DI) {
  assert(DI && "Invalid ASTUnaryOpNode argument!");

  ASTType UTy = ASTExpressionEvaluator::Instance().EvaluatesTo(DI);
  if (!ASTUtils::Instance().IsUnpromotedIntegralType(UTy)) {
    UOP = nullptr;
    DTy = ASTTypeUndefined;

    std::stringstream M;
    M << "Switch Quantity is not an integer.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(DI), M.str(), DiagLevel::Error);
  } else {
    UOP = DI;
    DTy = DI->GetASTType();
  }
}

void ASTSwitchStatementNode::ResolveQuantity(const ASTIdentifierNode *DI) {
  assert(DI && "Invalid ASTIdentifierNode argument!");

  switch (DI->GetSymbolType()) {
  case ASTTypeInt:
  case ASTTypeUInt:
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
    DId = DI;
    DTy = ASTTypeIdentifier;
    break;
  default: {
    DId = nullptr;
    DTy = ASTTypeUndefined;

    std::stringstream M;
    M << "Switch Quantity is not an integer.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(DI), M.str(), DiagLevel::Error);
  } break;
  }
}

void ASTSwitchStatementNode::ResolveQuantity(const ASTFunctionCallNode *DI) {
  assert(DI && "Invalid ASTFunctionCallNode argument!");

  if (DI->GetFunctionCallType() != ASTTypeFunctionCallExpression &&
      DI->GetFunctionCallType() != ASTTypeKernelCallExpression) {
    FC = nullptr;
    DTy = ASTTypeUndefined;

    std::stringstream M;
    M << "Only extern and function calls are allowed.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(DI), M.str(), DiagLevel::Error);
    return;
  }

  if (!DI->ReturnsResult()) {
    FC = nullptr;
    DTy = ASTTypeUndefined;

    std::stringstream M;
    M << "Function call requires a non-void return type.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(DI), M.str(), DiagLevel::Error);
    return;
  }

  ASTType RTy = ASTTypeUndefined;

  if (const ASTResultNode *RN = DI->GetResult())
    RTy = RN->GetResultType();

  if (!ASTUtils::Instance().IsUnpromotedIntegralType(RTy)) {
    FC = nullptr;
    DTy = ASTTypeUndefined;

    std::stringstream M;
    M << "Function call return type is not an integer.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(DI), M.str(), DiagLevel::Error);
    return;
  }

  FC = DI;
  DTy = DI->GetASTType();
}

} // namespace QASM
