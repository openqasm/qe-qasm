
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

#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTCastExpr.h>
#include <qasm/AST/ASTExpressionValidator.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTTypeCastController.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <iostream>
#include <string>

namespace QASM {

ASTTypeCastController ASTTypeCastController::TCC;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

bool ASTTypeCastController::CanCast(ASTType From, ASTType To) const {
  if (From == To)
    return true;

  switch (From) {
  case ASTTypeBitset:
    return ASTExpressionValidator::Instance().IsScalarIntegerType(To) ||
           To == ASTTypeAngle || To == ASTTypeMPInteger;
    break;
  case ASTTypeBool:
    return ASTExpressionValidator::Instance().IsScalarIntegerType(To) ||
           ASTExpressionValidator::Instance().IsFloatingPointType(To);
    break;
  case ASTTypeInt:
  case ASTTypeUInt:
  case ASTTypeFloat:
  case ASTTypeDouble:
  case ASTTypeLongDouble:
  case ASTTypeMPInteger:
  case ASTTypeMPDecimal:
    return ASTExpressionValidator::Instance().IsScalarIntegerType(To) ||
           ASTExpressionValidator::Instance().IsFloatingPointType(To) ||
           To == ASTTypeAngle || To == ASTTypeMPInteger ||
           To == ASTTypeMPDecimal || To == ASTTypeMPComplex;
    break;
  case ASTTypeAngle:
    switch (To) {
    case ASTTypeBitset:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeLongDouble:
    case ASTTypeMPInteger:
    case ASTTypeMPDecimal:
    case ASTTypeAngle:
      return true;
      break;
    default:
      return false;
      break;
    }
    break;
  case ASTTypeUTF8:
    return ASTExpressionValidator::Instance().IsScalarIntegerType(To) ||
           ASTExpressionValidator::Instance().IsFloatingPointType(To);
    break;
  case ASTTypeMPComplex:
    return To == ASTTypeFloat || To == ASTTypeDouble ||
           To == ASTTypeLongDouble || To == ASTTypeAngle ||
           To == ASTTypeMPDecimal;
    break;
  case ASTTypeBinaryOp:
  case ASTTypeUnaryOp:
    return ASTExpressionValidator::Instance().IsScalarIntegerType(To) ||
           ASTExpressionValidator::Instance().IsFloatingPointType(To) ||
           To == ASTTypeAngle || To == ASTTypeMPInteger ||
           To == ASTTypeMPDecimal || To == ASTTypeMPComplex;
    break;
  default:
    return false;
    break;
  }

  return false;
}

bool ASTTypeCastController::CanCast(const ASTIdentifierNode *From,
                                    ASTType To) const {
  assert(From && "Invalid ASTIdentifierNode argument!");

  if (From->IsReference()) {
    const ASTIdentifierRefNode *IdR =
        dynamic_cast<const ASTIdentifierRefNode *>(From);
    if (!IdR) {
      std::stringstream M;
      M << "An ASTIdentifierNode reference cannot be an ASTIdentifierNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(From), M.str(),
          DiagLevel::Error);
      return false;
    }

    return CanCast(IdR->GetSymbolType(), To);
  }

  return CanCast(From->GetSymbolType(), To);
}

bool ASTTypeCastController::CanCast(const ASTExpressionNode *From,
                                    ASTType To) const {
  assert(From && "Invalid ASTExpressionNode argument!");

  switch (From->GetASTType()) {
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
    return CanCast(From->GetIdentifier(), To);
    break;
  case ASTTypeBinaryOp: {
    if (const ASTBinaryOpNode *BOP =
            dynamic_cast<const ASTBinaryOpNode *>(From)) {
      ASTType BTy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
      return CanCast(BTy, To);
    }
  } break;
  case ASTTypeUnaryOp: {
    if (const ASTUnaryOpNode *UOP =
            dynamic_cast<const ASTUnaryOpNode *>(From)) {
      ASTType UTy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
      return CanCast(UTy, To);
    }
  } break;
  case ASTTypeImplicitConversion: {
    if (const ASTImplicitConversionNode *ICX =
            dynamic_cast<const ASTImplicitConversionNode *>(From)) {
      ASTType ICTy = ASTExpressionEvaluator::Instance().EvaluatesTo(ICX);
      return CanCast(ICTy, To);
    }
  } break;
  default:
    return CanCast(From->GetASTType(), To);
    break;
  }

  return false;
}

