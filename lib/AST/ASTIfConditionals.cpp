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

#include <qasm/AST/ASTIfConditionals.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTIfStatementBuilder.h>

namespace QASM {

ASTIfStatementNode::ASTIfStatementNode(const ASTExpressionNode* E,
                                       const ASTStatementNode* SN)
  : ASTStatementNode(E), ElseIf(), Else(nullptr), PIF(nullptr),
  OpNode(SN), OpList(nullptr), STM(), StackFrame((unsigned) ~0x0),
  ISC(ASTIfStatementBuilder::Instance().GetCurrentMapIndex()) { }

ASTIfStatementNode::ASTIfStatementNode(const ASTExpressionNode* E,
                                       const ASTStatementList* SL)
  : ASTStatementNode(E), ElseIf(), Else(nullptr), PIF(nullptr),
  OpNode(nullptr), OpList(SL), STM(), StackFrame((unsigned) ~0x0),
  ISC(ASTIfStatementBuilder::Instance().GetCurrentMapIndex()) { }

ASTIfStatementNode::ASTIfStatementNode(const ASTExpressionNode* E,
                                       const ASTStatementNode* SN,
                                       const ASTStatementList* SL)
  : ASTStatementNode(E), ElseIf(), Else(nullptr), PIF(nullptr),
  OpNode(SN), OpList(SL), STM(), StackFrame((unsigned) ~0x0),
  ISC(ASTIfStatementBuilder::Instance().GetCurrentMapIndex()) { }

ASTElseIfStatementNode::ASTElseIfStatementNode(const ASTIfStatementNode* IF)
  : ASTStatementNode(&ASTIdentifierNode::ElseIfExpression),
  IfStmt(IF), OpNode(nullptr), OpList(nullptr), STM(),
  StackFrame(IF->GetStackFrame()), ISC(IF->GetISC()) {
    this->AttachTo(IF);
    const_cast<ASTIfStatementNode*>(IF)->AddElseIf(this);
  }

ASTElseIfStatementNode::ASTElseIfStatementNode(const ASTIfStatementNode* IF,
                                               const ASTExpressionNode* EX,
                                               const ASTStatementList* SL)
  : ASTStatementNode(EX), IfStmt(IF), OpNode(nullptr), OpList(SL),
  STM(), StackFrame(IF->GetStackFrame()), ISC(IF->GetISC()) {
    this->AttachTo(IF);
    const_cast<ASTIfStatementNode*>(IF)->AddElseIf(this);
  }

ASTElseStatementNode::ASTElseStatementNode(const ASTIfStatementNode* IF)
  : ASTStatementNode(&ASTIdentifierNode::ElseExpression),
  IfStmt(IF), OpNode(nullptr), OpList(nullptr),
  STM(), StackFrame(IF->GetStackFrame()), ISC(IF->GetISC()) {
    this->AttachTo(IF);
    const_cast<ASTIfStatementNode*>(IF)->AddElse(this);
  }

ASTElseStatementNode::ASTElseStatementNode(const ASTIfStatementNode* IF,
                                           const ASTStatementList* SL)
  : ASTStatementNode(&ASTIdentifierNode::ElseExpression),
  IfStmt(IF), OpNode(nullptr), OpList(SL), STM(),
  StackFrame(IF->GetStackFrame()), ISC(IF->GetISC()) {
    this->AttachTo(IF);
    const_cast<ASTIfStatementNode*>(IF)->AddElse(this);
  }

void ASTElseIfStatementNode::AttachTo(const ASTIfStatementNode* IfNode) {
  IfStmt = IfNode;
  StackFrame = IfNode->GetStackFrame();
  ISC = IfNode->GetISC();
}

void ASTElseStatementNode::AttachTo(const ASTIfStatementNode* IfNode) {
  IfStmt = IfNode;
  StackFrame = IfNode->GetStackFrame();
  ISC = IfNode->GetISC();
}

void ASTIfStatementNode::print_header() const {
  std::cout << "<IfStatementHeader>" << std::endl;
  std::cout << "<StackFrame>" << StackFrame << "</StackFrame>"
    << std::endl;
  std::cout << "<ISC>" << ISC << "</ISC>" << std::endl;
  std::cout << "<IfStatement>" << (const void*) this
    << "</IfStatement>" << std::endl;
  std::cout << "</IfStatementHeader>" << std::endl;
}

void ASTIfStatementNode::print() const {
  std::cout << "<IfStatement>" << std::endl;
  std::cout << "<StackFrame>" << StackFrame << "</StackFrame>"
    << std::endl;
  std::cout << "<ISC>" << ISC << "</ISC>" << std::endl;
  if (Expr)
    Expr->print();

  if (PIF) {
    std::cout << "<ParentIf>" << std::endl;
    PIF->print_header();
    std::cout << "</ParentIf>" << std::endl;
  } else {
    std::cout << "<ParentIf>" << "0x0" << "</ParentIf>" << std::endl;
  }

  if (OpNode)
    OpNode->print();

  if (OpList)
    OpList->print();

  if (!STM.empty()) {
    std::cout << "<SymbolTable>" << std::endl;
    for (std::map<std::string, const ASTSymbolTableEntry*>::const_iterator PI =
         STM.begin(); PI != STM.end(); ++PI) {
      std::cout << "<SymbolName>" << (*PI).first << "</SymbolName>" << std::endl;
      if ((*PI).second)
        std::cout << "<SymbolType>" << PrintTypeEnum((*PI).second->GetValueType())
          << "</SymbolType>" << std::endl;
    }
    std::cout << "</SymbolTable>" << std::endl;
  }

  std::cout << "</IfStatement>" << std::endl;
}

void ASTElseIfStatementNode::print() const {
  std::cout << "<ElseIfStatement>" << std::endl;
  std::cout << "<StackFrame>" << StackFrame << "</StackFrame>"
    << std::endl;
  std::cout << "<ISC>" << ISC << "</ISC>" << std::endl;
  if (IfStmt) {
    std::cout << "<IfStatementLatch>" << std::endl;
    IfStmt->print_header();
    std::cout << "</IfStatementLatch>" << std::endl;
  }
  if (Expr)
    Expr->print();
  if (OpNode)
    OpNode->print();
  if (OpList)
    OpList->print();

  if (!STM.empty()) {
    std::cout << "<SymbolTable>" << std::endl;
    for (std::map<std::string, const ASTSymbolTableEntry*>::const_iterator PI =
         STM.begin(); PI != STM.end(); ++PI) {
      std::cout << "<SymbolName>" << (*PI).first << "</SymbolName>" << std::endl;
      if ((*PI).second)
        std::cout << "<SymbolType>" << PrintTypeEnum((*PI).second->GetValueType())
          << "</SymbolType>" << std::endl;
    }
    std::cout << "</SymbolTable>" << std::endl;
  }

  std::cout << "</ElseIfStatement>" << std::endl;
}

void ASTElseStatementNode::print() const {
  std::cout << "<ElseStatement>" << std::endl;
  std::cout << "<StackFrame>" << StackFrame << "</StackFrame>"
    << std::endl;
  std::cout << "<ISC>" << ISC << "</ISC>" << std::endl;
  if (IfStmt) {
    std::cout << "<IfStatementLatch>" << std::endl;
    IfStmt->print_header();
    std::cout << "</IfStatementLatch>" << std::endl;
  }
  if (Expr)
    Expr->print();
  if (OpNode)
    OpNode->print();
  if (OpList)
    OpList->print();

  if (!STM.empty()) {
    std::cout << "<SymbolTable>" << std::endl;
    for (std::map<std::string, const ASTSymbolTableEntry*>::const_iterator PI =
         STM.begin(); PI != STM.end(); ++PI) {
      std::cout << "<SymbolName>" << (*PI).first << "</SymbolName>" << std::endl;
      if ((*PI).second)
        std::cout << "<SymbolType>" << PrintTypeEnum((*PI).second->GetValueType())
          << "</SymbolType>" << std::endl;
    }
    std::cout << "</SymbolTable>" << std::endl;
  }

  std::cout << "</ElseStatement>" << std::endl;
}

} // namespace QASM

