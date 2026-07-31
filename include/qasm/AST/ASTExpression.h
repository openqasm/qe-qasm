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

#ifndef __QASM_AST_EXPRESSION_H
#define __QASM_AST_EXPRESSION_H

#include <qasm/AST/ASTAnyType.h>
#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTQualifiers.h>

#include <cassert>
#include <vector>

namespace QASM {


class ASTIdentifierNode;
class ASTIdentifierRefNode;
class ASTStatementNode;

class ASTExpression : public ASTBase {
public:
  ASTExpression() : ASTBase() {}

  ASTExpression(const ASTExpression &RHS) : ASTBase(RHS) {}

  virtual ~ASTExpression() = default;

  ASTExpression &operator=(const ASTExpression &RHS) {
    if (this != &RHS)
      ASTBase::operator=(RHS);

    return *this;
  }

  virtual ASTType GetASTType() const override { return ASTTypeExpression; }

  virtual const ASTIdentifierNode *GetIdentifier() const;

  virtual void print() const override {}

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTExpressionNode : public ASTExpression {
  friend class ASTStatementNode;
  friend class ASTBuilder;

protected:
  const ASTExpression *Expr;
  const ASTIdentifierNode *Ident;
  const ASTStatementNode *Stmt;
  mutable const ASTDeclarationContext *DC;
  union {
    const ASTIdentifierNode *IndVar;
    const ASTIdentifierNode *IxInd;
  };

  mutable ASTCVRQualifiers Q;
  ASTType Type;
  ASTExpressionType EXTy;
  mutable bool ICF;

private:
  ASTExpressionNode() = delete;

protected:
  ASTExpressionNode(const ASTIdentifierNode *Id, const ASTExpressionNode *EX,
                    ASTType Ty)
      : ASTExpression(), Expr(EX), Ident(Id), Stmt(nullptr), DC(nullptr),
        IndVar(nullptr), Q(), Type(Ty), EXTy(ASTEXTypeSSA), ICF(false) {
    DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
    DC->RegisterSymbol(this, GetASTType());
  }

  ASTExpressionNode(const ASTIdentifierRefNode *IdR,
                    const ASTExpressionNode *EX, ASTType Ty)
      : ASTExpression(), Expr(EX), Ident(reinterpret_cast<const ASTIdentifierNode *>(IdR)),
        Stmt(nullptr), DC(nullptr), IndVar(nullptr), Q(), Type(Ty),
        EXTy(ASTEXTypeSSA), ICF(false) {
    DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
    DC->RegisterSymbol(this, GetASTType());
  }

protected:
  virtual void SetASTType(ASTType Ty) { Type = Ty; }

  virtual void SetExpressionType(ASTExpressionType Ty) { EXTy = Ty; }

public:
  static const unsigned ExpressionBits = 64U;

public:
  ASTExpressionNode(const ASTExpression *E, const ASTIdentifierNode *Id,
                    ASTType Ty);

  ASTExpressionNode(const ASTIdentifierNode *Id, ASTType Ty);

  ASTExpressionNode(const ASTIdentifierRefNode *IdR, ASTType Ty);

  ASTExpressionNode(const ASTIdentifierNode *Id, const ASTStatementNode *ST,
                    ASTType Ty);

  ASTExpressionNode(const ASTExpressionNode &RHS)
      : ASTExpression(RHS), Expr(RHS.Expr), Ident(RHS.Ident), Stmt(RHS.Stmt),
        DC(RHS.DC), IndVar(RHS.IndVar), Q(RHS.Q), Type(RHS.Type),
        EXTy(RHS.EXTy), ICF(RHS.ICF) {
    switch (RHS.EXTy) {
    case ASTIITypeInductionVariable:
    case ASTAXTypeInductionVariable:
      IndVar = RHS.IndVar;
      break;
    case ASTIITypeIndexIdentifier:
    case ASTAXTypeIndexIdentifier:
      IxInd = RHS.IxInd;
      break;
    default:
      break;
    }
  }

  ASTExpressionNode &operator=(const ASTExpressionNode &RHS) {
    if (this != &RHS) {
      (void)ASTExpression::operator=(RHS);
      Expr = RHS.Expr;
      Ident = RHS.Ident;
      Stmt = RHS.Stmt;
      DC = RHS.DC;

      switch (RHS.EXTy) {
      case ASTIITypeInductionVariable:
      case ASTAXTypeInductionVariable:
        IndVar = RHS.IndVar;
        break;
      case ASTIITypeIndexIdentifier:
      case ASTAXTypeIndexIdentifier:
        IxInd = RHS.IxInd;
        break;
      default:
        IndVar = RHS.IndVar;
        break;
      }

      Q = RHS.Q;
      Type = RHS.Type;
      EXTy = RHS.EXTy;
      ICF = RHS.ICF;
    }

    return *this;
  }

  virtual ~ASTExpressionNode() = default;

  virtual ASTType GetASTType() const override { return Type; }

  virtual ASTSemaType GetSemaType() const { return SemaTypeExpression; }

  virtual void Mangle() {}

  virtual bool IsMangled() const;

  virtual bool IsConstantFolded() const { return ICF; }

  virtual void ResetMangle();

  virtual bool IsPointer() const { return false; }

  virtual bool IsIdentifier() const;

  virtual bool IsStatement() const { return Stmt != nullptr; }

  virtual bool HasParens() const { return false; }

  virtual bool HasInductionVariable() const {
    return IndVar && EXTy == ASTIITypeInductionVariable;
  }

  virtual bool IsInductionVariable() const {
    return IndVar && EXTy == ASTIITypeInductionVariable;
  }

  virtual bool HasIndexIdentifier() const {
    return IxInd && EXTy == ASTIITypeIndexIdentifier;
  }

  virtual bool IsIndexIdentifier() const {
    return IxInd && EXTy == ASTIITypeIndexIdentifier;
  }

  virtual bool IsSSA() const { return EXTy != ASTEXTypeUnknown; }

  virtual ASTExpressionType GetSSAExpressionType() const { return EXTy; }

  virtual bool IsExpression() const {
    return !IsIdentifier() && !IsStatement();
  }

  virtual bool IsAggregate() const { return false; }

  virtual bool IsInitializer() const { return false; }

  virtual const ASTExpressionNode *GetIdentifierExpression() const;

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return Ident;
  }

