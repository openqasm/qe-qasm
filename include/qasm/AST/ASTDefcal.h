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

#ifndef __QASM_AST_DEFCAL_H
#define __QASM_AST_DEFCAL_H

#include <qasm/AST/ASTDefcalParameterBuilder.h>
#include <qasm/AST/ASTDelay.h>
#include <qasm/AST/ASTDuration.h>
#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTMeasure.h>
#include <qasm/AST/ASTQubitList.h>
#include <qasm/AST/ASTReset.h>
#include <qasm/AST/ASTResult.h>
#include <qasm/AST/ASTStatement.h>
#include <qasm/AST/ASTTypes.h>

#include <map>
#include <string>

namespace QASM {

class ASTSymbolTableEntry;

class ASTDefcalNode : public ASTStatementNode {
  friend class ASTDefcalNodeBuilder;
  friend class ASTBuilder;

protected:
  ASTExpressionNodeList Params;
  ASTStatementList Statements;

  union {
    const void *Void;
    const ASTMeasureNode *Measure;
    const ASTResetNode *Reset;
    const ASTDelayNode *Delay;
    const ASTDurationOfNode *Duration;
  };

  ASTType OTy;

  std::map<std::string, const ASTSymbolTableEntry *> DSTM;
  mutable const ASTBoundQubitList *QTarget;
  ASTIdentifierList QIL;
  QubitKind QK;
  std::string Grammar;
  std::string Name;
  bool IsCall;

protected:
  static unsigned QIC;

protected:
  ASTDefcalNode(const ASTIdentifierNode *Id);
  void CloneArgumentsFrom(const ASTArgumentNodeList &AL);
  void CloneQubitsFrom(const ASTAnyTypeList &QL);
  void ToDefcalParamSymbolTable(const ASTIdentifierNode *Id,
                                const ASTSymbolTableEntry *STE);
  void ToDefcalParamSymbolTable(const std::string &Id,
                                const ASTSymbolTableEntry *STE);
  ASTAngleNode *CreateAngleConversion(const ASTIdentifierNode *XId,
                                      const ASTSymbolTableEntry *XSTE) const;
  ASTAngleNode *CreateAngleConversion(const ASTSymbolTableEntry *XSTE) const;
  ASTAngleNode *CreateAngleTemporary(const ASTSymbolTableEntry *XSTE) const;
  ASTAngleNode *CreateAngleSymbolTableEntry(ASTSymbolTableEntry *XSTE) const;

private:
  ASTDefcalNode() = delete;

  void print_qubits() const;

public:
  static const unsigned DefcalBits = 64U;

public:
  ASTDefcalNode(const ASTIdentifierNode *Id, const ASTExpressionNodeList &EL,
                const ASTStatementList &SL, const ASTBoundQubitList *QT);

  ASTDefcalNode(const ASTIdentifierNode *Id, const std::string &Grammar,
                const ASTExpressionNodeList &EL, const ASTStatementList &SL,
                const ASTBoundQubitList *QT);

  ASTDefcalNode(const ASTIdentifierNode *Id, const ASTStatementList &SL,
                const ASTMeasureNode *M);

  ASTDefcalNode(const ASTIdentifierNode *Id, const std::string &Grammar,
                const ASTStatementList &SL, const ASTMeasureNode *M);

  ASTDefcalNode(const ASTIdentifierNode *Id, const ASTStatementList &SL,
                const ASTResetNode *R);

  ASTDefcalNode(const ASTIdentifierNode *Id, const std::string &Grammar,
                const ASTStatementList &SL, const ASTResetNode *R);

  ASTDefcalNode(const ASTIdentifierNode *Id, const ASTDelayNode *D,
                const ASTStatementList &SL,
                const ASTBoundQubitList *QL = nullptr);

  ASTDefcalNode(const ASTIdentifierNode *Id, const std::string &Grammar,
                const ASTDelayNode *D, const ASTStatementList &SL,
                const ASTBoundQubitList *QL = nullptr);

