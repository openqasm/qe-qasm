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

#ifndef __QASM_AST_IDENTIFIER_NODE_H
#define __QASM_AST_IDENTIFIER_NODE_H

#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTExpression.h>

#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>

namespace QASM {

class ASTIdentifierRefNode;
class ASTExpressionNode;
class ASTBinaryOpNode;
class ASTUnaryOpNode;
class ASTSymbolTableEntry;

class ASTIdentifierNode : public ASTExpression {
  friend class ASTBuilder;
  friend class ASTIdentifierRefNode;

protected:
  std::string Name;
  mutable std::string MangledName;
  mutable std::string PolymorphicName;
  mutable std::string MangledLiteralName;
  std::string IndexIdentifier;
  mutable std::size_t Hash;
  mutable std::size_t MHash;
  mutable std::size_t MLHash;
  mutable std::map<unsigned, const ASTIdentifierRefNode *> References;
  mutable unsigned Bits;
  unsigned NumericIndex;
  mutable bool Indexed;
  mutable bool NoQubit;
  mutable bool GateLocal;
  mutable bool ComplexPart;
  mutable bool HasSTE;
  mutable const ASTIdentifierNode *RV;
  union {
    mutable const ASTBinaryOpNode *BOP;
    mutable const ASTUnaryOpNode *UOP;
  };

  mutable ASTExpressionNode *EXP;
  ASTSymbolTableEntry *STE;
  mutable const ASTDeclarationContext *CTX;
  ASTType EvalType;
  ASTType SType;
  ASTType PType;
  ASTOpType OpType;
  ASTSymbolScope SymScope;
  bool RD;
  mutable bool IV;
  const ASTIdentifierNode *PRD;

  static uint64_t SI;

private:
  ASTIdentifierNode() = delete;

private:
  void SetIndexIdentifier(uint64_t LB, uint64_t RB) {
    std::string IX = Name.substr(LB + 1, RB - LB - 1);
    if (!IX.empty()) {
      if (!isdigit(IX.c_str()[0]))
        IndexIdentifier = IX;
    }
  }

  void SetNumericIndex(uint64_t LB, uint64_t RB) {
    std::string IX = Name.substr(LB + 1, RB - LB - 1);
    if (!IX.empty() && isdigit(IX.c_str()[0]))
      NumericIndex = static_cast<unsigned>(std::stoul(IX));
  }

public:
  static const unsigned IdentifierBits = 64U;

public:
  ASTIdentifierNode(const std::string &Id, unsigned B = ~0x0)
      : ASTExpression(), Name(Id), MangledName(), PolymorphicName(Id),
        MangledLiteralName(), IndexIdentifier(), Hash(0UL), MHash(0UL),
        MLHash(0UL), References(), Bits(B),
        NumericIndex(static_cast<unsigned>(~0x0)), Indexed(false),
        NoQubit(false), GateLocal(false), ComplexPart(false), HasSTE(false),
        RV(nullptr), BOP(nullptr), EXP(nullptr), STE(nullptr),
        CTX(ASTDeclarationContextTracker::Instance().GetCurrentContext()),
        EvalType(ASTTypeUndefined), SType(ASTTypeUndefined),
        PType(ASTTypeUndefined), OpType(ASTOpTypeUndefined),
        SymScope(ASTDeclarationContextTracker::Instance().GetCurrentScope()),
        RD(false), IV(false), PRD(nullptr) {
    CTX->RegisterSymbol(this, GetASTType());
    std::string::size_type LB = Id.find_last_of('[');
    std::string::size_type RB = Id.find_last_of(']');
    if (LB != std::string::npos && RB != std::string::npos) {
      SetIndexed(true);
      SetIndexIdentifier(LB, RB);
      SetNumericIndex(LB, RB);
    }
  }

  ASTIdentifierNode(const std::string &Id, ASTType STy, unsigned B = ~0x0)
      : ASTExpression(), Name(Id), MangledName(), PolymorphicName(Id),
        MangledLiteralName(), IndexIdentifier(), Hash(0UL), MHash(0UL),
        MLHash(0UL), References(), Bits(B),
        NumericIndex(static_cast<unsigned>(~0x0)), Indexed(false),
        NoQubit(false), GateLocal(false), ComplexPart(false), HasSTE(false),
        RV(nullptr), BOP(nullptr), EXP(nullptr), STE(nullptr),
        CTX(ASTDeclarationContextTracker::Instance().GetCurrentContext()),
        EvalType(ASTTypeUndefined), SType(STy), PType(ASTTypeUndefined),
        OpType(ASTOpTypeUndefined),
        SymScope(ASTDeclarationContextTracker::Instance().GetCurrentScope()),
        RD(false), IV(false), PRD(nullptr) {
    CTX->RegisterSymbol(this, GetASTType());
    std::string::size_type LB = Id.find_last_of('[');
    std::string::size_type RB = Id.find_last_of(']');
    if (LB != std::string::npos && RB != std::string::npos) {
      SetIndexed(true);
      SetIndexIdentifier(LB, RB);
      SetNumericIndex(LB, RB);
    }
  }

