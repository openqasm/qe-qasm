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

#ifndef __QASM_AST_SWITCH_STATEMENT_H
#define __QASM_AST_SWITCH_STATEMENT_H

#include <qasm/AST/ASTStatement.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTFlowControl.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTScopeController.h>
#include <qasm/AST/ASTFunctionCallExpr.h>

#include <vector>
#include <map>
#include <string>
#include <cassert>

namespace QASM {

class ASTSymbolTableEntry;

class ASTCaseStatementNode : public ASTStatementNode {
private:
  // single-statement case statements use a one-element list.
  const ASTStatementList* SL;
  const ASTBreakStatementNode* BRK;
  std::map<std::string, const ASTSymbolTableEntry*> STM;
  int64_t CIX;
  unsigned IX;
  ASTType STy;
  bool HS;

protected:
  ASTCaseStatementNode(const ASTIdentifierNode* Id, const std::string& ERM,
                       int64_t CI)
  : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)),
  SL(nullptr), BRK(nullptr), STM(), CIX(CI), IX(static_cast<unsigned>(~0x0)),
  STy(ASTTypeExpressionError), HS(false) { }

public:
  static const unsigned CaseBits = 64U;

public:
  ASTCaseStatementNode(int64_t CX, unsigned ID, const ASTStatementList* STL,
                       bool HSC)
  : ASTStatementNode(), SL(STL), BRK(nullptr), STM(), CIX(CX), IX(ID),
  STy(ASTTypeStatementList), HS(HSC) {
    const ASTDeclarationContext* CTX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
    assert(CTX && "Could not obtain a valid ASTDeclarationContext!");

    ASTScopeController::Instance().SetDeclarationContext(
                                   const_cast<ASTStatementList*>(STL), CTX);
    ASTScopeController::Instance().SetLocalScope(
                                   const_cast<ASTStatementList*>(STL));
  }

  ASTCaseStatementNode(int64_t CX, unsigned ID, const ASTStatementList* STL,
                       const ASTBreakStatementNode* BK, bool HSC)
  : ASTStatementNode(), SL(STL), BRK(BK), STM(), CIX(CX), IX(ID),
  STy(ASTTypeStatementList), HS(HSC) {
    const ASTDeclarationContext* CTX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
    assert(CTX && "Could not obtain a valid ASTDeclarationContext!");

    ASTScopeController::Instance().SetDeclarationContext(
                                   const_cast<ASTStatementList*>(STL), CTX);
    ASTScopeController::Instance().SetLocalScope(
                                   const_cast<ASTStatementList*>(STL));
  }

  virtual ~ASTCaseStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeCaseStatement;
  }

  virtual ASTType GetStatementType() const {
    return STy;
  }

  bool HasScope() const {
    return HS;
  }

  bool HasBreak() const {
    return BRK != nullptr;
  }

  virtual void SetHasScope(bool V = true) {
    HS = V;
  }

  virtual const ASTStatementList* GetStatementList() const {
    return SL;
  }

  virtual const ASTBreakStatementNode* GetBreak() const {
    return BRK;
  }

  virtual bool CheckDeclarations() const;

  virtual void SetCaseIndex(int64_t CX) {
    CIX = CX;
  }

  virtual int64_t GetCaseIndex() const {
    return CIX;
  }

  virtual void SetStatementIndex(unsigned I) {
    IX = I;
  }

  virtual unsigned GetStatementIndex() const {
    return IX;
  }

  std::map<std::string, const ASTSymbolTableEntry*>& GetSymbolTable() {
    return STM;
  }

  const std::map<std::string, const ASTSymbolTableEntry*>&
  GetSymbolTable() const {
    return STM;
  }

  const ASTSymbolTableEntry* GetSymbol(const std::string& S) const {
    std::map<std::string, const ASTSymbolTableEntry*>::const_iterator MI =
      STM.find(S);
    return MI == STM.end() ? nullptr : (*MI).second;
  }

  virtual bool IsError() const override {
    return ASTStatementNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTCaseStatementNode* StatementError(const std::string& ERM,
                                              int64_t CIX) {
    return new ASTCaseStatementNode(ASTIdentifierNode::Case.Clone(), ERM, CIX);
  }

  virtual void print() const override {
    std::cout << "<CaseStatementNode>" << std::endl;
    std::cout << "<StatementType>" << PrintTypeEnum(STy)
      << "</StatementType>" << std::endl;
    std::cout << "<CaseLabelValue>" << CIX << "</CaseLabelValue>" << std::endl;
    std::cout << "<StatementIndex>" << IX << "</StatementIndex>" << std::endl;
    std::cout << "<HasBreak>" << std::boolalpha << HasBreak()
      << "</HasBreak>" << std::endl;
    if (SL)
      SL->print();
    std::cout << "<HasScope>" << std::boolalpha << HS
      << "</HasScope>" << std::endl;
    std::cout << "</CaseStatementNode>" << std::endl;
  }
};

