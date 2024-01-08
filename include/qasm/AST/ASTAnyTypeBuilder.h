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

#ifndef __QASM_AST_ANY_TYPE_BUILDER_H
#define __QASM_AST_ANY_TYPE_BUILDER_H

#include <qasm/AST/ASTAnyTypeList.h>

namespace QASM {

class ASTAnyTypeBuilder {
private:
  static ASTAnyTypeBuilder ATB;
  static ASTAnyTypeList ATL;
  static ASTAnyTypeList *ATP;
  static std::vector<ASTAnyTypeList *> ALV;

protected:
  ASTAnyTypeBuilder() {}

public:
  using list_type = std::vector<ASTAnyType>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTAnyTypeBuilder &Instance() { return ASTAnyTypeBuilder::ATB; }

  virtual ~ASTAnyTypeBuilder() = default;

  ASTAnyTypeList *List() const { return ASTAnyTypeBuilder::ATP; }

  static ASTAnyTypeList *NewList() {
    ASTAnyTypeList *TL = new ASTAnyTypeList();
    assert(TL && "Could not create a valid ASTAnyTypeList!");
    ATP = TL;
    ALV.push_back(ATP);
    return ATP;
  }

  static void Init() {
    if (!ATP) {
      ATP = NewList();
      assert(ATP && "Could not create a valid ASTAnyTypeList!");
      ALV.push_back(ATP);
    }
  }

  void Append(ASTAnyType &Node) {
    assert(ATP && "Invalid ASTAnyTypeList!");
    ATP->Graph.push_back(Node);
  }

  void Clear() {
    assert(ATP && "Invalid ASTAnyTypeList!");
    ATP->Graph.clear();
  }

  std::size_t Size() {
    assert(ATP && "Invalid ASTAnyTypeList!");
    return ATP->Graph.size();
  }

  iterator begin() { return ATP->Graph.begin(); }

  const_iterator begin() const { return ATP->Graph.begin(); }

  iterator end() { return ATP->Graph.end(); }

  const_iterator end() const { return ATP->Graph.end(); }
};

} // namespace QASM

#endif // __QASM_AST_ANY_TYPE_BUILDER_H