  ASTIdentifierNode(const std::string &Id, const ASTBinaryOpNode *BOp,
                    unsigned B = ~0x0);

  ASTIdentifierNode(const std::string &Id, const ASTUnaryOpNode *UOp,
                    unsigned B = ~0x0);

  virtual ~ASTIdentifierNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeIdentifier; }

  virtual ASTSemaType GetSemaType() const { return SemaTypeExpression; }

  virtual ASTType GetValueType() const;

  virtual const std::string &GetName() const { return Name; }

  virtual bool IsMangled() const {
    return !MangledName.empty() && MangledName.length() > 2U &&
           MangledName[0] == '_' && MangledName[1] == 'Q';
  }

  virtual const std::string &GetMangledName() const { return MangledName; }

  virtual const std::string &GetMangledLiteralName() const {
    return MangledLiteralName;
  }

  virtual const std::string &GetGateParamName() const {
    return PolymorphicName;
  }

  virtual const std::string &GetDefcalGroupName() const {
    return PolymorphicName;
  }

  virtual const std::string &GetResetName() const { return PolymorphicName; }

  virtual const std::string &GetPolymorphicName() const {
    return PolymorphicName;
  }

  virtual unsigned GetBits() const { return Bits; }

  virtual void SetBits(unsigned B) const { Bits = B; }

  virtual void SetRedeclaration(bool V = true) { RD = V; }

  virtual bool IsRedeclaration() const { return RD; }

  virtual void SetPredecessor(const ASTIdentifierNode *Id);

  virtual const ASTIdentifierNode *GetPredecessor() const { return PRD; }

  unsigned GetPredecessors(std::vector<const ASTIdentifierNode *> &V) const {
    const ASTIdentifierNode *P = GetPredecessor();

    while (P) {
      V.push_back(P);
      P = P->GetPredecessor();
    }

    return static_cast<unsigned>(V.size());
  }

  virtual void SetSymbolType(ASTType STy);

  virtual void SetPolymorphicType(ASTType PTy);

  virtual void RestoreType();

  virtual void SetInductionVariable(bool V) { IV = V; }

  virtual void SetInductionVariable(bool V) const { IV = V; }

  virtual bool IsInductionVariable() const { return IV; }

  virtual ASTType GetPolymorphicType() const { return PType; }

  virtual bool HasPolymorphicType() const {
    return PType != ASTTypeUndefined || PType != SType;
  }

  virtual void SetExpression(ASTExpressionNode *EX) {
    assert(EX && "Invalid ASTExpressionNode argument!");
    EXP = EX;
  }

  virtual void SetExpression(ASTExpressionNode *EX) const {
    assert(EX && "Invalid ASTExpressionNode argument!");
    EXP = EX;
  }

  virtual void SetMangledName(const char *MN, bool Force = false) {
    if (!IsMangled() || Force) {
      assert(MN && "Invalid mangled name argument!");
      MangledName = MN;
      MHash = std::hash<std::string>{}(MangledName);
    }
  }

  virtual void SetMangledName(const char *MN, bool Force = false) const {
    if (!IsMangled() || Force) {
      assert(MN && "Invalid mangled name argument!");
      MangledName = MN;
      MHash = std::hash<std::string>{}(MangledName);
    }
  }

  virtual void SetMangledName(const std::string &MN, bool Force = false) {
    if (!IsMangled() || Force) {
      assert(!MN.empty() && "Invalid mangled name argument!");
      MangledName = MN;
      MHash = std::hash<std::string>{}(MangledName);
    }
  }

  virtual void SetMangledName(const std::string &MN, bool Force = false) const {
    if (!IsMangled() || Force) {
      assert(!MN.empty() && "Invalid mangled name argument!");
      MangledName = MN;
      MHash = std::hash<std::string>{}(MangledName);
    }
  }

  virtual void SetGateParamName(const char *GPN) {
    assert(GPN && "Invalid Gate Param Name argument!");
    PolymorphicName = GPN;
  }

  virtual void SetGateParamName(const char *GPN) const {
    assert(GPN && "Invalid Gate Param Name argument!");
    PolymorphicName = GPN;
  }

  virtual void SetGateParamName(const std::string &GPN) {
    assert(!GPN.empty() && "Invalid Gate Param Name argument!");
    PolymorphicName = GPN;
  }

  virtual void SetGateParamName(const std::string &GPN) const {
    assert(!GPN.empty() && "Invalid Gate Param Name argument!");
    PolymorphicName = GPN;
  }

  virtual void SetDefcalGroupName(const char *DGN) {
    assert(DGN && "Invalid Defcal Group Name argument!");
    PolymorphicName = DGN;
  }

  virtual void SetDefcalGroupName(const char *DGN) const {
    assert(DGN && "Invalid Defcal Group Name argument!");
    PolymorphicName = DGN;
  }

  virtual void SetDefcalGroupName(const std::string &DGN) {
    assert(!DGN.empty() && "Invalid Defcal Group Name argument!");
    PolymorphicName = DGN;
  }