class ASTDefaultStatementNode : public ASTStatementNode {
private:
  const ASTStatementList* SL;
  const ASTBreakStatementNode* BRK;
  std::map<std::string, const ASTSymbolTableEntry*> STM;
  unsigned IX;
  ASTType STy;
  bool HS;

protected:
  ASTDefaultStatementNode(const ASTIdentifierNode* Id, const std::string& ERM)
  : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)),
  SL(nullptr), BRK(nullptr), STM(), IX(static_cast<unsigned>(~0x0)),
  STy(ASTTypeExpressionError), HS(false) { }

public:
  static const unsigned DefaultBits = 64U;

public:
  ASTDefaultStatementNode(unsigned ID, const ASTStatementList* STL, bool HSC)
  : ASTStatementNode(), SL(STL), BRK(nullptr), STM(), IX(ID),
  STy(ASTTypeStatementList), HS(HSC) {
    const ASTDeclarationContext* CTX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
    assert(CTX && "Could not obtain a valid ASTDeclarationContext!");

    ASTScopeController::Instance().SetDeclarationContext(
      const_cast<ASTStatementList*>(STL), CTX);
    ASTScopeController::Instance().SetLocalScope(
      const_cast<ASTStatementList*>(STL));
  }

  ASTDefaultStatementNode(unsigned ID, const ASTStatementList* STL,
                          const ASTBreakStatementNode* BK, bool HSC)
  : ASTStatementNode(), SL(STL), BRK(BK), STM(), IX(ID),
  STy(ASTTypeStatementList), HS(HSC) {
    const ASTDeclarationContext* CTX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
    assert(CTX && "Could not obtain a valid ASTDeclarationContext!");

    ASTScopeController::Instance().SetDeclarationContext(
      const_cast<ASTStatementList*>(STL), CTX);
    ASTScopeController::Instance().SetLocalScope(
      const_cast<ASTStatementList*>(STL));
  }

  virtual ~ASTDefaultStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeDefaultStatement;
  }

  bool HasScope() const {
    return HS;
  }

  bool HasBreak() const {
   return BRK != nullptr;
  }

  virtual void SetHasScope(bool V = true) {
    HS = V;
  }

  virtual const ASTStatementList* GetStatementList() const {
    return SL;
  }

  virtual const ASTBreakStatementNode* GetBreak() const {
    return BRK;
  }

  virtual bool CheckDeclarations() const;

  virtual void SetStatementIndex(unsigned I) {
    IX = I;
  }

  virtual unsigned GetStatementIndex() const {
    return IX;
  }

  std::map<std::string, const ASTSymbolTableEntry*>& GetSymbolTable() {
    return STM;
  }

  const std::map<std::string, const ASTSymbolTableEntry*>&
  GetSymbolTable() const {
    return STM;
  }

  const ASTSymbolTableEntry* GetSymbol(const std::string& S) const {
    std::map<std::string, const ASTSymbolTableEntry*>::const_iterator MI =
      STM.find(S);
    return MI == STM.end() ? nullptr : (*MI).second;
  }

  virtual bool IsError() const override {
    return ASTStatementNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTDefaultStatementNode* StatementError(const std::string& ERM) {
    return new ASTDefaultStatementNode(ASTIdentifierNode::Default.Clone(), ERM);
  }

  virtual void print() const override {
    std::cout << "<DefaultStatementNode>" << std::endl;
    std::cout << "<StatementType>" << PrintTypeEnum(STy)
      << "</StatementType>" << std::endl;
    std::cout << "<DefaultStatementIndex>" << IX << "</DefaultStatementIndex>"
      << std::endl;
    std::cout << "<HasBreak>" << std::boolalpha << HasBreak()
      << "</HasBreak>" << std::endl;
    if (SL)
      SL->print();
    std::cout << "<HasScope>" << std::boolalpha << HS
      << "</HasScope>" << std::endl;
    std::cout << "</DefaultStatementNode>" << std::endl;
  }
};

class ASTSwitchStatementNode : public ASTStatementNode {
private:
  std::map<unsigned, const ASTCaseStatementNode*> CSM;
  const ASTDefaultStatementNode* DSN;
  std::map<std::string, const ASTSymbolTableEntry*> STM;

