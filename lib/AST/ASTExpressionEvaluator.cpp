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

#include <qasm/AST/ASTExpressionEvaluator.h>
#include <qasm/AST/ASTExpressionValidator.h>
#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTAngleContextControl.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <iostream>
#include <cassert>

namespace QASM {

ASTExpressionEvaluator ASTExpressionEvaluator::EXE;
std::map<ASTType, unsigned> ASTExpressionEvaluator::RM;
std::map<unsigned, ASTType> ASTExpressionEvaluator::TM;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

void
ASTExpressionEvaluator::Init() {
  if (RM.empty()) {
    RM = {
      { ASTTypeChar, 0U },
      { ASTTypeUTF8, 0U },
      { ASTTypeBool, 1U },
      { ASTTypeBitset, 2U },
      { ASTTypeInt,  2U },
      { ASTTypeUInt, 3U },
      { ASTTypeMPInteger, 4U },
      { ASTTypeMPUInteger, 5U },
      { ASTTypeFloat, 6U },
      { ASTTypeDouble, 7U },
      { ASTTypeLongDouble, 8U },
      { ASTTypeMPDecimal, 9U },
      { ASTTypeMPComplex, 10U },
      { ASTTypeAngle, 100U },
      { ASTTypeDuration, 101U },
      { ASTTypeStretch, 102U },
      { ASTTypeTimeUnit, 103U },
      { ASTTypeOpenPulseFrame, 200U },
      { ASTTypeOpenPulsePort, 201U },
      { ASTTypeOpenPulseWaveform, 202U },
    };
  }

  if (TM.empty()) {
    TM = {
      { 0U, ASTTypeChar },
      { 1U, ASTTypeBool },
      { 2U, ASTTypeInt },
      { 3U, ASTTypeUInt },
      { 4U, ASTTypeMPInteger },
      { 5U, ASTTypeMPUInteger },
      { 6U, ASTTypeFloat },
      { 7U, ASTTypeDouble },
      { 8U, ASTTypeLongDouble },
      { 9U, ASTTypeMPDecimal },
      { 10U, ASTTypeMPComplex },
      { 100U, ASTTypeAngle },
      { 101U, ASTTypeDuration },
      { 102U, ASTTypeStretch },
      { 103U, ASTTypeTimeUnit },
      { 200U, ASTTypeOpenPulseFrame },
      { 201U, ASTTypeOpenPulsePort },
      { 202U, ASTTypeOpenPulseWaveform },
    };
  }
}

ASTType
ASTExpressionEvaluator::EvaluatesTo(const ASTBinaryOpNode* BOp) const {
  assert(BOp && "Invalid ASTBinaryOpNode argument!");

  ASTType LTy = BOp->GetLeft()->GetASTType();
  ASTType RTy = BOp->GetRight()->GetASTType();

  if (LTy == ASTTypeOpTy) {
    if (const ASTOperatorNode* OPN =
        dynamic_cast<const ASTOperatorNode*>(BOp->GetLeft())) {
      if (OPN->IsIdentifier())
        LTy = OPN->GetTargetIdentifier()->GetSymbolType();
      else
        LTy = OPN->GetTargetExpression()->GetASTType();
    } else {
      LTy = ASTTypeUndefined;
    }
  }

  if (LTy == ASTTypeOpndTy) {
    if (const ASTOperandNode* OPD =
        dynamic_cast<const ASTOperandNode*>(BOp->GetLeft())) {
      if (OPD->IsIdentifier())
        LTy = OPD->GetIdentifier()->GetSymbolType();
      else
        LTy = OPD->GetExpression()->GetASTType();
    } else {
      LTy = ASTTypeUndefined;
    }
  }

  if (RTy == ASTTypeOpTy) {
    if (const ASTOperatorNode* OPN =
        dynamic_cast<const ASTOperatorNode*>(BOp->GetRight())) {
      if (OPN->IsIdentifier())
        RTy = OPN->GetTargetIdentifier()->GetSymbolType();
      else
        RTy = OPN->GetTargetExpression()->GetASTType();
    } else {
      RTy = ASTTypeUndefined;
    }
  }

  if (RTy == ASTTypeOpndTy) {
    if (const ASTOperandNode* OPD =
        dynamic_cast<const ASTOperandNode*>(BOp->GetRight())) {
      if (OPD->IsIdentifier())
        RTy = OPD->GetIdentifier()->GetSymbolType();
      else
        RTy = OPD->GetExpression()->GetASTType();
    } else {
      RTy = ASTTypeUndefined;
    }
  }

  if (RTy == ASTTypeOpTy)
    RTy = dynamic_cast<const ASTOperatorNode*>(BOp->GetRight())->GetTargetType();
  if (RTy == ASTTypeOpndTy)
    RTy = dynamic_cast<const ASTOperandNode*>(BOp->GetRight())->GetTargetType();

  if (LTy == ASTTypeBinaryOp) {
    if (BOp->GetLeft()->GetASTType() == ASTTypeOpTy)
      LTy = EvaluatesTo(dynamic_cast<const ASTBinaryOpNode*>(
          dynamic_cast<const ASTOperatorNode*>(BOp->GetLeft())->GetTargetExpression()));
    else if (BOp->GetLeft()->GetASTType() == ASTTypeOpndTy)
      LTy = EvaluatesTo(dynamic_cast<const ASTBinaryOpNode*>(
          dynamic_cast<const ASTOperandNode*>(BOp->GetLeft())->GetExpression()));
    else
      LTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTBinaryOpNode*>(BOp->GetLeft()));
  } else if (LTy == ASTTypeUnaryOp) {
    if (BOp->GetLeft()->GetASTType() == ASTTypeOpTy)
      LTy = EvaluatesTo(dynamic_cast<const ASTUnaryOpNode*>(
          dynamic_cast<const ASTOperatorNode*>(BOp->GetLeft())->GetTargetExpression()));
    else if (BOp->GetLeft()->GetASTType() == ASTTypeOpndTy)
      LTy = EvaluatesTo(dynamic_cast<const ASTUnaryOpNode*>(
          dynamic_cast<const ASTOperandNode*>(BOp->GetLeft())->GetExpression()));
    else
      LTy = EvaluatesTo(dynamic_cast<const ASTUnaryOpNode*>(BOp->GetLeft()));
  } else if (LTy == ASTTypeIdentifier) {
    if (BOp->GetLeft()->GetASTType() == ASTTypeOpTy)
      LTy = dynamic_cast<const ASTOperatorNode*>(
        BOp->GetLeft())->GetTargetIdentifier()->GetSymbolType();
    else
      LTy = BOp->GetLeft()->GetIdentifier()->GetSymbolType();
  } else if (LTy == ASTTypeOpTy) {
    LTy = dynamic_cast<const ASTOperatorNode*>(BOp->GetLeft())->GetTargetType();
  } else if (LTy == ASTTypeOpndTy) {
    LTy = dynamic_cast<const ASTOperandNode*>(BOp->GetLeft())->GetTargetType();
  }

