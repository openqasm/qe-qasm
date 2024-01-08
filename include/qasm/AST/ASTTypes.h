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

#ifndef __QASM_AST_TYPES_H
#define __QASM_AST_TYPES_H

#include <qasm/AST/ASTAnyTypeList.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTMissingConstants.h>
#include <qasm/AST/ASTPrimitives.h>
#include <qasm/AST/ASTProgramBlock.h>
#include <qasm/AST/ASTStatement.h>
#include <qasm/AST/ASTStringUtils.h>
#include <qasm/AST/ASTTypeEnums.h>

#include <gmp.h>
#include <mpc.h>
#include <mpfr.h>

#include <any>
#include <array>
#include <bitset>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

namespace QASM {

class ASTTranslationUnit : public ASTExpression {
private:
  std::string ID;
  ASTProgramBlock *ProgramBlock;

public:
  ASTTranslationUnit() : ID(), ProgramBlock(nullptr) {}

  ASTTranslationUnit(const std::string &Id) : ID(Id), ProgramBlock(nullptr) {}

  ASTTranslationUnit(const std::string &Id, ASTProgramBlock *PB)
      : ID(Id), ProgramBlock(PB) {}

  virtual ~ASTTranslationUnit() = default;

  virtual void SetID(const std::string &Id) { ID = Id; }

  virtual const std::string &GetID() const { return ID; }

  virtual void SetProgramBlock(ASTProgramBlock *PB) { ProgramBlock = PB; }

  virtual const ASTProgramBlock *getProgramBlock() const {
    return ProgramBlock;
  }

  ASTProgramBlock *getProgramBlock() { return ProgramBlock; }

  virtual ASTType GetASTType() const override { return ASTTypeTranslationUnit; }

  virtual ASTSemaType GetSemaType() const { return SemaTypeTranslationUnit; }

  virtual void print() const override {}

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTStatementNode;
class ASTImplicitConversionNode;

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
      : ASTExpression(), Expr(EX), Ident(IdR), Stmt(nullptr), DC(nullptr),
        IndVar(nullptr), Q(), Type(Ty), EXTy(ASTEXTypeSSA), ICF(false) {
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
                    ASTType Ty)
      : ASTExpression(), Expr(E), Ident(Id), Stmt(nullptr), DC(nullptr),
        IndVar(nullptr), Q(), Type(Ty), EXTy(ASTEXTypeSSA), ICF(false) {
    Id->SetExpression(this);
    DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
    DC->RegisterSymbol(this, GetASTType());
  }

  ASTExpressionNode(const ASTIdentifierNode *Id, ASTType Ty)
      : ASTExpression(), Expr(Id), Ident(Id), Stmt(nullptr), DC(nullptr),
        IndVar(nullptr), Q(), Type(Ty), EXTy(ASTEXTypeSSA), ICF(false) {
    Id->SetExpression(this);
    DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
    DC->RegisterSymbol(this, GetASTType());
  }

  ASTExpressionNode(const ASTIdentifierRefNode *IdR, ASTType Ty)
      : ASTExpression(), Expr(IdR), Ident(IdR), Stmt(nullptr), DC(nullptr),
        IndVar(nullptr), Q(), Type(Ty), EXTy(ASTEXTypeSSA), ICF(false) {
    IdR->SetExpression(this);
    DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
    DC->RegisterSymbol(this, GetASTType());
  }

  ASTExpressionNode(const ASTIdentifierNode *Id, const ASTStatementNode *ST,
                    ASTType Ty)
      : ASTExpression(), Expr(nullptr), Ident(Id), Stmt(ST), DC(nullptr),
        IndVar(nullptr), Q(), Type(Ty), EXTy(ASTEXTypeSSA), ICF(false) {
    Id->SetExpression(this);
    DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
    DC->RegisterSymbol(this, GetASTType());
  }

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

  virtual bool IsMangled() const { return Ident->IsMangled(); }

  virtual bool IsConstantFolded() const { return ICF; }

  virtual void ResetMangle() { Ident->SetMangledName(""); }

  virtual bool IsPointer() const { return false; }

  virtual bool IsIdentifier() const { return Expr == Ident; }

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

  virtual const ASTExpressionNode *GetIdentifierExpression() const {
    return IsExpression() ? Ident->GetExpression() : nullptr;
  }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return Ident;
  }

  virtual const std::string &GetName() const { return Ident->GetName(); }

  virtual const std::string &GetMangledName() const {
    return Ident->GetMangledName();
  }

  virtual const std::string &GetPolymorphicName() const {
    return Ident->GetPolymorphicName();
  }

  virtual const std::string &GetMangledLiteralName() const {
    return Ident->GetMangledLiteralName();
  }

  virtual ASTIdentifierNode *GetIdentifier() {
    return const_cast<ASTIdentifierNode *>(Ident);
  }

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
                  const ASTExpressionNode *EN = nullptr) {
    ASTExpressionNode *ER =
        new ASTExpressionNode(Id, EN, ASTTypeExpressionError);
    assert(ER && "Could not create a valid ASTExpressionNode!");
    ER->SetLocation(Id->GetLocation());
    return ER;
  }

  static ASTExpressionNode *
  ExpressionError(const ASTIdentifierRefNode *IdR,
                  const ASTExpressionNode *EN = nullptr) {
    ASTExpressionNode *ER =
        new ASTExpressionNode(IdR, EN, ASTTypeExpressionError);
    assert(ER && "Could not create a valid ASTExpressionNode!");
    ER->SetLocation(IdR->GetLocation());
    return ER;
  }

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

