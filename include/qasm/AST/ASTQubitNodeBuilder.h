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

#ifndef __QASM_AST_QUBIT_NODE_BUILDER_H
#define __QASM_AST_QUBIT_NODE_BUILDER_H

#include <qasm/AST/ASTQubitNodeMap.h>

namespace QASM {

class ASTQubitNodeBuilder {
private:
  static ASTQubitNodeMap QM;
  static ASTQubitNodeBuilder B;

protected:
  ASTQubitNodeBuilder() {}

public:
  using map_type = std::map<std::string, ASTQubitNode *>;
  using iterator = typename map_type::iterator;
  using const_iterator = typename map_type::const_iterator;

public:
  static ASTQubitNodeBuilder &Instance() { return ASTQubitNodeBuilder::B; }

  ~ASTQubitNodeBuilder() = default;

  static ASTQubitNodeMap *Map() { return &ASTQubitNodeBuilder::QM; }

  void Append(ASTQubitNode *Node) { QM.push(Node); }

  void Clear() { QM.Clear(); }

  size_t Size() { return QM.Size(); }

  ASTQubitNode *Find(const std::string &K) { return QM[K]; }

  const ASTQubitNode *Find(const std::string &K) const { return QM[K]; }

  iterator begin() { return QM.begin(); }

  const_iterator begin() const { return QM.begin(); }

  iterator end() { return QM.end(); }

  const_iterator end() const { return QM.end(); }
};

} // namespace QASM

#endif // __QASM_AST_QUBIT_NODE_BUILDER_H
