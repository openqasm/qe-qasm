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

#ifndef __QASM_AST_NAMED_TYPE_DECLARATION_BUILDER_H
#define __QASM_AST_NAMED_TYPE_DECLARATION_BUILDER_H

#include <qasm/AST/ASTDeclarationList.h>
#include <qasm/AST/ASTDeclarationMap.h>

#include <iostream>

namespace QASM {

class ASTNamedTypeDeclarationBuilder {
private:
  static ASTDeclarationList DL;
  static ASTDeclarationMap DM;
  static ASTNamedTypeDeclarationBuilder DB;

protected:
  ASTNamedTypeDeclarationBuilder() { }

public:
  using list_type = std::vector<ASTDeclarationNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTNamedTypeDeclarationBuilder& Instance() {
    return ASTNamedTypeDeclarationBuilder::DB;
  }

  ~ASTNamedTypeDeclarationBuilder() = default;

  static ASTDeclarationList* List() {
    return new ASTDeclarationList();
  }

  static ASTDeclarationMap* Map() {
    return &ASTNamedTypeDeclarationBuilder::DM;
  }

  void Append(ASTDeclarationNode* Node) {
    if (Node->DuplicatesAllowed()) {
      DM.Insert(Node->GetIdentifier(), Node);
      DL.push(Node);
    } else {
      if (!DM.Find(Node->GetIdentifier())) {
        DM.Insert(Node->GetIdentifier(), Node);
        DL.push(Node);
      }
    }
  }

  bool DeclAlreadyExists(const ASTIdentifierNode* Id) {
    return DM.Find(Id) != nullptr;
  }

  void Clear() {
    DL.Clear();
    DM.Clear();
  }

  size_t Size() {
    return DL.List.size();
  }

  static ASTDeclarationNode* Root() {
    return DL.List.front();
  }

  iterator begin() { return DL.List.begin(); }

  const_iterator begin() const { return DL.List.begin(); }

  iterator end() { return DL.List.end(); }

  const_iterator end() const { return DL.List.end(); }
};

} // namespace QASM

#endif // __QASM_AST_NAMED_TYPE_DECLARATION_BUILDER_H


