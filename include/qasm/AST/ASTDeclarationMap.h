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

#ifndef __QASM_AST_DECLARATION_MAP_H
#define __QASM_AST_DECLARATION_MAP_H

#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <functional>
#include <map>

namespace QASM {

class ASTDeclarationMap {
  friend class ASTDeclarationBuilder;
  friend class ASTNamedTypeDeclarationBuilder;

private:
  std::multimap<std::size_t, ASTDeclarationNode *> HDM;

public:
  using map_type = std::multimap<std::size_t, ASTDeclarationNode *>;
  using iterator = typename map_type::iterator;
  using const_iterator = typename map_type::const_iterator;

public:
  ASTDeclarationMap() = default;

  virtual ~ASTDeclarationMap() = default;

  bool Insert(const ASTIdentifierNode *Id, ASTDeclarationNode *DN) {
    map_type::iterator I = HDM.find(Id->GetHash());

    if (I == HDM.end())
      HDM.insert(std::make_pair(Id->GetHash(), DN));
    else
      HDM.insert(I, std::make_pair(Id->GetHash(), DN));

    return true;
  }

  const ASTDeclarationNode *Find(const ASTIdentifierNode *Id) const {
    map_type::const_iterator I = HDM.find(Id->GetHash());
    return I == HDM.end() ? nullptr : (*I).second;
  }

  ASTDeclarationNode *Find(const ASTIdentifierNode *Id) {
    map_type::iterator I = HDM.find(Id->GetHash());
    return I == HDM.end() ? nullptr : (*I).second;
  }

  std::vector<const ASTDeclarationNode *>
  FindRange(const ASTIdentifierNode *Id) const {
    std::pair<const_iterator, const_iterator> P =
        HDM.equal_range(Id->GetHash());
    std::vector<const ASTDeclarationNode *> V;

    for (const_iterator I = P.first; I != P.second; ++I)
      V.push_back((*I).second);

    return V;
  }

  std::vector<ASTDeclarationNode *> FindRange(const ASTIdentifierNode *Id) {
    std::pair<iterator, iterator> P = HDM.equal_range(Id->GetHash());
    std::vector<ASTDeclarationNode *> V;

    for (iterator I = P.first; I != P.second; ++I)
      V.push_back((*I).second);

    return V;
  }

  void Erase(const ASTIdentifierNode *Id) {
    iterator I = HDM.find(Id->GetHash());

    if (I != HDM.end())
      HDM.erase(I);
  }

  void Clear() { HDM.clear(); }

  iterator begin() { return HDM.begin(); }

  const_iterator begin() const { return HDM.begin(); }

  iterator end() { return HDM.end(); }

  const_iterator end() const { return HDM.end(); }
};

} // namespace QASM

#endif // __QASM_AST_DECLARATION_MAP_H
