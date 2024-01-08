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

#ifndef __QASM_AST_IF_CONDITIONALS_H
#define __QASM_AST_IF_CONDITIONALS_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTStatement.h>
#include <qasm/AST/ASTTypes.h>

#include <map>
#include <string>

namespace QASM {

class ASTIfStatementNode;
class ASTSymbolTableEntry;

class ASTElseIfStatementNode : public ASTStatementNode {
  friend class ASTProductionFactory;
  friend class ASTIfStatementTracker;
  friend class ASTElseIfStatementTracker;
  friend class ASTElseStatementTracker;
  friend class ASTIfConditionalsGraphController;

private:
  static const ASTElseIfStatementNode *ElseIfTerminator;

private:
  const ASTIfStatementNode *IfStmt;
  const ASTStatementNode *OpNode;
  const ASTStatementList *OpList;
  const ASTElseIfStatementNode *NEIf;
  std::map<std::string, const ASTSymbolTableEntry *> STM;
  unsigned StackFrame;
  unsigned ISC;
  bool PendingElseIf;
  bool PendingElse;

private:
  ASTElseIfStatementNode() = delete;

protected:
  ASTElseIfStatementNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)),
        IfStmt(nullptr), OpNode(nullptr), OpList(nullptr), NEIf(nullptr), STM(),
        StackFrame(static_cast<unsigned>(~0x0)),
        ISC(static_cast<unsigned>(~0x0)), PendingElseIf(false),
        PendingElse(false) {}

protected:
  ASTStatementNode *GetOpNode() {
    return const_cast<ASTStatementNode *>(OpNode);
  }

  ASTStatementList *GetOpList() {
    return const_cast<ASTStatementList *>(OpList);
  }

public:
  ASTElseIfStatementNode(const ASTIfStatementNode *IF);

  ASTElseIfStatementNode(const ASTIfStatementNode *IF,
                         const ASTExpressionNode *EX,
                         const ASTStatementList *SL);

  virtual ~ASTElseIfStatementNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeElseIfStatement; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  void SetStackFrame(unsigned N) { StackFrame = N; }

  void SetISC(unsigned X) { ISC = X; }

  void SetPendingElseIf(bool V) { PendingElseIf = V; }

  void SetPendingElse(bool V) { PendingElse = V; }

  unsigned GetStackFrame() const { return StackFrame; }

  unsigned GetISC() const { return ISC; }

  bool HasPendingElseIf() const { return PendingElseIf; }

  bool HasPendingElse() const { return PendingElse; }

  void AttachTo(const ASTIfStatementNode *IfNode);

  const ASTIfStatementNode *GetIfStatement() const { return IfStmt; }

  const ASTIfStatementNode *GetParentIf() const { return IfStmt; }

  void SetParentIf(ASTIfStatementNode *IFN) { IfStmt = IFN; }

  void SetNextElseIf(const ASTElseIfStatementNode *EIF) { NEIf = EIF; }

  const ASTElseIfStatementNode *GetNextElseIf() const { return NEIf; }

  bool HasNextElseIf() const { return NEIf; }

  virtual const ASTExpressionNode *GetExpression() const override {
    return ASTStatementNode::Expr;
  }

  const ASTStatementNode *GetOpNode() const { return OpNode; }

  const ASTStatementList *GetOpList() const { return OpList; }

  bool IsOneStatement() const {
    return OpNode || (OpList && OpList->Size() == 1U);
  }

  bool IsMultipleStatements() const { return OpList && OpList->Size() > 1U; }

  bool Empty() const {
    if (!OpNode && !OpList)
      return true;

    return !OpNode && (OpList && !OpList->Size());
  }

  void Update(const ASTExpressionNode *E, const ASTStatementNode *SN,
              const ASTStatementList *SL) {
    Expr = E;
    OpNode = SN;
    OpList = SL;
  }

  void Update(const ASTStatementNode *SN, const ASTStatementList *SL) {
    OpNode = SN;
    OpList = SL;
  }

  void Update(const ASTStatementList *SL) { OpList = SL; }

  std::map<std::string, const ASTSymbolTableEntry *> &GetSymbolTable() {
    return STM;
  }

  const std::map<std::string, const ASTSymbolTableEntry *> &
  GetSymbolTable() const {
    return STM;
  }

  const ASTSymbolTableEntry *GetSymbol(const std::string &SN) const {
    std::map<std::string, const ASTSymbolTableEntry *>::const_iterator I =
        STM.find(SN);
    return I == STM.end() ? nullptr : (*I).second;
  }

  bool IsError() const override { return ASTStatementNode::IsError(); }

  const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTElseIfStatementNode *StatementError(const std::string &ERM) {
    return new ASTElseIfStatementNode(
        ASTIdentifierNode::ElseIfExpression.Clone(), ERM);
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTElseStatementNode : public ASTStatementNode {
  friend class ASTProductionFactory;
  friend class ASTIfStatementTracker;
  friend class ASTElseIfStatementTracker;
  friend class ASTElseStatementTracker;
  friend class ASTIfConditionalsGraphController;

private:
  static const ASTElseStatementNode *ElseTerminator;

private:
  const ASTIfStatementNode *IfStmt;
  const ASTStatementNode *OpNode;
  const ASTStatementList *OpList;
  std::map<std::string, const ASTSymbolTableEntry *> STM;
  unsigned StackFrame;
  unsigned ISC;

protected:
  ASTElseStatementNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)),
        IfStmt(nullptr), OpNode(nullptr), OpList(nullptr), STM(),
        StackFrame(static_cast<unsigned>(~0x0)),
        ISC(static_cast<unsigned>(~0x0)) {}

