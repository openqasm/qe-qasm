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

#ifndef __QASM_AST_GATE_OP_LIST_H
#define __QASM_AST_GATE_OP_LIST_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTGates.h>

#include <vector>
#include <set>
#include <map>
#include <cassert>

namespace QASM {

class ASTSymbolTableEntry;

class ASTGateQOpList : public ASTBase {
  friend class ASTGateOpBuilder;

private:
  std::vector<ASTGateQOpNode*> List;

public:
  using list_type = std::vector<ASTGateQOpNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTGateQOpList EmptyDefault;

public:
  ASTGateQOpList() : ASTBase(), List() { }

  ASTGateQOpList(const ASTGateQOpList& RHS)
  : ASTBase(RHS), List(RHS.List) { }

  virtual ~ASTGateQOpList() = default;

  ASTGateQOpList& operator=(const ASTGateQOpList& RHS) {
    if (this != &RHS) {
      ASTBase::operator=(RHS);
      List = RHS.List;
    }

    return *this;
  }

  virtual size_t Size() const {
    return List.size();
  }

  virtual void Clear() {
    List.clear();
  }

  virtual bool Empty() const {
    return List.size() == 0;
  }

  virtual void Append(ASTGateQOpNode* QN) {
    if (QN)
      List.push_back(QN);
  }

  // Implemented in ASTGates.cpp.
  virtual bool TransferToSymbolTable(std::map<std::string,
                                     const ASTSymbolTableEntry*>& MM) const;

  iterator begin() { return List.begin(); }
  const_iterator begin() const { return List.begin(); }

  iterator end() { return List.end(); }
  const_iterator end() const { return List.end(); }

  ASTGateQOpNode* front() { return List.front(); }
  const ASTGateOpNode* front() const { return List.front(); }

  ASTGateQOpNode* back() { return List.back(); }
  const ASTGateQOpNode* back() const { return List.back(); }

  virtual ASTType GetASTType() const {
    return ASTTypeGateQOpList;
  }

  inline virtual ASTGateQOpNode* operator[](size_t Index) {
    assert(Index < List.size() && "Index is out-of-range!");
    return List[Index];
  }

  inline virtual const ASTGateQOpNode* operator[](size_t Index) const {
    assert(Index < List.size() && "Index is out-of-range!");
    return List[Index];
  }

  virtual void print() const {
    std::cout << "<GateQOpList>" << std::endl;

    for (std::vector<ASTGateQOpNode*>::const_iterator I = List.begin();
         I != List.end(); ++I)
      (*I)->print();

    std::cout << "</GateQOpList>" << std::endl;
  }

  virtual void push(ASTBase* Node) {
    if (ASTGateQOpNode* GN = dynamic_cast<ASTGateQOpNode*>(Node))
      List.push_back(GN);
  }
};

} // namespace QASM

#endif // __QASM_AST_GATE_OP_LIST_H

