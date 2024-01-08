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

#ifndef __QASM_AST_ANGLE_NODE_BUILDER_H
#define __QASM_AST_ANGLE_NODE_BUILDER_H

#include <qasm/AST/ASTAngleNodeList.h>
#include <qasm/AST/ASTTypes.h>

namespace QASM {

class ASTAngleNodeBuilder {
private:
  static ASTAngleNodeList AL;
  static ASTAngleNodeMap AM;
  static ASTAngleNodeBuilder B;

protected:
  ASTAngleNodeBuilder() {}

public:
  using list_type = std::vector<ASTAngleNode *>;
  using list_iterator = typename list_type::iterator;
  using const_list_iterator = typename list_type::const_iterator;

  using map_type = typename std::map<ASTAngleKey, ASTAngleNode *>;
  using map_iterator = typename map_type::iterator;
  using const_map_iterator = typename map_type::const_iterator;

public:
  static ASTAngleNodeBuilder &Instance() { return ASTAngleNodeBuilder::B; }

  ~ASTAngleNodeBuilder() = default;

  static ASTAngleNodeList *List() { return &ASTAngleNodeBuilder::AL; }

  static ASTAngleNodeMap *Map() { return &ASTAngleNodeBuilder::AM; }

  void Append(ASTAngleNode *Node) {
    assert(Node && "Invalid ASTAngleNode!");
    AL.push(Node);
  }

  void Insert(ASTAngleNode *Node) {
    assert(Node && "Invalid ASTAngleNode!");
    ASTAngleNodeMap::Instance().Insert(Node->GetName(), Node);
  }

  ASTAngleNode *FindInMap(const std::string &Id, unsigned Bits) {
    return ASTAngleNodeMap::Instance().Find(Id, Bits);
  }

  void Clear() { AL.Clear(); }

  std::size_t Size() { return AL.Size(); }

  static ASTAngleNode *Root() { return AL.front(); }

  list_iterator list_begin() { return AL.begin(); }

  const_list_iterator list_begin() const { return AL.begin(); }

  list_iterator list_end() { return AL.end(); }

  const_list_iterator list_end() const { return AL.end(); }

  map_iterator map_begin() { return AM.begin(); }

  const_map_iterator map_begin() const { return AM.begin(); }

  map_iterator map_end() { return AM.end(); }

  const_map_iterator map_end() const { return AM.end(); }
};

} // namespace QASM

#endif // __QASM_AST_ANGLE_NODE_BUILDER_H
