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

#ifndef __QASM_AST_STRING_LIST_H
#define __QASM_AST_STRING_LIST_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTTypes.h>

#include <vector>
#include <string>

namespace QASM {

class ASTStringList : public ASTBase {
  friend class ASTStringListBuilder;

protected:
  std::vector<const ASTStringNode*> List;

public:
  using list_type = std::vector<const ASTStringNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTStringList() : ASTBase(), List() { }

  ASTStringList(const ASTStringList& RHS)
  : ASTBase(RHS), List(RHS.List) { }

  virtual ~ASTStringList() = default;

  ASTStringList& operator=(const ASTStringList& RHS) {
    if (this != &RHS) {
      (void) ASTBase::operator=(RHS);
      List = RHS.List;
    }

    return *this;
  }

  virtual ASTType GetASTType() const override {
    return ASTTypeStringList;
  }

  virtual unsigned Size() const {
    return static_cast<unsigned>(List.size());
  }

  virtual void Clear() {
    List.clear();
  }

  virtual bool Empty() const {
    return List.empty();
  }

  virtual void Append(const ASTStringNode* S) {
    assert(S && "Invalid ASTStringNode argument!");
    List.push_back(S);
  }

  virtual void Append(const std::string& S) {
    List.push_back(new ASTStringNode(S));
  }

  virtual void Append(const char* S) {
    assert(S && "Invalid const char* argument!");
    List.push_back(new ASTStringNode(S));
  }

  virtual void Prepend(const ASTStringNode* S) {
    assert(S && "Invalid ASTStringNode argument!");
    List.insert(List.begin(), S);
  }

  virtual void Prepend(const std::string& S) {
    List.insert(List.begin(), new ASTStringNode(S));
  }

  virtual void Prepend(const char* S) {
    assert(S && "Invalid const char* argument!");
    List.insert(List.begin(), new ASTStringNode(S));
  }

  const ASTStringNode* Front() {
    return List.size() ? List.front() : nullptr;
  }

  const ASTStringNode* Front() const {
    return List.size() ? List.front() : nullptr;
  }

  const ASTStringNode* Back() {
    return List.size() ? List.back() : nullptr;
  }

  const ASTStringNode* Back() const {
    return List.size() ? List.back() : nullptr;
  }

  iterator begin() {
    return List.begin();
  }

  const_iterator begin() const {
    return List.begin();
  }

  iterator end() {
    return List.end();
  }

  const_iterator end() const {
    return List.end();
  }

  virtual void print() const override {
    std::cout << "<StringList>" << std::endl;
    for (std::vector<const ASTStringNode*>::const_iterator I = List.begin();
         I != List.end(); ++I) {
      (*I)->print();
    }
    std::cout << "</StringList>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTStringListBuilder {
private:
  static ASTStringList SL;
  static ASTStringListBuilder SLB;
  static ASTStringList* SLP;
  static std::vector<ASTStringList*> SLV;

protected:
  ASTStringListBuilder() = default;

public:
  static ASTStringListBuilder& Instance() {
    return SLB;
  }

  ~ASTStringListBuilder() = default;

  static ASTStringList* List() {
    return SLP;
  }

  static ASTStringList* NewList() {
    ASTStringList* SSL = new ASTStringList();
    assert(SSL && "Could not create a valid ASTStringList!");
    SLP = SSL;
    SLV.push_back(SLP);
    return SLP;
  }

  static void Init() {
    if (!SLP) {
      SLP = NewList();
      assert(SLP && "Could not create a valid ASTStringList!");
      SLV.push_back(SLP);
    }
  }

  void Append(const std::string& S) {
    SLP->Append(S);
  }

  void Append(const char* S) {
    SLP->Append(S);
  }

  void Prepend(const std::string& S) {
    SLP->Prepend(S);
  }

  void Prepend(const char* S) {
    SLP->Prepend(S);
  }

  void Clear() {
    SLP->Clear();
  }

  unsigned Size() const {
    return SLP->Size();
  }
};

} // namespace QASM

#endif // __QASM_AST_STRING_LIST_H

