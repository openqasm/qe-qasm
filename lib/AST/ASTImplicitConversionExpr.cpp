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

#include <qasm/AST/ASTImplicitConversionExpr.h>
#include <qasm/AST/ASTExpressionEvaluator.h>
#include <qasm/AST/ASTExpressionValidator.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <iostream>
#include <string>
#include <sstream>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

bool
ASTImplicitConversionNode::IsValidConversion(ASTType FromTy,
                                             ASTType ToTy) const {
  if (FromType == ASTTypeUndefined ||
      ToType == ASTTypeUndefined)
    return false;

  switch (ToTy) {
  case ASTTypeBool:
    if (FromTy == ASTTypeBinaryOp &&
        ASTExpressionValidator::Instance().Validate(BOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
    else if (FromTy == ASTTypeUnaryOp &&
             ASTExpressionValidator::Instance().Validate(UOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
    else if (FromTy == ASTTypeIdentifier)
      FromTy = Id->GetSymbolType();

    return FromTy == ASTTypeBool ||
           FromTy == ASTTypeInt ||
           FromTy == ASTTypeUInt ||
           FromTy == ASTTypeMPInteger ||
           FromTy == ASTTypeMPUInteger ||
           FromTy == ASTTypeBitset ||
           FromTy == ASTTypeFloat ||
           FromTy == ASTTypeDouble ||
           FromTy == ASTTypeMPDecimal;
    break;
  case ASTTypeInt:
  case ASTTypeUInt:
    if (FromTy == ASTTypeBinaryOp &&
        ASTExpressionValidator::Instance().Validate(BOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
    else if (FromTy == ASTTypeUnaryOp &&
             ASTExpressionValidator::Instance().Validate(UOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
    else if (FromTy == ASTTypeIdentifier)
      FromTy = Id->GetSymbolType();

    return FromTy == ASTTypeBool ||
           FromTy == ASTTypeInt ||
           FromTy == ASTTypeUInt ||
           FromTy == ASTTypeMPInteger ||
           FromTy == ASTTypeMPUInteger ||
           FromTy == ASTTypeBitset ||
           FromTy == ASTTypeFloat ||
           FromTy == ASTTypeDouble ||
           FromTy == ASTTypeMPDecimal;
    break;
  case ASTTypeFloat:
    if (FromTy == ASTTypeBinaryOp &&
        ASTExpressionValidator::Instance().Validate(BOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
    else if (FromTy == ASTTypeUnaryOp &&
             ASTExpressionValidator::Instance().Validate(UOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
    else if (FromTy == ASTTypeIdentifier)
      FromTy = Id->GetSymbolType();

    return FromTy == ASTTypeBool ||
           FromTy == ASTTypeInt ||
           FromTy == ASTTypeUInt ||
           FromTy == ASTTypeMPInteger ||
           FromTy == ASTTypeMPUInteger ||
           FromTy == ASTTypeBitset ||
           FromTy == ASTTypeFloat ||
           FromTy == ASTTypeDouble ||
           FromTy == ASTTypeMPDecimal ||
           FromTy == ASTTypeAngle;
    break;
  case ASTTypeDouble:
    if (FromTy == ASTTypeBinaryOp &&
        ASTExpressionValidator::Instance().Validate(BOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
    else if (FromTy == ASTTypeUnaryOp &&
             ASTExpressionValidator::Instance().Validate(UOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
    else if (FromTy == ASTTypeIdentifier)
      FromTy = Id->GetSymbolType();

    return FromTy == ASTTypeBool ||
           FromTy == ASTTypeInt ||
           FromTy == ASTTypeUInt ||
           FromTy == ASTTypeMPInteger ||
           FromTy == ASTTypeMPUInteger ||
           FromTy == ASTTypeBitset ||
           FromTy == ASTTypeFloat ||
           FromTy == ASTTypeDouble ||
           FromTy == ASTTypeMPDecimal ||
           FromTy == ASTTypeAngle;
    break;
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
    if (FromTy == ASTTypeBinaryOp &&
        ASTExpressionValidator::Instance().Validate(BOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
    else if (FromTy == ASTTypeUnaryOp &&
             ASTExpressionValidator::Instance().Validate(UOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
    else if (FromTy == ASTTypeIdentifier)
      FromTy = Id->GetSymbolType();

    return FromTy == ASTTypeBool ||
           FromTy == ASTTypeInt ||
           FromTy == ASTTypeUInt ||
           FromTy == ASTTypeMPInteger ||
           FromTy == ASTTypeMPUInteger ||
           FromTy == ASTTypeBitset ||
           FromTy == ASTTypeFloat ||
           FromTy == ASTTypeDouble ||
           FromTy == ASTTypeMPDecimal;
    break;
  case ASTTypeMPDecimal:
    if (FromTy == ASTTypeBinaryOp &&
        ASTExpressionValidator::Instance().Validate(BOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
    else if (FromTy == ASTTypeUnaryOp &&
             ASTExpressionValidator::Instance().Validate(UOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
    else if (FromTy == ASTTypeIdentifier)
      FromTy = Id->GetSymbolType();

    return FromTy == ASTTypeBool ||
           FromTy == ASTTypeInt ||
           FromTy == ASTTypeUInt ||
           FromTy == ASTTypeMPInteger ||
           FromTy == ASTTypeMPUInteger ||
           FromTy == ASTTypeBitset ||
           FromTy == ASTTypeFloat ||
           FromTy == ASTTypeDouble ||
           FromTy == ASTTypeMPDecimal;
    break;
  case ASTTypeMPComplex:
    return FromTy == ASTTypeFloat ||
           FromTy == ASTTypeDouble ||
           FromTy == ASTTypeMPDecimal ||
           FromTy == ASTTypeMPComplex;
    break;
  case ASTTypeBitset:
    if (FromTy == ASTTypeBinaryOp &&
        ASTExpressionValidator::Instance().Validate(BOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
    else if (FromTy == ASTTypeUnaryOp &&
             ASTExpressionValidator::Instance().Validate(UOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
    else if (FromTy == ASTTypeIdentifier)
      FromTy = Id->GetSymbolType();

    return FromTy == ASTTypeBool ||
           FromTy == ASTTypeInt ||
           FromTy == ASTTypeUInt ||
           FromTy == ASTTypeMPInteger ||
           FromTy == ASTTypeMPUInteger ||
           FromTy == ASTTypeBitset ||
           FromTy == ASTTypeFloat ||
           FromTy == ASTTypeDouble ||
           FromTy == ASTTypeMPDecimal ||
           FromTy == ASTTypeAngle;
    break;
  case ASTTypeAngle:
    if (FromTy == ASTTypeBinaryOp &&
        ASTExpressionValidator::Instance().Validate(BOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
    else if (FromTy == ASTTypeUnaryOp &&
             ASTExpressionValidator::Instance().Validate(UOP))
      FromTy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
    else if (FromTy == ASTTypeIdentifier)
      FromTy = Id->GetSymbolType();

    return FromTy == ASTTypeBool ||
           FromTy == ASTTypeInt ||
           FromTy == ASTTypeUInt ||
           FromTy == ASTTypeMPInteger ||
           FromTy == ASTTypeMPUInteger ||
           FromTy == ASTTypeBitset ||
           FromTy == ASTTypeFloat ||
           FromTy == ASTTypeDouble ||
           FromTy == ASTTypeMPDecimal ||
           FromTy == ASTTypeAngle;
    break;
  default:
    break;
  }

  return false;
}

bool
ASTImplicitConversionNode::IsValidConversion() const {
  switch (FromType) {
  case ASTTypeIdentifier: {
    if (Id->GetSymbolType() == ASTTypeOpenPulseFrame) {
      if (ASTStringUtils::Instance().IsOpenPulseFramePhase(Id->GetName()))
        return IsValidConversion(ASTTypeAngle, ToType);
      else if (ASTStringUtils::Instance().IsOpenPulseFrameFrequency(Id->GetName()))
        return IsValidConversion(ASTTypeMPDecimal, ToType);
      else
        return IsValidConversion(ASTTypeOpenPulseFrame, ToType);
    } else if (Id->GetSymbolType() == ASTTypeMPComplex) {
      if (ASTStringUtils::Instance().IsComplexCReal(Id->GetName()) ||
          ASTStringUtils::Instance().IsComplexCImag(Id->GetName()))
        return IsValidConversion(ASTTypeMPDecimal, ToType);
      else
        return IsValidConversion(ASTTypeMPComplex, ToType);
    } else {
      return IsValidConversion(Id->GetSymbolType(), ToType);
    }
  }
    break;
  default:
    break;
  }

  return IsValidConversion(FromType, ToType);
}

ASTBoolNode*
ASTImplicitConversionNode::ConvertToBool() const {
  switch (FromType) {
  case ASTTypeIdentifier:
    return new ASTBoolNode(Id, Id);
    break;
  case ASTTypeBool:
    return const_cast<ASTBoolNode*>(Bool);
    break;
  case ASTTypeInt:
    return new ASTBoolNode(ASTIdentifierNode::Bool.Clone(), Int);
    break;
  case ASTTypeFloat:
    return new ASTBoolNode(ASTIdentifierNode::Bool.Clone(), Float);
    break;
  case ASTTypeDouble:
    return new ASTBoolNode(ASTIdentifierNode::Bool.Clone(), Double);
    break;
  case ASTTypeMPInteger:
    return new ASTBoolNode(ASTIdentifierNode::Bool.Clone(), MPI);
    break;
  case ASTTypeMPDecimal:
    return new ASTBoolNode(ASTIdentifierNode::Bool.Clone(), MPD);
    break;
  case ASTTypeBitset:
    return new ASTBoolNode(ASTIdentifierNode::Bool.Clone(), CBit);
    break;
  case ASTTypeAngle:
    return new ASTBoolNode(ASTIdentifierNode::Bool.Clone(), Angle);
    break;
  case ASTTypeBinaryOp:
    return new ASTBoolNode(ASTIdentifierNode::Bool.Clone(), BOP);
    break;
  case ASTTypeUnaryOp:
    return new ASTBoolNode(ASTIdentifierNode::Bool.Clone(), UOP);
    break;
  default: {
    std::stringstream M;
    M << "Impossible implicit conversion from " << PrintTypeEnum(FromType)
      << " to " << PrintTypeEnum(ToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(GetTargetIdentifier()),
                                              M.str(), DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  return nullptr;
}

ASTIntNode*
ASTImplicitConversionNode::ConvertToInt() const {
  switch (FromType) {
  case ASTTypeBool:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(),
                          static_cast<int32_t>(Bool->GetValue()));
    break;
  case ASTTypeInt:
    return const_cast<ASTIntNode*>(Int);
    break;
  case ASTTypeFloat:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(),
                          static_cast<int32_t>(Float->GetValue()));
    break;
  case ASTTypeDouble:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(),
                          static_cast<int32_t>(Double->GetValue()));
    break;
  case ASTTypeMPInteger:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(), MPI, 32);
    break;
  case ASTTypeMPDecimal:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(), MPD, 32);
    break;
  case ASTTypeBitset:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(), CBit, 32);
    break;
  case ASTTypeAngle:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(), Angle, 32);
     break;
  case ASTTypeBinaryOp:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(), BOP, 32);
     break;
  case ASTTypeUnaryOp:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(), UOP, 32);
     break;
  default: {
    std::stringstream M;
    M << "Impossible implicit conversion from " << PrintTypeEnum(FromType)
      << " to " << PrintTypeEnum(ToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(GetTargetIdentifier()),
                                              M.str(), DiagLevel::Error);
    return nullptr;
  }
     break;
  }

  return nullptr;
}

ASTFloatNode*
ASTImplicitConversionNode::ConvertToFloat() const {
  switch (FromType) {
  case ASTTypeBool:
    return new ASTFloatNode(ASTIdentifierNode::Float.Clone(), Bool);
    break;
  case ASTTypeInt:
    return new ASTFloatNode(ASTIdentifierNode::Float.Clone(), Int);
    break;
  case ASTTypeFloat:
    return const_cast<ASTFloatNode*>(Float);
    break;
  case ASTTypeDouble:
    return new ASTFloatNode(ASTIdentifierNode::Float.Clone(),
                            static_cast<float>(Double->GetValue()));
    break;
  case ASTTypeMPInteger:
    return new ASTFloatNode(ASTIdentifierNode::Float.Clone(), MPI);
    break;
  case ASTTypeMPDecimal:
    return new ASTFloatNode(ASTIdentifierNode::Float.Clone(), MPD);
    break;
  case ASTTypeBitset:
    return new ASTFloatNode(ASTIdentifierNode::Float.Clone(), CBit);
    break;
  case ASTTypeAngle:
    return new ASTFloatNode(ASTIdentifierNode::Float.Clone(), Angle);
     break;
  case ASTTypeBinaryOp:
    return new ASTFloatNode(ASTIdentifierNode::Float.Clone(), BOP);
     break;
  case ASTTypeUnaryOp:
    return new ASTFloatNode(ASTIdentifierNode::Float.Clone(), UOP);
     break;
  default: {
    std::stringstream M;
    M << "Impossible implicit conversion from " << PrintTypeEnum(FromType)
      << " to " << PrintTypeEnum(ToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(GetTargetIdentifier()),
                                              M.str(), DiagLevel::Error);
    return nullptr;
  }
     break;
  }

  return nullptr;
}

ASTDoubleNode*
ASTImplicitConversionNode::ConvertToDouble() const {
  switch (FromType) {
  case ASTTypeBool:
    return new ASTDoubleNode(ASTIdentifierNode::Double.Clone(), Bool);
    break;
  case ASTTypeInt:
    return new ASTDoubleNode(ASTIdentifierNode::Double.Clone(), Int);
    break;
  case ASTTypeFloat:
    return new ASTDoubleNode(ASTIdentifierNode::Double.Clone(), Float);
    break;
  case ASTTypeDouble:
    return const_cast<ASTDoubleNode*>(Double);
    break;
  case ASTTypeMPInteger:
    return new ASTDoubleNode(ASTIdentifierNode::Double.Clone(), MPI);
    break;
  case ASTTypeMPDecimal:
    return new ASTDoubleNode(ASTIdentifierNode::Double.Clone(), MPD);
    break;
  case ASTTypeBitset:
    return new ASTDoubleNode(ASTIdentifierNode::Double.Clone(), CBit);
    break;
  case ASTTypeAngle:
    return new ASTDoubleNode(ASTIdentifierNode::Double.Clone(), Angle);
     break;
  case ASTTypeBinaryOp:
    return new ASTDoubleNode(ASTIdentifierNode::Double.Clone(), BOP);
     break;
  case ASTTypeUnaryOp:
    return new ASTDoubleNode(ASTIdentifierNode::Double.Clone(), UOP);
     break;
  default: {
    std::stringstream M;
    M << "Impossible implicit conversion from " << PrintTypeEnum(FromType)
      << " to " << PrintTypeEnum(ToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(GetTargetIdentifier()),
                                              M.str(), DiagLevel::Error);
    return nullptr;
  }
     break;
  }

  return nullptr;
}

ASTMPIntegerNode*
ASTImplicitConversionNode::ConvertToMPInteger() const {
  ASTIntNode* I = nullptr;

  switch (FromType) {
  case ASTTypeBool:
    I = new ASTIntNode(static_cast<int32_t>(Bool->GetValue()));
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), I, 8);
    break;
  case ASTTypeInt:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Int,
                                Int->GetBits());
    break;
  case ASTTypeFloat:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Signed,
                                Float->GetBits(), Float);
    break;
  case ASTTypeDouble:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Signed,
                                Double->GetBits(), Double);
    break;
  case ASTTypeMPInteger:
    return const_cast<ASTMPIntegerNode*>(MPI);
    break;
  case ASTTypeMPDecimal:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Signed,
                                MPD->GetBits(), MPD);
    break;
  case ASTTypeBitset:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Signed,
                                CBit->Size(), CBit->AsString().c_str(), 2);
    break;
  case ASTTypeAngle:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Signed,
                                Angle->GetBits(),
                                Angle->GetValue(2).c_str(), 2);
     break;
  case ASTTypeBinaryOp:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Signed, 64,
                                BOP);
     break;
  case ASTTypeUnaryOp:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Signed, 64,
                                UOP);
     break;
  default: {
    std::stringstream M;
    M << "Impossible implicit conversion from " << PrintTypeEnum(FromType)
      << " to " << PrintTypeEnum(ToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(GetTargetIdentifier()),
                                              M.str(), DiagLevel::Error);
    return nullptr;
  }
     break;
  }

  return nullptr;
}

ASTMPDecimalNode*
ASTImplicitConversionNode::ConvertToMPDecimal() const {
  switch (FromType) {
  case ASTTypeBool:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(), 8, Bool);
    break;
  case ASTTypeInt:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                                Int->GetBits(), Int);
    break;
  case ASTTypeFloat:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                                Float->GetBits(), Float);
    break;
  case ASTTypeDouble:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                                Double->GetBits(), Double);
    break;
  case ASTTypeMPInteger:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                                MPI->GetBits(), MPI);
    break;
  case ASTTypeMPDecimal:
    return const_cast<ASTMPDecimalNode*>(MPD);
    break;
  case ASTTypeBitset:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                                CBit->Size(), CBit);
    break;
  case ASTTypeAngle:
    return Angle->AsMPDecimal();
     break;
  case ASTTypeBinaryOp:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(), 64, BOP);
     break;
  case ASTTypeUnaryOp:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(), 64, UOP);
     break;
  default: {
    std::stringstream M;
    M << "Impossible implicit conversion from " << PrintTypeEnum(FromType)
      << " to " << PrintTypeEnum(ToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(GetTargetIdentifier()),
                                              M.str(), DiagLevel::Error);
    return nullptr;
  }
     break;
  }

  return nullptr;
}

ASTMPComplexNode*
ASTImplicitConversionNode::ConvertToMPComplex() const {
  std::stringstream M;
  M << "Impossible implicit conversion from " << PrintTypeEnum(FromType)
    << " to " << PrintTypeEnum(ToType) << " requested.";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
    DIAGLineCounter::Instance().GetLocation(GetTargetIdentifier()),
                                            M.str(), DiagLevel::Error);
  return nullptr;
}

ASTCBitNode*
ASTImplicitConversionNode::ConvertToBitset() const {
  std::stringstream S;

  switch (FromType) {
  case ASTTypeBool:
    S << std::noboolalpha << Bool->GetValue();
    return new ASTCBitNode(ASTIdentifierNode::Bitset.Clone(), 1, S.str());
    break;
  case ASTTypeInt:
    S << (Int->IsSigned() ?
                         BitString(Int->GetSignedValue()) :
                         BitString(Int->GetUnsignedValue()));
    return new ASTCBitNode(ASTIdentifierNode::Bitset.Clone(), Int->GetBits(),
                           S.str());
    break;
  case ASTTypeFloat:
    S << BitString(Float->GetValue());
    return new ASTCBitNode(ASTIdentifierNode::Bitset.Clone(), Float->GetBits(),
                           S.str());
    break;
  case ASTTypeDouble:
    S << BitString(Double->GetValue());
    return new ASTCBitNode(ASTIdentifierNode::Bitset.Clone(), Double->GetBits(),
                           S.str());
    break;
  case ASTTypeMPInteger:
    S << MPI->GetValue(2);
    return new ASTCBitNode(ASTIdentifierNode::Bitset.Clone(), MPI->GetBits(),
                           S.str());
    break;
  case ASTTypeMPDecimal:
    S << MPD->GetValue(2);
    return new ASTCBitNode(ASTIdentifierNode::Bitset.Clone(), MPD->GetBits(),
                           S.str());
    break;
  case ASTTypeBitset:
    return const_cast<ASTCBitNode*>(CBit);
    break;
  case ASTTypeAngle:
    S << Angle->GetValue(2);
    return new ASTCBitNode(ASTIdentifierNode::Bitset.Clone(), Angle->GetBits(),
                           S.str());
    break;
  default: {
    std::stringstream M;
    M << "Impossible implicit conversion from " << PrintTypeEnum(FromType)
      << " to " << PrintTypeEnum(ToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(GetTargetIdentifier()),
                                              M.str(), DiagLevel::Error);
    return nullptr;
  }
     break;
  }

  return nullptr;
}

ASTAngleNode*
ASTImplicitConversionNode::ConvertToAngle() const {
  switch (FromType) {
  case ASTTypeBool:
    return new ASTAngleNode(ASTIdentifierNode::Angle.Clone(), Bool,
                            ASTAngleTypeGeneric, 8U);
    break;
  case ASTTypeInt:
    return new ASTAngleNode(ASTIdentifierNode::Angle.Clone(), Int,
                            ASTAngleTypeGeneric, Int->GetBits());
    break;
  case ASTTypeFloat:
    return new ASTAngleNode(ASTIdentifierNode::Angle.Clone(), Float,
                            ASTAngleTypeGeneric, Float->GetBits());
    break;
  case ASTTypeDouble:
    return new ASTAngleNode(ASTIdentifierNode::Angle.Clone(), Double,
                            ASTAngleTypeGeneric, Double->GetBits());
    break;
  case ASTTypeMPInteger:
    return new ASTAngleNode(ASTIdentifierNode::Angle.Clone(), MPI,
                            ASTAngleTypeGeneric, MPI->GetBits());
    break;
  case ASTTypeMPDecimal:
    return new ASTAngleNode(ASTIdentifierNode::Angle.Clone(), MPD,
                            ASTAngleTypeGeneric, MPD->GetBits());
    break;
  case ASTTypeBitset:
    return new ASTAngleNode(ASTIdentifierNode::Angle.Clone(), CBit,
                            ASTAngleTypeGeneric, CBit->Size());
    break;
  case ASTTypeAngle:
    return const_cast<ASTAngleNode*>(Angle);
    break;
  case ASTTypeBinaryOp:
    return new ASTAngleNode(ASTIdentifierNode::Angle.Clone(), BOP,
                            ASTAngleTypeGeneric, 64);
     break;
  case ASTTypeUnaryOp:
    return new ASTAngleNode(ASTIdentifierNode::Angle.Clone(), UOP,
                            ASTAngleTypeGeneric, 64);
     break;
  default: {
    std::stringstream M;
    M << "Impossible implicit conversion from " << PrintTypeEnum(FromType)
      << " to " << PrintTypeEnum(ToType) << " requested.";
    return nullptr;
  }
     break;
  }

  return nullptr;
}

void
ASTImplicitConversionNode::Mangle() {
  if (GetPolymorphicName().empty())
    const_cast<ASTIdentifierNode*>(GetIdentifier())->SetPolymorphicName("implconv");

  ASTMangler M;
  M.Start();

  M.TypeIdentifier(GetASTType(), GetPolymorphicName());
  M.Underscore();

  switch (FromType) {
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
    M.TypeIdentifier(Id->GetSymbolType(), Id->GetBits(),
                     Id->GetPolymorphicName());
    break;
  case ASTTypeBool:
    M.TypeIdentifier(Bool->GetASTType(), Bool->GetBits(),
                     Bool->GetIdentifier()->GetPolymorphicName());
    break;
  case ASTTypeInt:
  case ASTTypeUInt:
    M.TypeIdentifier(Int->GetASTType(), Int->GetBits(),
                     Int->GetIdentifier()->GetPolymorphicName());
    break;
  case ASTTypeFloat:
    M.TypeIdentifier(Float->GetASTType(), Float->GetBits(),
                     Float->GetIdentifier()->GetPolymorphicName());
    break;
  case ASTTypeDouble:
    M.TypeIdentifier(Double->GetASTType(), Double->GetBits(),
                     Double->GetIdentifier()->GetPolymorphicName());
    break;
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
    M.TypeIdentifier(MPI->GetASTType(), MPI->GetBits(),
                     MPI->GetIdentifier()->GetPolymorphicName());
    break;
  case ASTTypeMPDecimal:
    M.TypeIdentifier(MPD->GetASTType(), MPD->GetBits(),
                     MPD->GetIdentifier()->GetPolymorphicName());
    break;
  case ASTTypeBitset:
    M.TypeIdentifier(CBit->GetASTType(), CBit->Size(),
                     CBit->GetIdentifier()->GetPolymorphicName());
    break;
  case ASTTypeAngle:
    M.TypeIdentifier(Angle->GetASTType(), Angle->GetBits(),
                     Angle->GetIdentifier()->GetPolymorphicName());
    break;
  case ASTTypeBinaryOp:
    M.TypeIdentifier(BOP->GetASTType(),
                     BOP->GetIdentifier()->GetPolymorphicName());
    break;
  case ASTTypeUnaryOp:
    M.TypeIdentifier(UOP->GetASTType(),
                     UOP->GetIdentifier()->GetPolymorphicName());
    break;
  default:
    M.TypeIdentifier(ASTTypeUndefined, "none");
    break;
  }

  M.EndExpression();

  M.TypeSize(ToType, Bits);
  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString(),
                                                                  true);
}

} // namespace QASM