  union {
    const ASTIntNode* I;
    const ASTMPIntegerNode* MPI;
    const ASTBinaryOpNode* BOP;
    const ASTUnaryOpNode* UOP;
    const ASTIdentifierNode* DId;
    const ASTFunctionCallNode* FC;
  };

  ASTType DTy;

private:
  void ResolveQuantity(const ASTIntNode* DI);

  void ResolveQuantity(const ASTMPIntegerNode* DI);

  void ResolveQuantity(const ASTBinaryOpNode* DI);

  void ResolveQuantity(const ASTUnaryOpNode* DI);

  void ResolveQuantity(const ASTIdentifierNode* DI);

  void ResolveQuantity(const ASTFunctionCallNode* DI);

  void ResolveDefaultStatement(const ASTStatementList* SL);

  void ResolveCaseStatements(const ASTStatementList* SL);

  void VerifyStatements(const ASTStatementList* SL);

private:
  ASTSwitchStatementNode() = delete;

protected:
  ASTSwitchStatementNode(const ASTIdentifierNode* Id, const std::string& ERM)
  : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, new ASTStringNode(ERM))),
  CSM(), DSN(nullptr), STM(), I(nullptr), DTy(ASTTypeExpressionError) { }

public:
  static const unsigned SwitchBits = 64U;

