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

#ifndef __QASM_AST_IMPLICIT_CONVERSION_H
#define __QASM_AST_IMPLICIT_CONVERSION_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTQubit.h>

#include <cassert>

namespace QASM {

class ASTImplicitConversionNode : public ASTExpressionNode {
  friend class ASTBuilder;

private:
  union {
    const ASTVoidNode* Void;
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
    const ASTExpressionNode* Expr;
    const ASTStatementNode* Stmt;
  };

  unsigned Bits;
  mutable ASTType ToType;
  mutable ASTType FromType;

private:
  ASTImplicitConversionNode() = delete;

protected:
  ASTImplicitConversionNode(ASTType From, ASTType To)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  Void(nullptr), Bits(static_cast<unsigned>(~0x0)),
  ToType(To), FromType(From) { }

public:
  ASTImplicitConversionNode(const ASTIdentifierNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  Id(From), Bits(Bi), ToType(CT), FromType(From->GetASTType())
  { }

  ASTImplicitConversionNode(const ASTBoolNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  Bool(From), Bits(Bi), ToType(CT), FromType(From->GetASTType())
  { }

  ASTImplicitConversionNode(const ASTIntNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  Int(From), Bits(Bi), ToType(CT), FromType(From->GetASTType())
  { }

  ASTImplicitConversionNode(const ASTFloatNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  Float(From), Bits(Bi), ToType(CT), FromType(From->GetASTType())
  { }

  ASTImplicitConversionNode(const ASTDoubleNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  Double(From), Bits(Bi), ToType(CT), FromType(From->GetASTType())
  { }

  ASTImplicitConversionNode(const ASTMPIntegerNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  MPI(From), Bits(Bi), ToType(CT), FromType(From->GetASTType())
  { }

  ASTImplicitConversionNode(const ASTMPDecimalNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  MPD(From), Bits(Bi), ToType(CT), FromType(From->GetASTType())
  { }

  ASTImplicitConversionNode(const ASTMPComplexNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  MPC(From), Bits(Bi), ToType(CT), FromType(From->GetASTType())
  { }

  ASTImplicitConversionNode(const ASTCBitNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  CBit(From), Bits(Bi), ToType(CT), FromType(From->GetASTType())
  { }

  ASTImplicitConversionNode(const ASTAngleNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  Angle(From), Bits(Bi), ToType(CT), FromType(From->GetASTType())
  { }

  ASTImplicitConversionNode(const ASTBinaryOpNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  BOP(From), Bits(Bi), ToType(CT), FromType(From->GetASTType())
  { }

  ASTImplicitConversionNode(const ASTUnaryOpNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  UOP(From), Bits(Bi), ToType(CT), FromType(From->GetASTType())
  { }

  ASTImplicitConversionNode(const ASTStatementNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  Stmt(From), Bits(Bi), ToType(CT), FromType(ASTTypeStatement)
  { }

  ASTImplicitConversionNode(const ASTExpressionNode* From,
                            ASTType CT, unsigned Bi = 0)
  : ASTExpressionNode(ASTIdentifierNode::ImplConv.Clone(),
                      ASTTypeImplicitConversion),
  Expr(From), Bits(Bi), ToType(CT), FromType(ASTTypeExpression)
  { }

  virtual ~ASTImplicitConversionNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeImplicitConversion;
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

  virtual ASTType GetConvertFrom() const {
    return FromType;
  }

  virtual ASTType GetConvertTo() const {
    return ToType;
  }

  virtual const ASTIdentifierNode* GetTargetIdentifier() const {
    switch (FromType) {
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

  virtual bool IsValidConversion() const;

  virtual bool IsValidConversion(ASTType FromTy, ASTType ToTy) const;

  virtual const ASTBoolNode* GetBool() const {
    return FromType == ASTTypeBool ? Bool : nullptr;
  }

  virtual const ASTIntNode* GetInt() const {
    return FromType == ASTTypeInt ? Int : nullptr;
  }

  virtual const ASTFloatNode* GetFloat() const {
    return FromType == ASTTypeFloat ? Float : nullptr;
  }

  virtual const ASTDoubleNode* GetDouble() const {
    return FromType == ASTTypeDouble ? Double : nullptr;
  }

  virtual const ASTMPIntegerNode* GetMPInteger() const {
    return FromType == ASTTypeMPInteger ? MPI : nullptr;
  }

  virtual const ASTMPDecimalNode* GetMPDecimal() const {
    return FromType == ASTTypeMPDecimal ? MPD : nullptr;
  }

  virtual const ASTMPComplexNode* GetMPComplex() const {
    return FromType == ASTTypeMPComplex ? MPC : nullptr;
  }

  virtual const ASTCBitNode* GetCBit() const {
    return FromType == ASTTypeBitset ? CBit : nullptr;
  }

  virtual const ASTAngleNode* GetAngle() const {
    return FromType == ASTTypeAngle ? Angle : nullptr;
  }

  virtual const ASTBinaryOpNode* GetBinaryOp() const {
    return FromType == ASTTypeBinaryOp ? BOP : nullptr;
  }

  virtual const ASTUnaryOpNode* GetUnaryOp() const {
    return FromType == ASTTypeUnaryOp ? UOP : nullptr;
  }

  virtual const ASTStatementNode* GetStatement() const {
    return FromType == ASTTypeStatement ? Stmt : nullptr;
  }

  virtual const ASTExpressionNode* GetExpression() const {
    return FromType == ASTTypeExpression ? Expr : nullptr;
  }

  virtual ASTBoolNode* ConvertToBool() const;

  virtual ASTIntNode* ConvertToInt() const;

  virtual ASTFloatNode* ConvertToFloat() const;

  virtual ASTDoubleNode* ConvertToDouble() const;

  virtual ASTMPIntegerNode* ConvertToMPInteger() const;

  virtual ASTMPDecimalNode* ConvertToMPDecimal() const;

  virtual ASTMPComplexNode* ConvertToMPComplex() const;

  virtual ASTCBitNode* ConvertToBitset() const;

  virtual ASTAngleNode* ConvertToAngle() const;

  static ASTImplicitConversionNode* InvalidConversion(ASTType From,
                                                      ASTType To) {
    return new ASTImplicitConversionNode(From, To);
  }

  virtual void print() const override {
    std::cout << "<ImplicitConversionExpression>" << std::endl;
    std::cout << "<ConvertFrom>" << std::endl;
    std::cout << "<Type>" << PrintTypeEnum(FromType)
      << "</Type>" << std::endl;

    switch (FromType) {
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

    std::cout << "</ConvertFrom>" << std::endl;
    std::cout << "<ConvertTo>" << std::endl;
    std::cout << "<Type>" << PrintTypeEnum(ToType)
      << "</Type>" << std::endl;
    std::cout << "</ConvertTo>" << std::endl;
    std::cout << "</ImplicitConversionExpression>" << std::endl;
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

#endif // __QASM_AST_IMPLICIT_CONVERSION_EXPR_H

