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

#include <qasm/AST/ASTArray.h>
#include <qasm/AST/ASTCastExpr.h>
#include <qasm/AST/ASTExpressionEvaluator.h>
#include <qasm/AST/ASTExpressionValidator.h>
#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTImplicitConversionExpr.h>
#include <qasm/AST/ASTScopeController.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <cassert>
#include <iostream>

namespace QASM {

ASTExpressionValidator ASTExpressionValidator::EXV;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTType ASTExpressionValidator::ResolveOperatorType(const ASTOperatorNode *E,
                                                    ASTType Ty) const {
  assert(E && "Invalid ASTOperatorNode argument!");
  assert(Ty == E->GetASTType() && "Type must be ASTTypeOpTy!");

  if (!E || Ty == ASTTypeUndefined)
    return ASTTypeUndefined;

  ASTType RTy = Ty;
  const ASTExpressionNode *Ex = E;

  while (RTy == Ty) {
    if (Ex->IsIdentifier()) {
      if (const ASTOperatorNode *Op =
              dynamic_cast<const ASTOperatorNode *>(Ex)) {
        ASTScopeController::Instance().CheckIdentifier(
            Op->GetTargetIdentifier());
        RTy = Op->GetTargetIdentifier()->GetSymbolType();
        Ex = Op->GetTargetIdentifier()->GetExpression();
      } else {
        break;
      }
    } else if (Ex->IsExpression()) {
      if (const ASTOperatorNode *Op =
              dynamic_cast<const ASTOperatorNode *>(Ex)) {
        RTy = Op->GetTargetExpression()->GetASTType();
        Ex = Op->GetTargetExpression();
      } else {
        break;
      }
    }
  }

  switch (RTy) {
  case ASTTypeCast: {
    if (const ASTOperatorNode *Op = dynamic_cast<const ASTOperatorNode *>(E)) {
      if (const ASTCastExpressionNode *CX =
              dynamic_cast<const ASTCastExpressionNode *>(
                  Op->GetTargetExpression())) {
        RTy = CX->GetCastTo();
      }
    }
  } break;
  case ASTTypeImplicitConversion: {
    if (const ASTOperatorNode *Op = dynamic_cast<const ASTOperatorNode *>(E)) {
      if (const ASTImplicitConversionNode *ICX =
              dynamic_cast<const ASTImplicitConversionNode *>(
                  Op->GetTargetExpression())) {
        RTy = ICX->GetConvertTo();
      }
    }
  } break;
  default:
    break;
  }

  return RTy;
}

ASTType ASTExpressionValidator::ResolveOperandType(const ASTOperandNode *E,
                                                   ASTType Ty) const {
  assert(E && "Invalid ASTOperandNode argument!");
  assert(Ty == E->GetASTType() && "Type must be ASTTypeOpndTy!");

  if (!E || Ty == ASTTypeUndefined)
    return ASTTypeUndefined;

  ASTType RTy = Ty;
  const ASTExpressionNode *Ex = E;

  while (RTy == Ty) {
    if (Ex->IsIdentifier()) {
      if (const ASTOperandNode *Op = dynamic_cast<const ASTOperandNode *>(Ex)) {
        ASTScopeController::Instance().CheckIdentifier(
            Op->GetTargetIdentifier());
        RTy = Op->GetTargetIdentifier()->GetSymbolType();
        Ex = Op->GetTargetIdentifier()->GetExpression();
      } else {
        break;
      }
    } else if (Ex->IsExpression()) {
      if (const ASTOperandNode *Op = dynamic_cast<const ASTOperandNode *>(Ex)) {
        RTy = Op->GetExpression()->GetASTType();
        Ex = Op->GetExpression();
      } else {
        break;
      }
    }
  }

  switch (RTy) {
  case ASTTypeCast: {
    if (const ASTOperandNode *Op = dynamic_cast<const ASTOperandNode *>(E)) {
      if (const ASTCastExpressionNode *CX =
              dynamic_cast<const ASTCastExpressionNode *>(
                  Op->GetExpression())) {
        RTy = CX->GetCastTo();
      }
    }
  } break;
  case ASTTypeImplicitConversion: {
    if (const ASTOperandNode *Op = dynamic_cast<const ASTOperandNode *>(E)) {
      if (const ASTImplicitConversionNode *ICX =
              dynamic_cast<const ASTImplicitConversionNode *>(
                  Op->GetExpression())) {
        RTy = ICX->GetConvertTo();
      }
    }
  } break;
  default:
    break;
  }

  return RTy;
}

void ASTExpressionValidator::ValidateLogicalNot(
    const ASTUnaryOpNode *UOp) const {
  assert(UOp && "Invalid ASTUnaryOpNode argument!");

  const ASTBinaryOpNode *BOP =
      dynamic_cast<const ASTBinaryOpNode *>(UOp->GetExpression());

  ASTType ETy = ASTTypeUndefined;

  if (UOp->GetExpression()->GetASTType() == ASTTypeOpTy) {
    if (const ASTOperatorNode *OPN =
            dynamic_cast<const ASTOperatorNode *>(UOp->GetExpression())) {
      if (OPN->IsIdentifier()) {
        ASTScopeController::Instance().CheckIdentifier(
            OPN->GetTargetIdentifier());
        ETy = OPN->GetTargetIdentifier()->GetSymbolType();
      } else {
        ETy = OPN->GetTargetExpression()->GetASTType();
      }
    } else {
      ETy = ASTTypeUndefined;
    }
  } else if (UOp->GetExpression()->GetASTType() == ASTTypeOpndTy) {
    if (const ASTOperandNode *OPD =
            dynamic_cast<const ASTOperandNode *>(UOp->GetExpression())) {
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
            DIAGLineCounter::Instance().GetLocation(), M.str(),
            DiagLevel::Error);
      }
    } break;
    default:
      break;
    }
  }
}

