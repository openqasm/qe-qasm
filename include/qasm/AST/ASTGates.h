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

#ifndef __QASM_AST_GATE_NODES_H
#define __QASM_AST_GATE_NODES_H

#include <qasm/AST/ASTAngleNodeList.h>
#include <qasm/AST/ASTAnyTypeList.h>
#include <qasm/AST/ASTArgument.h>
#include <qasm/AST/ASTArray.h>
#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTGateOpList.h>
#include <qasm/AST/ASTGateTemplateParamBuilder.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTParameterList.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTQubitNodeBuilder.h>
#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace QASM {

class ASTSymbolTableEntry;

/// One gate parameter (classical) in source/declaration order.
struct ASTGateParam {
  ASTType Ty;
  ASTExpressionNode *Expr;

  ASTGateParam() : Ty(ASTTypeUndefined), Expr(nullptr) {}
  ASTGateParam(ASTType T, ASTExpressionNode *E) : Ty(T), Expr(E) {}
};

class ASTGateNode : public ASTExpressionNode {
  friend class ASTGateControlNode;
  friend class ASTGateNegControlNode;
  friend class ASTGateInverseNode;
  friend class ASTGatePowerNode;

protected:
  /// Parameters in source order (angles, complex, arrays, …).
  /// Quantum operands live in Operands / OperandParams.
  std::vector<ASTGateParam> Params;
  std::vector<ASTQubitNode *> Operands;
  std::vector<const ASTSymbolTableEntry *> OperandParams;
  std::map<unsigned, const ASTIdentifierNode *> OperandParamIds;
  ASTGateQOpList OpList;
  union {
    mutable const ASTGateControlNode *Ctrl;
    mutable const ASTGateNegControlNode *NegCtrl;
    mutable const ASTGateInverseNode *Inv;
    mutable const ASTGatePowerNode *Pow;
    mutable const void *Void;
  };

  const ASTIdentifierNode *GDId;
  std::map<std::string, const ASTSymbolTableEntry *> GSTM;
  mutable ASTType ControlType;
  bool Opaque;
  bool GateCall;
  /// True for ProductionRule_10030 fully-typed gate declarations.
  bool FullyTyped;
  /// Declared classical formal types in order (fully-typed gates only).
  std::vector<ASTType> FormalParamTypes;
  /// Parallel to FormalParamTypes: array length when known, else 0.
  std::vector<unsigned> FormalParamArraySizes;
  /// Declared quantum formal kinds (Qubit/Qumode) in order.
  std::vector<ASTType> FormalQuantumTypes;
  /// Gate template parameters (`uint N`, …) from `gate foo[…](…)`.
  /// Decl: Bound unset (NaN analogue). Call: Bound set; body `N` left alone.
  std::vector<ASTGateTemplateParam> TemplateParams;
  /// Parallel to FormalParamTypes: template index when array size is symbolic,
  /// else ~0U.
  std::vector<unsigned> FormalParamArraySizeTemplateIndices;

private:
  ASTGateNode() = delete;

private:
  void ToGateParamSymbolTable(const ASTIdentifierNode *Id,
                              const ASTSymbolTableEntry *STE);
  ASTSymbolTableEntry *MangleGateOperandParam(ASTIdentifierNode *Id,
                                              ASTSymbolTableEntry *&STE,
                                              unsigned IX, unsigned Bits = 0U,
                                              unsigned QBits = 0U);

  void MaterializeGateOperandParam(ASTIdentifierNode *Id);

  void MaterializeBuiltinUGate(const ASTIdentifierNode *GId,
                               const ASTParameterList &PL,
                               const ASTIdentifierList &IL);

  /// Resolve qubit vs qumode for Operands[Index] from FormalQuantumTypes,
  /// then the formal's polymorphic/symbol type. Defaults to qubit.
  ASTType ResolveOperandQuantumType(const ASTIdentifierNode *QId,
                                    unsigned Index) const;

  ASTAngleNode *CreateAngleConversion(const ASTSymbolTableEntry *XSTE) const;

