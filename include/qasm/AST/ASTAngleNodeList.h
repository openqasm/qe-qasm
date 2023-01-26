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

#ifndef __QASM_AST_ANGLE_NODE_LIST_H
#define __QASM_AST_ANGLE_NODE_LIST_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTTypes.h>

#include <vector>
#include <set>
#include <map>
#include <string>
#include <cassert>

namespace QASM {

class ASTAngleNodeList {
  friend class ASTAngleNodeBuilder;

private:
  std::vector<ASTAngleNode*> Graph;
  std::set<std::size_t> Hash;

public:
  using list_type = std::vector<ASTAngleNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTAngleNodeList() : Graph(), Hash() { }

  ASTAngleNodeList(const ASTAngleNodeList& RHS)
  : Graph(RHS.Graph), Hash(RHS.Hash) { }

  virtual ~ASTAngleNodeList() = default;

  ASTAngleNodeList& operator=(const ASTAngleNodeList& RHS) {
    if (this != &RHS) {
      Graph = RHS.Graph;
      Hash = RHS.Hash;
    }

    return *this;
  }

  virtual size_t Size() const {
    assert(Graph.size() == Hash.size() &&
           "Inconsistent Graph <-> Hash sizes!");
    return Graph.size();
  }

  virtual void Clear() {
    Graph.clear();
    Hash.clear();
  }

  iterator begin() { return Graph.begin(); }
  const_iterator begin() const { return Graph.begin(); }

  iterator end() { return Graph.end(); }
  const_iterator end() const { return Graph.end(); }

  ASTAngleNode* front() { return Graph.front(); }
  const ASTAngleNode* front() const { return Graph.front(); }

  ASTAngleNode* back() { return Graph.back(); }
  const ASTAngleNode* back() const { return Graph.back(); }

  virtual ASTType GetASTType() const {
    return ASTTypeAngleList;
  }

  inline virtual ASTAngleNode* operator[](size_t Index) {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index];
  }

  inline virtual const ASTAngleNode* operator[](size_t Index) const {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index];
  }

  virtual void print() const {
    std::cout << "<AngleList>" << std::endl;

    for (std::vector<ASTAngleNode*>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I)
      (*I)->print();

    std::cout << "</AngleList>" << std::endl;
  }

  virtual void push(ASTBase* Node) {
    if (ASTAngleNode* AN = dynamic_cast<ASTAngleNode*>(Node)) {
      if (Hash.find(AN->GetHash()) == Hash.end()) {
        Graph.push_back(AN);
        Hash.insert(AN->GetHash());
      }
    }
  }
};

struct ASTAngleKey {
  std::pair<std::string, unsigned> K;

  ASTAngleKey() : K() { }

  ASTAngleKey(const std::string& S, unsigned B)
  : K(S, B) { }

  ASTAngleKey(const ASTAngleKey& RHS)
  : K(RHS.K) { }

  ASTAngleKey& operator=(const ASTAngleKey& RHS) {
    if (this != &RHS)
      K = RHS.K;

    return *this;
  }

  ~ASTAngleKey() { }

  bool operator==(const ASTAngleKey& RHS) const {
    return K == RHS.K;
  }

  bool operator!=(const ASTAngleKey& RHS) const {
    return K != RHS.K;
  }

  bool operator<(const ASTAngleKey& RHS) const {
    return K < RHS.K;
  }

  bool operator>(const ASTAngleKey& RHS) const {
    return K > RHS.K;
  }
};

class ASTAngleNodeMap {
  friend class ASTAngleNodeBuilder;

private:
  std::map<ASTAngleKey, ASTAngleNode*> AngleMap;
  static ASTAngleNodeMap AM;

private:
  bool InvalidBits(unsigned Bits) {
    return Bits == 0 || Bits == static_cast<unsigned>(~0x0);
  }

public:
  using map_type = typename std::map<ASTAngleKey, ASTAngleNode*>;
  using iterator = map_type::iterator;
  using const_iterator = map_type::const_iterator;

protected:
  ASTAngleNodeMap() : AngleMap() { }

public:
  static ASTAngleNodeMap& Instance() {
    return ASTAngleNodeMap::AM;
  }

  virtual ~ASTAngleNodeMap() = default;

  virtual bool Insert(const std::string& Id, ASTAngleNode* Node) {
    if (InvalidBits(Node->GetBits()))
        return false;

    ASTAngleKey K(Id, Node->GetBits());

    iterator I = AngleMap.find(K);

    if (I != AngleMap.end())
      return true;

    return AngleMap.insert(std::make_pair(K, Node)).second;
  }

  virtual ASTAngleNode* Find(const std::string& Id,
                             unsigned NumBits = ASTAngleNode::AngleBits) {
    ASTAngleKey K(Id, NumBits);

    iterator I = AngleMap.find(K);

    if (I != AngleMap.end())
      return (*I).second;

    return nullptr;
  }

  virtual const ASTAngleNode* Find(const std::string& Id,
                                   unsigned NumBits = ASTAngleNode::AngleBits) const {
    ASTAngleKey K(Id, NumBits);

    const_iterator I = AngleMap.find(K);

    if (I != AngleMap.end())
      return (*I).second;

    return nullptr;
  }

  virtual void Clear() {
    AngleMap.clear();
  }

  virtual ASTType GetASTType() const {
    return ASTTypeAngleMap;
  }

  iterator begin() { return AngleMap.begin(); }
  const_iterator begin() const { return AngleMap.begin(); }

  iterator end() { return AngleMap.end(); }
  const_iterator end() const { return AngleMap.end(); }

  virtual void print() const {
    std::cout << "<AngleNodeMap>" << std::endl;

    for (const_iterator I = AngleMap.begin(); I != AngleMap.end(); ++I) {
      std::cout << "<Identifier>" << (*I).first.K.first
        << "</Identifier>" << std::endl;
      std::cout << "<Bits>" << (*I).first.K.second << "</Bits>" << std::endl;
      (*I).second->print();
    }

    std::cout << "</AngleNodeMap>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_ANGLE_NODE_LIST_H

