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

#ifndef __QASM_AST_GATE_NODE_LIST_H
#define __QASM_AST_GATE_NODE_LIST_H

#include "qasm/AST/ASTBase.h"
#include "qasm/AST/ASTGates.h"

#include <vector>
#include <set>
#include <string>
#include <cassert>

namespace QASM {

class ASTGateNodeList {
  friend class ASTGateNodeBuilder;

private:
  std::vector<ASTGateNode*> Graph;
  std::set<std::string> GlobalGates;

public:
  using list_type = std::vector<ASTGateNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTGateNodeList() : Graph(), GlobalGates() {
    GlobalGates.insert("CX");
    GlobalGates.insert("cx");
    GlobalGates.insert("CCX");
    GlobalGates.insert("ccx");
    GlobalGates.insert("CNOT");
    GlobalGates.insert("cnot");
    GlobalGates.insert("H");
    GlobalGates.insert("h");
    GlobalGates.insert("U");
  }

  ASTGateNodeList(const ASTGateNodeList& RHS)
  : Graph(RHS.Graph), GlobalGates(RHS.GlobalGates) { }

  virtual ~ASTGateNodeList() = default;

  ASTGateNodeList& operator=(const ASTGateNodeList& RHS) {
    if (this != &RHS) {
      Graph = RHS.Graph;
      GlobalGates = RHS.GlobalGates;
    }

    return *this;
  }

  virtual size_t Size() const {
    return Graph.size();
  }

  virtual void Clear() {
    Graph.clear();
  }

  virtual void Append(ASTGateNode* G) {
    if (G) {
      Graph.push_back(G);
      GlobalGates.insert(G->GetName());
    }
  }

  virtual bool IsGlobalGate(const std::string& GS) const {
    return GlobalGates.find(GS) != GlobalGates.end();
  }

  iterator begin() { return Graph.begin(); }
  const_iterator begin() const { return Graph.begin(); }

  iterator end() { return Graph.end(); }
  const_iterator end() const { return Graph.end(); }

  ASTGateNode* front() { return Graph.front(); }
  const ASTGateNode* front() const { return Graph.front(); }

  ASTGateNode* back() { return Graph.back(); }
  const ASTGateNode* back() const { return Graph.back(); }

  virtual ASTType GetASTType() const {
    return ASTTypeGateQOpList;
  }

  inline virtual ASTGateNode* operator[](size_t Index) {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index];
  }

  inline virtual const ASTGateNode* operator[](size_t Index) const {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index];
  }

  virtual void print() const {
    std::cout << "<GateQOpList>" << std::endl;

    for (std::vector<ASTGateNode*>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I)
      (*I)->print();

    std::cout << "</GateQOpList>" << std::endl;
  }

  virtual void push(ASTBase* Node) {
    if (ASTGateNode* GN = dynamic_cast<ASTGateNode*>(Node))
      Graph.push_back(GN);
  }
};

}

#endif // __QASM_AST_GATE_NODE_LIST_H

