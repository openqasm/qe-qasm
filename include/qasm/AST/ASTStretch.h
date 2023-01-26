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

#ifndef __QASM_AST_STRETCH_H
#define __QASM_AST_STRETCH_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTIdentifier.h>

namespace QASM {

class ASTStretchNode : public ASTExpressionNode {
private:
  uint64_t Duration;
  LengthUnit Units;

  union {
    const ASTVoidNode* Void;
    const ASTIntNode* I;
    const ASTFloatNode* F;
    const ASTDoubleNode* DBL;
    const ASTMPIntegerNode* MPI;
    const ASTMPDecimalNode* MPD;
    const ASTBinaryOpNode* BOP;
    const ASTUnaryOpNode* UOP;
  };

  ASTType VType;

private:
  ASTStretchNode() = delete;

private:
  void ParseDuration(const std::string& Unit);
  const char* ParseUnits() const;

protected:
  ASTStretchNode(const ASTIdentifierNode* Id, const std::string& ERM,
                 ASTType VTy)
  : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
  Duration(static_cast<uint64_t>(~0x0UL)), Units(LengthUnspecified),
  Void(nullptr), VType(VTy) { }

public:
  static const unsigned StretchBits = 64U;
  static const unsigned MaxValue = 255U;

public:
  ASTStretchNode(const ASTIdentifierNode* Id)
  : ASTExpressionNode(Id, ASTTypeStretch), Duration(0UL),
  Units(LengthUnspecified), Void(nullptr), VType(ASTTypeUndefined) { }

  ASTStretchNode(const ASTIdentifierNode* Id, uint64_t D, LengthUnit LU)
  : ASTExpressionNode(Id, ASTTypeStretch), Duration(D), Units(LU),
  VType(ASTTypeUndefined) { }

  ASTStretchNode(const ASTIdentifierNode* Id, const std::string& LU)
  : ASTExpressionNode(Id, ASTTypeStretch), Duration(0UL),
  Units(LengthUnspecified), Void(nullptr), VType(ASTTypeUndefined) {
    ParseDuration(LU);
  }

  ASTStretchNode(const ASTIdentifierNode* Id, const ASTIntNode* II)
  : ASTExpressionNode(Id, ASTTypeStretch), Duration(0UL),
  Units(LengthUnspecified), I(II), VType(II->GetASTType()) { }

  ASTStretchNode(const ASTIdentifierNode* Id, const ASTFloatNode* FF)
  : ASTExpressionNode(Id, ASTTypeStretch), Duration(0UL),
  Units(LengthUnspecified), F(FF), VType(FF->GetASTType()) { }

  ASTStretchNode(const ASTIdentifierNode* Id, const ASTDoubleNode* DD)
  : ASTExpressionNode(Id, ASTTypeStretch), Duration(0UL),
  Units(LengthUnspecified), DBL(DD), VType(DD->GetASTType()) { }

  ASTStretchNode(const ASTIdentifierNode* Id, const ASTMPIntegerNode* MPII)
  : ASTExpressionNode(Id, ASTTypeStretch), Duration(0UL),
  Units(LengthUnspecified), MPI(MPII), VType(MPII->GetASTType()) { }

  ASTStretchNode(const ASTIdentifierNode* Id, const ASTMPDecimalNode* MPDD)
  : ASTExpressionNode(Id, ASTTypeStretch), Duration(0UL),
  Units(LengthUnspecified), MPD(MPDD), VType(MPDD->GetASTType()) { }

  ASTStretchNode(const ASTIdentifierNode* Id, const ASTBinaryOpNode* BOPP)
  : ASTExpressionNode(Id, ASTTypeStretch), Duration(0UL),
  Units(LengthUnspecified), BOP(BOPP), VType(BOPP->GetASTType()) { }

  ASTStretchNode(const ASTIdentifierNode* Id, const ASTUnaryOpNode* UOPP)
  : ASTExpressionNode(Id, ASTTypeStretch), Duration(0UL),
  Units(LengthUnspecified), UOP(UOPP), VType(UOPP->GetASTType()) { }

  virtual ~ASTStretchNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeStretch;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTType GetVType() const {
    return VType;
  }

