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

#ifndef __QASM_AST_CAST_EXPRESSION_H
#define __QASM_AST_CAST_EXPRESSION_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTTypeCastController.h>

namespace QASM {

class ASTCastExpressionNode : public ASTExpressionNode {
  friend class ASTBuilder;

private:
  union {
    const ASTIdentifierNode* Id;
    const ASTBoolNode* Bool;
    const ASTIntNode* Int;
    const ASTFloatNode* Float;
    const ASTDoubleNode* Double;
    const ASTMPIntegerNode* MPI;
    const ASTMPDecimalNode* MPD;
    const ASTMPComplexNode* MPC;
    const ASTCBitNode* CBit;
    const ASTAngleNode* Angle;
    const ASTBinaryOpNode* BOP;
    const ASTUnaryOpNode* UOP;
  };

  unsigned Bits;
  ASTType CastToType;
  ASTType CastFromType;
  ASTTypeConversionMethod CM;

private:
  ASTCastExpressionNode() = delete;

protected:
  ASTCastExpressionNode(const std::string& ERM)
  : ASTExpressionNode(ASTIdentifierNode::BadCast.Clone(),
                      new ASTStringNode(ERM), ASTTypeExpressionError),
  Id(nullptr), Bits(static_cast<unsigned>(~0x0)), CastToType(ASTTypeBadCast),
  CastFromType(ASTTypeBadCast), CM(ASTConvMethodUnknown) { }


public:
  ASTCastExpressionNode(const ASTIdentifierNode* From, ASTType CT,
                        unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::Cast.Clone(), ASTTypeCast),
  Id(From), Bits(Bi), CastToType(CT), CastFromType(From->GetASTType()),
  CM(ASTConvMethodUnknown) {
    CM = ASTTypeCastController::Instance().ResolveConversionMethod(From, CT);
    this->Mangle();
  }

  ASTCastExpressionNode(const ASTBoolNode* From, ASTType CT,
                        unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::Cast.Clone(), ASTTypeCast),
  Bool(From), Bits(Bi), CastToType(CT), CastFromType(From->GetASTType()),
  CM(ASTConvMethodUnknown) {
    CM = ASTTypeCastController::Instance().ResolveConversionMethod(
                                           From->GetASTType(), CT);
    this->Mangle();
  }

  ASTCastExpressionNode(const ASTIntNode* From, ASTType CT,
                        unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::Cast.Clone(), ASTTypeCast),
  Int(From), Bits(Bi), CastToType(CT), CastFromType(From->GetASTType()),
  CM(ASTConvMethodUnknown) {
    CM = ASTTypeCastController::Instance().ResolveConversionMethod(
                                           From->GetASTType(), CT);
    this->Mangle();
  }

  ASTCastExpressionNode(const ASTFloatNode* From, ASTType CT,
                        unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::Cast.Clone(), ASTTypeCast),
  Float(From), Bits(Bi), CastToType(CT), CastFromType(From->GetASTType()),
  CM(ASTConvMethodUnknown) {
    CM = ASTTypeCastController::Instance().ResolveConversionMethod(
                                           From->GetASTType(), CT);
    this->Mangle();
  }

  ASTCastExpressionNode(const ASTDoubleNode* From, ASTType CT,
                        unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::Cast.Clone(), ASTTypeCast),
  Double(From), Bits(Bi), CastToType(CT), CastFromType(From->GetASTType()),
  CM(ASTConvMethodUnknown) {
    CM = ASTTypeCastController::Instance().ResolveConversionMethod(
                                           From->GetASTType(), CT);
    this->Mangle();
  }

  ASTCastExpressionNode(const ASTMPIntegerNode* From, ASTType CT,
                        unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::Cast.Clone(), ASTTypeCast),
  MPI(From), Bits(Bi), CastToType(CT), CastFromType(From->GetASTType()),
  CM(ASTConvMethodUnknown) {
    CM = ASTTypeCastController::Instance().ResolveConversionMethod(
                                           From->GetASTType(), CT);
    this->Mangle();
  }

  ASTCastExpressionNode(const ASTMPDecimalNode* From, ASTType CT,
                        unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::Cast.Clone(), ASTTypeCast),
  MPD(From), Bits(Bi), CastToType(CT), CastFromType(From->GetASTType()),
  CM(ASTConvMethodUnknown) {
    CM = ASTTypeCastController::Instance().ResolveConversionMethod(
                                           From->GetASTType(), CT);
    this->Mangle();
  }