  if (RTy == ASTTypeBinaryOp) {
    if (BOp->GetRight()->GetASTType() == ASTTypeOpTy)
      RTy = EvaluatesTo(dynamic_cast<const ASTBinaryOpNode*>(
          dynamic_cast<const ASTOperatorNode*>(BOp->GetRight())->GetTargetExpression()));
    else if (BOp->GetRight()->GetASTType() == ASTTypeOpndTy)
      RTy = EvaluatesTo(dynamic_cast<const ASTBinaryOpNode*>(
          dynamic_cast<const ASTOperandNode*>(BOp->GetRight())->GetExpression()));
    else
      RTy = EvaluatesTo(dynamic_cast<const ASTBinaryOpNode*>(BOp->GetRight()));
  } else if (RTy == ASTTypeUnaryOp) {
    if (BOp->GetRight()->GetASTType() == ASTTypeOpTy)
      RTy = EvaluatesTo(dynamic_cast<const ASTUnaryOpNode*>(
          dynamic_cast<const ASTOperatorNode*>(BOp->GetRight())->GetTargetExpression()));
    else if (BOp->GetRight()->GetASTType() == ASTTypeOpndTy)
      RTy = EvaluatesTo(dynamic_cast<const ASTUnaryOpNode*>(
          dynamic_cast<const ASTOperandNode*>(BOp->GetRight())->GetExpression()));
    else
      RTy = EvaluatesTo(dynamic_cast<const ASTUnaryOpNode*>(BOp->GetRight()));
  } else if (RTy == ASTTypeIdentifier) {
    if (BOp->GetRight()->GetASTType() == ASTTypeOpTy)
      RTy = dynamic_cast<const ASTOperatorNode*>(
        BOp->GetRight())->GetTargetIdentifier()->GetSymbolType();
    else
      RTy = BOp->GetRight()->GetIdentifier()->GetSymbolType();
  } else if (RTy == ASTTypeOpTy) {
    RTy = dynamic_cast<const ASTOperatorNode*>(BOp->GetRight())->GetTargetType();
  } else if (RTy == ASTTypeOpndTy) {
    RTy = dynamic_cast<const ASTOperandNode*>(BOp->GetRight())->GetTargetType();
  } else if (RTy == ASTTypeFunctionCall) {
    if (const ASTFunctionCallNode* FC =
        dynamic_cast<const ASTFunctionCallNode*>(BOp->GetRight())) {
      RTy = FC->GetResult()->GetResultType();
    }
  }

  assert(LTy != ASTTypeIdentifier && "LHS Type is ASTTypeIdentifier!");
  assert(RTy != ASTTypeIdentifier && "RHS Type is ASTTypeIdentifier!");

