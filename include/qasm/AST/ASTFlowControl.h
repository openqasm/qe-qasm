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

#ifndef __QASM_AST_FLOW_CONTROL_H
#define __QASM_AST_FLOW_CONTROL_H

#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTStatement.h>
#include <qasm/AST/ASTTypes.h>

namespace QASM {

class ASTBreakStatementNode : public ASTStatementNode {
public:
  static const unsigned BreakBits = 64U;

public:
  ASTBreakStatementNode(const ASTIdentifierNode *Id) : ASTStatementNode(Id) {}

  virtual ~ASTBreakStatementNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeBreak; }

  virtual void print() const override {
    std::cout << "<BreakStatement>" << std::endl;
    std::cout << "</BreakStatement>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTContinueStatementNode : public ASTStatementNode {
public:
  static const unsigned ContinueBits = 64U;

public:
  ASTContinueStatementNode(const ASTIdentifierNode *Id)
      : ASTStatementNode(Id) {}

  virtual ~ASTContinueStatementNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeContinue; }

  virtual void print() const override {
    std::cout << "<ContinueStatement>" << std::endl;
    std::cout << "</ContinueStatement>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_FLOW_CONTROL_H