  ASTAngleNode *CreateAngleTemporary(const ASTSymbolTableEntry *XSTE) const;

  ASTAngleNode *CreateAngleSymbolTableEntry(ASTSymbolTableEntry *XSTE) const;

  /// Build an MPComplex Param that retains BinaryOp/UnaryOp as Expr
  /// (no Evaluate — symbolic ops are not real/imag literals).
  ASTMPComplexNode *MaterializeComplexParamExpr(unsigned Index,
                                                const ASTExpressionNode *EN);

  /// True when call formal Index is complex (FormalParamTypes or builtin disp).
  bool FormalWantsComplex(unsigned Index) const;

  /// True when call formal Index is float/double/mpdecimal scalar.
  bool FormalWantsFloatScalar(unsigned Index) const;

  /// True when call formal Index is float/mpdecimal array.
  bool FormalWantsFloatArray(unsigned Index) const;

  /// Store a real scalar as complex real+0i for a complex formal.
  ASTMPComplexNode *MaterializeComplexFromReal(unsigned Index,
                                               const ASTMPDecimalNode *R);

  /// Convert a ProductionRule_10010 AngleArray literal to MPDecimalArray
  /// when the formal wants a float/mpdecimal array.
  ASTMPDecimalArrayNode *
  MaterializeMPDecimalArrayFromAngles(unsigned Index,
                                      const ASTAngleArrayNode *AAN);

  template <typename NodeTy>
  NodeTy *GetParamOfType(ASTType Ty, unsigned Index) const {
    unsigned Seen = 0;
    for (std::size_t I = 0; I < Params.size(); ++I) {
      if (Params[I].Ty != Ty)
        continue;
      if (Seen == Index)
        return dynamic_cast<NodeTy *>(Params[I].Expr);
      ++Seen;
    }
    return nullptr;
  }

  unsigned CountParamType(ASTType Ty) const {
    unsigned N = 0;
    for (std::size_t I = 0; I < Params.size(); ++I)
      if (Params[I].Ty == Ty)
        ++N;
    return N;
  }

protected:
  ASTIdentifierNode *GateCallIdentifier(const std::string &Name, ASTType Type,
                                        unsigned Bits) const;

public:
  using list_type = std::vector<ASTQubitNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static const unsigned GateBits = 64U;

public:
  ASTGateNode(const ASTIdentifierNode *Id)
      : ASTExpressionNode(Id, ASTTypeGate), Params(), Operands(),
        OperandParams(), OpList(), Ctrl(nullptr), GDId(Id), GSTM(),
        ControlType(ASTTypeUndefined), Opaque(false), GateCall(false),
        FullyTyped(false), FormalParamTypes(), FormalParamArraySizes(),
        FormalQuantumTypes(), TemplateParams(),
        FormalParamArraySizeTemplateIndices() {}

  // Implemented in ASTGates.cpp
  ASTGateNode(const ASTIdentifierNode *Id, const ASTArgumentNodeList &AL,
              const ASTAnyTypeList &QL, bool IsGateCall = false,
              const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault,
              const std::vector<ASTType> *CallFormalParamTypes = nullptr);

  // Implemented in ASTGates.cpp
  ASTGateNode(const ASTIdentifierNode *Id, const ASTParameterList &PL,
              const ASTIdentifierList &IL, bool IsGateCall = false,
              const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault);

  virtual ~ASTGateNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGate; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void ClearLocalGateSymbols() const;

  virtual bool IsCall() const { return GateCall; }

  virtual unsigned GetNumOperands() const {
    return static_cast<unsigned>(Operands.size());
  }

  virtual unsigned GetNumParams() const {
    return static_cast<unsigned>(Params.size());
  }

  virtual const std::vector<ASTGateParam> &GetParams() const { return Params; }

  virtual std::vector<ASTGateParam> &GetParams() { return Params; }

  virtual void AddParam(ASTType Ty, ASTExpressionNode *E) {
    assert(E && "Invalid gate parameter!");
    Params.emplace_back(Ty, E);
  }

