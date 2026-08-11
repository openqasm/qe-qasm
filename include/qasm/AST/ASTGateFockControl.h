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

#ifndef __QASM_AST_GATE_FOCK_CONTROL_H
#define __QASM_AST_GATE_FOCK_CONTROL_H

#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <iostream>

namespace QASM {

/// Fock-level control: `ctrl[level] @ target` (int / id / binary / unary).
/// Distinct from qubit n-control
/// (`ctrl` / `ctrl(n)` → ASTGateControlNode::CBits).
class ASTGateFockControlNode : public ASTExpressionNode {
private:
  union {
    const ASTIntNode *I;
    const ASTIdentifierNode *ID;
    const ASTBinaryOpNode *BOP;
    const ASTUnaryOpNode *UOP;
  };
  ASTType LevelType;

  union {
    const ASTGateNode *GN;
    const ASTGateQOpNode *GQN;
    const ASTGPhaseExpressionNode *GPN;
    const ASTGateGPhaseExpressionNode *GGEN;
  };
  ASTType TType;

private:
  ASTGateFockControlNode() = delete;

protected:
  ASTGateFockControlNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        I(nullptr), LevelType(ASTTypeExpressionError), GN(nullptr),
        TType(ASTTypeExpressionError) {}

  void InitTargetGate(const ASTGateNode *N) {
    GN = N;
    TType = ASTTypeGate;
  }
  void InitTargetQOp(const ASTGateQOpNode *QN) {
    GQN = QN;
    TType = ASTTypeGateQOpNode;
  }
  void InitTargetGPhase(const ASTGPhaseExpressionNode *N) {
    GPN = N;
    TType = ASTTypeGPhaseExpression;
  }
  void InitTargetGateGPhase(const ASTGateGPhaseExpressionNode *N) {
    GGEN = N;
    TType = ASTTypeGateGPhaseExpression;
  }

public:
  ASTGateFockControlNode(const ASTGateNode *N, const ASTIntNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateFockControl),
        I(Lv), LevelType(ASTTypeInt), GN(N), TType(ASTTypeGate) {
    assert(N && Lv);
  }
  ASTGateFockControlNode(const ASTGateNode *N, const ASTIdentifierNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateFockControl),
        ID(Lv), LevelType(ASTTypeIdentifier), GN(N), TType(ASTTypeGate) {
    assert(N && Lv);
  }
  ASTGateFockControlNode(const ASTGateNode *N, const ASTBinaryOpNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateFockControl),
        BOP(Lv), LevelType(ASTTypeBinaryOp), GN(N), TType(ASTTypeGate) {
    assert(N && Lv);
  }
  ASTGateFockControlNode(const ASTGateNode *N, const ASTUnaryOpNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateFockControl),
        UOP(Lv), LevelType(ASTTypeUnaryOp), GN(N), TType(ASTTypeGate) {
    assert(N && Lv);
  }

  ASTGateFockControlNode(const ASTGateQOpNode *QN, const ASTIntNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateFockControl),
        I(Lv), LevelType(ASTTypeInt), GQN(QN), TType(ASTTypeGateQOpNode) {
    assert(QN && Lv);
  }
  ASTGateFockControlNode(const ASTGateQOpNode *QN, const ASTIdentifierNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateFockControl),
        ID(Lv), LevelType(ASTTypeIdentifier), GQN(QN),
        TType(ASTTypeGateQOpNode) {
    assert(QN && Lv);
  }
  ASTGateFockControlNode(const ASTGateQOpNode *QN, const ASTBinaryOpNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateFockControl),
        BOP(Lv), LevelType(ASTTypeBinaryOp), GQN(QN),
        TType(ASTTypeGateQOpNode) {
    assert(QN && Lv);
  }
  ASTGateFockControlNode(const ASTGateQOpNode *QN, const ASTUnaryOpNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateFockControl),
        UOP(Lv), LevelType(ASTTypeUnaryOp), GQN(QN), TType(ASTTypeGateQOpNode) {
    assert(QN && Lv);
  }

