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

#ifndef __QASM_AST_CBIT_NODE_MAP_H
#define __QASM_AST_CBIT_NODE_MAP_H

#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTIdentifier.h>

#include <map>
#include <string>

namespace QASM {

class ASTCBitNodeMap : public ASTBase {
private:
  static ASTCBitNodeMap MI;
  static std::map<std::string, ASTCBitNode *> MM;

protected:
  ASTCBitNodeMap() {}

public:
  using map_type = std::map<std::string, ASTCBitNode *>;
  using iterator = typename map_type::iterator;
  using const_iterator = typename map_type::const_iterator;

public:
  static ASTCBitNodeMap &Instance() { return MI; }

  static const std::map<std::string, ASTCBitNode *> &Map() { return MM; }

  virtual ~ASTCBitNodeMap() = default;

  virtual bool Insert(const std::string &K, ASTCBitNode *N) {
    assert(N && "Invalid ASTCBitNode Argument!");

    if (K.empty())
      return false;

    const_iterator I = MM.find(K);

    if (I == MM.end())
      return MM.insert(std::make_pair(K, N)).second;

    return true;
  }

  virtual bool Insert(const ASTIdentifierNode *Id, ASTCBitNode *N) {
    assert(Id && "Invalid ASTIdentifierNode Argument!");

    return Insert(Id->GetName(), N);
  }

  virtual const ASTCBitNode *Find(const std::string &K) const {
    if (K.empty())
      return nullptr;

    const_iterator I = MM.find(K);
    return I == MM.end() ? nullptr : (*I).second;
  }

  virtual const ASTCBitNode *Find(const ASTIdentifierNode *Id) const {
    assert(Id && "Invalid ASTIdentifierNode Argument!");

    return Find(Id->GetName());
  }

  virtual ASTType GetASTType() const override { return ASTTypeCBitNodeMap; }

  virtual void print() const override {
    std::cout << "<CBitNodeMap>" << std::endl;

    for (const_iterator I = MM.begin(); I != MM.end(); ++I) {
      std::cout << "<Identifier>" << (*I).first << "</Identifier>" << std::endl;
      (*I).second->print();
    }

    std::cout << "</CBitNodeMap>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_CBIT_NODE_MAP_H
