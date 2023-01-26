/* -*- coding: utf-8 -*-
 *
 * Copyright 2020 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_FUNCTION_DEFINITION_BUILDER_H
#define __QASM_AST_FUNCTION_DEFINITION_BUILDER_H

#include <qasm/AST/ASTFunctions.h>

#include <map>
#include <string>

namespace QASM {

class ASTFunctionDefinitionBuilder {
private:
  static std::map<std::string, ASTFunctionDefinitionNode*> FM;
  static ASTFunctionDefinitionBuilder FDB;

protected:
  ASTFunctionDefinitionBuilder() = default;

public:
  using map_type = std::map<std::string, ASTFunctionDefinitionNode*>;
  using iterator = typename map_type::iterator;
  using const_iterator = typename map_type::const_iterator;

public:
  static ASTFunctionDefinitionBuilder& Instance() {
    return ASTFunctionDefinitionBuilder::FDB;
  }

  bool Insert(const std::string& S, ASTFunctionDefinitionNode* FN) {
    return FM.insert(std::make_pair(S, FN)).second;
  }

  ASTFunctionDefinitionNode* Lookup(const std::string& S) {
    if (S.empty())
      return nullptr;

    ASTFunctionDefinitionBuilder::iterator I = FM.find(S);
    return I == FM.end() ? nullptr : (*I).second;
  }

  const ASTFunctionDefinitionNode* Lookup(const std::string& S) const {
    if (S.empty())
      return nullptr;

    ASTFunctionDefinitionBuilder::const_iterator I = FM.find(S);
    return I == FM.end() ? nullptr : (*I).second;
  }

  iterator begin() {
    return FM.begin();
  }

  const_iterator begin() const {
    return FM.begin();
  }

  iterator end() {
    return FM.end();
  }

  const_iterator end() const {
    return FM.end();
  }

  ASTFunctionDefinitionNode* operator[](const std::string& S) {
    return Lookup(S);
  }

  const ASTFunctionDefinitionNode* operator[](const std::string& S) const {
    return Lookup(S);
  }
};

} // namespace QASM

#endif // __QASM_AST_FUNCTION_DEFINITION_BUILDER_H

