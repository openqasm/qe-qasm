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

#ifndef __QASM_AST_DEFCAL_PARAMETER_BUILDER_H
#define __QASM_AST_DEFCAL_PARAMETER_BUILDER_H

#include <qasm/AST/ASTExpressionNodeList.h>
#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <vector>

namespace QASM {

class ASTDefcalParameterBuilder {
private:
  static ASTExpressionNodeList EV;
  static ASTDefcalParameterBuilder DPB;

protected:
  ASTDefcalParameterBuilder() = default;

public:
  using list_type = std::vector<ASTExpressionNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ~ASTDefcalParameterBuilder() = default;

  static ASTDefcalParameterBuilder &Instance() {
    return ASTDefcalParameterBuilder::DPB;
  }

  static ASTExpressionNodeList *List() {
    return &ASTDefcalParameterBuilder::EV;
  }

  void Append(ASTExpressionNode *EN) {
    assert(EN && "Invalid ASTExpressionNode argument!");
    EV.Append(EN);
  }

  void Clear() { EV.Clear(); }

  size_t Size() const { return EV.Size(); }

  void print() const {
    std::cout << "<DefcalParameterList>" << std::endl;
    EV.print();
    std::cout << "</DefcalParameterList>" << std::endl;
  }

  iterator begin() { return EV.begin(); }

  const_iterator begin() const { return EV.begin(); }

  iterator end() { return EV.end(); }

  const_iterator end() const { return EV.end(); }
};

} // namespace QASM

#endif // __QASM_AST_DEFCAL_PARAMETER_BUILDER_H