  virtual unsigned GetNumArrayParams() const {
    return CountParamType(ASTTypeAngleArray);
  }

  virtual const ASTAngleArrayNode *GetArrayParam(unsigned Index) const {
    const ASTAngleArrayNode *A =
        GetParamOfType<ASTAngleArrayNode>(ASTTypeAngleArray, Index);
    assert(A && "Index is out-of-range!");
    return A;
  }

  virtual ASTAngleArrayNode *GetArrayParam(unsigned Index) {
    ASTAngleArrayNode *A =
        GetParamOfType<ASTAngleArrayNode>(ASTTypeAngleArray, Index);
    assert(A && "Index is out-of-range!");
    return A;
  }

  virtual unsigned GetNumComplexArrayParams() const {
    return CountParamType(ASTTypeMPComplexArray);
  }

  virtual const ASTMPComplexArrayNode *
  GetComplexArrayParam(unsigned Index) const {
    const ASTMPComplexArrayNode *A =
        GetParamOfType<ASTMPComplexArrayNode>(ASTTypeMPComplexArray, Index);
    assert(A && "Index is out-of-range!");
    return A;
  }

  virtual ASTMPComplexArrayNode *GetComplexArrayParam(unsigned Index) {
    ASTMPComplexArrayNode *A =
        GetParamOfType<ASTMPComplexArrayNode>(ASTTypeMPComplexArray, Index);
    assert(A && "Index is out-of-range!");
    return A;
  }

  virtual unsigned GetNumComplexParams() const {
    return CountParamType(ASTTypeMPComplex);
  }

  virtual const ASTMPComplexNode *GetComplexParam(unsigned Index) const {
    const ASTMPComplexNode *C =
        GetParamOfType<ASTMPComplexNode>(ASTTypeMPComplex, Index);
    assert(C && "Index is out-of-range!");
    return C;
  }

  virtual ASTMPComplexNode *GetComplexParam(unsigned Index) {
    ASTMPComplexNode *C =
        GetParamOfType<ASTMPComplexNode>(ASTTypeMPComplex, Index);
    assert(C && "Index is out-of-range!");
    return C;
  }

  virtual unsigned GetNumOperandParams() const {
    return static_cast<unsigned>(OperandParams.size());
  }

  virtual unsigned GetNumOperandParamIds() const {
    return static_cast<unsigned>(OperandParamIds.size());
  }

  virtual unsigned GetNumGateOps() const {
    return static_cast<unsigned>(OpList.Size());
  }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const ASTIdentifierNode *GetGateDefinitionId() const { return GDId; }

  virtual std::map<std::string, const ASTSymbolTableEntry *> &GetSymbolTable() {
    return GSTM;
  }

  virtual const std::map<std::string, const ASTSymbolTableEntry *> &
  GetSymbolTable() const {
    return GSTM;
  }

  virtual const ASTSymbolTableEntry *GetSymbol(const std::string &SN) const {
    std::map<std::string, const ASTSymbolTableEntry *>::const_iterator I =
        GSTM.find(SN);
    return I == GSTM.end() ? nullptr : (*I).second;
  }

  virtual void ClearGateOperands() const;

  static GateKind DetermineGateKind(const std::string &GN);

  static ASTType DetermineGateType(const std::string &GN);

  virtual ASTGateNode *CloneCall(const ASTIdentifierNode *Id,
                                 const ASTArgumentNodeList &AL,
                                 const ASTAnyTypeList &QL);

  virtual ASTSymbolTableEntry *GetSymbolTableEntry(const std::string &S) {
    std::map<std::string, const ASTSymbolTableEntry *>::iterator MI =
        GSTM.find(S);
    return MI == GSTM.end() ? nullptr
                            : const_cast<ASTSymbolTableEntry *>((*MI).second);
  }

  virtual const ASTSymbolTableEntry *
  GetSymbolTableEntry(const std::string &S) const {
    std::map<std::string, const ASTSymbolTableEntry *>::const_iterator MI =
        GSTM.find(S);
    return MI == GSTM.end() ? nullptr : (*MI).second;
  }