  virtual void SetDefcalGroupName(const std::string &DGN) const {
    assert(!DGN.empty() && "Invalid Defcal Group Name argument!");
    PolymorphicName = DGN;
  }

  virtual void SetResetName(const char *RN) {
    assert(RN && "Invalid Reset Name argument!");
    PolymorphicName = RN;
  }

  virtual void SetResetName(const char *RN) const {
    assert(RN && "Invalid Reset Name argument!");
    PolymorphicName = RN;
  }

  virtual void SetResetName(const std::string &RN) {
    assert(!RN.empty() && "Invalid Reset Name argument!");
    PolymorphicName = RN;
  }

  virtual void SetResetName(const std::string &RN) const {
    assert(!RN.empty() && "Invalid Reset Name argument!");
    PolymorphicName = RN;
  }

  virtual void SetPolymorphicName(const char *PN) {
    assert(PN && "Invalid Polymorphic Name argument!");
    PolymorphicName = PN;
  }

  virtual void SetPolymorphicName(const char *PN) const {
    assert(PN && "Invalid Polymorphic Name argument!");
    PolymorphicName = PN;
  }

  virtual void SetPolymorphicName(const std::string &PN) {
    assert(!PN.empty() && "Invalid Polymorphic Name argument!");
    PolymorphicName = PN;
  }

  virtual void SetPolymorphicName(const std::string &PN) const {
    assert(!PN.empty() && "Invalid Polymorphic Name argument!");
    PolymorphicName = PN;
  }

  virtual void SetMangledLiteralName(const char *ML) {
    assert(ML && "Invalid Mangled Literal Name argument!");
    MangledLiteralName = ML;
    MLHash = std::hash<std::string>{}(MangledLiteralName);
  }

  virtual void SetMangledLiteralName(const char *ML) const {
    assert(ML && "Invalid Mangled Literal Name argument!");
    MangledLiteralName = ML;
    MLHash = std::hash<std::string>{}(MangledLiteralName);
  }

  virtual void SetMangledLiteralName(const std::string &ML) {
    assert(!ML.empty() && "Invalid Mangled Literal Name argument!");
    MangledLiteralName = ML;
    MLHash = std::hash<std::string>{}(MangledLiteralName);
  }

  virtual void SetMangledLiteralName(const std::string &ML) const {
    assert(!ML.empty() && "Invalid Mangled Literal Name argument!");
    MangledLiteralName = ML;
    MLHash = std::hash<std::string>{}(MangledLiteralName);
  }

  virtual uint64_t GetHash() const {
    if (Hash == 0UL && !Name.empty())
      Hash = std::hash<std::string>{}(Name);

    return static_cast<uint64_t>(Hash);
  }

  virtual uint64_t GetMHash() const {
    if (MHash == 0UL && !MangledName.empty())
      MHash = std::hash<std::string>{}(MangledName);

    return static_cast<uint64_t>(MHash);
  }

  virtual uint64_t GetMLHash() const {
    if (MHash == 0UL && !MangledLiteralName.empty())
      MHash = std::hash<std::string>{}(MangledLiteralName);

    return static_cast<uint64_t>(MHash);
  }

  virtual void SetIndexIdentifier(const std::string &S) { IndexIdentifier = S; }

  virtual const std::string &GetIndexIdentifier() const {
    return IndexIdentifier;
  }

  virtual void SetIndexed(bool V = true) { Indexed = V; }

  virtual bool IsIndexed() const { return Indexed; }

  virtual bool HasInvalidBitWidth() const {
    return Bits == 0 || Bits == static_cast<unsigned>(~0x0);
  }

  static bool InvalidBits(unsigned Bits) {
    return Bits == static_cast<unsigned>(~0x0);
  }

  virtual void SetComplexPart(bool V = true) { ComplexPart = V; }

  virtual bool IsComplexPart() const { return ComplexPart; }

  virtual bool IsBoundQubit() const { return Name[0] == '$'; }

  virtual bool HasExpression() const { return EXP != nullptr; }

  virtual bool HasSymbolTableEntry() const { return HasSTE && STE != nullptr; }

  virtual void SetHasSymbolTableEntry(bool V = true) { HasSTE = V; }

  virtual void SetSymbolTableEntry(ASTSymbolTableEntry *ST);

  virtual void SetDeclarationContext(const ASTDeclarationContext *CX) {
    CTX->UnregisterSymbol(this);
    CTX = CX;
    CTX->RegisterSymbol(this, GetASTType());
  }

  virtual void SetDeclarationContext(const ASTDeclarationContext *CX) const {
    CTX->UnregisterSymbol(this);
    CTX = CX;
    CTX->RegisterSymbol(this, GetASTType());
  }

  virtual const ASTSymbolTableEntry *GetSymbolTableEntry() const { return STE; }

  virtual ASTExpressionNode *GetExpression() { return EXP; }

  virtual const ASTExpressionNode *GetExpression() const { return EXP; }