  virtual const std::string &GetName() const;

  virtual const std::string &GetMangledName() const;

  virtual const std::string &GetPolymorphicName() const;

  virtual const std::string &GetMangledLiteralName() const;

  virtual ASTIdentifierNode *GetIdentifier();

  virtual const ASTIdentifierNode *GetInductionVariable() {
    return EXTy == ASTIITypeInductionVariable ? IndVar : nullptr;
  }

  virtual const ASTIdentifierNode *GetInductionVariable() const {
    return EXTy == ASTIITypeInductionVariable ? IndVar : nullptr;
  }

  virtual const ASTIdentifierNode *GetIndexIdentifier() {
    return EXTy == ASTIITypeIndexIdentifier ? IxInd : nullptr;
  }

  virtual const ASTIdentifierNode *GetIndexIdentifier() const {
    return EXTy == ASTIITypeIndexIdentifier ? IxInd : nullptr;
  }

  const ASTExpression *GetExpression() const { return Expr; }

  const ASTStatementNode *GetStatement() const { return Stmt; }

  virtual bool IsIntegerConstantExpression() const { return false; }

  virtual const ASTDeclarationContext *GetDeclarationContext() const {
    return DC;
  }

  virtual unsigned GetContextIndex() const { return DC->GetIndex(); }

  virtual void SetInductionVariable(const ASTIdentifierNode *IDV) {
    assert(IDV && "Invalid ASTIdentifierNode argument!");
    IndVar = IDV;
    EXTy = ASTIITypeInductionVariable;
  }

  virtual void SetIndexIdentifier(const ASTIdentifierNode *IDX) {
    assert(IDX && "Invalid ASTIdentifierNode argument!");
    IxInd = IDX;
    EXTy = ASTIITypeIndexIdentifier;
  }

  virtual void SetDeclarationContext(const ASTDeclarationContext *DCX) {
    if (DC != DCX) {
      DC->UnregisterSymbol(this);
      DC = DCX;
      DC->RegisterSymbol(this, GetASTType());
    }
  }

  virtual void SetDeclarationContext(const ASTDeclarationContext *DCX) const {
    if (DC != DCX) {
      DC->UnregisterSymbol(this);
      DC = DCX;
      DC->RegisterSymbol(this, GetASTType());
    }
  }

  virtual void SetDeclarationContext(const ASTDeclarationContext *DCX,
                                     ASTType Ty) {
    if (DC != DCX && DC->GetContextType() != Ty) {
      DC->UnregisterSymbol(this);
      DC = DCX;
      DC->RegisterSymbol(this, Ty);
    }
  }

