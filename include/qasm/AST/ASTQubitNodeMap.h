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

#ifndef __QASM_AST_QUBIT_NODE_LIST_H
#define __QASM_AST_QUBIT_NODE_LIST_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTQubit.h>

#include <map>
#include <string>
#include <cassert>

namespace QASM {

class ASTQubitNodeMap : public ASTBase {
  friend class ASTQubitNodeBuilder;

private:
  std::map<std::string, ASTQubitNode*> Map;

public:
  using map_type = std::map<std::string, ASTQubitNode*>;
  using iterator = typename map_type::iterator;
  using const_iterator = typename map_type::const_iterator;

public:
  ASTQubitNodeMap() : ASTBase(), Map() { }

  ASTQubitNodeMap(const ASTQubitNodeMap& RHS)
  : ASTBase(RHS), Map(RHS.Map) { }

  virtual ~ASTQubitNodeMap() = default;

  ASTQubitNodeMap& operator=(const ASTQubitNodeMap& RHS) {
    if (this != &RHS) {
      ASTBase::operator=(RHS);
      Map = RHS.Map;
    }

    return *this;
  }

  virtual size_t Size() const {
    return Map.size();
  }

  virtual void Clear() {
    Map.clear();
  }

  iterator begin() { return Map.begin(); }
  const_iterator begin() const { return Map.begin(); }

  iterator end() { return Map.end(); }
  const_iterator end() const { return Map.end(); }

  virtual ASTType GetASTType() const override {
    return ASTTypeQubitMap;
  }

  virtual ASTQubitNode* operator[](const std::string& Index) {
    std::map<std::string, ASTQubitNode*>::iterator I = Map.find(Index);
    return I == Map.end() ? nullptr : (*I).second;
  }

  virtual const ASTQubitNode* operator[](const std::string& Index) const {
    std::map<std::string, ASTQubitNode*>::const_iterator I = Map.find(Index);
    return I == Map.end() ? nullptr : (*I).second;
  }

  virtual void print() const override {
    std::cout << "<QubitNodeMap>" << std::endl;

    for (std::map<std::string, ASTQubitNode*>::const_iterator I = Map.begin();
         I != Map.end(); ++I)
      (*I).second->print();

    std::cout << "</QubitNodeMap>" << std::endl;
  }

  virtual void push(ASTBase* Node) override {
    if (ASTQubitNode* QN = dynamic_cast<ASTQubitNode*>(Node)) {
      if (const ASTIdentifierNode* IDN = QN->GetIdentifier()) {
        const std::string& Id = IDN->GetName();
        Map.insert(std::make_pair(Id, QN));
      }
    }
  }
};

}

#endif // __QASM_AST_QUBIT_NODE_LIST_H

