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

#ifndef __QASM_AST_FUNCTION_STATEMENT_BUILDER_H
#define __QASM_AST_FUNCTION_STATEMENT_BUILDER_H

#include <qasm/AST/ASTStatement.h>
#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <vector>

namespace QASM {

class ASTFunctionStatementBuilder {
private:
  static ASTFunctionStatementBuilder FSB;
  ASTStatementList VS;

protected:
  ASTFunctionStatementBuilder() : VS() {}

public:
  using list_type = ASTStatementList;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTFunctionStatementBuilder &Instance() {
    return ASTFunctionStatementBuilder::FSB;
  }

  virtual ~ASTFunctionStatementBuilder() = default;

  ASTStatementList *List() { return &VS; }

  void Append(ASTStatementNode *SN) {
    assert(SN && "Invalid ASTStatementNode argument!");
    if (SN && !SN->IsDirective())
      VS.push(SN);
  }

  void Clear() { VS.Clear(); }

  std::size_t Size() const { return VS.Size(); }

  iterator begin() { return VS.begin(); }

  const_iterator begin() const { return VS.begin(); }

  iterator end() { return VS.end(); }

  const_iterator end() const { return VS.end(); }

  ASTStatement *operator[](unsigned Index) {
    assert(Index < VS.Size() && "Index is out-of-range!");
    return VS[Index];
  }

  const ASTStatement *operator[](unsigned Index) const {
    assert(Index < VS.Size() && "Index is out-of-range!");
    return VS[Index];
  }

  virtual void print() const {
    std::cout << "<ASTFunctionStatementBuilderList>" << std::endl;
    VS.print();
    std::cout << "</ASTFunctionStatementBuilderList>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_FUNCTION_STATEMENT_BUILDER_H