  virtual ASTSymbolTableEntry *GetSymbolTableEntry() { return STE; }

  virtual const ASTDeclarationContext *GetDeclarationContext() const {
    return CTX;
  }

  virtual unsigned GetContextIndext() const { return CTX->GetIndex(); }

  virtual bool InGlobalContext() const {
    return ASTDeclarationContextTracker::Instance().IsGlobalContext(CTX);
  }

  virtual bool IsAlive() const { return CTX->IsAlive(); }

  virtual bool IsDead() const { return CTX->IsDead(); }

  virtual ASTType GetSymbolType() const { return SType; }

  virtual std::string GetComplexPart() const {
    if (IsComplexPart())
      return Name.substr(Name.find('.') + 1, std::string::npos);
    return std::string();
  }

  virtual std::string GetQubitMnemonic() const {
    if (!Name.empty() && Name[0] == '$')
      return Name.substr(1, std::string::npos);

    return std::string();
  }

  virtual bool IndexIsIdentifier() const {
    return Indexed && !IndexIdentifier.empty();
  }

  virtual void SetNoQubit(bool V = true) const { NoQubit = V; }

  virtual bool IsNoQubit() const { return NoQubit; }

  virtual void SetGateLocal(bool V = true) const { GateLocal = V; }

  virtual bool IsGateLocal() const { return GateLocal; }

  virtual void SetRValue() const { RV = this; }

  virtual bool IsRValue() const { return RV; }

  virtual const ASTIdentifierNode *GetRValue() const { return RV; }

  virtual ASTOpType GetOpType() const { return OpType; }

  virtual bool NeedsEval() const { return OpType != ASTOpTypeUndefined; }

  virtual unsigned GetNumericIndex() const { return NumericIndex; }

  virtual void SetBinaryOp(const ASTBinaryOpNode *BOp);

  virtual void SetUnaryOp(const ASTUnaryOpNode *UOp);

  virtual ASTIdentifierNode *CreateRValueReference(unsigned NumBits) const {
    assert(NumBits != static_cast<unsigned>(~0x0) &&
           "Invalid number of Bits for RValue ASTIdentifierNode!");

    ASTIdentifierNode *RId = new ASTIdentifierNode(Name, NumBits);
    assert(RId && "Could not create a valid RValue ASTIdentifierNode!");

    RId->SetRValue();
    return RId;
  }

  static ASTIdentifierNode *StatementIdentifier() {
    std::stringstream SN;
    SN << "ast-statement-node-id-" << SI++;
    return new ASTIdentifierNode(SN.str(), ASTTypeStatement,
                                 ASTIdentifierNode::IdentifierBits);
  }

  virtual void SetLocalScope() {
    SymScope = ASTSymbolScope::Local;
    CTX = ASTDeclarationContextTracker::Instance().GetCurrentContext();
  }

  virtual void SetLocalScope(const ASTDeclarationContext *DC) {
    assert(DC && "Invalid ASTDeclarationContext argument!");
    SymScope = ASTSymbolScope::Local;
    CTX = DC;
  }

  virtual void SetGlobalScope() {
    SymScope = ASTSymbolScope::Global;
    CTX = ASTDeclarationContextTracker::Instance().GetGlobalContext();
  }

  virtual ASTSymbolScope GetSymbolScope() const { return SymScope; }

  virtual bool IsGlobalScope() const {
    return SymScope == ASTSymbolScope::Global;
  }

  virtual bool IsLocalScope() const {
    return SymScope == ASTSymbolScope::Local;
  }

  virtual void AddReference(const ASTIdentifierRefNode *IdRef) const;

  virtual bool IsReference(const ASTIdentifierRefNode *IdRef) const;

  virtual const ASTIdentifierRefNode *GetReference(unsigned IX) const;

  virtual const ASTBinaryOpNode *GetBinaryOp() const {
    return EvalType == ASTTypeBinaryOp ? BOP : nullptr;
  }

  virtual const ASTUnaryOpNode *GetUnaryOp() const {
    return EvalType == ASTTypeUnaryOp ? UOP : nullptr;
  }

  static std::string GenRandomString(unsigned Length = 24U);

  virtual bool operator<(const ASTIdentifierNode *RHS) const {
    return RHS ? Name < RHS->Name : false;
  }

  virtual bool operator<(const ASTIdentifierNode &RHS) const {
    return Name < RHS.Name;
  }

  virtual bool operator==(const ASTIdentifierNode &RHS) const {
    return Name == RHS.Name;
  }

  virtual bool operator!=(const ASTIdentifierNode &RHS) const {
    return !(Name == RHS.Name);
  }

  virtual bool IsReference() const { return false; }

  virtual bool IsUnresolvedLValue() const { return false; }

  virtual bool IsError() const { return SType == ASTTypeExpressionError; }

  virtual const std::string &GetError() const { return Name; }

  static ASTIdentifierNode *IdentifierError(const std::string &IS) {
    return new ASTIdentifierNode(IS, ASTTypeExpressionError);
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}

