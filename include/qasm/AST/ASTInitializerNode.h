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

#ifndef __QASM_AST_INITIALIZER_H
#define __QASM_AST_INITIALIZER_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTExpressionNodeList.h>

#include <iostream>
#include <variant>

namespace QASM {

template<typename __Type>
class ASTInitializerNode : public ASTExpressionNode {
private:
  std::variant<const __Type*, __Type*> TV;

private:
  ASTInitializerNode() = delete;

public:
  explicit ASTInitializerNode(const __Type* TP)
  : ASTExpressionNode(TP->GetIdentifier(), TP, TP->GetASTType()),
  TV(TP) { }

  explicit ASTInitializerNode(__Type* TP)
  : ASTExpressionNode(TP->GetIdentifier(), TP, TP->GetASTType()),
  TV(TP) { }

  virtual ~ASTInitializerNode() = default;

  virtual ASTType GetASTType() const override {
    return TV.index() == 0 ? std::get<0>(TV)->GetASTType() :
                             std::get<1>(TV)->GetASTType();
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  const __Type* GetContainedType() const {
    return TV.index() == 0 ? std::get<0>(TV) : nullptr;
  }

  __Type* GetContainedType() {
    return TV.index() == 1 ? std::get<1>(TV) : nullptr;
  }

  virtual bool IsAggregate() const override {
    return TV.index() == 0 ? std::get<0>(TV)->IsAggregate() :
                             std::get<1>(TV)->IsAggregate();
  }

  virtual bool IsConst() const {
    return TV.index() == 0;
  }

  virtual bool IsInitializer() const override {
    return true;
  }

  virtual void print() const override {
    std::cout << "<IntializerNode>" << std::endl;

    if (TV.index() == 0)
      std::get<0>(TV)->print();
    else
      std::get<1>(TV)->print();

    std::cout << "</IntializerNode>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTInitializerList : public ASTExpressionNode {
  friend class ASTInitializerListBuilder;

private:
  std::vector<std::variant<const ASTInitializerList*,
                           const ASTExpressionNodeList*>> List;
  std::set<uint64_t> TS;

public:
  using list_type = std::vector<std::variant<const ASTInitializerList*,
                                             const ASTExpressionNodeList*>>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static const std::variant<const ASTInitializerList*,
                            const ASTExpressionNodeList*> InvalidVariant;

public:
  ASTInitializerList()
  : ASTExpressionNode(ASTIdentifierNode::InitializerList.Clone(), this,
                      ASTTypeInitializerList),
  List(), TS() { }

  virtual ~ASTInitializerList() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeInitializerList;
  }

  virtual void Append(ASTExpressionNodeList* EL) {
    assert(EL && "Invalid ASTExpressionNodeList argument!");
    if (TS.insert(reinterpret_cast<uintptr_t>(EL)).second)
      List.push_back(EL);
  }

  virtual void Append(const ASTExpressionNodeList* EL) {
    assert(EL && "Invalid ASTExpressionNodeList argument!");
    if (TS.insert(reinterpret_cast<uintptr_t>(EL)).second)
      List.push_back(EL);
  }

  virtual void Append(ASTInitializerList* IL) {
    assert(IL && "Invalid ASTInitializerList argument!");
    if (TS.insert(reinterpret_cast<uintptr_t>(IL)).second)
      List.push_back(IL);
  }

  virtual void Append(const ASTInitializerList* IL) {
    assert(IL && "Invalid ASTInitializerList argument!");
    if (TS.insert(reinterpret_cast<uintptr_t>(IL)).second)
      List.push_back(IL);
  }

  virtual void Prepend(ASTExpressionNodeList* EL) {
    assert(EL && "Invalid ASTExpressionNodeList argument!");
    if (TS.insert(reinterpret_cast<uintptr_t>(EL)).second)
      List.insert(List.begin(), EL);
  }

  virtual void Prepend(const ASTExpressionNodeList* EL) {
    assert(EL && "Invalid ASTExpressionNodeList argument!");
    if (TS.insert(reinterpret_cast<uintptr_t>(EL)).second)
      List.insert(List.begin(), EL);
  }

  virtual void Prepend(ASTInitializerList* IL) {
    assert(IL && "Invalid ASTInitializerList argument!");
    if (TS.insert(reinterpret_cast<uintptr_t>(IL)).second)
      List.insert(List.begin(), IL);
  }

  virtual void Prepend(const ASTInitializerList* IL) {
    assert(IL && "Invalid ASTInitializerList argument!");
    if (TS.insert(reinterpret_cast<uintptr_t>(IL)).second)
      List.insert(List.begin(), IL);
  }

  virtual bool Empty() const {
    return List.empty();
  }

  virtual void Clear() {
    List.clear();
    TS.clear();
  }

  size_t Size() const {
    return List.size();
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

  const std::variant<const ASTInitializerList*,
                     const ASTExpressionNodeList*>& front() const {
    return List.size() ? List.front() : InvalidVariant;
  }

  const std::variant<const ASTInitializerList*,
                     const ASTExpressionNodeList*>& back() const {
    return List.size() ? List.back() : InvalidVariant;
  }

  virtual void print() const override {
    std::cout << "<InitializerList>" << std::endl;

    for (const_iterator I = List.begin(); I != List.end(); ++I) {
      switch ((*I).index()) {
      case 0:
        std::get<0>(*I)->print();
        break;
      case 1:
        std::get<1>(*I)->print();
        break;
      default:
        break;
      }
    }

    std::cout << "</InitializerList>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM

#endif // __QASM_AST_INITIALIZER_H

