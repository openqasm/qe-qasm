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

#include <qasm/AST/ASTCastExpr.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTExpressionEvaluator.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <iostream>
#include <string>
#include <sstream>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTBoolNode*
ASTCastExpressionNode::CastToBool() const {
  if (IsBadCast()) {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTBoolNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                        M.str());;
  }

  switch (CastFromType) {
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
  case ASTTypeMPComplex:
    return new ASTBoolNode(ASTIdentifierNode::Bool.Clone(),
                           MPC->GetRealAsMPDecimal());
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
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTBoolNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                        M.str());
  }
    break;
  }

  return nullptr;
}

ASTIntNode*
ASTCastExpressionNode::CastToInt() const {
  if (IsBadCast()) {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTIntNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                       M.str());
  }

  switch (CastFromType) {
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
  case ASTTypeMPComplex:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(),
                          MPC->GetRealAsMPDecimal(), 32);
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
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTIntNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                       M.str());
  }
     break;
  }

  return nullptr;
}

ASTIntNode*
ASTCastExpressionNode::CastToUInt() const {
  if (IsBadCast()) {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTIntNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                       M.str());
  }

  switch (CastFromType) {
  case ASTTypeBool:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(),
                          static_cast<uint32_t>(Bool->GetValue()));
    break;
  case ASTTypeInt:
    return const_cast<ASTIntNode*>(Int);
    break;
  case ASTTypeFloat:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(),
                          static_cast<uint32_t>(Float->GetValue()));
    break;
  case ASTTypeDouble:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(),
                          static_cast<uint32_t>(Double->GetValue()));
    break;
  case ASTTypeMPInteger:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(), MPI, 32, true);
    break;
  case ASTTypeMPDecimal:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(), MPD, 32, true);
    break;
  case ASTTypeMPComplex:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(),
                          MPC->GetRealAsMPDecimal(), 32, true);
    break;
  case ASTTypeBitset:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(), CBit, 32, true);
    break;
  case ASTTypeAngle:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(), Angle, 32, true);
    break;
  case ASTTypeBinaryOp:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(), BOP, 32, true);
    break;
  case ASTTypeUnaryOp:
    return new ASTIntNode(ASTIdentifierNode::Int.Clone(), UOP, 32, true);
    break;
  default: {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTIntNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                       M.str());
  }
    break;
  }

  return nullptr;
}

ASTFloatNode*
ASTCastExpressionNode::CastToFloat() const {
  if (IsBadCast()) {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTFloatNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                         M.str());
  }

  switch (CastFromType) {
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
  case ASTTypeMPComplex:
    return new ASTFloatNode(ASTIdentifierNode::Float.Clone(),
                            MPC->GetRealAsMPDecimal());
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
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTFloatNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                         M.str());
  }
     break;
  }

  return nullptr;
}

ASTDoubleNode*
ASTCastExpressionNode::CastToDouble() const {
  if (IsBadCast()) {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTDoubleNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                          M.str());
  }

  switch (CastFromType) {
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
  case ASTTypeMPComplex:
    return new ASTDoubleNode(ASTIdentifierNode::Double.Clone(),
                             MPC->GetRealAsMPDecimal());
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
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTDoubleNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                          M.str());
  }
     break;
  }

  return nullptr;
}

ASTMPIntegerNode*
ASTCastExpressionNode::CastToMPInteger() const {
  if (IsBadCast()) {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTMPIntegerNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                             M.str());
  }

  ASTIntNode* I = nullptr;

  switch (CastFromType) {
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
  case ASTTypeMPComplex:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Signed,
                                MPC->GetRealBits(), MPC->GetRealAsMPDecimal());
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
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Signed, 64, BOP);
     break;
  case ASTTypeUnaryOp:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Signed, 64, UOP);
     break;
  default: {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTMPIntegerNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                             M.str());
  }
     break;
  }

  return nullptr;
}

