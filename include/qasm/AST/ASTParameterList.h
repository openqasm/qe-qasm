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

#ifndef __QASM_AST_PARAMETER_LIST_H
#define __QASM_AST_PARAMETER_LIST_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTDeclarationList.h>
#include <qasm/AST/ASTPrimitives.h>

#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace QASM {

class ASTExpressionNode;
class ASTSymbolTableEntry;

class ASTParameterList : public ASTBase {
  friend class ASTParameterBuilder;

private:
  std::vector<ASTBase *> Graph;
  const ASTDeclarationList *DL;

public:
  using list_type = std::vector<ASTBase *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTParameterList() : ASTBase(), Graph(), DL(nullptr) {}

  ASTParameterList(const ASTParameterList &RHS)
      : ASTBase(RHS), Graph(RHS.Graph), DL(RHS.DL) {}

  ASTParameterList(const ASTDeclarationList *DDL)
      : ASTBase(), Graph(), DL(nullptr) {
    *this = DDL;
  }

  ASTParameterList(const ASTDeclarationList &DDL)
      : ASTBase(), Graph(), DL(nullptr) {
    *this = DDL;
  }

  virtual ~ASTParameterList() = default;

  ASTParameterList &operator=(const ASTParameterList &RHS) {
    if (this != &RHS) {
      ASTBase::operator=(RHS);
      Graph = RHS.Graph;
      DL = RHS.DL;
    }

    return *this;
  }

  ASTParameterList &operator=(const ASTIdentifierList *IL);
  ASTParameterList &operator=(const ASTIdentifierList &IL);
  ASTParameterList &operator=(const ASTDeclarationList *DL);
  ASTParameterList &operator=(const ASTDeclarationList &DL);
  ASTParameterList &operator=(const std::vector<std::string> &VS);

  virtual std::size_t Size() const { return Graph.size(); }

  virtual void Clear() { Graph.clear(); }

  virtual bool Empty() const { return Graph.size() == 0; }

  virtual void Append(ASTBase *N) {
    if (ASTParameter *P = dynamic_cast<ASTParameter *>(N))
      Graph.push_back(P);
  }

  virtual bool HasDeclarationList() const { return DL != nullptr; }

  virtual const ASTDeclarationList *GetDeclarationList() const { return DL; }

  virtual void
  TransferSymbols(std::map<std::string, const ASTSymbolTableEntry *> &M);
  virtual void TransferGlobalSymbolsToLocal();
  virtual void TransferLocalSymbolsToGlobal();
  virtual void DeleteSymbols();
  virtual void SetLocalScope();

  virtual void Append(const ASTExpressionNode *EN);
  virtual void Append(const ASTDeclarationNode *DN);
  virtual void Append(const ASTDeclarationList *DL);
  virtual void Append(const ASTIdentifierList *IL);

  virtual void Erase(const std::string &Id);

  iterator begin() { return Graph.begin(); }
  const_iterator begin() const { return Graph.begin(); }

  iterator end() { return Graph.end(); }
  const_iterator end() const { return Graph.end(); }

  ASTBase *front() { return Graph.front(); }
  const ASTBase *front() const { return Graph.front(); }

  ASTBase *back() { return Graph.back(); }
  const ASTBase *back() const { return Graph.back(); }

  virtual ASTType GetASTType() const override { return ASTTypeParameterList; }

  inline virtual ASTBase *operator[](std::size_t Index) {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index];
  }

  inline virtual const ASTBase *operator[](std::size_t Index) const {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index];
  }

  virtual void print() const override {
    std::cout << "<ParameterList>" << std::endl;

    for (std::vector<ASTBase *>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I)
      (*I)->print();

    std::cout << "</ParameterList>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_PARAMETER_LIST_H
