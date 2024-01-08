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

#ifndef __QASM_AST_PARAMETER_BUILDER_H
#define __QASM_AST_PARAMETER_BUILDER_H

#include <qasm/AST/ASTParameterList.h>

#include <vector>

namespace QASM {

class ASTParameterBuilder {
private:
  static ASTParameterList BL;
  static ASTParameterList *BLP;
  static ASTParameterBuilder B;
  static std::vector<ASTParameterList *> BLV;

protected:
  ASTParameterBuilder() = default;

public:
  using list_type = std::vector<ASTBase *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTParameterBuilder &Instance() { return ASTParameterBuilder::B; }

  ~ASTParameterBuilder() = default;

  static void Init() {
    if (!BLP) {
      BLP = new ASTParameterList();
      assert(BLP && "Could not create a valid ASTParameterList!");
      BLV.push_back(BLP);
    }
  }

  static ASTParameterList *NewList() {
    BLP = new ASTParameterList();
    assert(BLP && "Could not create a valid ASTParameterList!");
    BLV.push_back(BLP);
    return BLP;
  }

  static ASTParameterList *List() { return ASTParameterBuilder::BLP; }

  void Append(ASTBase *Node) { BLP->Append(Node); }

  void Clear() { BLP->Clear(); }

  size_t Size() { return BLP->Size(); }

  static ASTBase *Root() { return BLP->Graph.front(); }

  iterator begin() { return BLP->Graph.begin(); }

  const_iterator begin() const { return BLP->Graph.begin(); }

  iterator end() { return BLP->Graph.end(); }

  const_iterator end() const { return BLP->Graph.end(); }
};

} // namespace QASM

#endif // __QASM_AST_PARAMETER_BUILDER_H