bool ASTTypeCastController::CanCast(const ASTCastExpressionNode *XC) const {
  assert(XC && "Invalid ASTCastExpressionNode argument!");

  switch (XC->GetCastFrom()) {
  case ASTTypeUnaryOp: {
    const ASTUnaryOpNode *UOP = XC->GetUnaryOp();
    assert(UOP &&
           "Invalid ASTUnaryOpNode obtained from ASTCastExpressionNode!");
    ASTType OTy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
    return CanCast(OTy, XC->GetCastTo());
  } break;
  case ASTTypeBinaryOp: {
    const ASTBinaryOpNode *BOP = XC->GetBinaryOp();
    assert(BOP &&
           "Invalid ASTBinaryOpNode obtained from ASTCastExpressionNode!");
    ASTType OTy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
    return CanCast(OTy, XC->GetCastTo());
  } break;
  default:
    return CanCast(XC->GetCastFrom(), XC->GetCastTo());
    break;
  }

  return false;
}

bool ASTTypeCastController::CanCast(const ASTBinaryOpNode *From,
                                    ASTType To) const {
  assert(From && "Invalid ASTBinaryOpNode argument!");
  ASTType OTy = ASTExpressionEvaluator::Instance().EvaluatesTo(From);
  return CanCast(OTy, To);
}

bool ASTTypeCastController::CanCast(const ASTUnaryOpNode *From,
                                    ASTType To) const {
  assert(From && "Invalid ASTUnaryOpNode argument!");
  ASTType OTy = ASTExpressionEvaluator::Instance().EvaluatesTo(From);
  return CanCast(OTy, To);
}

bool ASTTypeCastController::CanImplicitConvert(ASTType From, ASTType To) const {
  if (From == To)
    return true;

  switch (From) {
  case ASTTypeBool:
  case ASTTypeInt:
  case ASTTypeUInt:
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
  case ASTTypeMPDecimal:
    return ASTExpressionValidator::Instance().IsIntegerType(To) ||
           ASTExpressionValidator::Instance().IsFloatingPointType(To);
    break;
  default:
    break;
  }

  return false;
}

bool ASTTypeCastController::CanImplicitConvert(const ASTBinaryOpNode *From,
                                               ASTType To) const {
  assert(From && "Invalid ASTBinaryOpNode argument!");
  ASTType OTy = ASTExpressionEvaluator::Instance().EvaluatesTo(From);
  return CanImplicitConvert(OTy, To);
}

bool ASTTypeCastController::CanImplicitConvert(const ASTUnaryOpNode *From,
                                               ASTType To) const {
  assert(From && "Invalid ASTUnaryOpNode argument!");
  ASTType OTy = ASTExpressionEvaluator::Instance().EvaluatesTo(From);
  return CanImplicitConvert(OTy, To);
}

bool ASTTypeCastController::CanImplicitConvert(const ASTIdentifierNode *From,
                                               ASTType To) const {
  assert(From && "Invalid ASTIdentifierNode argument!");

  if (From->IsReference()) {
    const ASTIdentifierRefNode *IdR =
        dynamic_cast<const ASTIdentifierRefNode *>(From);
    if (!IdR) {
      std::stringstream M;
      M << "An ASTIdentifierNode reference cannot be an ASTIdentifierNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(From), M.str(),
          DiagLevel::Error);
      return false;
    }

    return CanImplicitConvert(IdR->GetSymbolType(), To);
  }

  return CanImplicitConvert(From->GetSymbolType(), To);
}

bool ASTTypeCastController::CanImplicitConvert(const ASTExpressionNode *From,
                                               ASTType To) const {
  assert(From && "Invalid ASTExpressionNode argument!");

  switch (From->GetASTType()) {
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
    return CanImplicitConvert(From->GetIdentifier(), To);
    break;
  default:
    return CanImplicitConvert(From->GetASTType(), To);
    break;
  }

  return false;
}

bool ASTTypeCastController::CanImplicitConvert(const ASTCastExpressionNode *XC,
                                               ASTType To) const {
  assert(XC && "Invalid ASTCastExpressionNode argument!");
  ASTType OTy = XC->GetCastTo();
  return CanImplicitConvert(OTy, To);
}

