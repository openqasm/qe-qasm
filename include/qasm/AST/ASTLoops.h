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

#ifndef __QASM_AST_LOOPS_H
#define __QASM_AST_LOOPS_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTStatement.h>
#include <qasm/AST/ASTIntegerList.h>
#include <qasm/AST/ASTSymbolTable.h>

#include <map>
#include <string>

namespace QASM {

class ASTForLoopRangeExpressionNode : public ASTExpressionNode {
  friend class ASTForLoopNode;

private:
  ASTIntegerList IVL;
  ASTBinaryOpNode* BOP;

private:
  ASTForLoopRangeExpressionNode() = delete;

protected:
  ASTForLoopRangeExpressionNode(const std::string& ERM)
  : ASTExpressionNode(ASTIdentifierNode::ForLoopRange.Clone(),
                      new ASTStringNode(ERM), ASTTypeExpressionError),
  IVL(), BOP(nullptr) { }

public:
  ASTForLoopRangeExpressionNode(const ASTIntegerList& IL,
                                ASTBinaryOpNode* BOp)
  : ASTExpressionNode(ASTIdentifierNode::ForLoopRange.Clone(),
                      ASTTypeForLoopRange),
  IVL(IL), BOP(BOp) { }

  virtual ~ASTForLoopRangeExpressionNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeForLoopRange;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  // Implemented in ASTLoops.cpp
  virtual void Mangle() override;

  ASTIntegerList& GetIntegerList() {
    return IVL;
  }

  const ASTIntegerList& GetIntegerList() const {
    return IVL;
  }

  ASTBinaryOpNode* GetBinaryOp() {
    return BOP;
  }

  const ASTBinaryOpNode* GetBinaryOp() const {
    return BOP;
  }