  virtual void SetControl(const ASTGateControlNode *C) const {
    assert(C && "Invalid ASTGateControlNode argument!");
    Ctrl = C;
    ControlType = ASTTypeGateControl;
  }

  virtual void SetNegControl(const ASTGateNegControlNode *NC) const {
    assert(NC && "Invalid ASTGateNegControlNode argument!");
    NegCtrl = NC;
    ControlType = ASTTypeGateNegControl;
  }

  virtual void SetInverse(const ASTGateInverseNode *I) const {
    assert(I && "Invalid ASTGateInverseNode argument!");
    Inv = I;
    ControlType = ASTTypeGateInverse;
  }

  virtual void SetPower(const ASTGatePowerNode *P) const {
    assert(P && "Invalid ASTGatePowerNode argument!");
    Pow = P;
    ControlType = ASTTypeGatePower;
  }

  virtual void SetOpaque(bool O = true) { Opaque = O; }

  virtual void SetFullyTyped(bool FT = true) { FullyTyped = FT; }

  virtual bool IsFullyTyped() const { return FullyTyped; }

  virtual void SetFormalParamTypes(const std::vector<ASTType> &Tys) {
    FormalParamTypes = Tys;
  }

  virtual const std::vector<ASTType> &GetFormalParamTypes() const {
    return FormalParamTypes;
  }

  virtual void SetFormalParamArraySizes(const std::vector<unsigned> &SZs) {
    FormalParamArraySizes = SZs;
  }

  virtual const std::vector<unsigned> &GetFormalParamArraySizes() const {
    return FormalParamArraySizes;
  }

  virtual void SetFormalQuantumTypes(const std::vector<ASTType> &Tys) {
    FormalQuantumTypes = Tys;
  }

  virtual const std::vector<ASTType> &GetFormalQuantumTypes() const {
    return FormalQuantumTypes;
  }

  virtual void SetTemplateParams(const std::vector<ASTGateTemplateParam> &TPs) {
    TemplateParams = TPs;
  }

  virtual const std::vector<ASTGateTemplateParam> &GetTemplateParams() const {
    return TemplateParams;
  }

  /// Attach call-site bindings without mutating body identifiers named `N`.
  /// Parallel to replacing Params with call args while leaving GateQOpList
  /// formals as placeholders (angles use NaN; template formals stay unbound).
  virtual void
  SetTemplateParamBounds(const std::vector<std::optional<unsigned>> &Bounds) {
    for (std::size_t I = 0; I < TemplateParams.size() && I < Bounds.size(); ++I)
      if (Bounds[I])
        TemplateParams[I].Bound = *Bounds[I];
  }

  virtual void
  SetFormalParamArraySizeTemplateIndices(const std::vector<unsigned> &Idxs) {
    FormalParamArraySizeTemplateIndices = Idxs;
  }

  virtual const std::vector<unsigned> &
  GetFormalParamArraySizeTemplateIndices() const {
    return FormalParamArraySizeTemplateIndices;
  }

  virtual bool HasControl() const {
    return ControlType == ASTTypeGateControl && Ctrl != nullptr;
  }

  virtual bool HasNegControl() const {
    return ControlType == ASTTypeGateNegControl && NegCtrl != nullptr;
  }

  virtual bool HasInverse() const {
    return ControlType == ASTTypeGateInverse && Inv != nullptr;
  }

  virtual bool HasPower() const {
    return ControlType == ASTTypeGatePower && Pow != nullptr;
  }

  virtual bool IsOpaque() const { return Opaque; }

  virtual const ASTGateControlNode *GetControl() const {
    return ControlType == ASTTypeGateControl ? Ctrl : nullptr;
  }

  virtual const ASTGateNegControlNode *GetNegControl() const {
    return ControlType == ASTTypeGateNegControl ? NegCtrl : nullptr;
  }

  virtual const ASTGateInverseNode *GetInverse() const {
    return ControlType == ASTTypeGateInverse ? Inv : nullptr;
  }

  virtual const ASTGatePowerNode *GetPower() const {
    return ControlType == ASTTypeGatePower ? Pow : nullptr;
  }