  ASTGateFockControlNode(const ASTGateGPhaseExpressionNode *N,
                         const ASTIntNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateFockControl),
        I(Lv), LevelType(ASTTypeInt), GGEN(N),
        TType(ASTTypeGateGPhaseExpression) {
    assert(N && Lv);
  }
  ASTGateFockControlNode(const ASTGateGPhaseExpressionNode *N,
                         const ASTIdentifierNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateFockControl),
        ID(Lv), LevelType(ASTTypeIdentifier), GGEN(N),
        TType(ASTTypeGateGPhaseExpression) {
    assert(N && Lv);
  }
  ASTGateFockControlNode(const ASTGateGPhaseExpressionNode *N,
                         const ASTBinaryOpNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateFockControl),
        BOP(Lv), LevelType(ASTTypeBinaryOp), GGEN(N),
        TType(ASTTypeGateGPhaseExpression) {
    assert(N && Lv);
  }
  ASTGateFockControlNode(const ASTGateGPhaseExpressionNode *N,
                         const ASTUnaryOpNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateFockControl),
        UOP(Lv), LevelType(ASTTypeUnaryOp), GGEN(N),
        TType(ASTTypeGateGPhaseExpression) {
    assert(N && Lv);
  }

  virtual ~ASTGateFockControlNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGateFockControl; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetTargetType() const { return TType; }
  virtual ASTType GetLevelType() const { return LevelType; }

  virtual const ASTIntNode *GetLevelInt() const {
    return LevelType == ASTTypeInt ? I : nullptr;
  }
  virtual const ASTIdentifierNode *GetLevelIdentifier() const {
    return LevelType == ASTTypeIdentifier ? ID : nullptr;
  }
  virtual const ASTBinaryOpNode *GetLevelBinaryOp() const {
    return LevelType == ASTTypeBinaryOp ? BOP : nullptr;
  }
  virtual const ASTUnaryOpNode *GetLevelUnaryOp() const {
    return LevelType == ASTTypeUnaryOp ? UOP : nullptr;
  }

  virtual const ASTGateNode *GetGateNode() const {
    return TType == ASTTypeGate ? GN : nullptr;
  }
  virtual const ASTGateQOpNode *GetGateQOpNode() const {
    return TType == ASTTypeGateQOpNode ? GQN : nullptr;
  }
  virtual const ASTGateGPhaseExpressionNode *GetGateGPhaseNode() const {
    return TType == ASTTypeGateGPhaseExpression ? GGEN : nullptr;
  }

  virtual bool IsError() const override {
    return LevelType == ASTTypeExpressionError ||
           TType == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTGateFockControlNode *ExpressionError(const std::string &ERM) {
    return new ASTGateFockControlNode(ASTIdentifierNode::BadCtrl.Clone(), ERM);
  }

  virtual void print() const override;
  virtual void push(ASTBase * /* unused */) override {}
};

/// Fock-level negative control: `negctrl[level] @ target`.
class ASTGateFockNegControlNode : public ASTExpressionNode {
private:
  union {
    const ASTIntNode *I;
    const ASTIdentifierNode *ID;
    const ASTBinaryOpNode *BOP;
    const ASTUnaryOpNode *UOP;
  };
  ASTType LevelType;

  union {
    const ASTGateNode *GN;
    const ASTGateQOpNode *GQN;
    const ASTGPhaseExpressionNode *GPN;
    const ASTGateGPhaseExpressionNode *GGEN;
  };
  ASTType TType;

private:
  ASTGateFockNegControlNode() = delete;

protected:
  ASTGateFockNegControlNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        I(nullptr), LevelType(ASTTypeExpressionError), GN(nullptr),
        TType(ASTTypeExpressionError) {}

public:
  ASTGateFockNegControlNode(const ASTGateNode *N, const ASTIntNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::NegCtrl,
                          ASTTypeGateFockNegControl),
        I(Lv), LevelType(ASTTypeInt), GN(N), TType(ASTTypeGate) {
    assert(N && Lv);
  }
  ASTGateFockNegControlNode(const ASTGateNode *N, const ASTIdentifierNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::NegCtrl,
                          ASTTypeGateFockNegControl),
        ID(Lv), LevelType(ASTTypeIdentifier), GN(N), TType(ASTTypeGate) {
    assert(N && Lv);
  }
  ASTGateFockNegControlNode(const ASTGateNode *N, const ASTBinaryOpNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::NegCtrl,
                          ASTTypeGateFockNegControl),
        BOP(Lv), LevelType(ASTTypeBinaryOp), GN(N), TType(ASTTypeGate) {
    assert(N && Lv);
  }
  ASTGateFockNegControlNode(const ASTGateNode *N, const ASTUnaryOpNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::NegCtrl,
                          ASTTypeGateFockNegControl),
        UOP(Lv), LevelType(ASTTypeUnaryOp), GN(N), TType(ASTTypeGate) {
    assert(N && Lv);
  }