ASTMPIntegerNode*
ASTCastExpressionNode::CastToMPUInteger() const {
  if (IsBadCast()) {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTMPIntegerNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                             M.str());
  }

  ASTIntNode* I = nullptr;

  switch (CastFromType) {
  case ASTTypeBool:
    I = new ASTIntNode(static_cast<uint32_t>(Bool->GetValue()));
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), I, 8);
    break;
  case ASTTypeInt:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Int,
                                Int->GetBits());
    break;
  case ASTTypeFloat:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Unsigned,
                                Float->GetBits(), Float);
    break;
  case ASTTypeDouble:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Unsigned,
                                Double->GetBits(), Double);
    break;
  case ASTTypeMPInteger:
    return const_cast<ASTMPIntegerNode*>(MPI);
    break;
  case ASTTypeMPDecimal:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Unsigned,
                                MPD->GetBits(), MPD);
    break;
  case ASTTypeMPComplex:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Unsigned,
                                MPC->GetRealBits(), MPC->GetRealAsMPDecimal());
    break;
  case ASTTypeBitset:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Unsigned,
                                CBit->Size(), CBit->AsString().c_str(), 2);
    break;
  case ASTTypeAngle:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(), Unsigned,
                                Angle->GetBits(),
                                Angle->GetValue(2).c_str(), 2);
    break;
  case ASTTypeBinaryOp:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(),
                                Unsigned, 64, BOP);
    break;
  case ASTTypeUnaryOp:
    return new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(),
                                Unsigned, 64, UOP);
    break;
  default: {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTMPIntegerNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                             M.str());
  }
    break;
  }

  return nullptr;
}

ASTMPDecimalNode*
ASTCastExpressionNode::CastToMPDecimal() const {
  if (IsBadCast()) {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTMPDecimalNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                             M.str());
  }

  switch (CastFromType) {
  case ASTTypeBool:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(), 8, Bool);
    break;
  case ASTTypeInt:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(), Int->GetBits(),
                                Int);
    break;
  case ASTTypeFloat:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(), Float->GetBits(),
                                Float);
    break;
  case ASTTypeDouble:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(), Double->GetBits(),
                                Double);
    break;
  case ASTTypeMPInteger:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(), MPI->GetBits(),
                                MPI);
    break;
  case ASTTypeMPDecimal:
    return const_cast<ASTMPDecimalNode*>(MPD);
    break;
  case ASTTypeMPComplex:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(), MPC->GetRealBits(),
                                MPC->GetRealAsMPDecimal());
    break;
  case ASTTypeBitset:
    return new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(), CBit->Size(), CBit);
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
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTMPDecimalNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                             M.str());
  }
     break;
  }

  return nullptr;
}