  virtual ASTType GetControlType() const { return ControlType; }

  virtual void AddOperand(ASTQubitNode *QN) {
    assert(QN && "Invalid Qubit argument!");
    Operands.push_back(QN);
  }

  virtual void AddParam(ASTAngleNode *A) {
    assert(A && "Invalid Angle argument!");
    AddParam(ASTTypeAngle, A);
  }

  virtual void AddOpList(const ASTGateQOpList &OL) { OpList = OL; }

  virtual const ASTGateQOpList &GetOpList() const { return OpList; }

  virtual bool HasOpList() const { return !OpList.Empty(); }

  iterator operands_begin() { return Operands.begin(); }

  const_iterator operands_begin() const { return Operands.begin(); }

  iterator operands_end() { return Operands.end(); }

  const_iterator operands_end() const { return Operands.end(); }

  virtual std::size_t ParamsSize() const {
    return CountParamType(ASTTypeAngle);
  }

  virtual std::size_t OperandsSize() const { return Operands.size(); }

  virtual const ASTQubitNode *operands_front() const {
    return Operands.front();
  }

  virtual const ASTQubitNode *operands_back() const { return Operands.back(); }

  virtual const ASTQubitNode *GetOperand(const ASTIdentifierNode *Id) const {
    assert(Id && "Invalid Qubit Identifier!");

    for (std::vector<ASTQubitNode *>::const_iterator I = Operands.begin();
         I != Operands.end(); ++I) {
      if (*(*I)->GetIdentifier() == *Id)
        return *I;
    }

    return nullptr;
  }

  virtual const ASTAngleNode *GetAngle(const ASTIdentifierNode *Id) const {
    assert(Id && "Invalid Param Identifier!");

    for (std::size_t I = 0; I < Params.size(); ++I) {
      if (Params[I].Ty != ASTTypeAngle)
        continue;
      ASTAngleNode *AN = dynamic_cast<ASTAngleNode *>(Params[I].Expr);
      if (AN && *(*AN).GetIdentifier() == *Id)
        return AN;
    }

    return nullptr;
  }

  virtual const ASTQubitNode *GetOperand(unsigned Index) const {
    assert(Index < Operands.size() && "Index is out-of-range!");
    return Operands[Index];
  }

  virtual const ASTAngleNode *GetParam(unsigned Index) const {
    const ASTAngleNode *A = GetParamOfType<ASTAngleNode>(ASTTypeAngle, Index);
    assert(A && "Index is out-of-range!");
    return A;
  }

  virtual const std::vector<const ASTSymbolTableEntry *> &
  GetOperandParams() const {
    return OperandParams;
  }

  virtual const std::map<unsigned, const ASTIdentifierNode *> &
  GetOperandParamIds() const {
    return OperandParamIds;
  }

  virtual bool HasInductionVariableOperands() const {
    return !OperandParamIds.empty();
  }

  // Implemented in ASTGates.cpp.
  virtual void print() const override;

  virtual void push(ASTBase *Node) override {
    AddOperand(dynamic_cast<ASTQubitNode *>(Node));
  }
};

class ASTCNotGateNode : public ASTGateNode {
private:
  ASTCNotGateNode() = delete;

public:
  ASTCNotGateNode(const ASTIdentifierNode *Id) : ASTGateNode(Id) {}

  ASTCNotGateNode(const ASTIdentifierNode *Id, const ASTArgumentNodeList &AL,
                  const ASTAnyTypeList &TL, bool IsGateCall = false,
                  const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault)
      : ASTGateNode(Id, AL, TL, IsGateCall, OL) {}

  ASTCNotGateNode(const ASTIdentifierNode *Id, const ASTParameterList &PL,
                  const ASTIdentifierList &IL, bool IsGateCall = false,
                  const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault)
      : ASTGateNode(Id, PL, IL, IsGateCall, OL) {}

  virtual ~ASTCNotGateNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeCNotGate; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTCNotGateNode *CloneCall(const ASTIdentifierNode *Id,
                                     const ASTArgumentNodeList &AL,
                                     const ASTAnyTypeList &QL) override;

