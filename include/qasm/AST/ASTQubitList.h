/* -*- coding: utf-8 -*-
 *
 * Copyright 2023 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_QUBIT_LIST_H
#define __QASM_AST_QUBIT_LIST_H

#include <qasm/AST/ASTTypes.h>

#include <vector>

namespace QASM {

class ASTBoundQubitList : public ASTBase {
  friend class ASTBoundQubitListBuilder;

private:
  std::vector<ASTStringNode *> SV;

public:
  using list_type = std::vector<ASTStringNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTBoundQubitList() = default;

  ASTBoundQubitList(const ASTBoundQubitList &RHS) : ASTBase(RHS), SV(RHS.SV) {}

  virtual ~ASTBoundQubitList() = default;

  ASTBoundQubitList &operator=(const ASTBoundQubitList &RHS) {
    if (this != &RHS) {
      ASTBase::operator=(RHS);
      SV = RHS.SV;
    }

    return *this;
  }

  virtual ASTType GetASTType() const override { return ASTTypeBoundQubitList; }

  virtual void Append(ASTStringNode *N) {
    assert(N && "Invalid ASTStringNode argument!");
    SV.push_back(N);
  }

  virtual void Clear() { SV.clear(); }

  virtual unsigned Size() const { return static_cast<unsigned>(SV.size()); }

  virtual bool Empty() const { return SV.empty(); }

  virtual bool ValidateQubits() const;

  virtual bool ToASTIdentifierList(ASTIdentifierList &IL) const;

  iterator begin() { return SV.begin(); }

  const_iterator begin() const { return SV.begin(); }

  iterator end() { return SV.end(); }

  const_iterator end() const { return SV.end(); }

  virtual void print() const override {
    std::cout << "<BoundQubitList>" << std::endl;
    for (ASTBoundQubitList::const_iterator I = begin(); I != end(); ++I)
      (*I)->print();
    std::cout << "</BoundQubitList>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTUnboundQubitList : public ASTBase {
private:
  std::vector<ASTStringNode *> SV;

public:
  using list_type = std::vector<ASTStringNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTUnboundQubitList() = default;

  ASTUnboundQubitList(const ASTUnboundQubitList &RHS)
      : ASTBase(RHS), SV(RHS.SV) {}

  virtual ~ASTUnboundQubitList() = default;

  ASTUnboundQubitList &operator=(const ASTUnboundQubitList &RHS) {
    if (this != &RHS) {
      ASTBase::operator=(RHS);
      SV = RHS.SV;
    }

    return *this;
  }

  virtual ASTType GetASTType() const override {
    return ASTTypeUnboundQubitList;
  }

  virtual void Append(ASTStringNode *N) {
    assert(N && "Invalid ASTStringNode argument!");
    SV.push_back(N);
  }

  virtual void Clear() { SV.clear(); }

  virtual unsigned Size() const { return static_cast<unsigned>(SV.size()); }

  virtual bool Empty() const { return SV.empty(); }

  virtual bool ValidateQubits() const;

  iterator begin() { return SV.begin(); }

  const_iterator begin() const { return SV.begin(); }

  iterator end() { return SV.end(); }

  const_iterator end() const { return SV.end(); }

  virtual void print() const override {
    std::cout << "<UnboundQubitList>" << std::endl;
    for (ASTUnboundQubitList::const_iterator I = begin(); I != end(); ++I)
      (*I)->print();
    std::cout << "</UnboundQubitList>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTBoundQubitListBuilder {
protected:
  static ASTBoundQubitListBuilder BQB;
  static ASTBoundQubitList BQL;
  static ASTBoundQubitList *BQP;
  static std::vector<ASTBoundQubitList *> BQV;

protected:
  ASTBoundQubitListBuilder() = default;

public:
  static ASTBoundQubitListBuilder &Instance() { return BQB; }

  ~ASTBoundQubitListBuilder() = default;

  static ASTBoundQubitList *List() { return ASTBoundQubitListBuilder::BQP; }

  static ASTBoundQubitList *NewList() {
    ASTBoundQubitList *QL = new ASTBoundQubitList();
    assert(QL && "Could not create a valid ASTBoundQubitList!");

    BQP = QL;
    BQV.push_back(BQP);
    return BQP;
  }

  static void Init() {
    if (!BQP) {
      BQP = NewList();
      assert(BQP && "Could not create a valid ASTBoundQubitList!");
      BQV.push_back(BQP);
    }
  }

  void Append(const std::string &Q) { BQP->Append(new ASTStringNode(Q)); }

  void Append(ASTStringNode *Q) {
    assert(Q && "Invalid ASTStringNode argument!");
    BQP->Append(Q);
  }

  void Clear() { BQP->Clear(); }

  size_t Size() const { return BQP->Size(); }
};

} // namespace QASM

#endif // __QASM_AST_QUBIT_LIST_H
