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

#ifndef __QASM_AST_DECLARATION_BUILDER_H
#define __QASM_AST_DECLARATION_BUILDER_H

#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTDeclarationList.h>
#include <qasm/AST/ASTDeclarationMap.h>

#include <iostream>
#include <map>

namespace QASM {

class ASTResultNode;
class ASTReturnStatementNode;

class ASTDeclarationBuilder {
private:
  static ASTDeclarationList DL;
  static ASTDeclarationMap DM;
  static std::map<const ASTIdentifierNode *, const ASTDeclarationNode *> CDM;
  static ASTDeclarationBuilder DB;

protected:
  ASTDeclarationBuilder() {}

public:
  using list_type = std::vector<ASTDeclarationNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTDeclarationBuilder &Instance() { return ASTDeclarationBuilder::DB; }

  ~ASTDeclarationBuilder() = default;

  static ASTDeclarationList *List() { return &ASTDeclarationBuilder::DL; }

  static ASTDeclarationMap *Map() { return &ASTDeclarationBuilder::DM; }

  void Append(ASTDeclarationNode *DN) {
    assert(DN && "Invalid ASTDeclarationNode argument!");
    if (DN->GetIdentifier()->IsRedeclaration()) {
      DM.Insert(DN->GetIdentifier(), DN);
      DL.push(DN);
    } else if (!DM.Find(DN->GetIdentifier())) {
      DM.Insert(DN->GetIdentifier(), DN);
      DL.push(DN);
    }
  }

  bool ConstAppend(const ASTDeclarationNode *DN) {
    assert(DN && "Invalid ASTDeclarationNode argument!");
    return CDM.insert(std::make_pair(DN->GetIdentifier(), DN)).second;
  }

  bool IsConstDeclaration(const ASTDeclarationNode *DN) const {
    assert(DN && "Invalid ASTDeclarationNode argument!");
    return CDM.find(DN->GetIdentifier()) != CDM.end();
  }

  bool IsConstDeclaration(const ASTIdentifierNode *Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return CDM.find(Id) != CDM.end();
  }

  void CheckLoopInductionVariable(const ASTIdentifierNode *Id,
                                  const ASTStatementList *SL,
                                  const ASTDeclarationContext *DCX) const;

  void CheckLoopInductionVariable(const ASTIdentifierNode *Id,
                                  const ASTStatement *ST,
                                  const ASTDeclarationContext *DCX) const;

  bool ThisMayBeRedeclaration(const ASTIdentifierNode *Id,
                              const ASTDeclarationContext *DCX,
                              uint32_t *DCIX) const;

  bool DeclAlreadyExists(const ASTIdentifierNode *Id,
                         const ASTDeclarationContext *DCX = nullptr);

  bool TransferSymbol(const ASTIdentifierNode *Id,
                      std::map<std::string, const ASTSymbolTableEntry *> &MM);

  bool TransferResult(const ASTResultNode *RN,
                      std::map<std::string, const ASTSymbolTableEntry *> &MM);

  bool TransferReturn(const ASTReturnStatementNode *RN,
                      std::map<std::string, const ASTSymbolTableEntry *> &MM);

  std::vector<ASTDeclarationNode *> FindRange(const ASTIdentifierNode *Id) {
    return DM.FindRange(Id);
  }

  void Clear() {
    DL.List.clear();
    DM.HDM.clear();
  }

  size_t Size() { return DL.List.size(); }

  static ASTDeclarationNode *Root() { return DL.List.front(); }

  iterator begin() { return DL.List.begin(); }

  const_iterator begin() const { return DL.List.begin(); }

  iterator end() { return DL.List.end(); }

  const_iterator end() const { return DL.List.end(); }
};

} // namespace QASM

#endif // __QASM_AST_DECLARATION_BUILDER_H
