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

#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTAnyTypeList.h>
#include <qasm/AST/ASTParameterList.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTQubitNodeBuilder.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTArgument.h>
#include <qasm/AST/ASTAngleNodeList.h>
#include <qasm/AST/ASTGateOpList.h>

#include <vector>
#include <string>
#include <map>
#include <cassert>

namespace QASM {

class ASTSymbolTableEntry;

class ASTGateNode : public ASTExpressionNode {
  friend class ASTGateControlNode;
  friend class ASTGateNegControlNode;
  friend class ASTGateInverseNode;
  friend class ASTGatePowerNode;

protected:
  std::vector<ASTAngleNode*> Params;
  std::vector<ASTQubitNode*> Qubits;
  std::vector<const ASTSymbolTableEntry*> QCParams;
  std::map<unsigned, const ASTIdentifierNode*> QCParamIds;
  ASTGateQOpList OpList;
  union {
    mutable const ASTGateControlNode* Ctrl;
    mutable const ASTGateNegControlNode* NegCtrl;
    mutable const ASTGateInverseNode* Inv;
    mutable const ASTGatePowerNode* Pow;
    mutable const void* Void;
  };

  const ASTIdentifierNode* GDId;
  std::map<std::string, const ASTSymbolTableEntry*> GSTM;
  mutable ASTType ControlType;
  bool Opaque;
  bool GateCall;

private:
  ASTGateNode() = delete;

private:
  void ToGateParamSymbolTable(const ASTIdentifierNode* Id,
                              const ASTSymbolTableEntry* STE);
  ASTSymbolTableEntry* MangleGateQubitParam(ASTIdentifierNode* Id,
                                            ASTSymbolTableEntry*& STE,
                                            unsigned IX,
                                            unsigned Bits = 0U,
                                            unsigned QBits = 0U);

  void MaterializeGateQubitParam(ASTIdentifierNode* Id);

  void MaterializeBuiltinUGate(const ASTIdentifierNode* GId,
                               const ASTParameterList& PL,
                               const ASTIdentifierList& IL);

  ASTAngleNode* CreateAngleConversion(const ASTSymbolTableEntry* XSTE) const;

  ASTAngleNode* CreateAngleTemporary(const ASTSymbolTableEntry* XSTE) const;

  ASTAngleNode* CreateAngleSymbolTableEntry(ASTSymbolTableEntry* XSTE) const;

protected:
  ASTIdentifierNode* GateCallIdentifier(const std::string& Name,
                                        ASTType Type, unsigned Bits) const;

public:
  using list_type = std::vector<ASTQubitNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static const unsigned GateBits = 64U;

public:
  ASTGateNode(const ASTIdentifierNode* Id)
  : ASTExpressionNode(Id, ASTTypeGate), Params(), Qubits(),
  QCParams(), OpList(), Ctrl(nullptr), GDId(Id), GSTM(),
  ControlType(ASTTypeUndefined), Opaque(false), GateCall(false) { }

  // Implemented in ASTGates.cpp
  ASTGateNode(const ASTIdentifierNode* Id,
              const ASTArgumentNodeList& AL,
              const ASTAnyTypeList& QL,
              bool IsGateCall = false,
              const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault);

  // Implemented in ASTGates.cpp
  ASTGateNode(const ASTIdentifierNode* Id,
              const ASTParameterList& PL,
              const ASTIdentifierList& IL,
              bool IsGateCall = false,
              const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault);

  virtual ~ASTGateNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeGate;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual void ClearLocalGateSymbols() const;

  virtual bool IsCall() const {
    return GateCall;
  }

  virtual unsigned GetNumQubits() const {
    return static_cast<unsigned>(Qubits.size());
  }

  virtual unsigned GetNumParams() const {
    return static_cast<unsigned>(Params.size());
  }

  virtual unsigned GetNumQCParams() const {
    return static_cast<unsigned>(QCParams.size());
  }

  virtual unsigned GetNumQCParamIds() const {
    return static_cast<unsigned>(QCParamIds.size());
  }

  virtual unsigned GetNumGateOps() const {
    return static_cast<unsigned>(OpList.Size());
  }

  virtual const ASTIdentifierNode* GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const ASTIdentifierNode* GetGateDefinitionId() const {
    return GDId;
  }

  virtual std::map<std::string, const ASTSymbolTableEntry*>& GetSymbolTable() {
    return GSTM;
  }