  ASTCastExpressionNode(const ASTMPComplexNode* From, ASTType CT,
                        unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::Cast.Clone(), ASTTypeCast),
  MPC(From), Bits(Bi), CastToType(CT), CastFromType(From->GetASTType()),
  CM(ASTConvMethodUnknown) {
    CM = ASTTypeCastController::Instance().ResolveConversionMethod(
                                           From->GetASTType(), CT);
    this->Mangle();
  }

  ASTCastExpressionNode(const ASTCBitNode* From, ASTType CT,
                        unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::Cast.Clone(), ASTTypeCast),
  CBit(From), Bits(Bi), CastToType(CT), CastFromType(From->GetASTType()),
  CM(ASTConvMethodUnknown) {
    CM = ASTTypeCastController::Instance().ResolveConversionMethod(
                                           From->GetASTType(), CT);
    this->Mangle();
  }

  ASTCastExpressionNode(const ASTAngleNode* From, ASTType CT,
                        unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::Cast.Clone(), ASTTypeCast),
  Angle(From), Bits(Bi), CastToType(CT), CastFromType(From->GetASTType()),
  CM(ASTConvMethodUnknown) {
    CM = ASTTypeCastController::Instance().ResolveConversionMethod(
                                           From->GetASTType(), CT);
    this->Mangle();
  }

  ASTCastExpressionNode(const ASTBinaryOpNode* From, ASTType CT,
                        unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::Cast.Clone(), ASTTypeCast),
  BOP(From), Bits(Bi), CastToType(CT), CastFromType(From->GetASTType()),
  CM(ASTConvMethodUnknown) {
    CM = ASTTypeCastController::Instance().ResolveConversionMethod(From, CT);
    this->Mangle();
  }

  ASTCastExpressionNode(const ASTUnaryOpNode* From, ASTType CT,
                        unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::Cast.Clone(), ASTTypeCast),
  UOP(From), Bits(Bi), CastToType(CT), CastFromType(From->GetASTType()),
  CM(ASTConvMethodUnknown) {
    CM = ASTTypeCastController::Instance().ResolveConversionMethod(From, CT);
    this->Mangle();
  }

  virtual ~ASTCastExpressionNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeCast;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual const ASTIdentifierNode* GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string& GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual ASTType GetCastFrom() const {
    return CastFromType;
  }

  virtual ASTType GetCastTo() const {
    return CastToType;
  }

  virtual ASTTypeConversionMethod GetConversionMethod() const {
    return CM;
  }

  virtual bool IsBadCast() const {
    return CM == ASTConvMethodBadCast ||
           CM == ASTConvMethodUnknown;
  }

  virtual const ASTIdentifierNode* GetTargetIdentifier() const {
    switch (CastFromType) {
    case ASTTypeIdentifier:
    case ASTTypeIdentifierRef:
      return Id;
      break;
    case ASTTypeBool:
      return Bool ? Bool->GetIdentifier() : nullptr;
      break;
    case ASTTypeInt:
      return Int ? Int->GetIdentifier() : nullptr;
      break;
    case ASTTypeFloat:
      return Float ? Float->GetIdentifier() : nullptr;
      break;
    case ASTTypeMPInteger:
      return MPI ? MPI->GetIdentifier() : nullptr;
      break;
    case ASTTypeMPDecimal:
      return MPD ? MPD->GetIdentifier() : nullptr;
      break;
    case ASTTypeMPComplex:
      return MPC ? MPC->GetIdentifier() : nullptr;
      break;
    case ASTTypeBitset:
      return CBit ? CBit->GetIdentifier() : nullptr;
        break;
    case ASTTypeAngle:
      return Angle ? Angle->GetIdentifier() : nullptr;
      break;
    case ASTTypeBinaryOp:
      return BOP ? BOP->GetIdentifier() : nullptr;
      break;
    case ASTTypeUnaryOp:
      return UOP ? UOP->GetIdentifier() : nullptr;
      break;
    default:
      return nullptr;
      break;
    }
  }

  virtual const ASTBoolNode* GetBool() const {
    return CastFromType == ASTTypeBool ? Bool : nullptr;
  }

  virtual const ASTIntNode* GetInt() const {
    return CastFromType == ASTTypeInt ? Int : nullptr;
  }

  virtual const ASTFloatNode* GetFloat() const {
    return CastFromType == ASTTypeFloat ? Float : nullptr;
  }