  ASTIdentifierNode *Clone();

  ASTIdentifierNode *Clone(const ASTLocation &Loc);

  ASTIdentifierNode *Clone(unsigned Bits);

  static ASTIdentifierNode Char;
  static ASTIdentifierNode Short;
  static ASTIdentifierNode Int;
  static ASTIdentifierNode UInt;
  static ASTIdentifierNode Long;
  static ASTIdentifierNode ULong;
  static ASTIdentifierNode Float;
  static ASTIdentifierNode Double;
  static ASTIdentifierNode LongDouble;
  static ASTIdentifierNode Void;
  static ASTIdentifierNode String;
  static ASTIdentifierNode Bool;
  static ASTIdentifierNode Pointer;
  static ASTIdentifierNode Ellipsis;
  static ASTIdentifierNode MPInt;
  static ASTIdentifierNode MPDec;
  static ASTIdentifierNode MPComplex;
  static ASTIdentifierNode Imag;
  static ASTIdentifierNode Lambda;
  static ASTIdentifierNode Phi;
  static ASTIdentifierNode Theta;
  static ASTIdentifierNode Pi;
  static ASTIdentifierNode Tau;
  static ASTIdentifierNode Gamma;
  static ASTIdentifierNode Euler;
  static ASTIdentifierNode EulerNumber;
  static ASTIdentifierNode Gate;
  static ASTIdentifierNode GateQOp;
  static ASTIdentifierNode Defcal;
  static ASTIdentifierNode Duration;
  static ASTIdentifierNode DurationOf;
  static ASTIdentifierNode Stretch;
  static ASTIdentifierNode Length;
  static ASTIdentifierNode Measure;
  static ASTIdentifierNode Box;
  static ASTIdentifierNode BoxAs;
  static ASTIdentifierNode BoxTo;
  static ASTIdentifierNode Qubit;
  static ASTIdentifierNode QubitParam;
  static ASTIdentifierNode QCAlias;
  static ASTIdentifierNode QC;
  static ASTIdentifierNode Bitset;
  static ASTIdentifierNode Angle;
  static ASTIdentifierNode Null;
  static ASTIdentifierNode Operator;
  static ASTIdentifierNode Operand;
  static ASTIdentifierNode BinaryOp;
  static ASTIdentifierNode UnaryOp;
  static ASTIdentifierNode Inv;
  static ASTIdentifierNode Pow;
  static ASTIdentifierNode Cast;
  static ASTIdentifierNode BadCast;
  static ASTIdentifierNode ImplConv;
  static ASTIdentifierNode BadImplConv;
  static ASTIdentifierNode Ctrl;
  static ASTIdentifierNode NegCtrl;
  static ASTIdentifierNode BadCtrl;
  static ASTIdentifierNode Expression;
  static ASTIdentifierNode Statement;
  static ASTIdentifierNode Reset;
  static ASTIdentifierNode Result;
  static ASTIdentifierNode Return;
  static ASTIdentifierNode GPhase;
  static ASTIdentifierNode BadGPhase;
  static ASTIdentifierNode Delay;
  static ASTIdentifierNode Input;
  static ASTIdentifierNode Output;
  static ASTIdentifierNode IfExpression;
  static ASTIdentifierNode ElseIfExpression;
  static ASTIdentifierNode ElseExpression;
  static ASTIdentifierNode For;
  static ASTIdentifierNode ForLoopRange;
  static ASTIdentifierNode While;
  static ASTIdentifierNode DoWhile;
  static ASTIdentifierNode Switch;
  static ASTIdentifierNode Case;
  static ASTIdentifierNode Default;
  static ASTIdentifierNode QPPDirective;
  static ASTIdentifierNode Pragma;
  static ASTIdentifierNode Annotation;
  static ASTIdentifierNode Popcount;
  static ASTIdentifierNode Rotl;
  static ASTIdentifierNode Rotr;
  static ASTIdentifierNode Rotate;
  static ASTIdentifierNode ArraySubscript;
  static ASTIdentifierNode CBitArray;
  static ASTIdentifierNode QubitArray;
  static ASTIdentifierNode BoolArray;
  static ASTIdentifierNode IntArray;
  static ASTIdentifierNode FloatArray;
  static ASTIdentifierNode MPIntArray;
  static ASTIdentifierNode MPDecArray;
  static ASTIdentifierNode MPComplexArray;
  static ASTIdentifierNode AngleArray;
  static ASTIdentifierNode DurationArray;
  static ASTIdentifierNode FrameArray;
  static ASTIdentifierNode PortArray;
  static ASTIdentifierNode WaveformArray;
  static ASTIdentifierNode InvalidArray;
  static ASTIdentifierNode InitializerList;
  static ASTIdentifierNode SyntaxError;

  // OpenPulse
  static ASTIdentifierNode Port;
  static ASTIdentifierNode Play;
  static ASTIdentifierNode Frame;
  static ASTIdentifierNode Waveform;
};

class ASTArraySubscriptNode;
class ASTArraySubscriptList;