  virtual bool IsError() const override {
    return ASTExpressionNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTForLoopRangeExpressionNode* ExpressionError(const std::string& ERM) {
    return new ASTForLoopRangeExpressionNode(ERM);
  }

  virtual void print() const override {
    std::cout << "<ForLoopRangeExpression>" << std::endl;
    IVL.print();
    BOP->print();
    std::cout << "</ForLoopRangeExpression>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTForLoopNode : public ASTStatementNode {
  friend class ASTForStatementNode;

public:
  enum IVMethod {
    IVMonotonic = 1,
    IVDiscrete,
    IVRange
  };

private:
  ASTIntegerList IVL;
  ASTStatementList FSL;
  ASTForLoopRangeExpressionNode* LRE;
  ASTIntNode* IV;
  std::map<std::string, const ASTSymbolTableEntry*> STM;
  int STP;
  IVMethod IVM;

private:
  void DetermineStepping() {
    switch (IVM) {
    case IVMonotonic:
      if (IVL.Size() == 3)
        STP = IVL[1];
      else
        STP = 1;
      break;
    case IVDiscrete:
      STP = -1;
      break;
    default:
      STP = 0;
      break;
    }

    IVL.SetStepping(STP);
  }

public:
  static const unsigned ForLoopBits = 64U;

public:
  ASTForLoopNode(const ASTIdentifierNode* Id, const ASTIntegerList& IL,
                 const ASTStatementList& SL, ASTIntNode* IVR,
                 IVMethod MT = IVMonotonic)
  : ASTStatementNode(Id), IVL(IL), FSL(SL), LRE(nullptr), IV(IVR),
  STM(), STP(1), IVM(MT) {
    DetermineStepping();
  }

  ASTForLoopNode(const ASTIdentifierNode* Id, const ASTIntegerList& IL,
                 ASTStatement* S, ASTIntNode* IVR, IVMethod MT = IVMonotonic)
  : ASTStatementNode(Id), IVL(IL), FSL(), LRE(nullptr), IV(IVR),
  STM(), STP(1), IVM(MT) {
    FSL.Append(S);
    DetermineStepping();
  }

  ASTForLoopNode(const ASTIdentifierNode* Id, ASTForLoopRangeExpressionNode* LE,
                 const ASTStatementList& SL, ASTIntNode* IVR)
  : ASTStatementNode(Id), IVL(), FSL(SL), LRE(LE), IV(IVR),
  STM(), STP(1), IVM(IVMonotonic) {
    DetermineStepping();
  }

  ASTForLoopNode(const ASTIdentifierNode* Id, ASTForLoopRangeExpressionNode* LE,
                 ASTStatement* S, ASTIntNode* IVR)
  : ASTStatementNode(Id), IVL(), FSL(), LRE(LE), IV(IVR),
  STM(), STP(1), IVM(IVMonotonic) {
    FSL.Append(S);
    DetermineStepping();
  }

  virtual ~ASTForLoopNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeForLoop;
  }

  virtual void SetStepping(int S) {
    STP = S;
  }

  virtual int GetStepping() const {
    return STP;
  }

  virtual void SetIVMethod(IVMethod M) {
    IVM = M;
  }

  virtual IVMethod GetIVMethod() const {
    return IVM;
  }

  virtual ASTIntNode* GetIndVar() {
    return IV;
  }

  virtual const ASTIntNode* GetIndVar() const {
    return IV;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual ASTForLoopRangeExpressionNode* GetLoopRangeExpression() {
    return LRE;
  }

  virtual const ASTForLoopRangeExpressionNode* GetLoopRangeExpression() const {
    return LRE;
  }

  const ASTIntegerList& GetIntegerList() const {
    return IVL;
  }

  const ASTStatementList& GetStatementList() const {
    return FSL;
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

  static const char* PrintIVMethod(IVMethod M) {
    switch (M) {
    case IVMonotonic:
      return "IVMonotonic";
      break;
    case IVDiscrete:
      return "IVDiscrete";
      break;
    case IVRange:
      return "IVRange";
      break;
    default:
      return "Unknown";
      break;
    }
  }

  virtual void print() const override {
    std::cout << "<ForLoop>" << std::endl;
    std::cout << "<IVMethod>" << PrintIVMethod(IVM) << "</IVMethod>"
      << std::endl;
    std::cout << "<Stepping>" << STP << "</Stepping>" << std::endl;
    std::cout << "<InductionVariable>" << IV->GetName()
      << "</InductionVariable>" << std::endl;
    std::cout << "<IndexSet>" << std::endl;
    IVL.print();
    std::cout << "</IndexSet>" << std::endl;
    FSL.print();
    if (LRE)
      LRE->print();
    std::cout << "</ForLoop>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTForStatementNode : public ASTStatementNode {
private:
  const ASTForLoopNode* Loop;

protected:
  ASTForStatementNode(const ASTIdentifierNode* Id, const std::string& ERM)
  : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, new ASTStringNode(ERM))),
  Loop(nullptr) { }

public:
  ASTForStatementNode(const ASTForLoopNode* L)
  : ASTStatementNode(L->GetIdentifier()), Loop(L) { }

  virtual ~ASTForStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeForStatement;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual const ASTForLoopNode* GetLoop() const {
    return Loop;
  }

  bool IsError() const override {
    return ASTStatementNode::IsError();
  }

  const std::string& GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTForStatementNode* StatementError(const std::string& ERM) {
    return new ASTForStatementNode(ASTIdentifierNode::For.Clone(), ERM);
  }

  static ASTForStatementNode* StatementError(const ASTIdentifierNode* Id,
                                           const std::string& ERM) {
    return new ASTForStatementNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<ForStatementNode>" << std::endl;
    Loop->print();
    std::cout << "</ForStatementNode>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTWhileLoopNode : public ASTStatementNode {
private:
  const ASTExpressionNode* Expr;
  ASTStatementList* WSL;
  std::map<std::string, const ASTSymbolTableEntry*> STM;
  ASTType ETy;

public:
  static const unsigned WhileLoopBits = 64U;

public:
  ASTWhileLoopNode(const ASTIdentifierNode* Id, const ASTExpressionNode* EN,
                   ASTStatementList* SL)
  : ASTStatementNode(Id), Expr(EN), WSL(SL), STM(), ETy(EN->GetASTType()) { }

  virtual ~ASTWhileLoopNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeWhileLoop;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  const ASTExpressionNode* GetExpression() const override {
    return Expr;
  }

  const ASTStatementList& GetStatementList() const {
    return *WSL;
  }

  ASTType GetExpressionType() const {
    return ETy;
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

  virtual void print() const override {
    std::cout << "<WhileLoopNode>" << std::endl;
    std::cout << "<ExpressionType>"
      << QASM::PrintTypeEnum(ETy) << "</ExpressionType>" << std::endl;
    Expr->print();
    WSL->print();
    std::cout << "</WhileLoopNode>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTWhileStatementNode : public ASTStatementNode {
private:
  const ASTWhileLoopNode* Loop;

protected:
  ASTWhileStatementNode(const ASTIdentifierNode* Id, const std::string& ERM)
  : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, new ASTStringNode(ERM))),
  Loop(nullptr) { }

public:
  ASTWhileStatementNode(const ASTWhileLoopNode* L)
  : ASTStatementNode(L->GetIdentifier()), Loop(L) { }

  virtual ~ASTWhileStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeWhileStatement;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual const ASTWhileLoopNode* GetLoop() const {
    return Loop;
  }

  bool IsError() const override {
    return ASTStatementNode::IsError();
  }

  const std::string& GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTWhileStatementNode* StatementError(const std::string& ERM) {
    return new ASTWhileStatementNode(ASTIdentifierNode::While.Clone(), ERM);
  }

  static ASTWhileStatementNode* StatementError(const ASTIdentifierNode* Id,
                                               const std::string& ERM) {
    return new ASTWhileStatementNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<WhileStatementNode>" << std::endl;
    Loop->print();
    std::cout << "</WhileStatementNode>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTDoWhileLoopNode : public ASTStatementNode {
private:
  const ASTExpressionNode* Expr;
  ASTStatementList* DWSL;
  std::map<std::string, const ASTSymbolTableEntry*> STM;
  ASTType ETy;

public:
  static const unsigned DoWhileLoopBits = 64U;

public:
  ASTDoWhileLoopNode(const ASTIdentifierNode* Id, const ASTExpressionNode* EN,
                     ASTStatementList* SL)
  : ASTStatementNode(Id), Expr(EN), DWSL(SL), STM(), ETy(EN->GetASTType()) { }

  virtual ~ASTDoWhileLoopNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeDoWhileLoop;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  const ASTExpressionNode* GetExpression() const override {
    return Expr;
  }

  const ASTStatementList& GetStatementList() const {
    return *DWSL;
  }

  ASTType GetExpressionType() const {
    return ETy;
  }

  virtual std::map<std::string, const ASTSymbolTableEntry*>& GetSymbolTable() {
    return STM;
  }

  virtual const std::map<std::string, const ASTSymbolTableEntry*>&
  GetSymbolTable() const {
    return STM;
  }

  virtual const ASTSymbolTableEntry* GetSymbol(const std::string& SN) const {
    std::map<std::string, const ASTSymbolTableEntry*>::const_iterator I =
      STM.find(SN);
    return I == STM.end() ? nullptr : (*I).second;
  }

  virtual void print() const override {
    std::cout << "<DoWhileLoopNode>" << std::endl;
    std::cout << "<ExpressionType>"
      << QASM::PrintTypeEnum(ETy) << "</ExpressionType>" << std::endl;
    Expr->print();
    DWSL->print();
    std::cout << "</DoWhileLoopNode>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTDoWhileStatementNode : public ASTStatementNode {
private:
  const ASTDoWhileLoopNode* Loop;

protected:
  ASTDoWhileStatementNode(const ASTIdentifierNode* Id, const std::string& ERM)
  : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, new ASTStringNode(ERM))),
  Loop(nullptr) { }

public:
  ASTDoWhileStatementNode(const ASTDoWhileLoopNode* L)
  : ASTStatementNode(L->GetIdentifier()), Loop(L) { }

  virtual ~ASTDoWhileStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeDoWhileStatement;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual const ASTDoWhileLoopNode* GetLoop() const {
    return Loop;
  }

  bool IsError() const override {
    return ASTStatementNode::IsError();
  }

  const std::string& GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTDoWhileStatementNode* StatementError(const std::string& ERM) {
    return new ASTDoWhileStatementNode(ASTIdentifierNode::DoWhile.Clone(), ERM);
  }

  static ASTDoWhileStatementNode* StatementError(const ASTIdentifierNode* Id,
                                                 const std::string& ERM) {
    return new ASTDoWhileStatementNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<DoWhileStatementNode>" << std::endl;
    Loop->print();
    std::cout << "</DoWhileStatementNode>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM

#endif // __QASM_AST_LOOPS_H