bool ASTExpressionValidator::Validate(const ASTBinaryOpNode *BOp) const {
  assert(BOp && "Invalid ASTBinaryOpNode argument!");

  ASTType LTy = BOp->GetLeft()->GetASTType();
  ASTType RTy = BOp->GetRight()->GetASTType();

  const ASTDeclarationContext *DCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(DCX && "Could not obtain a valid DeclarationContext!");

  switch (LTy) {
  case ASTTypeOpTy:
    LTy = ResolveOperatorType(
        dynamic_cast<const ASTOperatorNode *>(BOp->GetLeft()), LTy);
    break;
  case ASTTypeOpndTy:
    LTy = ResolveOperandType(
        dynamic_cast<const ASTOperandNode *>(BOp->GetLeft()), LTy);
    break;
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
    ASTScopeController::Instance().CheckIdentifier(
        BOp->GetLeft()->GetIdentifier());
    LTy = BOp->GetLeft()->GetIdentifier()->GetSymbolType();
    break;
  case ASTTypeFunctionCall:
    LTy = dynamic_cast<const ASTFunctionCallNode *>(BOp->GetLeft())
              ->GetResultType();
    break;
  case ASTTypeCast:
    LTy = dynamic_cast<const ASTCastExpressionNode *>(BOp->GetLeft())
              ->GetCastTo();
    break;
  case ASTTypeImplicitConversion:
    LTy = dynamic_cast<const ASTImplicitConversionNode *>(BOp->GetLeft())
              ->GetConvertTo();
    break;
  case ASTTypeUnaryOp:
    LTy = dynamic_cast<const ASTUnaryOpNode *>(BOp->GetLeft())
              ->GetExpressionType();
    break;
  case ASTTypeBinaryOp:
    LTy = dynamic_cast<const ASTBinaryOpNode *>(BOp->GetLeft())
              ->GetExpressionType();
    break;
  default:
    break;
  }

  switch (RTy) {
  case ASTTypeOpTy:
    RTy = ResolveOperatorType(
        dynamic_cast<const ASTOperatorNode *>(BOp->GetRight()), RTy);
    break;
  case ASTTypeOpndTy:
    RTy = ResolveOperandType(
        dynamic_cast<const ASTOperandNode *>(BOp->GetRight()), RTy);
    break;
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
    ASTScopeController::Instance().CheckIdentifier(
        BOp->GetRight()->GetIdentifier());
    RTy = BOp->GetRight()->GetIdentifier()->GetSymbolType();
    break;
  case ASTTypeFunctionCall:
    RTy = dynamic_cast<const ASTFunctionCallNode *>(BOp->GetRight())
              ->GetResultType();
    break;
  case ASTTypeCast:
    RTy = dynamic_cast<const ASTCastExpressionNode *>(BOp->GetRight())
              ->GetCastTo();
    break;
  case ASTTypeImplicitConversion:
    RTy = dynamic_cast<const ASTImplicitConversionNode *>(BOp->GetRight())
              ->GetConvertTo();
    break;
  case ASTTypeUnaryOp:
    RTy = dynamic_cast<const ASTUnaryOpNode *>(BOp->GetRight())
              ->GetExpressionType();
    break;
  case ASTTypeBinaryOp:
    RTy = dynamic_cast<const ASTBinaryOpNode *>(BOp->GetRight())
              ->GetExpressionType();
    break;
  default:
    break;
  }

  if (LTy == ASTTypeBinaryOp) {
    if (BOp->GetLeft()->GetASTType() == ASTTypeOpTy)
      LTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTBinaryOpNode *>(
              dynamic_cast<const ASTOperatorNode *>(BOp->GetLeft())
                  ->GetTargetExpression()));
    else if (BOp->GetLeft()->GetASTType() == ASTTypeOpndTy)
      LTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTBinaryOpNode *>(
              dynamic_cast<const ASTOperandNode *>(BOp->GetLeft())
                  ->GetExpression()));
    else
      LTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTBinaryOpNode *>(BOp->GetLeft()));
  } else if (LTy == ASTTypeUnaryOp) {
    if (BOp->GetLeft()->GetASTType() == ASTTypeOpTy)
      LTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTUnaryOpNode *>(
              dynamic_cast<const ASTOperatorNode *>(BOp->GetLeft())
                  ->GetTargetExpression()));
    else if (BOp->GetLeft()->GetASTType() == ASTTypeOpndTy)
      LTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTUnaryOpNode *>(
              dynamic_cast<const ASTOperandNode *>(BOp->GetLeft())
                  ->GetExpression()));
    else
      LTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTUnaryOpNode *>(BOp->GetLeft()));
  } else if (LTy == ASTTypeIdentifier) {
    if (BOp->GetLeft()->GetASTType() == ASTTypeOpTy)
      LTy = dynamic_cast<const ASTOperatorNode *>(BOp->GetLeft())
                ->GetTargetIdentifier()
                ->GetSymbolType();
    else
      LTy = BOp->GetLeft()->GetIdentifier()->GetSymbolType();
  } else if (LTy == ASTTypeOpTy) {
    LTy =
        dynamic_cast<const ASTOperatorNode *>(BOp->GetLeft())->GetTargetType();
  } else if (LTy == ASTTypeOpndTy) {
    LTy = dynamic_cast<const ASTOperandNode *>(BOp->GetLeft())->GetTargetType();
  }

  if (RTy == ASTTypeBinaryOp) {
    if (BOp->GetRight()->GetASTType() == ASTTypeOpTy)
      RTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTBinaryOpNode *>(
              dynamic_cast<const ASTOperatorNode *>(BOp->GetRight())
                  ->GetTargetExpression()));
    else if (BOp->GetRight()->GetASTType() == ASTTypeOpndTy)
      RTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTBinaryOpNode *>(
              dynamic_cast<const ASTOperandNode *>(BOp->GetRight())
                  ->GetExpression()));
    else
      RTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTBinaryOpNode *>(BOp->GetRight()));
  } else if (RTy == ASTTypeUnaryOp) {
    if (BOp->GetRight()->GetASTType() == ASTTypeOpTy)
      RTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTUnaryOpNode *>(
              dynamic_cast<const ASTOperatorNode *>(BOp->GetRight())
                  ->GetTargetExpression()));
    else if (BOp->GetRight()->GetASTType() == ASTTypeOpndTy)
      RTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTUnaryOpNode *>(
              dynamic_cast<const ASTOperandNode *>(BOp->GetRight())
                  ->GetExpression()));
    else
      RTy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTUnaryOpNode *>(BOp->GetRight()));
  } else if (RTy == ASTTypeIdentifier) {
    if (BOp->GetRight()->GetASTType() == ASTTypeOpTy)
      RTy = dynamic_cast<const ASTOperatorNode *>(BOp->GetRight())
                ->GetTargetIdentifier()
                ->GetSymbolType();
    else
      RTy = BOp->GetRight()->GetIdentifier()->GetSymbolType();
  } else if (RTy == ASTTypeOpTy) {
    RTy =
        dynamic_cast<const ASTOperatorNode *>(BOp->GetRight())->GetTargetType();
  } else if (RTy == ASTTypeOpndTy) {
    RTy =
        dynamic_cast<const ASTOperandNode *>(BOp->GetRight())->GetTargetType();
  } else if (RTy == ASTTypeFunctionCall) {
    if (const ASTFunctionCallNode *FC =
            dynamic_cast<const ASTFunctionCallNode *>(BOp->GetRight())) {
      RTy = FC->GetResult()->GetResultType();
    }
  }

  switch (DCX->GetContextType()) {
  case ASTTypeForStatement:
  case ASTTypeWhileStatement:
    if (LTy == ASTTypeUndefined) {
      const_cast<ASTIdentifierNode *>(BOp->GetLeft()->GetIdentifier())
          ->SetSymbolType(ASTTypeInt);
      LTy = BOp->GetLeft()->GetIdentifier()->GetSymbolType();
    }
    if (RTy == ASTTypeUndefined) {
      const_cast<ASTIdentifierNode *>(BOp->GetRight()->GetIdentifier())
          ->SetSymbolType(ASTTypeInt);
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
    if (const ASTUnaryOpNode *UOp =
            dynamic_cast<const ASTUnaryOpNode *>(BOp->GetLeft())) {
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
              DIAGLineCounter::Instance().GetLocation(), M.str(),
              DiagLevel::Error);
        } break;
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
          DIAGLineCounter::Instance().GetLocation(), M.str(),
          DiagLevel::Warning);
    }

    return (IsIntegerType(LTy) && IsIntegerType(RTy)) ||
           (IsAngleType(LTy) && IsIntegerType(RTy));
  } break;
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
               IsAngleType(RTy) || IsTimeType(RTy) || IsStringType(RTy) ||
               IsReturningType(RTy);
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

