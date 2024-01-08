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

#ifndef __QASM_AST_DECLARATION_LIST_H
#define __QASM_AST_DECLARATION_LIST_H

#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace QASM {

class ASTSymbolTableEntry;

class ASTDeclarationList : public ASTBase {
  friend class ASTDeclarationBuilder;
  friend class ASTNamedTypeDeclarationBuilder;

private:
  mutable std::vector<ASTDeclarationNode *> List;

public:
  using list_type = std::vector<ASTDeclarationNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTDeclarationList() = default;

  virtual ~ASTDeclarationList() = default;

  virtual std::size_t Size() const { return List.size(); }

  virtual void Clear() { List.clear(); }

  virtual bool Empty() const { return List.empty(); }

  virtual void Erase(const_iterator &DI) const {
    if (DI != List.end())
      List.erase(DI);
  }

  virtual void Append(ASTDeclarationNode *DN) { this->push(DN); }

  // Implemented in ASTDeclarationBuilder.cpp.
  virtual void SetLocalScope();

  // Implemented in ASTDeclarationBuilder.cpp.
  virtual void SetDeclarationContext(const ASTDeclarationContext *CX);

  // Implemented in ASTDeclarationBuilder.cpp.
  virtual bool ExportQuantumParameters(ASTDeclarationList &QL);

  // Implemented in ASTDeclarationBuilder.cpp.
  virtual bool
  TransferSymbols(std::map<std::string, const ASTSymbolTableEntry *> &M) const;

  // Implemented in ASTDeclarationBuilder.cpp.
  virtual void CreateBuiltinParameterSymbols(const ASTIdentifierNode *FId);

  iterator begin() { return List.begin(); }

  const_iterator begin() const { return List.begin(); }

  iterator end() { return List.end(); }

  const_iterator end() const { return List.end(); }

  ASTDeclarationNode *front() { return List.front(); }

  const ASTDeclarationNode *front() const { return List.front(); }

  ASTDeclarationNode *back() { return List.back(); }

  const ASTDeclarationNode *back() const { return List.back(); }

  virtual ASTType GetASTType() const { return ASTTypeDeclarationList; }

  inline virtual ASTDeclarationNode *operator[](std::size_t Index) {
    assert(Index < List.size() && "Index is out-of-range!");
    return List[Index];
  }

  inline virtual const ASTDeclarationNode *operator[](std::size_t Index) const {
    assert(Index < List.size() && "Index is out-of-range!");
    return List[Index];
  }

  virtual void print() const {
    std::cout << "<DeclarationList>" << std::endl;

    for (std::vector<ASTDeclarationNode *>::const_iterator I = List.begin();
         I != List.end(); ++I)
      (*I)->print();

    std::cout << "</DeclarationList>" << std::endl;
  }

  virtual void push(ASTBase *Node) {
    if (ASTDeclarationNode *DN = dynamic_cast<ASTDeclarationNode *>(Node))
      List.push_back(DN);
  }
};

} // namespace QASM

#endif // __QASM_AST_DECLARATION_LIST_H
