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

#include <qasm/AST/ASTBinaryOpAssignBuilder.h>
#include <qasm/AST/ASTCastExpr.h>
#include <qasm/AST/ASTExpressionEvaluator.h>
#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTImplicitConversionExpr.h>

namespace QASM {

ASTBinaryOpAssignBuilder ASTBinaryOpAssignBuilder::BOB;

ASTType ASTBinaryOpNode::GetExpressionType() const {
  ASTType LTy = Left->GetASTType();
  ASTType RTy = Right->GetASTType();

  switch (LTy) {
  case ASTTypeOpTy:
    if (const ASTOperatorNode *OPR =
            dynamic_cast<const ASTOperatorNode *>(Left)) {
      LTy = OPR->GetEvaluatedTargetType();
      if (LTy == ASTTypeIdentifier || LTy == ASTTypeIdentifierRef)
        LTy = OPR->GetTargetIdentifier()->GetSymbolType();
    }
    break;
  case ASTTypeOpndTy:
    if (const ASTOperandNode *OPN =
            dynamic_cast<const ASTOperandNode *>(Left)) {
      LTy = OPN->GetEvaluatedTargetType();
      if (LTy == ASTTypeIdentifier || LTy == ASTTypeIdentifierRef)
        LTy = OPN->GetTargetIdentifier()->GetSymbolType();
    }
    break;
  case ASTTypeCast:
    if (const ASTCastExpressionNode *CXN =
            dynamic_cast<const ASTCastExpressionNode *>(Left)) {
      LTy = CXN->GetCastTo();
    }
    break;
  case ASTTypeImplicitConversion:
    if (const ASTImplicitConversionNode *ICX =
            dynamic_cast<const ASTImplicitConversionNode *>(Left)) {
      LTy = ICX->GetConvertTo();
    }
    break;
  case ASTTypeFunctionCall:
    if (const ASTFunctionCallNode *FCN =
            dynamic_cast<const ASTFunctionCallNode *>(Left)) {
      LTy = FCN->GetResultType();
    }
    break;
  case ASTTypeBinaryOp:
    if (const ASTBinaryOpNode *BOP =
            dynamic_cast<const ASTBinaryOpNode *>(Left)) {
      LTy = BOP->GetExpressionType();
    }
    break;
  case ASTTypeUnaryOp:
    if (const ASTUnaryOpNode *UOP = dynamic_cast<const ASTUnaryOpNode *>(Left))
      LTy = UOP->GetExpressionType();
    break;
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
    LTy = Left->GetIdentifier()->GetSymbolType();
    break;
  default:
    break;
  }

  switch (RTy) {
  case ASTTypeOpTy:
    if (const ASTOperatorNode *OPR =
            dynamic_cast<const ASTOperatorNode *>(Right)) {
      RTy = OPR->GetEvaluatedTargetType();
      if (RTy == ASTTypeIdentifier || RTy == ASTTypeIdentifierRef)
        RTy = OPR->GetTargetIdentifier()->GetSymbolType();
    }
    break;
  case ASTTypeOpndTy:
    if (const ASTOperandNode *OPN =
            dynamic_cast<const ASTOperandNode *>(Right)) {
      RTy = OPN->GetEvaluatedTargetType();
      if (RTy == ASTTypeIdentifier || RTy == ASTTypeIdentifierRef)
        RTy = OPN->GetTargetIdentifier()->GetSymbolType();
    }
    break;
  case ASTTypeCast:
    if (const ASTCastExpressionNode *CXN =
            dynamic_cast<const ASTCastExpressionNode *>(Right)) {
      RTy = CXN->GetCastTo();
    }
    break;
  case ASTTypeImplicitConversion:
    if (const ASTImplicitConversionNode *ICX =
            dynamic_cast<const ASTImplicitConversionNode *>(Right)) {
      RTy = ICX->GetConvertTo();
    }
    break;
  case ASTTypeFunctionCall:
    if (const ASTFunctionCallNode *FCN =
            dynamic_cast<const ASTFunctionCallNode *>(Right)) {
      RTy = FCN->GetResultType();
    }
    break;
  case ASTTypeBinaryOp:
    if (const ASTBinaryOpNode *BOP =
            dynamic_cast<const ASTBinaryOpNode *>(Right)) {
      RTy = BOP->GetExpressionType();
    }
    break;
  case ASTTypeUnaryOp:
    if (const ASTUnaryOpNode *UOP = dynamic_cast<const ASTUnaryOpNode *>(Right))
      RTy = UOP->GetExpressionType();
    break;
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
    RTy = Right->GetIdentifier()->GetSymbolType();
    break;
  default:
    break;
  }

  assert(LTy != ASTTypeUndefined &&
         "Left ASTType should not be ASTTypeUndefined!");
  assert(RTy != ASTTypeUndefined &&
         "Right ASTType should not be ASTTypeUndefined!");
  assert(LTy != ASTTypeIdentifier &&
         "Left ASTType should not be ASTTypeIdentifier!");
  assert(RTy != ASTTypeIdentifier &&
         "Right ASTType should not be ASTTypeIdentifier!");

  unsigned LR = ASTExpressionEvaluator::Instance().GetRank(LTy);
  unsigned RR = ASTExpressionEvaluator::Instance().GetRank(RTy);

  if (LR == RR)
    return LTy;

  return LR > RR ? LTy : RTy;
}

ASTType ASTUnaryOpNode::GetExpressionType() const {
  switch (RTy) {
  case ASTTypeOpTy:
    if (const ASTOperatorNode *OPR =
            dynamic_cast<const ASTOperatorNode *>(Right)) {
      RTy = OPR->GetEvaluatedTargetType();
      if (RTy == ASTTypeIdentifier || RTy == ASTTypeIdentifierRef)
        RTy = OPR->GetTargetIdentifier()->GetSymbolType();
    }
    break;
  case ASTTypeOpndTy:
    if (const ASTOperandNode *OPN =
            dynamic_cast<const ASTOperandNode *>(Right)) {
      RTy = OPN->GetEvaluatedTargetType();
      if (RTy == ASTTypeIdentifier || RTy == ASTTypeIdentifierRef)
        RTy = OPN->GetTargetIdentifier()->GetSymbolType();
    }
    break;
  case ASTTypeCast:
    if (const ASTCastExpressionNode *CXN =
            dynamic_cast<const ASTCastExpressionNode *>(Right)) {
      RTy = CXN->GetCastTo();
    }
    break;
  case ASTTypeImplicitConversion:
    if (const ASTImplicitConversionNode *ICX =
            dynamic_cast<const ASTImplicitConversionNode *>(Right)) {
      RTy = ICX->GetConvertTo();
    }
    break;
  case ASTTypeFunctionCall:
    if (const ASTFunctionCallNode *FCN =
            dynamic_cast<const ASTFunctionCallNode *>(Right)) {
      RTy = FCN->GetResultType();
    }
    break;
  case ASTTypeBinaryOp:
    if (const ASTBinaryOpNode *BOP =
            dynamic_cast<const ASTBinaryOpNode *>(Right)) {
      RTy = BOP->GetExpressionType();
    }
    break;
  case ASTTypeUnaryOp:
    if (const ASTUnaryOpNode *UOP = dynamic_cast<const ASTUnaryOpNode *>(Right))
      RTy = UOP->GetExpressionType();
    break;
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
    RTy = Right->GetIdentifier()->GetSymbolType();
    break;
  default:
    break;
  }

  assert(RTy != ASTTypeUndefined &&
         "Right ASTType should not be ASTTypeUndefined!");
  assert(RTy != ASTTypeIdentifier &&
         "Right ASTType should not be ASTTypeIdentifier!");
  return RTy;
}

} // namespace QASM
