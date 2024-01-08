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

#ifndef __QASM_AST_BINARY_OP_ASSIGN_LIST_H
#define __QASM_AST_BINARY_OP_ASSIGN_LIST_H

#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <vector>

namespace QASM {

class ASTBinaryOpAssignList {
private:
  std::vector<ASTExpressionNode *> List;

public:
  using list_type = std::vector<ASTExpressionNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTBinaryOpAssignList() = default;
  virtual ~ASTBinaryOpAssignList() = default;

  void Append(ASTBinaryOpNode *BOP) {
    assert(BOP && "Invalid ASTBinaryOpNode argument!");
    assert(BOP->GetOpType() == ASTOpTypeAssign && "Invalid BinaryOp Type!");
    if (BOP && (BOP->GetOpType() == ASTOpTypeAssign))
      List.push_back(dynamic_cast<ASTExpressionNode *>(BOP));
  }

  void Append(ASTIdentifierNode *IDN) {
    assert(IDN && "Invalid ASTIdentifierNode argument!");
    if (IDN)
      List.push_back(dynamic_cast<ASTExpressionNode *>(IDN));
  }

  void Append(ASTExpressionNode *EN) {
    assert(EN && "Invalid ASTExpressionNode argument!");
    switch (EN->GetASTType()) {
    case ASTTypeIdentifier:
      if (ASTIdentifierNode *Id = dynamic_cast<ASTIdentifierNode *>(EN))
        List.push_back(dynamic_cast<ASTExpressionNode *>(Id));
      break;
    case ASTTypeIdentifierRef:
      if (ASTIdentifierRefNode *IdR = dynamic_cast<ASTIdentifierRefNode *>(EN))
        List.push_back(dynamic_cast<ASTExpressionNode *>(IdR));
      break;
    case ASTTypeBinaryOp:
      if (ASTBinaryOpNode *BOP = dynamic_cast<ASTBinaryOpNode *>(EN)) {
        if (BOP->GetOpType() == ASTOpTypeAssign)
          List.push_back(dynamic_cast<ASTExpressionNode *>(BOP));
      }
      break;
    default:
      break;
    }
  }

  void Clear() { List.clear(); }

  bool Empty() const { return List.empty(); }

  size_t Size() const { return List.size(); }

  iterator begin() { return List.begin(); }

  const_iterator begin() const { return List.begin(); }

  iterator end() { return List.end(); }

  const_iterator end() const { return List.end(); }

  void print() const {
    std::cout << "<BinaryOpAssignList>" << std::endl;

    for (const_iterator I = List.begin(); I != List.end(); ++I)
      (*I)->print();

    std::cout << "</BinaryOpAssignList>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_BINARY_OP_ASSIGN_LIST_H
