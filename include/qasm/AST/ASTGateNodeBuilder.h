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

#ifndef __QASM_AST_GATE_NODE_BUILDER_H
#define __QASM_AST_GATE_NODE_BUILDER_H

#include <qasm/AST/ASTGates.h>
#include <qasm/AST/ASTGateNodeList.h>

namespace QASM {

class ASTGateNodeBuilder {
private:
  static ASTGateNodeList NL;
  static ASTGateNodeBuilder B;

protected:
  ASTGateNodeBuilder() { }

public:
  using list_type = std::vector<ASTGateNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTGateNodeBuilder& Instance() {
    return ASTGateNodeBuilder::B;
  }

  ~ASTGateNodeBuilder() = default;

  static ASTGateNodeList* List() {
    return &ASTGateNodeBuilder::NL;
  }

  void Append(ASTGateNode* G) {
    NL.Append(G);
  }

  void Clear() {
    NL.Clear();
  }

  size_t Size() {
    return NL.Size();
  }

  bool IsGlobalGate(const std::string& GS) {
    return NL.IsGlobalGate(GS);
  }

  static ASTGateNode* Root() {
    return NL.front();
  }

  iterator begin() { return NL.begin(); }

  const_iterator begin() const { return NL.begin(); }

  iterator end() { return NL.end(); }

  const_iterator end() const { return NL.end(); }
};

} // namespace QASM

#endif // __QASM_AST_GATE_NODE_BUILDER_H

