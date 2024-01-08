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

#ifndef __QASM_AST_EXPRESSION_H
#define __QASM_AST_EXPRESSION_H

#include <qasm/AST/ASTAnyType.h>
#include <qasm/AST/ASTBase.h>

#include <cassert>
#include <vector>

namespace QASM {

class ASTIdentifierNode;

class ASTExpression : public ASTBase {
public:
  ASTExpression() : ASTBase() {}

  ASTExpression(const ASTExpression &RHS) : ASTBase(RHS) {}

  virtual ~ASTExpression() = default;

  ASTExpression &operator=(const ASTExpression &RHS) {
    if (this != &RHS)
      ASTBase::operator=(RHS);

    return *this;
  }

  virtual ASTType GetASTType() const override { return ASTTypeExpression; }

  virtual const ASTIdentifierNode *GetIdentifier() const;

  virtual void print() const override {}

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTExpressionList {
  friend class ASTExpressionBuilder;

private:
  std::vector<ASTExpression *> List;

public:
  using list_type = std::vector<ASTExpression *>;
  using iterator = typename std::vector<ASTExpression *>::iterator;
  using const_iterator = typename std::vector<ASTExpression *>::const_iterator;

public:
  ASTExpressionList() : List() {}

  ASTExpressionList(const ASTExpressionList &RHS) : List(RHS.List) {}

  virtual ~ASTExpressionList() = default;

  ASTExpressionList &operator=(const ASTExpressionList &RHS) {
    if (this != &RHS)
      List = RHS.List;

    return *this;
  }

  virtual std::size_t Size() const { return List.size(); }

  virtual bool Empty() const { return List.empty(); }

  virtual void Append(ASTExpression *BN) { this->push(BN); }

  iterator begin() { return List.begin(); }
  const_iterator begin() const { return List.begin(); }

  iterator end() { return List.end(); }
  const_iterator end() const { return List.end(); }

  ASTExpression *front() { return List.front(); }
  const ASTExpression *front() const { return List.front(); }

  ASTExpression *back() { return List.back(); }
  const ASTExpression *back() const { return List.back(); }

  virtual ASTType GetASTType() const { return ASTTypeExpressionList; }

  virtual ASTExpression *operator[](std::size_t Index) {
    assert(Index < List.size() && "Index is out-of-range!");

    try {
      return List.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTExpression *operator[](std::size_t Index) const {
    assert(Index < List.size() && "Index is out-of-range!");

    try {
      return List.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual void print() const {
    std::cout << "<ExpressionList>" << std::endl;

    for (std::vector<ASTExpression *>::const_iterator I = List.begin();
         I != List.end(); ++I) {
      std::cout << "<ExpressionNode>" << std::endl;
      (*I)->print();
      std::cout << "</ExpressionNode>" << std::endl;
    }

    std::cout << "</ExpressionList>" << std::endl;
  }

  virtual void push(ASTExpression *EX) {
    assert(EX && "Invalid ASTExpression argument!");

    if (EX)
      List.push_back(EX);
  }
};

} // namespace QASM

#endif // __QASM_AST_EXPRESSION_H
