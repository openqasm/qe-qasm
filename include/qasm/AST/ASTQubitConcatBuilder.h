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

#ifndef __QASM_AST_QUBIT_CONCAT_BUILDER_H
#define __QASM_AST_QUBIT_CONCAT_BUILDER_H

#include <qasm/AST/ASTIdentifier.h>

#include <vector>

namespace QASM {

class ASTQubitConcatList {
  friend class ASTQubitConcatBuilder;

private:
  std::vector<ASTIdentifierNode*> IL;

public:
  using list_type = std::vector<ASTIdentifierNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTQubitConcatList() : IL() { }
  ASTQubitConcatList(const ASTQubitConcatList& RHS) : IL(RHS.IL) { }
  ~ASTQubitConcatList() = default;
  ASTQubitConcatList& operator=(const ASTQubitConcatList& RHS) {
    if (this != &RHS)
      IL = RHS.IL;

    return *this;
  }

  std::size_t Size() const {
    return IL.size();
  }

  void Clear() {
    IL.clear();
  }

  void Append(ASTIdentifierNode* IN) {
    assert(IN && "Invalid ASTIdentifierNode argument!");
    if (IN)
      IL.push_back(IN);
  }

  iterator begin() {
    return IL.begin();
  }

  const_iterator begin() const {
    return IL.begin();
  }

  iterator end() {
    return IL.end();
  }

  const_iterator end() const {
    return IL.end();
  }

  ASTIdentifierNode* front() {
    return IL.front();
  }

  const ASTIdentifierNode* front() const {
    return IL.front();
  }

  ASTIdentifierNode* back() {
    return IL.back();
  }

  const ASTIdentifierNode* back() const {
    return IL.back();
  }

  void print() const {
    std::cout << "<QubitConcatList>" << std::endl;

    for (const_iterator I = IL.begin(); I != IL.end(); ++I)
      (*I)->print();

    std::cout << "</QubitConcatList>" << std::endl;
  }
};

class ASTQubitConcatListBuilder {
  friend class ASTTypeSystemBuilder;

private:
  static ASTQubitConcatList* IL;
  static ASTQubitConcatListBuilder QCB;
  static std::vector<ASTQubitConcatList*> ILV;

protected:
  ASTQubitConcatListBuilder() = default;

protected:
  static void Init() {
    if (!IL)
      IL = new ASTQubitConcatList();
  }

public:
  using list_type = std::vector<ASTIdentifierNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTQubitConcatListBuilder& Instance() {
    return ASTQubitConcatListBuilder::QCB;
  }

  ~ASTQubitConcatListBuilder() = default;

  static ASTQubitConcatList* List() {
    return ILV.back();
  }

  static ASTQubitConcatList* NewList() {
    ILV.push_back(IL);
    IL = new ASTQubitConcatList();
    return IL;
  }

  void Clear() {
    IL->Clear();
  }

  std::size_t Size() const {
    return IL->Size();
  }

  void Append(ASTIdentifierNode* IN) {
    IL->Append(IN);
  }

  iterator begin() {
    return IL->begin();
  }

  const_iterator begin() const {
    return IL->begin();
  }

  iterator end() {
    return IL->end();
  }

  const iterator end() const {
    return IL->end();
  }

  ASTIdentifierNode* front() {
    return IL->front();
  }

  const ASTIdentifierNode* front() const {
    return IL->front();
  }

  ASTIdentifierNode* back() {
    return IL->back();
  }

  const ASTIdentifierNode* back() const {
    return IL->back();
  }

  void print() const {
    IL->print();
  }
};

} // namespace QASM

#endif // __QASM_AST_QUBIT_CONCAT_BUILDER_H

