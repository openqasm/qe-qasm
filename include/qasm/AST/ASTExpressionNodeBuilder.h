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

#ifndef __QASM_AST_EXPRESSION_NODE_BUELDER_H
#define __QASM_AST_EXPRESSION_NODE_BUELDER_H

#include <qasm/AST/ASTExpressionNodeList.h>
#include <qasm/AST/ASTTypes.h>

namespace QASM {

class ASTExpressionNodeBuilder {
private:
  static ASTExpressionNodeList EL;
  static ASTExpressionNodeBuilder B;
  static ASTExpressionNodeList *ELP;
  static std::vector<ASTExpressionNodeList *> ELV;

protected:
  ASTExpressionNodeBuilder() = default;

public:
  using list_type = std::vector<ASTExpressionNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTExpressionNodeBuilder &Instance() {
    return ASTExpressionNodeBuilder::B;
  }

  virtual ~ASTExpressionNodeBuilder() = default;

  static ASTExpressionNodeList *List() { return ASTExpressionNodeBuilder::ELP; }

  static ASTExpressionNodeList *NewList() {
    ASTExpressionNodeList *IEL = new ASTExpressionNodeList();
    assert(IEL && "Could not create a valid ASTExpressionNodeList!");
    ELP = IEL;
    ELV.push_back(ELP);
    return ELP;
  }

  static void Init() {
    if (!ELP) {
      ELP = NewList();
      assert(ELP && "Could not create a valid ASTExpressionNodeList!");
      ELV.push_back(ELP);
    }
  }

  void Append(ASTExpressionNode *Node) { ELP->Append(Node); }

  void Prepend(ASTExpressionNode *Node) { ELP->Prepend(Node); }

  void Restart() {
    ASTExpressionNodeList *IEL = new ASTExpressionNodeList();
    assert(IEL && "Could not create a valid ASTExpressionNodeList!");
    ELP = IEL;
    ELV.push_back(ELP);
  }

  void Clear() { ELP->Clear(); }

  std::size_t Size() { return ELP->Size(); }

  static ASTExpressionNode *Root() { return ELP->List.front(); }

  iterator begin() { return ELP->List.begin(); }

  const_iterator begin() const { return ELP->List.begin(); }

  iterator end() { return ELP->List.end(); }

  const_iterator end() const { return ELP->List.end(); }
};

} // namespace QASM

#endif // __QASM_AST_EXPRESSION_NODE_BUELDER_H