protected:
  ASTStatementNode *GetOpNode() {
    return const_cast<ASTStatementNode *>(OpNode);
  }

  ASTStatementList *GetOpList() {
    return const_cast<ASTStatementList *>(OpList);
  }

public:
  ASTElseStatementNode(const ASTIfStatementNode *IF);

  ASTElseStatementNode(const ASTIfStatementNode *IF,
                       const ASTStatementList *SL);

  virtual ~ASTElseStatementNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeElseStatement; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  void SetStackFrame(unsigned N) { StackFrame = N; }

  void SetISC(unsigned X) { ISC = X; }

  unsigned GetStackFrame() const { return StackFrame; }

  unsigned GetISC() const { return ISC; }

  void AttachTo(const ASTIfStatementNode *IfNode);

  const ASTIfStatementNode *GetIfStatement() const { return IfStmt; }

  const ASTIfStatementNode *GetParentIf() const { return IfStmt; }

  void SetParentIf(ASTIfStatementNode *IFN) { IfStmt = IFN; }

  virtual const ASTExpressionNode *GetExpression() const override {
    return ASTStatementNode::GetExpression();
  }

  const ASTStatementNode *GetOpNode() const { return OpNode; }

  const ASTStatementList *GetOpList() const { return OpList; }

  bool IsOneStatement() const {
    return OpNode || (OpList && OpList->Size() == 1U);
  }

  bool IsMultipleStatements() const { return OpList && OpList->Size() > 1U; }

  bool Empty() const {
    if (!OpNode && !OpList)
      return true;

    return !OpNode && (OpList && !OpList->Size());
  }

  void Update(const ASTExpressionNode *E, const ASTStatementNode *ON,
              const ASTStatementList *SL) {
    Expr = E;
    OpNode = ON;
    OpList = SL;
  }

  void Update(const ASTStatementNode *ON, const ASTStatementList *SL) {
    OpNode = ON;
    OpList = SL;
  }

  void Update(const ASTStatementList *SL) { OpList = SL; }

  std::map<std::string, const ASTSymbolTableEntry *> &GetSymbolTable() {
    return STM;
  }

  const std::map<std::string, const ASTSymbolTableEntry *> &
  GetSymbolTable() const {
    return STM;
  }

  const ASTSymbolTableEntry *GetSymbol(const std::string &SN) const {
    std::map<std::string, const ASTSymbolTableEntry *>::const_iterator I =
        STM.find(SN);
    return I == STM.end() ? nullptr : (*I).second;
  }

  bool IsError() const override { return ASTStatementNode::IsError(); }

  const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTElseStatementNode *StatementError(const std::string &ERM) {
    return new ASTElseStatementNode(ASTIdentifierNode::ElseExpression.Clone(),
                                    ERM);
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTIfStatementNode : public ASTStatementNode {
  friend class ASTProductionFactory;
  friend class ASTIfStatementTracker;
  friend class ASTElseIfStatementTracker;
  friend class ASTElseStatementTracker;
  friend class ASTIfConditionalsGraphController;

private:
  mutable std::vector<ASTElseIfStatementNode *> ElseIf;
  mutable ASTElseStatementNode *Else;
  ASTIfStatementNode *PIF;
  const ASTStatementNode *OpNode;
  const ASTStatementList *OpList;
  std::map<std::string, const ASTSymbolTableEntry *> STM;
  unsigned StackFrame;
  unsigned ISC;
  bool PendingElseIf;
  bool PendingElse;
  bool Braces;

private:
  ASTIfStatementNode() = delete;

protected:
  ASTIfStatementNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)),
        ElseIf(), Else(nullptr), PIF(nullptr), OpNode(nullptr), OpList(nullptr),
        STM(), StackFrame(static_cast<unsigned>(~0x0)),
        ISC(static_cast<unsigned>(~0x0)), PendingElseIf(false),
        PendingElse(false), Braces(false) {}

  std::vector<ASTElseIfStatementNode *> *GetElseIfPointer() { return &ElseIf; }

  const std::vector<ASTElseIfStatementNode *> *GetElseIfPointer() const {
    return &ElseIf;
  }

  ASTStatementNode *GetOpNode() {
    return const_cast<ASTStatementNode *>(OpNode);
  }

  ASTStatementList *GetOpList() {
    return const_cast<ASTStatementList *>(OpList);
  }

