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

#include <qasm/AST/ASTExpressionValidator.h>
#include <qasm/AST/ASTExpressionEvaluator.h>
#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTScopeController.h>
#include <qasm/AST/ASTCastExpr.h>
#include <qasm/AST/ASTImplicitConversionExpr.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <iostream>
#include <cassert>

namespace QASM {

ASTExpressionValidator ASTExpressionValidator::EXV;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTType ASTExpressionValidator::ResolveOperatorType(const ASTOperatorNode* E,
                                                    ASTType Ty) const {
  assert(E && "Invalid ASTOperatorNode argument!");
  assert(Ty == E->GetASTType() && "Type must be ASTTypeOpTy!");

  if (!E || Ty == ASTTypeUndefined)
    return ASTTypeUndefined;

  ASTType RTy = Ty;
  const ASTExpressionNode* Ex = E;

  while (RTy == Ty) {
    if (Ex->IsIdentifier()) {
      if (const ASTOperatorNode* Op = dynamic_cast<const ASTOperatorNode*>(Ex)) {
        ASTScopeController::Instance().CheckIdentifier(Op->GetTargetIdentifier());
        RTy = Op->GetTargetIdentifier()->GetSymbolType();
        Ex = Op->GetTargetIdentifier()->GetExpression();
      } else {
        break;
      }
    } else if (Ex->IsExpression()) {
      if (const ASTOperatorNode* Op = dynamic_cast<const ASTOperatorNode*>(Ex)) {
        RTy = Op->GetTargetExpression()->GetASTType();
        Ex = Op->GetTargetExpression();
      } else {
        break;
      }
    }
  }

  switch (RTy) {
  case ASTTypeCast: {
    if (const ASTOperatorNode* Op = dynamic_cast<const ASTOperatorNode*>(E)) {
      if (const ASTCastExpressionNode* CX =
          dynamic_cast<const ASTCastExpressionNode*>(Op->GetTargetExpression())) {
        RTy = CX->GetCastTo();
      }
    }
  }
    break;
  case ASTTypeImplicitConversion: {
    if (const ASTOperatorNode* Op = dynamic_cast<const ASTOperatorNode*>(E)) {
      if (const ASTImplicitConversionNode* ICX =
          dynamic_cast<const ASTImplicitConversionNode*>(Op->GetTargetExpression())) {
        RTy = ICX->GetConvertTo();
      }
    }
  }
    break;
  default:
    break;
  }

  return RTy;
}

ASTType ASTExpressionValidator::ResolveOperandType(const ASTOperandNode* E,
                                                   ASTType Ty) const {
  assert(E && "Invalid ASTOperandNode argument!");
  assert(Ty == E->GetASTType() && "Type must be ASTTypeOpndTy!");

  if (!E || Ty == ASTTypeUndefined)
    return ASTTypeUndefined;

  ASTType RTy = Ty;
  const ASTExpressionNode* Ex = E;

  while (RTy == Ty) {
    if (Ex->IsIdentifier()) {
      if (const ASTOperandNode* Op = dynamic_cast<const ASTOperandNode*>(Ex)) {
        ASTScopeController::Instance().CheckIdentifier(Op->GetTargetIdentifier());
        RTy = Op->GetTargetIdentifier()->GetSymbolType();
        Ex = Op->GetTargetIdentifier()->GetExpression();
      } else {
        break;
      }
    } else if (Ex->IsExpression()) {
      if (const ASTOperandNode* Op = dynamic_cast<const ASTOperandNode*>(Ex)) {
        RTy = Op->GetExpression()->GetASTType();
        Ex = Op->GetExpression();
      } else {
        break;
      }
    }
  }

  switch (RTy) {
  case ASTTypeCast: {
    if (const ASTOperandNode* Op = dynamic_cast<const ASTOperandNode*>(E)) {
      if (const ASTCastExpressionNode* CX =
          dynamic_cast<const ASTCastExpressionNode*>(Op->GetExpression())) {
        RTy = CX->GetCastTo();
      }
    }
  }
    break;
  case ASTTypeImplicitConversion: {
    if (const ASTOperandNode* Op = dynamic_cast<const ASTOperandNode*>(E)) {
      if (const ASTImplicitConversionNode* ICX =
          dynamic_cast<const ASTImplicitConversionNode*>(Op->GetExpression())) {
        RTy = ICX->GetConvertTo();
      }
    }
  }
    break;
  default:
    break;
  }

  return RTy;
}

void ASTExpressionValidator::ValidateLogicalNot(const ASTUnaryOpNode* UOp) const {
  assert(UOp && "Invalid ASTUnaryOpNode argument!");

  const ASTBinaryOpNode* BOP =
    dynamic_cast<const ASTBinaryOpNode*>(UOp->GetExpression());

  ASTType ETy = ASTTypeUndefined;

  if (UOp->GetExpression()->GetASTType() == ASTTypeOpTy) {
    if (const ASTOperatorNode* OPN =
        dynamic_cast<const ASTOperatorNode*>(UOp->GetExpression())) {
      if (OPN->IsIdentifier()) {
        ASTScopeController::Instance().CheckIdentifier(OPN->GetTargetIdentifier());
        ETy = OPN->GetTargetIdentifier()->GetSymbolType();
      } else {
        ETy = OPN->GetTargetExpression()->GetASTType();
      }
    } else {
      ETy = ASTTypeUndefined;
    }
  } else if (UOp->GetExpression()->GetASTType() == ASTTypeOpndTy) {
    if (const ASTOperandNode* OPD =
        dynamic_cast<const ASTOperandNode*>(UOp->GetExpression())) {
      if (OPD->IsIdentifier()) {
        ASTScopeController::Instance().CheckIdentifier(OPD->GetIdentifier());
        ETy = OPD->GetIdentifier()->GetSymbolType();
      } else {
        ETy = OPD->GetExpression()->GetASTType();
      }
    } else {
      ETy = ASTTypeUndefined;
    }
  } else {
    ETy = UOp->GetExpression()->GetASTType();
  }

  if (BOP && !BOP->HasParens()) {
    switch (BOP->GetOpType()) {
    case ASTOpTypeAdd:
    case ASTOpTypeAddAssign:
    case ASTOpTypeSub:
    case ASTOpTypeSubAssign:
    case ASTOpTypeMul:
    case ASTOpTypeMulAssign:
    case ASTOpTypeDiv:
    case ASTOpTypeDivAssign:
    case ASTOpTypeMod:
    case ASTOpTypeModAssign:
    case ASTOpTypeBitAnd:
    case ASTOpTypeBitAndAssign:
    case ASTOpTypeBitOr:
    case ASTOpTypeBitOrAssign:
    case ASTOpTypeXor:
    case ASTOpTypeXorAssign:
    case ASTOpTypeAssign:
    case ASTOpTypeCompEq:
    case ASTOpTypeCompNeq:
    case ASTOpTypeLT:
    case ASTOpTypeGT:
    case ASTOpTypeLE:
    case ASTOpTypeGE:
    case ASTOpTypeNegate:
    case ASTOpTypePositive:
    case ASTOpTypeNegative:
    case ASTOpTypeBitNot:
    case ASTOpTypeLeftShift:
    case ASTOpTypeLeftShiftAssign:
    case ASTOpTypeRightShift:
    case ASTOpTypeRightShiftAssign: {
      if (IsScalarType(ETy) || IsAngleType(ETy) || IsComplexType(ETy) ||
          IsNumericType(ETy) || IsUnPromotedIntegerType(ETy)) {
        std::stringstream M;
        M << "Logical not operator yielding an integer constant expression "
          << "for the left operand of the binary op.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      }
    }
      break;
    default:
      break;
    }
  }
}

bool ASTExpressionValidator::Validate(const ASTBinaryOpNode* BOp) const {
  assert(BOp && "Invalid ASTBinaryOpNode argument!");

  ASTType LTy = BOp->GetLeft()->GetASTType();
  ASTType RTy = BOp->GetRight()->GetASTType();

  const ASTDeclarationContext* DCX =
    ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(DCX && "Could not obtain a valid DeclarationContext!");

  switch (LTy) {
  case ASTTypeOpTy:
    LTy = ResolveOperatorType(dynamic_cast<const ASTOperatorNode*>(
                                           BOp->GetLeft()), LTy);
    break;
  case ASTTypeOpndTy:
    LTy = ResolveOperandType(dynamic_cast<const ASTOperandNode*>(
                                          BOp->GetLeft()), LTy);
    break;
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
    ASTScopeController::Instance().CheckIdentifier(BOp->GetLeft()->GetIdentifier());
    LTy = BOp->GetLeft()->GetIdentifier()->GetSymbolType();
    break;
  case ASTTypeFunctionCall:
    LTy = dynamic_cast<const ASTFunctionCallNode*>(
                                            BOp->GetLeft())->GetResultType();
    break;
  case ASTTypeCast:
    LTy = dynamic_cast<const ASTCastExpressionNode*>(BOp->GetLeft())->GetCastTo();
    break;
  case ASTTypeImplicitConversion:
    LTy = dynamic_cast<const ASTImplicitConversionNode*>(BOp->GetLeft())->GetConvertTo();
    break;
  case ASTTypeUnaryOp:
    LTy = dynamic_cast<const ASTUnaryOpNode*>(BOp->GetLeft())->GetExpressionType();
    break;
  case ASTTypeBinaryOp:
    LTy = dynamic_cast<const ASTBinaryOpNode*>(BOp->GetLeft())->GetExpressionType();
    break;
  default:
    break;
  }

  switch (RTy) {
  case ASTTypeOpTy:
    RTy = ResolveOperatorType(dynamic_cast<const ASTOperatorNode*>(
                                           BOp->GetRight()), RTy);
    break;
  case ASTTypeOpndTy:
    RTy = ResolveOperandType(dynamic_cast<const ASTOperandNode*>(
                                          BOp->GetRight()), RTy);
    break;
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
    ASTScopeController::Instance().CheckIdentifier(BOp->GetRight()->GetIdentifier());
    RTy = BOp->GetRight()->GetIdentifier()->GetSymbolType();
    break;
  case ASTTypeFunctionCall:
    RTy = dynamic_cast<const ASTFunctionCallNode*>(
                                            BOp->GetRight())->GetResultType();
    break;
  case ASTTypeCast:
    RTy = dynamic_cast<const ASTCastExpressionNode*>(BOp->GetRight())->GetCastTo();
    break;
  case ASTTypeImplicitConversion:
    RTy = dynamic_cast<const ASTImplicitConversionNode*>(BOp->GetRight())->GetConvertTo();
    break;
  case ASTTypeUnaryOp:
    RTy = dynamic_cast<const ASTUnaryOpNode*>(BOp->GetRight())->GetExpressionType();
    break;
  case ASTTypeBinaryOp:
    RTy = dynamic_cast<const ASTBinaryOpNode*>(BOp->GetRight())->GetExpressionType();
    break;
  default:
    break;
  }

  if (LTy == ASTTypeBinaryOp) {
    if (BOp->GetLeft()->GetASTType() == ASTTypeOpTy)
      LTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTBinaryOpNode*>(
          dynamic_cast<const ASTOperatorNode*>(BOp->GetLeft())->GetTargetExpression()));
    else if (BOp->GetLeft()->GetASTType() == ASTTypeOpndTy)
      LTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTBinaryOpNode*>(
          dynamic_cast<const ASTOperandNode*>(BOp->GetLeft())->GetExpression()));
    else
      LTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTBinaryOpNode*>(BOp->GetLeft()));
  } else if (LTy == ASTTypeUnaryOp) {
    if (BOp->GetLeft()->GetASTType() == ASTTypeOpTy)
      LTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTUnaryOpNode*>(
          dynamic_cast<const ASTOperatorNode*>(BOp->GetLeft())->GetTargetExpression()));
    else if (BOp->GetLeft()->GetASTType() == ASTTypeOpndTy)
      LTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTUnaryOpNode*>(
          dynamic_cast<const ASTOperandNode*>(BOp->GetLeft())->GetExpression()));
    else
      LTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTUnaryOpNode*>(BOp->GetLeft()));
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
      RTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTBinaryOpNode*>(
          dynamic_cast<const ASTOperatorNode*>(BOp->GetRight())->GetTargetExpression()));
    else if (BOp->GetRight()->GetASTType() == ASTTypeOpndTy)
      RTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTBinaryOpNode*>(
          dynamic_cast<const ASTOperandNode*>(BOp->GetRight())->GetExpression()));
    else
      RTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTBinaryOpNode*>(BOp->GetRight()));
  } else if (RTy == ASTTypeUnaryOp) {
    if (BOp->GetRight()->GetASTType() == ASTTypeOpTy)
      RTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTUnaryOpNode*>(
          dynamic_cast<const ASTOperatorNode*>(BOp->GetRight())->GetTargetExpression()));
    else if (BOp->GetRight()->GetASTType() == ASTTypeOpndTy)
      RTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTUnaryOpNode*>(
          dynamic_cast<const ASTOperandNode*>(BOp->GetRight())->GetExpression()));
    else
      RTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTUnaryOpNode*>(BOp->GetRight()));
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

  switch (DCX->GetContextType()) {
  case ASTTypeForStatement:
  case ASTTypeWhileStatement:
    if (LTy == ASTTypeUndefined) {
      const_cast<ASTIdentifierNode*>(
                 BOp->GetLeft()->GetIdentifier())->SetSymbolType(ASTTypeInt);
      LTy = BOp->GetLeft()->GetIdentifier()->GetSymbolType();
    }
    if (RTy == ASTTypeUndefined) {
      const_cast<ASTIdentifierNode*>(
                 BOp->GetRight()->GetIdentifier())->SetSymbolType(ASTTypeInt);
      RTy = BOp->GetRight()->GetIdentifier()->GetSymbolType();
    }
    break;
  default:
    assert(LTy != ASTTypeIdentifier && "LHS Type is an ASTTypeIdentifier!");
    assert(RTy != ASTTypeIdentifier && "RHS Type is an ASTTypeIdentifier!");

    assert(LTy != ASTTypeUndefined && "LHS Type is an ASTTypeUndefined!");
    assert(RTy != ASTTypeUndefined && "RHS Type is an ASTTypeUndefined!");

    assert(LTy != ASTTypeOpTy && "LHS Type is an ASTTypeOpTy!");
    assert(RTy != ASTTypeOpTy && "RHS Type is an ASTTypeOpTy!");

    assert(LTy != ASTTypeOpndTy && "LHS Type is an ASTTypeOpndTy!");
    assert(RTy != ASTTypeOpndTy && "RHS Type is an ASTTypeOpndTy!");
    break;
  }

  if (BOp->GetLeft()->GetASTType() == ASTTypeUnaryOp) {
    if (const ASTUnaryOpNode* UOp =
        dynamic_cast<const ASTUnaryOpNode*>(BOp->GetLeft())) {
      if (UOp->GetOpType() == ASTOpTypeLogicalNot) {
        switch (BOp->GetOpType()) {
        case ASTOpTypeAddAssign:
        case ASTOpTypeSubAssign:
        case ASTOpTypeMulAssign:
        case ASTOpTypeDivAssign:
        case ASTOpTypeModAssign:
        case ASTOpTypeLeftShift:
        case ASTOpTypeLeftShiftAssign:
        case ASTOpTypeRightShift:
        case ASTOpTypeRightShiftAssign:
        case ASTOpTypeBitAndAssign:
        case ASTOpTypeBitOrAssign:
        case ASTOpTypeXorAssign:
        case ASTOpTypeAssign: {
          std::stringstream M;
          M << "Left operand of a binary op requires a lvalue.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
        }
          break;
        default:
          break;
        }
      }
    }
  }

  switch (BOp->GetOpType()) {
  case ASTOpTypeCompEq:
  case ASTOpTypeCompNeq:
    return (IsIntegerType(LTy) && IsIntegerType(RTy)) ||
           (IsIntegerType(LTy) && IsFloatingPointType(RTy)) ||
           (IsFloatingPointType(LTy) && IsIntegerType(RTy)) ||
           (IsFloatingPointType(LTy) && IsFloatingPointType(RTy)) ||
           (IsComplexType(LTy) && IsComplexType(RTy)) ||
           (IsAngleType(LTy) && IsAngleType(RTy)) ||
           (IsAngleType(LTy) && IsScalarType(RTy)) ||
           (IsScalarType(LTy) && IsAngleType(RTy)) ||
           (IsQuantumArithmeticType(LTy) && IsQuantumArithmeticType(RTy));
    break;
  case ASTOpTypeAdd:
  case ASTOpTypeAddAssign:
  case ASTOpTypeSub:
  case ASTOpTypeSubAssign:
    return (IsScalarType(LTy) && IsScalarType(RTy)) ||
           (IsComplexType(LTy) && IsComplexType(RTy)) ||
           (IsAngleType(LTy) && IsAngleType(RTy)) ||
           (IsAngleType(LTy) && IsScalarType(RTy)) ||
           (IsScalarType(LTy) && IsAngleType(RTy)) ||
           (IsQuantumArithmeticType(LTy) && IsNumericType(RTy)) ||
           (IsNumericType(LTy) && IsQuantumArithmeticType(RTy)) ||
           (IsQuantumArithmeticType(LTy) && IsQuantumArithmeticType(RTy));
    break;
  case ASTOpTypeMul:
  case ASTOpTypeMulAssign:
    return (IsScalarType(LTy) && IsScalarType(RTy)) ||
           (IsComplexType(LTy) && IsComplexType(RTy)) ||
           (IsScalarType(LTy) && IsComplexType(RTy)) ||
           (IsComplexType(LTy) && IsScalarType(RTy)) ||
           (IsAngleType(LTy) && IsScalarType(RTy)) ||
           (IsScalarType(LTy) && IsAngleType(RTy)) ||
           (IsQuantumArithmeticType(LTy) && IsNumericType(RTy)) ||
           (IsNumericType(LTy) && IsQuantumArithmeticType(RTy)) ||
           (IsQuantumArithmeticType(LTy) && IsQuantumArithmeticType(RTy));
    break;
  case ASTOpTypeDiv:
  case ASTOpTypeDivAssign:
    return (IsScalarType(LTy) && IsScalarType(RTy)) ||
           (IsComplexType(LTy) && IsComplexType(RTy)) ||
           (IsScalarType(LTy) && IsComplexType(RTy)) ||
           (IsComplexType(LTy) && IsScalarType(RTy)) ||
           (IsAngleType(LTy) && IsScalarType(RTy)) ||
           (IsScalarType(LTy) && IsAngleType(RTy)) ||
           (IsQuantumArithmeticType(LTy) && IsNumericType(RTy)) ||
           (IsNumericType(LTy) && IsQuantumArithmeticType(RTy)) ||
           (IsQuantumArithmeticType(LTy) && IsQuantumArithmeticType(RTy));
    break;
  case ASTOpTypeMod:
  case ASTOpTypeModAssign:
    return IsScalarType(LTy) && IsScalarType(RTy);
    break;
  case ASTOpTypeBitAnd:
  case ASTOpTypeBitAndAssign:
  case ASTOpTypeBitOr:
  case ASTOpTypeBitOrAssign:
  case ASTOpTypeBitNot:
  case ASTOpTypeXor:
  case ASTOpTypeXorAssign:
  case ASTOpTypeLeftShift:
  case ASTOpTypeLeftShiftAssign:
  case ASTOpTypeRightShift:
  case ASTOpTypeRightShiftAssign: {
    if (IsAngleType(LTy)) {
      std::stringstream M;
      M << "This is incorrect. It's temporarily allowed until the OQ3 "
        << "Spec is clarified.\n";
      M << "    https://github.com/Qiskit/openqasm/issues/332";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
    }

    return (IsIntegerType(LTy) && IsIntegerType(RTy)) ||
           (IsAngleType(LTy) && IsIntegerType(RTy));
  }
    break;
  case ASTOpTypePreInc:
  case ASTOpTypePreDec:
  case ASTOpTypePostInc:
  case ASTOpTypePostDec:
    return (IsIntegerType(LTy) && IsIntegerType(RTy));
    break;
  case ASTOpTypeLT:
  case ASTOpTypeGT:
  case ASTOpTypeGE:
  case ASTOpTypeLE:
    return (IsIntegerType(LTy) && IsIntegerType(RTy)) ||
           (IsIntegerType(LTy) && IsFloatingPointType(RTy)) ||
           (IsFloatingPointType(LTy) && IsIntegerType(RTy)) ||
           (IsFloatingPointType(LTy) && IsFloatingPointType(RTy)) ||
           (IsComplexType(LTy) && IsComplexType(RTy)) ||
           (IsAngleType(LTy) && IsAngleType(RTy));
    break;
  case ASTOpTypeLogicalAnd:
  case ASTOpTypeLogicalOr:
    return (IsIntegerType(LTy) && IsIntegerType(RTy)) ||
           (IsIntegerType(LTy) && IsFloatingPointType(RTy)) ||
           (IsFloatingPointType(LTy) && IsIntegerType(RTy)) ||
           (IsFloatingPointType(LTy) && IsFloatingPointType(RTy)) ||
           (IsComplexType(LTy) && IsComplexType(RTy)) ||
           (IsComplexType(LTy) && IsIntegerType(RTy)) ||
           (IsIntegerType(LTy) && IsComplexType(RTy)) ||
           (IsComplexType(LTy) && IsFloatingPointType(RTy)) ||
           (IsFloatingPointType(LTy) && IsComplexType(RTy));
    break;
  case ASTOpTypeNegate:
  case ASTOpTypePositive:
  case ASTOpTypeNegative:
    return IsNumericType(LTy) && !(LTy == ASTTypeBool);
    break;
  case ASTOpTypePow:
    return (IsUnPromotedIntegerType(LTy) && IsUnPromotedIntegerType(RTy)) ||
           (IsFloatingPointType(LTy) && IsFloatingPointType(RTy)) ||
           (IsUnPromotedIntegerType(LTy) && IsFloatingPointType(RTy)) ||
           (IsFloatingPointType(LTy) && IsUnPromotedIntegerType(RTy)) ||
           (IsComplexType(LTy) && IsComplexType(RTy));
    break;
  case ASTOpTypeAssign:
    if (IsQubitType(LTy))
      return false;

    if (IsIntegerType(LTy) || IsFloatingPointType(LTy)) {
      return IsIntegerType(RTy) || IsFloatingPointType(RTy) ||
             IsReturningType(RTy) || IsAngleType(RTy);
    } else if (IsComplexType(LTy)) {
      return IsComplexType(RTy) || IsIntegerType(RTy) ||
             IsFloatingPointType(RTy) || IsReturningType(RTy);
    } else if (IsAngleType(LTy)) {
      return IsIntegerType(RTy) || IsFloatingPointType(RTy) ||
             IsAngleType(RTy) || IsReturningType(RTy);
    } else if (IsAssignableType(LTy)) {
      if (LTy == ASTTypeOpenPulseFrame)
        return IsIntegerType(RTy) || IsFloatingPointType(RTy) ||
               IsAngleType(RTy) || IsTimeType(RTy) ||
               IsStringType(RTy) || IsReturningType(RTy);
      else
        return IsIntegerType(RTy) || IsFloatingPointType(RTy) ||
               IsAngleType(RTy) || IsReturningType(RTy);
    }
    break;
  case ASTOpTypeBinaryLeftFold:
  case ASTOpTypeBinaryRightFold:
  case ASTOpTypeUnaryLeftFold:
  case ASTOpTypeUnaryRightFold:
    return true;
    break;
  default:
    break;
  }

  return false;
}

