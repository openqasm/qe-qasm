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

#ifndef __QASM_AST_KERNEL_BUILDER_H
#define __QASM_AST_KERNEL_BUILDER_H

#include <qasm/AST/ASTKernel.h>

#include <map>
#include <string>

namespace QASM {

class ASTKernelBuilder {
private:
  static std::map<std::string, ASTKernelNode*> KM;
  static ASTKernelBuilder KB;

protected:
  ASTKernelBuilder() = default;

public:
  using map_type = std::map<std::string, ASTKernelNode*>;
  using iterator = typename map_type::iterator;
  using const_iterator = typename map_type::const_iterator;

public:
  static ASTKernelBuilder& Instance() {
    return ASTKernelBuilder::KB;
  }

  bool Insert(const std::string& S, ASTKernelNode* FN) {
    return KM.insert(std::make_pair(S, FN)).second;
  }

  ASTKernelNode* Lookup(const std::string& S) {
    if (S.empty())
      return nullptr;

    ASTKernelBuilder::iterator I = KM.find(S);
    return I == KM.end() ? nullptr : (*I).second;
  }

  const ASTKernelNode* Lookup(const std::string& S) const {
    if (S.empty())
      return nullptr;

    ASTKernelBuilder::const_iterator I = KM.find(S);
    return I == KM.end() ? nullptr : (*I).second;
  }

  iterator begin() {
    return KM.begin();
  }

  const_iterator begin() const {
    return KM.begin();
  }

  iterator end() {
    return KM.end();
  }

  const_iterator end() const {
    return KM.end();
  }

  ASTKernelNode* operator[](const std::string& S) {
    return Lookup(S);
  }

  const ASTKernelNode* operator[](const std::string& S) const {
    return Lookup(S);
  }
};

} // namespace QASM

#endif // __QASM_AST_KERNEL_BUILDER_H