  ASTGateFockNegControlNode(const ASTGateQOpNode *QN, const ASTIntNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::NegCtrl,
                          ASTTypeGateFockNegControl),
        I(Lv), LevelType(ASTTypeInt), GQN(QN), TType(ASTTypeGateQOpNode) {
    assert(QN && Lv);
  }
  ASTGateFockNegControlNode(const ASTGateQOpNode *QN,
                            const ASTIdentifierNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::NegCtrl,
                          ASTTypeGateFockNegControl),
        ID(Lv), LevelType(ASTTypeIdentifier), GQN(QN),
        TType(ASTTypeGateQOpNode) {
    assert(QN && Lv);
  }
  ASTGateFockNegControlNode(const ASTGateQOpNode *QN, const ASTBinaryOpNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::NegCtrl,
                          ASTTypeGateFockNegControl),
        BOP(Lv), LevelType(ASTTypeBinaryOp), GQN(QN),
        TType(ASTTypeGateQOpNode) {
    assert(QN && Lv);
  }
  ASTGateFockNegControlNode(const ASTGateQOpNode *QN, const ASTUnaryOpNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::NegCtrl,
                          ASTTypeGateFockNegControl),
        UOP(Lv), LevelType(ASTTypeUnaryOp), GQN(QN), TType(ASTTypeGateQOpNode) {
    assert(QN && Lv);
  }

  ASTGateFockNegControlNode(const ASTGateGPhaseExpressionNode *N,
                            const ASTIntNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::NegCtrl,
                          ASTTypeGateFockNegControl),
        I(Lv), LevelType(ASTTypeInt), GGEN(N),
        TType(ASTTypeGateGPhaseExpression) {
    assert(N && Lv);
  }
  ASTGateFockNegControlNode(const ASTGateGPhaseExpressionNode *N,
                            const ASTIdentifierNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::NegCtrl,
                          ASTTypeGateFockNegControl),
        ID(Lv), LevelType(ASTTypeIdentifier), GGEN(N),
        TType(ASTTypeGateGPhaseExpression) {
    assert(N && Lv);
  }
  ASTGateFockNegControlNode(const ASTGateGPhaseExpressionNode *N,
                            const ASTBinaryOpNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::NegCtrl,
                          ASTTypeGateFockNegControl),
        BOP(Lv), LevelType(ASTTypeBinaryOp), GGEN(N),
        TType(ASTTypeGateGPhaseExpression) {
    assert(N && Lv);
  }
  ASTGateFockNegControlNode(const ASTGateGPhaseExpressionNode *N,
                            const ASTUnaryOpNode *Lv)
      : ASTExpressionNode(&ASTIdentifierNode::NegCtrl,
                          ASTTypeGateFockNegControl),
        UOP(Lv), LevelType(ASTTypeUnaryOp), GGEN(N),
        TType(ASTTypeGateGPhaseExpression) {
    assert(N && Lv);
  }

  virtual ~ASTGateFockNegControlNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeGateFockNegControl;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetTargetType() const { return TType; }
  virtual ASTType GetLevelType() const { return LevelType; }

  virtual const ASTIntNode *GetLevelInt() const {
    return LevelType == ASTTypeInt ? I : nullptr;
  }
  virtual const ASTIdentifierNode *GetLevelIdentifier() const {
    return LevelType == ASTTypeIdentifier ? ID : nullptr;
  }
  virtual const ASTBinaryOpNode *GetLevelBinaryOp() const {
    return LevelType == ASTTypeBinaryOp ? BOP : nullptr;
  }
  virtual const ASTUnaryOpNode *GetLevelUnaryOp() const {
    return LevelType == ASTTypeUnaryOp ? UOP : nullptr;
  }

  virtual const ASTGateNode *GetGateNode() const {
    return TType == ASTTypeGate ? GN : nullptr;
  }
  virtual const ASTGateQOpNode *GetGateQOpNode() const {
    return TType == ASTTypeGateQOpNode ? GQN : nullptr;
  }
  virtual const ASTGateGPhaseExpressionNode *GetGateGPhaseNode() const {
    return TType == ASTTypeGateGPhaseExpression ? GGEN : nullptr;
  }

  virtual bool IsError() const override {
    return LevelType == ASTTypeExpressionError ||
           TType == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTGateFockNegControlNode *ExpressionError(const std::string &ERM) {
    return new ASTGateFockNegControlNode(ASTIdentifierNode::BadCtrl.Clone(),
                                         ERM);
  }

  virtual void print() const override;
  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_GATE_FOCK_CONTROL_H
