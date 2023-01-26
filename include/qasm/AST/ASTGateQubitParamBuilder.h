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

#ifndef __QASM_AST_GATE_QUBIT_PARAM_BUILDER_H
#define __QASM_AST_GATE_QUBIT_PARAM_BUILDER_H

#include <qasm/AST/ASTIdentifier.h>

namespace QASM {

class ASTGateQubitParamBuilder {
private:
  static ASTIdentifierList IL;
  static ASTGateQubitParamBuilder GQB;
  static ASTIdentifierList* ILP;
  static std::vector<ASTIdentifierList*> ILV;

protected:
  ASTGateQubitParamBuilder() { }

public:
  using list_type = std::vector<ASTIdentifierNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTGateQubitParamBuilder& Instance() {
    return ASTGateQubitParamBuilder::GQB;
  }

  virtual ~ASTGateQubitParamBuilder() = default;

  ASTIdentifierList* List() {
    return ASTGateQubitParamBuilder::ILP;
  }

  static ASTIdentifierList* NewList() {
    ASTIdentifierList* IIL = new ASTIdentifierList();
    assert(IIL && "Could not create a valid ASTIdentifierList!");
    ILP = IIL;
    ILV.push_back(ILP);
    return ILP;
  }

  static void Init() {
    if (!ILP) {
      ILP = NewList();
      assert(ILP && "Could not create a valid ASTIdentifierList!");
      ILV.push_back(ILP);
    }
  }

  void Append(ASTIdentifierNode* I) {
    ILP->Append(I);
  }

  void Clear() {
    ILP->Clear();
  }

  unsigned Size() {
    return static_cast<unsigned>(ILP->Size());
  }

  static ASTIdentifierNode* Root() {
    return ILP->Graph.front();
  }

  void SetGateLocal();

  void ReleaseQubits();

  iterator begin() { return ILP->Graph.begin(); }

  const_iterator begin() const { return ILP->Graph.begin(); }

  iterator end() { return ILP->Graph.end(); }

  const_iterator end() const { return ILP->Graph.end(); }
};

} // namespace QASM

#endif // __QASM_AST_GATE_QUBIT_PARAM_BUILDER_H