ASTMPComplexNode*
ASTCastExpressionNode::CastToMPComplex() const {
  if (IsBadCast()) {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTMPComplexNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                             M.str());
  }

  switch (CastFromType) {
  case ASTTypeInt: {
    ASTMPIntegerNode* RI = new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(),
                                                Int, Int->GetBits());
    assert(RI && "Could not create a valid ASTMPIntegerNode!");
    ASTMPIntegerNode* II = new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(),
                                                Signed, ASTIntNode::IntBits,
                                                "1");
    assert(II && "Could not create a valid ASTMPIntegerNode!");
    return new ASTMPComplexNode(ASTIdentifierNode::MPComplex.Clone(), RI, II,
                                ASTOpTypeMul, ASTIntNode::IntBits);
  }
    break;
  case ASTTypeFloat: {
    ASTMPDecimalNode* RD = new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                                                Float->GetBits(), Float->GetValue());
    assert(RD && "Could not create a valid ASTMPDecimalNode!");
    ASTMPDecimalNode* ID = new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                                                Float->GetBits(), double(1.0));
    assert(ID && "Could not create a valid ASTMPDecimalNode!");
    return new ASTMPComplexNode(ASTIdentifierNode::MPComplex.Clone(), RD, ID,
                                ASTOpTypeMul, Float->GetBits());
  }
    break;
  case ASTTypeDouble: {
    ASTMPDecimalNode* RD = new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                                                Double->GetBits(), Double->GetValue());
    assert(RD && "Could not create a valid ASTMPDecimalNode!");
    ASTMPDecimalNode* ID = new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                                                Double->GetBits(), double(1.0));
    assert(ID && "Could not create a valid ASTMPDecimalNode!");
    return new ASTMPComplexNode(ASTIdentifierNode::MPComplex.Clone(), RD, ID,
                                ASTOpTypeMul, Double->GetBits());
  }
    break;
  case ASTTypeMPInteger: {
    ASTMPIntegerNode* II = new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(),
                                                Signed, MPI->GetBits(), "1");
    assert(II && "Could not create a valid ASTMPIntegerNode!");
    return new ASTMPComplexNode(ASTIdentifierNode::MPComplex.Clone(), MPI, II,
                                ASTOpTypeMul, MPI->GetBits());
  }
    break;
  case ASTTypeMPDecimal: {
    ASTMPDecimalNode* ID = new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                                                MPD->GetBits(), double(1.0));
    assert(ID && "Could not create a valid ASTMPDecimalNode!");
    return new ASTMPComplexNode(ASTIdentifierNode::MPComplex.Clone(), MPD, ID,
                                ASTOpTypeMul, MPD->GetBits());
  }
    break;
  case ASTTypeMPComplex:
    return const_cast<ASTMPComplexNode*>(MPC);
    break;
  case ASTTypeAngle: {
    ASTMPDecimalNode* ID = new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                                                Angle->GetBits(), double(1.0));
    assert(ID && "Could not create a valid ASTMPDecimalNode!");
    return new ASTMPComplexNode(ASTIdentifierNode::MPComplex.Clone(),
                                Angle->AsMPDecimal(), ID, ASTOpTypeMul,
                                Angle->GetBits());
  }
    break;
  case ASTTypeBinaryOp: {
    ASTComplexExpressionNode* CEX =
      new ASTComplexExpressionNode(ASTIdentifierNode::MPComplex.Clone(), BOP);
    assert(CEX && "Could not create a valid ASTComplexExpressionNode!");
    return new ASTMPComplexNode(ASTIdentifierNode::MPComplex.Clone(), CEX,
                                ASTMPComplexNode::DefaultBits);
  }
    break;
  case ASTTypeUnaryOp: {
    ASTComplexExpressionNode* CEX =
      new ASTComplexExpressionNode(ASTIdentifierNode::MPComplex.Clone(), UOP);
    assert(CEX && "Could not create a valid ASTComplexExpressionNode!");
    return new ASTMPComplexNode(ASTIdentifierNode::MPComplex.Clone(), CEX,
                                ASTMPComplexNode::DefaultBits);
  }
    break;
  default: {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTMPComplexNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                             M.str());
  }
    break;
  }

  return nullptr;
}

ASTCBitNode*
ASTCastExpressionNode::CastToBitset() const {
  if (IsBadCast()) {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTCBitNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                        M.str());
  }

  std::stringstream S;

  switch (CastFromType) {
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
  case ASTTypeMPComplex:
    S << MPC->GetRealAsMPDecimal()->GetValue(2);
    return new ASTCBitNode(ASTIdentifierNode::Bitset.Clone(),
                           MPC->GetRealBits(), S.str());
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
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTCBitNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                        M.str());
  }
     break;
  }

  return nullptr;
}

ASTAngleNode*
ASTCastExpressionNode::CastToAngle() const {
  if (IsBadCast()) {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTAngleNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                         M.str());
  }

  switch (CastFromType) {
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
  case ASTTypeMPComplex:
    return new ASTAngleNode(ASTIdentifierNode::Angle.Clone(),
                            MPC->GetRealAsMPDecimal(),
                            ASTAngleTypeGeneric, MPC->GetRealBits());
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
    M << "Impossible cast from " << PrintTypeEnum(CastFromType)
      << " to " << PrintTypeEnum(CastToType) << " requested.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(), DiagLevel::Error);
    return ASTAngleNode::ExpressionError(ASTIdentifierNode::BadCast.Clone(),
                                         M.str());
  }
     break;
  }

  return nullptr;
}

void
ASTCastExpressionNode::Mangle() {
  if (GetPolymorphicName().empty())
    const_cast<ASTIdentifierNode*>(GetIdentifier())->SetPolymorphicName("cast");

  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetPolymorphicName());
  M.Underscore();

  switch (CastFromType) {
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

  M.TypeSize(CastToType, Bits);
  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString(),
                                                                  true);
}

} // namespace QASM

