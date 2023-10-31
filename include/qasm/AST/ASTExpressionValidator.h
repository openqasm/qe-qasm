/* -*- coding: utf-8 -*-
 *
 * Copyright 2023 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_EXPRESSION_VALIDATOR_H
#define __QASM_AST_EXPRESSION_VALIDATOR_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTCastExpr.h>
#include <qasm/AST/ASTExpressionEvaluator.h>

namespace QASM {

class ASTExpressionValidator {
private:
  static ASTExpressionValidator EXV;

protected:
  ASTExpressionValidator() = default;

protected:
  ASTType ResolveOperatorType(const ASTOperatorNode* Op,
                              ASTType Ty) const;

  ASTType ResolveOperandType(const ASTOperandNode* Op,
                             ASTType Ty) const;

public:
  static ASTExpressionValidator& Instance() {
    return EXV;
  }

  bool Validate(const ASTBinaryOpNode* BOp) const;

  bool Validate(const ASTUnaryOpNode* UOp) const;

  bool Validate(const ASTCastExpressionNode* XOp) const;

  void ValidateLogicalNot(const ASTUnaryOpNode* UOp) const;

  bool IsIntegerType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeBitset:
    case ASTTypeBool:
    case ASTTypeChar:
    case ASTTypeUTF8:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeRotateExpr:
    case ASTTypeRotateStatement:
    case ASTTypePopcountExpr:
    case ASTTypePopcountStmt:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsScalarIntegerType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeBitset:
    case ASTTypeBool:
    case ASTTypeChar:
    case ASTTypeUTF8:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsSimpleIntegerType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeBool:
    case ASTTypeInt:
    case ASTTypeUInt:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsUnPromotedIntegerType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeRotateExpr:
    case ASTTypeRotateStatement:
    case ASTTypePopcountExpr:
    case ASTTypePopcountStmt:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsFloatingPointType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeLongDouble:
    case ASTTypeMPDecimal:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsSimpleFloatingPointType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeLongDouble:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsScalarType(ASTType Ty) const {
    return IsIntegerType(Ty) || IsFloatingPointType(Ty);
  }

  bool IsUnPromotedScalarType(ASTType Ty) const {
    return IsUnPromotedIntegerType(Ty) || IsFloatingPointType(Ty);
  }

  bool IsComplexType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeMPComplex:
    case ASTTypeComplexExpression:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsNumericType(ASTType Ty) const {
    return IsIntegerType(Ty) || IsFloatingPointType(Ty) ||
           IsComplexType(Ty);
  }

  bool IsAngleType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeAngle:
    case ASTTypeEulerAngle:
    case ASTTypeLambdaAngle:
    case ASTTypePhiAngle:
    case ASTTypePiAngle:
    case ASTTypeTauAngle:
    case ASTTypeThetaAngle:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsTimeType(ASTType Ty) const {
    return Ty == ASTTypeTimeUnit;
  }

  bool IsStringType(ASTType Ty) const {
    return Ty == ASTTypeStringLiteral;
  }

  bool IsFunctionType(ASTType Ty) const {
    return Ty == ASTTypeFunction ||
           Ty == ASTTypeFunctionCall;
  }

  bool IsQubitType(ASTType Ty) const {
    return Ty == ASTTypeQubit ||
           Ty == ASTTypeQubitContainer ||
           Ty == ASTTypeQubitContainerAlias ||
           Ty == ASTTypeGateQubitParam;
  }

  bool IsQuantumArithmeticType(ASTType Ty) const {
    return Ty == ASTTypeDuration ||
           Ty == ASTTypeStretch;
  }

  bool IsArithmeticType(ASTType Ty) const {
    return IsNumericType(Ty) || IsAngleType(Ty) || IsTimeType(Ty);
  }

  bool IsArithmeticType(const ASTBinaryOpNode* BOP) const {
    assert(BOP && "Invalid ASTBinaryOpNode argument!");
    ASTType BTy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
    return IsArithmeticType(BTy);
  }

  bool IsArithmeticType(const ASTUnaryOpNode* UOP) const {
    assert(UOP && "Invalid ASTUnaryOpNode argument!");
    ASTType UTy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
    return IsArithmeticType(UTy);
  }

  bool IsArithmeticType(const ASTExpressionNode* EXN) const {
    assert(EXN && "Invalid ASTExpressionNode argument!");
    if (EXN->GetASTType() == ASTTypeIdentifier ||
        EXN->GetASTType() == ASTTypeIdentifierRef)
      return IsArithmeticType(EXN->GetIdentifier()->GetSymbolType());

    return IsArithmeticType(EXN->GetASTType());
  }

  bool CanBeConst(ASTType Ty) const {
    return IsNumericType(Ty) || IsAngleType(Ty) ||
           IsTimeType(Ty) || IsStringType(Ty);
  }

  bool CanBeConst(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return CanBeConst(Id->GetSymbolType());
  }

  bool CanDoArithmeticNegPos(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeChar:
    case ASTTypeInt:
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeMPInteger:
    case ASTTypeMPDecimal:
    case ASTTypeMPComplex:
    case ASTTypeAngle:
    case ASTTypeEulerAngle:
    case ASTTypeLambdaAngle:
    case ASTTypePhiAngle:
    case ASTTypePiAngle:
    case ASTTypeTauAngle:
    case ASTTypeThetaAngle:
    case ASTTypeGateAngleParam:
    case ASTTypeGateGPhaseExpression:
    case ASTTypeGateGPhaseStatement:
    case ASTTypeGPhaseExpression:
    case ASTTypeGPhaseStatement:
    case ASTTypeExp:
    case ASTTypeLn:
    case ASTTypePi:
    case ASTTypePow:
    case ASTTypeSqrt:
    case ASTTypeGatePower:
    case ASTTypeGatePowerStatement:
    case ASTTypePopcountExpr:
    case ASTTypeRotl:
    case ASTTypeRotr:
    case ASTTypeRotateExpr:
    case ASTTypeRotateStatement:
    case ASTTypeSin:
    case ASTTypeCos:
    case ASTTypeTan:
    case ASTTypeArcSin:
    case ASTTypeArcCos:
    case ASTTypeArcTan:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsAssignmentOp(ASTOpType OTy) const {
    switch (OTy) {
    case ASTOpTypeAddAssign:
    case ASTOpTypeSubAssign:
    case ASTOpTypeMulAssign:
    case ASTOpTypeDivAssign:
    case ASTOpTypeModAssign:
    case ASTOpTypeBitAndAssign:
    case ASTOpTypeBitOrAssign:
    case ASTOpTypeXorAssign:
    case ASTOpTypeLeftShiftAssign:
    case ASTOpTypeRightShiftAssign:
    case ASTOpTypePreInc:
    case ASTOpTypePreDec:
    case ASTOpTypePostInc:
    case ASTOpTypePostDec:
    case ASTOpTypeAssign:
      return true;
      break;
    default:
      return false;
      break;
    }
  }

  bool IsAssignableType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeAngle:
    case ASTTypeBitset:
    case ASTTypeMPComplex:
    case ASTTypeDuration:
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeLongDouble:
    case ASTTypeMPDecimal:
    case ASTTypeBool:
    case ASTTypeChar:
    case ASTTypeUTF8:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeStretch:
    case ASTTypeOpenPulseFrame:
    case ASTTypeOpenPulsePort:
    case ASTTypeOpenPulseWaveform:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsAssignableType(const ASTExpressionNode* EXN) const {
    assert(EXN && "Invalid ASTExpressionNode argument!");

    switch (EXN->GetASTType()) {
    case ASTTypeIdentifier:
    case ASTTypeIdentifierRef: {
      const ASTIdentifierNode* XId = EXN->GetIdentifier();
      assert(XId && "Invalid ASTIdentifierNode contained in an ASTExpressionNode!");
      return IsAssignableType(XId->GetSymbolType());
    }
      break;
    case ASTTypeUnaryOp: {
      if (const ASTUnaryOpNode* UOP = dynamic_cast<const ASTUnaryOpNode*>(EXN)) {
        ASTType ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
        return IsAssignableType(ETy);
      }

      return false;
    }
      break;
    case ASTTypeBinaryOp: {
      if (const ASTBinaryOpNode* BOP = dynamic_cast<const ASTBinaryOpNode*>(EXN)) {
        ASTType ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
        return IsAssignableType(ETy);
      }

      return false;
    }
      break;
    default:
      break;
    }

    return IsAssignableType(EXN->GetASTType());
  }

  bool IsAssignableType(const ASTIdentifierNode* XId) const {
    assert(XId && "Invalid ASTIdentifierNode argument!");
    return IsAssignableType(XId->GetSymbolType());
  }

  bool IsAssignableType(const ASTIdentifierRefNode* XId) const {
    assert(XId && "Invalid ASTIdentifierNode argument!");
    return IsAssignableType(XId->GetSymbolType());
  }

  bool IsLogicallyNegateType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeAngle:
    case ASTTypeBitset:
    case ASTTypeMPComplex:
    case ASTTypeDuration:
    case ASTTypeDurationOf:
    case ASTTypeRotateExpr:
    case ASTTypePopcountExpr:
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeLongDouble:
    case ASTTypeMPDecimal:
    case ASTTypeBool:
    case ASTTypeChar:
    case ASTTypeUTF8:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeStretch:
    case ASTTypeOpenPulseFrame:
    case ASTTypeOpenPulsePort:
    case ASTTypeOpenPulseWaveform:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsLogicallyNegateType(const ASTExpressionNode* EXN) const {
    assert(EXN && "Invalid ASTExpressionNode argument!");

    switch (EXN->GetASTType()) {
    case ASTTypeIdentifier:
    case ASTTypeIdentifierRef: {
      const ASTIdentifierNode* XId = EXN->GetIdentifier();
      assert(XId && "Invalid ASTIdentifierNode contained in an ASTExpressionNode!");
      return IsLogicallyNegateType(XId->GetSymbolType());
    }
      break;
    case ASTTypeUnaryOp: {
      if (const ASTUnaryOpNode* UOP = dynamic_cast<const ASTUnaryOpNode*>(EXN)) {
        ASTType ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
        return IsLogicallyNegateType(ETy);
      }

      return false;
    }
      break;
    case ASTTypeBinaryOp: {
      if (const ASTBinaryOpNode* BOP = dynamic_cast<const ASTBinaryOpNode*>(EXN)) {
        ASTType ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
        return IsLogicallyNegateType(ETy);
      }

      return false;
    }
      break;
    default:
      break;
    }

    return IsLogicallyNegateType(EXN->GetASTType());
  }

  bool IsLogicallyNegateType(const ASTIdentifierNode* XId) const {
    assert(XId && "Invalid ASTIdentifierNode argument!");
    return IsLogicallyNegateType(XId->GetSymbolType());
  }

  bool IsLogicallyNegateType(const ASTIdentifierRefNode* XId) const {
    assert(XId && "Invalid ASTIdentifierNode argument!");
    return IsLogicallyNegateType(XId->GetSymbolType());
  }

  bool IsReturningType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeFunction:
    case ASTTypeFunctionCall:
    case ASTTypeMeasure:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsAssignableToComplexType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeMPComplex:
    case ASTTypeMPDecimal:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeFloat:
    case ASTTypeDouble:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsArrayType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeCBitArray:
    case ASTTypeQubitArray:
    case ASTTypeAngleArray:
    case ASTTypeBoolArray:
    case ASTTypeIntArray:
    case ASTTypeMPIntegerArray:
    case ASTTypeFloatArray:
    case ASTTypeMPDecimalArray:
    case ASTTypeMPComplexArray:
    case ASTTypeDurationArray:
    case ASTTypeOpenPulseFrameArray:
    case ASTTypeOpenPulsePortArray:
    case ASTTypeOpenPulseWaveformArray:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsArrayType(const ASTExpressionNode* EX) const {
    return IsArrayType(EX->GetASTType());
  }

  bool CanBeArrayType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeBitset:
    case ASTTypeBool:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeDouble:
    case ASTTypeFloat:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeMPDecimal:
    case ASTTypeMPComplex:
    case ASTTypeDuration:
    case ASTTypeDurationOf:
    case ASTTypeOpenPulseFrame:
    case ASTTypeOpenPulsePort:
    case ASTTypeOpenPulseWaveform:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsArbitraryWidthType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeBitset:
    case ASTTypeAngle:
    case ASTTypeQubit:
    case ASTTypeQubitContainerAlias:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeMPDecimal:
    case ASTTypeMPComplex:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsNonArrayIndexableType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeQubitContainer:
    case ASTTypeQubitContainerAlias:
    case ASTTypeAngle:
    case ASTTypeBitset:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool CanBeAssignedTo(const ASTIdentifierNode* Id) const;

  bool CanBeAssignedTo(const ASTExpressionNode* EX) const;
};

} // namespace QASM

#endif // __QASM_AST_EXPRESSION_VALIDATOR_H

