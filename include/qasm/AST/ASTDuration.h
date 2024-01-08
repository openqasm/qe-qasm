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

#ifndef __QASM_AST_DURATION_NODE_H
#define __QASM_AST_DURATION_NODE_H

#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <string>
#include <vector>

namespace QASM {

class ASTDurationOfNode : public ASTStatementNode {
private:
  const ASTIdentifierNode *Target;
  std::vector<ASTOperandNode *> Args;
  ASTIdentifierList Qubits;
  const ASTGateQOpNode *QOP;

private:
  ASTDurationOfNode() = default;

protected:
  ASTDurationOfNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTStatementNode(
            Id, ASTExpressionNode::ExpressionError(Id, new ASTStringNode(ERM))),
        Target(nullptr), Args(), Qubits(), QOP(nullptr) {}

public:
  static const unsigned DurationOfBits = 64U;

public:
  ASTDurationOfNode(const ASTIdentifierNode *Id, const ASTIdentifierNode *TId)
      : ASTStatementNode(Id), Target(TId), Args(), Qubits(), QOP(nullptr) {}

  ASTDurationOfNode(const ASTIdentifierNode *Id, const ASTIdentifierNode *TId,
                    const ASTIdentifierList &QIL)
      : ASTStatementNode(Id), Target(TId), Args(), Qubits(QIL), QOP(nullptr) {}

  ASTDurationOfNode(const ASTIdentifierNode *Id, const ASTIdentifierNode *TId,
                    const ASTExpressionList &AIL, const ASTIdentifierList &QIL)
      : ASTStatementNode(Id), Target(TId), Args(), Qubits(QIL), QOP(nullptr) {
    for (ASTExpressionList::const_iterator AI = AIL.begin(); AI != AIL.end();
         ++AI) {
      assert((*AI) && "Invalid ASTExpression Operand!");
      if ((*AI)->GetASTType() == ASTTypeIdentifier) {
        if (const ASTIdentifierNode *AId =
                dynamic_cast<const ASTIdentifierNode *>(
                    (*AI)->GetIdentifier())) {
          Args.push_back(new ASTOperandNode(AId));
        }
      } else if (const ASTExpressionNode *AEx =
                     dynamic_cast<const ASTExpressionNode *>(*AI)) {
        Args.push_back(new ASTOperandNode(AEx));
      }
    }

    assert(Args.size() == AIL.Size() && "Inconsistent Arguments size!");
  }

  ASTDurationOfNode(const ASTIdentifierNode *Id, const ASTGateQOpNode *QOp)
      : ASTStatementNode(Id), Target(QOp->GetOperand()), Args(), Qubits(),
        QOP(QOp) {}

  virtual ~ASTDurationOfNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeDurationOf; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual void Mangle() override;

  virtual const std::string &GetName() const override {
    return ASTStatementNode::GetName();
  }

  virtual const std::string &GetMangledName() const override {
    return ASTStatementNode::GetMangledName();
  }

  virtual bool HasTarget() const { return Target != nullptr; }

  virtual bool HasOpTarget() const { return QOP != nullptr; }

  virtual bool HasArguments() const { return !Args.empty(); }

  virtual bool HasQubits() const { return !Qubits.Empty(); }

  const ASTIdentifierNode *GetTarget() const { return Target; }

  const ASTGateQOpNode *GetOpTarget() const { return QOP; }

  ASTGateQOpNode *GetOpTarget() { return const_cast<ASTGateQOpNode *>(QOP); }

  const ASTIdentifierList &GetQubits() const { return Qubits; }

  const std::vector<ASTOperandNode *> &GetArgs() const { return Args; }

  bool IsError() const override { return ASTStatementNode::IsError(); }

  const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTDurationOfNode *StatementError(const std::string &ERM) {
    return new ASTDurationOfNode(ASTIdentifierNode::DurationOf.Clone(), ERM);
  }

