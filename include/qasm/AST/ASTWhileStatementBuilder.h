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

#ifndef __QASM_AST_WHILE_STATEMENT_BUILDER_H
#define __QASM_AST_WHILE_STATEMENT_BUILDER_H

#include <qasm/AST/ASTStatement.h>
#include <qasm/AST/ASTTypes.h>

#include <vector>
#include <cassert>

namespace QASM {

class ASTWhileStatementBuilder {
private:
  static ASTWhileStatementBuilder WSB;
  static ASTStatementList SL;
  static ASTStatementList* SLP;
  static std::vector<ASTStatementList*> SLV;

protected:
  ASTWhileStatementBuilder() = default;

public:
  using list_type = ASTStatementList;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTWhileStatementBuilder& Instance() {
    return ASTWhileStatementBuilder::WSB;
  }

  ~ASTWhileStatementBuilder() = default;

  static ASTStatementList* List() {
    return ASTWhileStatementBuilder::SLP;
  }

  static ASTStatementList* NewList() {
    ASTStatementList* SSL = new ASTStatementList();
    assert(SSL && "Could not create a valid ASTStatementList!");
    SLP = SSL;
    SLV.push_back(SLP);
    return SLP;
  }

  static void Init() {
    if (!SLP) {
      SLP = NewList();
      assert(SLP && "Could not create a valid ASTStatementList!");
      SLV.push_back(SLP);
    }
  }

  void Append(ASTStatementNode* SN) {
    assert(SN && "Invalid ASTStatementNode argument!");
    if (SN && !SN->IsDirective())
      SLP->Append(SN);
  }

  void Prepend(ASTStatementNode* SN) {
    assert(SN && "Invalid ASTStatementNode argument!");
    if (SN && !SN->IsDirective())
      SLP->Prepend(SN);
  }

  void Clear() {
    SLP->Clear();
  }

  std::size_t Size() const {
    return SLP->Size();
  }

  iterator begin() {
    return SLP->begin();
  }

  const_iterator begin() const {
    return SLP->begin();
  }

  iterator end() {
    return SLP->end();
  }

  const_iterator end() const {
    return SLP->end();
  }

  ASTStatement* operator[](unsigned Index) {
    assert(Index < SLP->Size() && "Index is out-of-range!");
    return SLP->operator[](Index);
  }

  const ASTStatement* operator[](unsigned Index) const {
    assert(Index < SLP->Size() && "Index is out-of-range!");
    return SLP->operator[](Index);
  }

  virtual void print() const {
    std::cout << "<WhileStatementBuilderList>" << std::endl;
    SLP->print();
    std::cout << "</WhileStatementBuilderList>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_WHILE_STATEMENT_BUILDER_H

