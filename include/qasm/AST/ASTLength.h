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

#ifndef __QASM_AST_LENGTH_NODE_H
#define __QASM_AST_LENGTH_NODE_H

#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <string>

namespace QASM {

class ASTLengthOfNode : public ASTStatementNode {
private:
  const ASTIdentifierNode *Target;

private:
  ASTLengthOfNode() = default;

public:
  static const unsigned LengthOfBits = 64U;

public:
  ASTLengthOfNode(const ASTIdentifierNode *Id, const ASTIdentifierNode *TId)
      : ASTStatementNode(Id), Target(TId) {}

  virtual ~ASTLengthOfNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeLengthOf; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual void Mangle() override;

  virtual const std::string &GetName() const override {
    return ASTStatementNode::Ident->GetName();
  }

  const ASTIdentifierNode *GetTarget() const { return Target; }

  virtual void print() const override {
    std::cout << "<LengthOf>" << std::endl;
    std::cout << "<Identifier>" << GetName() << "</Identifier>" << std::endl;
    std::cout << "<TargetIdentifier>" << Target->GetName()
              << "</TargetIdentifier>" << std::endl;
    std::cout << "</LengthOf>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTLengthNode : public ASTExpressionNode {
private:
  uint64_t Duration;
  LengthUnit Units;
  const ASTLengthOfNode *LO;
  const ASTBinaryOpNode *BOP;

private:
  ASTLengthNode() = delete;
  void ParseDuration(const std::string &Unit);

public:
  static const unsigned LengthBits = 64U;

public:
  ASTLengthNode(const ASTIdentifierNode *Id, const std::string &Unit)
      : ASTExpressionNode(Id, ASTTypeLength),
        Duration(static_cast<uint64_t>(~0x0UL)), Units(LengthUnspecified),
        LO(nullptr), BOP(nullptr) {
    ParseDuration(Unit);
  }

  ASTLengthNode(const ASTIdentifierNode *Id, uint64_t D, LengthUnit U)
      : ASTExpressionNode(Id, ASTTypeLength), Duration(D), Units(U),
        LO(nullptr), BOP(nullptr) {}

  ASTLengthNode(const ASTIdentifierNode *Id, const ASTLengthOfNode *LON)
      : ASTExpressionNode(Id, ASTTypeLength), Duration(0UL),
        Units(LengthOfDependent), LO(LON), BOP(nullptr) {}

  ASTLengthNode(const ASTIdentifierNode *Id, const ASTBinaryOpNode *BOp)
      : ASTExpressionNode(Id, ASTTypeLength),
        Duration(static_cast<uint64_t>(~0x0UL)), Units(BinaryOpDependent),
        LO(nullptr), BOP(BOp) {}

  virtual ~ASTLengthNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeLength; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual uint64_t GetDuration() const { return Duration; }

  virtual LengthUnit GetLengthUnit() const { return Units; }

  virtual std::string AsString() const;

  virtual const ASTLengthOfNode *GetOperand() const { return LO; }

  virtual ASTLengthOfNode *GetOperand() {
    return const_cast<ASTLengthOfNode *>(LO);
  }

  virtual const ASTBinaryOpNode *GetBinaryOp() const { return BOP; }

  virtual ASTBinaryOpNode *GetBinaryOp() {
    return const_cast<ASTBinaryOpNode *>(BOP);
  }

  virtual bool IsIntegerConstantExpression() const override { return true; }

  virtual bool IsBinaryOp() const { return BOP != nullptr; }

  virtual unsigned GetBits() const { return ASTLengthNode::LengthBits; }

  virtual void print() const override {
    std::cout << "<Length>" << std::endl;
    std::cout << "<Identifier>" << GetName() << "</Identifier>" << std::endl;

    if (LO)
      LO->print();
    if (BOP)
      BOP->print();

    std::cout << "<Duration>" << Duration << "</Duration>" << std::endl;
    std::cout << "<LengthUnit>" << PrintLengthUnit(Units) << "</LengthUnit>"
              << std::endl;
    std::cout << "</Length>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_LENGTH_NODE_H