  ASTDefcalNode(const ASTIdentifierNode *Id, const ASTDurationOfNode *D,
                const ASTStatementList &SL,
                const ASTBoundQubitList *QL = nullptr);

  ASTDefcalNode(const ASTIdentifierNode *Id, const std::string &Grammar,
                const ASTDurationOfNode *D, const ASTStatementList &SL,
                const ASTBoundQubitList *QL = nullptr);

  virtual ~ASTDefcalNode() = default;

  void ResolveQubits();

  void ResolveParams();

  void ResolveMeasure();

  void ResolveReset();

  void MangleParams();

  virtual ASTType GetASTType() const override { return ASTTypeDefcal; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual void Mangle() override;

  virtual const ASTExpressionNode *AsExpression() const;

  virtual bool IsMeasure() const {
    return OTy == ASTTypeMeasure ? Measure != nullptr : false;
  }

  virtual bool IsReset() const {
    return OTy == ASTTypeReset ? Reset != nullptr : false;
  }

  virtual bool IsDelay() const {
    return OTy == ASTTypeDelay ? Delay != nullptr : false;
  }

  virtual bool IsDuration() const {
    return OTy == ASTTypeDurationOf ? Duration != nullptr : false;
  }

  virtual bool IsDefcalCall() const { return IsCall; }

  virtual bool HasResult() const {
    return OTy == ASTTypeMeasure && Measure && Measure->HasResult();
  }

  virtual ASTType GetResultType() const {
    if (HasResult())
      return Measure->GetResultType();

    return ASTTypeUndefined;
  }

  virtual bool HasQubitTargets() const { return !QIL.Empty(); }

  virtual bool HasParameters() const { return !Params.Empty(); }

  virtual bool HasStatements() const { return !Statements.Empty(); }

  virtual const ASTMeasureNode *GetMeasure() const {
    return OTy == ASTTypeMeasure ? Measure : nullptr;
  }

  virtual const ASTResetNode *GetReset() const {
    return OTy == ASTTypeReset ? Reset : nullptr;
  }

  virtual const ASTDelayNode *GetDelay() const {
    return OTy == ASTTypeDelay ? Delay : nullptr;
  }

  virtual const ASTDurationOfNode *GetDuration() const {
    return OTy == ASTTypeDurationOf ? Duration : nullptr;
  }

  virtual const ASTIdentifierList &GetQubitTargets() const { return QIL; }

  virtual const ASTExpressionNodeList &GetParameters() const { return Params; }

  virtual const ASTStatementList &GetStatements() const { return Statements; }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTStatementNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTStatementNode::Ident->GetName();
  }

  virtual QubitKind GetQubitKind() const { return QK; }

  virtual std::map<std::string, const ASTSymbolTableEntry *> &GetSymbolTable() {
    return DSTM;
  }

  virtual const std::map<std::string, const ASTSymbolTableEntry *> &
  GetSymbolTable() const {
    return DSTM;
  }

  virtual const ASTSymbolTableEntry *GetSymbol(const std::string &SN) const {
    std::map<std::string, const ASTSymbolTableEntry *>::const_iterator I =
        DSTM.find(SN);
    return I == DSTM.end() ? nullptr : (*I).second;
  }

  virtual void TransferLocalSymbolTable();

  virtual void SetDefcalCall(bool V = true) { IsCall = V; }

  virtual ASTDefcalNode *CloneCall(const ASTIdentifierNode *Id,
                                   const ASTArgumentNodeList &AL,
                                   const ASTAnyTypeList &QL) const;

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTDefcalDeclarationNode : public ASTDeclarationNode {
private:
  const ASTDefcalNode *DN;

protected:
  ASTDefcalDeclarationNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTDeclarationNode(Id, new ASTStringNode(ERM),
                           ASTTypeDeclarationError) {
    ASTDeclarationNode::SetLocation(Id->GetLocation());
  }

public:
  ASTDefcalDeclarationNode(const ASTIdentifierNode *Id, const ASTDefcalNode *D)
      : ASTDeclarationNode(Id, ASTTypeDefcal), DN(D) {}

  virtual ~ASTDefcalDeclarationNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeDefcalDeclaration;
  }

