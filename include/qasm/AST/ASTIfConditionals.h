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

#ifndef __QASM_AST_IF_CONDITIONALS_H
#define __QASM_AST_IF_CONDITIONALS_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTStatement.h>

#include <map>
#include <string>

namespace QASM {

class ASTIfStatementNode;
class ASTSymbolTableEntry;

class ASTElseIfStatementNode : public ASTStatementNode {
private:
  static const ASTElseIfStatementNode* ElseIfTerminator;

private:
  const ASTIfStatementNode* IfStmt;
  const ASTStatementNode* OpNode;
  const ASTStatementList* OpList;
  std::map<std::string, const ASTSymbolTableEntry*> STM;
  unsigned StackFrame;
  unsigned ISC;

private:
  ASTElseIfStatementNode() = delete;

public:
  ASTElseIfStatementNode(const ASTIfStatementNode* IF);

  ASTElseIfStatementNode(const ASTIfStatementNode* IF,
                         const ASTExpressionNode* EX,
                         const ASTStatementList* SL);

  virtual ~ASTElseIfStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeElseIfStatement;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual void SetStackFrame(unsigned N) {
    StackFrame = N;
  }

  virtual void SetISC(unsigned X) {
    ISC = X;
  }

  virtual unsigned GetStackFrame() const {
    return StackFrame;
  }

  virtual unsigned GetISC() const {
    return ISC;
  }

  virtual void AttachTo(const ASTIfStatementNode* IfNode);

  virtual const ASTIfStatementNode* GetIfStatement() const {
    return IfStmt;
  }

  virtual const ASTExpressionNode* GetExpression() const override {
    return ASTStatementNode::Expr;
  }

  virtual const ASTStatementNode* GetOpNode() const {
    return OpNode;
  }

  virtual const ASTStatementList* GetOpList() const {
    return OpList;
  }

  virtual bool IsOneStatement() const {
    return OpNode;
  }

  virtual bool IsMultipleStatements() const {
    return OpList;
  }

  virtual bool Empty() const {
    if (!OpNode && !OpList)
      return true;

    return !OpNode && (OpList && !OpList->Size());
  }

  virtual void Update(const ASTExpressionNode* E,
                      const ASTStatementNode* SN,
                      const ASTStatementList* SL) {
    Expr = E;
    OpNode = SN;
    OpList = SL;
  }

  virtual void Update(const ASTStatementNode* SN,
                      const ASTStatementList* SL) {
    OpNode = SN;
    OpList = SL;
  }

  virtual void Update(const ASTStatementList* SL) {
    OpList = SL;
  }

  virtual std::map<std::string, const ASTSymbolTableEntry*>& GetSymbolTable() {
    return STM;
  }

  virtual const std::map<std::string,
                        const ASTSymbolTableEntry*>& GetSymbolTable() const {
    return STM;
  }

  virtual const ASTSymbolTableEntry* GetSymbol(const std::string& SN) const {
    std::map<std::string, const ASTSymbolTableEntry*>::const_iterator I =
      STM.find(SN);
    return I == STM.end() ? nullptr : (*I).second;
  }