  virtual void Mangle() override;

  virtual const ASTIdentifierNode* GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string& GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual std::string AsString() const;

  virtual void SetDuration(uint64_t D) {
    Duration = D;
  }

  virtual void SetLengthUnit(LengthUnit LU) {
    Units = LU;
  }

  virtual uint64_t GetDuration() const {
    return Duration;
  }

  virtual LengthUnit GetLengthUnit() const {
    return Units;
  }

  const ASTIntNode* GetInt() const {
    return VType == ASTTypeInt ? I : nullptr;
  }

  const ASTFloatNode* GetFloat() const {
    return VType == ASTTypeFloat ? F : nullptr;
  }

  const ASTDoubleNode* GetDouble() const {
    return VType == ASTTypeDouble ? DBL : nullptr;
  }

  const ASTMPIntegerNode* GetMPInteger() const {
    return VType == ASTTypeMPInteger ? MPI : nullptr;
  }

  const ASTMPDecimalNode* GetMPDecimal() const {
    return VType == ASTTypeMPDecimal ? MPD : nullptr;
  }

  const ASTBinaryOpNode* GetBinaryOp() const {
    return VType == ASTTypeBinaryOp ? BOP : nullptr;
  }

  const ASTUnaryOpNode* GetUnaryOp() const {
    return VType == ASTTypeUnaryOp ? UOP : nullptr;
  }

  virtual bool IsError() const override {
    return VType == ASTTypeExpressionError;
  }

  virtual const std::string& GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTStretchNode* ExpressionError(const std::string& ERM) {
    return new ASTStretchNode(ASTIdentifierNode::Stretch.Clone(), ERM,
                              ASTTypeExpressionError);
  }

  static ASTStretchNode* ExpressionError(const ASTIdentifierNode* Id,
                                         const std::string& ERM) {
    return new ASTStretchNode(Id, ERM, ASTTypeExpressionError);
  }

  virtual void print() const override {
    std::cout << "<Stretch>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<Duration>" << Duration << "</Duration>" << std::endl;
    std::cout << "<LengthUnit>" << PrintLengthUnit(Units)
      << "</LengthUnit>" << std::endl;
    if (VType != ASTTypeUndefined) {
      std::cout << "<Value>" << std::endl;

      switch (VType) {
      case ASTTypeInt:
        I->print();
        break;
      case ASTTypeFloat:
        F->print();
        break;
      case ASTTypeDouble:
        DBL->print();
        break;
      case ASTTypeMPInteger:
        MPI->print();
        break;
      case ASTTypeMPDecimal:
        MPD->print();
        break;
      case ASTTypeBinaryOp:
        BOP->print();
        break;
      case ASTTypeUnaryOp:
        UOP->print();
        break;
      case ASTTypeExpressionError:
        std::cout << "<Error>" << GetError() << "</Error>"
          << std::endl;
        break;
      default:
        break;
      }

      std::cout << "</Value>" << std::endl;
    }
    std::cout << "</Stretch>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTStretchStatementNode : public ASTStatementNode {
private:
  const ASTStretchNode* Stretch;

private:
  ASTStretchStatementNode() = delete;

protected:
  ASTStretchStatementNode(const ASTIdentifierNode* Id, const std::string& ERM)
  : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)),
  Stretch(ASTStretchNode::ExpressionError(Id, ERM)) { }

public:
  ASTStretchStatementNode(const ASTStretchNode* S)
  : ASTStatementNode(S->GetIdentifier()), Stretch(S) { }

  virtual ~ASTStretchStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeStretchStatement;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual const ASTStretchNode* GetStretch() const {
    return Stretch;
  }

  virtual ASTStretchNode* GetStretch() {
    return const_cast<ASTStretchNode*>(Stretch);
  }

  virtual bool IsError() const override {
    return ASTStatementNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTStretchStatementNode* StatementError(const std::string& ERM) {
    return new ASTStretchStatementNode(ASTIdentifierNode::Stretch.Clone(), ERM);
  }

  virtual void print() const override {
    std::cout << "<StretchStatement>" << std::endl;
    Stretch->print();
    std::cout << "</StretchStatement>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM

#endif // __QASM_AST_STRETCH_H

