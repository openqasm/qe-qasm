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

#ifndef __QASM_AST_SWITCH_STATEMENT_BUILDER_H
#define __QASM_AST_SWITCH_STATEMENT_BUILDER_H

#include <qasm/AST/ASTSwitchStatement.h>

#include <cassert>
#include <vector>

namespace QASM {

class ASTSwitchStatementBuilder {
private:
  static ASTSwitchStatementBuilder SSB;
  static ASTStatementList SL;
  static ASTStatementList *SLP;
  static std::vector<ASTStatementList *> SLV;

protected:
  ASTSwitchStatementBuilder() = default;

public:
  using list_type = ASTStatementList;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTSwitchStatementBuilder &Instance() {
    return ASTSwitchStatementBuilder::SSB;
  }

  ~ASTSwitchStatementBuilder() = default;

  static ASTStatementList *List() { return ASTSwitchStatementBuilder::SLP; }

  static ASTStatementList *NewList() {
    ASTStatementList *SSL = new ASTStatementList();
    assert(SSL && "Could not create a valid ASTStatementList!");
    SLP = SSL;
    SLV.push_back(SLP);
    return SLP;
  }

  static void Init() {
    if (!SLP) {
      SLP = NewList();
      assert(SLP && "Could not create a valid ASTStatementList!");
      SLV.push_back(SLP);
    }
  }

  void Append(ASTStatementNode *SN) {
    assert(SN && "Invalid ASTStatementNode argument!");
    if (SN && !SN->IsDirective())
      SLP->Append(SN);
  }

  void Prepend(ASTStatementNode *SN) {
    assert(SN && "Invalid ASTStatementNode argument!");
    if (SN && !SN->IsDirective())
      SLP->Prepend(SN);
  }

  void Clear() { SLP->Clear(); }

  std::size_t Size() const { return SLP->Size(); }

  iterator begin() { return SLP->begin(); }

  const_iterator begin() const { return SLP->begin(); }

  iterator end() { return SLP->end(); }

  const_iterator end() const { return SLP->end(); }

  ASTStatement *operator[](unsigned Index) {
    assert(Index < SLP->Size() && "Index is out-of-range!");
    return SLP->operator[](Index);
  }

  const ASTStatement *operator[](unsigned Index) const {
    assert(Index < SLP->Size() && "Index is out-of-range!");
    return SLP->operator[](Index);
  }

  virtual void print() const {
    std::cout << "<ForStatementBuilderList>" << std::endl;
    SLP->print();
    std::cout << "</ForStatementBuilderList>" << std::endl;
  }
};

class ASTSwitchScopedStatementBuilder {
private:
  static ASTSwitchScopedStatementBuilder SSSB;
  static ASTStatementList SL;
  static ASTStatementList *SLP;
  static std::vector<ASTStatementList *> SLV;

protected:
  ASTSwitchScopedStatementBuilder() = default;

public:
  using list_type = ASTStatementList;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTSwitchScopedStatementBuilder &Instance() {
    return ASTSwitchScopedStatementBuilder::SSSB;
  }

  ~ASTSwitchScopedStatementBuilder() = default;

  static ASTStatementList *List() {
    return ASTSwitchScopedStatementBuilder::SLP;
  }

  static ASTStatementList *NewList() {
    ASTStatementList *SSL = new ASTStatementList();
    assert(SSL && "Could not create a valid ASTStatementList!");
    SLP = SSL;
    SLV.push_back(SLP);
    return SLP;
  }

  static void Init() {
    if (!SLP) {
      SLP = NewList();
      assert(SLP && "Could not create a valid ASTStatementList!");
      SLV.push_back(SLP);
    }
  }

  void Append(ASTStatementNode *SN) {
    assert(SN && "Invalid ASTStatementNode argument!");
    if (SN && !SN->IsDirective())
      SLP->Append(SN);
  }

  void Prepend(ASTStatementNode *SN) {
    assert(SN && "Invalid ASTStatementNode argument!");
    if (SN && !SN->IsDirective())
      SLP->Prepend(SN);
  }

  void Clear() { SLP->Clear(); }

  std::size_t Size() const { return SLP->Size(); }

  iterator begin() { return SLP->begin(); }

  const_iterator begin() const { return SLP->begin(); }

  iterator end() { return SLP->end(); }

  const_iterator end() const { return SLP->end(); }

  ASTStatement *operator[](unsigned Index) {
    assert(Index < SLP->Size() && "Index is out-of-range!");
    return SLP->operator[](Index);
  }

  const ASTStatement *operator[](unsigned Index) const {
    assert(Index < SLP->Size() && "Index is out-of-range!");
    return SLP->operator[](Index);
  }

  virtual void print() const {
    std::cout << "<SwitchScopedStatementBuilderList>" << std::endl;
    SLP->print();
    std::cout << "</SwitchScopedStatementBuilderList>" << std::endl;
  }
};

class ASTSwitchUnscopedStatementBuilder {
private:
  static ASTSwitchUnscopedStatementBuilder SUSB;
  static ASTStatementList SL;
  static ASTStatementList *SLP;
  static std::vector<ASTStatementList *> SLV;

protected:
  ASTSwitchUnscopedStatementBuilder() = default;

public:
  using list_type = ASTStatementList;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTSwitchUnscopedStatementBuilder &Instance() {
    return ASTSwitchUnscopedStatementBuilder::SUSB;
  }

  ~ASTSwitchUnscopedStatementBuilder() = default;

  static ASTStatementList *List() {
    return ASTSwitchUnscopedStatementBuilder::SLP;
  }

  static ASTStatementList *NewList() {
    ASTStatementList *SSL = new ASTStatementList();
    assert(SSL && "Could not create a valid ASTStatementList!");
    SLP = SSL;
    SLV.push_back(SLP);
    return SLP;
  }

  static void Init() {
    if (!SLP) {
      SLP = NewList();
      assert(SLP && "Could not create a valid ASTStatementList!");
      SLV.push_back(SLP);
    }
  }

  void Append(ASTStatementNode *SN) {
    assert(SN && "Invalid ASTStatementNode argument!");
    if (SN && !SN->IsDirective())
      SLP->Append(SN);
  }

  void Prepend(ASTStatementNode *SN) {
    assert(SN && "Invalid ASTStatementNode argument!");
    if (SN && !SN->IsDirective())
      SLP->Prepend(SN);
  }

  void Clear() { SLP->Clear(); }

  std::size_t Size() const { return SLP->Size(); }

  iterator begin() { return SLP->begin(); }

  const_iterator begin() const { return SLP->begin(); }

  iterator end() { return SLP->end(); }

  const_iterator end() const { return SLP->end(); }

  ASTStatement *operator[](unsigned Index) {
    assert(Index < SLP->Size() && "Index is out-of-range!");
    return SLP->operator[](Index);
  }

  const ASTStatement *operator[](unsigned Index) const {
    assert(Index < SLP->Size() && "Index is out-of-range!");
    return SLP->operator[](Index);
  }

  virtual void print() const {
    std::cout << "<SwitchUnscopedStatementBuilderList>" << std::endl;
    SLP->print();
    std::cout << "</SwitchUnscopedStatementBuilderList>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_SWITCH_STATEMENT_BUILDER_H
