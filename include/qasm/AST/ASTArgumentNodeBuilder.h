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

#ifndef __QASM_AST_ARGUMENT_NODE_BUILDER_H
#define __QASM_AST_ARGUMENT_NODE_BUILDER_H

#include <qasm/AST/ASTArgument.h>

namespace QASM {

class ASTArgumentNodeBuilder {
private:
  static ASTArgumentNodeList AL;
  static ASTArgumentNodeBuilder B;
  static ASTArgumentNodeList *ALP;
  static std::vector<ASTArgumentNodeList *> ALV;

protected:
  ASTArgumentNodeBuilder() = default;

public:
  using list_type = std::vector<ASTArgumentNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTArgumentNodeBuilder &Instance() {
    return ASTArgumentNodeBuilder::B;
  }

  ~ASTArgumentNodeBuilder() = default;

  static ASTArgumentNodeList *List() { return ASTArgumentNodeBuilder::ALP; }

  static ASTArgumentNodeList *NewList() {
    ASTArgumentNodeList *ANL = new ASTArgumentNodeList();
    assert(ANL && "Could not create a valid ASTArgumentNodeList!");
    ALP = ANL;
    ALV.push_back(ALP);
    return ALP;
  }

  static void Init() {
    if (!ALP) {
      ALP = NewList();
      assert(ALP && "Could not create a valid ASTArgumentNodeList!");
      ALV.push_back(ALP);
    }
  }

  void Append(ASTBase *Node) { ALP->push(Node); }

  void Clear() { ALP->Clear(); }

  size_t Size() { return ALP->Size(); }

  static ASTBase *Root() { return ALP->front(); }

  iterator begin() { return ALP->begin(); }

  const_iterator begin() const { return ALP->begin(); }

  iterator end() { return ALP->end(); }

  const_iterator end() const { return ALP->end(); }
};

} // namespace QASM

#endif // __QASM_AST_ARGUMENT_NODE_BUILDER_H