bool ASTExpressionValidator::Validate(const ASTUnaryOpNode* UOp) const {
  assert(UOp && "Invalid ASTUnaryOpNode argument!");

  ValidateLogicalNot(UOp);

  ASTType ETy = UOp->GetExpression()->GetASTType();

  const ASTDeclarationContext* DCX =
    ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(DCX && "Could not obtain a valid DeclarationContext!");

  switch (ETy) {
  case ASTTypeOpTy:
    ETy = ResolveOperatorType(dynamic_cast<const ASTOperatorNode*>(
                                      UOp->GetExpression()), ETy);
    break;
  case ASTTypeOpndTy:
    ETy = ResolveOperandType(dynamic_cast<const ASTOperandNode*>(
                                      UOp->GetExpression()), ETy);
    break;
  case ASTTypeCast:
    ETy = dynamic_cast<const ASTCastExpressionNode*>(UOp->GetExpression())->GetCastTo();
    break;
  default:
    break;
  }

  if (ETy == ASTTypeBinaryOp) {
    if (UOp->GetExpression()->GetASTType() == ASTTypeOpTy)
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTBinaryOpNode*>(
          dynamic_cast<const ASTOperatorNode*>(
                             UOp->GetExpression())->GetTargetExpression()));
    else if (UOp->GetExpression()->GetASTType() == ASTTypeOpndTy)
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTBinaryOpNode*>(
          dynamic_cast<const ASTOperandNode*>(
                             UOp->GetExpression())->GetExpression()));
    else
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTBinaryOpNode*>(UOp->GetExpression()));
  } else if (ETy == ASTTypeUnaryOp) {
    if (UOp->GetExpression()->GetASTType() == ASTTypeOpTy)
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTUnaryOpNode*>(
          dynamic_cast<const ASTOperatorNode*>(
                             UOp->GetExpression())->GetTargetExpression()));
    else if (UOp->GetExpression()->GetASTType() == ASTTypeOpndTy)
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTUnaryOpNode*>(
          dynamic_cast<const ASTOperandNode*>(
                             UOp->GetExpression())->GetExpression()));
    else
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(
        dynamic_cast<const ASTUnaryOpNode*>(UOp->GetExpression()));
  } else if (ETy == ASTTypeIdentifier) {
    if (UOp->GetExpression()->GetASTType() == ASTTypeOpTy)
      ETy = dynamic_cast<const ASTOperatorNode*>(
        UOp->GetExpression())->GetTargetIdentifier()->GetSymbolType();
    else
      ETy = UOp->GetExpression()->GetIdentifier()->GetSymbolType();
  } else if (ETy == ASTTypeOpTy) {
    ETy = dynamic_cast<const ASTOperatorNode*>(
                             UOp->GetExpression())->GetTargetType();
    if (ETy == ASTTypeIdentifier)
      ETy = dynamic_cast<const ASTOperatorNode*>(
                         UOp->GetExpression())->GetTargetIdentifier()->GetSymbolType();
  } else if (ETy == ASTTypeOpndTy) {
    ETy = dynamic_cast<const ASTOperandNode*>(
                             UOp->GetExpression())->GetTargetType();
    if (ETy == ASTTypeIdentifier)
      ETy = dynamic_cast<const ASTOperandNode*>(
                               UOp->GetExpression())->GetIdentifier()->GetSymbolType();
  }

  switch (DCX->GetContextType()) {
  case ASTTypeForStatement:
  case ASTTypeWhileStatement:
    if (ETy == ASTTypeUndefined) {
      const_cast<ASTIdentifierNode*>(
        UOp->GetExpression()->GetIdentifier())->SetSymbolType(ASTTypeInt);
      ETy = UOp->GetExpression()->GetIdentifier()->GetSymbolType();
    }
    break;
  default:
    assert(ETy != ASTTypeIdentifier && "Expression is an ASTTypeIdentifier!");
    assert(ETy != ASTTypeUndefined && "Expression is an ASTTypeUndefined!");
    break;
  }

  switch (UOp->GetOpType()) {
  case ASTOpTypeBinaryLeftFold:
  case ASTOpTypeBinaryRightFold:
  case ASTOpTypeUnaryLeftFold:
  case ASTOpTypeUnaryRightFold:
    return true;
    break;
  case ASTOpTypeLogicalNot:
    return IsIntegerType(ETy) || IsFloatingPointType(ETy) ||
           IsComplexType(ETy) || IsUnPromotedIntegerType(ETy) ||
           ETy == ASTTypeUnaryOp || ETy == ASTTypeBinaryOp ||
           ETy == ASTTypeIdentifier;
    break;
  case ASTOpTypeRotation:
  case ASTOpTypeRotl:
  case ASTOpTypeRotr:
  case ASTOpTypePopcount:
    return IsIntegerType(ETy);
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
    return IsUnPromotedIntegerType(ETy) || IsFloatingPointType(ETy) ||
           IsComplexType(ETy);
    break;
  case ASTOpTypeNegative:
  case ASTOpTypePositive:
    return ASTExpressionValidator::Instance().CanDoArithmeticNegPos(ETy);
    break;
  default:
    break;
  }

  return false;
}

bool ASTExpressionValidator::Validate(const ASTCastExpressionNode* XOp) const {
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
    return IsScalarType(FTy);
    break;
  case ASTTypeAngle:
    return IsAngleType(FTy) || IsUnPromotedScalarType(FTy);
    break;
  case ASTTypeMPComplex:
    return IsComplexType(FTy);
    break;
  default:
    break;
  }

  return false;
}

} // namespace QASM

