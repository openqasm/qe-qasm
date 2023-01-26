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

#ifndef __QASM_AST_ARGUMENT_NODE_H
#define __QASM_AST_ARGUMENT_NODE_H

#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTDeclaration.h>
#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTAnyType.h>
#include <qasm/AST/ASTTypes.h>

#include <any>
#include <functional>
#include <vector>
#include <set>
#include <cassert>

namespace QASM {

class ASTArgumentNode : public ASTExpressionNode {
private:
  std::pair<const ASTIdentifierNode*, const ASTIdentifierRefNode*> Ident;
  ASTAnyType Value;
  bool Expr;
  bool ConstExpr;

private:
  ASTArgumentNode() = delete;

public:
  ASTArgumentNode(const ASTIdentifierNode* Id, const std::any& Val,
                  ASTType Ty)
  : ASTExpressionNode(Id, Ty), Ident(Id, nullptr), Value(Val, Ty),
  Expr(Ty == ASTTypeExpression), ConstExpr(true) { }

  ASTArgumentNode(const ASTIdentifierRefNode* Id, const std::any& Val,
                  ASTType Ty)
  : ASTExpressionNode(Id->GetIdentifier(), Ty), Ident(nullptr, Id),
  Value(Val, Ty), Expr(Ty == ASTTypeExpression), ConstExpr(true) { }

  ASTArgumentNode(const ASTExpressionNode* E)
  : ASTExpressionNode(E->GetIdentifier(), E->GetASTType()),
  Ident(E->GetIdentifier(), nullptr), Value(E, E->GetASTType()),
  Expr(true), ConstExpr(true) { }

  ASTArgumentNode(ASTExpressionNode* E)
  : ASTExpressionNode(E->GetIdentifier(), E->GetASTType()),
  Ident(E->GetIdentifier(), nullptr), Value(E, E->GetASTType()),
  Expr(true), ConstExpr(false) { }

  virtual ~ASTArgumentNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeArgument;
  }

  virtual bool IsExpression() const override {
    return Expr;
  }

  virtual bool IsConstExpr() const {
    return ConstExpr;
  }

  virtual bool IsValue() const {
    return Value.second != ASTTypeUndefined;
  }

  virtual bool IsIdentifier() const override {
    return Ident.first;
  }

  virtual bool IsIdentifierRef() const {
    return Ident.second;
  }

  virtual const ASTIdentifierNode* GetIdentifier() const override {
    if (IsIdentifier())
      return Ident.first;
    else if (IsIdentifierRef())
      return Ident.second->GetIdentifier();
    else if (IsExpression())
      return IsConstExpr() ?
             std::any_cast<const ASTExpressionNode*>(Value.first)->GetIdentifier() :
             std::any_cast<ASTExpressionNode*>(Value.first)->GetIdentifier();

    return ASTExpressionNode::GetIdentifier();
  }

  virtual const ASTIdentifierRefNode* GetIdentifierRef() const {
    return Ident.second;
  }

  virtual const std::string& GetName() const override {
    if (IsIdentifier())
      return Ident.first ? Ident.first->GetName() :
                           Ident.second->GetName();
    else if (IsExpression()) {
      try {
        const ASTExpressionNode* EN =
          std::any_cast<const ASTExpressionNode*>(Value.first);
        assert(EN && "Invalid any_cast to ASTExpressionNode!");

        const ASTIdentifierNode* ID = EN->GetIdentifier();
        return ID ? ID->GetName() :
                    ASTIdentifierNode::Null.GetName();
      } catch (const std::bad_any_cast& E) {
        return ASTIdentifierNode::Null.GetName();
      }
    } else if (IsValue()) {
      // FIXME: Implement ASTAnyType  std::any::print();
      return Ident.first ? Ident.first->GetName() :
                           Ident.second->GetName();
    } else
      return ASTExpressionNode::GetIdentifier()->GetName();
  }

  virtual std::any& GetValue() {
    return Value.first;
  }