  virtual const std::map<std::string, const ASTSymbolTableEntry*>&
  GetSymbolTable() const {
    return GSTM;
  }

  virtual const ASTSymbolTableEntry* GetSymbol(const std::string& SN) const {
    std::map<std::string, const ASTSymbolTableEntry*>::const_iterator I =
      GSTM.find(SN);
    return I == GSTM.end() ? nullptr : (*I).second;
  }

  virtual void ClearGateQubits() const;

  static GateKind DetermineGateKind(const std::string& GN);

  static ASTType DetermineGateType(const std::string& GN);

  virtual ASTGateNode* CloneCall(const ASTIdentifierNode* Id,
                                 const ASTArgumentNodeList& AL,
                                 const ASTAnyTypeList& QL);

  virtual ASTSymbolTableEntry* GetSymbolTableEntry(const std::string& S) {
    std::map<std::string, const ASTSymbolTableEntry*>::iterator MI =
      GSTM.find(S);
    return MI == GSTM.end() ? nullptr : const_cast<ASTSymbolTableEntry*>((*MI).second);
  }

  virtual const ASTSymbolTableEntry* GetSymbolTableEntry(const std::string& S) const {
    std::map<std::string, const ASTSymbolTableEntry*>::const_iterator MI =
      GSTM.find(S);
    return MI == GSTM.end() ? nullptr : (*MI).second;
  }

  virtual void SetControl(const ASTGateControlNode* C) const {
    assert(C && "Invalid ASTGateControlNode argument!");
    Ctrl = C;
    ControlType = ASTTypeGateControl;
  }

  virtual void SetNegControl(const ASTGateNegControlNode* NC) const {
    assert(NC && "Invalid ASTGateNegControlNode argument!");
    NegCtrl = NC;
    ControlType = ASTTypeGateNegControl;
  }

  virtual void SetInverse(const ASTGateInverseNode* I) const {
    assert(I && "Invalid ASTGateInverseNode argument!");
    Inv = I;
    ControlType = ASTTypeGateInverse;
  }

  virtual void SetPower(const ASTGatePowerNode* P) const {
    assert(P && "Invalid ASTGatePowerNode argument!");
    Pow = P;
    ControlType = ASTTypeGatePower;
  }

  virtual void SetOpaque(bool O = true) {
    Opaque = O;
  }

  virtual bool HasControl() const {
    return ControlType == ASTTypeGateControl &&
      Ctrl != nullptr;
  }

  virtual bool HasNegControl() const {
    return ControlType == ASTTypeGateNegControl &&
      NegCtrl != nullptr;
  }

  virtual bool HasInverse() const {
    return ControlType == ASTTypeGateInverse &&
      Inv != nullptr;
  }

  virtual bool HasPower() const {
    return ControlType == ASTTypeGatePower && Pow != nullptr;
  }

  virtual bool IsOpaque() const {
    return Opaque;
  }

  virtual const ASTGateControlNode* GetControl() const {
    return ControlType == ASTTypeGateControl ? Ctrl : nullptr;
  }

  virtual const ASTGateNegControlNode* GetNegControl() const {
    return ControlType == ASTTypeGateNegControl ? NegCtrl : nullptr;
  }

  virtual const ASTGateInverseNode* GetInverse() const {
    return ControlType == ASTTypeGateInverse ? Inv : nullptr;
  }

  virtual const ASTGatePowerNode* GetPower() const {
    return ControlType == ASTTypeGatePower ? Pow : nullptr;
  }

  virtual ASTType GetControlType() const {
    return ControlType;
  }

  virtual void AddQubit(ASTQubitNode* QN) {
    assert(QN && "Invalid Qubit argument!");
    Qubits.push_back(QN);
  }

  virtual void AddParam(ASTAngleNode* A) {
    assert(A && "Invalid Angle argument!");
    Params.push_back(A);
  }

  virtual void AddOpList(const ASTGateQOpList& OL) {
    OpList = OL;
  }

  virtual const ASTGateQOpList& GetOpList() const {
    return OpList;
  }

  virtual bool HasOpList() const {
    return !OpList.Empty();
  }

  iterator qubits_begin() {
    return Qubits.begin();
  }

  const_iterator qubits_begin() const {
    return Qubits.begin();
  }

  iterator qubits_end() {
    return Qubits.end();
  }

  const_iterator qubits_end() const {
    return Qubits.end();
  }

