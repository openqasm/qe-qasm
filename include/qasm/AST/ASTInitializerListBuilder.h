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

#ifndef __QASM_AST_INITIALIZER_LIST_BUILDER_H
#define __QASM_AST_INITIALIZER_LIST_BUILDER_H

#include <qasm/AST/ASTInitializerNode.h>
#include <qasm/AST/ASTTypes.h>

namespace QASM {

class ASTInitializerListBuilder {
private:
  static ASTInitializerList IL;
  static ASTInitializerListBuilder B;
  static ASTInitializerList *ILP;
  static std::vector<ASTInitializerList *> ILV;

protected:
  ASTInitializerListBuilder() = default;

public:
  static ASTInitializerListBuilder &Instance() {
    return ASTInitializerListBuilder::B;
  }

  ~ASTInitializerListBuilder() = default;

  static ASTInitializerList *List() { return ASTInitializerListBuilder::ILP; }

  static ASTInitializerList *NewList() {
    ASTInitializerList *IIL = new ASTInitializerList();
    assert(IIL && "Could not create a valid ASTInitializerList!");
    ILP = IIL;
    ILV.push_back(ILP);
    return ILP;
  }

  static void Init() {
    if (!ILP) {
      ILP = NewList();
      assert(ILP && "Could not create a valid ASTInitializerList!");
      ILV.push_back(ILP);
    }
  }

  void Clear() { ILP->Clear(); }

  std::size_t Size() { return ILP->Size(); }
};

} // namespace QASM

#endif // __QASM_AST_INITIALIZER_LIST_BUILDER_H
