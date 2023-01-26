/* -*- coding: utf-8 -*-
 *
 * Copyright 2021 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_FOR_RANGE_INIT_BUILDER_H
#define __QASM_AST_FOR_RANGE_INIT_BUILDER_H

#include <qasm/AST/ASTTypes.h>

#include <vector>
#include <string>

namespace QASM {

class ASTForRangeInitList {
  friend class ASTForRangeInitListBuilder;

private:
  std::vector<ASTExpressionNode*> List;
  std::string Operator;
  char Separator;

public:
  using list_type = std::vector<ASTExpressionNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTForRangeInitList() : List(), Operator(""), Separator('\0') { }
  ASTForRangeInitList(const ASTForRangeInitList& RHS)
  : List(RHS.List), Operator(RHS.Operator), Separator(RHS.Separator) { }

  ~ASTForRangeInitList() = default;

  ASTForRangeInitList& operator=(const ASTForRangeInitList& RHS) {
    if (this != &RHS) {
      List = RHS.List;
      Operator = RHS.Operator;
      Separator = RHS.Separator;
    }

    return *this;
  }

  void Append(ASTIntNode* I) {
    assert(I && "Invalid ASTIntNode argument!");
    List.push_back(I);
  }

  void Append(ASTBinaryOpNode* BOp) {
    assert(BOp && "Invalid ASTBinaryOpNode argument!");
    List.push_back(BOp);
  }

  void Append(ASTUnaryOpNode* UOp) {
    assert(UOp && "Inalid ASTUnaryOpNode argument!");
    List.push_back(UOp);
  }

  void Append(ASTExpressionNode* EN) {
    assert(EN && "Invalid ASTExpressionNode argument!");
    List.push_back(EN);
  }

  void Append(char S) {
    Separator = S;
  }

  void Append(const std::string& O) {
    Operator = O;
  }

  void Clear() {
    List.clear();
  }

  std::size_t Size() const {
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

  ASTExpressionNode* front() {
    return List.front();
  }

  const ASTExpressionNode* front() const {
    return List.front();
  }

  ASTExpressionNode* back() {
    return List.back();
  }

  const ASTExpressionNode* back() const {
    return List.back();
  }

  void print() const {
    std::cout << "<ForRangeInitList>" << std::endl;

    for (const_iterator I = List.begin(); I != List.end(); ++I)
      (*I)->print();

    std::cout << "</ForRangeInitList>" << std::endl;
  }
};

class ASTForRangeInitListBuilder {
private:
  static ASTForRangeInitList RIL;
  static ASTForRangeInitListBuilder RIB;

protected:
  ASTForRangeInitListBuilder() = default;

public:
  using list_type = std::vector<ASTExpressionNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTForRangeInitListBuilder& Instance() {
    return ASTForRangeInitListBuilder::RIB;
  }

  ~ASTForRangeInitListBuilder() = default;

  static ASTForRangeInitList* List() {
    return &RIL;
  }

  void Clear() {
    RIL.Clear();
  }

  std::size_t Size() const {
    return RIL.Size();
  }

  void Append(ASTIntNode* I) {
    RIL.Append(I);
  }

  void Append(ASTBinaryOpNode* BOp) {
    RIL.Append(BOp);
  }

  void Append(ASTUnaryOpNode* UOp) {
    RIL.Append(UOp);
  }

  void Append(ASTExpressionNode* EN) {
    RIL.Append(EN);
  }

  void Append(char S) {
    RIL.Append(S);
  }

  void Append(const std::string& O) {
    RIL.Append(O);
  }

  iterator begin() {
    return RIL.begin();
  }

  const_iterator begin() const {
    return RIL.begin();
  }

  iterator end() {
    return RIL.end();
  }

  const iterator end() const {
    return RIL.end();
  }

  ASTExpressionNode* front() {
    return RIL.front();
  }

  const ASTExpressionNode* front() const {
    return RIL.front();
  }

  ASTExpressionNode* back() {
    return RIL.back();
  }

  const ASTExpressionNode* back() const {
    return RIL.back();
  }

  void print() const {
    RIL.print();
  }
};

} // namespace QASM


#endif // __QASM_AST_FOR_RANGE_INIT_BUILDER_H

