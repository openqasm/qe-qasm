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

#ifndef __QASM_AST_CTRL_ASSOC_BUILDER_H
#define __QASM_AST_CTRL_ASSOC_BUILDER_H

#include <qasm/AST/ASTTypes.h>

#include <vector>

namespace QASM {

class ASTCtrlAssocList {
  friend class CtrlAssocBuilder;

private:
  std::vector<ASTExpressionNode *> CL;

public:
  using list_type = std::vector<ASTExpressionNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTCtrlAssocList() : CL() {}
  ASTCtrlAssocList(const ASTCtrlAssocList &RHS) : CL(RHS.CL) {}
  ~ASTCtrlAssocList() = default;
  ASTCtrlAssocList &operator=(const ASTCtrlAssocList &RHS) {
    if (this != &RHS)
      CL = RHS.CL;

    return *this;
  }

  std::size_t Size() const { return CL.size(); }

  void Clear() { CL.clear(); }

  void Append(ASTExpressionNode *GN) {
    assert(GN && "Invalid ASTExpressionNode argument!");
    CL.push_back(GN);
  }

  iterator begin() { return CL.begin(); }

  const_iterator begin() const { return CL.begin(); }

  iterator end() { return CL.end(); }

  const_iterator end() const { return CL.end(); }

  ASTExpressionNode *front() { return CL.front(); }

  const ASTExpressionNode *front() const { return CL.front(); }

  ASTExpressionNode *back() { return CL.back(); }

  const ASTExpressionNode *back() const { return CL.back(); }

  void print() const {
    std::cout << "<CtrlAssocList>" << std::endl;

    for (const_iterator I = CL.begin(); I != CL.end(); ++I)
      (*I)->print();

    std::cout << "</CtrlAssocList>" << std::endl;
  }
};

class ASTCtrlAssocListBuilder {
private:
  static ASTCtrlAssocList CAL;
  static ASTCtrlAssocListBuilder CAB;

protected:
  ASTCtrlAssocListBuilder() = default;

public:
  using list_type = std::vector<ASTExpressionNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTCtrlAssocListBuilder &Instance() {
    return ASTCtrlAssocListBuilder::CAB;
  }

  ~ASTCtrlAssocListBuilder() = default;

  static ASTCtrlAssocList *List() { return &CAL; }

  void Clear() { CAL.Clear(); }

  std::size_t Size() const { return CAL.Size(); }

  void Append(ASTExpressionNode *GN) { CAL.Append(GN); }

  iterator begin() { return CAL.begin(); }

  const_iterator begin() const { return CAL.begin(); }

  iterator end() { return CAL.end(); }

  const iterator end() const { return CAL.end(); }

  ASTExpressionNode *front() { return CAL.front(); }

  const ASTExpressionNode *front() const { return CAL.front(); }

  ASTExpressionNode *back() { return CAL.back(); }

  const ASTExpressionNode *back() const { return CAL.back(); }

  void print() const { CAL.print(); }
};

class ASTControlExpressionNode : public ASTExpressionNode {
private:
  const ASTCtrlAssocList *CL;

private:
  ASTControlExpressionNode() = delete;

public:
  static const unsigned ControlExpressionBits = 64U;

public:
  ASTControlExpressionNode(const ASTIdentifierNode *Id,
                           const ASTCtrlAssocList *CAL)
      : ASTExpressionNode(Id, ASTTypeControlExpression), CL(CAL) {}

  virtual ~ASTControlExpressionNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeControlExpression;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual const ASTCtrlAssocList &GetControlList() const { return *CL; }

  virtual void print() const override {
    std::cout << "<ControlExpression>" << std::endl;
    CL->print();
    std::cout << "</ControlExpression>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_CTRL_ASSOC_BUILDER_H
