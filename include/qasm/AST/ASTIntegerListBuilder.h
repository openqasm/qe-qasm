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
 * distributed under the License is distributed on an "AS IL" BASIL,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * =============================================================================
 */

#ifndef __QASM_AST_INTEGER_LIST_BUILDER_H
#define __QASM_AST_INTEGER_LIST_BUILDER_H

#include <qasm/AST/ASTIntegerList.h>
#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <vector>

namespace QASM {

class ASTIntegerListBuilder {
private:
  static ASTIntegerListBuilder ILB;
  static ASTIntegerList IL;
  static ASTIntegerList *ILP;
  static std::vector<ASTIntegerList *> ILV;

protected:
  ASTIntegerListBuilder() = default;

public:
  using list_type = ASTIntegerList;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTIntegerListBuilder &Instance() {
    return ASTIntegerListBuilder::ILB;
  }

  virtual ~ASTIntegerListBuilder() = default;

  ASTIntegerList *List() const { return ASTIntegerListBuilder::ILP; }

  static ASTIntegerList *NewList() {
    ASTIntegerList *IIL = new ASTIntegerList();
    assert(IIL && "Could not create a valid ASTIntegerList!");
    ILP = IIL;
    ILV.push_back(ILP);
    return ILP;
  }

  static void Init() {
    if (!ILP) {
      ILP = NewList();
      assert(ILP && "Could not create a valid ASTIntegerList!");
      ILV.push_back(ILP);
    }
  }

  void Append(ASTIntNode *IN) {
    assert(IN && "Invalid ASTIntNode argument!");
    int32_t I = IN->IsSigned() ? IN->GetSignedValue()
                               : static_cast<int32_t>(IN->GetUnsignedValue());
    IL.Append(I);
  }

  void Append(int32_t V) { IL.Append(V); }

  void Prepend(ASTIntNode *IN) {
    assert(IN && "Invalid ASTIntNode argument!");
    int32_t I = IN->IsSigned() ? IN->GetSignedValue()
                               : static_cast<int32_t>(IN->GetUnsignedValue());
    IL.Prepend(I);
  }

  void Prepend(int32_t V) { IL.Prepend(V); }

  void Clear() { ILP->Clear(); }

  void SetSeparator(char C) { ILP->SetSeparator(C); }

  std::size_t Size() const { return ILP->Size(); }

  bool Empty() const { return ILP->Size() == 0; }

  iterator begin() { return ILP->List.begin(); }

  const_iterator begin() const { return ILP->List.begin(); }

  iterator end() { return ILP->List.end(); }

  const_iterator end() const { return ILP->List.end(); }

  virtual void print() const {
    std::cout << "<IntegerList>" << std::endl;
    ILP->print();
    std::cout << "</IntegerList>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_INTEGER_LIST_BUILDER_H