class ASTStringNode : public ASTExpressionNode {
private:
  std::string Value;
  bool ConstLiteral;
  static ASTStringNode *TN;
  static ASTStringNode *FN;

private:
  ASTStringNode() = delete;

protected:
  ASTStringNode(const std::string &V, ASTType Ty)
      : ASTExpressionNode(ASTIdentifierNode::String.Clone(), this, Ty),
        Value(V), ConstLiteral(false) {
    SetQualifiers(ASTCVRQualifiers());
  }

public:
  ASTStringNode(const std::string &V, bool Literal = false,
                const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(ASTIdentifierNode::String.Clone(),
                          ASTTypeStringLiteral),
        Value(V), ConstLiteral(Literal) {
    SetQualifiers(CVR);
  }

  ASTStringNode(const ASTIdentifierNode *Id, const std::string &V,
                bool Literal = false,
                const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(Id, ASTTypeStringLiteral), Value(V),
        ConstLiteral(Literal) {
    SetQualifiers(CVR);
  }

  ASTStringNode(const char *V, bool Literal = false,
                const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::String, ASTTypeStringLiteral),
        Value(V), ConstLiteral(Literal) {
    SetQualifiers(CVR);
  }

  ASTStringNode(const ASTIdentifierNode *Id, const char *V,
                bool Literal = false,
                const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(Id, ASTTypeStringLiteral), Value(V),
        ConstLiteral(Literal) {
    SetQualifiers(CVR);
  }

  virtual ~ASTStringNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeStringLiteral; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void SetValue(const std::string &V) { Value = V; }

  virtual void SetValue(const char *V) { Value = V; }

  virtual const std::string &GetValue() const { return Value; }

  virtual unsigned Size() const { return Value.length(); }

  virtual bool IsLiteral() const { return ConstLiteral; }

  virtual std::string SplitLeftOnDot() const {
    std::string::size_type D = Value.find('.');
    if (D != std::string::npos)
      return Value.substr(0, D);

    return std::string();
  }

  virtual std::string SplitRightOnDot() const {
    std::string::size_type D = Value.find('.');
    if (D != std::string::npos)
      return Value.substr(D + 1, std::string::npos);

    return std::string();
  }

  static ASTStringNode *True() { return TN; }

  static ASTStringNode *False() { return FN; }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTStringNode *ExpressionError(const std::string &ERM) {
    return new ASTStringNode(ERM, ASTTypeExpressionError);
  }

  virtual void print() const override {
    std::cout << "<String>" << std::endl;
    std::cout << "<Value>";
    std::cout << Value;
    std::cout << "</Value>" << std::endl;
    std::cout << "</String>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTEllipsisNode : public ASTExpressionNode {
private:
  std::string Value;

public:
  static const unsigned EllipsisBits = 64U;

public:
  ASTEllipsisNode()
      : ASTExpressionNode(&ASTIdentifierNode::Ellipsis, ASTTypeEllipsis),
        Value("...") {}

  ASTEllipsisNode(const ASTIdentifierNode *Id)
      : ASTExpressionNode(Id, ASTTypeEllipsis), Value("...") {}

  virtual ~ASTEllipsisNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeEllipsis; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual bool IsIntegerConstantExpression() const override { return false; }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual void print() const override {
    std::cout << "<Ellipsis>" << std::endl;
    std::cout << "<Value>" << Value << "</Value>" << std::endl;
    std::cout << "</Ellipsis>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

// An ASTOperatorNode will never have a SymbolTable Entry.
class ASTOperatorNode : public ASTExpressionNode {
private:
  union {
    const ASTIdentifierNode *TId;
    const ASTExpressionNode *TEx;
  };

  ASTType TTy;
  ASTOpType OTy;

private:
  ASTOperatorNode() = delete;

public:
  static const unsigned OperatorBits = 64U;

public:
  ASTOperatorNode(const ASTIdentifierNode *Id, ASTOpType OT)
      : ASTExpressionNode(ASTIdentifierNode::Operator.Clone(), this,
                          ASTTypeOpTy),
        TId(Id), TTy(ASTTypeIdentifier), OTy(OT) {}

  ASTOperatorNode(const ASTExpressionNode *Ex, ASTOpType OT)
      : ASTExpressionNode(ASTIdentifierNode::Operator.Clone(), this,
                          ASTTypeOpTy),
        TEx(Ex), TTy(Ex->GetASTType()), OTy(OT) {}

  virtual ~ASTOperatorNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeOpTy; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetTargetType() const { return TTy; }

  virtual ASTType GetEvaluatedTargetType() const;

  virtual ASTOpType GetOpType() const { return OTy; }

  virtual bool IsIdentifier() const override {
    return TTy == ASTTypeIdentifier && TId != nullptr;
  }

  virtual bool IsExpression() const override {
    return TTy != ASTTypeIdentifier && TEx != nullptr;
  }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual const ASTIdentifierNode *GetTargetIdentifier() const {
    return TTy == ASTTypeIdentifier ? TId : nullptr;
  }

  virtual const ASTExpressionNode *GetTargetExpression() const {
    return TTy != ASTTypeIdentifier ? TEx : nullptr;
  }

  virtual void print() const override {
    std::cout << "<OperatorNode>" << std::endl;
    std::string X = "[x]";
    std::cout << "<Operator>" << PrintOpTypeOperator(OTy, X) << "</Operator>"
              << std::endl;

    std::cout << "<Target>" << std::endl;
    if (TTy == ASTTypeIdentifier)
      TId->print();
    else
      TEx->print();
    std::cout << "</Target>" << std::endl;

    std::cout << "</OperatorNode>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

// An ASTOperandNode will never have a SymbolTable Entry.
class ASTOperandNode : public ASTExpressionNode {
private:
  union {
    const ASTIdentifierNode *TId;
    const ASTExpressionNode *TEx;
  };

  ASTType TTy;

private:
  ASTOperandNode() = delete;

public:
  static const unsigned OperandBits = 64U;

public:
  ASTOperandNode(const ASTIdentifierNode *Id)
      : ASTExpressionNode(ASTIdentifierNode::Operand.Clone(), this,
                          ASTTypeOpndTy),
        TId(Id), TTy(ASTTypeIdentifier) {}

  ASTOperandNode(const ASTExpressionNode *Ex)
      : ASTExpressionNode(ASTIdentifierNode::Operand.Clone(), this,
                          ASTTypeOpndTy),
        TEx(Ex), TTy(Ex->GetASTType()) {}

  virtual ~ASTOperandNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeOpndTy; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTType GetTargetType() const { return TTy; }

  virtual ASTType GetEvaluatedTargetType() const;

  virtual void Mangle() override;

  virtual void SetMangledName(const std::string &MN) {
    ASTExpressionNode::Ident->SetMangledName(MN);
  }

  virtual const std::string &GetMangledName() const override {
    return ASTExpressionNode::Ident->GetMangledName();
  }

  virtual bool IsExpression() const override {
    return TTy != ASTTypeIdentifier && TEx != nullptr;
  }

  virtual bool IsIdentifier() const override {
    return TTy == ASTTypeIdentifier && TId != nullptr;
  }

  virtual const ASTExpressionNode *GetExpression() const {
    return TTy != ASTTypeIdentifier ? TEx : nullptr;
  }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return IsExpression() ? GetExpression()->GetIdentifier() : TId;
  }

  virtual const ASTIdentifierNode *GetTargetIdentifier() const {
    return TTy == ASTTypeIdentifier ? TId : nullptr;
  }

  virtual const std::string &GetName() const override {
    if (IsExpression())
      return GetExpression()->GetName();

    return GetIdentifier()->GetName();
  }

  virtual void print() const override {
    std::cout << "<OperandNode>" << std::endl;
    std::cout << "<Target>" << std::endl;
    if (TTy == ASTTypeIdentifier)
      TId->print();
    else
      TEx->print();
    std::cout << "</Target>" << std::endl;

    std::cout << "</OperandNode>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTTypeExpressionNode : public ASTExpressionNode {
private:
  unsigned TW;
  unsigned CW;

private:
  ASTTypeExpressionNode() = delete;

public:
  ASTTypeExpressionNode(ASTType Ty, unsigned TypeWidth)
      : ASTExpressionNode(&ASTIdentifierNode::Expression, Ty), TW(TypeWidth),
        CW(0U) {}

  ASTTypeExpressionNode(ASTType Ty, unsigned TypeWidth, unsigned ContainerWidth)
      : ASTExpressionNode(&ASTIdentifierNode::Expression, Ty), TW(TypeWidth),
        CW(ContainerWidth) {}

  virtual ~ASTTypeExpressionNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeTypeExpression; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTType GetExpressionType() const {
    return ASTExpressionNode::GetASTType();
  }

  virtual unsigned GetTypeWidth() const { return TW; }

  virtual unsigned GetContainerWidth() const { return CW; }

  virtual void print() const override {
    std::cout << "<TypeExpression>" << std::endl;
    std::cout << "<Type>" << PrintTypeEnum(GetExpressionType()) << "</Type>"
              << std::endl;
    std::cout << "<TypeWidth>" << TW << "</TypeWidth>" << std::endl;
    if (CW)
      std::cout << "<ContainerWidth>" << CW << "</ContainerWidth>" << std::endl;
    std::cout << "</TypeExpression>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTMPIntegerNode;

class ASTIntNode : public ASTExpressionNode {
private:
  ASTSignbit Signbit;
  union {
    int32_t S;
    uint32_t U;
  } Value;

  mutable std::string SR;
  const ASTExpressionNode *Expr;
  unsigned Bits;
  bool OVF;
  bool MP;

protected:
  ASTIntNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        Signbit(Unsigned), Value(), SR(ERM), Expr(this), Bits(32), OVF(true),
        MP(false) {
    Value.U = 0U;
  }

public:
  static const unsigned IntBits = 32U;

public:
  ASTIntNode(ASTSignbit SB = Signed)
      : ASTExpressionNode(&ASTIdentifierNode::Int, ASTTypeInt), Signbit(SB),
        Value(), SR(""), Expr(nullptr), Bits(32), OVF(false), MP(false) {
    Value.U = 0U;
  }

  explicit ASTIntNode(const ASTExpressionNode *E, unsigned NumBits,
                      bool U = false)
      : ASTExpressionNode(&ASTIdentifierNode::Int, ASTTypeInt),
        Signbit(U ? Unsigned : Signed), Value(), SR(""), Expr(E), Bits(NumBits),
        OVF(false), MP(false) {
    assert(E && "Cannot instantiate an ASTIntNode from a "
                "nullptr Expression!");
    Value.U = 0U;
  }

  explicit ASTIntNode(const ASTIdentifierNode *Id, const ASTExpressionNode *E,
                      unsigned NumBits, bool U = false)
      : ASTExpressionNode(Id, ASTTypeInt), Signbit(U ? Unsigned : Signed),
        Value(), SR(""), Expr(E), Bits(NumBits), OVF(false), MP(false) {
    assert(E && "Cannot instantiate an ASTIntNode from a "
                "nullptr Expression!");
    Value.U = 0U;
  }

  explicit ASTIntNode(const ASTIdentifierNode *Id, const ASTExpressionNode *E,
                      unsigned NumBits, const ASTCVRQualifiers &CVR,
                      ASTSignbit SB = ASTSignbit::Signed)
      : ASTExpressionNode(Id, ASTTypeInt), Signbit(SB), Value(), SR(""),
        Expr(E), Bits(NumBits), OVF(false), MP(false) {
    assert(E && "Cannot instantiate an ASTIntNode from a "
                "nullptr Expression!");
    SetQualifiers(CVR);
    Value.U = 0U;
  }

  explicit ASTIntNode(int32_t V,
                      const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Int, ASTTypeInt), Signbit(Signed),
        Value(), SR(""), Expr(nullptr), Bits(sizeof(V) * CHAR_BIT), OVF(false),
        MP(false) {
    SetQualifiers(CVR);
    Value.S = V;
  }

  explicit ASTIntNode(const ASTIdentifierNode *Id, int32_t V,
                      const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(Id, ASTTypeInt), Signbit(Signed), Value(), SR(""),
        Expr(nullptr), Bits(sizeof(V) * CHAR_BIT), OVF(false), MP(false) {
    SetQualifiers(CVR);
    Value.S = V;
  }

  explicit ASTIntNode(uint32_t V,
                      const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Int, ASTTypeInt),
        Signbit(Unsigned), Value(), SR(""), Expr(nullptr),
        Bits(sizeof(V) * CHAR_BIT), OVF(false), MP(false) {
    SetQualifiers(CVR);
    Value.U = V;
  }

  explicit ASTIntNode(const std::string &S,
                      const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Int, ASTTypeInt), Signbit(Signed),
        Value(), SR(S), Expr(nullptr), Bits(sizeof(int32_t) * CHAR_BIT),
        OVF(false), MP(false) {
    SetQualifiers(CVR);
    Value.U = 0U;
  }

  explicit ASTIntNode(const ASTIdentifierNode *Id, uint32_t V,
                      const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(Id, ASTTypeInt), Signbit(Unsigned), Value(), SR(""),
        Expr(nullptr), Bits(sizeof(V) * CHAR_BIT), OVF(false), MP(false) {
    SetQualifiers(CVR);
    Value.U = V;
  }

  virtual ~ASTIntNode() = default;

  virtual bool IsSigned() const { return Signbit == Signed; }

  virtual ASTIntNode *SignedToUnsigned() const {
    if (IsSigned()) {
      uint32_t X = static_cast<uint32_t>(Value.S);
      return new ASTIntNode(X);
    }

    return nullptr;
  }

  virtual ASTType GetASTType() const override { return ASTTypeInt; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void MangleLiteral();

  unsigned Size() const;

  unsigned GetBits() const;

  unsigned Popcount() const {
    return IsSigned() ? __builtin_popcount(Value.S)
                      : __builtin_popcount(Value.U);
  }

  virtual bool IsExpression() const override { return Expr; }

  virtual bool IsMPInteger() const {
    return Expr && (Expr->GetASTType() == ASTTypeMPInteger ||
                    Expr->GetASTType() == ASTTypeMPUInteger);
  }

  virtual bool IsIntegerConstantExpression() const override { return true; }

  virtual void SetMP(bool V) { MP = V; }

  virtual bool IsMP() const { return MP; }

  virtual const ASTExpressionNode *GetExpression() const { return Expr; }

  virtual const ASTMPIntegerNode *GetMPInteger() const;

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual void SetSignBit(ASTSignbit S) { Signbit = S; }

  virtual ASTSignbit GetSignBit() const { return Signbit; }

  virtual int32_t GetSignedValue() const {
    return reinterpret_cast<int32_t>(Value.S);
  }

  virtual uint32_t GetUnsignedValue() const {
    return reinterpret_cast<uint32_t>(Value.U);
  }

  virtual void SetValue(int32_t V) {
    Value.S = V;
    Signbit = Signed;
    Bits = sizeof(V) * CHAR_BIT;
  }

  virtual void SetValue(uint32_t V) {
    Value.U = V;
    Signbit = Unsigned;
    Bits = sizeof(V) * CHAR_BIT;
  }

  virtual void SetOverflow(bool V = true) { OVF = V; }

  virtual bool IsString() const { return !SR.empty(); }

  virtual bool IsOverflow() const { return OVF; }

  virtual void SetString(const std::string &S) { SR = S; }

  virtual const std::string &GetString() const {
    if (SR.empty()) {
      if (IsSigned())
        SR = std::to_string(Value.S);
      else
        SR = std::to_string(Value.U);
    }

    return SR;
  }

  std::vector<bool> AsBitVector() const {
    uint32_t V = IsSigned() ? static_cast<unsigned>(Value.S)
                            : static_cast<unsigned>(Value.U);
    std::vector<bool> R;
    std::bitset<sizeof(uint32_t)> S = V;

    for (unsigned I = 0; I < 32; ++I)
      R.push_back(S[I]);

    return R;
  }

  bool AsBool() const {
    if (IsSigned())
      return Value.S != 0;
    else
      return Value.U != 0U;
  }

  ASTMPIntegerNode *AsMPInteger(unsigned W = 128U) const;

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTIntNode *ExpressionError(const ASTIdentifierNode *Id,
                                     const std::string &ERM) {
    return new ASTIntNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << (IsSigned() ? "<SignedInt>" : "<UnsignedInt>") << std::endl;
    std::cout << "<Identifier>" << this->GetName() << "</Identifier>"
              << std::endl;
    std::cout << "<Bits>" << Bits << "</Bits>" << std::endl;
    if (Expr) {
      std::cout << "<Expression>" << std::endl;
      Expr->print();
      std::cout << "</Expression>" << std::endl;
    } else {
      std::cout << "<Value>";
      switch (Bits) {
      case 32:
        if (IsSigned())
          std::cout << std::dec << GetSignedValue();
        else
          std::cout << std::dec << GetUnsignedValue();
        break;
      default:
        assert(0 && "ASTIntNode can only have 32 Bits!");
        break;
      }
      std::cout << "</Value>" << std::endl;
    }

    std::cout << (IsSigned() ? "</SignedInt>" : "</UnsignedInt>") << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTMPDecimalNode;

class ASTMPIntegerNode : public ASTExpressionNode {
  friend class ASTIntNode;

private:
  ASTSignbit Signbit;
  unsigned Bits;
  mpz_t MPValue;
  const ASTExpressionNode *Expr;

private:
  ASTMPIntegerNode() = delete;

protected:
  void InitFromString(const char *NS, ASTSignbit SB, unsigned NumBits,
                      int Base);

  ASTMPIntegerNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        Signbit(Unsigned), Bits(static_cast<unsigned>(~0x0)), MPValue(),
        Expr(this) {}

public:
  static const unsigned DefaultBits = 64U;

public:
  explicit ASTMPIntegerNode(const ASTIdentifierNode *Id, ASTSignbit S,
                            unsigned NumBits)
      : ASTExpressionNode(Id, ASTTypeMPInteger), Signbit(S), Bits(NumBits),
        MPValue(), Expr(nullptr) {
    Id->SetBits(NumBits);
    mpz_init2(MPValue, Bits);
    if (Signbit == Signed)
      mpz_set_si(MPValue, 0L);
    else
      mpz_set_ui(MPValue, 0UL);
  }

  explicit ASTMPIntegerNode(const ASTIdentifierNode *Id, ASTSignbit S,
                            unsigned NumBits, const char *String, int Base = 10)
      : ASTExpressionNode(Id, ASTTypeMPInteger), Signbit(S), Bits(NumBits),
        MPValue(), Expr(nullptr) {
    Id->SetBits(NumBits);
    InitFromString(String, S, NumBits, Base);
  }

  explicit ASTMPIntegerNode(const ASTIdentifierNode *Id, ASTSignbit S,
                            unsigned NumBits, const ASTExpressionNode *E)
      : ASTExpressionNode(Id, ASTTypeMPInteger), Signbit(S), Bits(NumBits),
        MPValue(), Expr(E) {
    Id->SetBits(NumBits);
    mpz_init2(MPValue, Bits);
    if (Signbit == Signed)
      mpz_set_si(MPValue, 0L);
    else
      mpz_set_ui(MPValue, 0UL);
  }

  explicit ASTMPIntegerNode(const ASTIntNode *I, unsigned NumBits)
      : ASTExpressionNode(&ASTIdentifierNode::MPInt, ASTTypeMPInteger),
        Signbit(I->GetSignBit()), Bits(NumBits), MPValue(), Expr(I) {
    mpz_init2(MPValue, Bits);
    if (I->IsSigned())
      mpz_set_si(MPValue, static_cast<int64_t>(I->GetSignedValue()));
    else
      mpz_set_ui(MPValue, static_cast<uint64_t>(I->GetUnsignedValue()));
  }

  explicit ASTMPIntegerNode(const ASTIdentifierNode *Id, const ASTIntNode *I,
                            unsigned NumBits)
      : ASTExpressionNode(Id, ASTTypeMPInteger), Signbit(I->GetSignBit()),
        Bits(NumBits), MPValue(), Expr(I) {
    Id->SetBits(NumBits);
    mpz_init2(MPValue, Bits);
    if (I->IsSigned())
      mpz_set_si(MPValue, static_cast<int64_t>(I->GetSignedValue()));
    else
      mpz_set_ui(MPValue, static_cast<uint64_t>(I->GetUnsignedValue()));
  }

  explicit ASTMPIntegerNode(const ASTIdentifierNode *Id, unsigned NumBits,
                            const mpz_t &MPZ, bool Unsigned)
      : ASTExpressionNode(Id, ASTTypeMPInteger),
        Signbit(Unsigned ? ASTSignbit::Unsigned : ASTSignbit::Signed),
        Bits(NumBits), MPValue(), Expr(nullptr) {
    Id->SetBits(NumBits);
    mpz_init2(MPValue, NumBits);
    mpz_set(MPValue, MPZ);
  }

  virtual ~ASTMPIntegerNode() { mpz_clear(MPValue); }

  virtual ASTType GetASTType() const override { return ASTTypeMPInteger; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void MangleLiteral();

  virtual bool IsValid() const { return Bits > 0; }

  virtual bool IsExpression() const override { return Expr; }

  virtual const ASTExpressionNode *GetExpression() const { return Expr; }

  virtual bool IsIntegerConstantExpression() const override { return true; }

  virtual bool IsCastResult() const {
    return Expr && (Expr->GetASTType() == ASTTypeInt);
  }

  virtual bool IsImplicitConversion() const;

  virtual void SetImplicitConversion(const ASTImplicitConversionNode *ICX);

  virtual bool IsZero() const { return mpz_sgn(MPValue) == 0; }

  virtual bool IsNegative() const { return mpz_sgn(MPValue) == -1; }

  virtual bool IsPositive() const { return mpz_sgn(MPValue) == 1; }

  virtual std::string GetValue(int Base = 10) const;

  static std::string GetValue(const mpz_t &MPZ, int Base = 10);

  virtual bool SetValue(const char *String, int Base = 10) {
    if (mpz_set_str(MPValue, String, Base) != 0) {
      if (Signbit == Signed)
        mpz_set_si(MPValue, 0L);
      else
        mpz_set_ui(MPValue, 0UL);
      Bits = 0;

      return false;
    }

    return true;
  }

  virtual unsigned GetBits() const { return Bits; }

  static int InitMPZFromString(mpz_t &MPV, const char *NS, ASTSignbit SB);

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual const mpz_t &GetMPValue() const { return MPValue; }

  virtual void GetMPValue(mpz_t &Out) { mpz_set(Out, MPValue); }

  virtual void SetSignBit(ASTSignbit S) { Signbit = S; }

  virtual ASTSignbit GetSignBit() const { return Signbit; }

  virtual bool IsSigned() const { return Signbit == Signed; }

  virtual uint32_t ToUnsignedInt() const {
    return static_cast<uint32_t>(mpz_get_ui(MPValue));
  }

  virtual int32_t ToSignedInt() const {
    return static_cast<int32_t>(mpz_get_si(MPValue));
  }

  virtual uint64_t ToUnsignedLong() const { return mpz_get_ui(MPValue); }

  virtual int64_t ToSignedLong() const { return mpz_get_si(MPValue); }

  ASTMPDecimalNode *AsMPDecimal() const;

  std::vector<bool> AsBitVector() const {
    std::vector<bool> R;
    std::string BIS = GetValue(2);

    for (unsigned I = 0; I < BIS.length(); ++I)
      R.push_back(BIS[I] == u8'1');

    return R;
  }

  virtual uint64_t Popcount() const {
    return static_cast<uint64_t>(mpz_popcount(MPValue));
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTMPIntegerNode *ExpressionError(const ASTIdentifierNode *Id,
                                           const std::string &ERM) {
    return new ASTMPIntegerNode(Id, ERM);
  }

  static ASTMPIntegerNode *ExpressionError(const std::string &ERM) {
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), ERM);
  }

  virtual void print() const override {
    std::cout << "<MPInteger>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<Bits>" << Bits << "</Bits>" << std::endl;
    std::cout << "<Value>" << GetValue() << "</Value>" << std::endl;
    if (IsSigned())
      std::cout << "<Signbit>Signed</Signbit>" << std::endl;
    else
      std::cout << "<Signbit>Unsigned</Signbit>" << std::endl;

    if (Expr) {
      std::cout << "<Expression>" << std::endl;
      Expr->print();
      std::cout << "</Expression>" << std::endl;
    }

    std::cout << "</MPInteger>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

#ifdef __cplusplus
extern "C" {
#endif

struct SFloat {
  uint32_t M : 23;
  uint32_t E : 8;
  uint32_t S : 1;
};

#ifdef __cplusplus
} // extern "C"
#endif

class ASTFloatNode : public ASTExpressionNode {
private:
  const ASTExpressionNode *Expr;
  float Value;

private:
  static ASTFloatNode *DPi;
  static ASTFloatNode *DNegPi;
  static ASTFloatNode *DTau;
  static ASTFloatNode *DNegTau;
  static ASTFloatNode *DEuler;
  static ASTFloatNode *DNegEuler;

private:
  ASTFloatNode() = delete;

protected:
  ASTFloatNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        Expr(this), Value(0.0f / 0.0f) {}

public:
  class UFloat {
  private:
    union {
      float F;
      SFloat X;
    };

  public:
    uint32_t Mantissa() const { return static_cast<uint32_t>(X.M); }

    uint32_t Exponent() const { return static_cast<uint32_t>(X.E); }

    uint32_t Signbit() const { return static_cast<uint32_t>(X.S); }

    uint32_t AsUnsignedInt() const {
      return *(reinterpret_cast<const uint32_t *>(&X));
    }

    UFloat(float V) : F(V) {}
    UFloat(const UFloat &RHS) : F(RHS.F) {}

    UFloat(uint32_t MV, uint32_t EV, uint32_t SV) : F(0.0f) {
      X.M = MV;
      X.E = EV;
      X.S = SV;
    }

    UFloat &operator=(const UFloat &RHS) {
      F = RHS.F;
      return *this;
    }

    ~UFloat() = default;
  };

public:
  static const unsigned FloatBits = 32U;

public:
  ASTFloatNode(float V, const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Float, ASTTypeFloat),
        Expr(nullptr), Value(V) {
    SetQualifiers(CVR);
  }

  ASTFloatNode(const ASTIdentifierNode *Id, float V,
               const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(Id, ASTTypeFloat), Expr(nullptr), Value(V) {
    SetQualifiers(CVR);
  }

  ASTFloatNode(const ASTExpressionNode *E,
               const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Float, ASTTypeFloat), Expr(E),
        Value(0.0f) {
    SetQualifiers(CVR);
  }

  ASTFloatNode(const ASTIdentifierNode *Id, const ASTExpressionNode *E,
               const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(Id, ASTTypeFloat), Expr(E), Value(0.0f) {
    SetQualifiers(CVR);
  }

  virtual ~ASTFloatNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeFloat; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void MangleLiteral();

  virtual float GetValue() const { return Value; }

  virtual unsigned Size() const;

  virtual unsigned GetBits() const;

  bool IsExpression() const override { return Expr; }

  virtual bool IsMPDecimal() const {
    return Expr && Expr->GetASTType() == ASTTypeMPDecimal;
  }

  virtual ASTIntNode *AsInt() const {
    ASTIntNode *I = new ASTIntNode(ASTIdentifierNode::Int.Clone(),
                                   static_cast<int32_t>(Value));
    assert(I && "Could not create a valid ASTIntNode!");
    return I;
  }

  virtual UFloat AsUFloat() const { return UFloat(Value); }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  ASTExpressionNode *GetExpression() {
    return Expr ? const_cast<ASTExpressionNode *>(Expr) : nullptr;
  }

  const ASTExpressionNode *GetExpression() const {
    return Expr ? Expr : nullptr;
  }

  const ASTMPDecimalNode *GetMPDecimal() const;

  static ASTFloatNode *Pi() {
    return DPi ? DPi
               : DPi = new ASTFloatNode(ASTIdentifierNode::Pi.Clone(),
                                        float(M_PI));
  }

  static ASTFloatNode *NegPi() {
    return DNegPi ? DNegPi
                  : DNegPi = new ASTFloatNode(ASTIdentifierNode::Pi.Clone(),
                                              float(M_PI) * -1.0f);
  }

  static ASTFloatNode *Tau() {
    return DTau ? DTau
                : DTau = new ASTFloatNode(ASTIdentifierNode::Tau.Clone(),
                                          float(M_PI) * 2.0f);
  }

  static ASTFloatNode *NegTau() {
    return DNegTau ? DNegTau
                   : DNegTau = new ASTFloatNode(ASTIdentifierNode::Tau.Clone(),
                                                (float(M_PI) * 2.0f) * -1.0f);
  }

  static ASTFloatNode *Euler() {
    return DEuler ? DEuler
                  : DEuler =
                        new ASTFloatNode(ASTIdentifierNode::EulerNumber.Clone(),
                                         sinhf(1.0f) + coshf(1.0f));
  }

  static ASTFloatNode *NegEuler() {
    return DNegEuler
               ? DNegEuler
               : DNegEuler =
                     new ASTFloatNode(ASTIdentifierNode::EulerNumber.Clone(),
                                      (sinhf(1.0f) + coshf(1.0f)) * -1.0f);
  }

  virtual bool IsNaN() const {
#if defined(__APPLE__)
    return isnan(Value);
#else
    return isnanf(Value);
#endif
  }

  virtual bool IsInf() const { return std::fpclassify(Value) == FP_INFINITE; }

  virtual bool IsZero() const { return std::fpclassify(Value) == FP_ZERO; }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTFloatNode *ExpressionError(const ASTIdentifierNode *Id,
                                       const std::string &ERM) {
    return new ASTFloatNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<Float>" << std::endl;
    std::cout << "<Identifier>" << this->GetName() << "</Identifier>"
              << std::endl;
    if (Expr)
      Expr->print();
    else {
      std::cout << "<Value>";
      std::cout << std::fixed << std::setprecision(8) << Value;
      std::cout << "</Value>" << std::endl;
    }
    std::cout << "</Float>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

#ifdef __cplusplus
extern "C" {
#endif

struct SDouble {
  uint64_t M : 52;
  uint64_t E : 11;
  uint64_t S : 1;
};

#ifdef __cplusplus
} // extern "C"
#endif

class ASTMPDecimalNode;

class ASTDoubleNode : public ASTExpressionNode {
private:
  const ASTExpressionNode *Expr;
  double Value;
  mutable std::string SR;
  bool MP;

private:
  static ASTDoubleNode *DPi;
  static ASTDoubleNode *DNegPi;
  static ASTDoubleNode *DTau;
  static ASTDoubleNode *DNegTau;
  static ASTDoubleNode *DEuler;
  static ASTDoubleNode *DNegEuler;

private:
  ASTDoubleNode() = delete;

public:
  class UDouble {
  private:
    union {
      double D;
      SDouble X;
    };

  public:
    uint64_t Mantissa() const { return static_cast<uint64_t>(X.M); }

    uint64_t Exponent() const { return static_cast<uint64_t>(X.E); }

    uint64_t Signbit() const { return static_cast<uint64_t>(X.S); }

    uint64_t AsUnsignedLong() const {
      return *(reinterpret_cast<const uint64_t *>(&X));
    }

    UDouble(double V) : D(V) {}
    UDouble(const UDouble &RHS) : D(RHS.D) {}

    UDouble(uint64_t MV, uint64_t EV, uint64_t SV) : D(0.0) {
      X.M = MV;
      X.E = EV;
      X.S = SV;
    }

    UDouble &operator=(const UDouble &RHS) {
      D = RHS.D;
      return *this;
    }

    ~UDouble() = default;
  };

protected:
  ASTDoubleNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        Expr(this), Value(0.0 / 0.0), SR(ERM), MP(false) {}

public:
  static const unsigned DoubleBits = 64U;

public:
  ASTDoubleNode(double V, const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Double, ASTTypeDouble),
        Expr(nullptr), Value(V), SR(""), MP(false) {
    SetQualifiers(CVR);
  }

  ASTDoubleNode(const std::string &S,
                const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Double, ASTTypeDouble),
        Expr(nullptr), Value(0.0), SR(S), MP(false) {
    SetQualifiers(CVR);
  }

  ASTDoubleNode(const ASTIdentifierNode *Id, double V,
                const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(Id, ASTTypeDouble), Expr(nullptr), Value(V), SR(""),
        MP(false) {
    SetQualifiers(CVR);
  }

  ASTDoubleNode(const ASTExpressionNode *E,
                const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Double, ASTTypeDouble), Expr(E),
        Value(0.0), SR(""), MP(false) {
    SetQualifiers(CVR);
  }

  ASTDoubleNode(const ASTIdentifierNode *Id, const ASTExpressionNode *E,
                const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(Id, ASTTypeDouble), Expr(E), Value(0.0), SR(""),
        MP(false) {
    SetQualifiers(CVR);
  }

  virtual ~ASTDoubleNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeDouble; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void MangleLiteral();

  virtual bool IsExpression() const override { return Expr; }

  virtual bool IsString() const { return !SR.empty(); }

  virtual bool IsMP() const { return MP; }

  virtual unsigned Size() const;

  virtual unsigned GetBits() const;

  virtual bool IsMPDecimal() const {
    return Expr && Expr->GetASTType() == ASTTypeMPDecimal;
  }

  virtual bool IsOverflow() const { return MP; }

  virtual const std::string &GetString() const {
    if (SR.empty())
      SR = std::to_string(Value);

    return SR;
  }

  const ASTMPDecimalNode *GetMPDecimal() const;

  virtual void SetString(const std::string &S) { SR = S; }

  virtual void SetMP(bool V = true) { MP = V; }

  ASTIntNode *AsInt() const {
    ASTIntNode *I =
        new ASTIntNode(&ASTIdentifierNode::Int, static_cast<int32_t>(Value));
    assert(I && "Could not create a valid ASTIntNode!");
    return I;
  }

  ASTMPDecimalNode *AsMPDecimal(unsigned W = 128) const;

  UDouble AsUDouble() const { return UDouble(Value); }

  virtual double GetValue() const { return Value; }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual ASTExpressionNode *GetExpression() {
    return Expr ? const_cast<ASTExpressionNode *>(Expr) : nullptr;
  }

  virtual const ASTExpressionNode *GetExpression() const {
    return Expr ? Expr : nullptr;
  }

  static ASTDoubleNode *Pi() {
    return DPi ? DPi
               : DPi = new ASTDoubleNode(ASTIdentifierNode::Pi.Clone(),
                                         double(M_PI));
  }

  static ASTDoubleNode *NegPi() {
    return DNegPi ? DNegPi
                  : DNegPi = new ASTDoubleNode(ASTIdentifierNode::Pi.Clone(),
                                               double(M_PI) * -1.0);
  }

  static ASTDoubleNode *Tau() {
    return DTau ? DTau
                : DTau = new ASTDoubleNode(ASTIdentifierNode::Tau.Clone(),
                                           double(M_PI) * 2.0);
  }

  static ASTDoubleNode *NegTau() {
    return DNegTau ? DNegTau
                   : DNegTau = new ASTDoubleNode(ASTIdentifierNode::Tau.Clone(),
                                                 (double(M_PI) * 2.0) * -1.0);
  }

  static ASTDoubleNode *Euler() {
    return DEuler
               ? DEuler
               : DEuler =
                     new ASTDoubleNode(ASTIdentifierNode::EulerNumber.Clone(),
                                       sinh(1.0) + cosh(1.0));
  }

  static ASTDoubleNode *NegEuler() {
    return DNegEuler
               ? DNegEuler
               : DNegEuler =
                     new ASTDoubleNode(ASTIdentifierNode::EulerNumber.Clone(),
                                       (sinh(1.0) + cosh(1.0)) * -1.0);
  }

  virtual float ToFloat() const { return static_cast<float>(Value); }

  virtual long double ToLongDouble() const {
    return static_cast<long double>(Value);
  }

  virtual bool IsNan() const {
#if defined(__APPLE__)
    return isnan(Value);
#else
    return std::isnan(Value);
#endif
  }

  virtual bool IsInf() const { return std::fpclassify(Value) == FP_INFINITE; }

  virtual bool IsZero() const { return std::fpclassify(Value) == FP_ZERO; }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTDoubleNode *ExpressionError(const ASTIdentifierNode *Id,
                                        const std::string &ERM) {
    return new ASTDoubleNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<Double>" << std::endl;
    std::cout << "<Identifier>" << this->GetName() << "</Identifier>"
              << std::endl;
    if (Expr)
      Expr->print();
    else {
      std::cout << "<Value>";
      std::cout << std::fixed << std::setprecision(16) << Value;
      std::cout << "</Value>" << std::endl;
    }
    std::cout << "</Double>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTLongDoubleNode : public ASTExpressionNode {
private:
  long double Value;

private:
  static ASTLongDoubleNode *DPi;
  static ASTLongDoubleNode *DNegPi;
  static ASTLongDoubleNode *DTau;
  static ASTLongDoubleNode *DNegTau;
  static ASTLongDoubleNode *DEuler;
  static ASTLongDoubleNode *DNegEuler;

protected:
  ASTLongDoubleNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        Value(0.0 / 0.0) {}

public:
  static const unsigned LongDoubleBits = 128U;

public:
  ASTLongDoubleNode()
      : ASTExpressionNode(&ASTIdentifierNode::LongDouble, ASTTypeLongDouble),
        Value(0.0 / 0.0) {}

  ASTLongDoubleNode(double V, const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::LongDouble, ASTTypeLongDouble),
        Value(V) {
    SetQualifiers(CVR);
  }

  ASTLongDoubleNode(const ASTIdentifierNode *Id, double V,
                    const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(Id, ASTTypeLongDouble), Value(V) {
    SetQualifiers(CVR);
  }

  ASTLongDoubleNode(long double V,
                    const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::LongDouble, ASTTypeLongDouble),
        Value(V) {
    SetQualifiers(CVR);
  }

  ASTLongDoubleNode(const ASTIdentifierNode *Id, long double V,
                    const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(Id, ASTTypeLongDouble), Value(V) {
    SetQualifiers(CVR);
  }

  ASTLongDoubleNode(const char *V,
                    const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::LongDouble, ASTTypeLongDouble),
        Value(0.0 / 0.0) {
    SetQualifiers(CVR);
    if (V)
      Value = std::strtold(V, NULL);
  }

  ASTLongDoubleNode(const ASTIdentifierNode *Id, const char *V,
                    const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(Id, ASTTypeLongDouble), Value(0.0 / 0.0) {
    SetQualifiers(CVR);
    if (V)
      Value = std::strtold(V, NULL);
  }

  virtual ~ASTLongDoubleNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeLongDouble; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void MangleLiteral();

  virtual void SetValue(long double V) { Value = V; }

  virtual long double GetValue() const { return Value; }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return &ASTIdentifierNode::LongDouble;
  }

  static ASTLongDoubleNode *Pi() {
    return DPi ? DPi
               : DPi = new ASTLongDoubleNode(ASTIdentifierNode::Pi.Clone(),
                                             static_cast<long double>(M_PIl));
  }

  static ASTLongDoubleNode *NegPi() {
    return DNegPi ? DNegPi
                  : DNegPi = new ASTLongDoubleNode(
                        ASTIdentifierNode::Pi.Clone(),
                        static_cast<long double>(M_PIl) * -1.0);
  }

  static ASTLongDoubleNode *Tau() {
    return DTau ? DTau
                : DTau = new ASTLongDoubleNode(ASTIdentifierNode::Tau.Clone(),
                                               static_cast<long double>(M_PIl) *
                                                   2.0);
  }

  static ASTLongDoubleNode *NegTau() {
    return DNegTau ? DNegTau
                   : DNegTau = new ASTLongDoubleNode(
                         ASTIdentifierNode::Tau.Clone(),
                         (static_cast<long double>(M_PIl) * 2.0) * -1.0);
  }

  static ASTLongDoubleNode *Euler() {
    return DEuler ? DEuler
                  : DEuler = new ASTLongDoubleNode(
                        ASTIdentifierNode::EulerNumber.Clone(),
                        sinhl(1.0) + coshl(1.0));
  }

  static ASTLongDoubleNode *NegEuler() {
    return DNegEuler ? DNegEuler
                     : DNegEuler = new ASTLongDoubleNode(
                           ASTIdentifierNode::EulerNumber.Clone(),
                           (sinhl(1.0) + coshl(1.0)) * -1.0);
  }

  virtual float ToFloat() const { return static_cast<float>(Value); }

  virtual double ToDouble() const { return static_cast<double>(Value); }

  virtual bool IsNaN() const {
#if defined(__APPLE__)
    return isnan(Value);
#else
    return isnanl(Value);
#endif
  }

  virtual bool IsInf() const { return std::fpclassify(Value) == FP_INFINITE; }

  virtual bool IsZero() const { return std::fpclassify(Value) == FP_ZERO; }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTLongDoubleNode *ExpressionError(const ASTIdentifierNode *Id,
                                            const std::string &ERM) {
    return new ASTLongDoubleNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<LongDouble>" << std::endl;
    std::cout << "<Value>" << Value << "</Value>" << std::endl;
    std::cout << "</LongDouble>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTMPComplexNode;

class ASTMPDecimalNode : public ASTExpressionNode {
private:
  unsigned Bits;
  unsigned Precision;
  unsigned DeclBits;
  mpfr_t MPValue;
  const ASTExpressionNode *Expr;

private:
  static bool SPW;

private:
  ASTMPDecimalNode() = delete;

protected:
  ASTMPDecimalNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        Bits(static_cast<unsigned>(~0x0)),
        Precision(static_cast<unsigned>(~0x0)),
        DeclBits(static_cast<unsigned>(~0x0)), MPValue(), Expr(this) {}

public:
  static const unsigned DefaultBits = 64U;

public:
  explicit ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits = 64U)
      : ASTExpressionNode(Id, ASTTypeMPDecimal),
        Bits(NumBits >= 32 ? NumBits : 32),
        Precision(NumBits >= 32 ? NumBits : 32), DeclBits(NumBits), MPValue(),
        Expr(nullptr) {
    Id->SetBits(NumBits > 32 ? NumBits : 32);
    mpfr_init2(MPValue, NumBits > 32 ? NumBits : 32);
    mpfr_set_nan(MPValue);
  }

  explicit ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits,
                            unsigned Prec)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(Prec >= NumBits ? Prec : NumBits), DeclBits(NumBits),
        MPValue(), Expr(nullptr) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, NumBits);
    mpfr_set_nan(MPValue);
  }

  explicit ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits,
                            const char *String, int Base = 10)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(NumBits >= 32 ? NumBits : 32), DeclBits(NumBits), MPValue(),
        Expr(nullptr) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, NumBits);
    // MPFR_RNDN == round-to-nearest. This is IEEE-754 compliant.
    if (mpfr_strtofr(MPValue, String, NULL, Base, MPFR_RNDN) != 0)
      mpfr_set_nan(MPValue);
  }

  explicit ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits,
                            unsigned Prec, const char *String, int Base = 10)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(Prec >= NumBits ? Prec : NumBits), DeclBits(NumBits),
        MPValue(), Expr(nullptr) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, NumBits);
    // MPFR_RNDN == round-to-nearest. This is IEEE-754 compliant.
    if (mpfr_strtofr(MPValue, String, NULL, Base, MPFR_RNDN) != 0)
      mpfr_set_nan(MPValue);
  }

  ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits,
                   const mpfr_t &Value)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(NumBits >= 32 ? NumBits : 32), DeclBits(NumBits), MPValue(),
        Expr(nullptr) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, NumBits);
    if (mpfr_set(MPValue, Value, MPFR_RNDN) != 0)
      mpfr_set_nan(MPValue);
  }

  explicit ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits,
                            unsigned Prec, const mpfr_t &Value)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(Prec >= 32 ? Prec : 32), DeclBits(NumBits), MPValue(),
        Expr(nullptr) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, NumBits);
    if (mpfr_set(MPValue, Value, MPFR_RNDN) != 0)
      mpfr_set_nan(MPValue);
  }

  ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits, double Value)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(NumBits >= 64 ? NumBits : 64), DeclBits(NumBits), MPValue(),
        Expr(nullptr) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, NumBits);
    if (mpfr_set_d(MPValue, Value, MPFR_RNDN) != 0)
      mpfr_set_nan(MPValue);
  }

  explicit ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits,
                            unsigned Prec, double Value)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(Prec >= NumBits ? Prec : NumBits), DeclBits(NumBits),
        MPValue(), Expr(nullptr) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, NumBits);
    if (mpfr_set_d(MPValue, Value, MPFR_RNDN) != 0)
      mpfr_set_nan(MPValue);
  }

  ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits, float Value)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(NumBits >= 32 ? NumBits : 32), DeclBits(NumBits), MPValue(),
        Expr(nullptr) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, NumBits);
    if (mpfr_set_d(MPValue, static_cast<double>(Value), MPFR_RNDN) != 0)
      mpfr_set_nan(MPValue);
  }

  ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits, unsigned Prec,
                   float Value)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(Prec >= NumBits ? Prec : NumBits), DeclBits(NumBits),
        MPValue(), Expr(nullptr) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, NumBits);
    if (mpfr_set_d(MPValue, static_cast<double>(Value), MPFR_RNDN) != 0)
      mpfr_set_nan(MPValue);
  }

  ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits,
                   long double Value)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(NumBits >= 128 ? NumBits : 128), DeclBits(NumBits), MPValue(),
        Expr(nullptr) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, NumBits);
    if (mpfr_set_ld(MPValue, Value, MPFR_RNDN) != 0)
      mpfr_set_nan(MPValue);
  }

  explicit ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits,
                            const ASTMPIntegerNode *MPI)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(NumBits >= MPI->GetBits() ? NumBits : MPI->GetBits()),
        DeclBits(NumBits), MPValue(), Expr(MPI) {
    Id->SetBits(NumBits >= MPI->GetBits() ? NumBits : MPI->GetBits());
    mpfr_init2(MPValue, NumBits);
    // MPFR_RNDD == round to nearest.
    mpfr_set_z(MPValue, MPI->GetMPValue(), MPFR_RNDN);
  }

  explicit ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits,
                            const ASTMPDecimalNode *MPD)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(NumBits >= MPD->GetBits() ? NumBits : MPD->GetBits()),
        DeclBits(NumBits), MPValue(), Expr(MPD) {
    Id->SetBits(NumBits >= MPD->GetBits() ? NumBits : MPD->GetBits());
    mpfr_init2(MPValue, NumBits);
    // MPFR_RNDD == round to nearest.
    mpfr_set(MPValue, MPD->GetMPValue(), MPFR_RNDN);
  }

  ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits,
                   const ASTExpressionNode *E)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(NumBits >= 32 ? NumBits : 32), DeclBits(NumBits), MPValue(),
        Expr(E) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, NumBits);
    // MPFR_RNDD == round to nearest.
    mpfr_set_d(MPValue, 0.0, MPFR_RNDN);
  }

  ASTMPDecimalNode(const ASTIdentifierNode *Id, unsigned NumBits, unsigned Prec,
                   const ASTExpressionNode *E)
      : ASTExpressionNode(Id, ASTTypeMPDecimal), Bits(NumBits),
        Precision(Prec >= NumBits ? Prec : NumBits), DeclBits(NumBits),
        MPValue(), Expr(E) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, NumBits);
    // MPFR_RNDD == round to nearest.
    if (mpfr_set_d(MPValue, 0.0, MPFR_RNDN) != 0)
      mpfr_set_nan(MPValue);
  }

  ASTMPDecimalNode(const ASTDoubleNode *D, unsigned NumBits = 64U)
      : ASTExpressionNode(&ASTIdentifierNode::MPDec, ASTTypeMPDecimal),
        Bits(NumBits), Precision(NumBits >= 32 ? NumBits : 32),
        DeclBits(NumBits), MPValue(), Expr(nullptr) {
    mpfr_init2(MPValue, NumBits);
    if (mpfr_set_d(MPValue, D->GetValue(), MPFR_RNDN) != 0)
      mpfr_set_nan(MPValue);
  }

  ASTMPDecimalNode(double Value, unsigned NumBits = 64U)
      : ASTExpressionNode(&ASTIdentifierNode::MPDec, ASTTypeMPDecimal),
        Bits(NumBits), Precision(NumBits >= 32 ? NumBits : 32),
        DeclBits(NumBits), MPValue(), Expr(nullptr) {
    mpfr_init2(MPValue, NumBits);
    if (mpfr_set_d(MPValue, Value, MPFR_RNDN) != 0)
      mpfr_set_nan(MPValue);
  }

  virtual ~ASTMPDecimalNode() { mpfr_clear(MPValue); }

  virtual ASTType GetASTType() const override { return ASTTypeMPDecimal; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void MangleLiteral();

  virtual bool IsValid() const {
    return Bits > 0 && Bits != static_cast<unsigned>(~0x0);
  }

  virtual bool IsMP() const { return Bits > 64U; }

  virtual unsigned GetBits() const { return Bits; }

  virtual unsigned GetDeclBits() const { return DeclBits; }

  static void SetDefaultPrecision(int Precision) {
    mpfr_set_default_prec(Precision);
  }

  static int GetDefaultPrecision() {
    return static_cast<int>(mpfr_get_default_prec());
  }

  static void EnablePrecisionWarnings() { ASTMPDecimalNode::SPW = true; }

  static void DisablePrecisionWarnings() { ASTMPDecimalNode::SPW = false; }

  unsigned GetPrecision() const { return Precision; }

  virtual bool IsNan() const { return mpfr_nan_p(MPValue) != 0; }

  virtual bool IsInf() const { return mpfr_inf_p(MPValue) != 0; }

  virtual bool IsZero() const { return mpfr_zero_p(MPValue) != 0; }

  virtual bool IsNegative() const { return mpfr_sgn(MPValue) < 0; }

  virtual bool IsPositive() const { return mpfr_sgn(MPValue) > 0; }

  // return true if MPValue is neither NaN nor Inf.
  virtual bool IsNumber() const { return mpfr_number_p(MPValue) != 0; }

  // return true if MPValue is neither NaN nor Inf nor Zero.
  virtual bool IsRegular() const { return mpfr_regular_p(MPValue) != 0; }

  static ASTMPDecimalNode *Pi(int Bits = 64);
  static ASTMPDecimalNode *Pi(int Bits, int Prec);

  static ASTMPDecimalNode *NegPi(int Bits = 64);
  static ASTMPDecimalNode *NegPi(int Bits, int Prec);

  static ASTMPDecimalNode *Tau(int Bits = 64);
  static ASTMPDecimalNode *Tau(int Bits, int Prec);

  static ASTMPDecimalNode *NegTau(int Bits = 64);
  static ASTMPDecimalNode *NegTau(int Bits, int Prec);

  static ASTMPDecimalNode *Euler(int Bits = 64);
  static ASTMPDecimalNode *Euler(int Bits, int Prec);

  static ASTMPDecimalNode *NegEuler(int Bits = 64);
  static ASTMPDecimalNode *NegEuler(int Bits, int Prec);

  virtual ASTSignbit GetSignBit() const {
    return mpfr_signbit(MPValue) == 0 ? Unsigned : Signed;
  }

  virtual float ToFloat() const { return mpfr_get_flt(MPValue, MPFR_RNDN); }

  virtual double ToDouble() const { return mpfr_get_d(MPValue, MPFR_RNDN); }

  virtual long double ToLongDouble() const {
    return mpfr_get_ld(MPValue, MPFR_RNDN);
  }

  virtual ASTMPIntegerNode *ToMPInteger() const {
    mpz_t ROP;
    mpz_init2(ROP, Bits + Bits / 2);
    (void)mpfr_get_z(ROP, MPValue, MPFR_RNDN);

    ASTMPIntegerNode *MPI = new ASTMPIntegerNode(
        ASTIdentifierNode::MPInt.Clone(), Bits + Bits / 2, ROP, false);
    assert(MPI && "Could not create a valid ASTMPIntegerNode!");
    return MPI;
  }

  virtual ASTMPComplexNode *ToMPComplex() const;

  static int InitMPFRFromString(mpfr_t &MPV, const char *S, int Base = 10);

  virtual std::string GetValue(int Base) const {
    if (IsNan())
      return "NaN";
    else if (IsZero())
      return "0.0";
    else if (IsInf())
      return "Inf";

    std::string R;
    mpfr_exp_t E = 0;
    char *S = mpfr_get_str(NULL, &E, Base, 0, MPValue, MPFR_RNDN);
    if (S) {
      std::stringstream SSR;
      if (E) {
        if (S[0] == u8'-' || S[0] == u8'+')
          E += 1U;
        SSR.write(S, (size_t)E);
        SSR << u8'.';
        const char *SP = S + (size_t)E;
        SSR << SP;
        R = SSR.str();
      } else {
        SSR << "0.";
        if (Bits <= 32U)
          SSR.write(S, 8U);
        else if (Bits <= 64U)
          SSR.write(S, 16U);
        else if (Bits <= 128U)
          SSR.write(S, 32U);
        else
          SSR.write(S, 34U);
        R = SSR.str();
      }

      mpfr_free_str(S);
    }

    ASTStringUtils::Instance().SaneDecimal(R);
    return R;
  }

  virtual std::string GetValue(const char *Format) const {
    if (IsNan())
      return "NaN";
    else if (IsZero())
      return "0.0";
    else if (IsInf())
      return "Inf";

    std::string R;
    char S[1024];
    (void)memset(S, 0, sizeof(S));

    if (mpfr_sprintf(S, Format, MPValue) < 0)
      return R;

    R = S;
    ASTStringUtils::Instance().SaneDecimal(R);
    return R;
  }

  virtual std::string GetValue() const {
    if (IsNan())
      return "NaN";
    else if (IsZero())
      return "0.0";
    else if (IsInf())
      return "Inf";
    else
      return GetValue(10);
  }

  virtual bool SetValue(const char *String, int Base = 10) {
    if (mpfr_set_str(MPValue, String, Base, MPFR_RNDN) != 0) {
      mpfr_set_nan(MPValue);
      return false;
    }

    return true;
  }

  virtual void SetImplicitConversion(const ASTImplicitConversionNode *ICX);

  virtual bool IsImplicitConversion() const;

  virtual bool IsExpression() const override { return Expr; }

  virtual const ASTExpressionNode *GetExpression() const { return Expr; }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual const mpfr_t &GetMPValue() const { return MPValue; }

  virtual bool GetMPValue(mpfr_t &Out) {
    if (mpfr_set(Out, MPValue, MPFR_RNDN) != 0) {
      mpfr_set_nan(Out);
      return false;
    }

    return true;
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTMPDecimalNode *ExpressionError(const ASTIdentifierNode *Id,
                                           const std::string &ERM) {
    return new ASTMPDecimalNode(Id, ERM);
  }

  static ASTMPDecimalNode *ExpressionError(const std::string &ERM) {
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(), ERM);
  }

  virtual void print() const override {
    std::cout << "<MPDecimal>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<Bits>" << Bits << "</Bits>" << std::endl;
    std::cout << "<Precision>" << Precision << "</Precision>" << std::endl;
    std::cout << "<Value>" << GetValue() << "</Value>" << std::endl;

    if (Expr) {
      std::cout << "<Expression>" << std::endl;
      Expr->print();
      std::cout << "</Expression>" << std::endl;
    }

    std::cout << "</MPDecimal>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTCharNode : public ASTExpressionNode {
private:
  ASTSignbit Signbit;
  union {
    int8_t SV;
    uint8_t UV;
  } Value;

public:
  static const unsigned CharBits = 8U;

public:
  ASTCharNode()
      : ASTExpressionNode(&ASTIdentifierNode::Char, ASTTypeChar),
        Signbit(Signed) {
    Value.UV = 0;
  }

  ASTCharNode(int8_t V, const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Char, ASTTypeChar),
        Signbit(Signed) {
    SetQualifiers(CVR);
    Value.SV = V;
  }

  ASTCharNode(uint8_t V, const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Char, ASTTypeChar),
        Signbit(Unsigned) {
    SetQualifiers(CVR);
    Value.UV = V;
  }

  virtual ~ASTCharNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeChar; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual bool IsSigned() const { return Signbit == Signed; }

  virtual void SetSignBit(ASTSignbit S) { Signbit = S; }

  virtual ASTSignbit GetSignBit() const { return Signbit; }

  virtual void SetValue(int8_t V) {
    Value.SV = V;
    Signbit = Signed;
  }

  virtual void SetValue(uint8_t V) {
    Value.UV = V;
    Signbit = Unsigned;
  }

  virtual void print() const override {
    std::cout << (IsSigned() ? "<SignedChar>" : "<UnsignedChar>") << std::endl;
    std::cout << "<Value>";
    if (Signbit == Signed)
      std::cout << Value.SV;
    else
      std::cout << Value.UV;
    std::cout << "</Value>" << std::endl;
    std::cout << (IsSigned() ? "</SignedChar>" : "</UnsignedChar>")
              << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTShortNode : public ASTExpressionNode {
private:
  ASTSignbit Signbit;
  union {
    int16_t SV;
    uint16_t UV;
  } Value;

public:
  ASTShortNode()
      : ASTExpressionNode(&ASTIdentifierNode::Short, ASTTypeShort),
        Signbit(Signed) {
    Value.UV = 0U;
  }

  ASTShortNode(int16_t V, const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Short, ASTTypeShort),
        Signbit(Signed) {
    SetQualifiers(CVR);
    Value.SV = V;
  }

  ASTShortNode(uint16_t V, const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Short, ASTTypeShort),
        Signbit(Unsigned) {
    SetQualifiers(CVR);
    Value.UV = V;
  }

  virtual ~ASTShortNode() = default;

  virtual bool IsSigned() const { return Signbit == Signed; }

  virtual ASTType GetASTType() const override { return ASTTypeShort; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void SetSignBit(ASTSignbit S) { Signbit = S; }

  virtual ASTSignbit GetSignBit() const { return Signbit; }

  virtual void SetValue(int16_t V) {
    Value.SV = V;
    Signbit = Signed;
  }

  virtual void SetValue(uint16_t V) {
    Value.UV = V;
    Signbit = Unsigned;
  }

  virtual bool IsIntegerConstantExpression() const override { return true; }

  virtual void print() const override {
    std::cout << (IsSigned() ? "<SignedShort>" : "<UnsignedShort>")
              << std::endl;
    std::cout << "<Value>";
    if (Signbit == Signed)
      std::cout << Value.SV;
    else
      std::cout << Value.UV;
    std::cout << "</Value>" << std::endl;
    std::cout << (IsSigned() ? "</SignedShort>" : "</UnsignedShort>")
              << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTLongNode : public ASTExpressionNode {
private:
  ASTSignbit Signbit;
  union {
    int64_t SV;
    uint64_t UV;
  } Value;

public:
  ASTLongNode()
      : ASTExpressionNode(&ASTIdentifierNode::Long, ASTTypeLong),
        Signbit(Signed) {
    Value.UV = 0UL;
  }

  ASTLongNode(int64_t V, const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Long, ASTTypeLong),
        Signbit(Signed) {
    SetQualifiers(CVR);
    Value.SV = V;
  }

  ASTLongNode(uint64_t V, const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Long, ASTTypeLong),
        Signbit(Unsigned) {
    SetQualifiers(CVR);
    Value.UV = V;
  }

  virtual ~ASTLongNode() = default;

  virtual bool IsSigned() const { return Signbit == Signed; }

  virtual ASTType GetASTType() const override { return ASTTypeLong; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void SetSignBit(ASTSignbit S) { Signbit = S; }

  virtual ASTSignbit GetSignBit() const { return Signbit; }

  virtual void SetValue(int64_t V) {
    Value.SV = V;
    Signbit = Signed;
  }

  virtual void SetValue(uint64_t V) {
    Value.UV = V;
    Signbit = Unsigned;
  }

  virtual bool IsIntegerConstantExpression() const override { return true; }

  unsigned Popcount() const {
    return IsSigned() ? __builtin_popcount(Value.SV)
                      : __builtin_popcount(Value.UV);
  }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return &ASTIdentifierNode::Long;
  }

  virtual void print() const override {
    std::cout << (IsSigned() ? "<SignedLong>" : "<UnsignedLong>") << std::endl;
    std::cout << "<Value>";
    if (Signbit == Signed)
      std::cout << Value.SV;
    else
      std::cout << Value.UV;
    std::cout << "</Value>" << std::endl;
    std::cout << (IsSigned() ? "</SignedLong>" : "</UnsignedLong>")
              << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTVoidNode : public ASTExpressionNode {
private:
  void *Value;

private:
  ASTVoidNode() = delete;

public:
  static const unsigned VoidBits = 0U;

public:
  ASTVoidNode(void *V, const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Void, ASTTypeVoid), Value(V) {
    SetQualifiers(CVR);
  }

  ASTVoidNode(const ASTIdentifierNode *Id, void *V,
              const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(Id, ASTTypeVoid), Value(V) {
    SetQualifiers(CVR);
  }

  virtual ~ASTVoidNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeVoid; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void SetValue(void *V) { Value = V; }

  virtual void *GetValue() const { return Value; }

  virtual bool IsPointer() const override { return true; }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return &ASTIdentifierNode::Void;
  }

  virtual void print() const override {
    std::cout << "<Void*>" << std::endl;
    std::cout << "<Value>";
    std::cout << Value;
    std::cout << "</Value>" << std::endl;
    std::cout << "</Void*>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTSyntaxErrorNode : public ASTExpressionNode {
private:
  ASTSyntaxErrorNode() = delete;

public:
  ASTSyntaxErrorNode(const std::string &ERM)
      : ASTExpressionNode(ASTIdentifierNode::SyntaxError.Clone(),
                          new ASTStringNode(ERM), ASTTypeSyntaxError) {}

  virtual ~ASTSyntaxErrorNode() = default;

  virtual bool IsError() const override { return true; }

  virtual ASTType GetASTType() const override { return ASTTypeSyntaxError; }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  virtual void print() const override {
    std::cout << "<SyntaxError>" << std::endl;
    std::cout << "<ErrorMessage>" << std::endl;
    std::cout << ASTExpressionNode::GetError() << std::endl;
    std::cout << "</ErrorMessage>" << std::endl;
    std::cout << "</SyntaxError>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTBlockNode : public ASTExpressionNode {
public:
  ASTBlockNode() : ASTExpressionNode(&ASTIdentifierNode::Void, ASTTypeBlock) {}

  virtual ~ASTBlockNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeBlock; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }
};

class ASTBoolNode : public ASTExpressionNode {
private:
  const ASTExpressionNode *Expr;
  bool Value;
  static ASTBoolNode *TN;
  static ASTBoolNode *FN;

protected:
  ASTBoolNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        Expr(this), Value(false) {}

public:
  static const unsigned BoolBits = 8U;

public:
  ASTBoolNode(bool V)
      : ASTExpressionNode(&ASTIdentifierNode::Bool, ASTTypeBool), Expr(nullptr),
        Value(V) {}

  ASTBoolNode(const ASTIdentifierNode *Id, bool V)
      : ASTExpressionNode(Id, ASTTypeBool), Expr(nullptr), Value(V) {}

  ASTBoolNode(const ASTIdentifierNode *Id, const ASTExpressionNode *E)
      : ASTExpressionNode(Id, ASTTypeBool), Expr(E), Value(false) {}

  virtual ~ASTBoolNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeBool; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual bool IsExpression() const override { return Expr; }

  virtual const ASTExpressionNode *GetExpression() const { return Expr; }

  virtual bool IsIntegerConstantExpression() const override { return true; }

  virtual bool GetValue() const { return Value; }

  virtual void SetValue(bool B) { Value = B; }

  static ASTBoolNode *True() { return TN; }

  static ASTBoolNode *False() { return FN; }

  virtual unsigned GetBits() const { return ASTBoolNode::BoolBits; }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTBoolNode *ExpressionError(const std::string &ERM) {
    return new ASTBoolNode(ASTIdentifierNode::Bool.Clone(), ERM);
  }

  static ASTBoolNode *ExpressionError(const ASTIdentifierNode *Id,
                                      const std::string &ERM) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return new ASTBoolNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<Bool>" << std::endl;
    std::cout << "<Identifier>" << this->GetName() << "</Identifier>"
              << std::endl;
    if (Expr) {
      Expr->print();
    } else {
      std::cout << "<Value>";
      std::cout << std::boolalpha << Value;
      std::cout << "</Value>" << std::endl;
    }
    std::cout << "</Bool>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTPointerNode : public ASTExpressionNode {
private:
  const ASTExpressionNode *Pointee;

public:
  ASTPointerNode()
      : ASTExpressionNode(&ASTIdentifierNode::Pointer, ASTTypePointer),
        Pointee(nullptr) {}

  ASTPointerNode(const ASTExpressionNode *Ptr,
                 const ASTCVRQualifiers &CVR = ASTCVRQualifiers())
      : ASTExpressionNode(&ASTIdentifierNode::Pointer, ASTTypePointer),
        Pointee(Ptr) {
    SetQualifiers(CVR);
  }

  virtual ~ASTPointerNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypePointer; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual bool IsPointer() const override { return true; }

  virtual void SetPointee(const ASTExpressionNode *Ptr) { Pointee = Ptr; }

  virtual const ASTExpression *GetPointee() const { return Pointee; }

  virtual void print() const override {
    std::cout << "<Pointer>" << std::endl;
    std::cout << "<Value>" << std::endl;
    Pointee->print();
    std::cout << "</Value>" << std::endl;
    std::cout << "</Pointer>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTImaginaryNode : public ASTExpressionNode {
  friend class ASTBinaryOpNode;

private:
  ASTImaginaryNode() = delete;

protected:
  ASTImaginaryNode(const ASTExpressionNode *EXN)
      : ASTExpressionNode(ASTIdentifierNode::Imag.Clone(), EXN,
                          ASTTypeImaginary) {}

public:
  static const unsigned ImaginaryBits = 64U;

public:
  virtual ~ASTImaginaryNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeImaginary; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual const ASTExpressionNode *GetExpression() const {
    return dynamic_cast<const ASTExpressionNode *>(
        ASTExpressionNode::GetExpression());
  }

  virtual void print() const override {
    std::cout << "<Imaginary></Imaginary>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTBinaryOpNode : public ASTExpressionNode {
private:
  ASTOpType OpType;
  ASTEvaluationMethod EM;
  mutable bool Parens;
  bool SQP;
  const ASTExpressionNode *Left;
  const ASTExpressionNode *Right;
  mutable ASTImaginaryNode *IM;

private:
  ASTBinaryOpNode() = delete;

protected:
  ASTBinaryOpNode(const ASTIdentifierNode *Id, const std::string &ERM,
                  ASTOpType OT)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        OpType(OT), EM(Arithmetic), Parens(false),
        SQP(OT == ASTOpTypeLogicalAnd || OT == ASTOpTypeLogicalOr),
        Left(nullptr), Right(nullptr), IM(nullptr) {}

  ASTBinaryOpNode(const std::string &ERM, ASTOpType OT)
      : ASTExpressionNode(ASTIdentifierNode::BinaryOp.Clone(),
                          new ASTStringNode(ERM), ASTTypeExpressionError),
        OpType(OT), EM(Arithmetic), Parens(false),
        SQP(OT == ASTOpTypeLogicalAnd || OT == ASTOpTypeLogicalOr),
        Left(nullptr), Right(nullptr), IM(nullptr) {}

public:
  static const unsigned BinaryOpBits = 64U;

public:
  ASTBinaryOpNode(const ASTIdentifierNode *Identifier,
                  const ASTExpressionNode *LHS, const ASTExpressionNode *RHS,
                  ASTOpType OT)
      : ASTExpressionNode(this, Identifier, ASTTypeBinaryOp), OpType(OT),
        EM(Arithmetic), Parens(false),
        SQP(OT == ASTOpTypeLogicalAnd || OT == ASTOpTypeLogicalOr), Left(LHS),
        Right(RHS), IM(nullptr) {}

  virtual ~ASTBinaryOpNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeBinaryOp; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTType GetExpressionType() const;

  virtual ASTEvaluationMethod GetEvalMethod() const { return EM; }

  virtual void Mangle() override;

  virtual void AddParens() const { Parens = true; }

  virtual bool HasParens() const override { return Parens; }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual const ASTExpressionNode *GetLeft() const { return Left; }

  virtual const ASTExpressionNode *GetRight() const { return Right; }

  virtual const ASTIdentifierNode *GetLeftIdentifier() const {
    return Left->GetIdentifier();
  }

  virtual const ASTIdentifierNode *GetRightIdentifier() const {
    return Right->GetIdentifier();
  }

  virtual ASTOpType GetOpType() const { return OpType; }

  virtual void SetEvalMethod(ASTEvaluationMethod M) { EM = M; }

  virtual bool IsIntegerConstantExpression() const override;

  virtual bool IsSequencePoint() const { return SQP; }

  virtual void MakeImaginary() {
    IM = new ASTImaginaryNode(Right);
    assert(IM && "Could not create a valid ASTImaginaryNode!");
  }

  virtual void MakeImaginary() const {
    IM = new ASTImaginaryNode(Right);
    assert(IM && "Could not create a valid ASTImaginaryNode!");
  }

  virtual bool IsImaginaryPart() const { return IM; }

  static ASTBinaryOpNode *ExpressionError(const ASTIdentifierNode *Id,
                                          const std::string &ERM,
                                          ASTOpType OTy) {
    return new ASTBinaryOpNode(Id, ERM, OTy);
  }

  static ASTBinaryOpNode *ExpressionError(const std::string &ERM,
                                          ASTOpType OTy) {
    return new ASTBinaryOpNode(ERM, OTy);
  }

  virtual bool IsError() const override {
    return ASTExpressionNode::Type == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTUnaryOpNode : public ASTExpressionNode {
private:
  const ASTExpressionNode *Right;
  mutable ASTType RTy;
  ASTOpType OpType;
  ASTEvaluationMethod EM;
  mutable bool Parens;
  mutable bool IsLValue;

private:
  ASTUnaryOpNode() = delete;

protected:
  ASTUnaryOpNode(const std::string &ERM, ASTOpType OT)
      : ASTExpressionNode(ASTIdentifierNode::UnaryOp.Clone(),
                          new ASTStringNode(ERM), ASTTypeExpressionError),
        Right(nullptr), RTy(ASTTypeExpressionError), OpType(OT), EM(Arithmetic),
        Parens(false), IsLValue(true) {}

  void ResolveRTy() {
    switch (RTy) {
    case ASTTypeOpndTy:
      if (const ASTOperandNode *OPN =
              dynamic_cast<const ASTOperandNode *>(Right))
        RTy = OPN->GetTargetType();
      break;
    case ASTTypeOpTy:
      if (const ASTOperatorNode *OPR =
              dynamic_cast<const ASTOperatorNode *>(Right))
        RTy = OPR->GetTargetType();
      break;
    default:
      break;
    }
  }

public:
  static const unsigned UnaryOpBits = 64U;

public:
  ASTUnaryOpNode(const ASTIdentifierNode *Identifier,
                 const ASTExpressionNode *RHS, ASTOpType OT, bool LV = false)
      : ASTExpressionNode(Identifier, this, ASTTypeUnaryOp), Right(RHS),
        RTy(RHS->GetASTType()), OpType(OT), EM(Arithmetic), Parens(false),
        IsLValue(LV) {}

  virtual ~ASTUnaryOpNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeUnaryOp; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTEvaluationMethod GetEvalMethod() const { return EM; }

  virtual void Mangle() override;

  virtual void AddParens() const { Parens = true; }

  virtual bool HasParens() const override { return Parens; }

  virtual bool HasOperator() const {
    return dynamic_cast<const ASTOperatorNode *>(Right);
  }

  virtual bool HasOperand() const {
    return dynamic_cast<const ASTOperandNode *>(Right);
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual const ASTExpressionNode *GetExpression() const { return Right; }

  virtual const ASTOperatorNode *GetOperator() const {
    return dynamic_cast<const ASTOperatorNode *>(Right);
  }

  virtual const ASTOperandNode *GetOperand() const {
    return dynamic_cast<const ASTOperandNode *>(Right);
  }

  virtual const ASTIdentifierNode *GetRightIdentifier() const {
    return Right->GetIdentifier();
  }

  virtual ASTOpType GetOpType() const { return OpType; }

  // Implemented in ASTBinaryOp.cpp.
  virtual ASTType GetExpressionType() const;

  virtual bool GetIsLValue() const { return IsLValue; }

  virtual void SetIsLValue(bool LV = true) const { IsLValue = LV; }

  virtual bool IsIntegerConstantExpression() const override;

  virtual void SetEvalMethod(ASTEvaluationMethod M) { EM = M; }

  virtual bool IsError() const override {
    return ASTExpressionNode::Type == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTUnaryOpNode *ExpressionError(const std::string &ERM,
                                         ASTOpType OTy) {
    return new ASTUnaryOpNode(ERM, OTy);
  }

  virtual void print() const override {
    std::cout << "<UnaryOpNode>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    if (Parens)
      std::cout << "<LParen>"
                << "("
                << "</LParen>" << std::endl;

    std::cout << "<Op>" << PrintOpTypeEnum(OpType) << "</Op>" << std::endl;
    std::cout << "<ExpressionType>" << PrintTypeEnum(RTy) << "</ExpressionType>"
              << std::endl;
    std::cout << "<EvalMethod>" << PrintEvalMethod(EM) << "</EvalMethod>"
              << std::endl;

    Right->print();
    if (Parens)
      std::cout << "<RParen>"
                << ")"
                << "</RParen>" << std::endl;
    std::cout << "</UnaryOpNode>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTComplexExpressionNode : public ASTExpressionNode {
private:
  union {
    const ASTBinaryOpNode *BOP;
    const ASTUnaryOpNode *UOP;
  };

  ASTType EType;
  ASTOpType OTy;

private:
  ASTComplexExpressionNode() = delete;

public:
  static const unsigned ExpressionBits = 64U;

public:
  ASTComplexExpressionNode(const ASTIdentifierNode *Id,
                           const ASTBinaryOpNode *BOp)
      : ASTExpressionNode(Id, ASTTypeComplexExpression), BOP(BOp),
        EType(BOp->GetASTType()), OTy(BOp->GetOpType()) {}

  ASTComplexExpressionNode(const ASTIdentifierNode *Id,
                           const ASTUnaryOpNode *UOp)
      : ASTExpressionNode(Id, ASTTypeComplexExpression), UOP(UOp),
        EType(UOp->GetASTType()), OTy(UOp->GetOpType()) {}

  ASTComplexExpressionNode(const ASTIdentifierNode *Id,
                           const ASTBinaryOpNode *LHS,
                           const ASTBinaryOpNode *RHS, ASTOpType OT)
      : ASTExpressionNode(Id, ASTTypeComplexExpression),
        BOP(new ASTBinaryOpNode(Id, LHS, RHS, OT)), EType(BOP->GetASTType()),
        OTy(OT) {}

  ASTComplexExpressionNode(const ASTIdentifierNode *Id,
                           const ASTBinaryOpNode *LHS,
                           const ASTUnaryOpNode *RHS, ASTOpType OT)
      : ASTExpressionNode(Id, ASTTypeComplexExpression),
        BOP(new ASTBinaryOpNode(Id, LHS, RHS, OT)), EType(BOP->GetASTType()),
        OTy(OT) {}

  ASTComplexExpressionNode(const ASTIdentifierNode *Id,
                           const ASTUnaryOpNode *LHS,
                           const ASTBinaryOpNode *RHS, ASTOpType OT)
      : ASTExpressionNode(Id, ASTTypeComplexExpression),
        BOP(new ASTBinaryOpNode(Id, LHS, RHS, OT)), EType(BOP->GetASTType()),
        OTy(OT) {}

  ASTComplexExpressionNode(const ASTIdentifierNode *Id,
                           const ASTUnaryOpNode *LHS, const ASTUnaryOpNode *RHS,
                           ASTOpType OT)
      : ASTExpressionNode(Id, ASTTypeComplexExpression),
        BOP(new ASTBinaryOpNode(Id, LHS, RHS, OT)), EType(BOP->GetASTType()),
        OTy(OT) {}

  ASTComplexExpressionNode(const ASTIdentifierNode *Id,
                           const ASTMPIntegerNode *LHS,
                           const ASTMPIntegerNode *RHS, ASTOpType OT)
      : ASTExpressionNode(Id, ASTTypeComplexExpression),
        BOP(new ASTBinaryOpNode(Id, LHS, RHS, OT)), EType(BOP->GetASTType()),
        OTy(OT) {}

  ASTComplexExpressionNode(const ASTIdentifierNode *Id,
                           const ASTMPDecimalNode *LHS,
                           const ASTMPDecimalNode *RHS, ASTOpType OT)
      : ASTExpressionNode(Id, ASTTypeComplexExpression),
        BOP(new ASTBinaryOpNode(Id, LHS, RHS, OT)), EType(BOP->GetASTType()),
        OTy(OT) {}

  ASTComplexExpressionNode(const ASTIdentifierNode *Id,
                           const ASTMPIntegerNode *LHS,
                           const ASTMPDecimalNode *RHS, ASTOpType OT)
      : ASTExpressionNode(Id, ASTTypeComplexExpression),
        BOP(new ASTBinaryOpNode(Id, LHS, RHS, OT)), EType(BOP->GetASTType()),
        OTy(OT) {}

  ASTComplexExpressionNode(const ASTIdentifierNode *Id,
                           const ASTMPDecimalNode *LHS,
                           const ASTMPIntegerNode *RHS, ASTOpType OT)
      : ASTExpressionNode(Id, ASTTypeComplexExpression),
        BOP(new ASTBinaryOpNode(Id, LHS, RHS, OT)), EType(BOP->GetASTType()),
        OTy(OT) {}

  virtual ~ASTComplexExpressionNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeComplexExpression;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetExpressionType() const { return EType; }

  virtual ASTType GetLeftASTType() const {
    return BOP->GetLeft()->GetASTType();
  }

  virtual ASTType GetRightASTType() const {
    return BOP->GetRight()->GetASTType();
  }

  virtual const ASTBinaryOpNode *GetBinaryOp() const {
    return EType == ASTTypeBinaryOp ? BOP : nullptr;
  }

  virtual const ASTUnaryOpNode *GetUnaryOp() const {
    return EType == ASTTypeUnaryOp ? UOP : nullptr;
  }

  virtual ASTOpType GetOpType() const { return OTy; }

  virtual void print() const override {
    std::cout << "<ComplexExpressionNode>" << std::endl;
    BOP->print();
    std::cout << "</ComplexExpressionNode>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

typedef std::variant<const ASTBinaryOpNode *, const ASTUnaryOpNode *,
                     const ASTComplexExpressionNode *>
    ASTVariantOpNode;

class ASTFunctionCallNode;

class ASTMPComplexNode : public ASTExpressionNode {
private:
  enum OpBias : unsigned {
    Exact = 0,
    Overflow = 1,
    Underflow = 2,
  };

private:
  unsigned Bits;
  unsigned Precision;
  unsigned DeclBits;
  ASTOpType OpType;
  OpBias Bias;
  mpc_t MPValue;
  const ASTComplexExpressionNode *Expr;
  const ASTFunctionCallNode *FC;
  bool NE;
  ASTType RTy;
  ASTType ITy;
  unsigned RBits;
  unsigned IBits;

public:
  static const unsigned DefaultBits = 64U;
  static const unsigned DefaultPrecision = 16U;

private:
  ASTMPComplexNode() = delete;

private:
  void SetBias(int B) {
    if (B < 0)
      Bias = Underflow;
    else if (B > 0)
      Bias = Overflow;
    else
      Bias = Exact;
  }

  void Evaluate(const ASTComplexExpressionNode *E, unsigned NumBits);

  void Evaluate(const ASTMPDecimalNode *R, const ASTMPDecimalNode *I,
                ASTOpType OT, unsigned NumBits);

  void Evaluate(const ASTMPIntegerNode *R, const ASTMPIntegerNode *I,
                ASTOpType OT, unsigned NumBits);

  void Evaluate(const ASTMPComplexNode *L, const ASTMPComplexNode *R,
                ASTOpType OT, unsigned NumBits);

  void Evaluate(const ASTMPComplexNode *L, const ASTMPDecimalNode *R,
                ASTOpType OT);

  void Evaluate(const ASTMPComplexNode *L, const ASTMPIntegerNode *R,
                ASTOpType OT);

  void Evaluate(const ASTMPComplexNode *L, const ASTFloatNode *R, ASTOpType OT);

  void Evaluate(const ASTMPComplexNode *L, const ASTIntNode *R, ASTOpType OT);

  void Evaluate(const ASTMPDecimalNode *L, const ASTMPComplexNode *R,
                ASTOpType OT);

  void Evaluate(const ASTMPIntegerNode *L, const ASTMPComplexNode *R,
                ASTOpType OT);

  void Evaluate(const ASTFloatNode *L, const ASTMPComplexNode *R, ASTOpType OT);

  void Evaluate(const ASTIntNode *L, const ASTMPComplexNode *R, ASTOpType OT);

  void Evaluate(const ASTMPDecimalNode *R, const ASTMPIntegerNode *I,
                ASTOpType OT, unsigned NumBits);

  void Evaluate(const ASTMPIntegerNode *R, const ASTMPDecimalNode *I,
                ASTOpType OT, unsigned NumBits);

  void Evaluate(const ASTFloatNode *R, const ASTFloatNode *I, ASTOpType OT,
                unsigned NumBits = 32);

  void Evaluate(const ASTDoubleNode *R, const ASTDoubleNode *I, ASTOpType OT,
                unsigned NumBits = 64);

  void Evaluate(const ASTIntNode *R, const ASTIntNode *I, ASTOpType OT,
                unsigned NumBits = 32);

  void Evaluate(const ASTDoubleNode *R, const ASTIntNode *I, ASTOpType OT,
                unsigned NumBits = 64);

  void Evaluate(const ASTIntNode *R, const ASTDoubleNode *I, ASTOpType OT,
                unsigned NumBits = 64);

  void Evaluate(const ASTFloatNode *R, const ASTIntNode *I, ASTOpType OT,
                unsigned NumBits = 32);

  void Evaluate(const ASTIntNode *R, const ASTFloatNode *I, ASTOpType OT,
                unsigned NumBits = 32);

  void Evaluate(const ASTFloatNode *R, const ASTDoubleNode *I, ASTOpType OT,
                unsigned NumBits = 64);

  void Evaluate(const ASTDoubleNode *R, const ASTFloatNode *I, ASTOpType OT,
                unsigned NumBits = 64);

  void Evaluate(const ASTIdentifierNode *RId, const ASTIdentifierNode *IId,
                ASTOpType OT, unsigned NumBits);

  void Evaluate(const ASTFloatNode *R, const ASTMPDecimalNode *I, ASTOpType OT,
                unsigned NumBits);

  void Evaluate(const ASTDoubleNode *R, const ASTMPDecimalNode *I, ASTOpType OT,
                unsigned NumBits);

  void Evaluate(const ASTIntNode *R, const ASTMPDecimalNode *I, ASTOpType OT,
                unsigned NumBits);

  void Evaluate(const ASTFloatNode *R, const ASTMPIntegerNode *I, ASTOpType OT,
                unsigned NumBits);

  void Evaluate(const ASTDoubleNode *R, const ASTMPIntegerNode *I, ASTOpType OT,
                unsigned NumBits);

  void Evaluate(const ASTIntNode *R, const ASTMPIntegerNode *I, ASTOpType OT,
                unsigned NumBits);

  void Evaluate(const ASTMPDecimalNode *R, const ASTFloatNode *I, ASTOpType OT,
                unsigned NumBits);

  void Evaluate(const ASTMPDecimalNode *R, const ASTDoubleNode *I, ASTOpType OT,
                unsigned NumBits);

  void Evaluate(const ASTMPDecimalNode *R, const ASTIntNode *I, ASTOpType OT,
                unsigned NumBits);

  void Evaluate(const ASTMPIntegerNode *R, const ASTFloatNode *I, ASTOpType OT,
                unsigned NumBits);

  void Evaluate(const ASTMPIntegerNode *R, const ASTDoubleNode *I, ASTOpType OT,
                unsigned NumBits);

  void Evaluate(const ASTMPIntegerNode *R, const ASTIntNode *I, ASTOpType OT,
                unsigned NumBits);

protected:
  explicit ASTMPComplexNode(const ASTIdentifierNode *Id, const std::string &ERM,
                            bool Unused)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        Bits(0U), Precision(0U), DeclBits(0U), OpType(ASTOpTypeNone),
        Bias(Exact), MPValue(), Expr(nullptr), FC(nullptr), NE(false),
        RTy(ASTTypeMPDecimal), ITy(ASTTypeImaginary), RBits(0U), IBits(0U) {
    (void)Unused;
  }

public:
  ASTMPComplexNode(const ASTIdentifierNode *Id,
                   const ASTComplexExpressionNode *E,
                   unsigned NumBits = DefaultBits)
      : ASTExpressionNode(Id, ASTTypeMPComplex), Bits(NumBits),
        Precision(NumBits), DeclBits(Bits),
        OpType(E ? E->GetOpType() : ASTOpTypeNone), Bias(Exact), MPValue(),
        Expr(E), FC(nullptr), NE(true), RTy(ASTTypeMPDecimal),
        ITy(ASTTypeImaginary), RBits(DefaultBits), IBits(DefaultBits) {
    Id->SetBits(NumBits);
    mpc_init2(MPValue, NumBits);
    Evaluate(E, NumBits);
  }

  ASTMPComplexNode(const ASTIdentifierNode *Id, unsigned NumBits = DefaultBits)
      : ASTExpressionNode(Id, ASTTypeMPComplex), Bits(NumBits),
        Precision(NumBits), DeclBits(Bits), OpType(ASTOpTypeNone), Bias(Exact),
        MPValue(), Expr(nullptr), FC(nullptr), NE(true), RTy(ASTTypeMPDecimal),
        ITy(ASTTypeImaginary), RBits(DefaultBits), IBits(DefaultBits) {
    Id->SetBits(NumBits);
    mpc_init2(MPValue, NumBits);
    mpc_set_nan(MPValue);
  }

  ASTMPComplexNode(const ASTIdentifierNode *Id, const ASTMPDecimalNode *R,
                   const ASTMPDecimalNode *I, ASTOpType OT,
                   unsigned NumBits = DefaultBits);

  ASTMPComplexNode(const ASTIdentifierNode *Id, const ASTMPIntegerNode *R,
                   const ASTMPIntegerNode *I, ASTOpType OT,
                   unsigned NumBits = DefaultBits);

  ASTMPComplexNode(const ASTIdentifierNode *Id, const ASTMPDecimalNode *R,
                   const ASTMPIntegerNode *I, ASTOpType OT,
                   unsigned NumBits = DefaultBits);

  ASTMPComplexNode(const ASTIdentifierNode *Id, const ASTMPIntegerNode *R,
                   const ASTMPDecimalNode *I, ASTOpType OT,
                   unsigned NumBits = DefaultBits);

  explicit ASTMPComplexNode(const ASTIdentifierNode *Id,
                            const ASTMPComplexNode &MPCC,
                            unsigned NumBits = DefaultBits);

  explicit ASTMPComplexNode(const ASTIdentifierNode *Id, const std::string &REP,
                            unsigned NumBits = DefaultBits);

  explicit ASTMPComplexNode(const ASTIdentifierNode *Id, const char *REP,
                            unsigned NumBits = DefaultBits);

  explicit ASTMPComplexNode(const ASTIdentifierNode *Id,
                            const ASTFunctionCallNode *FN,
                            unsigned NumBits = DefaultBits);

  virtual ~ASTMPComplexNode() { mpc_clear(MPValue); }

  virtual ASTType GetASTType() const override { return ASTTypeMPComplex; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTType GetRealType() const { return RTy; }

  virtual ASTType GetImaginaryType() const { return ITy; }

  virtual void Mangle() override;

  virtual void MangleLiteral();

  virtual unsigned GetBits() const { return Bits; }

  virtual unsigned GetDeclBits() const { return DeclBits; }

  virtual unsigned GetRealBits() const { return RBits; }

  virtual unsigned GetImaginaryBits() const { return IBits; }

  virtual unsigned GetPrecision() const { return Precision; }

  virtual int GetMPPrecision() const { return mpc_get_prec(MPValue); }

  virtual ASTOpType GetOpType() const { return OpType; }

  virtual void SetOpType(ASTOpType OTy) { OpType = OTy; }

  virtual bool IsExpression() const override { return Expr; }

  virtual bool IsAggregate() const override { return true; }

  virtual bool NeedsEval() const { return NE; }

  virtual const ASTExpressionNode *GetExpression() const { return Expr; }

  virtual const ASTFunctionCallNode *GetFunctionCall() const { return FC; }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const mpc_t &GetMPValue() const { return MPValue; }

  virtual std::string GetValue(int Base = 10) const {
    std::string R;

    if (char *C = mpc_get_str(Base, 0, MPValue, MPC_RNDND)) {
      R = C;
      mpc_free_str(C);
    }

    return R;
  }

  virtual bool GetRealAsMPFR(mpfr_t &MPR) const {
    mpfr_init2(MPR, Bits);
    mpfr_set_d(MPR, 0.0, MPFR_RNDN);

    if (mpc_real(MPR, MPValue, MPFR_RNDN) != 0) {
      mpfr_set_nan(MPR);
      return false;
    }

    return true;
  }

  virtual ASTMPDecimalNode *GetRealAsMPDecimal() const {
    mpfr_t MPR;
    mpfr_init2(MPR, Bits);
    mpfr_set_d(MPR, 0.0, MPFR_RNDN);

    if (mpc_real(MPR, MPValue, MPFR_RNDN) != 0)
      mpfr_set_nan(MPR);

    ASTMPDecimalNode *MPD =
        new ASTMPDecimalNode(GetIdentifier(), GetBits(), MPR);
    assert(MPD && "Could not create a valid ASTMPDecimalNode!");

    mpfr_clear(MPR);
    return MPD;
  }

  virtual bool GetImagAsMPFR(mpfr_t &MPR) const {
    mpfr_init2(MPR, Bits);
    mpfr_set_d(MPR, 0.0, MPFR_RNDN);

    if (mpc_imag(MPR, MPValue, MPFR_RNDN) != 0) {
      mpfr_set_nan(MPR);
      return false;
    }

    return true;
  }

  virtual ASTMPDecimalNode *GetImagAsMPDecimal() const {
    mpfr_t MPR;
    mpfr_init2(MPR, Bits);
    mpfr_set_d(MPR, 0.0, MPFR_RNDN);

    if (mpc_imag(MPR, MPValue, MPFR_RNDN) != 0)
      mpfr_set_nan(MPR);

    ASTMPDecimalNode *MPD =
        new ASTMPDecimalNode(GetIdentifier(), GetBits(), MPR);
    assert(MPD && "Could not create a valid ASTMPDecimalNode!");

    mpfr_clear(MPR);
    return MPD;
  }

  virtual OpBias GetBias() const { return Bias; }

  virtual std::string GetBiasAsString() const {
    switch (Bias) {
    case Exact:
      return "Exact";
      break;
    case Underflow:
      return "Underflow";
      break;
    case Overflow:
      return "Overflow";
      break;
    default:
      return "Exact";
      break;
    }
  }

  virtual bool IsNan() const {
    mpfr_t MPR;
    mpfr_t MPI;

    if (!GetRealAsMPFR(MPR))
      return true;

    if (!GetImagAsMPFR(MPI))
      return true;

    bool R = ((mpfr_nan_p(MPR) != 0) || (mpfr_nan_p(MPI) != 0));

    mpfr_clear(MPR);
    mpfr_clear(MPI);
    return R;
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTMPComplexNode *ExpressionError(const ASTIdentifierNode *Id,
                                           const std::string &ERM) {
    return new ASTMPComplexNode(Id, ERM, true);
  }

  static ASTMPComplexNode *ExpressionError(const std::string &ERM) {
    return new ASTMPComplexNode(ASTIdentifierNode::MPComplex.Clone(), ERM);
  }

  virtual void CReal(const ASTIntNode *I);

  virtual void CReal(const ASTFloatNode *F);

  virtual void CReal(const ASTDoubleNode *D);

  virtual void CReal(const ASTMPIntegerNode *MPI);

  virtual void CReal(const ASTMPDecimalNode *MPD);

  virtual void CImag(const ASTIntNode *I);

  virtual void CImag(const ASTFloatNode *F);

  virtual void CImag(const ASTDoubleNode *D);

  virtual void CImag(const ASTMPIntegerNode *MPI);

  virtual void CImag(const ASTMPDecimalNode *MPD);

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTAssignmentNode : public ASTBinaryOpNode {
private:
  ASTAssignmentNode() = delete;

public:
  ASTAssignmentNode(const ASTIdentifierNode *Identifier,
                    const ASTExpressionNode *LHS, const ASTExpressionNode *RHS)
      : ASTBinaryOpNode(Identifier, LHS, RHS, ASTOpTypeAssign) {}

  virtual ~ASTAssignmentNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeAssignment; }

  virtual void print() const override {
    std::cout << "<AssignmentNode>" << std::endl;
    ASTBinaryOpNode::print();
    std::cout << "</AssignmentNode>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTComparisonNode : public ASTBinaryOpNode {
private:
  ASTComparisonNode() = delete;

public:
  ASTComparisonNode(const ASTIdentifierNode *Identifier,
                    const ASTExpressionNode *LHS, const ASTExpressionNode *RHS)
      : ASTBinaryOpNode(Identifier, LHS, RHS, ASTOpTypeCompEq) {}

  virtual ~ASTComparisonNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeComparison; }

  virtual void print() const override {
    std::cout << "<ComparisonNode>" << std::endl;
    ASTBinaryOpNode::print();
    std::cout << "</ComparisonNode>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTIfElseNode : public ASTStatement {
private:
  ASTType AType;
  ASTSemaType SType;
  std::list<ASTBase *> Graph;

public:
  ASTIfElseNode()
      : ASTStatement(), AType(ASTTypeIfElse), SType(SemaTypeStatement),
        Graph() {}

  virtual ~ASTIfElseNode() = default;

  virtual ASTType GetASTType() const override { return AType; }

  virtual ASTSemaType GetSemaType() const { return SType; }

  virtual void print() const override {
    for (std::list<ASTBase *>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I)
      (*I)->print();
  }

  virtual void push(ASTBase *Node) override { Graph.push_back(Node); }
};

class ASTStatementNode : public ASTStatement {
protected:
  const ASTIdentifierNode *Ident;
  const ASTExpressionNode *Expr;
  mutable const ASTDeclarationContext *DC;

protected:
  ASTStatementNode()
      : ASTStatement(), Ident(ASTIdentifierNode::StatementIdentifier()),
        Expr(nullptr), DC(nullptr) {
    DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
    DC->RegisterSymbol(this, GetASTType());
  }

public:
  ASTStatementNode(const ASTIdentifierNode *Id)
      : ASTStatement(), Ident(Id), Expr(nullptr), DC(nullptr) {
    DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
    DC->RegisterSymbol(this, GetASTType());
  }

  ASTStatementNode(const ASTExpressionNode *E)
      : ASTStatement(), Ident(E->Ident), Expr(E), DC(nullptr) {
    DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
    DC->RegisterSymbol(this, GetASTType());
  }

  ASTStatementNode(const ASTIdentifierNode *Id, const ASTExpressionNode *E)
      : Ident(Id), Expr(E), DC(nullptr) {
    DC = ASTDeclarationContextTracker::Instance().GetCurrentContext();
    DC->RegisterSymbol(this, GetASTType());
  }

  virtual ~ASTStatementNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeStatement; }

  virtual ASTSemaType GetSemaType() const { return SemaTypeStatement; }

  virtual void Mangle() {}

  virtual const ASTIdentifierNode *GetIdentifier() const {
    return Ident ? Ident : Expr ? Expr->GetIdentifier() : nullptr;
  }

  virtual const std::string &GetName() const {
    return Ident  ? Ident->GetName()
           : Expr ? Expr->GetIdentifier()->GetName()
                  : ASTIdentifierNode::Statement.GetName();
  }

  virtual const std::string &GetMangledName() const {
    return Ident  ? Ident->GetMangledName()
           : Expr ? Expr->GetIdentifier()->GetMangledName()
                  : ASTIdentifierNode::Statement.GetName();
  }

  virtual bool Skip() const override { return false; }

  virtual bool IsExpression() const { return Expr; }

  virtual const ASTExpressionNode *GetExpression() const { return Expr; }

  virtual ASTExpressionNode *GetExpression() {
    return const_cast<ASTExpressionNode *>(Expr);
  }

  virtual const ASTDeclarationContext *GetDeclarationContext() const {
    return DC;
  }

  virtual unsigned GetContextIndex() const { return DC->GetIndex(); }

  virtual void SetDeclarationContext(const ASTDeclarationContext *DCX) {
    if (DC)
      DC->UnregisterSymbol(this);

    DC = DCX;
    DC->RegisterSymbol(this, GetASTType());
  }

  virtual void SetDeclarationContext(const ASTDeclarationContext *DCX) const {
    if (DC)
      DC->UnregisterSymbol(this);

    DC = DCX;
    DC->RegisterSymbol(this, GetASTType());
  }

  virtual bool IsDeclaration() const { return false; }

  virtual bool IsDirective() const { return false; }

  virtual bool IsError() const {
    if (const ASTExpressionNode *EN =
            dynamic_cast<const ASTExpressionNode *>(Expr))
      return EN->IsError();

    return false;
  }

  virtual const std::string &GetError() const {
    if (const ASTExpressionNode *EN =
            dynamic_cast<const ASTExpressionNode *>(Expr))
      return EN->GetError();

    return ASTStringUtils::Instance().EmptyString();
  }

  static ASTStatementNode *StatementError(const ASTIdentifierNode *Id) {
    ASTExpressionNode *ER =
        ASTExpressionNode::ExpressionError(Id, Id->GetExpression());
    assert(ER && "Could not create a valid ASTExpressionNode!");

    ASTStatementNode *SR = new ASTStatementNode(Id, ER);
    assert(SR && "Could not create a valid ASTStatementNode!");

    SR->SetLocation(Id->GetLocation());
    return SR;
  }

  static ASTStatementNode *StatementError(const ASTIdentifierNode *Id,
                                          const std::string &ERM) {
    ASTExpressionNode *ER = ASTExpressionNode::ExpressionError(Id, ERM);
    assert(ER && "Could not create a valid ASTExpressionNode!");

    ASTStatementNode *SR = new ASTStatementNode(Id, ER);
    assert(SR && "Could not create a valid ASTStatementNode!");

    SR->SetLocation(Id->GetLocation());
    return SR;
  }

  static ASTStatementNode *StatementError(const ASTIdentifierRefNode *IdR,
                                          const std::string &ERM) {
    ASTExpressionNode *ER = ASTExpressionNode::ExpressionError(IdR, ERM);
    assert(ER && "Could not create a valid ASTExpressionNode!");

    ASTStatementNode *SR = new ASTStatementNode(IdR, ER);
    assert(SR && "Could not create a valid ASTStatementNode!");

    SR->SetLocation(IdR->GetLocation());
    return SR;
  }

  static ASTStatementNode *StatementError(const ASTIdentifierNode *Id,
                                          const ASTSyntaxErrorNode *SYX) {
    ASTStatementNode *SR = new ASTStatementNode(Id, SYX);
    assert(SR && "Could not create a valid ASTStatementNode!");

    SR->SetLocation(Id->GetLocation());
    return SR;
  }

  virtual void print() const override {
    std::cout << "<StatementNode>" << std::endl;
    if (Ident)
      Ident->print();
    if (Expr)
      Expr->print();
    std::cout << "</StatementNode>" << std::endl;
  }
};

class ASTBinaryOpStatementNode : public ASTStatementNode {
private:
  ASTBinaryOpStatementNode() = delete;

public:
  ASTBinaryOpStatementNode(const ASTBinaryOpNode *BOp)
      : ASTStatementNode(BOp->GetIdentifier(), BOp) {}

  virtual ~ASTBinaryOpStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeBinaryOpStatement;
  }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual const ASTBinaryOpNode *GetBinaryOp() const {
    return dynamic_cast<const ASTBinaryOpNode *>(this->GetExpression());
  }

  virtual ASTBinaryOpNode *GetBinaryOp() {
    return dynamic_cast<ASTBinaryOpNode *>(this->GetExpression());
  }

  virtual void print() const override {
    std::cout << "<BinaryOpStatementNode>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</BinaryOpStatementNode>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTUnaryOpStatementNode : public ASTStatementNode {
private:
  ASTUnaryOpStatementNode() = delete;

public:
  ASTUnaryOpStatementNode(const ASTUnaryOpNode *UOp)
      : ASTStatementNode(UOp->GetIdentifier(), UOp) {}

  virtual ~ASTUnaryOpStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeUnaryOpStatement;
  }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual const ASTUnaryOpNode *GetUnaryOp() const {
    return dynamic_cast<const ASTUnaryOpNode *>(this->GetExpression());
  }

  virtual ASTUnaryOpNode *GetUnaryOp() {
    return dynamic_cast<ASTUnaryOpNode *>(this->GetExpression());
  }

  virtual void print() const override {
    std::cout << "<UnaryOpStatementNode>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</UnaryOpStatementNode>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTGateNode;
class ASTGateControlNode;
class ASTGateNegControlNode;
class ASTGatePowerNode;
class ASTGateInverseNode;
class ASTGPhaseExpressionNode;
class ASTGateGPhaseExpressionNode;
class ASTDefcalNode;
class ASTDefcalGroupNode;

class ASTGateOpNode : public ASTStatementNode {
protected:
  bool IsDefcal;
  ASTType MTy;
  ASTType OTy;

protected:
  ASTGateOpNode()
      : ASTStatementNode(), IsDefcal(false), MTy(ASTTypeUndefined),
        OTy(ASTTypeUndefined) {}

  ASTGateOpNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)),
        IsDefcal(false), MTy(ASTTypeExpressionError),
        OTy(ASTTypeExpressionError) {}

public:
  static const unsigned GateOpBits = 64U;

public:
  ASTGateOpNode(const ASTIdentifierNode *Id)
      : ASTStatementNode(Id), IsDefcal(false), MTy(ASTTypeUndefined),
        OTy(ASTTypeUndefined) {}

  ASTGateOpNode(const ASTIdentifierNode *Id, const ASTGateNode *GN);

  ASTGateOpNode(const ASTIdentifierNode *Id, const ASTDefcalNode *DN);

  ASTGateOpNode(const ASTIdentifierNode *Id, const ASTDefcalGroupNode *DGN);

  explicit ASTGateOpNode(const ASTIdentifierNode *Id,
                         const ASTGPhaseExpressionNode *GPE);

  explicit ASTGateOpNode(const ASTIdentifierNode *Id,
                         const ASTGateControlNode *GCN);

  explicit ASTGateOpNode(const ASTIdentifierNode *Id,
                         const ASTGateNegControlNode *GNCN);

  explicit ASTGateOpNode(const ASTIdentifierNode *Id,
                         const ASTGatePowerNode *GPN);

  explicit ASTGateOpNode(const ASTIdentifierNode *Id,
                         const ASTGateInverseNode *GIN);

  explicit ASTGateOpNode(const ASTIdentifierNode *Id,
                         const ASTGateGPhaseExpressionNode *GGEN);

  virtual ~ASTGateOpNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGateOpNode; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override = 0;

  virtual bool IsDefcalOp() const { return IsDefcal; }

  virtual bool HasModifier() const;

  virtual ASTType GetModifierType() const { return MTy; }

  virtual ASTType GetOpType() const { return OTy; }

  virtual void SetModifier(const ASTGateControlNode *N);

  virtual void SetModifier(const ASTGateNegControlNode *N);

  virtual void SetModifier(const ASTGatePowerNode *N);

  virtual void SetModifier(const ASTGateInverseNode *N);

  const ASTGateControlNode *GetGateControlModifier() const;

  const ASTGateNegControlNode *GetGateNegControlModifier() const;

  const ASTGatePowerNode *GetPowerModifier() const;

  const ASTGateInverseNode *GetInverseModifier() const;

  virtual bool IsError() const override { return ASTStatementNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTGateQOpNode : public ASTGateOpNode {
protected:
  ASTGateQOpNode() : ASTGateOpNode() {}

  ASTGateQOpNode(const ASTIdentifierNode *Id) : ASTGateOpNode(Id) {}

  ASTGateQOpNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTGateOpNode(Id, ERM) {}

  virtual void Mangle(ASTGateQOpNode *G);

public:
  ASTGateQOpNode(const ASTIdentifierNode *Id, const ASTGateNode *GN)
      : ASTGateOpNode(Id, GN) {}

  ASTGateQOpNode(const ASTIdentifierNode *Id, const ASTDefcalNode *DN)
      : ASTGateOpNode(Id, DN) {}

  ASTGateQOpNode(const ASTIdentifierNode *Id, const ASTDefcalGroupNode *DGN)
      : ASTGateOpNode(Id, DGN) {}

  explicit ASTGateQOpNode(const ASTIdentifierNode *Id,
                          const ASTGPhaseExpressionNode *GPE)
      : ASTGateOpNode(Id, GPE) {}

  explicit ASTGateQOpNode(const ASTGPhaseExpressionNode *GPE);

  explicit ASTGateQOpNode(const ASTIdentifierNode *Id,
                          const ASTGateControlNode *GCN)
      : ASTGateOpNode(Id, GCN) {}

  explicit ASTGateQOpNode(const ASTGateControlNode *GCN);

  explicit ASTGateQOpNode(const ASTIdentifierNode *Id,
                          const ASTGateNegControlNode *GNCN)
      : ASTGateOpNode(Id, GNCN) {}

  explicit ASTGateQOpNode(const ASTGateNegControlNode *GNCN);

  explicit ASTGateQOpNode(const ASTIdentifierNode *Id,
                          const ASTGatePowerNode *GPN)
      : ASTGateOpNode(Id, GPN) {}

  explicit ASTGateQOpNode(const ASTGatePowerNode *GPN);

  explicit ASTGateQOpNode(const ASTIdentifierNode *Id,
                          const ASTGateInverseNode *GIN)
      : ASTGateOpNode(Id, GIN) {}

  explicit ASTGateQOpNode(const ASTGateInverseNode *GIN);

  explicit ASTGateQOpNode(const ASTIdentifierNode *Id,
                          const ASTGateGPhaseExpressionNode *GGEN)
      : ASTGateOpNode(Id, GGEN) {}

  explicit ASTGateQOpNode(const ASTGateGPhaseExpressionNode *GGEN);

  virtual ~ASTGateQOpNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGateQOpNode; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetOpASTType() const {
    return ASTStatementNode::Expr->GetASTType();
  }

  virtual ASTType GetTargetType() const {
    return ASTStatementNode::Expr->GetASTType();
  }

  virtual const ASTIdentifierNode *GetOperand() const {
    return ASTStatementNode::Ident;
  }

  virtual const ASTGateNode *GetGateNode() const;

  virtual ASTGateNode *GetGateNode();

  virtual const ASTDefcalNode *GetDefcalNode() const;

  virtual ASTDefcalNode *GetDefcalNode();

  virtual const ASTDefcalGroupNode *GetDefcalGroupNode() const;

  virtual ASTDefcalGroupNode *GetDefcalGroupNode();

  virtual const ASTGPhaseExpressionNode *GetGPhaseNode() const;

  virtual ASTGPhaseExpressionNode *GetGPhaseNode();

  virtual const ASTGateGPhaseExpressionNode *GetGateGPhaseNode() const;

  virtual ASTGateGPhaseExpressionNode *GetGateGPhaseNode();

  virtual const ASTGateControlNode *GetGateControlNode() const;

  virtual ASTGateControlNode *GetGateControlNode();

  virtual const ASTGateNegControlNode *GetGateNegControlNode() const;

  virtual ASTGateNegControlNode *GetGateNegControlNode();

  virtual const ASTGatePowerNode *GetGatePowerNode() const;

  virtual ASTGatePowerNode *GetGatePowerNode();

  virtual const ASTGateInverseNode *GetGateInverseNode() const;

  virtual ASTGateInverseNode *GetGateInverseNode();

  virtual bool IsError() const override { return ASTStatementNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTGateQOpNode *StatementError(const ASTIdentifierNode *Id,
                                        const std::string &ERM) {
    return new ASTGateQOpNode(Id, ERM);
  }

  static ASTGateQOpNode *StatementError(const std::string &ERM) {
    return new ASTGateQOpNode(ASTIdentifierNode::GateQOp.Clone(), ERM);
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTUGateOpNode : public ASTGateQOpNode {
private:
  ASTUGateOpNode() = delete;

public:
  ASTUGateOpNode(const ASTIdentifierNode *Id, const ASTGateNode *GN)
      : ASTGateQOpNode(Id, GN) {}

  ASTUGateOpNode(const ASTIdentifierNode *Id, const ASTDefcalNode *DN)
      : ASTGateQOpNode(Id, DN) {}

  virtual ~ASTUGateOpNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGateUOpNode; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTGenericGateOpNode : public ASTUGateOpNode {
private:
  ASTGenericGateOpNode() = delete;

public:
  ASTGenericGateOpNode(const ASTIdentifierNode *Id, const ASTGateNode *GN)
      : ASTUGateOpNode(Id, GN) {}

  ASTGenericGateOpNode(const ASTIdentifierNode *Id, const ASTDefcalNode *DN)
      : ASTUGateOpNode(Id, DN) {}

  virtual ~ASTGenericGateOpNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeGateGenericOpNode;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTHGateOpNode : public ASTUGateOpNode {
private:
  ASTHGateOpNode() = delete;

public:
  ASTHGateOpNode(const ASTIdentifierNode *Id, const ASTGateNode *GN)
      : ASTUGateOpNode(Id, GN) {}

  virtual ~ASTHGateOpNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGateHOpNode; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTCXGateOpNode : public ASTUGateOpNode {
private:
  ASTCXGateOpNode() = delete;

public:
  ASTCXGateOpNode(const ASTIdentifierNode *Id, const ASTGateNode *GN)
      : ASTUGateOpNode(Id, GN) {}

  virtual ~ASTCXGateOpNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeCXGateOpNode; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTCCXGateOpNode : public ASTUGateOpNode {
private:
  ASTCCXGateOpNode() = delete;

public:
  ASTCCXGateOpNode(const ASTIdentifierNode *Id, const ASTGateNode *GN)
      : ASTUGateOpNode(Id, GN) {}

  virtual ~ASTCCXGateOpNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeCCXGateOpNode; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTCNotGateOpNode : public ASTUGateOpNode {
private:
  ASTCNotGateOpNode() = delete;

public:
  ASTCNotGateOpNode(const ASTIdentifierNode *Id, const ASTGateNode *GN)
      : ASTUGateOpNode(Id, GN) {}

  virtual ~ASTCNotGateOpNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeCNotGateOpNode; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTGateControlNode;
class ASTGateNegControlNode;
class ASTGatePowerNode;
class ASTGateInverseNode;
class ASTGPhaseExpressionNode;
class ASTGateGPhaseExpressionNode;

class ASTGateControlNode : public ASTExpressionNode {
private:
  // Targets.
  union {
    mutable const ASTGateNode *GN;
    mutable const ASTGateQOpNode *GQN;
    mutable const ASTGPhaseExpressionNode *GPN;
    mutable const ASTGateGPhaseExpressionNode *GGEN;
    mutable const ASTGateControlNode *CN;
    mutable const ASTGateNegControlNode *NCN;
    mutable const ASTGatePowerNode *PN;
    mutable const ASTGateInverseNode *IN;
  };

  ASTType TType;

  // Modifiers (if any).
  union {
    mutable const void *MV;
    mutable const ASTGateControlNode *MCN;
    mutable const ASTGateNegControlNode *MNCN;
    mutable const ASTGatePowerNode *MPN;
    mutable const ASTGateInverseNode *MIN;
  };

  mutable ASTType MType;
  unsigned CBits;

private:
  ASTGateControlNode() = delete;

protected:
  ASTGateControlNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        GN(nullptr), TType(ASTTypeExpressionError), MV(nullptr),
        MType(ASTTypeExpressionError), CBits(static_cast<unsigned>(~0x0)) {}

public:
  static const unsigned GateControlBits = 64U;

public:
  ASTGateControlNode(const ASTGateNode *N);

  ASTGateControlNode(const ASTGateNode *N, const ASTIntNode *CB);

  ASTGateControlNode(const ASTGateQOpNode *QN);

  ASTGateControlNode(const ASTGateQOpNode *QN, const ASTIntNode *CB);

  ASTGateControlNode(const ASTGateControlNode *N);

  ASTGateControlNode(const ASTGateControlNode *N, const ASTIntNode *CB);

  ASTGateControlNode(const ASTGateNegControlNode *N);

  ASTGateControlNode(const ASTGateNegControlNode *N, const ASTIntNode *CB);

  ASTGateControlNode(const ASTGatePowerNode *N);

  ASTGateControlNode(const ASTGatePowerNode *N, const ASTIntNode *CB);

  ASTGateControlNode(const ASTGateInverseNode *N);

  ASTGateControlNode(const ASTGateInverseNode *N, const ASTIntNode *CB);

  ASTGateControlNode(const ASTGPhaseExpressionNode *N);

  ASTGateControlNode(const ASTGPhaseExpressionNode *N, const ASTIntNode *CB);

  ASTGateControlNode(const ASTGateGPhaseExpressionNode *N);

  ASTGateControlNode(const ASTGateGPhaseExpressionNode *N,
                     const ASTIntNode *CB);

  virtual ~ASTGateControlNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGateControl; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetTargetType() const { return TType; }

  virtual ASTType GetModifierType() const { return MType; }

  virtual unsigned GetControlBits() const { return CBits; }

  virtual const ASTGateNode *GetGateNode() const {
    return TType == ASTTypeGate ? GN : nullptr;
  }

  virtual ASTGateNode *GetGateNode() {
    return TType == ASTTypeGate ? const_cast<ASTGateNode *>(GN) : nullptr;
  }

  virtual const ASTGateQOpNode *GetGateQOpNode() const {
    return TType == ASTTypeGateQOpNode ? GQN : nullptr;
  }

  virtual ASTGateQOpNode *GetGateQOpNode() {
    return TType == ASTTypeGateQOpNode ? const_cast<ASTGateQOpNode *>(GQN)
                                       : nullptr;
  }

  virtual const ASTGPhaseExpressionNode *GetGPhaseNode() const {
    return TType == ASTTypeGPhaseExpression ? GPN : nullptr;
  }

  virtual ASTGPhaseExpressionNode *GetGPhaseNode() {
    return TType == ASTTypeGPhaseExpression
               ? const_cast<ASTGPhaseExpressionNode *>(GPN)
               : nullptr;
  }

  virtual const ASTGateGPhaseExpressionNode *GetGateGPhaseNode() const {
    return TType == ASTTypeGateGPhaseExpression ? GGEN : nullptr;
  }

  virtual ASTGateGPhaseExpressionNode *GetGateGPhaseNode() {
    return TType == ASTTypeGateGPhaseExpression
               ? const_cast<ASTGateGPhaseExpressionNode *>(GGEN)
               : nullptr;
  }

  virtual const ASTGateControlNode *GetControlNode() const {
    return TType == ASTTypeGateControl ? CN : nullptr;
  }

  virtual ASTGateControlNode *GetControlNode() {
    return TType == ASTTypeGateControl ? const_cast<ASTGateControlNode *>(CN)
                                       : nullptr;
  }

  virtual const ASTGateNegControlNode *GetNegControlNode() const {
    return TType == ASTTypeGateNegControl ? NCN : nullptr;
  }

  virtual ASTGateNegControlNode *GetNegControlNode() {
    return TType == ASTTypeGateNegControl
               ? const_cast<ASTGateNegControlNode *>(NCN)
               : nullptr;
  }

  virtual const ASTGatePowerNode *GetPowerNode() const {
    return TType == ASTTypeGatePower ? PN : nullptr;
  }

  virtual ASTGatePowerNode *GetPowerNode() {
    return TType == ASTTypeGatePower ? const_cast<ASTGatePowerNode *>(PN)
                                     : nullptr;
  }

  virtual const ASTGateInverseNode *GetInverseNode() const {
    return TType == ASTTypeGateInverse ? IN : nullptr;
  }

  virtual ASTGateInverseNode *GetInverseNode() {
    return TType == ASTTypeGateInverse ? const_cast<ASTGateInverseNode *>(IN)
                                       : nullptr;
  }

  virtual void SetModifier(const ASTGateControlNode *N) const {
    assert(N && "Invalid ASTGateControlNode argument!");

    MCN = N;
    MType = ASTTypeGateControl;
  }

  virtual void SetModifier(const ASTGateNegControlNode *N) const {
    assert(N && "Invalid ASTGateNegControlNode argument!");

    MNCN = N;
    MType = ASTTypeGateNegControl;
  }

  virtual void SetModifier(const ASTGatePowerNode *N) const {
    assert(N && "Invalid ASTGatePowerNode argument!");

    MPN = N;
    MType = ASTTypeGatePower;
  }

  virtual void SetModifier(const ASTGateInverseNode *N) const {
    assert(N && "Invalid ASTGateInverseNode argument!");

    MIN = N;
    MType = ASTTypeGateInverse;
  }

  virtual bool HasModifier() const { return MV != nullptr; }

  const ASTGateControlNode *GetGateControlModifier() const {
    return MType == ASTTypeGateControl ? MCN : nullptr;
  }

  const ASTGateNegControlNode *GetGateNegControlModifier() const {
    return MType == ASTTypeGateNegControl ? MNCN : nullptr;
  }

  const ASTGatePowerNode *GetPowerModifier() const {
    return MType == ASTTypeGatePower ? MPN : nullptr;
  }

  const ASTGateInverseNode *GetInverseModifier() const {
    return MType == ASTTypeGateInverse ? MIN : nullptr;
  }

  virtual ASTGateNode *Resolve();

  virtual bool IsError() const override {
    return MType == ASTTypeExpressionError || TType == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTGateControlNode *ExpressionError(const std::string &ERM) {
    return new ASTGateControlNode(ASTIdentifierNode::BadCtrl.Clone(), ERM);
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTGateNegControlNode : public ASTExpressionNode {
private:
  // Targets.
  union {
    mutable const ASTGateNode *GN;
    mutable const ASTGateQOpNode *GQN;
    mutable const ASTGPhaseExpressionNode *GPN;
    mutable const ASTGateGPhaseExpressionNode *GGEN;
    mutable const ASTGateControlNode *CN;
    mutable const ASTGateNegControlNode *NCN;
    mutable const ASTGatePowerNode *PN;
    mutable const ASTGateInverseNode *IN;
  };

  ASTType TType;

  // Modifiers (if any).
  union {
    mutable const void *MV;
    mutable const ASTGateControlNode *MCN;
    mutable const ASTGateNegControlNode *MNCN;
    mutable const ASTGatePowerNode *MPN;
    mutable const ASTGateInverseNode *MIN;
  };

  mutable ASTType MType;
  unsigned CBits;

private:
  ASTGateNegControlNode() = delete;

protected:
  ASTGateNegControlNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        GN(nullptr), TType(ASTTypeExpressionError), MV(nullptr),
        MType(ASTTypeExpressionError), CBits(static_cast<unsigned>(~0x0)) {}

public:
  static const unsigned GateNegControlBits = 64U;

public:
  ASTGateNegControlNode(const ASTGateNode *N);

  ASTGateNegControlNode(const ASTGateNode *N, const ASTIntNode *CB);

  ASTGateNegControlNode(const ASTGateQOpNode *QN);

  ASTGateNegControlNode(const ASTGateQOpNode *QN, const ASTIntNode *CB);

  ASTGateNegControlNode(const ASTGateControlNode *N);

  ASTGateNegControlNode(const ASTGateControlNode *N, const ASTIntNode *CB);

  ASTGateNegControlNode(const ASTGateNegControlNode *N);

  ASTGateNegControlNode(const ASTGateNegControlNode *N, const ASTIntNode *CB);

  ASTGateNegControlNode(const ASTGatePowerNode *N);

  ASTGateNegControlNode(const ASTGatePowerNode *N, const ASTIntNode *CB);

  ASTGateNegControlNode(const ASTGateInverseNode *N);

  ASTGateNegControlNode(const ASTGateInverseNode *N, const ASTIntNode *CB);

  ASTGateNegControlNode(const ASTGPhaseExpressionNode *N);

  ASTGateNegControlNode(const ASTGPhaseExpressionNode *N, const ASTIntNode *CB);

  ASTGateNegControlNode(const ASTGateGPhaseExpressionNode *N);

  ASTGateNegControlNode(const ASTGateGPhaseExpressionNode *N,
                        const ASTIntNode *CB);

  virtual ~ASTGateNegControlNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGateNegControl; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetTargetType() const { return TType; }

  virtual ASTType GetModifierType() const { return MType; }

  virtual unsigned GetControlBits() const { return CBits; }

  virtual const ASTGateNode *GetGateNode() const { return GN; }

  virtual ASTGateNode *GetGateNode() { return const_cast<ASTGateNode *>(GN); }

  virtual const ASTGateQOpNode *GetGateQOpNode() const {
    return TType == ASTTypeGateQOpNode ? GQN : nullptr;
  }

  virtual ASTGateQOpNode *GetGateQOpNode() {
    return TType == ASTTypeGateQOpNode ? const_cast<ASTGateQOpNode *>(GQN)
                                       : nullptr;
  }

  virtual const ASTGPhaseExpressionNode *GetGPhaseNode() const {
    return TType == ASTTypeGPhaseExpression ? GPN : nullptr;
  }

  virtual ASTGPhaseExpressionNode *GetGPhaseNode() {
    return TType == ASTTypeGPhaseExpression
               ? const_cast<ASTGPhaseExpressionNode *>(GPN)
               : nullptr;
  }

  virtual const ASTGateGPhaseExpressionNode *GetGateGPhaseNode() const {
    return TType == ASTTypeGateGPhaseExpression ? GGEN : nullptr;
  }

  virtual ASTGateGPhaseExpressionNode *GetGateGPhaseNode() {
    return TType == ASTTypeGateGPhaseExpression
               ? const_cast<ASTGateGPhaseExpressionNode *>(GGEN)
               : nullptr;
  }

  virtual const ASTGateControlNode *GetControlNode() const {
    return TType == ASTTypeGateControl ? CN : nullptr;
  }

  virtual ASTGateControlNode *GetControlNode() {
    return TType == ASTTypeGateControl ? const_cast<ASTGateControlNode *>(CN)
                                       : nullptr;
  }

  virtual const ASTGateNegControlNode *GetNegControlNode() const {
    return TType == ASTTypeGateNegControl ? NCN : nullptr;
  }

  virtual ASTGateNegControlNode *GetNegControlNode() {
    return TType == ASTTypeGateNegControl
               ? const_cast<ASTGateNegControlNode *>(NCN)
               : nullptr;
  }

  virtual const ASTGatePowerNode *GetPowerNode() const {
    return TType == ASTTypeGatePower ? PN : nullptr;
  }

  virtual ASTGatePowerNode *GetPowerNode() {
    return TType == ASTTypeGatePower ? const_cast<ASTGatePowerNode *>(PN)
                                     : nullptr;
  }

  virtual const ASTGateInverseNode *GetInverseNode() const {
    return TType == ASTTypeGateInverse ? IN : nullptr;
  }

  virtual ASTGateInverseNode *GetInverseNode() {
    return TType == ASTTypeGateInverse ? const_cast<ASTGateInverseNode *>(IN)
                                       : nullptr;
  }

  virtual void SetModifier(const ASTGateControlNode *N) const {
    assert(N && "Invalid ASTGateControlNode argument!");

    MCN = N;
    MType = ASTTypeGateControl;
  }

  virtual void SetModifier(const ASTGateNegControlNode *N) const {
    assert(N && "Invalid ASTGateNegControlNode argument!");

    MNCN = N;
    MType = ASTTypeGateNegControl;
  }

  virtual void SetModifier(const ASTGatePowerNode *N) const {
    assert(N && "Invalid ASTGatePowerNode argument!");

    MPN = N;
    MType = ASTTypeGatePower;
  }

  virtual void SetModifier(const ASTGateInverseNode *N) const {
    assert(N && "Invalid ASTGateInverseNode argument!");

    MIN = N;
    MType = ASTTypeGateInverse;
  }

  virtual bool HasModifier() const { return MV != nullptr; }

  const ASTGateControlNode *GetGateControlModifier() const {
    return MType == ASTTypeGateControl ? MCN : nullptr;
  }

  const ASTGateNegControlNode *GetGateNegControlModifier() const {
    return MType == ASTTypeGateNegControl ? MNCN : nullptr;
  }

  const ASTGatePowerNode *GetPowerModifier() const {
    return MType == ASTTypeGatePower ? MPN : nullptr;
  }

  const ASTGateInverseNode *GetInverseModifier() const {
    return MType == ASTTypeGateInverse ? MIN : nullptr;
  }

  virtual ASTGateNode *Resolve();

  virtual bool IsError() const override {
    return MType == ASTTypeExpressionError || TType == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTGateNegControlNode *ExpressionError(const std::string &ERM) {
    return new ASTGateNegControlNode(ASTIdentifierNode::BadCtrl.Clone(), ERM);
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTGateInverseNode : public ASTExpressionNode {
private:
  // Targets.
  union {
    mutable const ASTGateNode *GN;
    mutable const ASTGateQOpNode *GQN;
    mutable const ASTGPhaseExpressionNode *GPN;
    mutable const ASTGateGPhaseExpressionNode *GGEN;
    mutable const ASTGateControlNode *CN;
    mutable const ASTGateNegControlNode *NCN;
    mutable const ASTGatePowerNode *PN;
    mutable const ASTGateInverseNode *IN;
  };

  ASTType TType;

  // Modifiers (if any).
  union {
    mutable const void *MV;
    mutable const ASTGateControlNode *MCN;
    mutable const ASTGateNegControlNode *MNCN;
    mutable const ASTGatePowerNode *MPN;
    mutable const ASTGateInverseNode *MIN;
  };

  mutable ASTType MType;

private:
  ASTGateInverseNode() = delete;

protected:
  ASTGateInverseNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        GN(nullptr), TType(ASTTypeExpressionError), MV(nullptr),
        MType(ASTTypeExpressionError) {}

public:
  ASTGateInverseNode(const ASTGateNode *N);

  ASTGateInverseNode(const ASTGateQOpNode *QN);

  ASTGateInverseNode(const ASTGateControlNode *N);

  ASTGateInverseNode(const ASTGateNegControlNode *N);

  ASTGateInverseNode(const ASTGatePowerNode *N);

  ASTGateInverseNode(const ASTGateInverseNode *N);

  ASTGateInverseNode(const ASTGPhaseExpressionNode *N);

  ASTGateInverseNode(const ASTGateGPhaseExpressionNode *N);

  virtual ~ASTGateInverseNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGateInverse; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetTargetType() const { return TType; }

  virtual ASTType GetModifierType() const { return MType; }

  virtual const ASTGateNode *GetGateNode() const { return GN; }

  virtual ASTGateNode *GetGateNode() { return const_cast<ASTGateNode *>(GN); }

  virtual const ASTGateQOpNode *GetGateQOpNode() const {
    return TType == ASTTypeGateQOpNode ? GQN : nullptr;
  }

  virtual ASTGateQOpNode *GetGateQOpNode() {
    return TType == ASTTypeGateQOpNode ? const_cast<ASTGateQOpNode *>(GQN)
                                       : nullptr;
  }

  virtual const ASTGPhaseExpressionNode *GetGPhaseNode() const {
    return TType == ASTTypeGPhaseExpression ? GPN : nullptr;
  }

  virtual ASTGPhaseExpressionNode *GetGPhaseNode() {
    return TType == ASTTypeGPhaseExpression
               ? const_cast<ASTGPhaseExpressionNode *>(GPN)
               : nullptr;
  }

  virtual const ASTGateGPhaseExpressionNode *GetGateGPhaseNode() const {
    return TType == ASTTypeGateGPhaseExpression ? GGEN : nullptr;
  }

  virtual ASTGateGPhaseExpressionNode *GetGateGPhaseNode() {
    return TType == ASTTypeGateGPhaseExpression
               ? const_cast<ASTGateGPhaseExpressionNode *>(GGEN)
               : nullptr;
  }

  virtual const ASTGateControlNode *GetControlNode() const {
    return TType == ASTTypeGateControl ? CN : nullptr;
  }

  virtual ASTGateControlNode *GetControlNode() {
    return TType == ASTTypeGateControl ? const_cast<ASTGateControlNode *>(CN)
                                       : nullptr;
  }

  virtual const ASTGateNegControlNode *GetNegControlNode() const {
    return TType == ASTTypeGateNegControl ? NCN : nullptr;
  }

  virtual ASTGateNegControlNode *GetNegControlNode() {
    return TType == ASTTypeGateNegControl
               ? const_cast<ASTGateNegControlNode *>(NCN)
               : nullptr;
  }

  virtual const ASTGatePowerNode *GetPowerNode() const {
    return TType == ASTTypeGatePower ? PN : nullptr;
  }

  virtual ASTGatePowerNode *GetPowerNode() {
    return TType == ASTTypeGatePower ? const_cast<ASTGatePowerNode *>(PN)
                                     : nullptr;
  }

  virtual const ASTGateInverseNode *GetInverseNode() const {
    return TType == ASTTypeGateInverse ? IN : nullptr;
  }

  virtual ASTGateInverseNode *GetInverseNode() {
    return TType == ASTTypeGateInverse ? const_cast<ASTGateInverseNode *>(IN)
                                       : nullptr;
  }

  virtual void SetModifier(const ASTGateControlNode *N) const {
    assert(N && "Invalid ASTGateControlNode argument!");

    MCN = N;
    MType = ASTTypeGateControl;
  }

  virtual void SetModifier(const ASTGateNegControlNode *N) const {
    assert(N && "Invalid ASTGateNegControlNode argument!");

    MNCN = N;
    MType = ASTTypeGateNegControl;
  }

  virtual void SetModifier(const ASTGatePowerNode *N) const {
    assert(N && "Invalid ASTGatePowerNode argument!");

    MPN = N;
    MType = ASTTypeGatePower;
  }

  virtual void SetModifier(const ASTGateInverseNode *N) const {
    assert(N && "Invalid ASTGateInverseNode argument!");

    MIN = N;
    MType = ASTTypeGateInverse;
  }

  virtual bool HasModifier() const { return MV != nullptr; }

  const ASTGateControlNode *GetGateControlModifier() const {
    return MType == ASTTypeGateControl ? MCN : nullptr;
  }

  const ASTGateNegControlNode *GetGateNegControlModifier() const {
    return MType == ASTTypeGateNegControl ? MNCN : nullptr;
  }

  const ASTGatePowerNode *GetPowerModifier() const {
    return MType == ASTTypeGatePower ? MPN : nullptr;
  }

  const ASTGateInverseNode *GetInverseModifier() const {
    return MType == ASTTypeGateInverse ? MIN : nullptr;
  }

  virtual ASTGateNode *Resolve();

  virtual bool IsError() const override {
    return MType == ASTTypeExpressionError || TType == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTGateInverseNode *ExpressionError(const std::string &ERM) {
    return new ASTGateInverseNode(ASTIdentifierNode::BadCtrl.Clone(), ERM);
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTGatePowerNode : public ASTExpressionNode {
private:
  // Exponent.
  union {
    const ASTIntNode *I;
    const ASTIdentifierNode *ID;
    const ASTBinaryOpNode *BOP;
    const ASTUnaryOpNode *UOP;
  };

  // Targets.
  union {
    mutable const ASTGateNode *GN;
    mutable const ASTGateQOpNode *GQN;
    mutable const ASTGPhaseExpressionNode *GPN;
    mutable const ASTGateGPhaseExpressionNode *GGEN;
    mutable const ASTGateControlNode *CN;
    mutable const ASTGateNegControlNode *NCN;
    mutable const ASTGatePowerNode *PN;
    mutable const ASTGateInverseNode *IN;
  };

  ASTType EType;
  ASTType TType;

  // Modifiers (if any).
  union {
    mutable const void *MV;
    mutable const ASTGateControlNode *MCN;
    mutable const ASTGateNegControlNode *MNCN;
    mutable const ASTGatePowerNode *MPN;
    mutable const ASTGateInverseNode *MIN;
  };

  mutable ASTType MType;

private:
  ASTGatePowerNode() = delete;

protected:
  ASTGatePowerNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        I(nullptr), GN(nullptr), EType(ASTTypeExpressionError),
        TType(ASTTypeExpressionError), MV(nullptr),
        MType(ASTTypeExpressionError) {}

public:
  ASTGatePowerNode(const ASTGateNode *N, const ASTIntNode *Exp);

  ASTGatePowerNode(const ASTGateNode *N, const ASTIdentifierNode *Exp);

  ASTGatePowerNode(const ASTGateNode *N, const ASTBinaryOpNode *Exp);

  ASTGatePowerNode(const ASTGateNode *N, const ASTUnaryOpNode *Exp);

  ASTGatePowerNode(const ASTGateQOpNode *QN);

  ASTGatePowerNode(const ASTGateQOpNode *QN, const ASTIntNode *Exp);

  ASTGatePowerNode(const ASTGateQOpNode *QN, const ASTIdentifierNode *Exp);

  ASTGatePowerNode(const ASTGateQOpNode *QN, const ASTBinaryOpNode *Exp);

  ASTGatePowerNode(const ASTGateQOpNode *QN, const ASTUnaryOpNode *Exp);

  ASTGatePowerNode(const ASTGateControlNode *N);

  ASTGatePowerNode(const ASTGateControlNode *N, const ASTIntNode *Exp);

  ASTGatePowerNode(const ASTGateControlNode *N, const ASTIdentifierNode *Exp);

  ASTGatePowerNode(const ASTGateControlNode *N, const ASTBinaryOpNode *Exp);

  ASTGatePowerNode(const ASTGateControlNode *N, const ASTUnaryOpNode *Exp);

  ASTGatePowerNode(const ASTGateNegControlNode *N);

  ASTGatePowerNode(const ASTGateNegControlNode *N, const ASTIntNode *Exp);

  ASTGatePowerNode(const ASTGateNegControlNode *N,
                   const ASTIdentifierNode *Exp);

  ASTGatePowerNode(const ASTGateNegControlNode *N, const ASTBinaryOpNode *Exp);

  ASTGatePowerNode(const ASTGateNegControlNode *N, const ASTUnaryOpNode *Exp);

  ASTGatePowerNode(const ASTGatePowerNode *N);

  ASTGatePowerNode(const ASTGatePowerNode *N, const ASTIntNode *Exp);

  ASTGatePowerNode(const ASTGatePowerNode *N, const ASTIdentifierNode *Exp);

  ASTGatePowerNode(const ASTGatePowerNode *N, const ASTBinaryOpNode *Exp);

  ASTGatePowerNode(const ASTGatePowerNode *N, const ASTUnaryOpNode *Exp);

  ASTGatePowerNode(const ASTGateInverseNode *N);

  ASTGatePowerNode(const ASTGateInverseNode *N, const ASTIntNode *Exp);

  ASTGatePowerNode(const ASTGateInverseNode *N, const ASTIdentifierNode *Exp);

  ASTGatePowerNode(const ASTGateInverseNode *N, const ASTBinaryOpNode *Exp);

  ASTGatePowerNode(const ASTGateInverseNode *N, const ASTUnaryOpNode *Exp);

  ASTGatePowerNode(const ASTGPhaseExpressionNode *N);

  ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N);

  ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N, const ASTIntNode *Exp);

  ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N,
                   const ASTIdentifierNode *Exp);

  ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N,
                   const ASTBinaryOpNode *Exp);

  ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N,
                   const ASTUnaryOpNode *Exp);

  virtual ~ASTGatePowerNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGatePower; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetExponentType() const { return EType; }

  virtual ASTType GetTargetType() const { return TType; }

  virtual ASTType GetModifierType() const { return MType; }

  virtual const ASTGateNode *GetGateNode() const { return GN; }

  virtual ASTGateNode *GetGateNode() { return const_cast<ASTGateNode *>(GN); }

  virtual const ASTGateQOpNode *GetGateQOpNode() const {
    return TType == ASTTypeGateQOpNode ? GQN : nullptr;
  }

  virtual ASTGateQOpNode *GetGateQOpNode() {
    return TType == ASTTypeGateQOpNode ? const_cast<ASTGateQOpNode *>(GQN)
                                       : nullptr;
  }

  virtual const ASTIntNode *GetExponent() const {
    return EType == ASTTypeInt ? I : nullptr;
  }

  virtual const ASTBinaryOpNode *GetExponentExpr() const {
    return EType == ASTTypeBinaryOp ? BOP : nullptr;
  }

  virtual const ASTGPhaseExpressionNode *GetGPhaseNode() const {
    return TType == ASTTypeGPhaseExpression ? GPN : nullptr;
  }

  virtual ASTGPhaseExpressionNode *GetGPhaseNode() {
    return TType == ASTTypeGPhaseExpression
               ? const_cast<ASTGPhaseExpressionNode *>(GPN)
               : nullptr;
  }

  virtual const ASTGateGPhaseExpressionNode *GetGateGPhaseNode() const {
    return TType == ASTTypeGateGPhaseExpression ? GGEN : nullptr;
  }

  virtual ASTGateGPhaseExpressionNode *GetGateGPhaseNode() {
    return TType == ASTTypeGateGPhaseExpression
               ? const_cast<ASTGateGPhaseExpressionNode *>(GGEN)
               : nullptr;
  }

  virtual const ASTGateControlNode *GetControlNode() const {
    return TType == ASTTypeGateControl ? CN : nullptr;
  }

  virtual ASTGateControlNode *GetControlNode() {
    return TType == ASTTypeGateControl ? const_cast<ASTGateControlNode *>(CN)
                                       : nullptr;
  }

  virtual const ASTGateNegControlNode *GetNegControlNode() const {
    return TType == ASTTypeGateNegControl ? NCN : nullptr;
  }

  virtual ASTGateNegControlNode *GetNegControlNode() {
    return TType == ASTTypeGateNegControl
               ? const_cast<ASTGateNegControlNode *>(NCN)
               : nullptr;
  }

  virtual const ASTGatePowerNode *GetPowerNode() const {
    return TType == ASTTypeGatePower ? PN : nullptr;
  }

  virtual ASTGatePowerNode *GetPowerNode() {
    return TType == ASTTypeGatePower ? const_cast<ASTGatePowerNode *>(PN)
                                     : nullptr;
  }

  virtual const ASTGateInverseNode *GetInverseNode() const {
    return TType == ASTTypeGateInverse ? IN : nullptr;
  }

  virtual ASTGateInverseNode *GetInverseNode() {
    return TType == ASTTypeGateInverse ? const_cast<ASTGateInverseNode *>(IN)
                                       : nullptr;
  }

  virtual void SetModifier(const ASTGateControlNode *N) const {
    assert(N && "Invalid ASTGateControlNode argument!");

    MCN = N;
    MType = ASTTypeGateControl;
  }

  virtual void SetModifier(const ASTGateNegControlNode *N) const {
    assert(N && "Invalid ASTGateNegControlNode argument!");

    MNCN = N;
    MType = ASTTypeGateNegControl;
  }

  virtual void SetModifier(const ASTGatePowerNode *N) const {
    assert(N && "Invalid ASTGatePowerNode argument!");

    MPN = N;
    MType = ASTTypeGatePower;
  }

  virtual void SetModifier(const ASTGateInverseNode *N) const {
    assert(N && "Invalid ASTGateInverseNode argument!");

    MIN = N;
    MType = ASTTypeGateInverse;
  }

  virtual bool HasModifier() const { return MV != nullptr; }

  const ASTGateControlNode *GetGateControlModifier() const {
    return MType == ASTTypeGateControl ? MCN : nullptr;
  }

  const ASTGateNegControlNode *GetGateNegControlModifier() const {
    return MType == ASTTypeGateNegControl ? MNCN : nullptr;
  }

  const ASTGatePowerNode *GetPowerModifier() const {
    return MType == ASTTypeGatePower ? MPN : nullptr;
  }

  const ASTGateInverseNode *GetInverseModifier() const {
    return MType == ASTTypeGateInverse ? MIN : nullptr;
  }

  virtual std::vector<ASTGateNode *> Resolve();

  virtual bool IsError() const override {
    return MType == ASTTypeExpressionError || EType == ASTTypeExpressionError ||
           TType == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTGatePowerNode *ExpressionError(const std::string &ERM) {
    return new ASTGatePowerNode(ASTIdentifierNode::BadCtrl.Clone(), ERM);
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTDirtyNode : public ASTStatementNode {
private:
  std::list<ASTBase *> Graph;

public:
  ASTDirtyNode(const ASTIdentifierNode *Id) : ASTStatementNode(Id), Graph() {}

  virtual ~ASTDirtyNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeDirty; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void print() const override {
    for (std::list<ASTBase *>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I)
      (*I)->print();
  }

  virtual void push(ASTBase *Node) override { Graph.push_back(Node); }
};

class ASTOpaqueNode : public ASTStatementNode {
private:
  std::list<ASTBase *> Graph;

public:
  ASTOpaqueNode(const ASTIdentifierNode *Id) : ASTStatementNode(Id), Graph() {}

  virtual ~ASTOpaqueNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeOpaque; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void print() const override {
    for (std::list<ASTBase *>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I)
      (*I)->print();
  }

  virtual void push(ASTBase *Node) override { Graph.push_back(Node); }
};

class ASTInputModifierNode : public ASTExpressionNode {
private:
  static ASTInputModifierNode IM;

protected:
  ASTInputModifierNode()
      : ASTExpressionNode(&ASTIdentifierNode::Input, ASTTypeInputModifier) {}

public:
  static ASTInputModifierNode *Instance() { return &IM; }

  virtual ~ASTInputModifierNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeInputModifier; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void print() const override {
    std::cout << "<InputModifier></InputModiifer>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTOutputModifierNode : public ASTExpressionNode {
private:
  static ASTOutputModifierNode OM;

protected:
  ASTOutputModifierNode()
      : ASTExpressionNode(&ASTIdentifierNode::Output, ASTTypeOutputModifier) {}

public:
  static ASTOutputModifierNode *Instance() { return &OM; }

  virtual ~ASTOutputModifierNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeOutputModifier; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void print() const override {
    std::cout << "<OutputModifier></OutputModiifer>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTDeclarationNode : public ASTStatementNode {
private:
  ASTDeclarationNode() = delete;

protected:
  ASTType Type;
  uint64_t PO;
  bool IsTypeDecl;
  bool IsNamedTypeDecl;
  bool AllowDuplicates;

  union {
    const void *VM;
    const ASTInputModifierNode *IM;
    const ASTOutputModifierNode *OM;
  };

  ASTType MType;

public:
  ASTDeclarationNode(const ASTIdentifierNode *Id, ASTType Ty,
                     bool TypeDecl = false)
      : ASTStatementNode(Id), Type(Ty), PO(static_cast<uint64_t>(~0x0UL)),
        IsTypeDecl(TypeDecl), IsNamedTypeDecl(false), AllowDuplicates(false),
        VM(nullptr), MType(ASTTypeUndefined) {
    AllowDuplicates = Ty == ASTTypeGate || Ty == ASTTypeCNotGate ||
                      Ty == ASTTypeHadamardGate || Ty == ASTTypeCCXGate ||
                      Ty == ASTTypeCXGate || Ty == ASTTypeUGate ||
                      Ty == ASTTypeDefcal || Ty == ASTTypeOpaque ||
                      Ty == ASTTypeEllipsis || Ty == ASTTypeVoid;
  }

  ASTDeclarationNode(const ASTIdentifierNode *Id, const ASTExpressionNode *EX,
                     ASTType Ty, bool NamedTypeDecl = true,
                     bool TypeDecl = false)
      : ASTStatementNode(Id, EX), Type(Ty), PO(static_cast<uint64_t>(~0x0UL)),
        IsTypeDecl(TypeDecl), IsNamedTypeDecl(NamedTypeDecl),
        AllowDuplicates(false), VM(nullptr), MType(ASTTypeUndefined) {
    AllowDuplicates = Ty == ASTTypeGate || Ty == ASTTypeCNotGate ||
                      Ty == ASTTypeHadamardGate || Ty == ASTTypeCCXGate ||
                      Ty == ASTTypeCXGate || Ty == ASTTypeUGate ||
                      Ty == ASTTypeDefcal || Ty == ASTTypeOpaque ||
                      Ty == ASTTypeEllipsis || Ty == ASTTypeVoid;
  }

  virtual ~ASTDeclarationNode() = default;

  virtual ASTType GetASTType() const override { return Type; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeDeclaration;
  }

  virtual const std::string &GetName() const override {
    return GetIdentifier()->GetName();
  }

  virtual const std::string &GetMangledName() const override {
    return GetIdentifier()->GetMangledName();
  }

  virtual bool IsTypeDeclaration() const { return IsTypeDecl; }

  virtual bool IsNamedTypeDeclaration() const { return IsNamedTypeDecl; }

  virtual bool DuplicatesAllowed() const { return AllowDuplicates; }

  virtual bool IsDeclaration() const override { return true; }

  virtual uint64_t GetParameterOrder() const { return PO; }

  virtual bool HasModifier() const { return MType != ASTTypeUndefined; }

  virtual ASTType GetModifierType() const { return MType; }

  virtual void SetModifier(const ASTInputModifierNode *M) {
    IM = M;
    MType = M->GetASTType();
  }
  virtual void SetModifier(const ASTOutputModifierNode *M) {
    OM = M;
    MType = M->GetASTType();
  }

  virtual void SetParameterOrder(uint64_t V) { PO = V; }

  virtual void SetConst(bool V = true) {
    if (IsExpression())
      ASTStatementNode::GetExpression()->SetConst(V);
  }

  virtual void SetConst(bool V = true) const {
    if (IsExpression())
      ASTStatementNode::GetExpression()->SetConst(V);
  }

  virtual bool IsConst() const {
    return IsExpression() && ASTStatementNode::GetExpression()->IsConst();
  }

  virtual bool IsError() const override {
    return Type == ASTTypeDeclarationError;
  }

  virtual const std::string &GetError() const override {
    if (Type == ASTTypeDeclarationError) {
      if (const ASTStringNode *SN =
              dynamic_cast<const ASTStringNode *>(GetExpression()))
        return SN->GetValue();
    }

    return ASTStringUtils::Instance().EmptyString();
  }

  static ASTDeclarationNode *DeclarationError(const ASTIdentifierNode *Id) {
    ASTDeclarationNode *DR = new ASTDeclarationNode(Id, Id->GetExpression(),
                                                    ASTTypeDeclarationError);
    assert(DR && "Could not create a valid ASTDeclarationNode!");
    DR->SetLocation(Id->GetLocation());
    return DR;
  }

  static ASTDeclarationNode *DeclarationError(const ASTIdentifierNode *Id,
                                              const std::string &ERM) {
    ASTDeclarationNode *DR = new ASTDeclarationNode(Id, new ASTStringNode(ERM),
                                                    ASTTypeDeclarationError);
    assert(DR && "Could not create a valid ASTDeclarationNode!");
    DR->SetLocation(Id->GetLocation());
    return DR;
  }

  virtual void print() const override {
    std::cout << "<Declaration>" << std::endl;
    std::cout << "<Identifier>" << this->GetName() << "</Identifier>"
              << std::endl;
    std::cout << "<MangledName>" << this->GetMangledName() << "</MangledName>"
              << std::endl;
    std::cout << "<Type>" << QASM::PrintTypeEnum(Type) << "</Type>"
              << std::endl;

    if (PO != static_cast<uint64_t>(~0x0UL))
      std::cout << "<ParameterOrder>" << PO << "</ParameterOrder>" << std::endl;
    std::cout << "<IsTypeDeclaration>" << std::boolalpha << IsTypeDecl
              << "</IsTypeDeclaration>" << std::endl;
    std::cout << "<IsNamedTypeDeclaration>" << std::boolalpha << IsNamedTypeDecl
              << "</IsNamedTypeDeclaration>" << std::endl;
    std::cout << "<IsConst>" << std::boolalpha << IsConst() << "</IsConst>"
              << std::endl;

    std::cout << "<Modifier>" << std::endl;
    if (HasModifier()) {
      if (MType == ASTTypeInputModifier)
        IM->print();
      else if (MType == ASTTypeOutputModifier)
        OM->print();
    }
    std::cout << "</Modifier>" << std::endl;

    ASTStatementNode::print();
    std::cout << "</Declaration>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTOpaqueDeclarationNode : public ASTStatementNode {
private:
  ASTOpaqueDeclarationNode() = delete;

protected:
  std::list<ASTOpaqueDeclarationNode *> Graph;

public:
  ASTOpaqueDeclarationNode(const ASTIdentifierNode *Id)
      : ASTStatementNode(Id), Graph() {}

  virtual ~ASTOpaqueDeclarationNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeOpaqueDeclaration;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeDeclaration;
  }

  virtual void print() const override {
    for (std::list<ASTOpaqueDeclarationNode *>::const_iterator I =
             Graph.begin();
         I != Graph.end(); ++I)
      (*I)->print();
  }

  virtual void push(ASTBase *Node) override {
    Graph.push_back(dynamic_cast<ASTOpaqueDeclarationNode *>(Node));
  }
};

class ASTImplicitConversionNode;

class ASTAngleNode : public ASTExpressionNode {
  friend class ASTBuilder;
  friend class ASTTypeSystemBuilder;

protected:
  static std::map<std::string, ASTAngleType> ATM;

protected:
  unsigned Bits;
  mpfr_t MPValue;
  union {
    const ASTIntNode *I;
    const ASTFloatNode *F;
    const ASTDoubleNode *D;
    const ASTLongDoubleNode *LD;
    const ASTMPIntegerNode *MPI;
    const ASTMPDecimalNode *MPD;
    const ASTBinaryOpNode *BOP;
    const ASTUnaryOpNode *UOP;
    const ASTExpressionNode *EX;
    const ASTIdentifierNode *ID;
  };

  // [P0][P1][P2][P3]
  std::array<uint64_t, 4> IR;
  std::array<const ASTIdentifierRefNode *, 4> IIR;
  mutable std::bitset<64> BST;
  std::string GateParamName;
  ASTNumericConstant NC;
  std::size_t Hash;
  const ASTImplicitConversionNode *ICE;
  ASTType ExprType;
  ASTAngleType AngleType;

protected:
  virtual void AddPointElement(const ASTIdentifierRefNode *IId, unsigned IX) {
    assert(IX < 4 && "Array index is out-of-range!");
    IIR[IX] = IId;
  }

  virtual void AddPointElement(uint64_t PV, unsigned IX) {
    assert(IX < 4 && "Array index is out-of-range!");
    IR[IX] = PV;
  }

  ASTAngleNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        Bits(0U), MPValue(), I(nullptr), IR(), IIR(), BST(), GateParamName(),
        NC(CNone), Hash(0UL), ICE(nullptr), ExprType(ASTTypeExpressionError),
        AngleType(ASTAngleTypeGeneric) {
    IR.fill(0UL);
  }

private:
  ASTAngleNode() = delete;

public:
  static const unsigned AngleBits = 64U;

public:
  ASTAngleNode(const ASTIdentifierNode *Id,
               ASTAngleType ATy = ASTAngleTypeGeneric,
               unsigned NumBits = ASTAngleNode::AngleBits)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(),
        I(nullptr), IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL),
        ICE(nullptr), ExprType(ASTTypeUndefined), AngleType(ATy) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    mpfr_set_nan(MPValue);
    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  ASTAngleNode(const ASTIdentifierNode *Id, unsigned NumBits, const mpfr_t &MPV,
               ASTAngleType ATy = ASTAngleTypeGeneric)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(),
        I(nullptr), IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL),
        ICE(nullptr), ExprType(ASTTypeMPDecimal), AngleType(ATy) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    // MPFR_RNDN == round-to-nearest. This is IEEE-754 compliant.
    if (mpfr_set(MPValue, MPV, MPFR_RNDN) != 0) {
      mpfr_set_d(MPValue, 0.0, MPFR_RNDD);
      Bits = 0;
    }

    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  ASTAngleNode(const ASTIdentifierNode *Id, unsigned NumBits,
               const std::string &String,
               ASTAngleType ATy = ASTAngleTypeGeneric, int Base = 10)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(),
        I(nullptr), IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL),
        ICE(nullptr), ExprType(ASTTypeMPDecimal), AngleType(ATy) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    // MPFR_RNDN == round-to-nearest. This is IEEE-754 compliant.
    if (mpfr_set_str(MPValue, String.c_str(), Base, MPFR_RNDN) != 0) {
      mpfr_set_d(MPValue, 0.0, MPFR_RNDN);
      Bits = 0;
    }

    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  explicit ASTAngleNode(const ASTIdentifierNode *Id, uint64_t W, uint64_t X,
                        uint64_t Y, uint64_t Z,
                        ASTAngleType ATy = ASTAngleTypeGeneric,
                        unsigned NumBits = ASTAngleNode::AngleBits)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(),
        I(nullptr), IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL),
        ICE(nullptr), ExprType(ASTTypeArray), AngleType(ATy) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    mpfr_set_nan(MPValue);
    Hash = std::hash<std::string>{}(Id->GetName());
    IR[0] = W;
    IR[1] = X;
    IR[2] = Y;
    IR[3] = Z;
    IIR.fill(nullptr);
  }

  explicit ASTAngleNode(const ASTIdentifierNode *Id, const ASTBinaryOpNode *E,
                        ASTAngleType ATy,
                        unsigned NumBits = ASTAngleNode::AngleBits)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(), BOP(E),
        IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL), ICE(nullptr),
        ExprType(E->GetASTType()), AngleType(ATy) {
    assert(E && "Invalid ASTBinaryOpNode argument!");
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    mpfr_set_nan(MPValue);
    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  explicit ASTAngleNode(const ASTIdentifierNode *Id, const ASTUnaryOpNode *E,
                        ASTAngleType ATy,
                        unsigned NumBits = ASTAngleNode::AngleBits)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(), UOP(E),
        IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL), ICE(nullptr),
        ExprType(E->GetASTType()), AngleType(ATy) {
    assert(E && "Invalid ASTUnaryOpNode argument!");
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    mpfr_set_nan(MPValue);
    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  explicit ASTAngleNode(const ASTIdentifierNode *Id, const ASTMPIntegerNode *E,
                        ASTAngleType ATy,
                        unsigned NumBits = ASTAngleNode::AngleBits)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(), MPI(E),
        IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL), ICE(nullptr),
        ExprType(E->GetASTType()), AngleType(ATy) {
    assert(E && "Invalid ASTMPIntegerNode argument!");
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    // MPFR_RNDN == round to nearest.
    mpfr_set_z(MPValue, E->GetMPValue(), MPFR_RNDN);
    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  explicit ASTAngleNode(const ASTIdentifierNode *Id, const ASTMPDecimalNode *E,
                        ASTAngleType ATy,
                        unsigned NumBits = ASTAngleNode::AngleBits)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(), MPD(E),
        IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL), ICE(nullptr),
        ExprType(E->GetASTType()), AngleType(ATy) {
    assert(E && "Invalid ASTMPDecimalNode argument!");
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    // MPFR_RNDN == round to nearest.
    mpfr_set(MPValue, E->GetMPValue(), MPFR_RNDN);
    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  explicit ASTAngleNode(const ASTIdentifierNode *Id, const ASTIntNode *E,
                        ASTAngleType ATy,
                        unsigned NumBits = ASTAngleNode::AngleBits)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(), I(E),
        IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL), ICE(nullptr),
        ExprType(E->GetASTType()), AngleType(ATy) {
    assert(E && "Invalid ASTIntNode argument!");
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    // MPFR_RNDN == round to nearest.
    mpfr_set_d(MPValue,
               E->IsSigned() ? static_cast<double>(E->GetSignedValue())
                             : static_cast<double>(E->GetUnsignedValue()),
               MPFR_RNDN);
    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  explicit ASTAngleNode(const ASTIdentifierNode *Id, const ASTFloatNode *E,
                        ASTAngleType ATy,
                        unsigned NumBits = ASTAngleNode::AngleBits)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(), F(E),
        IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL), ICE(nullptr),
        ExprType(E->GetASTType()), AngleType(ATy) {
    assert(E && "Invalid ASTFloatNode argument!");
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    // MPFR_RNDN == round to nearest.
    mpfr_set_flt(MPValue, E->GetValue(), MPFR_RNDN);
    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  explicit ASTAngleNode(const ASTIdentifierNode *Id, const ASTDoubleNode *E,
                        ASTAngleType ATy,
                        unsigned NumBits = ASTAngleNode::AngleBits)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(), D(E),
        IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL), ICE(nullptr),
        ExprType(E->GetASTType()), AngleType(ATy) {
    assert(E && "Invalid ASTDoubleNode argument!");
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    // MPFR_RNDN == round to nearest.
    mpfr_set_d(MPValue, E->GetValue(), MPFR_RNDN);
    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  explicit ASTAngleNode(const ASTIdentifierNode *Id, const ASTLongDoubleNode *E,
                        ASTAngleType ATy, unsigned NumBits = 128U)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(), LD(E),
        IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL), ICE(nullptr),
        ExprType(E->GetASTType()), AngleType(ATy) {
    assert(E && "Invalid ASTDoubleNode argument!");
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    // MPFR_RNDN == round to nearest.
    mpfr_set_ld(MPValue, E->GetValue(), MPFR_RNDN);
    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  explicit ASTAngleNode(const ASTIdentifierNode *Id, const ASTExpressionNode *E,
                        ASTAngleType ATy,
                        unsigned NumBits = ASTAngleNode::AngleBits)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(), EX(E),
        IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL), ICE(nullptr),
        ExprType(E->GetASTType()), AngleType(ATy) {
    assert(E && "Invalid ASTDoubleNode argument!");
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    mpfr_set_nan(MPValue);
    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  explicit ASTAngleNode(const ASTIdentifierNode *Id, double DD,
                        ASTAngleType ATy,
                        unsigned NumBits = ASTAngleNode::AngleBits)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(),
        D(nullptr), IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL),
        ICE(nullptr), ExprType(ASTTypeDouble), AngleType(ATy) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    // MPFR_RNDN == round to nearest.
    mpfr_set_d(MPValue, DD, MPFR_RNDN);
    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  explicit ASTAngleNode(const ASTIdentifierNode *Id, long double LLD,
                        ASTAngleType ATy,
                        unsigned NumBits = ASTAngleNode::AngleBits)
      : ASTExpressionNode(Id, ASTTypeAngle), Bits(NumBits), MPValue(),
        D(nullptr), IR(), IIR(), BST(), GateParamName(), NC(CNone), Hash(0UL),
        ICE(nullptr), ExprType(ASTTypeDouble), AngleType(ATy) {
    Id->SetBits(NumBits);
    mpfr_init2(MPValue, Bits);
    // MPFR_RNDN == round to nearest.
    mpfr_set_ld(MPValue, LLD, MPFR_RNDN);
    Hash = std::hash<std::string>{}(Id->GetName());
    IR.fill(0UL);
    IIR.fill(nullptr);
  }

  virtual ~ASTAngleNode() { mpfr_clear(MPValue); }

  virtual ASTAngleNode *Clone(const ASTIdentifierNode *Id) const {
    ASTAngleNode *R =
        new ASTAngleNode(Id, GetBits(), GetMPValue(), GetAngleType());
    assert(R && "Could not create a valid ASTAngleNode clone!");

    R->ExprType = ExprType;
    R->EX = EX;
    R->IR = IR;
    R->IIR = IIR;
    R->BST = BST;
    R->GateParamName = GateParamName;
    R->NC = NC;
    R->ICE = ICE;
    return R;
  }

  virtual ASTAngleNode *Clone(const ASTIdentifierNode *Id,
                              unsigned NumBits) const {
    ASTAngleNode *R =
        new ASTAngleNode(Id, NumBits, GetMPValue(), GetAngleType());
    assert(R && "Could not create a valid ASTAngleNode clone!");

    R->ExprType = ExprType;
    R->EX = EX;
    R->IR = IR;
    R->IIR = IIR;
    R->BST = BST;
    R->GateParamName = GateParamName;
    R->NC = NC;
    R->ICE = ICE;
    return R;
  }

  virtual void Clone(ASTAngleNode *R) const {
    assert(R && "Invalid ASTAngleNode argument!");

    R->ExprType = ExprType;
    R->EX = EX;
    R->IR = IR;
    R->IIR = IIR;
    R->BST = BST;
    R->GateParamName = GateParamName;
    R->NC = NC;
    R->ICE = ICE;
    mpfr_set(R->MPValue, MPValue, MPFR_RNDN);
  }

  virtual ASTType GetASTType() const override { return ASTTypeAngle; }

  virtual ASTType GetExprType() const { return ExprType; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTNumericConstant GetNumericConstant() const { return NC; }

  // Implemented in ASTAngleNodeBuilder.cpp.
  virtual void Mangle() override;

  // Implemented in ASTAngleNodeBuilder.cpp.
  virtual void MangleLiteral();

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual const std::string &GetMangledName() const override {
    return ASTExpressionNode::Ident->GetMangledName();
  }

  virtual const std::string &GetMangledLiteralName() const override {
    return ASTExpressionNode::Ident->GetMangledLiteralName();
  }

  virtual const std::string &GetGateParamName() const { return GateParamName; }

  virtual std::size_t GetHash() const { return Hash; }

  virtual const ASTImplicitConversionNode *GetImplicitConversion() const {
    return ICE;
  }

  virtual const std::array<uint64_t, 4> &GetIR() const { return IR; }

  virtual bool IsExpression() const override {
    return ExprType != ASTTypeInt && ExprType != ASTTypeFloat &&
           ExprType != ASTTypeDouble && ExprType != ASTTypeLongDouble &&
           ExprType != ASTTypeMPInteger && ExprType != ASTTypeMPDecimal;
  }

  virtual bool IsValue() const { return !IsExpression(); }

  virtual const ASTExpressionNode *GetExpression() const {
    switch (ExprType) {
    case ASTTypeInt:
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeLongDouble:
    case ASTTypeMPInteger:
    case ASTTypeMPDecimal:
      return nullptr;
      break;
    case ASTTypeUnaryOp:
      return UOP;
      break;
    case ASTTypeBinaryOp:
      return BOP;
      break;
    default:
      return EX;
      break;
    }
  }

  virtual const ASTIdentifierNode *GetValueIdentifier() const {
    switch (ExprType) {
    case ASTTypeIdentifier:
      return ID;
      break;
    default:
      return nullptr;
      break;
    }
  }

  static ASTAngleType DetermineAngleType(const std::string &S) {
    std::map<std::string, ASTAngleType>::const_iterator I = ATM.find(S);
    return I == ATM.end() ? ASTAngleTypeGeneric : (*I).second;
  }

  static ASTAngleType DetermineAngleType(const ASTIdentifierNode *Id) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    std::map<std::string, ASTAngleType>::const_iterator I =
        ATM.find(Id->GetName());
    return I == ATM.end() ? ASTAngleTypeGeneric : (*I).second;
  }

  virtual bool IsNan() const { return mpfr_nan_p(MPValue) != 0; }

  virtual bool IsInf() const { return mpfr_inf_p(MPValue) != 0; }

  virtual bool IsZero() const { return mpfr_zero_p(MPValue) != 0; }

  virtual bool IsNegative() const { return mpfr_sgn(MPValue) < 0; }

  virtual bool IsPositive() const { return mpfr_sgn(MPValue) > 0; }

  // return true if MPValue is neither NaN nor Inf.
  virtual bool IsNumber() const { return mpfr_number_p(MPValue) != 0; }

  // return true if MPValue is neither NaN nor Inf nor Zero.
  virtual bool IsRegular() const { return mpfr_regular_p(MPValue) != 0; }

  virtual std::string GetValue() const {
    if (IsNan())
      return "NaN";
    else if (IsZero())
      return "0.0";
    else if (IsInf())
      return "Inf";

    std::string R;
    mpfr_exp_t E = 0;
    char *S = mpfr_get_str(NULL, &E, 10, 0, MPValue, MPFR_RNDN);
    if (S) {
      std::stringstream SSR;
      if (E) {
        if (S[0] == u8'-' || S[0] == u8'+')
          E += 1U;
        SSR.write(S, (size_t)E);
        SSR << u8'.';
        const char *SP = S + (size_t)E;
        SSR << SP;
        R = SSR.str();
      } else {
        SSR << "0.";
        if (Bits <= 32U)
          SSR.write(S, 8U);
        else if (Bits <= 64U)
          SSR.write(S, 16U);
        else if (Bits <= 128U)
          SSR.write(S, 32U);
        else
          SSR.write(S, 34U);
        R = SSR.str();
      }

      mpfr_free_str(S);
    }

    ASTStringUtils::Instance().SaneDecimal(R);
    return R;
  }

  virtual std::string GetValue(size_t Sz, const char *Fmt) const {
    if (IsNan())
      return "NaN";
    else if (IsZero())
      return "0.0";
    else if (IsInf())
      return "Inf";

    char S[Sz + 1];
    mpfr_sprintf(S, Fmt, MPValue);
    std::string R = S;
    return R;
  }

  virtual std::string GetNanString() const {
    char S[4];
    mpfr_sprintf(S, "%RNf", MPValue);
    S[0] = std::toupper(S[0]);
    S[2] = std::toupper(S[2]);
    std::string R = S;
    return R;
  }

  virtual std::string GetInfString() const {
    char S[4];
    mpfr_sprintf(S, "%RNf", MPValue);
    S[0] = std::toupper(S[0]);
    std::string R = S;
    return R;
  }

  virtual mpfr_t &GetMPValue() { return MPValue; }

  virtual const mpfr_t &GetMPValue() const { return MPValue; }

  virtual std::string GetValue(int Base) const {
    std::string R;

    if (IsValue()) {
      if (IsNan()) {
        R = "NaN";
      } else if (IsZero()) {
        R = "0.0";
      } else if (IsInf()) {
        R = "Inf";
      } else {
        mpfr_exp_t E = 0;
        const char *C = mpfr_get_str(NULL, &E, Base, 64, MPValue, MPFR_RNDN);
        R = C ? C : "";
      }
    }

    return R;
  }

  virtual ASTMPDecimalNode *AsMPDecimal() const {
    return new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, Bits, MPValue);
  }

  virtual float AsFloat() const { return mpfr_get_flt(MPValue, MPFR_RNDN); }

  virtual ASTFloatNode *AsASTFloatNode() const {
    return new ASTFloatNode(&ASTIdentifierNode::Float,
                            mpfr_get_flt(MPValue, MPFR_RNDN));
  }

  virtual double AsDouble() const { return mpfr_get_d(MPValue, MPFR_RNDN); }

  virtual ASTDoubleNode *AsASTDoubleNode() const {
    return new ASTDoubleNode(&ASTIdentifierNode::Double,
                             mpfr_get_d(MPValue, MPFR_RNDN));
  }

  virtual std::bitset<64> &AsBitset() {
    mpfr_exp_t E = 0;

    if (Bits > 64) {
      mpfr_t MPT;
      mpfr_init2(MPT, 64);
      (void)mpfr_set(MPT, MPValue, MPFR_RNDN);

      if (char *S = mpfr_get_str(NULL, &E, 2, 0, MPT, MPFR_RNDN)) {
        BST = std::bitset<64>(S);
        mpfr_free_str(S);
      }

      mpfr_clear(MPT);
    } else if (char *S = mpfr_get_str(NULL, &E, 2, 0, MPValue, MPFR_RNDN)) {
      BST = std::bitset<64>(S);
      mpfr_free_str(S);
    }

    return BST;
  }

  virtual const std::bitset<64> &AsBitset() const {
    mpfr_exp_t E = 0;

    if (Bits > ASTAngleNode::AngleBits) {
      mpfr_t MPT;
      mpfr_init2(MPT, ASTAngleNode::AngleBits);
      (void)mpfr_set(MPT, MPValue, MPFR_RNDN);

      if (char *S = mpfr_get_str(NULL, &E, 2, 0, MPT, MPFR_RNDN)) {
        BST = std::bitset<ASTAngleNode::AngleBits>(S);
        mpfr_free_str(S);
      }

      mpfr_clear(MPT);
    } else if (char *S = mpfr_get_str(NULL, &E, 2, 0, MPValue, MPFR_RNDN)) {
      BST = std::bitset<64>(S);
      mpfr_free_str(S);
    }

    return BST;
  }

  virtual std::bitset<64> &GetBitset() { return BST; }

  virtual const std::bitset<64> &GetBitset() const { return BST; }

  virtual void SetExpression(const ASTExpressionNode *EXN) {
    EX = EXN;
    ExprType = EXN->GetASTType();
  }

  virtual void SetExpression(const ASTIdentifierNode *Id) {
    ID = Id;
    ExprType = ASTTypeIdentifier;
  }

  virtual bool SetValue(const char *String, int Base = 10) {
    if (mpfr_set_str(MPValue, String, Base, MPFR_RNDN) != 0) {
      mpfr_set_d(MPValue, 0.0, MPFR_RNDN);
      Bits = 0;
      return false;
    }

    return true;
  }

  virtual bool SetValue(const std::string &S, int Base = 10) {
    return SetValue(S.c_str(), Base);
  }

  virtual bool SetValue(int Value) {
    std::string S = std::to_string(Value);
    S += ".00";
    ExprType = ASTTypeInt;
    return SetValue(S.c_str());
  }

  virtual bool SetValue(float Value) {
    std::string S = std::to_string(Value);
    ExprType = ASTTypeFloat;
    return SetValue(S.c_str());
  }

  virtual bool SetValue(double Value) {
    std::string S = std::to_string(Value);
    ExprType = ASTTypeDouble;
    return SetValue(S.c_str());
  }

  virtual bool SetValue(long double Value) {
    std::string S = std::to_string(Value);
    ExprType = ASTTypeLongDouble;
    return SetValue(S.c_str());
  }

  virtual bool SetValue(const mpfr_t &Value) {
    ExprType = ASTTypeMPDecimal;
    if (mpfr_set(MPValue, Value, MPFR_RNDN) != 0) {
      mpfr_set_d(MPValue, 0.0, MPFR_RNDD);
      Bits = 0;
      return false;
    }

    return true;
  }

  virtual void SetNumericConstant(ASTNumericConstant C) { NC = C; }

  virtual void SetImplicitConversion(const ASTImplicitConversionNode *IC) {
    ICE = IC;
  }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual unsigned GetBits() const { return Bits; }

  virtual void SetBits(unsigned B) { Bits = B; }

  virtual void SetGateParamName(const std::string &GPN) { GateParamName = GPN; }

  virtual ASTAngleType GetAngleType() const { return AngleType; }

  virtual bool HasImplicitConversion() const { return ICE != nullptr; }

  virtual void SetAngleType(ASTAngleType AT) { AngleType = AT; }

  virtual void P0(uint64_t V) { IR[0] = V; }

  virtual void P1(uint64_t V) { IR[1] = V; }

  virtual void P2(uint64_t V) { IR[2] = V; }

  virtual void P3(uint64_t V) { IR[3] = V; }

  virtual uint64_t P0() const { return IR[0]; }

  virtual uint64_t P1() const { return IR[1]; }

  virtual uint64_t P2() const { return IR[2]; }

  virtual uint64_t P3() const { return IR[3]; }

  virtual std::bitset<sizeof(uint64_t) * CHAR_BIT> WBitset() const {
    return std::bitset<sizeof(uint64_t) * CHAR_BIT>(IR[0]);
  }

  virtual std::bitset<sizeof(uint64_t) * CHAR_BIT> XBitset() const {
    return std::bitset<sizeof(uint64_t) * CHAR_BIT>(IR[1]);
  }

  virtual std::bitset<sizeof(int64_t) * CHAR_BIT> YBitset() const {
    return std::bitset<sizeof(int64_t) * CHAR_BIT>(IR[2]);
  }

  virtual std::bitset<sizeof(int64_t) * CHAR_BIT> ZBitset() const {
    return std::bitset<sizeof(int64_t) * CHAR_BIT>(IR[3]);
  }

  virtual const ASTIdentifierRefNode *GetPoint(unsigned IX) const {
    assert(IX < 3 && "Index is out-of-range!");
    return IX < 3 ? IIR[IX] : nullptr;
  }

  virtual void EraseFromLocalSymbolTable();

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTAngleNode *ExpressionError(const ASTIdentifierNode *Id,
                                       const std::string &ERM) {
    return new ASTAngleNode(Id, ERM);
  }

  static ASTAngleNode *ExpressionError(const std::string &ERM) {
    return new ASTAngleNode(ASTIdentifierNode::Angle.Clone(), ERM);
  }

  virtual void print() const override {
    std::cout << "<Angle>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;
    std::cout << "<Type>" << PrintAngleType(AngleType) << "</Type>"
              << std::endl;
    std::cout << "<Bits>" << std::dec << Bits << "</Bits>" << std::endl;

    if (!IR.empty()) {
      switch (IR.size()) {
      case 1:
        std::cout << "<Point0>" << IR[0] << "</Point0>" << std::endl;
        break;
      case 2:
        std::cout << "<Point0>" << IR[0] << "</Point0>" << std::endl;
        std::cout << "<Point1>" << IR[1] << "</Point1>" << std::endl;
        break;
      case 3:
        std::cout << "<Point0>" << IR[0] << "</Point0>" << std::endl;
        std::cout << "<Point1>" << IR[1] << "</Point1>" << std::endl;
        std::cout << "<Point2>" << IR[2] << "</Point2>" << std::endl;
        break;
      case 4:
        std::cout << "<Point0>" << IR[0] << "</Point0>" << std::endl;
        std::cout << "<Point1>" << IR[1] << "</Point1>" << std::endl;
        std::cout << "<Point2>" << IR[2] << "</Point2>" << std::endl;
        std::cout << "<Point3>" << IR[3] << "</Point3>" << std::endl;
        break;
      default:
        break;
      }
    }

    if (NC != CNone)
      std::cout << "<NumericConstant>" << PrintNumericConstant(NC)
                << "</NumericConstan>" << std::endl;

    std::cout << "<ImplicitConversion>" << std::boolalpha
              << HasImplicitConversion() << "</ImplicitConversion>"
              << std::endl;

    switch (ExprType) {
    case ASTTypeInt:
      if (I)
        I->print();
      break;
    case ASTTypeFloat:
      if (F)
        F->print();
      break;
    case ASTTypeDouble:
      if (D)
        D->print();
      break;
    case ASTTypeLongDouble:
      if (LD)
        LD->print();
      break;
    case ASTTypeMPInteger:
      if (MPI)
        MPI->print();
      break;
    case ASTTypeMPDecimal:
      if (MPD)
        MPD->print();
      break;
    case ASTTypeUnaryOp:
      if (UOP)
        UOP->print();
      break;
    case ASTTypeBinaryOp:
      if (BOP)
        BOP->print();
      break;
    case ASTTypeIdentifier:
      if (ID)
        ID->print();
      break;
    default:
      if (EX)
        EX->print();
      break;
    }

    if (IsNan())
      std::cout << "<Value>" << GetNanString() << "</Value>" << std::endl;
    else if (IsInf())
      std::cout << "<Value>" << GetInfString() << "</Value>" << std::endl;
    else if (IsZero())
      std::cout << "<Value>0.00</Value>" << std::endl;
    else
      std::cout << "<Value>" << GetValue() << "</Value>" << std::endl;
    std::cout << "</Angle>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTPowNode : public ASTExpressionNode {
private:
  union {
    const ASTIdentifierNode *Id;
    const ASTIntNode *Int;
    const ASTMPIntegerNode *MPI;
    const ASTBinaryOpNode *BOP;
    const ASTUnaryOpNode *UOP;
  };

  ASTType TargetType;

private:
  ASTPowNode() = delete;

public:
  ASTPowNode(const ASTIdentifierNode *I)
      : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypePow), Id(I),
        TargetType(I->GetASTType()) {}

  ASTPowNode(const ASTIntNode *I)
      : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypePow), Int(I),
        TargetType(I->GetASTType()) {}

  ASTPowNode(const ASTMPIntegerNode *MI)
      : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypePow), MPI(MI),
        TargetType(MI->GetASTType()) {}

  ASTPowNode(const ASTBinaryOpNode *BOp)
      : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypePow), BOP(BOp),
        TargetType(BOp->GetASTType()) {}

  ASTPowNode(const ASTUnaryOpNode *UOp)
      : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypePow), UOP(UOp),
        TargetType(UOp->GetASTType()) {}

  virtual ~ASTPowNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypePow; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetTargetASTType() const { return TargetType; }

  virtual const ASTIdentifierNode *GetIdentifierNode() const {
    return TargetType == ASTTypeIdentifier ? Id : nullptr;
  }

  virtual const ASTIntNode *GetIntegerNode() const {
    return TargetType == ASTTypeInt ? Int : nullptr;
  }

  virtual const ASTMPIntegerNode *GetMPIntegerNode() const {
    return TargetType == ASTTypeMPInteger ? MPI : nullptr;
  }

  virtual const ASTBinaryOpNode *GetBinaryOpNode() const {
    return TargetType == ASTTypeBinaryOp ? BOP : nullptr;
  }

  virtual const ASTUnaryOpNode *GetUnaryOpNode() const {
    return TargetType == ASTTypeUnaryOp ? UOP : nullptr;
  }

  virtual void print() const override {
    std::cout << "<PowNode>" << std::endl;
    switch (TargetType) {
    case ASTTypeIdentifier:
      Id->print();
      break;
    case ASTTypeInt:
      Int->print();
      break;
    case ASTTypeMPInteger:
      MPI->print();
      break;
    case ASTTypeBinaryOp:
      BOP->print();
      break;
    case ASTTypeUnaryOp:
      UOP->print();
      break;
    default:
      break;
    }
    std::cout << "</PowNode>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTOpenQASMExpressionNode : public ASTExpressionNode {
private:
  double Value;

private:
  ASTOpenQASMExpressionNode() = delete;

public:
  ASTOpenQASMExpressionNode(const ASTIdentifierNode *Id, double V)
      : ASTExpressionNode(Id, ASTTypeOpenQASMExpression), Value(V) {}

  virtual ~ASTOpenQASMExpressionNode() = default;

  virtual bool ValidateVersion() const { return Value == 2.0 || Value == 3.0; }

  virtual double GetVersion() const { return Value; }

  virtual std::string GetVersionAsString() const {
    return std::to_string(Value);
  }

  virtual ASTType GetASTType() const override {
    return ASTTypeOpenQASMExpression;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::GetIdentifier();
  }

  virtual const std::string &GetName() const override {
    return GetIdentifier()->GetName();
  }

  virtual void print() const override {
    std::cout << "<OpenQASMExpression>" << std::endl;
    std::cout << "<Version>" << std::fixed << std::setprecision(1) << Value
              << "</Version>" << std::endl;
    std::cout << "</OpenQASMExpression>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTOpenQASMStatementNode : public ASTStatementNode {
private:
  ASTOpenQASMStatementNode() = delete;

public:
  ASTOpenQASMStatementNode(const ASTOpenQASMExpressionNode *OQE)
      : ASTStatementNode(OQE->GetIdentifier(), OQE) {}

  virtual ~ASTOpenQASMStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeOpenQASMStatement;
  }

  virtual void print() const override {
    std::cout << "<OpenQASMStatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</OpenQASMStatement>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_TYPES_H