  virtual void print() const override;

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTElseStatementNode : public ASTStatementNode {
private:
  static const ASTElseStatementNode* ElseTerminator;

private:
  const ASTIfStatementNode* IfStmt;
  const ASTStatementNode* OpNode;
  const ASTStatementList* OpList;
  std::map<std::string, const ASTSymbolTableEntry*> STM;
  unsigned StackFrame;
  unsigned ISC;

public:
  ASTElseStatementNode(const ASTIfStatementNode* IF);

  ASTElseStatementNode(const ASTIfStatementNode* IF,
                       const ASTStatementList* SL);

  virtual ~ASTElseStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeElseStatement;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual void SetStackFrame(unsigned N) {
    StackFrame = N;
  }

  virtual void SetISC(unsigned X) {
    ISC = X;
  }

  virtual unsigned GetStackFrame() const {
    return StackFrame;
  }

  virtual unsigned GetISC() const {
    return ISC;
  }

  virtual void AttachTo(const ASTIfStatementNode* IfNode);

  virtual const ASTIfStatementNode* GetIfStatement() const {
    return IfStmt;
  }

  virtual const ASTExpressionNode* GetExpression() const override {
    return ASTStatementNode::GetExpression();
  }

  virtual const ASTStatementNode* GetOpNode() const {
    return OpNode;
  }

  virtual const ASTStatementList* GetOpList() const {
    return OpList;
  }

  virtual bool IsOneStatement() const {
    return OpNode;
  }

  virtual bool IsMultipleStatements() const {
    return OpList;
  }

  virtual bool Empty() const {
    if (!OpNode && !OpList)
      return true;

    return !OpNode && (OpList && !OpList->Size());
  }

  virtual void Update(const ASTExpressionNode* E,
                      const ASTStatementNode* ON,
                      const ASTStatementList* SL) {
    Expr = E;
    OpNode = ON;
    OpList = SL;
  }

  virtual void Update(const ASTStatementNode* ON,
                      const ASTStatementList* SL) {
    OpNode = ON;
    OpList = SL;
  }

  virtual void Update(const ASTStatementList* SL) {
    OpList = SL;
  }

  virtual std::map<std::string, const ASTSymbolTableEntry*>& GetSymbolTable() {
    return STM;
  }

  virtual const std::map<std::string,
                        const ASTSymbolTableEntry*>& GetSymbolTable() const {
    return STM;
  }

  virtual const ASTSymbolTableEntry* GetSymbol(const std::string& SN) const {
    std::map<std::string, const ASTSymbolTableEntry*>::const_iterator I =
      STM.find(SN);
    return I == STM.end() ? nullptr : (*I).second;
  }

  virtual void print() const override;

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTIfStatementNode : public ASTStatementNode {
private:
  std::vector<ASTElseIfStatementNode*> ElseIf;
  ASTElseStatementNode* Else;
  ASTIfStatementNode* PIF;
  const ASTStatementNode* OpNode;
  const ASTStatementList* OpList;
  std::map<std::string, const ASTSymbolTableEntry*> STM;
  unsigned StackFrame;
  unsigned ISC;

private:
  ASTIfStatementNode() = delete;

public:
  ASTIfStatementNode(const ASTExpressionNode* E,
                     const ASTStatementNode* SN);

  ASTIfStatementNode(const ASTExpressionNode* E,
                     const ASTStatementList* SL);

  ASTIfStatementNode(const ASTExpressionNode* E,
                     const ASTStatementNode* SN,
                     const ASTStatementList* SL);

  virtual ~ASTIfStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeIfStatement;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual void SetStackFrame(unsigned N) {
    StackFrame = N;
  }

  virtual void SetISC(unsigned X) {
    ISC = X;
  }

  virtual void SetParentIf(ASTIfStatementNode* ISN) {
    PIF = ISN;
  }

  virtual unsigned GetStackFrame() const {
    return StackFrame;
  }

  virtual unsigned GetISC() const {
    return ISC;
  }

  virtual const ASTExpressionNode* GetExpression() const override {
    return ASTStatementNode::GetExpression();
  }

  virtual const ASTStatementNode* GetOpNode() const {
    return OpNode;
  }

  virtual const ASTStatementList* GetOpList() const {
    return OpList;
  }

  virtual bool IsOneStatement() const {
    return OpNode;
  }

  virtual bool IsMultipleStatements() const {
    return OpList;
  }

  virtual ASTIfStatementNode* GetParentIf() {
    return PIF;
  }

  virtual const ASTIfStatementNode* GetParentIf() const {
    return PIF;
  }

  virtual bool HasParentIf() const {
    return PIF != nullptr;
  }

  virtual std::vector<ASTElseIfStatementNode*>& GetElseIf() {
    return ElseIf;
  }

  virtual const std::vector<ASTElseIfStatementNode*>& GetElseIf() const {
    return ElseIf;
  }

  virtual bool HasElseIf() const {
    return !ElseIf.empty();
  }

  virtual ASTElseStatementNode* GetElse() {
    return Else;
  }

  virtual const ASTElseStatementNode* GetElse() const {
    return Else;
  }

  virtual bool HasElse() const {
    return Else != nullptr;
  }

  virtual bool HasStatementList() const {
    return OpList && (ISC != static_cast<unsigned>(~0x0));
  }

  virtual std::map<std::string, const ASTSymbolTableEntry*>& GetSymbolTable() {
    return STM;
  }

  virtual const std::map<std::string,
                        const ASTSymbolTableEntry*>& GetSymbolTable() const {
    return STM;
  }

  virtual const ASTSymbolTableEntry* GetSymbol(const std::string& SN) const {
    std::map<std::string, const ASTSymbolTableEntry*>::const_iterator I =
      STM.find(SN);
    return I == STM.end() ? nullptr : (*I).second;
  }

  virtual void print() const override;

  virtual void print_header() const;

  virtual void push(ASTBase* /* unused */) override { }

  virtual void AddElseIf(ASTElseIfStatementNode* Node) {
    Node->AttachTo(this);
    Node->SetStackFrame(StackFrame);
    Node->SetISC(ISC);
    ElseIf.push_back(Node);
  }

  virtual void AddElse(ASTElseStatementNode* Node) {
    Node->AttachTo(this);
    Node->SetStackFrame(StackFrame);
    Node->SetISC(ISC);
    Else = Node;
  }
};

} // namespace QASM

#endif // __QASM_AST_IF_CONDITIONALS_H