public:
  ASTSwitchStatementNode(const ASTIdentifierNode* Id, const ASTIntNode* DI,
                         const ASTStatementList* SL)
  : ASTStatementNode(Id), CSM(), DSN(nullptr), STM(), I(DI),
  DTy(DI->GetASTType()) {
    ResolveQuantity(DI);
    ResolveDefaultStatement(SL);
    ResolveCaseStatements(SL);
    VerifyStatements(SL);
  }

  ASTSwitchStatementNode(const ASTIdentifierNode* Id,
                         const ASTMPIntegerNode* DMPI,
                         const ASTStatementList* SL)
  : ASTStatementNode(Id), CSM(), DSN(nullptr), STM(), MPI(DMPI),
  DTy(DMPI->GetASTType()) {
    ResolveQuantity(DMPI);
    ResolveDefaultStatement(SL);
    ResolveCaseStatements(SL);
    VerifyStatements(SL);
  }

  ASTSwitchStatementNode(const ASTIdentifierNode* Id,
                         const ASTBinaryOpNode* DBOP,
                         const ASTStatementList* SL)
  : ASTStatementNode(Id), CSM(), DSN(nullptr), STM(), BOP(DBOP),
  DTy(DBOP->GetASTType()) {
    ResolveQuantity(DBOP);
    ResolveDefaultStatement(SL);
    ResolveCaseStatements(SL);
    VerifyStatements(SL);
  }

  ASTSwitchStatementNode(const ASTIdentifierNode* Id,
                         const ASTUnaryOpNode* DUOP,
                         const ASTStatementList* SL)
  : ASTStatementNode(Id), CSM(), DSN(nullptr), STM(), UOP(DUOP),
  DTy(DUOP->GetASTType()) {
    ResolveQuantity(DUOP);
    ResolveDefaultStatement(SL);
    ResolveCaseStatements(SL);
    VerifyStatements(SL);
  }

  ASTSwitchStatementNode(const ASTIdentifierNode* Id,
                         const ASTIdentifierNode* DDId,
                         const ASTStatementList* SL)
  : ASTStatementNode(Id), CSM(), DSN(nullptr), STM(), DId(DDId),
  DTy(DDId->GetSymbolType()) {
    ResolveQuantity(DDId);
    ResolveDefaultStatement(SL);
    ResolveCaseStatements(SL);
    VerifyStatements(SL);
  }

  ASTSwitchStatementNode(const ASTIdentifierNode* Id,
                         const ASTFunctionCallNode* DFC,
                         const ASTStatementList* SL)
  : ASTStatementNode(Id), CSM(), DSN(nullptr), STM(), FC(DFC),
  DTy(DFC->GetASTType()) {
    ResolveQuantity(DFC);
    ResolveDefaultStatement(SL);
    ResolveCaseStatements(SL);
    VerifyStatements(SL);
  }

  virtual ~ASTSwitchStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeSwitchStatement;
  }

  const std::map<unsigned, const ASTCaseStatementNode*>&
  GetCaseStatementsMap() const {
    return CSM;
  }

  const ASTCaseStatementNode* GetCaseStatement(unsigned IX) const {
    std::map<unsigned, const ASTCaseStatementNode*>::const_iterator FI =
      CSM.find(IX);
    return FI == CSM.end() ? nullptr : (*FI).second;
  }

  unsigned GetNumCaseStatements() const {
    return static_cast<unsigned>(CSM.size());
  }

  const ASTDefaultStatementNode* GetDefaultStatement() const {
    return DSN;
  }

  bool HasDefaultLabel() const {
    return DSN;
  }

  virtual bool CheckValidLabels() const {
    return DSN || CSM.size();
  }

  virtual std::map<std::string, const ASTSymbolTableEntry*>& GetSymbolTable() {
    return STM;
  }

  virtual const std::map<std::string, const ASTSymbolTableEntry*>&
  GetSymbolTable() const {
    return STM;
  }

  virtual const ASTSymbolTableEntry* GetSymbol(const std::string& SN) const {
    std::map<std::string, const ASTSymbolTableEntry*>::const_iterator MI =
      STM.find(SN);
    return MI == STM.end() ? nullptr : (*MI).second;
  }

  ASTType GetQuantityType() const {
    return DTy;
  }

  const ASTIdentifierNode* GetIdentifierQuantity() const {
    return DTy == ASTTypeIdentifier ? DId : nullptr;
  }

  const ASTIntNode* GetIntQuantity() const {
    return (DTy == ASTTypeInt || DTy == ASTTypeUInt) ? I : nullptr;
  }

  const ASTMPIntegerNode* GetMPIntegerQuantity() const {
    return (DTy == ASTTypeMPInteger || DTy == ASTTypeMPUInteger) ? MPI : nullptr;
  }

  const ASTBinaryOpNode* GetBinaryOpQuantity() const {
    return DTy == ASTTypeBinaryOp ? BOP : nullptr;
  }

  const ASTUnaryOpNode* GetUnaryOpQuantity() const {
    return DTy == ASTTypeUnaryOp ? UOP : nullptr;
  }

  const ASTFunctionCallNode* GetFunctionCallQuantity() const {
    return DTy == ASTTypeFunctionCallExpression ? FC : nullptr;
  }

  bool IsError() const override {
    return ASTStatementNode::IsError();
  }

  const std::string& GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTSwitchStatementNode* StatementError(const std::string& ERM) {
    return new ASTSwitchStatementNode(ASTIdentifierNode::Switch.Clone(), ERM);
  }

  static ASTSwitchStatementNode* StatementError(const ASTIdentifierNode* Id,
                                                const std::string& ERM) {
    return new ASTSwitchStatementNode(Id, ERM);
  }

  virtual void print_quantity() const {
    std::cout << "<SwitchQuantity>" << std::endl;

    switch (DTy) {
    case ASTTypeInt:
    case ASTTypeUInt:
      std::cout << "<Type>" << PrintTypeEnum(I->GetASTType())
        << "</Type>" << std::endl;
      I->print();
      break;
    case ASTTypeMPInteger:
      std::cout << "<Type>" << PrintTypeEnum(MPI->GetASTType())
        << "</Type>" << std::endl;
      MPI->print();
      break;
    case ASTTypeBinaryOp:
      std::cout << "<Type>" << PrintTypeEnum(BOP->GetASTType())
        << "</Type>" << std::endl;
      BOP->print();
      break;
    case ASTTypeUnaryOp:
      std::cout << "<Type>" << PrintTypeEnum(UOP->GetASTType())
        << "</Type>" << std::endl;
      UOP->print();
      break;
    case ASTTypeIdentifier:
      std::cout << "<Type>" << PrintTypeEnum(DId->GetASTType())
        << "</Type>" << std::endl;
      DId->print();
      break;
    case ASTTypeFunctionCall:
      std::cout << "<Type>" << PrintTypeEnum(FC->GetASTType())
        << "</Type>" << std::endl;
      FC->print();
      break;
    default:
      break;
    }

    std::cout << "</SwitchQuantity>" << std::endl;
  }

  virtual void print_switch_cases() const {
    std::cout << "<SwitchCaseStatements>" << std::endl;

    std::map<unsigned, const ASTCaseStatementNode*>::const_iterator FI;
    for (FI = CSM.begin(); FI != CSM.end(); ++FI) {
      (*FI).second->print();
    }

    std::cout << "</SwitchCaseStatements>" << std::endl;
  }

  virtual void print_switch_default() const {
    std::cout << "<SwitchDefaultStatement>" << std::endl;
    if (DSN)
      DSN->print();
    std::cout << "</SwitchDefaultStatement>" << std::endl;
  }

  virtual void print() const override {
    std::cout << "<SwitchStatement>" << std::endl;
    print_quantity();
    print_switch_cases();
    print_switch_default();
    std::cout << "</SwitchStatement>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_SWITCH_STATEMENT_H