  virtual void SetDeclarationContext(const ASTDeclarationContext *DCX,
                                     ASTType Ty) const {
    if (DC != DCX && DC->GetContextType() != Ty) {
      DC->UnregisterSymbol(this);
      DC = DCX;
      DC->RegisterSymbol(this, Ty);
    }
  }

  virtual void SetQualifiers(const ASTCVRQualifiers &CVR) { Q = CVR; }

  virtual const ASTCVRQualifiers &GetQualifiers() const { return Q; }

  virtual void SetConst(bool V = true) { Q.SetConst(V); }

  virtual void SetConst(bool V = true) const { Q.SetConst(V); }

  virtual void SetVolatile(bool V = true) { Q.SetVolatile(V); }

  virtual void SetVolatile(bool V = true) const { Q.SetVolatile(V); }

  virtual void SetRestrict(bool V = true) { Q.SetVolatile(V); }

  virtual void SetRestrict(bool V = true) const { Q.SetVolatile(V); }

  virtual void SetConstantFolded(bool V = true) { ICF = V; }

  virtual void SetConstantFolded(bool V = true) const { ICF = V; }

  virtual bool IsConst() const { return Q.IsConst(); }

  virtual bool IsVolatile() const { return Q.IsVolatile(); }

  virtual bool IsRestrict() const { return Q.IsRestrict(); }

  virtual bool IsError() const { return Type == ASTTypeExpressionError; }

  virtual const std::string &GetError() const;

  static ASTExpressionNode *
  ExpressionError(const ASTIdentifierNode *Id,
                  const ASTExpressionNode *EN = nullptr);

  static ASTExpressionNode *
  ExpressionError(const ASTIdentifierRefNode *IdR,
                  const ASTExpressionNode *EN = nullptr);

  static ASTExpressionNode *ExpressionError(const ASTIdentifierNode *Id,
                                            const std::string &ERM);

  static ASTExpressionNode *ExpressionError(const ASTIdentifierRefNode *Id,
                                            const std::string &ERM);

  template <typename __To>
  const __To *DynCast() const {
    return dynamic_cast<const __To *>(Expr);
  }

  virtual void print_qualifiers() const { Q.print(); }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTExpressionList {
  friend class ASTExpressionBuilder;

private:
  std::vector<ASTExpression *> List;

public:
  using list_type = std::vector<ASTExpression *>;
  using iterator = typename std::vector<ASTExpression *>::iterator;
  using const_iterator = typename std::vector<ASTExpression *>::const_iterator;

public:
  ASTExpressionList() : List() {}

  ASTExpressionList(const ASTExpressionList &RHS) : List(RHS.List) {}

  virtual ~ASTExpressionList() = default;

  ASTExpressionList &operator=(const ASTExpressionList &RHS) {
    if (this != &RHS)
      List = RHS.List;

    return *this;
  }

  virtual std::size_t Size() const { return List.size(); }

  virtual bool Empty() const { return List.empty(); }

  virtual void Append(ASTExpression *BN) { this->push(BN); }

  iterator begin() { return List.begin(); }
  const_iterator begin() const { return List.begin(); }

  iterator end() { return List.end(); }
  const_iterator end() const { return List.end(); }

  ASTExpression *front() { return List.front(); }
  const ASTExpression *front() const { return List.front(); }

  ASTExpression *back() { return List.back(); }
  const ASTExpression *back() const { return List.back(); }

  virtual ASTType GetASTType() const { return ASTTypeExpressionList; }

  virtual ASTExpression *operator[](std::size_t Index) {
    assert(Index < List.size() && "Index is out-of-range!");

    try {
      return List.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTExpression *operator[](std::size_t Index) const {
    assert(Index < List.size() && "Index is out-of-range!");

    try {
      return List.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual void print() const {
    std::cout << "<ExpressionList>" << std::endl;

    for (std::vector<ASTExpression *>::const_iterator I = List.begin();
         I != List.end(); ++I) {
      std::cout << "<ExpressionNode>" << std::endl;
      (*I)->print();
      std::cout << "</ExpressionNode>" << std::endl;
    }

    std::cout << "</ExpressionList>" << std::endl;
  }

  virtual void push(ASTExpression *EX) {
    assert(EX && "Invalid ASTExpression argument!");

    if (EX)
      List.push_back(EX);
  }
};

} // namespace QASM

#include <qasm/AST/ASTExpressionNodeImpl.h>

#endif // __QASM_AST_EXPRESSION_H
