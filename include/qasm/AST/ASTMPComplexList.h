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

#ifndef __QASM_AST_MPCOMPLEX_LIST_H
#define __QASM_AST_MPCOMPLEX_LIST_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTExpression.h>

#include <iostream>
#include <vector>
#include <cassert>

namespace QASM {

class ASTMPComplexRep : public ASTBase {
private:
  ASTMPComplexNode* MPC;

  union {
    const ASTBinaryOpNode* BOP;
    const ASTUnaryOpNode* UOP;
    const ASTIdentifierNode* ID;
    const ASTIdentifierRefNode* IDR;
    const void* Void;
  };

  ASTType RType;

private:
  ASTMPComplexRep() = delete;

protected:
  std::string MangleIdentifier(const ASTIdentifierNode* Id) const;

public:
  explicit ASTMPComplexRep(ASTMPComplexNode* X)
  : MPC(X), Void(nullptr), RType(X->GetASTType()) { }

  explicit ASTMPComplexRep(const ASTBinaryOpNode* X)
  : MPC(nullptr), BOP(X), RType(X->GetASTType()) { }

  explicit ASTMPComplexRep(const ASTBinaryOpNode* X,
                           ASTMPComplexNode* CX)
  : MPC(CX), BOP(X), RType(X->GetASTType()) { }

  explicit ASTMPComplexRep(const ASTUnaryOpNode* X)
  : MPC(nullptr), UOP(X), RType(X->GetASTType()) { }

  explicit ASTMPComplexRep(const ASTUnaryOpNode* X,
                           ASTMPComplexNode* CX)
  : MPC(CX), UOP(X), RType(X->GetASTType()) { }

  explicit ASTMPComplexRep(const ASTIdentifierNode* X)
  : MPC(nullptr), ID(X), RType(X->GetASTType()) { }

  explicit ASTMPComplexRep(const ASTIdentifierNode* X,
                           ASTMPComplexNode* CX)
  : MPC(CX), ID(X), RType(X->GetASTType()) { }

  explicit ASTMPComplexRep(const ASTIdentifierRefNode* X)
  : MPC(nullptr), IDR(X), RType(X->GetASTType()) { }

  explicit ASTMPComplexRep(const ASTIdentifierRefNode* X,
                           ASTMPComplexNode* CX)
  : MPC(CX), IDR(X), RType(X->GetASTType()) { }

  virtual ~ASTMPComplexRep() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeMPComplexRep;
  }

  virtual ASTMPComplexNode* GetComplex() {
    return MPC;
  }

  virtual const ASTMPComplexNode* GetComplex() const {
    return MPC;
  }

  virtual const ASTBinaryOpNode* GetBinaryOp() const {
    return RType == ASTTypeBinaryOp ? BOP : nullptr;
  }

  virtual const ASTUnaryOpNode* GetUnaryOp() const {
    return RType == ASTTypeUnaryOp ? UOP : nullptr;
  }

  virtual const ASTIdentifierNode* GetIdentifier() const {
    return RType == ASTTypeIdentifier ? ID : nullptr;
  }

  virtual const ASTIdentifierRefNode* GetIdentifierRef() const {
    return RType == ASTTypeIdentifierRef ? IDR : nullptr;
  }

  // Implemented in ASTMPComplexList.cpp.
  virtual std::string Mangle();

  virtual void print() const override {
    if (MPC) {
      MPC->print();
    } else {
      std::cout << "<MPComplex></MPComplex>" << std::endl;
    }

    switch (RType) {
    case ASTTypeBinaryOp:
      BOP->print();
      break;
    case ASTTypeUnaryOp:
      UOP->print();
      break;
    case ASTTypeIdentifier:
      ID->print();
      break;
    case ASTTypeIdentifierRef:
      IDR->print();
      break;
    default:
      break;
    }
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTMPComplexList : public ASTBase {
private:
  std::vector<ASTMPComplexRep*> List;

private:
  void ConstructFromExpressionList(const ASTExpressionList& EL);

public:
  using list_type = typename std::vector<ASTMPComplexRep*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTMPComplexList() = default;

  ASTMPComplexList(const ASTExpressionList& EL);

  ASTMPComplexList(const ASTMPComplexList& RHS)
  : ASTBase(RHS), List(RHS.List) { }

  virtual ~ASTMPComplexList() = default;

  ASTMPComplexList&
  operator=(const ASTMPComplexList& RHS) {
    if (this != &RHS) {
      ASTBase::operator=(RHS);
      List = RHS.List;
    }

    return *this;
  }

  virtual ASTType GetASTType() const override {
    return ASTTypeComplexList;
  }

  virtual void Clear() {
    List.clear();
  }

  virtual size_t Size() const {
    return List.size();
  }

  virtual bool Empty() const {
    return List.empty();
  }

  virtual void Append(ASTMPComplexRep* CXN) {
    List.push_back(CXN);
  }

  virtual void Prepend(ASTMPComplexRep* CXN) {
    List.insert(List.begin(), CXN);
  }

  // Implemented in ASTMPComplexList.cpp.
  virtual std::string Mangle();

  ASTMPComplexNode* GetComplex(unsigned Index) {
    assert(Index < List.size() && "Index is out-of-range!");
    try {
      ASTMPComplexRep* CR = List.at(Index);
      return CR ? CR->GetComplex() : nullptr;
    } catch (const std::out_of_range& E) {
      (void) E;
    } catch ( ... ) {
    }

    return nullptr;
  }

  const ASTMPComplexNode* GetComplex(unsigned Index) const {
    assert(Index < List.size() && "Index is out-of-range!");
    try {
      const ASTMPComplexRep* CR = List.at(Index);
      return CR ? CR->GetComplex() : nullptr;
    } catch (const std::out_of_range& E) {
      (void) E;
    } catch ( ... ) {
    }

    return nullptr;
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

  ASTMPComplexNode* First() {
    return List.front()->GetComplex();
  }

  const ASTMPComplexNode* First() const {
    return List.front()->GetComplex();
  }

  ASTMPComplexNode* Last() {
    return List.back()->GetComplex();
  }

  const ASTMPComplexNode* Last() const {
    return List.back()->GetComplex();
  }

  virtual void print() const override {
    std::cout << "<MPComplexList>" << std::endl;
    for (ASTMPComplexList::const_iterator I = List.begin();
         I != List.end(); ++I) {
      (*I)->print();
    }

    std::cout << "</MPComplexList>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM

#endif // __QASM_AST_MPCOMPLEX_LIST_H