public:
  ASTIfStatementNode(const ASTExpressionNode *E, const ASTStatementNode *SN);

  ASTIfStatementNode(const ASTExpressionNode *E, const ASTStatementList *SL);

  ASTIfStatementNode(const ASTExpressionNode *E, const ASTStatementNode *SN,
                     const ASTStatementList *SL);

  virtual ~ASTIfStatementNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeIfStatement; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  void SetStackFrame(unsigned N) { StackFrame = N; }

  void SetISC(unsigned X) { ISC = X; }

  void SetParentIf(ASTIfStatementNode *ISN) { PIF = ISN; }

  void SetPendingElseIf(bool V) { PendingElseIf = V; }

  void SetPendingElse(bool V) { PendingElse = V; }

  void SetBraces(bool V) { Braces = V; }

  unsigned GetStackFrame() const { return StackFrame; }

  unsigned GetISC() const { return ISC; }

  bool HasPendingElseIf() const { return PendingElseIf; }

  bool HasPendingElse() const { return PendingElse; }

  bool HasBraces() const { return Braces; }

  virtual const ASTExpressionNode *GetExpression() const override {
    return ASTStatementNode::GetExpression();
  }

  const ASTStatementNode *GetOpNode() const { return OpNode; }

  const ASTStatementList *GetOpList() const { return OpList; }

  bool IsOneStatement() const {
    return OpNode || (OpList && OpList->Size() == 1U);
  }

  bool IsMultipleStatements() const { return OpList && OpList->Size() > 1U; }

  ASTIfStatementNode *GetParentIf() { return PIF; }

  const ASTIfStatementNode *GetParentIf() const { return PIF; }

  bool HasParentIf() const { return PIF != nullptr; }

  std::vector<ASTElseIfStatementNode *> &GetElseIf() { return ElseIf; }

  const std::vector<ASTElseIfStatementNode *> &GetElseIf() const {
    return ElseIf;
  }

  bool HasElseIf() const { return !ElseIf.empty(); }

  void NormalizeElseIf();

  ASTElseStatementNode *GetElse() { return Else; }

  const ASTElseStatementNode *GetElse() const { return Else; }

  bool HasElse() const { return Else != nullptr; }

  bool HasStatementList() const {
    return OpList && (ISC != static_cast<unsigned>(~0x0));
  }

  std::map<std::string, const ASTSymbolTableEntry *> &GetSymbolTable() {
    return STM;
  }

  const std::map<std::string, const ASTSymbolTableEntry *> &
  GetSymbolTable() const {
    return STM;
  }

  const ASTSymbolTableEntry *GetSymbol(const std::string &SN) const {
    std::map<std::string, const ASTSymbolTableEntry *>::const_iterator I =
        STM.find(SN);
    return I == STM.end() ? nullptr : (*I).second;
  }

  bool IsError() const override { return ASTStatementNode::IsError(); }

  const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTIfStatementNode *StatementError(const std::string &ERM) {
    return new ASTIfStatementNode(ASTIdentifierNode::IfExpression.Clone(), ERM);
  }

  virtual void print() const override;

  virtual void print_header() const;

  virtual void print_expression() const;

  virtual void push(ASTBase * /* unused */) override {}

  void AddElseIf(ASTElseIfStatementNode *Node) {
    for (std::vector<ASTElseIfStatementNode *>::iterator EI = ElseIf.begin();
         EI != ElseIf.end(); ++EI) {
      if ((*EI) == Node)
        return;
    }

    ElseIf.push_back(Node);
  }

  void AddElseIf(ASTElseIfStatementNode *Node) const {
    for (std::vector<ASTElseIfStatementNode *>::iterator EI = ElseIf.begin();
         EI != ElseIf.end(); ++EI) {
      if ((*EI) == Node)
        return;
    }

    ElseIf.push_back(Node);
  }

  void AddElse(ASTElseStatementNode *Node) {
    if (Else == Node)
      return;

    Else = Node;
  }

  void AddElse(ASTElseStatementNode *Node) const {
    if (Else == Node)
      return;

    Else = Node;
  }
};

} // namespace QASM

#endif // __QASM_AST_IF_CONDITIONALS_H