  virtual const ASTDefcalNode *GetDefcal() const { return DN; }

  virtual bool IsError() const override {
    return ASTDeclarationNode::IsError();
  }

  virtual const std::string &GetError() const override {
    return ASTDeclarationNode::GetError();
  }

  static ASTDefcalDeclarationNode *DeclarationError(const ASTIdentifierNode *Id,
                                                    const std::string &ERM) {
    return new ASTDefcalDeclarationNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<DefcalDeclaration>" << std::endl;
    DN->print();
    std::cout << "</DefcalDeclaration>" << std::endl;
  }
};

class ASTDefcalGroupNode : public ASTExpressionNode {
private:
  std::vector<ASTGateQOpNode *> DV;

private:
  ASTDefcalGroupNode() = delete;

public:
  using vector_type = std::vector<ASTGateQOpNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTDefcalGroupNode(const ASTIdentifierNode *Id)
      : ASTExpressionNode(Id, ASTTypeDefcalGroup), DV() {}

  ASTDefcalGroupNode(const ASTIdentifierNode *Id,
                     const std::vector<const ASTSymbolTableEntry *> &DGV);

  virtual ~ASTDefcalGroupNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeDefcalGroup; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual unsigned Size() const { return static_cast<unsigned>(DV.size()); }

  virtual bool Empty() const { return DV.empty(); }

  virtual void AddDefcal(ASTDefcalNode *DN) {
    assert(DN && "Invalid ASTDefcalNode argument!");
    DV.push_back(new ASTGateQOpNode(DN->GetIdentifier(), DN));
  }

  virtual ASTDefcalNode *GetDefcalNode(unsigned IX) {
    assert(IX < DV.size() && "Index is out-of-range!");

    try {
      ASTGateQOpNode *QON = DV.at(IX);
      assert(QON && "Invalid ASTGateQOpNode obtained from defcal vector!");
      return const_cast<ASTDefcalNode *>(QON->GetDefcalNode());
    } catch (const std::out_of_range &E) {
      return nullptr;
    } catch (...) {
      return nullptr;
    }

    return nullptr;
  }

  virtual const ASTDefcalNode *GetDefcalNode(unsigned IX) const {
    assert(IX < DV.size() && "Index is out-of-range!");

    try {
      ASTGateQOpNode *QON = DV.at(IX);
      assert(QON && "Invalid ASTGateQOpNode obtained from defcal vector!");
      return QON->GetDefcalNode();
    } catch (const std::out_of_range &E) {
      return nullptr;
    } catch (...) {
      return nullptr;
    }

    return nullptr;
  }

  ASTDefcalNode *operator[](unsigned IX) { return GetDefcalNode(IX); }

  const ASTDefcalNode *operator[](unsigned IX) const {
    return GetDefcalNode(IX);
  }

  iterator begin() { return DV.begin(); }

  const_iterator begin() const { return DV.begin(); }

  iterator end() { return DV.end(); }

  const_iterator end() const { return DV.end(); }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTDefcalGroupOpNode : public ASTGateQOpNode {
private:
  ASTDefcalGroupOpNode() = default;

public:
  ASTDefcalGroupOpNode(const ASTIdentifierNode *Id,
                       const ASTDefcalGroupNode *DGN)
      : ASTGateQOpNode(Id, DGN) {}

  virtual ~ASTDefcalGroupOpNode() = default;

  virtual void Mangle() override;

  virtual ASTType GetASTType() const override {
    return ASTTypeDefcalGroupOpNode;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_DEFCAL_H
