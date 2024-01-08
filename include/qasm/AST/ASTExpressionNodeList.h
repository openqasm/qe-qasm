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

#ifndef __QASM_AST_EXPRESSION_NODE_LIST_H
#define __QASM_AST_EXPRESSION_NODE_LIST_H

#include <qasm/AST/ASTDeclarationList.h>
#include <qasm/AST/ASTParameterList.h>
#include <qasm/AST/ASTStringList.h>
#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <vector>

namespace QASM {

class ASTExpressionNodeList : public ASTBase {
  friend class ASTDefcalParameterBuilder;
  friend class ASTExpressionNodeBuilder;

protected:
  std::vector<ASTExpressionNode *> List;

public:
  using list_type = std::vector<ASTExpressionNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTExpressionNodeList() = default;

  ASTExpressionNodeList(const ASTDeclarationList &DL);

  ASTExpressionNodeList(const ASTParameterList &PL);

  ASTExpressionNodeList(const ASTExpressionList &EL);

  ASTExpressionNodeList(const ASTStringList &SL);

  virtual ~ASTExpressionNodeList() = default;

  void Clear() { List.clear(); }

  virtual ASTType GetASTType() const override {
    return ASTTypeExpressionNodeList;
  }

  bool Empty() const { return List.empty(); }

  void Append(ASTExpressionNode *EN) {
    assert(EN && "Invalid ASTExpressionNode argument!");
    List.push_back(EN);
  }

  void Append(const ASTExpressionNode *EN) {
    assert(EN && "Invalid ASTExpressionNode argument!");
    List.push_back(const_cast<ASTExpressionNode *>(EN));
  }

  void Prepend(ASTExpressionNode *EN) {
    assert(EN && "Invalid ASTExpressionNode argument!");
    List.insert(List.begin(), EN);
  }

  void Prepend(const ASTExpressionNode *EN) {
    assert(EN && "Invalid ASTExpressionNode argument!");
    List.insert(List.begin(), const_cast<ASTExpressionNode *>(EN));
  }

  iterator begin() { return List.begin(); }

  const_iterator begin() const { return List.begin(); }

  iterator end() { return List.end(); }

  const_iterator end() const { return List.end(); }

  ASTExpressionNode *front() { return List.size() ? List.front() : nullptr; }

  const ASTExpressionNode *front() const {
    return List.size() ? List.front() : nullptr;
  }

  ASTExpressionNode *back() { return List.size() ? List.back() : nullptr; }

  const ASTExpressionNode *back() const {
    return List.size() ? List.back() : nullptr;
  }

  size_t Size() const { return List.size(); }

  virtual void print() const override {
    std::cout << "<ExpressionNodeList>" << std::endl;

    for (const_iterator I = List.begin(); I != List.end(); ++I)
      (*I)->print();

    std::cout << "</ExpressionNodeList>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_EXPRESSION_NODE_LIST_H