  virtual size_t ParamsSize() const {
    return Params.size();
  }

  virtual size_t QubitsSize() const {
    return Qubits.size();
  }

  virtual const ASTQubitNode* qubits_front() const {
    return Qubits.front();
  }

  virtual const ASTQubitNode* qubits_back() const {
    return Qubits.back();
  }

  virtual const ASTQubitNode* GetQubit(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid Qubit Identifier!");

    for (std::vector<ASTQubitNode*>::const_iterator I = Qubits.begin();
         I != Qubits.end(); ++I) {
      if (*(*I)->GetIdentifier() == *Id)
        return *I;
    }

    return nullptr;
  }

  virtual const ASTAngleNode* GetAngle(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid Param Identifier!");

    for (std::vector<ASTAngleNode*>::const_iterator I = Params.begin();
         I != Params.end(); ++I) {
      if (*(*I)->GetIdentifier() == *Id)
        return *I;
    }

    return nullptr;
  }

  virtual const ASTQubitNode* GetQubit(unsigned Index) const {
    assert(Index < Qubits.size() && "Index is out-of-range!");
    return Qubits[Index];
  }

  virtual const ASTAngleNode* GetParam(unsigned Index) const {
    assert(Index < Params.size() && "Index is out-of-range!");
    return Params[Index];
  }

  virtual const std::vector<const ASTSymbolTableEntry*>& GetQCParams() const {
    return QCParams;
  }

  virtual const std::map<unsigned, const ASTIdentifierNode*>&
  GetQCParamIds() const {
    return QCParamIds;
  }

  virtual bool HasInductionVariableQubits() const {
    return !QCParamIds.empty();
  }

  // Implemented in ASTGates.cpp.
  virtual void print() const override;

  virtual void push(ASTBase* Node) override {
    AddQubit(dynamic_cast<ASTQubitNode*>(Node));
  }
};

class ASTCNotGateNode : public ASTGateNode {
private:
  ASTCNotGateNode() = delete;

public:
  ASTCNotGateNode(const ASTIdentifierNode* Id) : ASTGateNode(Id) { }

  ASTCNotGateNode(const ASTIdentifierNode* Id,
                  const ASTArgumentNodeList& AL,
                  const ASTAnyTypeList& TL,
                  bool IsGateCall = false,
                  const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault)
  : ASTGateNode(Id, AL, TL, IsGateCall, OL) { }

  ASTCNotGateNode(const ASTIdentifierNode* Id,
                  const ASTParameterList& PL,
                  const ASTIdentifierList& IL,
                  bool IsGateCall = false,
                  const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault)
  : ASTGateNode(Id, PL, IL, IsGateCall, OL) { }

  virtual ~ASTCNotGateNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeCNotGate;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTCNotGateNode* CloneCall(const ASTIdentifierNode* Id,
                                     const ASTArgumentNodeList& AL,
                                     const ASTAnyTypeList& QL) override;

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
  ASTHadamardGateNode(const ASTIdentifierNode* Id)
  : ASTGateNode(Id) { }

  ASTHadamardGateNode(const ASTIdentifierNode* Id,
                      const ASTArgumentNodeList& AL,
                      const ASTAnyTypeList& TL,
                      bool IsGateCall = false,
                      const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault)
  : ASTGateNode(Id, AL, TL, IsGateCall, OL) { }

  ASTHadamardGateNode(const ASTIdentifierNode* Id,
                      const ASTParameterList& PL,
                      const ASTIdentifierList& IL,
                      bool IsGateCall,
                      const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault)
  : ASTGateNode(Id, PL, IL, IsGateCall, OL) { }

  virtual ~ASTHadamardGateNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeHadamardGate;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTHadamardGateNode* CloneCall(const ASTIdentifierNode* Id,
                                         const ASTArgumentNodeList& AL,
                                         const ASTAnyTypeList& QL) override;

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
  ASTCCXGateNode(const ASTIdentifierNode* Id)
  : ASTGateNode(Id) { }

  ASTCCXGateNode(const ASTIdentifierNode* Id,
                 const ASTArgumentNodeList& AL,
                 const ASTAnyTypeList& TL,
                 bool IsGateCall = false,
                 const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault)
  : ASTGateNode(Id, AL, TL, IsGateCall, OL) { }

  ASTCCXGateNode(const ASTIdentifierNode* Id,
                 const ASTParameterList& PL,
                 const ASTIdentifierList& IL,
                 bool IsGateCall = false,
                 const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault)
  : ASTGateNode(Id, PL, IL, IsGateCall, OL) { }

