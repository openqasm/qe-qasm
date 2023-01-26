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

#ifndef __QASM_AST_STATEMENT_BUILDER_H
#define __QASM_AST_STATEMENT_BUILDER_H

#include <qasm/AST/ASTStatement.h>

#include <iostream>
#include <map>

namespace QASM {

class ASTSymbolTableEntry;
class ASTStatementNode;

class ASTStatementBuilder {
private:
  static ASTStatementList SL;
  static ASTStatementBuilder B;
  static std::map<uintptr_t, const ASTStatement*> SM;

protected:
  ASTStatementBuilder() { }

public:
  using list_type = std::vector<ASTStatement*>;
  using iterator = typename std::vector<ASTStatement*>::iterator;
  using const_iterator = typename std::vector<ASTStatement*>::const_iterator;

public:
  static ASTStatementBuilder& Instance() {
    return ASTStatementBuilder::B;
  }

  ~ASTStatementBuilder() = default;

  static ASTStatementList* List() {
    return &ASTStatementBuilder::SL;
  }

  void Append(ASTStatement* SN) {
    if (SN) {
      uintptr_t A = reinterpret_cast<uintptr_t>(SN);
      std::map<uintptr_t, const ASTStatement*>::const_iterator I = SM.find(A);
      if (I == SM.end()) {
        if (!SM.insert(std::make_pair(A, SN)).second) {
          // FIXME: Use Diagnostic subsystem.
          std::cerr << "Error: ASTStatementNode map insertion failed!"
            << std::endl;
          return;
        }

        SL.Append(SN);
      }
    }
  }

  void Prepend(ASTStatement* SN) {
    if (SN) {
      uintptr_t A = reinterpret_cast<uintptr_t>(SN);
      std::map<uintptr_t, const ASTStatement*>::const_iterator I = SM.find(A);
      if (I == SM.end()) {
        if (!SM.insert(std::make_pair(A, SN)).second) {
          // FIXME: Use Diagnostic subsystem.
          std::cerr << "Error: ASTStatementNode map insertion failed!"
            << std::endl;
          return;
        }

        SL.Prepend(SN);
      }
    }
  }

  static ASTStatement* Root() {
    return dynamic_cast<ASTStatement*>(SL.List.front());
  }

  void Clear() {
    SL.List.clear();
  }

  size_t Size() {
    return SL.List.size();
  }

  bool TransferStatement(ASTStatementNode* SN,
                         std::map<std::string, const ASTSymbolTableEntry*>& MM) const;

  iterator begin() { return SL.List.begin(); }

  const_iterator begin() const { return SL.List.begin(); }

  iterator end() { return SL.List.end(); }

  const_iterator end() const { return SL.List.end(); }
};

} // namespace QASM

#endif // __QASM_AST_STATEMENT_BUILDER_H

