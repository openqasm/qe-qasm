/* -*- coding: utf-8 -*-
 *
 * Copyright 2021 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_BINARY_OP_ASSIGN_BUILDER_H
#define __QASM_AST_BINARY_OP_ASSIGN_BUILDER_H

#include <qasm/AST/ASTBinaryOpAssignList.h>

namespace QASM {

class ASTBinaryOpAssignBuilder {
private:
  static ASTBinaryOpAssignBuilder BOB;
  ASTBinaryOpAssignList BOPL;

protected:
  ASTBinaryOpAssignBuilder() {}

public:
  using list_type = std::vector<ASTExpressionNode *>;
  using iterator = typename std::vector<ASTExpressionNode *>::iterator;
  using const_iterator =
      typename std::vector<ASTExpressionNode *>::const_iterator;

public:
  static ASTBinaryOpAssignBuilder &Instance() {
    return ASTBinaryOpAssignBuilder::BOB;
  }

  virtual ~ASTBinaryOpAssignBuilder() = default;

  ASTBinaryOpAssignList *List() { return &BOPL; }

  void Append(ASTBinaryOpNode *Node) { BOPL.Append(Node); }

  void Clear() { BOPL.Clear(); }

  size_t Size() { return BOPL.Size(); }

  iterator begin() { return BOPL.begin(); }

  const_iterator begin() const { return BOPL.begin(); }

  iterator end() { return BOPL.end(); }

  const_iterator end() const { return BOPL.end(); }
};

} // namespace QASM

#endif // __QASM_AST_BINARY_OP_ASSIGN_BUILDER_H
