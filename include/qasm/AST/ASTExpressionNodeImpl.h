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

// This header is included at the bottom of ASTExpression.h, outside the
// include-guard of ASTExpression.h but inside that of this file.  At the point
// of inclusion ASTIdentifier.h has already been pulled in transitively (via
// ASTDeclarationContext.h → ... or directly), so ASTIdentifierNode is a
// complete type and the out-of-line definitions below are valid.

#ifndef __QASM_AST_EXPRESSION_NODE_IMPL_H
#define __QASM_AST_EXPRESSION_NODE_IMPL_H

#include <qasm/AST/ASTIdentifier.h>

namespace QASM {

// ASTExpressionNode constructors

inline ASTExpressionNode::ASTExpressionNode(const ASTExpression *E,
                                             const ASTIdentifierNode *Id,
                                             ASTType Ty)
    : ASTExpression(), Expr(E), Ident(Id), Stmt(nullptr), DC(nullptr),
      IndVar(nullptr), Q(), Type(Ty), EXTy(ASTEXTypeSSA), ICF(false) {
  DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
  DC->RegisterSymbol(this, GetASTType());
}

inline ASTExpressionNode::ASTExpressionNode(const ASTIdentifierNode *Id,
                                             ASTType Ty)
    : ASTExpression(), Expr(Id), Ident(Id), Stmt(nullptr), DC(nullptr),
      IndVar(nullptr), Q(), Type(Ty), EXTy(ASTEXTypeSSA), ICF(false) {
  DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
  DC->RegisterSymbol(this, GetASTType());
}

inline ASTExpressionNode::ASTExpressionNode(const ASTIdentifierRefNode *IdR,
                                             ASTType Ty)
    : ASTExpression(), Expr(IdR), Ident(IdR), Stmt(nullptr), DC(nullptr),
      IndVar(nullptr), Q(), Type(Ty), EXTy(ASTEXTypeSSA), ICF(false) {
  DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
  DC->RegisterSymbol(this, GetASTType());
}

inline ASTExpressionNode::ASTExpressionNode(const ASTIdentifierNode *Id,
                                             const ASTStatementNode *ST,
                                             ASTType Ty)
    : ASTExpression(), Expr(nullptr), Ident(Id), Stmt(ST), DC(nullptr),
      IndVar(nullptr), Q(), Type(Ty), EXTy(ASTEXTypeSSA), ICF(false) {
  DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
  DC->RegisterSymbol(this, GetASTType());
}

// ASTExpressionNode methods that need a complete ASTIdentifierNode

inline bool ASTExpressionNode::IsIdentifier() const {
  return Expr == static_cast<const ASTExpression *>(Ident);
}


inline bool ASTExpressionNode::IsMangled() const {
  return Ident->IsMangled();
}

inline void ASTExpressionNode::ResetMangle() {
  Ident->SetMangledName("");
}

inline const ASTExpressionNode *
ASTExpressionNode::GetIdentifierExpression() const {
  return IsExpression() ? this : nullptr;
}

inline const std::string &ASTExpressionNode::GetName() const {
  return Ident->GetName();
}

inline const std::string &ASTExpressionNode::GetMangledName() const {
  return Ident->GetMangledName();
}

inline const std::string &ASTExpressionNode::GetPolymorphicName() const {
  return Ident->GetPolymorphicName();
}

inline const std::string &ASTExpressionNode::GetMangledLiteralName() const {
  return Ident->GetMangledLiteralName();
}

inline ASTIdentifierNode *ASTExpressionNode::GetIdentifier() {
  return const_cast<ASTIdentifierNode *>(Ident);
}

// ASTExpressionNode static error helpers

inline ASTExpressionNode *
ASTExpressionNode::ExpressionError(const ASTIdentifierNode *Id,
                                    const ASTExpressionNode *EN) {
  ASTExpressionNode *ER =
      new ASTExpressionNode(Id, EN, ASTTypeExpressionError);
  assert(ER && "Could not create a valid ASTExpressionNode!");
  ER->SetLocation(Id->GetLocation());
  return ER;
}

inline ASTExpressionNode *
ASTExpressionNode::ExpressionError(const ASTIdentifierRefNode *IdR,
                                    const ASTExpressionNode *EN) {
  ASTExpressionNode *ER =
      new ASTExpressionNode(IdR, EN, ASTTypeExpressionError);
  assert(ER && "Could not create a valid ASTExpressionNode!");
  ER->SetLocation(IdR->GetLocation());
  return ER;
}

// ASTCVRQualifiers / ASTStorageQualifiers / ASTGateQualifiers

inline ASTCVRQualifiers::ASTCVRQualifiers(const ASTIdentifierNode &ID)
    : ASTQualifiers(), CVRQualifiers(0) {
  const std::string &Id = ID.GetName();
  if (Id == "const")
    SetConst(true);
  else if (Id == "volatile")
    SetVolatile(true);
  else if (Id == "restrict")
    SetRestrict(true);
}

inline ASTStorageQualifiers::ASTStorageQualifiers(const ASTIdentifierNode &ID)
    : ASTQualifiers(), StorageQualifiers(0) {
  const std::string &Id = ID.GetName();
  if (Id == "static")
    SetStatic(true);
  else if (Id == "extern")
    SetExtern(true);
}

inline ASTGateQualifiers::ASTGateQualifiers(const ASTIdentifierNode &ID)
    : ASTQualifiers(), GateQualifiers(0) {
  const std::string &Id = ID.GetName();
  if (Id == "dirty")
    SetDirty(true);
  else if (Id == "opaque")
    SetOpaque(true);
}

} // namespace QASM

#endif // __QASM_AST_EXPRESSION_NODE_IMPL_H