  virtual const std::any& GetValue() const {
    return Value.first;
  }

  virtual ASTType GetValueType() const {
    return Value.second;
  }

  virtual const ASTExpressionNode* GetExpression() const {
    if (!IsExpression())
      return nullptr;

    try {
      return std::any_cast<const ASTExpressionNode*>(Value.first);
    } catch (const std::bad_any_cast& E) {
      (void) E;
      return nullptr;
    } catch ( ... ) {
      return nullptr;
    }
  }

  virtual ASTExpressionNode* GetExpression() {
    if (!IsExpression())
      return nullptr;

    try {
      return std::any_cast<ASTExpressionNode*>(Value.first);
    } catch (const std::bad_any_cast& E) {
      (void) E;
      return nullptr;
    } catch ( ... ) {
      return nullptr;
    }
  }

  virtual void print() const override {
    std::cout << "<Argument>" << std::endl;
    std::cout << "<Id>" << GetName() << "</Id>" << std::endl;
    std::cout << "<IsValue>" << std::boolalpha << IsValue()
      << "</IsValue>" << std::endl;
    std::cout << "<ValueType>" << PrintTypeEnum(GetValueType())
      << "</ValueType>" << std::endl;
    std::cout << "<IsExpression>" << std::boolalpha << IsExpression()
      << "</IsExpression>" << std::endl;
    std::cout << "</Argument>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTArgumentNodeList {
  friend class ASTArgumentNodeBuilder;

private:
  std::vector<ASTArgumentNode*> Graph;

public:
  using list_type = std::vector<ASTArgumentNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTArgumentNodeList() : Graph() { }

  ASTArgumentNodeList(const ASTArgumentNodeList& RHS)
  : Graph(RHS.Graph) { }

  ASTArgumentNodeList(const ASTExpressionList* EL)
  : Graph() {
    *this = EL;
  }

  virtual ~ASTArgumentNodeList() = default;

  ASTArgumentNodeList& operator=(const ASTArgumentNodeList& RHS) {
    if (this != &RHS) {
      Graph = RHS.Graph;
    }

    return *this;
  }

  ASTArgumentNodeList& operator=(const ASTExpressionList* EL);

  virtual size_t Size() const {
    return Graph.size();
  }

  virtual void Clear() {
    Graph.clear();
  }

  virtual bool Empty() const {
    return Graph.empty();
  }

  virtual void Append(ASTArgumentNode* N) {
    if (N)
      Graph.push_back(N);
  }

  virtual void Append(ASTExpressionNode* EN) {
    if (EN)
      Graph.push_back(new ASTArgumentNode(EN));
  }

  virtual void Append(const ASTExpressionNode* EN) {
    if (EN)
      Graph.push_back(new ASTArgumentNode(EN));
  }

  iterator begin() { return Graph.begin(); }
  const_iterator begin() const { return Graph.begin(); }

  iterator end() { return Graph.end(); }
  const_iterator end() const { return Graph.end(); }

  ASTArgumentNode* front() { return Graph.front(); }
  const ASTArgumentNode* front() const { return Graph.front(); }

  ASTArgumentNode* back() { return Graph.back(); }
  const ASTArgumentNode* back() const { return Graph.back(); }

  inline virtual ASTArgumentNode* operator[](size_t Index) {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index];
  }

  inline virtual const ASTArgumentNode* operator[](size_t Index) const {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index];
  }

  virtual void print() const {
    std::cout << "<ArgumentList>" << std::endl;

    for (std::vector<ASTArgumentNode*>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I)
      (*I)->print();

    std::cout << "</ArgumentList>" << std::endl;
  }

  virtual void push(ASTBase* Node) {
    if (ASTArgumentNode* AN = dynamic_cast<ASTArgumentNode*>(Node))
      Graph.push_back(AN);
  }
};

} // namespace QASM

#endif // __QASM_AST_ARGUMENT_NODE_H