  virtual const ASTDoubleNode* GetDouble() const {
    return CastFromType == ASTTypeDouble ? Double : nullptr;
  }

  virtual const ASTMPIntegerNode* GetMPInteger() const {
    return CastFromType == ASTTypeMPInteger ? MPI : nullptr;
  }

  virtual const ASTMPDecimalNode* GetMPDecimal() const {
    return CastFromType == ASTTypeMPDecimal ? MPD : nullptr;
  }

  virtual const ASTMPComplexNode* GetMPComplex() const {
    return CastFromType == ASTTypeMPComplex ? MPC : nullptr;
  }

  virtual const ASTCBitNode* GetCBit() const {
    return CastFromType == ASTTypeBitset ? CBit : nullptr;
  }

  virtual const ASTAngleNode* GetAngle() const {
    return CastFromType == ASTTypeAngle ? Angle : nullptr;
  }

  virtual const ASTBinaryOpNode* GetBinaryOp() const {
    return CastFromType == ASTTypeBinaryOp ? BOP : nullptr;
  }

  virtual const ASTUnaryOpNode* GetUnaryOp() const {
    return CastFromType == ASTTypeUnaryOp ? UOP : nullptr;
  }

  virtual ASTBoolNode* CastToBool() const;

  virtual ASTIntNode* CastToInt() const;

  virtual ASTIntNode* CastToUInt() const;

  virtual ASTFloatNode* CastToFloat() const;

  virtual ASTDoubleNode* CastToDouble() const;

  virtual ASTMPIntegerNode* CastToMPInteger() const;

  virtual ASTMPIntegerNode* CastToMPUInteger() const;

  virtual ASTMPDecimalNode* CastToMPDecimal() const;

  virtual ASTMPComplexNode* CastToMPComplex() const;

  virtual ASTCBitNode* CastToBitset() const;

  virtual ASTAngleNode* CastToAngle() const;

  virtual bool IsError() const override {
    return ASTExpressionNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTCastExpressionNode* ExpressionError(const std::string& ERM) {
    return new ASTCastExpressionNode(ERM);
  }

  virtual void print() const override {
    std::cout << "<CastExpression>" << std::endl;
    std::cout << "<MangledName>" << GetIdentifier()->GetMangledName()
      << "</MangledName>" << std::endl;
    std::cout << "<ConversionMethod>" << PrintTypeConversionMethod(CM)
      << "</ConversionMethod>" << std::endl;
    std::cout << "<CastFrom>" << std::endl;
    std::cout << "<Type>" << PrintTypeEnum(CastFromType)
      << "</Type>" << std::endl;

    switch (CastFromType) {
    case ASTTypeBool:
      Bool->print();
      break;
    case ASTTypeInt:
      Int->print();
      break;
    case ASTTypeFloat:
      Float->print();
      break;
    case ASTTypeDouble:
      Double->print();
      break;
    case ASTTypeMPInteger:
      MPI->print();
      break;
    case ASTTypeMPDecimal:
      MPD->print();
      break;
    case ASTTypeMPComplex:
      MPC->print();
      break;
    case ASTTypeBitset:
      CBit->print();
      break;
    case ASTTypeAngle:
      Angle->print();
      break;
    case ASTTypeBinaryOp:
      BOP->print();
      break;
    case ASTTypeUnaryOp:
      UOP->print();
      break;
    case ASTTypeIdentifier:
    case ASTTypeIdentifierRef:
      Id->print();
      break;
    default:
      break;
    }

    std::cout << "</CastFrom>" << std::endl;
    std::cout << "<CastTo>" << std::endl;
    std::cout << "<Type>" << PrintTypeEnum(CastToType)
      << "</Type>" << std::endl;
    std::cout << "</CastTo>" << std::endl;
    std::cout << "</CastExpression>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }

private:
  template<typename __Type>
  std::string BitString(const __Type& T) const {
    size_t Size = sizeof(T);
    if (Size > 8)
      return std::string();

    union AL {
      double __Pad;
      const uint64_t* V;
    };

    std::stringstream S;
    AL Align;
    Align.V = reinterpret_cast<const uint64_t*>(&T);

    for (unsigned I = 0; I < Size * CHAR_BIT; ++I) {
      if ((*(Align.V) & ((1UL << (uint64_t) I))) != 0)
        S << '1';
      else
        S << '0';
    }

    return S.str();
  }
};

} // namespace QASM

#endif // __QASM_AST_CAST_EXPRESSION_H

