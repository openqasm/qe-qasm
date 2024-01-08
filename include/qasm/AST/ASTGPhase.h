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

#ifndef __QASM_AST_GPHASE_H
#define __QASM_AST_GPHASE_H

#include <qasm/AST/ASTTypes.h>

namespace QASM {

class ASTGPhaseExpressionNode : public ASTExpressionNode {
private:
  union {
    mutable const ASTIdentifierNode *Id;
    mutable const ASTBinaryOpNode *BOP;
    mutable const ASTUnaryOpNode *UOP;
  };

  ASTType GT;

private:
  ASTGPhaseExpressionNode() = delete;

protected:
  ASTGPhaseExpressionNode(const std::string &ERM)
      : ASTExpressionNode(ASTIdentifierNode::GPhase.Clone(),
                          new ASTStringNode(ERM), ASTTypeExpressionError),
        Id(nullptr), GT(ASTTypeExpressionError) {}

public:
  static const unsigned GPhaseBits = 64U;

public:
  ASTGPhaseExpressionNode(const ASTIdentifierNode *GId,
                          const ASTIdentifierNode *TId)
      : ASTExpressionNode(GId, ASTTypeGPhaseExpression), Id(TId),
        GT(TId->GetSymbolType()) {}

  ASTGPhaseExpressionNode(const ASTIdentifierNode *GId,
                          const ASTBinaryOpNode *BOp)
      : ASTExpressionNode(GId, ASTTypeGPhaseExpression), BOP(BOp),
        GT(ASTTypeBinaryOp) {}

  ASTGPhaseExpressionNode(const ASTIdentifierNode *GId,
                          const ASTUnaryOpNode *UOp)
      : ASTExpressionNode(GId, ASTTypeGPhaseExpression), UOP(UOp),
        GT(ASTTypeUnaryOp) {}

  virtual ~ASTGPhaseExpressionNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeGPhaseExpression;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTType GetTargetType() const { return GT; }

  // Implemented in ASTGateOps.cpp.
  virtual void Mangle() override;

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual const ASTIdentifierNode *GetTargetIdentifier() const {
    return GT == ASTTypeIdentifier ? Id : nullptr;
  }

  virtual const ASTBinaryOpNode *GetBinaryOp() const {
    return GT == ASTTypeBinaryOp ? BOP : nullptr;
  }

  virtual ASTBinaryOpNode *GetBinaryOp() {
    return GT == ASTTypeBinaryOp ? const_cast<ASTBinaryOpNode *>(BOP) : nullptr;
  }

  virtual const ASTUnaryOpNode *GetUnaryOp() const {
    return GT == ASTTypeUnaryOp ? UOP : nullptr;
  }

  virtual ASTUnaryOpNode *GetUnaryOp() {
    return GT == ASTTypeUnaryOp ? const_cast<ASTUnaryOpNode *>(UOP) : nullptr;
  }

  virtual bool IsError() const override { return GT == ASTTypeExpressionError; }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTGPhaseExpressionNode *ExpressionError(const std::string &ERM) {
    return new ASTGPhaseExpressionNode(ERM);
  }