  virtual void print() const override {
    std::cout << "<CNotGate>" << std::endl;
    ASTGateNode::print();
    std::cout << "</CNotGate>" << std::endl;
  }
};

class ASTHadamardGateNode : public ASTGateNode {
private:
  ASTHadamardGateNode() = delete;

public:
  ASTHadamardGateNode(const ASTIdentifierNode *Id) : ASTGateNode(Id) {}

  ASTHadamardGateNode(const ASTIdentifierNode *Id,
                      const ASTArgumentNodeList &AL, const ASTAnyTypeList &TL,
                      bool IsGateCall = false,
                      const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault)
      : ASTGateNode(Id, AL, TL, IsGateCall, OL) {}

  ASTHadamardGateNode(const ASTIdentifierNode *Id, const ASTParameterList &PL,
                      const ASTIdentifierList &IL, bool IsGateCall,
                      const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault)
      : ASTGateNode(Id, PL, IL, IsGateCall, OL) {}

  virtual ~ASTHadamardGateNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeHadamardGate; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTHadamardGateNode *CloneCall(const ASTIdentifierNode *Id,
                                         const ASTArgumentNodeList &AL,
                                         const ASTAnyTypeList &QL) override;

  virtual void print() const override {
    std::cout << "<HadamardGate>" << std::endl;
    ASTGateNode::print();
    std::cout << "</HadamardGate>" << std::endl;
  }
};

class ASTCCXGateNode : public ASTGateNode {
private:
  ASTCCXGateNode() = delete;

public:
  ASTCCXGateNode(const ASTIdentifierNode *Id) : ASTGateNode(Id) {}

  ASTCCXGateNode(const ASTIdentifierNode *Id, const ASTArgumentNodeList &AL,
                 const ASTAnyTypeList &TL, bool IsGateCall = false,
                 const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault)
      : ASTGateNode(Id, AL, TL, IsGateCall, OL) {}

  ASTCCXGateNode(const ASTIdentifierNode *Id, const ASTParameterList &PL,
                 const ASTIdentifierList &IL, bool IsGateCall = false,
                 const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault)
      : ASTGateNode(Id, PL, IL, IsGateCall, OL) {}

  virtual ~ASTCCXGateNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeCCXGate; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTCCXGateNode *CloneCall(const ASTIdentifierNode *Id,
                                    const ASTArgumentNodeList &AL,
                                    const ASTAnyTypeList &QL) override;

  virtual ASTCCXGateNode *CloneCall(const ASTIdentifierNode *Id,
                                    const ASTParameterList &PL,
                                    const ASTIdentifierList &IL);

  virtual void print() const override {
    std::cout << "<CCXGate>" << std::endl;
    ASTGateNode::print();
    std::cout << "</CCXGate>" << std::endl;
  }
};

class ASTCXGateNode : public ASTGateNode {
private:
  ASTCXGateNode() = delete;

public:
  ASTCXGateNode(const ASTIdentifierNode *Id) : ASTGateNode(Id) {}

  ASTCXGateNode(const ASTIdentifierNode *Id, const ASTArgumentNodeList &AL,
                const ASTAnyTypeList &TL, bool IsGateCall = false,
                const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault)
      : ASTGateNode(Id, AL, TL, IsGateCall, OL) {}

  ASTCXGateNode(const ASTIdentifierNode *Id, const ASTParameterList &PL,
                const ASTIdentifierList &IL, bool IsGateCall = false,
                const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault)
      : ASTGateNode(Id, PL, IL, IsGateCall, OL) {}

  virtual ~ASTCXGateNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeCXGate; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTCXGateNode *CloneCall(const ASTIdentifierNode *Id,
                                   const ASTArgumentNodeList &AL,
                                   const ASTAnyTypeList &QL) override;

  virtual ASTCXGateNode *CloneCall(const ASTIdentifierNode *Id,
                                   const ASTParameterList &PL,
                                   const ASTIdentifierList &IL);

