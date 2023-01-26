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

#ifndef __QASM_AST_BARRIER_H
#define __QASM_AST_BARRIER_H

#include <qasm/AST/ASTGates.h>

namespace QASM {

class ASTBarrierNode : public ASTGateQOpNode {
private:
  ASTIdentifierList OperandList;

private:
  ASTBarrierNode() = delete;

public:
  using list_type = typename std::vector<ASTIdentifierNode*>;
  using const_iterator = typename list_type::const_iterator;

public:
  static const unsigned BarrierBits = 64U;

public:
  ASTBarrierNode(const ASTIdentifierList& List)
  : ASTGateQOpNode(), OperandList(List) { }

  ASTBarrierNode(const ASTIdentifierNode* Id, const ASTIdentifierList& List)
  : ASTGateQOpNode(Id), OperandList(List) { }

  virtual ~ASTBarrierNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeBarrier;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  const ASTIdentifierList& GetOperandList() const {
    return OperandList;
  }

  ASTIdentifierList& GetOperandList() {
    return OperandList;
  }

  const_iterator begin() const { return OperandList.begin(); }

  const_iterator end() const { return OperandList.end(); }

  virtual void print() const override {
    std::cout << "<BarrierNode>" << std::endl;
    ASTStatementNode::print();

    if (!OperandList.Empty()) {
      std::cout << "<OperandList>" << std::endl;
      OperandList.print();
      std::cout << "</OperandList>" << std::endl;
    }

    std::cout << "</BarrierNode>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM

#endif // __QASM_AST_BARRIER_H