class ASTIdentifierRefNode : public ASTIdentifierNode {
protected:
  const ASTIdentifierNode *Id;
  union {
    const ASTArraySubscriptNode *ASN;
    const ASTIdentifierNode *IxID;
  };

  const ASTArraySubscriptList *ASL;
  std::string IVX;
  unsigned Index;
  ASTType RTy;
  ASTExpressionType IITy;
  bool ULV; // unresolved lvalue.

private:
  ASTIdentifierRefNode() = delete;
  ASTIdentifierRefNode(const ASTIdentifierRefNode &R) = delete;
  ASTIdentifierNode &operator=(const ASTIdentifierNode &R) = delete;

  void SetIndex(const std::string &IdS) {
    std::string::size_type SP = IdS.find_last_of('[');
    std::string::size_type EP = IdS.find_last_of(']');
    if (SP != std::string::npos && EP != std::string::npos)
      Index =
          static_cast<unsigned>(std::stoul(IdS.substr(SP + 1, EP - (SP + 1))));
  }

  ASTType ResolveReferenceType(ASTType ITy) const;

public:
  ASTIdentifierRefNode(const ASTIdentifierNode *IDN, unsigned NumBits,
                       bool LV = false)
      : ASTIdentifierNode(IDN->Name, IDN->GetSymbolType(), NumBits), Id(IDN),
        ASN(nullptr), ASL(nullptr), IVX(), Index(static_cast<unsigned>(~0x0)),
        RTy(ASTTypeUndefined), IITy(ASTEXTypeSSA), ULV(LV) {
    this->CTX->UnregisterSymbol(IDN);
    this->CTX = IDN->GetDeclarationContext();
    this->CTX->RegisterSymbol(this, GetASTType());
    SetIndex(IDN->GetName());
    this->RV = this;
    Id->AddReference(this);
    if (IDN->IsInductionVariable()) {
      this->SetInductionVariable(true);
      IITy = ASTAXTypeInductionVariable;
    }

    RTy = ResolveReferenceType(IDN->GetSymbolType());
    assert(RTy != ASTTypeUndefined &&
           "Undefined type for ASTIdentifierRefNode!");
  }

  ASTIdentifierRefNode(const std::string &ID, const ASTIdentifierNode *IDN,
                       unsigned NumBits, bool LV = false)
      : ASTIdentifierNode(ID, IDN->GetSymbolType(), NumBits), Id(IDN),
        ASN(nullptr), ASL(nullptr), IVX(), Index(static_cast<unsigned>(~0x0)),
        RTy(ASTTypeUndefined), IITy(ASTEXTypeSSA), ULV(LV) {
    this->CTX->UnregisterSymbol(IDN);
    this->CTX = IDN->GetDeclarationContext();
    this->CTX->RegisterSymbol(this, GetASTType());
    SetIndex(ID);
    this->RV = this;
    SymScope = IDN->GetSymbolScope();
    Id->AddReference(this);
    if (IDN->IsInductionVariable()) {
      this->SetInductionVariable(true);
      IITy = ASTAXTypeInductionVariable;
    }

    RTy = ResolveReferenceType(IDN->GetSymbolType());
    assert(RTy != ASTTypeUndefined &&
           "Undefined type for ASTIdentifierRefNode!");
  }

  ASTIdentifierRefNode(const std::string &ID, ASTType Ty,
                       const ASTIdentifierNode *IDN, unsigned NumBits,
                       bool LV = false)
      : ASTIdentifierNode(ID, Ty, NumBits), Id(IDN), ASN(nullptr), ASL(nullptr),
        IVX(), Index(static_cast<unsigned>(~0x0)), RTy(ASTTypeUndefined),
        IITy(ASTEXTypeSSA), ULV(LV) {
    this->CTX->UnregisterSymbol(IDN);
    this->CTX = IDN->GetDeclarationContext();
    this->CTX->RegisterSymbol(this, GetASTType());
    SetIndex(ID);
    this->RV = this;
    SymScope = IDN->GetSymbolScope();
    Id->AddReference(this);
    if (IDN->IsInductionVariable()) {
      this->SetInductionVariable(true);
      IITy = ASTAXTypeInductionVariable;
    }

    RTy = Ty;
    assert(RTy != ASTTypeUndefined &&
           "Undefined type for ASTIdentifierRefNode!");
  }

  ASTIdentifierRefNode(const std::string &US, const std::string &IS, ASTType Ty,
                       const ASTIdentifierNode *IDN, unsigned NumBits,
                       bool LV = false, ASTSymbolTableEntry *ST = nullptr,
                       const ASTArraySubscriptNode *AN = nullptr,
                       const ASTArraySubscriptList *AL = nullptr);

  virtual ~ASTIdentifierRefNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeIdentifierRef; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTType GetReferenceType() const { return RTy; }

  virtual ASTExpressionType GetIndexIdentifierType() const { return IITy; }

  virtual bool IsIndexedIdentifier() const {
    return IITy == ASTIITypeIndexIdentifier || IITy == ASTAXTypeIndexIdentifier;
  }