  assert(LTy != ASTTypeOpTy && "LHS Type is an ASTTypeOpTy!");
  assert(RTy != ASTTypeOpTy && "RHS Type is an ASTTypeOpTy!");

  assert(LTy != ASTTypeOpndTy && "LHS Type is an ASTTypeOpndTy!");
  assert(RTy != ASTTypeOpndTy && "RHS Type is an ASTTypeOpndTy!");

  unsigned LR = GetRank(LTy);
  unsigned RR = GetRank(RTy);

  if (LR != RR &&
      (LR == static_cast<unsigned>(~0x0) || RR == static_cast<unsigned>(~0x0))) {
    std::stringstream M;
    M << "Illegal arithmetic expression " << PrintTypeEnum(LTy) << ' '
      << PrintOpTypeEnum(BOp->GetOpType()) << ' ' << PrintTypeEnum(RTy) << '.';
    const ASTIdentifierNode* Id = BOp->GetIdentifier();
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return ASTTypeUndefined;
  }

  if (LR != RR && (LTy == ASTTypeAngle || RTy == ASTTypeAngle)) {
    switch (BOp->GetOpType()) {
    case ASTOpTypeAdd:
    case ASTOpTypeSub:
    case ASTOpTypeMul:
    case ASTOpTypeDiv:
      if (LTy == ASTTypeAngle &&
          ASTExpressionValidator::Instance().IsUnPromotedScalarType(RTy)) {
        if (ASTAngleContextControl::Instance().InOpenContext())
          return ASTTypeAngle;

        return ASTTypeDouble;
      } else if (RTy == ASTTypeAngle &&
               ASTExpressionValidator::Instance().IsUnPromotedScalarType(LTy)) {
        if (ASTAngleContextControl::Instance().InOpenContext())
          return ASTTypeAngle;

        return ASTTypeDouble;
      } else {
        std::stringstream M;
        M << "Illegal arithmetic expression " << PrintTypeEnum(LTy) << ' '
          << PrintOpTypeEnum(BOp->GetOpType()) << ' ' << PrintTypeEnum(RTy)
          << '.';
        const ASTIdentifierNode* Id = BOp->GetIdentifier();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
        return ASTTypeUndefined;
      }
      break;
    default: {
      std::stringstream M;
      M << "Illegal arithmetic expression " << PrintTypeEnum(LTy) << ' '
        << PrintOpTypeEnum(BOp->GetOpType()) << ' ' << PrintTypeEnum(RTy)
        << '.';
      const ASTIdentifierNode* Id = BOp->GetIdentifier();
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return ASTTypeUndefined;
    }
      break;
    }
  }

  if (LR == RR && LTy == ASTTypeAngle) {
    switch (BOp->GetOpType()) {
    case ASTOpTypeAdd:
    case ASTOpTypeSub:
    case ASTOpTypeMul:
    case ASTOpTypeDiv:
      return ASTTypeAngle;
      break;
    default: {
      std::stringstream M;
      M << "Illegal arithmetic expression " << PrintTypeEnum(LTy) << ' '
        << PrintOpTypeEnum(BOp->GetOpType()) << ' ' << PrintTypeEnum(RTy)
        << '.';
      const ASTIdentifierNode* Id = BOp->GetIdentifier();
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return ASTTypeUndefined;
    }
      break;
    }
  }

  if (LR == RR && LR == static_cast<unsigned>(~0x0)) {
    std::stringstream M;
    M << "Illegal arithmetic expression " << PrintTypeEnum(LTy) << ' '
      << PrintOpTypeEnum(BOp->GetOpType()) << ' ' << PrintTypeEnum(RTy) << '.';
    const ASTIdentifierNode* Id = BOp->GetIdentifier();
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return ASTTypeUndefined;
  }

  if (LR == RR)
    return GetType(LR);

  return GetType(std::max(LR, RR));
}

