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

#ifndef __QASM_AST_ROTATE_H
#define __QASM_AST_ROTATE_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTCBit.h>

#include <iostream>
#include <limits>
#include <cassert>

namespace QASM {

template<typename __Type>
__Type rotr(__Type X, int S) noexcept;

template<typename __Type>
__Type rotl(__Type X, int S) noexcept {
  static_assert(std::is_integral<__Type>::value,
                "attempting rotl of non-integer type!");
  static_assert(!std::is_signed<__Type>::value,
                "attempting rotl of signed integer type!");

  if (S == std::numeric_limits<__Type>::digits || S == 0) {
    return X;
  } else if (S < 0) {
    return rotr(X, -S);
  } else {
    __Type T = X;
    X <<= S;
    X |= T >> (std::numeric_limits<__Type>::digits - S);
  }

  return X;
}

template<typename __Type>
__Type rotr(__Type X, int S) noexcept {
  static_assert(std::is_integral<__Type>::value,
                "attempting rotr of non-integer type!");
  static_assert(!std::is_signed<__Type>::value,
                "attempting rotr of signed integer type!");

  if (S == std::numeric_limits<__Type>::digits || S == 0) {
    return X;
  } else if (S < 0) {
    return rotl(X, -S);
  } else {
    __Type T = X;
    X >>= S;
    X |= T << (std::numeric_limits<__Type>::digits - S);
  }

  return X;
}

class ASTRotateNode : public ASTExpressionNode {
private:
  ASTRotationType OpType;
  int32_t S;

  union {
    const ASTIntNode* I;
    const ASTMPIntegerNode* MPI;
    const ASTCBitNode* CBI;
  };

  ASTType IType;

private:
  ASTRotateNode() = delete;

protected:
  ASTRotateNode(const ASTIdentifierNode* Id, const std::string& ERM)
  : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
  OpType(ASTRotationTypeUnknown), S(0), I(nullptr),
  IType(ASTTypeExpressionError) { }

public:
  ASTRotateNode(const ASTIdentifierNode* Id,
                const ASTIntNode* Target,
                const ASTIntNode* Shift,
                ASTRotationType RoTy)
  : ASTExpressionNode(Id, ASTTypeRotateExpr), OpType(RoTy),
  S(Shift->GetSignedValue()), I(Target), IType(ASTTypeInt) {
    assert(!Target->IsSigned() && "attempting rotate of signed integer type!");
  }

  ASTRotateNode(const ASTIdentifierNode* Id,
                const ASTMPIntegerNode* Target,
                const ASTIntNode* Shift,
                ASTRotationType RoTy)
  : ASTExpressionNode(Id, ASTTypeRotateExpr), OpType(RoTy),
  S(Shift->GetSignedValue()), MPI(Target), IType(ASTTypeMPInteger) {
    assert(!Target->IsSigned() && "attempting rotate of signed integer type!");
  }

  ASTRotateNode(const ASTIdentifierNode* Id,
                const ASTCBitNode* Target,
                const ASTIntNode* Shift,
                ASTRotationType RoTy)
  : ASTExpressionNode(Id, ASTTypeRotateExpr), OpType(RoTy),
  S(Shift->GetSignedValue()), CBI(Target), IType(ASTTypeBitset) { }

  virtual ~ASTRotateNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeRotateExpr;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  // Implemented in ASTTypes.cpp.
  virtual void Mangle() override;

  virtual const ASTIdentifierNode* GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string& GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual ASTType GetTargetType() const {
    return IType;
  }

  virtual ASTRotationType GetRotationType() const {
    return OpType;
  }

  virtual unsigned GetBits() const {
    switch (IType) {
    case ASTTypeInt:
    case ASTTypeUInt:
      return I->GetBits();
      break;
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
      return MPI->GetBits();
      break;
    case ASTTypeBitset:
      return CBI->GetBits();
      break;
    default:
      break;
    }

    return 0U;
  }

  virtual const ASTIntNode* GetIntegerTarget() const {
    return IType == ASTTypeInt ? I : nullptr;
  }

  virtual const ASTMPIntegerNode* GetMPIntegerTarget() const {
    return IType == ASTTypeMPInteger ? MPI : nullptr;
  }

  virtual const ASTCBitNode* GetCBitTarget() const {
    return IType == ASTTypeBitset ? CBI : nullptr;
  }

  virtual int32_t GetShiftValue() const {
    return S;
  }

  virtual bool IsError() const override {
    return IType == ASTTypeExpressionError;
  }

  virtual const std::string& GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTRotateNode* ExpressionError(const std::string& ERM,
                                        ASTRotationType RoTy) {
    switch (RoTy) {
    case ASTRotationTypeLeft:
      return new ASTRotateNode(ASTIdentifierNode::Rotl.Clone(), ERM);
      break;
    case ASTRotationTypeRight:
      return new ASTRotateNode(ASTIdentifierNode::Rotr.Clone(), ERM);
      break;
    default:
      return new ASTRotateNode(ASTIdentifierNode::Rotate.Clone(), ERM);
      break;
    }
  }

  virtual void print() const override {
    std::cout << "<RotationNode>" << std::endl;
    std::cout << "<RotationType>" << PrintRotationType(OpType)
      << "</RotationType>" << std::endl;
    std::cout << "<Shift>" << S << "</Shift>" << std::endl;
    std::cout << "<Target>" << std::endl;
    if (IType == ASTTypeInt)
      I->print();
    else if (IType == ASTTypeMPInteger)
      MPI->print();
    else if (IType == ASTTypeBitset)
      CBI->print();
    std::cout << "</Target>" << std::endl;
    std::cout << "</RotationNode>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTRotateStatementNode : public ASTStatementNode {
private:
  ASTRotateStatementNode() = delete;

protected:
  ASTRotateStatementNode(const ASTIdentifierNode* Id, const std::string& ERM)
  : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)) { }

public:
  ASTRotateStatementNode(const ASTRotateNode* RN)
  : ASTStatementNode(RN->GetIdentifier(), RN) { }

  virtual ~ASTRotateStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeRotateStatement;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual const ASTRotateNode* GetRotateNode() const {
    return dynamic_cast<const ASTRotateNode*>(ASTStatementNode::GetExpression());
  }

  virtual bool IsError() const override {
    return ASTStatementNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTRotateStatementNode* StatementError(const ASTIdentifierNode* Id,
                                                const std::string& ERM) {
    return new ASTRotateStatementNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<RotateStatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</RotateStatement>" << std::endl;
  }

  virtual void push(ASTBase* /* unused*/) override { }
};

} // namespace QASM

#endif // __QASM_AST_ROTATE_H