ASTTypeConversionMethod
ASTTypeCastController::ResolveConversionMethod(ASTType From, ASTType To) const {
  if (!ASTTypeCastController::Instance().CanCast(From, To))
    return ASTConvMethodBadCast;

  switch (From) {
  case ASTTypeBool:
    switch (To) {
    case ASTTypeBool:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeBitset:
    case ASTTypeAngle:
      return ASTConvMethodBitcast;
      break;
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeMPDecimal:
      return ASTConvMethodPromotion;
      break;
    case ASTTypeMPComplex:
      return ASTConvMethodConversion;
      break;
    default:
      return ASTConvMethodBadCast;
      break;
    }
    break;
  case ASTTypeInt:
  case ASTTypeUInt:
    switch (To) {
    case ASTTypeBool:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeBitset:
    case ASTTypeAngle:
      return ASTConvMethodBitcast;
      break;
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeMPDecimal:
      return ASTConvMethodPromotion;
      break;
    case ASTTypeMPComplex:
      return ASTConvMethodConversion;
      break;
    default:
      return ASTConvMethodBadCast;
      break;
    }
    break;
  case ASTTypeFloat:
  case ASTTypeDouble:
  case ASTTypeMPDecimal:
    switch (To) {
    case ASTTypeBool:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
      return ASTConvMethodTruncation;
      break;
    case ASTTypeMPDecimal:
    case ASTTypeAngle:
    case ASTTypeBitset:
      return ASTConvMethodBitcast;
      break;
    case ASTTypeMPComplex:
      return ASTConvMethodConversion;
      break;
    default:
      return ASTConvMethodBadCast;
      break;
    }
    break;
  case ASTTypeMPComplex:
    switch (To) {
    case ASTTypeBool:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeMPDecimal:
    case ASTTypeAngle:
      return ASTConvMethodConversion;
      break;
    case ASTTypeMPComplex:
      return ASTConvMethodBitcast;
      break;
    default:
      return ASTConvMethodBadCast;
      break;
    }
    break;
  case ASTTypeBitset:
    switch (To) {
    case ASTTypeBool:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeAngle:
      return ASTConvMethodBitcast;
      break;
    default:
      return ASTConvMethodBadCast;
      break;
    }
    break;
  case ASTTypeAngle:
    switch (To) {
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeBitset:
      return ASTConvMethodBitcast;
      break;
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeMPDecimal:
    case ASTTypeMPComplex:
      return ASTConvMethodConversion;
      break;
    default:
      return ASTConvMethodBadCast;
      break;
    }
    break;
  default:
    return ASTConvMethodBadCast;
    break;
  }

  return ASTConvMethodBadCast;
}

ASTTypeConversionMethod
ASTTypeCastController::ResolveConversionMethod(const ASTIdentifierNode *Id,
                                               ASTType To) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->IsReference()) {
    const ASTIdentifierRefNode *IdR =
        dynamic_cast<const ASTIdentifierRefNode *>(Id);
    if (!IdR) {
      std::stringstream M;
      M << "An ASTIdentifierNode reference cannot be an ASTIdentifierNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return ASTConvMethodBadCast;
    }

    return ResolveConversionMethod(IdR->GetSymbolType(), To);
  }

  return ResolveConversionMethod(Id->GetSymbolType(), To);
}

ASTTypeConversionMethod
ASTTypeCastController::ResolveConversionMethod(const ASTBinaryOpNode *BOP,
                                               ASTType To) const {
  assert(BOP && "Invalid ASTBinaryOpNode argument!");
  ASTType OTy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
  return ResolveConversionMethod(OTy, To);
}

ASTTypeConversionMethod
ASTTypeCastController::ResolveConversionMethod(const ASTUnaryOpNode *UOP,
                                               ASTType To) const {
  assert(UOP && "Invalid ASTUnaryOpNode argument!");
  ASTType OTy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
  return ResolveConversionMethod(OTy, To);
}

ASTTypeConversionMethod
ASTTypeCastController::ResolveConversionMethod(const ASTExpressionNode *EX,
                                               ASTType To) const {
  assert(EX && "Invalid ASTExpressionNode argument!");

  if (EX->GetASTType() == ASTTypeIdentifier) {
    const ASTIdentifierNode *Id = EX->GetIdentifier();
    assert(Id && "Could not obtain a valid ASTIdentifierNode!");

    if (Id->IsReference()) {
      const ASTIdentifierRefNode *IdR =
          dynamic_cast<const ASTIdentifierRefNode *>(Id);
      if (!IdR) {
        std::stringstream M;
        M << "An ASTIdentifierNode reference cannot be an ASTIdentifierNode.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(EX), M.str(),
            DiagLevel::Error);
        return ASTConvMethodBadCast;
      }

      return ResolveConversionMethod(IdR->GetSymbolType(), To);
    }

    return ResolveConversionMethod(Id->GetSymbolType(), To);
  }

  return ResolveConversionMethod(EX->GetASTType(), To);
}

} // namespace QASM
