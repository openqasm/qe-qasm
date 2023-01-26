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

#ifndef __QASM_AST_INVERSE_ASSOC_BUILDER_H
#define __QASM_AST_INVERSE_ASSOC_BUILDER_H

#include <qasm/AST/ASTTypes.h>

#include <vector>

namespace QASM {

class ASTInverseAssocList {
  friend class InverseAssocBuilder;

private:
  std::vector<ASTExpressionNode*> EL;

public:
  using list_type = std::vector<ASTExpressionNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTInverseAssocList() : EL() { }
  ASTInverseAssocList(const ASTInverseAssocList& RHS) : EL(RHS.EL) { }
  ~ASTInverseAssocList() = default;
  ASTInverseAssocList& operator=(const ASTInverseAssocList& RHS) {
    if (this != &RHS)
      EL = RHS.EL;

    return *this;
  }

  std::size_t Size() const {
    return EL.size();
  }

  void Clear() {
    EL.clear();
  }

  void Append(ASTExpressionNode* GN) {
    assert(GN && "Invalid ASTExpressionNode argument!");
    EL.push_back(GN);
  }

  iterator begin() {
    return EL.begin();
  }

  const_iterator begin() const {
    return EL.begin();
  }

  iterator end() {
    return EL.end();
  }

  const_iterator end() const {
    return EL.end();
  }

  ASTExpressionNode* front() {
    return EL.front();
  }

  const ASTExpressionNode* front() const {
    return EL.front();
  }

  ASTExpressionNode* back() {
    return EL.back();
  }

  const ASTExpressionNode* back() const {
    return EL.back();
  }

  void print() const {
    std::cout << "<InverseAssocList>" << std::endl;

    for (const_iterator I = EL.begin(); I != EL.end(); ++I)
      (*I)->print();

    std::cout << "</InverseAssocList>" << std::endl;
  }
};

class ASTInverseAssocListBuilder {
private:
  static ASTInverseAssocList IAL;
  static ASTInverseAssocListBuilder IAB;

protected:
  ASTInverseAssocListBuilder() = default;

public:
  using list_type = std::vector<ASTExpressionNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTInverseAssocListBuilder& Instance() {
    return ASTInverseAssocListBuilder::IAB;
  }

  ~ASTInverseAssocListBuilder() = default;

  static ASTInverseAssocList* List() {
    return &IAL;
  }

  void Clear() {
    IAL.Clear();
  }

  std::size_t Size() const {
    return IAL.Size();
  }

  void Append(ASTExpressionNode* GN) {
    IAL.Append(GN);
  }

  iterator begin() {
    return IAL.begin();
  }

  const_iterator begin() const {
    return IAL.begin();
  }

  iterator end() {
    return IAL.end();
  }

  const iterator end() const {
    return IAL.end();
  }

  ASTExpressionNode* front() {
    return IAL.front();
  }

  const ASTExpressionNode* front() const {
    return IAL.front();
  }

  ASTExpressionNode* back() {
    return IAL.back();
  }

  const ASTExpressionNode* back() const {
    return IAL.back();
  }

  void print() const {
    IAL.print();
  }
};

class ASTInverseExpressionNode : public ASTExpressionNode {
private:
  const ASTInverseAssocList* IL;

private:
  ASTInverseExpressionNode() = delete;

public:
  static const unsigned InverseExpressionBits = 64U;

public:
  ASTInverseExpressionNode(const ASTIdentifierNode* Id,
                           const ASTInverseAssocList* IAL)
  : ASTExpressionNode(Id, ASTTypeInverseExpression), IL(IAL) { }

  virtual ~ASTInverseExpressionNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeInverseExpression;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual const ASTIdentifierNode* GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string& GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual const ASTInverseAssocList& GetControlList() const {
    return *IL;
  }

  virtual void print() const override {
    std::cout << "<InverseExpression>" << std::endl;
    IL->print();
    std::cout << "</InverseExpression>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM



#endif // __QASM_AST_INVERSE_ASSOC_BUILDER_H

