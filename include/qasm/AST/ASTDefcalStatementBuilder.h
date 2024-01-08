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

#ifndef __QASM_AST_DEFCAL_STATEMENT_BUILDER_H
#define __QASM_AST_DEFCAL_STATEMENT_BUILDER_H

#include <qasm/AST/ASTDefcal.h>
#include <qasm/AST/ASTStatement.h>

#include <cassert>
#include <vector>

namespace QASM {

class ASTDefcalStatementBuilder {
private:
  static ASTDefcalStatementBuilder DSB;
  static ASTStatementList DS;

protected:
  ASTDefcalStatementBuilder() = default;

public:
  using list_type = ASTStatementList;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTDefcalStatementBuilder &Instance() {
    return ASTDefcalStatementBuilder::DSB;
  }

  virtual ~ASTDefcalStatementBuilder() = default;

  ASTStatementList *List() const { return &ASTDefcalStatementBuilder::DS; }

  void Append(ASTStatementNode *SN) {
    assert(SN && "Invalid ASTStatementNode argument!");
    if (!SN->IsDirective())
      DS.push(SN);
  }

  void Clear() { DS.Clear(); }

  std::size_t Size() const { return DS.Size(); }

  iterator begin() { return DS.begin(); }

  const_iterator begin() const { return DS.begin(); }

  iterator end() { return DS.end(); }

  const_iterator end() const { return DS.end(); }

  ASTStatement *operator[](unsigned Index) {
    assert(Index < DS.Size() && "Index is out-of-range!");
    return DS[Index];
  }

  const ASTStatement *operator[](unsigned Index) const {
    assert(Index < DS.Size() && "Index is out-of-range!");
    return DS[Index];
  }

  virtual void print() const {
    std::cout << "<DefcalStatementList>" << std::endl;
    DS.print();
    std::cout << "</DefcalStatementList>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_DEFCAL_STATEMENT_BUILDER_H
