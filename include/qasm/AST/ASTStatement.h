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

#ifndef __QASM_AST_STATEMENT_H
#define __QASM_AST_STATEMENT_H

#include <qasm/AST/ASTBase.h>

#include <map>
#include <vector>
#include <string>
#include <cassert>

namespace QASM {

class ASTDeclarationContext;
class ASTSymbolTableEntry;

class ASTStatement : public ASTBase {
  friend class ASTStatementBuilder;

public:
  ASTStatement() : ASTBase() { }

  ASTStatement(const ASTStatement& RHS)
  : ASTBase(RHS) { }

  ASTStatement& operator=(const ASTStatement& RHS) {
    if (this != &RHS)
      ASTBase::operator=(RHS);

    return *this;
  }

  virtual ~ASTStatement() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeStatement;
  }

  virtual bool Skip() const {
    return false;
  }

  virtual void print() const override { }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTStatementList : public ASTStatement {
  friend class ASTStatementBuilder;

protected:
  std::vector<ASTStatement*> List;
  unsigned ISC;

public:
  using list_type = std::vector<ASTStatement*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;
  using reference = typename list_type::reference;
  using const_reference = typename list_type::const_reference;

public:
  ASTStatementList() : ASTStatement(),
  List(), ISC(static_cast<unsigned>(~0x0)) { }

  ASTStatementList(unsigned MI)
  : ASTStatement(), List(), ISC(MI) { }

  ASTStatementList(const ASTStatementList& RHS)
  : ASTStatement(RHS), List(RHS.List), ISC(RHS.ISC) { }

  virtual ~ASTStatementList() {
    List.clear();
  }

  ASTStatementList& operator=(const ASTStatementList& RHS) {
    if (this != &RHS) {
      ASTStatement::operator=(RHS);
      List = RHS.List;
      ISC = RHS.ISC;
    }

    return *this;
  }

  virtual size_t Size() const {
    return List.size();
  }

  iterator begin() { return List.begin(); }

  const_iterator begin() const { return List.begin(); }

  iterator end() { return List.end(); }

  const_iterator end() const { return List.end(); }

  virtual ASTType GetASTType() const override {
    return ASTTypeStatementList;
  }

  reference back() { return List.back(); }

  const_reference back() const { return List.back(); }

  reference front() { return List.front(); }

  const_reference front() const { return List.front(); }

  inline ASTStatement* operator[](size_t Index) {
    assert(Index < List.size() && "Index is out-of-range!");
    return List[Index];
  }

  inline const ASTStatement* operator[](size_t Index) const {
    assert(Index < List.size() && "Index is out-of-range!");
    return List[Index];
  }

  virtual bool Empty() const {
    return List.empty();
  }

  virtual unsigned GetMapIndex() const {
    return ISC;
  }

  virtual void Clear() {
    List.clear();
  }

  virtual void Append(ASTStatement* S) {
    assert(S && "Invalid ASTStatement argument!");
    if (!S->Skip())
      List.push_back(S);
  }

  virtual void Append(const ASTStatementList& SL) {
    if (!SL.Empty()) {
      for (ASTStatementList::const_iterator LI = SL.begin();
           LI != SL.end(); ++LI) {
        Append(*LI);
      }
    }
  }

  virtual void Prepend(ASTStatement* S) {
    if (S && !S->Skip())
      List.insert(List.begin(), S);
  }

  virtual void Prepend(const ASTStatementList& SL) {
    if (!SL.Empty()) {
      for (ASTStatementList::const_iterator LI = SL.begin();
           LI != SL.end(); ++LI) {
        Prepend(*LI);
      }
    }
  }

  // Implemented in ASTStatementBuilder.cpp.
  virtual void SetLocalScope();

  // Implemented in ASTStatementBuilder.cpp.
  virtual void SetDeclarationContext(const ASTDeclarationContext* DCX);

  // Implemented in ASTStatementBuilder.cpp.
  virtual bool TransferDeclarations(std::map<std::string,
                                    const ASTSymbolTableEntry*>& MM) const;

  // Implemented in ASTStatementBuilder.cpp.
  virtual bool TransferStatements(std::map<std::string,
                                  const ASTSymbolTableEntry*>& MM) const;


  virtual void print() const override {
    std::cout << "<StatementList>" << std::endl;

    for (ASTStatementList::const_iterator I = List.begin();
         I != List.end(); ++I) {
      if (ASTStatement* ST = dynamic_cast<ASTStatement*>(*I))
        ST->print();
    }

    std::cout << "</StatementList>" << std::endl;
  }

  virtual void push(ASTBase* Node) override {
    if (ASTStatement* ST = dynamic_cast<ASTStatement*>(Node)) {
      if (!ST->Skip())
        List.push_back(ST);
    }
  }
};

} // namespace QASM

#endif // __QASM_AST_STATEMENT_H