  virtual void print() const override {
    std::cout << "<CXGate>" << std::endl;
    ASTGateNode::print();
    std::cout << "</CXGate>" << std::endl;
  }
};

class ASTUGateNode : public ASTGateNode {
private:
  bool Identity;

private:
  ASTUGateNode() = delete;

public:
  ASTUGateNode(const ASTIdentifierNode *Id, const ASTArgumentNodeList &AL,
               const ASTAnyTypeList &TL, bool IsGateCall = false,
               const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault,
               bool IsIdent = false)
      : ASTGateNode(Id, AL, TL, IsGateCall, OL), Identity(IsIdent) {}

  ASTUGateNode(const ASTIdentifierNode *Id, const ASTParameterList &PL,
               const ASTIdentifierList &IL, bool IsGateCall = false,
               const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault)
      : ASTGateNode(Id, PL, IL, IsGateCall, OL) {}

  virtual ~ASTUGateNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeUGate; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTUGateNode *CloneCall(const ASTIdentifierNode *Id,
                                  const ASTArgumentNodeList &AL,
                                  const ASTAnyTypeList &QL) override;

  virtual bool IsIdentity() const { return Identity; }

  virtual void SetIdentity(bool BV) { Identity = BV; }

  virtual void print() const override {
    std::cout << "<UGate>" << std::endl;
    ASTGateNode::print();
    std::cout << "</UGate>" << std::endl;
  }
};

class ASTDispGateNode : public ASTGateNode {
private:
  ASTDispGateNode() = delete;

public:
  ASTDispGateNode(const ASTIdentifierNode *Id, const ASTArgumentNodeList &AL,
                  const ASTAnyTypeList &TL, bool IsGateCall = false,
                  const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault)
      : ASTGateNode(Id, AL, TL, IsGateCall, OL) {}

  ASTDispGateNode(const ASTIdentifierNode *Id, const ASTParameterList &PL,
                  const ASTIdentifierList &IL, bool IsGateCall = false,
                  const ASTGateQOpList &OL = ASTGateQOpList::EmptyDefault)
      : ASTGateNode(Id, PL, IL, IsGateCall, OL) {}

  virtual ~ASTDispGateNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeDispGate; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTDispGateNode *CloneCall(const ASTIdentifierNode *Id,
                                     const ASTArgumentNodeList &AL,
                                     const ASTAnyTypeList &QL) override;

  virtual void print() const override {
    std::cout << "<DispGate>" << std::endl;
    ASTGateNode::print();
    std::cout << "</DispGate>" << std::endl;
  }
};

class ASTGateDeclarationNode : public ASTStatementNode {
private:
  ASTGateDeclarationNode() = delete;

protected:
  ASTGateDeclarationNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)) {}

public:
  ASTGateDeclarationNode(const ASTIdentifierNode *Id, const ASTGateNode *G)
      : ASTStatementNode(Id, G) {
    assert(Id && "Invalid ASTIdentifierNode!");
    assert(G && "Invalid ASTGateNode!");
  }

  virtual ~ASTGateDeclarationNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGateDeclaration; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeDeclaration;
  }

  virtual bool DuplicatesAllowed() const { return true; }

  virtual const ASTGateNode *GetGateNode() const {
    return dynamic_cast<const ASTGateNode *>(ASTStatementNode::Expr);
  }

  virtual void print() const override {
    std::cout << "<GateDeclarationNode>" << std::endl;
    const ASTGateNode *GN =
        dynamic_cast<const ASTGateNode *>(ASTStatementNode::Expr);
    GN->print();
    std::cout << "</GateDeclarationNode>" << std::endl;
  }

  static ASTGateDeclarationNode *DeclarationError(const ASTIdentifierNode *Id,
                                                  const std::string &EM) {
    return new ASTGateDeclarationNode(Id, EM);
  }

  virtual bool IsError() const override { return ASTStatementNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  virtual void push(ASTBase * /* unused*/) override {}
};

} // namespace QASM

#endif // __QASM_AST_GATE_NODES_H