ASTType
ASTExpressionEvaluator::EvaluatesTo(const ASTUnaryOpNode* UOp) const {
  assert(UOp && "Invalid ASTUnaryOpNode argument!");

  ASTType ETy = UOp->GetExpression()->GetASTType();

  if (ETy == ASTTypeBinaryOp) {
    ETy = EvaluatesTo(dynamic_cast<const ASTBinaryOpNode*>(UOp->GetExpression()));
  } else if (ETy == ASTTypeUnaryOp) {
    ETy = EvaluatesTo(dynamic_cast<const ASTUnaryOpNode*>(UOp->GetExpression()));
  } else if (ETy == ASTTypeIdentifier) {
    ETy = UOp->GetExpression()->GetIdentifier()->GetSymbolType();
  } else if (ETy == ASTTypeOpTy) {
    if (const ASTOperatorNode* OPN =
        dynamic_cast<const ASTOperatorNode*>(UOp->GetExpression())) {
      if (OPN->IsIdentifier())
        ETy = OPN->GetTargetIdentifier()->GetSymbolType();
      else
        ETy = OPN->GetTargetExpression()->GetASTType();
    } else {
      ETy = ASTTypeUndefined;
    }
  } else if (ETy == ASTTypeOpndTy) {
    if (const ASTOperandNode* OPD =
        dynamic_cast<const ASTOperandNode*>(UOp->GetExpression())) {
      if (OPD->IsIdentifier())
        ETy = OPD->GetIdentifier()->GetSymbolType();
      else
        ETy = OPD->GetExpression()->GetASTType();
    } else {
      ETy = ASTTypeUndefined;
    }
  }

  assert(ETy != ASTTypeUndefined && "Expression Type is ASTTypeUndefined!");
  assert(ETy != ASTTypeOpTy && "Expression Type is ASTTypeOpTy!");
  assert(ETy != ASTTypeOpndTy && "Expression Type is ASTTypeOpndTy!");
  assert(ETy != ASTTypeIdentifier && "Expression Type is ASTTypeIdentifier!");

  switch (UOp->GetOpType()) {
  case ASTOpTypeBinaryLeftFold:
  case ASTOpTypeBinaryRightFold:
  case ASTOpTypeUnaryLeftFold:
  case ASTOpTypeUnaryRightFold:
    return ASTTypeUndefined;
    break;
  case ASTOpTypeLogicalNot:
    return ASTTypeBool;
    break;
  case ASTOpTypeRotation:
  case ASTOpTypeRotl:
  case ASTOpTypeRotr:
  case ASTOpTypePopcount:
    if (ASTExpressionValidator::Instance().IsUnPromotedIntegerType(ETy))
      return ETy;
    break;
  case ASTOpTypeSin:
  case ASTOpTypeCos:
  case ASTOpTypeTan:
  case ASTOpTypeArcSin:
  case ASTOpTypeArcCos:
  case ASTOpTypeArcTan:
  case ASTOpTypeExp:
  case ASTOpTypeLn:
  case ASTOpTypeSqrt:
    if (ASTExpressionValidator::Instance().IsUnPromotedIntegerType(ETy) ||
        ASTExpressionValidator::Instance().IsFloatingPointType(ETy))
      return ASTTypeDouble;
    else if (ASTExpressionValidator::Instance().IsComplexType(ETy))
      return ASTTypeMPComplex;
    else if (ASTExpressionValidator::Instance().IsAngleType(ETy))
      return ASTTypeAngle;
    break;
  case ASTOpTypeNegative:
  case ASTOpTypePositive:
    if (ASTExpressionValidator::Instance().CanDoArithmeticNegPos(ETy))
      return ETy;
    break;
  default:
    break;
  }

  return ASTTypeUndefined;
}

ASTType
ASTExpressionEvaluator::EvaluatesTo(const ASTCastExpressionNode* XOp) const {
  assert(XOp && "Invalid ASTCastExpressionNode argument!");

  ASTType TTy = XOp->GetCastTo();
  ASTType FTy = XOp->GetCastFrom();

  if (FTy == ASTTypeBinaryOp)
    FTy = ASTExpressionEvaluator::Instance().EvaluatesTo(XOp->GetBinaryOp());
  else if (FTy == ASTTypeUnaryOp)
    FTy = ASTExpressionEvaluator::Instance().EvaluatesTo(XOp->GetUnaryOp());

  assert(FTy != ASTTypeIdentifier && "From Type is ASTTypeIdentifier!");
  assert(FTy != ASTTypeUndefined && "From Type is ASTTypeUndefined!");

  switch (TTy) {
    case ASTTypeBool:
    case ASTTypeChar:
    case ASTTypeUTF8:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeLongDouble:
    case ASTTypeMPDecimal:
      if (ASTExpressionValidator::Instance().IsIntegerType(FTy) ||
          ASTExpressionValidator::Instance().IsFloatingPointType(FTy))
        return TTy;
      break;
    case ASTTypeAngle:
      if (ASTExpressionValidator::Instance().IsAngleType(FTy) ||
          ASTExpressionValidator::Instance().IsUnPromotedScalarType(FTy))
        return TTy;
      break;
    case ASTTypeMPComplex:
      if (ASTExpressionValidator::Instance().IsComplexType(FTy))
        return TTy;
      break;
    default:
      break;
  }

  return ASTTypeUndefined;
}

ASTType
ASTExpressionEvaluator::EvaluatesTo(const ASTImplicitConversionNode* ICX) const {
  assert(ICX && "Invalid ASTImplicitConversionNode argument!");

  return ICX->IsValidConversion() ? ICX->GetConvertTo() : ASTTypeUndefined;
}

} // namespace QASM