  virtual void print() const override {
    std::cout << "<GPhaseExpression>" << std::endl;
    ASTExpressionNode::print();

    switch (GT) {
    case ASTTypeIdentifier:
      Id->print();
      break;
    case ASTTypeAngle:
      Id->print();
      break;
    case ASTTypeBinaryOp:
      BOP->print();
      break;
    case ASTTypeUnaryOp:
      UOP->print();
      break;
    case ASTTypeExpressionError:
      std::cout << "<Error>" << GetError() << "</Error>" << std::endl;
      break;
    default:
      break;
    }

    std::cout << "</GPhaseExpression>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTGateGPhaseExpressionNode : public ASTExpressionNode {
private:
  union {
    const ASTGateQOpNode *GQN;
    const ASTGPhaseExpressionNode *GPE;
    const ASTGateControlNode *CN;
    const ASTGateNegControlNode *NCN;
    const ASTGatePowerNode *PN;
    const ASTGateInverseNode *IN;
  };

  ASTType TType;
  ASTIdentifierList IL;

private:
  ASTGateGPhaseExpressionNode() = delete;

protected:
  ASTGateGPhaseExpressionNode(const std::string &ERM)
      : ASTExpressionNode(ASTIdentifierNode::BadGPhase.Clone(),
                          new ASTStringNode(ERM), ASTTypeExpressionError),
        GQN(nullptr), TType(ASTTypeExpressionError), IL() {}

public:
  static const unsigned GateGPhaseBits = 64U;

public:
  ASTGateGPhaseExpressionNode(const ASTGPhaseExpressionNode *N,
                              const ASTIdentifierList &L)
      : ASTExpressionNode(N->GetIdentifier(), ASTTypeGateGPhaseExpression),
        GPE(N), TType(N->GetASTType()), IL(L) {}

  ASTGateGPhaseExpressionNode(const ASTGateControlNode *N,
                              const ASTIdentifierList &L)
      : ASTExpressionNode(N->GetIdentifier(), ASTTypeGateGPhaseExpression),
        CN(N), TType(N->GetASTType()), IL(L) {}

  ASTGateGPhaseExpressionNode(const ASTGateNegControlNode *N,
                              const ASTIdentifierList &L)
      : ASTExpressionNode(N->GetIdentifier(), ASTTypeGateGPhaseExpression),
        NCN(N), TType(N->GetASTType()), IL(L) {}

  ASTGateGPhaseExpressionNode(const ASTGateInverseNode *N,
                              const ASTIdentifierList &L)
      : ASTExpressionNode(N->GetIdentifier(), ASTTypeGateGPhaseExpression),
        IN(N), TType(N->GetASTType()), IL(L) {}

  ASTGateGPhaseExpressionNode(const ASTGatePowerNode *N,
                              const ASTIdentifierList &L)
      : ASTExpressionNode(N->GetIdentifier(), ASTTypeGateGPhaseExpression),
        PN(N), TType(N->GetASTType()), IL(L) {}

  ASTGateGPhaseExpressionNode(const ASTGateQOpNode *N,
                              const ASTIdentifierList &L)
      : ASTExpressionNode(N->GetIdentifier(), ASTTypeGateGPhaseExpression),
        GQN(N), TType(N->GetASTType()), IL(L) {}

  virtual ~ASTGateGPhaseExpressionNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeGateGPhaseExpression;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTType GetTargetType() const { return TType; }

  // Implemented in ASTGateOps.cpp.
  virtual void Mangle() override;

  virtual const ASTGPhaseExpressionNode *GetGPhaseExpression() const {
    return GPE;
  }

  virtual const ASTIdentifierList &GetIdentifierList() const { return IL; }

  virtual bool IsError() const override {
    return TType == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTGateGPhaseExpressionNode *ExpressionError(const std::string &ERM) {
    return new ASTGateGPhaseExpressionNode(ERM);
  }

  virtual void print() const override {
    std::cout << "<GateControlExpression>" << std::endl;

    switch (TType) {
    case ASTTypeGPhaseExpression:
      GPE->print();
      break;
    case ASTTypeGateControl:
      CN->print();
      break;
    case ASTTypeGateNegControl:
      NCN->print();
      break;
    case ASTTypeGateInverse:
      IN->print();
      break;
    case ASTTypeGatePower:
      PN->print();
      break;
    case ASTTypeGateQOpNode:
      GQN->print();
      break;
    default:
      break;
    }

    IL.print();
    std::cout << "</GateControlExpression>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTGPhaseStatementNode : public ASTStatementNode {
private:
  ASTGPhaseStatementNode() = delete;

public:
  static const unsigned GPhaseStmtBits = 64U;

public:
  ASTGPhaseStatementNode(const ASTGPhaseExpressionNode *GPE)
      : ASTStatementNode(GPE) {}

  virtual ~ASTGPhaseStatementNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGPhaseStatement; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual void print() const override {
    std::cout << "<GPhaseStatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</GPhaseStatement>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_GPHASE_H