  virtual const ASTIdentifierNode *AsIdentifier() const {
    return dynamic_cast<const ASTIdentifierNode *>(this);
  }

  virtual const ASTIdentifierNode *GetIdentifier() const override { return Id; }

  virtual unsigned GetIndex() const { return Index; }

  virtual void SetRedeclaration(bool V = true) override {
    (void)V; // Quiet compiler warning (-Wunused-parameter)
    RD = false;
  }

  virtual void SetPredecessor(const ASTIdentifierNode *Id) override;

  virtual const ASTIdentifierNode *GetPredecessor() const override {
    return nullptr;
  }

  virtual void SetSymbolTableEntry(ASTSymbolTableEntry *ST) override {
    assert(ST && "Invalid ASTSymbolTableEntry argument!");
    ASTIdentifierNode::STE = ST;
    ASTIdentifierNode::HasSTE = true;
    SetIndexed(true);
  }

  virtual const ASTSymbolTableEntry *GetSymbolTableEntry() const override {
    return ASTIdentifierNode::GetSymbolTableEntry();
  }

  virtual ASTSymbolTableEntry *GetSymbolTableEntry() override {
    return ASTIdentifierNode::GetSymbolTableEntry();
  }

  static unsigned GetIndexChain(const ASTIdentifierRefNode *IdR,
                                std::vector<unsigned> &IXC);

  virtual unsigned GetIndexChain(std::vector<unsigned> &IXC) const {
    return ASTIdentifierRefNode::GetIndexChain(this, IXC);
  }

  virtual void Mangle();

  virtual bool IsReference() const override { return true; }

  virtual bool IsUnresolvedLValue() const override { return ULV; }

  virtual void SetBinaryOp(const ASTBinaryOpNode *BOp) override;

  virtual void SetUnaryOp(const ASTUnaryOpNode *UOp) override;

  virtual void SetMangledName(const std::string &MN,
                              bool Force = false) override {
    ASTIdentifierNode::SetMangledName(MN, Force);
  }

  virtual void SetMangledName(const std::string &MN,
                              bool Force = false) const override {
    ASTIdentifierNode::SetMangledName(MN, Force);
  }

  virtual void SetMangledName(const char *MN, bool Force = false) override {
    ASTIdentifierNode::SetMangledName(MN, Force);
  }

  virtual void SetMangledName(const char *MN,
                              bool Force = false) const override {
    ASTIdentifierNode::SetMangledName(MN, Force);
  }

  virtual void SetArraySubscriptNode(const ASTArraySubscriptNode *SN);

  virtual void SetArraySubscriptList(const ASTArraySubscriptList *SL) {
    ASL = SL;
  }

  virtual const ASTArraySubscriptNode *GetArraySubscriptNode() const {
    return ASN;
  }

  virtual const ASTArraySubscriptList *GetArraySubscriptList() const {
    return ASL;
  }

  virtual const ASTIdentifierNode *GetInductionVariable() const;

  virtual const ASTIdentifierNode *GetIndexedIdentifier() const;

  const std::string &GetInductionVariableIndexedName() const {
    if (this->IsInductionVariable())
      return IVX;

    return ASTStringUtils::Instance().EmptyString();
  }

  virtual bool NeedsEval() const override {
    return OpType != ASTOpTypeUndefined;
  }

  virtual const ASTBinaryOpNode *GetBinaryOp() const override {
    return EvalType == ASTTypeBinaryOp ? BOP : nullptr;
  }

  virtual const ASTUnaryOpNode *GetUnaryOp() const override {
    return EvalType == ASTTypeUnaryOp ? UOP : nullptr;
  }

  virtual bool operator<(const ASTIdentifierRefNode &RHS) const {
    return Name < RHS.Name;
  }

  virtual bool operator<(const ASTIdentifierNode &IId) const override {
    return Name < IId.Name;
  }

  virtual bool IsError() const override {
    return RTy == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return Id->GetError();
  }

  static ASTIdentifierRefNode *IdentifierError(const std::string &ERM) {
    return new ASTIdentifierRefNode(ERM, ASTTypeExpressionError,
                                    ASTIdentifierNode::IdentifierError(ERM),
                                    static_cast<unsigned>(~0x0), true);
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}

  static ASTIdentifierRefNode *MPInteger(const std::string &Id,
                                         unsigned NumBits);
  static ASTIdentifierRefNode *MPDecimal(const std::string &Id,
                                         unsigned NumBits);
};

class ASTTimedIdentifierNode : public ASTIdentifierNode {
private:
  void ParseDuration(const std::string &DU);

protected:
  uint64_t Duration;
  LengthUnit Units;

public:
  ASTTimedIdentifierNode(const std::string &Id, const std::string &DU,
                         unsigned B = ~0x0)
      : ASTIdentifierNode(Id, B), Duration(static_cast<uint64_t>(~0x0UL)),
        Units(LengthUnspecified) {
    ParseDuration(DU);
  }

