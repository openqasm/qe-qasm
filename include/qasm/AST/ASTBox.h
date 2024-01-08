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

#ifndef __QASM_AST_BOX_EXPRESSIONS_H
#define __QASM_AST_BOX_EXPRESSIONS_H

#include <qasm/AST/ASTBoxStatementBuilder.h>
#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <iostream>
#include <string>

namespace QASM {

class ASTBoxExpressionNode : public ASTExpressionNode {
  friend class ASTBoxStatementBuilder;

protected:
  ASTStatementList SL;

private:
  ASTBoxExpressionNode() = delete;

protected:
  ASTBoxExpressionNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        SL() {}

public:
  static const unsigned BoxBits = 64U;

public:
  ASTBoxExpressionNode(const ASTIdentifierNode *Id, const ASTStatementList &STL)
      : ASTExpressionNode(Id, ASTTypeBox), SL(STL) {}

  virtual ~ASTBoxExpressionNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeBox; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::GetName();
  }

  virtual const std::string &GetMangledName() const override {
    return ASTExpressionNode::GetMangledName();
  }

  virtual const ASTStatementList &GetStatementList() const { return SL; }

  virtual ASTStatementList &GetStatementList() { return SL; }

  virtual const ASTStatement *GetStatement(unsigned Index) const {
    assert(Index < SL.Size() && "Index is out-of-range!");
    return SL[Index];
  }

  virtual ASTStatement *GetStatement(unsigned Index) {
    assert(Index < SL.Size() && "Index is out-of-range!");
    return SL[Index];
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTBoxExpressionNode *ExpressionError(const std::string &ERM) {
    return new ASTBoxExpressionNode(ASTIdentifierNode::Box.Clone(), ERM);
  }

  static ASTBoxExpressionNode *ExpressionError(const ASTIdentifierNode *Id,
                                               const std::string &ERM) {
    return new ASTBoxExpressionNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<BoxExpression>" << std::endl;
    std::cout << "<Identifier>" << this->GetName() << "</Identifier>"
              << std::endl;
    SL.print();
    std::cout << "</BoxExpression>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTBoxAsExpressionNode : public ASTBoxExpressionNode {
private:
  ASTBoxAsExpressionNode() = delete;

protected:
  ASTBoxAsExpressionNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTBoxExpressionNode(Id, ERM) {}

public:
  static const unsigned BoxAsBits = 64U;

public:
  ASTBoxAsExpressionNode(const ASTIdentifierNode *Id,
                         const ASTStatementList &STL)
      : ASTBoxExpressionNode(Id, STL) {}

  virtual ~ASTBoxAsExpressionNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeBoxAs; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTBoxAsExpressionNode *ExpressionError(const std::string &ERM) {
    return new ASTBoxAsExpressionNode(ASTIdentifierNode::BoxAs.Clone(), ERM);
  }

  static ASTBoxAsExpressionNode *ExpressionError(const ASTIdentifierNode *Id,
                                                 const std::string &ERM) {
    return new ASTBoxAsExpressionNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<BoxAsExpression>" << std::endl;
    ASTBoxExpressionNode::print();
    std::cout << "</BoxAsExpression>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTBoxToExpressionNode : public ASTBoxExpressionNode {
private:
  uint64_t Duration;
  LengthUnit Units;

private:
  void ParseDuration(const std::string &Timing);
  const char *ParseUnits() const;
  ASTBoxToExpressionNode() = delete;

protected:
  ASTBoxToExpressionNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTBoxExpressionNode(Id, ERM), Duration(static_cast<uint64_t>(~0x0ULL)),
        Units(LengthUnspecified) {}

public:
  static const unsigned BoxToBits = 64U;

public:
  ASTBoxToExpressionNode(const ASTIdentifierNode *Id, const std::string &Timing,
                         const ASTStatementList &STL)
      : ASTBoxExpressionNode(Id, STL), Duration(static_cast<uint64_t>(~0x0ULL)),
        Units(LengthUnspecified) {
    ParseDuration(Timing);
  }

  virtual ~ASTBoxToExpressionNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeBoxTo; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual uint64_t GetDuration() const { return Duration; }

  virtual LengthUnit GetLengthUnit() const { return Units; }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTBoxToExpressionNode *ExpressionError(const std::string &ERM) {
    return new ASTBoxToExpressionNode(ASTIdentifierNode::BoxTo.Clone(), ERM);
  }

  static ASTBoxToExpressionNode *ExpressionError(const ASTIdentifierNode *Id,
                                                 const std::string &ERM) {
    return new ASTBoxToExpressionNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<BoxToExpression>" << std::endl;
    std::cout << "<Duration>" << Duration << "</Duration>" << std::endl;
    std::cout << "<LengthUnit>" << PrintLengthUnit(Units) << "</LengthUnit>"
              << std::endl;
    ASTBoxExpressionNode::print();
    std::cout << "</BoxToExpression>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTBoxStatementNode : public ASTStatementNode {
private:
  ASTBoxStatementNode() = delete;

protected:
  ASTBoxStatementNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)) {}

public:
  ASTBoxStatementNode(const ASTBoxExpressionNode *BXN)
      : ASTStatementNode(BXN->GetIdentifier(), BXN) {}

  ASTBoxStatementNode(const ASTBoxAsExpressionNode *BXN)
      : ASTStatementNode(BXN->GetIdentifier(), BXN) {}

  ASTBoxStatementNode(const ASTBoxToExpressionNode *BXN)
      : ASTStatementNode(BXN->GetIdentifier(), BXN) {}

  virtual ~ASTBoxStatementNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeBoxStatement; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual bool IsError() const override { return ASTStatementNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTBoxStatementNode *StatementError(const ASTIdentifierNode *Id,
                                             const std::string &ERM) {
    return new ASTBoxStatementNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<BoxStatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</BoxStatement>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_BOX_EXPRESSIONS_H