bool ASTExpressionValidator::Validate(const ASTUnaryOpNode *UOp) const {
  assert(UOp && "Invalid ASTUnaryOpNode argument!");

  ValidateLogicalNot(UOp);

  ASTType ETy = UOp->GetExpression()->GetASTType();

  const ASTDeclarationContext *DCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(DCX && "Could not obtain a valid DeclarationContext!");

  switch (ETy) {
  case ASTTypeOpTy:
    ETy = ResolveOperatorType(
        dynamic_cast<const ASTOperatorNode *>(UOp->GetExpression()), ETy);
    break;
  case ASTTypeOpndTy:
    ETy = ResolveOperandType(
        dynamic_cast<const ASTOperandNode *>(UOp->GetExpression()), ETy);
    break;
  case ASTTypeCast:
    ETy = dynamic_cast<const ASTCastExpressionNode *>(UOp->GetExpression())
              ->GetCastTo();
    break;
  default:
    break;
  }

  if (ETy == ASTTypeBinaryOp) {
    if (UOp->GetExpression()->GetASTType() == ASTTypeOpTy)
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTBinaryOpNode *>(
              dynamic_cast<const ASTOperatorNode *>(UOp->GetExpression())
                  ->GetTargetExpression()));
    else if (UOp->GetExpression()->GetASTType() == ASTTypeOpndTy)
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTBinaryOpNode *>(
              dynamic_cast<const ASTOperandNode *>(UOp->GetExpression())
                  ->GetExpression()));
    else
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTBinaryOpNode *>(UOp->GetExpression()));
  } else if (ETy == ASTTypeUnaryOp) {
    if (UOp->GetExpression()->GetASTType() == ASTTypeOpTy)
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTUnaryOpNode *>(
              dynamic_cast<const ASTOperatorNode *>(UOp->GetExpression())
                  ->GetTargetExpression()));
    else if (UOp->GetExpression()->GetASTType() == ASTTypeOpndTy)
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTUnaryOpNode *>(
              dynamic_cast<const ASTOperandNode *>(UOp->GetExpression())
                  ->GetExpression()));
    else
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(
          dynamic_cast<const ASTUnaryOpNode *>(UOp->GetExpression()));
  } else if (ETy == ASTTypeIdentifier) {
    if (UOp->GetExpression()->GetASTType() == ASTTypeOpTy)
      ETy = dynamic_cast<const ASTOperatorNode *>(UOp->GetExpression())
                ->GetTargetIdentifier()
                ->GetSymbolType();
    else
      ETy = UOp->GetExpression()->GetIdentifier()->GetSymbolType();
  } else if (ETy == ASTTypeOpTy) {
    ETy = dynamic_cast<const ASTOperatorNode *>(UOp->GetExpression())
              ->GetTargetType();
    if (ETy == ASTTypeIdentifier)
      ETy = dynamic_cast<const ASTOperatorNode *>(UOp->GetExpression())
                ->GetTargetIdentifier()
                ->GetSymbolType();
  } else if (ETy == ASTTypeOpndTy) {
    ETy = dynamic_cast<const ASTOperandNode *>(UOp->GetExpression())
              ->GetTargetType();
    if (ETy == ASTTypeIdentifier)
      ETy = dynamic_cast<const ASTOperandNode *>(UOp->GetExpression())
                ->GetIdentifier()
                ->GetSymbolType();
  }

  switch (DCX->GetContextType()) {
  case ASTTypeForStatement:
  case ASTTypeWhileStatement:
    if (ETy == ASTTypeUndefined) {
      const_cast<ASTIdentifierNode *>(UOp->GetExpression()->GetIdentifier())
          ->SetSymbolType(ASTTypeInt);
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
  case ASTOpTypeBitNot:
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

bool ASTExpressionValidator::Validate(const ASTCastExpressionNode *XOp) const {
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

bool ASTExpressionValidator::CanBeAssignedTo(
    const ASTIdentifierNode *Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->IsReference()) {
    const ASTIdentifierRefNode *IdR =
        dynamic_cast<const ASTIdentifierRefNode *>(Id);
    if (IdR) {
      unsigned IX = IdR->GetIndex();
      Id = IdR->GetIdentifier();
      assert(Id && "Could not obtain a valid ASTIdentifierNode!");

      if (IsArrayType(Id->GetSymbolType())) {
        const ASTExpressionNode *EX = Id->GetExpression();
        assert(EX && "Could not obtain a valid ASTExpressionNode!");

        const ASTArrayNode *AN = dynamic_cast<const ASTArrayNode *>(EX);
        assert(AN && "Could not obtain a valid ASTArrayNode!");

        if (IX >= AN->Size()) {
          std::stringstream M;
          M << "Array element index access is out-of-bounds.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(IdR), M.str(),
              DiagLevel::Error);
          return false;
        }

        if (EX && EX->GetASTType() == Id->GetSymbolType())
          return !EX->IsConst();

        const ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().Lookup(
            Id, Id->GetBits(), Id->GetSymbolType());
        assert(STE && "Could not obtain a valid ASTSymbolTable Entry!");

        switch (Id->GetSymbolType()) {
        case ASTTypeCBitArray: {
          ASTCBitArrayNode *AR =
              STE->GetValue()->GetValue<ASTCBitArrayNode *>();
          assert(AR && "Could not obtain a valid ASTCBitArrayNode!");
          return AR && !AR->IsConst();
        } break;
        case ASTTypeQubitArray: {
          ASTQubitArrayNode *AR =
              STE->GetValue()->GetValue<ASTQubitArrayNode *>();
          assert(AR && "Could not obtain a valid ASTQubitArrayNode!");
          return AR && !AR->IsConst();
        } break;
        case ASTTypeAngleArray: {
          ASTAngleArrayNode *AR =
              STE->GetValue()->GetValue<ASTAngleArrayNode *>();
          assert(AR && "Could not obtain a valid ASTAngleArrayNode!");
          return AR && !AR->IsConst();
        } break;
        case ASTTypeBoolArray: {
          ASTBoolArrayNode *AR =
              STE->GetValue()->GetValue<ASTBoolArrayNode *>();
          assert(AR && "Could not obtain a valid ASTBoolArrayNode!");
          return AR && !AR->IsConst();
        } break;
        case ASTTypeIntArray: {
          ASTIntArrayNode *AR = STE->GetValue()->GetValue<ASTIntArrayNode *>();
          assert(AR && "Could not obtain a valid ASTIntArrayNode!");
          return AR && !AR->IsConst();
        } break;
        case ASTTypeMPIntegerArray: {
          ASTMPIntegerArrayNode *AR =
              STE->GetValue()->GetValue<ASTMPIntegerArrayNode *>();
          assert(AR && "Could not obtain a valid ASTMPIntegerArrayNode!");
          return !AR->IsConst();
        } break;
        case ASTTypeFloatArray: {
          ASTFloatArrayNode *AR =
              STE->GetValue()->GetValue<ASTFloatArrayNode *>();
          assert(AR && "Could not obtain a valid ASTFloatArrayNode!");
          return AR && !AR->IsConst();
        } break;
        case ASTTypeMPDecimalArray: {
          ASTMPDecimalArrayNode *AR =
              STE->GetValue()->GetValue<ASTMPDecimalArrayNode *>();
          assert(AR && "Could not obtain a valid ASTMPDecimalArrayNode!");
          return AR && !AR->IsConst();
        } break;
        case ASTTypeMPComplexArray: {
          ASTMPComplexArrayNode *AR =
              STE->GetValue()->GetValue<ASTMPComplexArrayNode *>();
          assert(AR && "Could not obtain a valid ASTMPComplexArrayNode!");
          return AR && !AR->IsConst();
        } break;
        case ASTTypeDurationArray: {
          ASTDurationArrayNode *AR =
              STE->GetValue()->GetValue<ASTDurationArrayNode *>();
          assert(AR && "Could not obtain a valid ASTDurationArrayNode!");
          return AR && !AR->IsConst();
        } break;
        case ASTTypeOpenPulseFrameArray: {
          ASTOpenPulseFrameArrayNode *AR =
              STE->GetValue()->GetValue<ASTOpenPulseFrameArrayNode *>();
          assert(AR && "Could not obtain a valid ASTOpenPulseFrameArrayNode!");
          return AR && !AR->IsConst();
        } break;
        case ASTTypeOpenPulsePortArray: {
          ASTOpenPulsePortArrayNode *AR =
              STE->GetValue()->GetValue<ASTOpenPulsePortArrayNode *>();
          assert(AR && "Could not obtain a valid ASTOpenPulsePortArrayNode!");
          return AR && !AR->IsConst();
        } break;
        case ASTTypeOpenPulseWaveformArray: {
          ASTOpenPulseWaveformArrayNode *AR =
              STE->GetValue()->GetValue<ASTOpenPulseWaveformArrayNode *>();
          assert(AR &&
                 "Could not obtain a valid ASTOpenPulseWaveformArrayNode!");
          return AR && !AR->IsConst();
        } break;
        default: {
          std::stringstream M;
          M << "Unknown/Invalid array type "
            << PrintTypeEnum(Id->GetSymbolType()) << '.';
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
              DiagLevel::Error);
          return false;
        } break;
        }
      } else {
        const ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().Lookup(
            Id, Id->GetBits(), Id->GetSymbolType());
        assert(STE && "Could not obtain a valid ASTSymbolTable Entry!");

        switch (Id->GetSymbolType()) {
        case ASTTypeBitset: {
          ASTCBitNode *E = STE->GetValue()->GetValue<ASTCBitNode *>();
          assert(E && "Could not obtain a valid ASTCBitNode!");

          if (E && IX >= E->Size()) {
            std::stringstream M;
            M << "Bitset indexed element access is out-of-bounds.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(IdR), M.str(),
                DiagLevel::Error);
            return false;
          }

          return E && !E->IsConst();
        } break;
        case ASTTypeQubitContainer: {
          ASTQubitContainerNode *E =
              STE->GetValue()->GetValue<ASTQubitContainerNode *>();
          assert(E && "Could not obtain a valid ASTQubitContainerNode!");

          if (E && IX >= E->Size()) {
            std::stringstream M;
            M << "QubitContainer indexed element access is out-of-bounds.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(IdR), M.str(),
                DiagLevel::Error);
            return false;
          }

          return E && !E->IsConst();
        } break;
        case ASTTypeQubitContainerAlias: {
          ASTQubitContainerAliasNode *E =
              STE->GetValue()->GetValue<ASTQubitContainerAliasNode *>();
          assert(E && "Could not obtain a valid ASTQubitContainerAliasNode!");

          if (E && IX >= E->Size()) {
            std::stringstream M;
            M << "QubitContainerAlias indexed element access is out-of-bounds.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(IdR), M.str(),
                DiagLevel::Error);
            return false;
          }

          return E && !E->IsConst();
        } break;
        case ASTTypeAngle: {
          if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
            std::stringstream M;
            M << "Cannot assign to a reserved keyword constant.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(IdR), M.str(),
                DiagLevel::Error);
            return false;
          }

          ASTAngleNode *E = STE->GetValue()->GetValue<ASTAngleNode *>();
          assert(E && "Could not obtain a valid ASTAngleNode!");
          return E && !E->IsConst();
        } break;
        default: {
          std::stringstream M;
          M << "Unknown indexed reference type "
            << PrintTypeEnum(Id->GetSymbolType()) << '.';
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
              DiagLevel::Error);
          return false;
          break;
        }
        }
      }
    } else {
      std::stringstream M;
      M << "Indexed ASTIdentifierNode does not have a valid "
        << "ASTIdentifierRefNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return false;
    }
  } else {
    const ASTSymbolTableEntry *STE = Id->GetSymbolTableEntry();
    assert(STE && "Could not obtain a valid ASTSymbolTable Entry!");

    switch (Id->GetSymbolType()) {
    case ASTTypeBool: {
      ASTBoolNode *E = STE->GetValue()->GetValue<ASTBoolNode *>();
      assert(E && "Could not obtain a valid ASTBoolNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeInt:
    case ASTTypeUInt: {
      ASTIntNode *E = STE->GetValue()->GetValue<ASTIntNode *>();
      assert(E && "Could not obtain a valid ASTIntNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeFloat: {
      ASTFloatNode *E = STE->GetValue()->GetValue<ASTFloatNode *>();
      assert(E && "Could not obtain a valid ASTFloatNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeDouble: {
      ASTDoubleNode *E = STE->GetValue()->GetValue<ASTDoubleNode *>();
      assert(E && "Could not obtain a valid ASTDoubleNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeBitset: {
      ASTCBitNode *E = STE->GetValue()->GetValue<ASTCBitNode *>();
      assert(E && "Could not obtain a valid ASTCBitNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger: {
      ASTMPIntegerNode *E = STE->GetValue()->GetValue<ASTMPIntegerNode *>();
      assert(E && "Could not obtain a valid ASTMPIntegerNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeMPDecimal: {
      ASTMPDecimalNode *E = STE->GetValue()->GetValue<ASTMPDecimalNode *>();
      assert(E && "Could not obtain a valid ASTMPDecimalNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeMPComplex: {
      ASTMPComplexNode *E = STE->GetValue()->GetValue<ASTMPComplexNode *>();
      assert(E && "Could not obtain a valid ASTMPComplexNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeAngle: {
      if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
        std::stringstream M;
        M << "Cannot assign to a reserved keyword constant.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
            DiagLevel::Error);
        return false;
      }

      ASTAngleNode *E = STE->GetValue()->GetValue<ASTAngleNode *>();
      assert(E && "Could not obtain a valid ASTAngleNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeEulerAngle:
    case ASTTypeTauAngle:
    case ASTTypePiAngle: {
      std::stringstream M;
      M << "Cannot assign to a reserved keyword constant.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return false;
    } break;
    case ASTTypeLambdaAngle:
    case ASTTypePhiAngle:
    case ASTTypeThetaAngle: {
      ASTAngleNode *E = STE->GetValue()->GetValue<ASTAngleNode *>();
      assert(E && "Could not obtain a valid ASTAngleNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeQubit: {
      ASTQubitNode *E = STE->GetValue()->GetValue<ASTQubitNode *>();
      assert(E && "Could not obtain a valid ASTQubitNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeQubitContainer: {
      ASTQubitContainerNode *E =
          STE->GetValue()->GetValue<ASTQubitContainerNode *>();
      assert(E && "Could not obtain a valid ASTQubitContainerNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeQubitContainerAlias: {
      ASTQubitContainerAliasNode *E =
          STE->GetValue()->GetValue<ASTQubitContainerAliasNode *>();
      assert(E && "Could not obtain a valid ASTQubitContainerAliasNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeDuration: {
      ASTDurationNode *E = STE->GetValue()->GetValue<ASTDurationNode *>();
      assert(E && "Could not obtain a valid ASTDurationNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeOpenPulsePort: {
      OpenPulse::ASTOpenPulsePortNode *E =
          STE->GetValue()->GetValue<OpenPulse::ASTOpenPulsePortNode *>();
      assert(E && "Could not obtain a valid ASTOpenPulsePortNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeOpenPulseWaveform: {
      OpenPulse::ASTOpenPulseWaveformNode *E =
          STE->GetValue()->GetValue<OpenPulse::ASTOpenPulseWaveformNode *>();
      assert(E && "Could not obtain a valid ASTOpenPulseWaveformNode!");
      return E && !E->IsConst();
    } break;
    case ASTTypeOpenPulseFrame: {
      OpenPulse::ASTOpenPulseFrameNode *E =
          STE->GetValue()->GetValue<OpenPulse::ASTOpenPulseFrameNode *>();
      assert(E && "Could not obtain a valid ASTOpenPulseFrameNode!");
      return E && !E->IsConst();
    } break;
    default: {
      std::stringstream M;
      M << "Invalid Expression Type " << PrintTypeEnum(Id->GetSymbolType())
        << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return false;
    } break;
    }
  }

  return false;
}

bool ASTExpressionValidator::CanBeAssignedTo(
    const ASTExpressionNode *EX) const {
  assert(EX && "Invalid ASTExpressionNode argument!");

  if (EX->GetASTType() == ASTTypeIdentifier)
    return CanBeAssignedTo(EX->GetIdentifier());

  switch (EX->GetASTType()) {
  case ASTTypeCBitArray: {
    const ASTCBitArrayNode *AR = dynamic_cast<const ASTCBitArrayNode *>(EX);
    assert(AR && "Could not obtain a valid ASTCBitArrayNode!");
    return AR && !AR->IsConst();
  } break;
  case ASTTypeQubitArray: {
    const ASTQubitArrayNode *AR = dynamic_cast<const ASTQubitArrayNode *>(EX);
    assert(AR && "Could not obtain a valid ASTQubitArrayNode!");
    return AR && !AR->IsConst();
  } break;
  case ASTTypeAngleArray: {
    const ASTAngleArrayNode *AR = dynamic_cast<const ASTAngleArrayNode *>(EX);
    assert(AR && "Could not obtain a valid ASTAngleArrayNode!");
    return AR && !AR->IsConst();
  } break;
  case ASTTypeBoolArray: {
    const ASTBoolArrayNode *AR = dynamic_cast<const ASTBoolArrayNode *>(EX);
    assert(AR && "Could not obtain a valid ASTBoolArrayNode!");
    return AR && !AR->IsConst();
  } break;
  case ASTTypeIntArray: {
    const ASTIntArrayNode *AR = dynamic_cast<const ASTIntArrayNode *>(EX);
    assert(AR && "Could not obtain a valid ASTIntArrayNode!");
    return AR && !AR->IsConst();
  } break;
  case ASTTypeMPIntegerArray: {
    const ASTMPIntegerArrayNode *AR =
        dynamic_cast<const ASTMPIntegerArrayNode *>(EX);
    assert(AR && "Could not obtain a valid ASTMPIntegerArrayNode!");
    return AR && !AR->IsConst();
  } break;
  case ASTTypeFloatArray: {
    const ASTFloatArrayNode *AR = dynamic_cast<const ASTFloatArrayNode *>(EX);
    assert(AR && "Could not obtain a valid ASTFloatArrayNode!");
    return AR && !AR->IsConst();
  } break;
  case ASTTypeMPDecimalArray: {
    const ASTMPDecimalArrayNode *AR =
        dynamic_cast<const ASTMPDecimalArrayNode *>(EX);
    assert(AR && "Could not obtain a valid ASTMPDecimalArrayNode!");
    return AR && !AR->IsConst();
  } break;
  case ASTTypeMPComplexArray: {
    const ASTMPComplexArrayNode *AR =
        dynamic_cast<const ASTMPComplexArrayNode *>(EX);
    assert(AR && "Could not obtain a valid ASTMPComplexArrayNode!");
    return AR && !AR->IsConst();
  } break;
  case ASTTypeDurationArray: {
    const ASTDurationArrayNode *AR =
        dynamic_cast<const ASTDurationArrayNode *>(EX);
    assert(AR && "Could not obtain a valid ASTDurationArrayNode!");
    return AR && !AR->IsConst();
  } break;
  case ASTTypeOpenPulseFrameArray: {
    const ASTOpenPulseFrameArrayNode *AR =
        dynamic_cast<const ASTOpenPulseFrameArrayNode *>(EX);
    assert(AR && "Could not obtain a valid ASTOpenPulseFrameArrayNode!");
    return AR && !AR->IsConst();
  } break;
  case ASTTypeOpenPulsePortArray: {
    const ASTOpenPulsePortArrayNode *AR =
        dynamic_cast<const ASTOpenPulsePortArrayNode *>(EX);
    assert(AR && "Could not obtain a valid ASTOpenPulsePortArrayNode!");
    return AR && !AR->IsConst();
  } break;
  case ASTTypeOpenPulseWaveformArray: {
    const ASTOpenPulseWaveformArrayNode *AR =
        dynamic_cast<const ASTOpenPulseWaveformArrayNode *>(EX);
    assert(AR && "Could not obtain a valid ASTOpenPulseWaveformArrayNode!");
    return AR && !AR->IsConst();
  } break;
  case ASTTypeBool: {
    const ASTBoolNode *E = dynamic_cast<const ASTBoolNode *>(EX);
    assert(E && "Could not obtain a valid ASTBoolNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeInt:
  case ASTTypeUInt: {
    const ASTIntNode *E = dynamic_cast<const ASTIntNode *>(EX);
    assert(E && "Could not obtain a valid ASTIntNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeFloat: {
    const ASTFloatNode *E = dynamic_cast<const ASTFloatNode *>(EX);
    assert(E && "Could not obtain a valid ASTFloatNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeDouble: {
    const ASTDoubleNode *E = dynamic_cast<const ASTDoubleNode *>(EX);
    assert(E && "Could not obtain a valid ASTDoubleNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeBitset: {
    const ASTCBitNode *E = dynamic_cast<const ASTCBitNode *>(EX);
    assert(E && "Could not obtain a valid ASTCBitNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger: {
    const ASTMPIntegerNode *E = dynamic_cast<const ASTMPIntegerNode *>(EX);
    assert(E && "Could not obtain a valid ASTMPIntegerNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeMPDecimal: {
    const ASTMPDecimalNode *E = dynamic_cast<const ASTMPDecimalNode *>(EX);
    assert(E && "Could not obtain a valid ASTMPDecimalNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeMPComplex: {
    const ASTMPComplexNode *E = dynamic_cast<const ASTMPComplexNode *>(EX);
    assert(E && "Could not obtain a valid ASTMPComplexNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeAngle: {
    const ASTAngleNode *E = dynamic_cast<const ASTAngleNode *>(EX);
    assert(E && "Could not obtain a valid ASTAngleNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeQubit: {
    const ASTQubitNode *E = dynamic_cast<const ASTQubitNode *>(EX);
    assert(E && "Could not obtain a valid ASTQubitNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeQubitContainer: {
    const ASTQubitContainerNode *E =
        dynamic_cast<const ASTQubitContainerNode *>(EX);
    assert(E && "Could not obtain a valid ASTQubitContainerNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeQubitContainerAlias: {
    const ASTQubitContainerAliasNode *E =
        dynamic_cast<const ASTQubitContainerAliasNode *>(EX);
    assert(E && "Could not obtain a valid ASTQubitContainerAliasNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeDuration: {
    const ASTDurationNode *E = dynamic_cast<const ASTDurationNode *>(EX);
    assert(E && "Could not obtain a valid ASTDurationNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeOpenPulsePort: {
    const OpenPulse::ASTOpenPulsePortNode *E =
        dynamic_cast<const OpenPulse::ASTOpenPulsePortNode *>(EX);
    assert(E && "Could not obtain a valid ASTOpenPulsePortNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeOpenPulseWaveform: {
    const OpenPulse::ASTOpenPulseWaveformNode *E =
        dynamic_cast<const OpenPulse::ASTOpenPulseWaveformNode *>(EX);
    assert(E && "Could not obtain a valid ASTOpenPulseWaveformNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeOpenPulseFrame: {
    const OpenPulse::ASTOpenPulseFrameNode *E =
        dynamic_cast<const OpenPulse::ASTOpenPulseFrameNode *>(EX);
    assert(E && "Could not obtain a valid ASTOpenPulseFrameNode!");
    return E && !E->IsConst();
  } break;
  case ASTTypeBinaryOp:
  case ASTTypeUnaryOp: {
    std::stringstream M;
    M << "An lvalue is required as left operand of assignment expression.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(EX), M.str(), DiagLevel::Error);
    return false;
  } break;
  default: {
    std::stringstream M;
    M << "Invalid Expression Type " << PrintTypeEnum(EX->GetASTType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(EX), M.str(), DiagLevel::Error);
    return false;
  } break;
  }

  return false;
}

} // namespace QASM