  ASTTimedIdentifierNode(const std::string &Id, ASTType Ty, unsigned B = ~0x0)
      : ASTIdentifierNode(Id, Ty, B), Duration(static_cast<uint64_t>(~0x0UL)),
        Units(LengthUnspecified) {}

  ASTTimedIdentifierNode(const std::string &Id, ASTType Ty, uint64_t D,
                         LengthUnit LU, unsigned B = ~0x0)
      : ASTIdentifierNode(Id, Ty, B), Duration(D), Units(LU) {}

  virtual ~ASTTimedIdentifierNode() = default;

  virtual uint64_t GetDuration() const { return Duration; }

  virtual LengthUnit GetUnits() const { return Units; }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTIdentifierList : public ASTBase {
  friend class ASTIdentifierBuilder;
  friend class ASTGateQubitParamBuilder;

protected:
  std::vector<ASTIdentifierNode *> Graph;
  std::set<uint64_t> Hash;

public:
  using list_type = std::vector<ASTIdentifierNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTIdentifierList() : ASTBase(), Graph(), Hash() {}

  ASTIdentifierList(const ASTIdentifierList &RHS)
      : ASTBase(), Graph(RHS.Graph), Hash(RHS.Hash) {}

  virtual ~ASTIdentifierList() = default;

  ASTIdentifierList &operator=(const ASTIdentifierList &RHS) {
    if (this != &RHS) {
      Graph = RHS.Graph;
      Hash = RHS.Hash;
    }

    return *this;
  }

  virtual void CheckOutOfScope() const;

  virtual bool CheckOnlyQubits() const;

  virtual size_t Size() const { return Graph.size(); }

  virtual void Clear() {
    Graph.clear();
    Hash.clear();
  }

  virtual bool Empty() const { return Size() == 0; }

  virtual void Append(ASTIdentifierNode *Id) {
    if (Id) {
      if (Hash.insert(Id->GetHash()).second)
        Graph.push_back(Id);
    }
  }

  virtual void Prepend(ASTIdentifierNode *Id) {
    if (Id) {
      if (Hash.insert(Id->GetHash()).second)
        Graph.insert(Graph.begin(), Id);
    }
  }

  virtual void Erase(ASTIdentifierNode *Id) {
    if (Id) {
      size_t H = Id->GetHash();

      for (iterator I = Graph.begin(); I != Graph.end(); ++I) {
        if ((*I)->GetHash() == H) {
          Graph.erase(I);
          Hash.erase(H);
          break;
        }
      }
    }
  }

  iterator begin() { return Graph.begin(); }

  const_iterator begin() const { return Graph.begin(); }

  iterator end() { return Graph.end(); }

  const_iterator end() const { return Graph.end(); }

  ASTIdentifierNode *front() { return Graph.front() ? Graph.front() : nullptr; }

  const ASTIdentifierNode *front() const {
    return Graph.front() ? Graph.front() : nullptr;
  }

  ASTIdentifierNode *back() { return Graph.back() ? Graph.back() : nullptr; }

  const ASTIdentifierNode *back() const {
    return Graph.back() ? Graph.back() : nullptr;
  }

  virtual ASTType GetASTType() const { return ASTTypeIdentifierList; }

  virtual ASTIdentifierNode *operator[](size_t Index) {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index];
  }

  virtual const ASTIdentifierNode *operator[](size_t Index) const {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index];
  }

  virtual void DeleteSymbols() const;

  virtual void print() const {
    std::cout << "<IdentifierList>" << std::endl;

    for (std::vector<ASTIdentifierNode *>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I) {
      (*I)->print();
    }

    std::cout << "</IdentifierList>" << std::endl;
  }

  virtual void push(ASTBase *Node) {
    if (ASTIdentifierNode *ID = dynamic_cast<ASTIdentifierNode *>(Node)) {
      if (Hash.find(ID->GetHash()) == Hash.end()) {
        Graph.push_back(ID);
        Hash.insert(ID->GetHash());
      }
    }
  }
};

class ASTIdentifierRefList {
private:
  std::list<ASTIdentifierRefNode *> Graph;

public:
  ASTIdentifierRefList() : Graph() {}

  ASTIdentifierRefList(const ASTIdentifierRefList &RHS) : Graph(RHS.Graph) {}

  virtual ~ASTIdentifierRefList() = default;

  ASTIdentifierRefList &operator=(const ASTIdentifierRefList &RHS) {
    if (this != &RHS)
      Graph = RHS.Graph;

    return *this;
  }

  virtual ASTType GetASTType() const { return ASTTypeIdentifierRefList; }

  virtual void print() const {
    std::cout << "<IdentifierRefList>" << std::endl;

    for (std::list<ASTIdentifierRefNode *>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I) {
      (*I)->print();
    }

    std::cout << "</IdentifierRefList>" << std::endl;
  }

  virtual void push(ASTBase *Node) {
    Graph.push_back(dynamic_cast<ASTIdentifierRefNode *>(Node));
  }
};

} // namespace QASM

#endif // __QASM_AST_IDENTIFIER_NODE_H