  virtual ~ASTCCXGateNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeCCXGate;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTCCXGateNode* CloneCall(const ASTIdentifierNode* Id,
                                    const ASTArgumentNodeList& AL,
                                    const ASTAnyTypeList& QL) override;

  virtual ASTCCXGateNode* CloneCall(const ASTIdentifierNode* Id,
                                    const ASTParameterList& PL,
                                    const ASTIdentifierList& IL);

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
  ASTCXGateNode(const ASTIdentifierNode* Id)
  : ASTGateNode(Id) { }

  ASTCXGateNode(const ASTIdentifierNode* Id,
                const ASTArgumentNodeList& AL,
                const ASTAnyTypeList& TL,
                bool IsGateCall = false,
                const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault)
  : ASTGateNode(Id, AL, TL, IsGateCall, OL) { }

  ASTCXGateNode(const ASTIdentifierNode* Id,
                const ASTParameterList& PL,
                const ASTIdentifierList& IL,
                bool IsGateCall = false,
                const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault)
  : ASTGateNode(Id, PL, IL, IsGateCall, OL) { }

  virtual ~ASTCXGateNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeCXGate;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTCXGateNode* CloneCall(const ASTIdentifierNode* Id,
                                   const ASTArgumentNodeList& AL,
                                   const ASTAnyTypeList& QL) override;

  virtual ASTCXGateNode* CloneCall(const ASTIdentifierNode* Id,
                                   const ASTParameterList& PL,
                                   const ASTIdentifierList& IL);

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
  ASTUGateNode(const ASTIdentifierNode* Id,
               const ASTArgumentNodeList& AL,
               const ASTAnyTypeList& TL,
               bool IsGateCall = false,
               const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault,
               bool IsIdent = false)
  : ASTGateNode(Id, AL, TL, IsGateCall, OL), Identity(IsIdent) { }

  ASTUGateNode(const ASTIdentifierNode* Id,
               const ASTParameterList& PL,
               const ASTIdentifierList& IL,
               bool IsGateCall = false,
               const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault)
  : ASTGateNode(Id, PL, IL, IsGateCall, OL) { }

  virtual ~ASTUGateNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeUGate;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTUGateNode* CloneCall(const ASTIdentifierNode* Id,
                                  const ASTArgumentNodeList& AL,
                                  const ASTAnyTypeList& QL) override;

  virtual bool IsIdentity() const {
    return Identity;
  }

  virtual void SetIdentity(bool BV) {
    Identity = BV;
  }

  virtual void print() const override {
    std::cout << "<UGate>" << std::endl;
    ASTGateNode::print();
    std::cout << "</UGate>" << std::endl;
  }
};

class ASTGateDeclarationNode : public ASTStatementNode {
private:
  ASTGateDeclarationNode() = delete;

protected:
  ASTGateDeclarationNode(const ASTIdentifierNode* Id,
                         const std::string& ERM)
  : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM))
  { }

public:
  ASTGateDeclarationNode(const ASTIdentifierNode* Id, const ASTGateNode* G)
  : ASTStatementNode(Id, G) {
    assert(Id && "Invalid ASTIdentifierNode!");
    assert(G && "Invalid ASTGateNode!");
  }

  virtual ~ASTGateDeclarationNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeGateDeclaration;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeDeclaration;
  }

  virtual bool DuplicatesAllowed() const {
    return true;
  }

  virtual const ASTGateNode* GetGateNode() const {
    return dynamic_cast<const ASTGateNode*>(ASTStatementNode::Expr);
  }

  virtual void print() const override {
    std::cout << "<GateDeclarationNode>" << std::endl;
    const ASTGateNode* GN =
      dynamic_cast<const ASTGateNode*>(ASTStatementNode::Expr);
    GN->print();
    std::cout << "</GateDeclarationNode>" << std::endl;
  }

  static ASTGateDeclarationNode* DeclarationError(const ASTIdentifierNode* Id,
                                                  const std::string& EM) {
    return new ASTGateDeclarationNode(Id, EM);
  }

  virtual bool IsError() const override {
    return ASTStatementNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTStatementNode::GetError();
  }

  virtual void push(ASTBase* /* unused*/) override { }
};

} // namespace QASM

#endif // __QASM_AST_GATE_NODES_H