  static ASTDurationOfNode *StatementError(const ASTIdentifierNode *Id,
                                           const std::string &ERM) {
    return new ASTDurationOfNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<DurationOf>" << std::endl;
    std::cout << "<Identifier>" << GetName() << "</Identifier>" << std::endl;
    std::cout << "<TargetIdentifier>" << Target->GetName()
              << "</TargetIdentifier>" << std::endl;

    if (!Args.empty()) {
      for (std::vector<ASTOperandNode *>::const_iterator I = Args.begin();
           I != Args.end(); ++I) {
        if ((*I)->IsExpression())
          std::cout << "<Argument>"
                    << (*I)->GetExpression()->GetPolymorphicName()
                    << "</Argument>" << std::endl;
        else
          std::cout << "<Argument>"
                    << (*I)->GetIdentifier()->GetPolymorphicName()
                    << "</Argument>" << std::endl;
      }
    }

    if (!Qubits.Empty())
      for (ASTIdentifierList::const_iterator I = Qubits.begin();
           I != Qubits.end(); ++I)
        std::cout << "<Qubit>" << (*I)->GetName() << "</Qubit>" << std::endl;

    if (HasOpTarget())
      QOP->print();

    std::cout << "</DurationOf>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTDurationNode : public ASTExpressionNode {
  friend class ASTDurationArrayNode;

private:
  uint64_t Duration;
  LengthUnit Units;
  ASTType DTy;

  const ASTDurationOfNode *LO;
  const ASTBinaryOpNode *BOP;
  const ASTExpressionNode *EX;

private:
  ASTDurationNode() = delete;
  void ParseDuration(const std::string &Unit);
  const char *ParseUnits() const;

protected:
  ASTDurationNode(const ASTIdentifierNode *Id, const std::string &ERM,
                  ASTType ETy)
      : ASTExpressionNode(Id, ASTExpressionNode::ExpressionError(Id, ERM), ETy),
        Duration(static_cast<uint64_t>(~0x0UL)), Units(LengthUnspecified),
        DTy(ETy), LO(nullptr), BOP(nullptr), EX(nullptr) {}

  virtual void SetLengthUnit(LengthUnit LU) { Units = LU; }

  virtual void SetDuration(uint64_t D) { Duration = D; }

public:
  static const unsigned DurationBits = 64U;

public:
  ASTDurationNode(const ASTIdentifierNode *Id, const std::string &Unit)
      : ASTExpressionNode(Id, ASTTypeDuration),
        Duration(static_cast<uint64_t>(~0x0UL)), Units(LengthUnspecified),
        DTy(ASTTypeDuration), LO(nullptr), BOP(nullptr), EX(nullptr) {
    ParseDuration(Unit);
  }

  ASTDurationNode(const ASTIdentifierNode *Id, uint64_t D, LengthUnit U)
      : ASTExpressionNode(Id, ASTTypeDuration), Duration(D), Units(U),
        DTy(ASTTypeDuration), LO(nullptr), BOP(nullptr), EX(nullptr) {}

  ASTDurationNode(const ASTIdentifierNode *Id, const ASTDurationOfNode *LON)
      : ASTExpressionNode(Id, ASTTypeDuration), Duration(0UL),
        Units(LengthOfDependent), DTy(LON->GetASTType()), LO(LON),
        BOP(nullptr) {}

  ASTDurationNode(const ASTIdentifierNode *Id, const ASTBinaryOpNode *BOp)
      : ASTExpressionNode(Id, ASTTypeDuration),
        Duration(static_cast<uint64_t>(~0x0UL)), Units(BinaryOpDependent),
        DTy(BOp->GetASTType()), LO(nullptr), BOP(BOp), EX(nullptr) {}

  ASTDurationNode(const ASTIdentifierNode *Id, const ASTExpressionNode *EXP)
      : ASTExpressionNode(Id, ASTTypeDuration),
        Duration(static_cast<uint64_t>(~0x0UL)), Units(FunctionCallDependent),
        DTy(EXP->GetASTType()), LO(nullptr), BOP(nullptr), EX(EXP) {}

  virtual ~ASTDurationNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeDuration; }

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

  virtual const std::string &GetMangledName() const override {
    return ASTExpressionNode::Ident->GetMangledName();
  }

  virtual uint64_t GetDuration() const { return Duration; }

  virtual LengthUnit GetLengthUnit() const { return Units; }

  virtual ASTType GetDurationType() const { return DTy; }

  virtual bool IsDurationOf() const { return DTy == ASTTypeDurationOf; }

  virtual bool IsDuration() const { return DTy == ASTTypeDuration; }

  virtual std::string AsString() const;

  virtual const ASTDurationOfNode *GetOperand() const { return LO; }

  virtual bool HasOperand() const { return LO != nullptr; }

  virtual bool HasFunctionCall() const { return EX != nullptr; }

  virtual ASTDurationOfNode *GetOperand() {
    return const_cast<ASTDurationOfNode *>(LO);
  }

  virtual const ASTBinaryOpNode *GetBinaryOp() const { return BOP; }

  virtual ASTBinaryOpNode *GetBinaryOp() {
    return const_cast<ASTBinaryOpNode *>(BOP);
  }

  virtual const ASTExpressionNode *GetFunctionCall() const { return EX; }

  virtual ASTExpressionNode *GetFunctionCall() {
    return const_cast<ASTExpressionNode *>(EX);
  }

  virtual bool IsIntegerConstantExpression() const override { return true; }

  virtual bool IsBinaryOp() const { return BOP != nullptr; }

  virtual unsigned GetBits() const { return ASTDurationNode::DurationBits; }

  virtual bool IsError() const override {
    return DTy == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTDurationNode *ExpressionError(const ASTIdentifierNode *Id,
                                          const std::string &ERM) {
    return new ASTDurationNode(Id, ERM, ASTTypeExpressionError);
  }

  static ASTDurationNode *ExpressionError(const std::string &ERM) {
    return new ASTDurationNode(ASTIdentifierNode::Duration.Clone(), ERM,
                               ASTTypeExpressionError);
  }

  virtual void print() const override {
    std::cout << "<Duration>" << std::endl;
    std::cout << "<Identifier>" << GetName() << "</Identifier>" << std::endl;

    if (LO)
      LO->print();
    if (BOP)
      BOP->print();

    std::cout << "<Duration>" << Duration << "</Duration>" << std::endl;
    std::cout << "<LengthUnit>" << PrintLengthUnit(Units) << "</LengthUnit>"
              << std::endl;
    std::cout << "</Duration>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_DURATION_NODE_H
