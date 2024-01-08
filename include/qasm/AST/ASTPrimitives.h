/* -*- coding: utf-8 -*-
 *
 * Copyright 2021 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_PRIMITIVES_H
#define __QASM_AST_PRIMITIVES_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTDeclaration.h>
#include <qasm/AST/ASTQualifiers.h>
#include <qasm/AST/ASTValue.h>

#include <iostream>
#include <string>
#include <vector>

namespace QASM {

class ASTVariable : public ASTBase {
private:
  const ASTIdentifierNode *Ident;
  ASTExpression *Type;

public:
  ASTVariable(const ASTIdentifierNode *Id, ASTExpression *E = nullptr)
      : Ident(Id), Type(E) {}

  virtual ~ASTVariable() = default;

  virtual void SetType(ASTExpression *E) { Type = E; }

  virtual const ASTExpression *GetType() const { return Type; }

  virtual ASTExpression *GetType() { return Type; }

  virtual ASTType GetASTType() const override { return ASTTypeVariable; }

  virtual void print() const override {
    std::cout << "<Variable id=\"" << Ident->GetIdentifier() << "\" />"
              << std::endl;
    if (Type)
      Type->print();
  }

  virtual void push(ASTBase * /* unused */) override {}
};

typedef std::vector<ASTVariable *> ASTVariableList;

class ASTVariableDeclaration : public ASTDeclaration {
protected:
  ASTVariable *Var;
  ASTStorageQualifiers Quals;

private:
  ASTVariableDeclaration() = delete;

public:
  ASTVariableDeclaration(const ASTIdentifierNode *VarId,
                         const ASTStorageQualifiers &Q = ASTStorageQualifiers())
      : ASTDeclaration(ASTTypeVariable, VarId), Var(new ASTVariable(VarId)),
        Quals(Q) {}

  virtual void SetVariable(ASTVariable *V) { Var = V; }

  virtual void SetQualifiers(const ASTStorageQualifiers &Q) { Quals = Q; }

  virtual const ASTVariable *GetVariable() const { return Var; }

  virtual ASTVariable *GetVariable() { return Var; }

  virtual const ASTStorageQualifiers &GetQualifiers() const { return Quals; }

  virtual ASTStorageQualifiers &GetQualifiers() { return Quals; }

  virtual ~ASTVariableDeclaration() = default;

  virtual ASTType GetASTType() const override { return ASTTypeVarDecl; }
};

class ASTDeclarationNode;
class ASTExpressionNode;

class ASTParameter : public ASTBase {
private:
  std::string ID;
  const ASTIdentifierNode *Ident;
  const ASTExpression *Expr;
  const ASTDeclarationNode *Decl;

public:
  ASTParameter(const std::string &Id)
      : ID(Id), Ident(nullptr), Expr(nullptr), Decl(nullptr) {}

  ASTParameter(const ASTIdentifierNode *Id)
      : ID(Id->GetName()), Ident(Id), Expr(nullptr), Decl(nullptr) {}

  ASTParameter(const ASTIdentifierNode *Id, const ASTDeclarationNode *D)
      : ID(Id->GetName()), Ident(Id), Expr(nullptr), Decl(D) {}

  ASTParameter(const ASTIdentifierNode *Id, const ASTExpression *E,
               const ASTDeclarationNode *D)
      : ID(Id->GetName()), Ident(Id), Expr(E), Decl(D) {}

  ASTParameter(const ASTExpression *E, const std::string &Id = "");

  ASTParameter(const ASTIdentifierNode *Id, const ASTExpression *E)
      : ID(Id->GetName()), Ident(Id), Expr(E), Decl(nullptr) {}

  virtual ~ASTParameter() = default;

  virtual void SetExpression(ASTExpression *E) { Expr = E; }

  virtual const std::string &GetName() const { return ID; }

  virtual const ASTIdentifierNode *GetIdentifier() const { return Ident; }

  virtual const ASTExpression *GetExpression() const { return Expr; }

  virtual ASTExpression *GetExpression() {
    return const_cast<ASTExpression *>(Expr);
  }

  virtual const ASTDeclarationNode *GetDeclaration() const { return Decl; }

  virtual ASTDeclarationNode *GetDeclaration() {
    return const_cast<ASTDeclarationNode *>(Decl);
  }

  virtual ASTType GetASTType() const override;

  virtual bool HasIdentifier() const { return Ident; }

  virtual bool HasExpr() const { return Expr; }

  virtual bool HasDecl() const { return Decl; }

  virtual bool ExprIsIdentifier() const {
    return Expr && dynamic_cast<const ASTIdentifierNode *>(Expr);
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTParameterDeclaration : public ASTDeclaration {
protected:
  const ASTParameter *Param;

public:
  ASTParameterDeclaration(const ASTIdentifierNode *DeclId,
                          const ASTParameter *P)
      : ASTDeclaration(ASTTypeParameter, DeclId), Param(P) {}

  virtual ~ASTParameterDeclaration() = default;

  virtual void setParameter(ASTParameter *P) { Param = P; }

  virtual const ASTParameter *getParameter() const { return Param; }
};

} // namespace QASM

#endif // __QASM_AST_PRIMITIVES_H
