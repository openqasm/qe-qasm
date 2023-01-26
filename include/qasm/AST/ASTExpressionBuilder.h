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

#ifndef __QASM_AST_EXPRESSION_BUILDER_H
#define __QASM_AST_EXPRESSION_BUILDER_H

#include <qasm/AST/ASTExpression.h>

#include <cassert>

namespace QASM {

class ASTExpressionBuilder {
  friend class ASTTypeSystemBuilder;

private:
  static ASTExpressionList* EL;
  static ASTExpressionBuilder B;

protected:
  ASTExpressionBuilder() = default;

  static void Init() {
    if (!EL) {
      EL = new ASTExpressionList();
      assert(EL && "Could not create a valid ASTExpressionList!");
    }
  }

public:
  using list_type = std::vector<ASTExpression*>;
  using iterator = typename std::vector<ASTExpression*>::iterator;
  using const_iterator = typename std::vector<ASTExpression*>::const_iterator;

public:
  static ASTExpressionBuilder& Instance() {
    return ASTExpressionBuilder::B;
  }

  ~ASTExpressionBuilder() = default;

  static ASTExpressionList* NewList() {
    return EL = new ASTExpressionList();
  }

  void Append(ASTExpression* EX) {
    EL->push(EX);
  }

  void Clear() { }

  size_t Size() {
    return EL->List.size();
  }

  static ASTExpression* Root() {
    return EL->List.front();
  }

  iterator begin() { return EL->List.begin(); }

  const_iterator begin() const { return EL->List.begin(); }

  iterator end() { return EL->List.end(); }

  const_iterator end() const { return EL->List.end(); }
};

} // namespace QASM

#endif // __QASM_AST_EXPRESSION_BUILDER_H

