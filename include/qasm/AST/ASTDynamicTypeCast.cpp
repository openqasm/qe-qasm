/* -*- coding: utf-8 -*-
 *
 * Copyright 2021 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_DYNAMIC_TYPE_CAST_CPP
#error "This Translation Unit must not be used directly."
#error "Use ASTDynamicTypeCast.h instead."
#endif

#include <type_traits>
#include <typeindex>
#include <typeinfo>

namespace QASM {

ASTDynamicTypeCast ASTDynamicTypeCast::DTC;

template <typename __To>
__To *ASTDynamicTypeCast::DynCast(ASTBase *From) {
  if (!From)
    return nullptr;

  ASTType Type = From->GetASTType();

  switch (Type) {
  case ASTTypeUndefined:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeAndAssign:
    return nullptr;
    break;
  case ASTTypeAndOp:
    return nullptr;
    break;
  case ASTTypeAngle:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeArgument:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeAssignment:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeAtomic:
    return nullptr;
    break;
  case ASTTypeAuto:
    return nullptr;
    break;
  case ASTTypeBarrier:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeBinaryOp:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeBlock:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeBool:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeBreak:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeCase:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeCos:
    return nullptr;
    break;
  case ASTTypeChar:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeCNotGate:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeComparison:
    return nullptr;
    break;
  case ASTTypeCommonDeclarator:
    return nullptr;
    break;
  case ASTTypeComplex:
    return nullptr;
    break;
  case ASTTypeConst:
    return nullptr;
    break;
  case ASTTypeContinue:
    return nullptr;
    break;
  case ASTTypeCReg:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeCXGate:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeCCXGate:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeCVRQualifier:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeDeclaration:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeDecOp:
    return nullptr;
    break;
  case ASTTypeDefault:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeDefcal:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeDirty:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeDivAssign:
    return nullptr;
    break;
  case ASTTypeDo:
    return nullptr;
    break;
  case ASTTypeDouble:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeDoWhileLoop:
    return nullptr;
    break;
  case ASTTypeEnd:
    return nullptr;
    break;
  case ASTTypeEnum:
    return nullptr;
    break;
  case ASTTypeEnumConstant:
    return nullptr;
    break;
  case ASTTypeEqOp:
    return nullptr;
    break;
  case ASTTypeExp:
    return nullptr;
    break;
  case ASTTypeExpression:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeExtern:
    return nullptr;
    break;
  case ASTTypeFloat:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeForLoop:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeForStatement:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeFpConstant:
    return nullptr;
    break;
  case ASTTypeFunctionCall:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeFunction:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeGate:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeGateDeclaration:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeGateQualifier:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeGateOpNode:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeGateHOpNode:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeGateQOpNode:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeGateUOpNode:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeGeneric:
    return nullptr;
    break;
  case ASTTypeGeOp:
    return nullptr;
    break;
  case ASTTypeGlobal:
    return nullptr;
    break;
  case ASTTypeGoTo:
    return nullptr;
    break;
  case ASTTypeHadamardGate:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeIdentifier:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeIdentifierList:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeIdentifierRef:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeIdentifierRefList:
    return nullptr;
    break;
  case ASTTypeIfStatement:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeElseIfStatement:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeElseStatement:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeImaginary:
    return nullptr;
    break;
  case ASTTypeIncOp:
    return nullptr;
    break;
  case ASTTypeInline:
    return nullptr;
    break;
  case ASTTypeInt:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeIntegerConstant:
    return nullptr;
    break;
  case ASTTypeKernel:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeLambdaAngle:
  case ASTTypeInverseLambdaAngle:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeLambdaGate:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeLeftAssign:
    return nullptr;
    break;
  case ASTTypeLeftOp:
    return nullptr;
    break;
  case ASTTypeLeOp:
    return nullptr;
    break;
  case ASTTypeLn:
    return nullptr;
    break;
  case ASTTypeLong:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeLongDouble:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeMeasure:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeModAssign:
    return nullptr;
    break;
  case ASTTypeMPDecimal:
    return nullptr;
    break;
  case ASTTypeMPInteger:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeMulAssign:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeNeqOp:
    return nullptr;
    break;
  case ASTTypeNoReturn:
    return nullptr;
    break;
  case ASTTypeOpaque:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeOpaqueDeclaration:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeOpenQASMDecl:
    return nullptr;
    break;
  case ASTTypeOrAssign:
    return nullptr;
    break;
  case ASTTypeOrOp:
    return nullptr;
    break;
  case ASTTypeParameter:
    return nullptr;
    break;
  case ASTTypePhiAngle:
  case ASTTypeInversePhiAngle:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypePhiGate:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypePi:
    return nullptr;
    break;
  case ASTTypePointer:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypePtrOp:
    return nullptr;
    break;
  case ASTTypeQReg:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeQubit:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeRegister:
    return nullptr;
    break;
  case ASTTypeReset:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeRestrict:
    return nullptr;
    break;
  case ASTTypeReturn:
    return nullptr;
    break;
  case ASTTypeRightAssign:
    return nullptr;
    break;
  case ASTTypeRightOp:
    return nullptr;
    break;
  case ASTTypeShort:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeSigned:
    return nullptr;
    break;
  case ASTTypeSignedChar:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeSignedShort:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeSignedInt:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeSignedLong:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeSin:
    return nullptr;
    break;
  case ASTTypeSizeof:
    return nullptr;
    break;
  case ASTTypeSqrt:
    return nullptr;
    break;
  case ASTTypeStatic:
    return nullptr;
    break;
  case ASTTypeStatement:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeStorageQualifier:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeStringLiteral:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeStruct:
    return nullptr;
    break;
  case ASTTypeSubAssign:
    return nullptr;
    break;
  case ASTTypeSwitchStatement:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeTan:
    return nullptr;
    break;
  case ASTTypeThetaAngle:
  case ASTTypeInverseThetaAngle:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeThetaGate:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeThreadLocal:
    return nullptr;
    break;
  case ASTTypeTranslationUnit:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeTypedef:
    return nullptr;
    break;
  case ASTTypeTypedefName:
    return nullptr;
    break;
  case ASTTypeTypeQualifier:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeUGate:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeUnaryOp:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeUnion:
    return nullptr;
    break;
  case ASTTypeUnsigned:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeUnsignedChar:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeUnsignedShort:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeUnsignedInt:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeUnsignedLong:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeVarDecl:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeVariable:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeVoid:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeVolatile:
    return nullptr;
    break;
  case ASTTypeWhileLoop:
    return nullptr;
    break;
  case ASTTypeWhileStatement:
    return dynamic_cast<__To *>(From);
    break;
  case ASTTypeXorAssign:
    return nullptr;
    break;
  default:
    return nullptr;
    break;
  }

  return nullptr;
}

template <typename __To>
const __To *ASTDynamicTypeCast::DynCast(const ASTBase *From) {
  if (!From)
    return nullptr;

  ASTType Type = From->GetASTType();

  switch (Type) {
  case ASTTypeUndefined:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeAndAssign:
    return nullptr;
    break;
  case ASTTypeAndOp:
    return nullptr;
    break;
  case ASTTypeAngle:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeArgument:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeAssignment:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeAtomic:
    return nullptr;
    break;
  case ASTTypeAuto:
    return nullptr;
    break;
  case ASTTypeBarrier:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeBinaryOp:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeBlock:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeBool:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeBreak:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeCase:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeCos:
    return nullptr;
    break;
  case ASTTypeChar:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeCNotGate:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeComparison:
    return nullptr;
    break;
  case ASTTypeCommonDeclarator:
    return nullptr;
    break;
  case ASTTypeComplex:
    return nullptr;
    break;
  case ASTTypeConst:
    return nullptr;
    break;
  case ASTTypeContinue:
    return nullptr;
    break;
  case ASTTypeCReg:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeCXGate:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeCCXGate:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeCVRQualifier:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeDeclaration:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeDecOp:
    return nullptr;
    break;
  case ASTTypeDefault:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeDefcal:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeDirty:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeDivAssign:
    return nullptr;
    break;
  case ASTTypeDo:
    return nullptr;
    break;
  case ASTTypeDouble:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeDoWhileLoop:
    return nullptr;
    break;
  case ASTTypeEnd:
    return nullptr;
    break;
  case ASTTypeEnum:
    return nullptr;
    break;
  case ASTTypeEnumConstant:
    return nullptr;
    break;
  case ASTTypeEqOp:
    return nullptr;
    break;
  case ASTTypeExp:
    return nullptr;
    break;
  case ASTTypeExpression:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeExtern:
    return nullptr;
    break;
  case ASTTypeFloat:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeForLoop:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeForStatement:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeFpConstant:
    return nullptr;
    break;
  case ASTTypeFunctionCall:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeFunction:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeGate:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeGateDeclaration:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeGateQualifier:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeGateOpNode:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeGateHOpNode:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeGateQOpNode:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeGateUOpNode:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeGeneric:
    return nullptr;
    break;
  case ASTTypeGeOp:
    return nullptr;
    break;
  case ASTTypeGlobal:
    return nullptr;
    break;
  case ASTTypeGoTo:
    return nullptr;
    break;
  case ASTTypeHadamardGate:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeIdentifier:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeIdentifierList:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeIdentifierRef:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeIdentifierRefList:
    return nullptr;
    break;
  case ASTTypeIfStatement:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeElseIfStatement:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeElseStatement:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeImaginary:
    return nullptr;
    break;
  case ASTTypeIncOp:
    return nullptr;
    break;
  case ASTTypeInline:
    return nullptr;
    break;
  case ASTTypeInt:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeIntegerConstant:
    return nullptr;
    break;
  case ASTTypeKernel:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeLambdaAngle:
  case ASTTypeInverseLambdaAngle:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeLambdaGate:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeLeftAssign:
    return nullptr;
    break;
  case ASTTypeLeftOp:
    return nullptr;
    break;
  case ASTTypeLeOp:
    return nullptr;
    break;
  case ASTTypeLn:
    return nullptr;
    break;
  case ASTTypeLong:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeLongDouble:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeMeasure:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeModAssign:
    return nullptr;
    break;
  case ASTTypeMPDecimal:
    return nullptr;
    break;
  case ASTTypeMPInteger:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeMulAssign:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeNeqOp:
    return nullptr;
    break;
  case ASTTypeNoReturn:
    return nullptr;
    break;
  case ASTTypeOpaque:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeOpaqueDeclaration:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeOpenQASMDecl:
    return nullptr;
    break;
  case ASTTypeOrAssign:
    return nullptr;
    break;
  case ASTTypeOrOp:
    return nullptr;
    break;
  case ASTTypeParameter:
    return nullptr;
    break;
  case ASTTypePhiAngle:
  case ASTTypeInversePhiAngle:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypePhiGate:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypePi:
    return nullptr;
    break;
  case ASTTypePointer:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypePtrOp:
    return nullptr;
    break;
  case ASTTypeQReg:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeQubit:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeRegister:
    return nullptr;
    break;
  case ASTTypeReset:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeRestrict:
    return nullptr;
    break;
  case ASTTypeReturn:
    return nullptr;
    break;
  case ASTTypeRightAssign:
    return nullptr;
    break;
  case ASTTypeRightOp:
    return nullptr;
    break;
  case ASTTypeShort:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeSigned:
    return nullptr;
    break;
  case ASTTypeSignedChar:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeSignedShort:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeSignedInt:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeSignedLong:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeSin:
    return nullptr;
    break;
  case ASTTypeSizeof:
    return nullptr;
    break;
  case ASTTypeSqrt:
    return nullptr;
    break;
  case ASTTypeStatic:
    return nullptr;
    break;
  case ASTTypeStatement:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeStorageQualifier:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeStringLiteral:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeStruct:
    return nullptr;
    break;
  case ASTTypeSubAssign:
    return nullptr;
    break;
  case ASTTypeSwitchStatement:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeTan:
    return nullptr;
    break;
  case ASTTypeThetaAngle:
  case ASTTypeInverseThetaAngle:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeThetaGate:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeThreadLocal:
    return nullptr;
    break;
  case ASTTypeTranslationUnit:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeTypedef:
    return nullptr;
    break;
  case ASTTypeTypedefName:
    return nullptr;
    break;
  case ASTTypeTypeQualifier:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeUGate:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeUnaryOp:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeUnion:
    return nullptr;
    break;
  case ASTTypeUnsigned:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeUnsignedChar:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeUnsignedShort:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeUnsignedInt:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeUnsignedLong:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeVarDecl:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeVariable:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeVoid:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeVolatile:
    return nullptr;
    break;
  case ASTTypeWhileLoop:
    return nullptr;
    break;
  case ASTTypeWhileStatement:
    return dynamic_cast<const __To *>(From);
    break;
  case ASTTypeXorAssign:
    return nullptr;
    break;
  default:
    return nullptr;
    break;
  }

  return nullptr;
}

template <typename __To>
__To *ASTDynamicTypeCast::DynCast(ASTAnyType &From) {
  if (!From.first.has_value())
    return nullptr;

  switch (From.second) {
  case ASTTypeUndefined:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeAndAssign:
  case ASTTypeAndOp:
    return nullptr;
    break;
  case ASTTypeAngle:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeArgument:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeAssignment:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeAtomic:
  case ASTTypeAuto:
    return nullptr;
    break;
  case ASTTypeBarrier:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeBinaryOp:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeBlock:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeBool:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeBreak:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeCase:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeCos:
    return nullptr;
    break;
  case ASTTypeChar:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeCNotGate:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeComparison:
  case ASTTypeCommonDeclarator:
  case ASTTypeComplex:
  case ASTTypeConst:
  case ASTTypeContinue:
    return nullptr;
    break;
  case ASTTypeCReg:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeCXGate:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeCCXGate:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeCVRQualifier:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeDeclaration:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeDecOp:
    return nullptr;
    break;
  case ASTTypeDefault:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeDefcal:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeDirty:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeDivAssign:
  case ASTTypeDo:
    return nullptr;
    break;
  case ASTTypeDouble:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeDoWhileLoop:
  case ASTTypeEnd:
  case ASTTypeEnum:
  case ASTTypeEnumConstant:
  case ASTTypeEqOp:
  case ASTTypeExp:
    return nullptr;
    break;
  case ASTTypeExpression:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeExtern:
    return nullptr;
    break;
  case ASTTypeFloat:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeForLoop:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeForStatement:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeFpConstant:
    return nullptr;
    break;
  case ASTTypeFunctionCall:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeFunction:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGate:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGateDeclaration:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGateQualifier:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGateOpNode:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGateHOpNode:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGateQOpNode:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGateUOpNode:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGeneric:
  case ASTTypeGeOp:
  case ASTTypeGlobal:
  case ASTTypeGoTo:
    return nullptr;
    break;
  case ASTTypeHadamardGate:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeIdentifier:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeIdentifierList:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeIdentifierRef:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeIdentifierRefList:
    return nullptr;
    break;
  case ASTTypeIfStatement:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeElseIfStatement:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeElseStatement:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeImaginary:
  case ASTTypeIncOp:
  case ASTTypeInline:
    return nullptr;
    break;
  case ASTTypeInt:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeIntegerConstant:
    return nullptr;
    break;
  case ASTTypeKernel:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeLambdaAngle:
  case ASTTypeInverseLambdaAngle:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeLambdaGate:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeLeftAssign:
  case ASTTypeLeftOp:
  case ASTTypeLeOp:
  case ASTTypeLn:
    return nullptr;
    break;
  case ASTTypeLong:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeLongDouble:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeMeasure:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeModAssign:
  case ASTTypeMPDecimal:
    return nullptr;
    break;
  case ASTTypeMPInteger:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeMulAssign:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeNeqOp:
  case ASTTypeNoReturn:
    return nullptr;
    break;
  case ASTTypeOpaque:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeOpaqueDeclaration:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeOpenQASMDecl:
  case ASTTypeOrAssign:
  case ASTTypeOrOp:
  case ASTTypeParameter:
    return nullptr;
    break;
  case ASTTypePhiAngle:
  case ASTTypeInversePhiAngle:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypePhiGate:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypePi:
    return nullptr;
    break;
  case ASTTypePointer:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypePtrOp:
    return nullptr;
    break;
  case ASTTypeQReg:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeQubit:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeRegister:
    return nullptr;
    break;
  case ASTTypeReset:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeRestrict:
  case ASTTypeReturn:
  case ASTTypeRightAssign:
  case ASTTypeRightOp:
    return nullptr;
    break;
  case ASTTypeShort:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeSigned:
    return nullptr;
    break;
  case ASTTypeSignedChar:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeSignedShort:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeSignedInt:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeSignedLong:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeSin:
  case ASTTypeSizeof:
  case ASTTypeSqrt:
  case ASTTypeStatic:
    return nullptr;
    break;
  case ASTTypeStatement:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeStorageQualifier:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeStringLiteral:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeStruct:
  case ASTTypeSubAssign:
    return nullptr;
    break;
  case ASTTypeSwitchStatement:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeTan:
    return nullptr;
    break;
  case ASTTypeThetaAngle:
  case ASTTypeInverseThetaAngle:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeThetaGate:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeThreadLocal:
    return nullptr;
    break;
  case ASTTypeTranslationUnit:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeTypedef:
  case ASTTypeTypedefName:
    return nullptr;
    break;
  case ASTTypeTypeQualifier:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUGate:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUnaryOp:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUnion:
    return nullptr;
    break;
  case ASTTypeUnsigned:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUnsignedChar:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUnsignedShort:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUnsignedInt:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUnsignedLong:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeVarDecl:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeVariable:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeVoid:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeVolatile:
  case ASTTypeWhileLoop:
    return nullptr;
    break;
  case ASTTypeWhileStatement:
    try {
      return std::any_cast<__To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeXorAssign:
    return nullptr;
    break;
  default:
    return nullptr;
    break;
  }

  return nullptr;
}

template <typename __To>
const __To *ASTDynamicTypeCast::DynCast(const ASTAnyType &From) {
  if (!From.first.has_value())
    return nullptr;

  switch (From.second) {
  case ASTTypeUndefined:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeAndAssign:
  case ASTTypeAndOp:
    return nullptr;
    break;
  case ASTTypeAngle:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeArgument:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeAssignment:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeAtomic:
  case ASTTypeAuto:
    return nullptr;
    break;
  case ASTTypeBarrier:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeBinaryOp:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeBlock:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeBool:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeBreak:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeCase:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeCos:
    return nullptr;
    break;
  case ASTTypeChar:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeCNotGate:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeComparison:
  case ASTTypeCommonDeclarator:
  case ASTTypeComplex:
  case ASTTypeConst:
  case ASTTypeContinue:
    return nullptr;
    break;
  case ASTTypeCReg:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeCXGate:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeCCXGate:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeCVRQualifier:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeDeclaration:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeDecOp:
    return nullptr;
    break;
  case ASTTypeDefault:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeDefcal:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeDirty:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeDivAssign:
  case ASTTypeDo:
    return nullptr;
    break;
  case ASTTypeDouble:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeDoWhileLoop:
  case ASTTypeEnd:
  case ASTTypeEnum:
  case ASTTypeEnumConstant:
  case ASTTypeEqOp:
  case ASTTypeExp:
    return nullptr;
    break;
  case ASTTypeExpression:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeExtern:
    return nullptr;
    break;
  case ASTTypeFloat:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeForLoop:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeForStatement:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeFpConstant:
    return nullptr;
    break;
  case ASTTypeFunctionCall:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeFunction:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGate:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGateDeclaration:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGateQualifier:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGateOpNode:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGateHOpNode:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGateQOpNode:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGateUOpNode:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeGeneric:
  case ASTTypeGeOp:
  case ASTTypeGlobal:
  case ASTTypeGoTo:
    return nullptr;
    break;
  case ASTTypeHadamardGate:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeIdentifier:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeIdentifierList:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeIdentifierRef:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeIdentifierRefList:
    return nullptr;
    break;
  case ASTTypeIfStatement:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeElseIfStatement:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeElseStatement:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeImaginary:
  case ASTTypeIncOp:
  case ASTTypeInline:
    return nullptr;
    break;
  case ASTTypeInt:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeIntegerConstant:
    return nullptr;
    break;
  case ASTTypeKernel:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeLambdaAngle:
  case ASTTypeInverseLambdaAngle:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeLambdaGate:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeLeftAssign:
  case ASTTypeLeftOp:
  case ASTTypeLeOp:
  case ASTTypeLn:
    return nullptr;
    break;
  case ASTTypeLong:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeLongDouble:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeMeasure:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeModAssign:
  case ASTTypeMPDecimal:
    return nullptr;
    break;
  case ASTTypeMPInteger:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeMulAssign:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeNeqOp:
  case ASTTypeNoReturn:
    return nullptr;
    break;
  case ASTTypeOpaque:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeOpaqueDeclaration:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeOpenQASMDecl:
  case ASTTypeOrAssign:
  case ASTTypeOrOp:
  case ASTTypeParameter:
    return nullptr;
    break;
  case ASTTypePhiAngle:
  case ASTTypeInversePhiAngle:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypePhiGate:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypePi:
    return nullptr;
    break;
  case ASTTypePointer:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypePtrOp:
    return nullptr;
    break;
  case ASTTypeQReg:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeQubit:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeRegister:
    return nullptr;
    break;
  case ASTTypeReset:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeRestrict:
  case ASTTypeReturn:
  case ASTTypeRightAssign:
  case ASTTypeRightOp:
  case ASTTypeShort:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeSigned:
    return nullptr;
    break;
  case ASTTypeSignedChar:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeSignedShort:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeSignedInt:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeSignedLong:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeSin:
  case ASTTypeSizeof:
  case ASTTypeSqrt:
  case ASTTypeStatic:
    return nullptr;
    break;
  case ASTTypeStatement:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeStorageQualifier:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeStringLiteral:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeStruct:
  case ASTTypeSubAssign:
    return nullptr;
    break;
  case ASTTypeSwitchStatement:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeTan:
    return nullptr;
    break;
  case ASTTypeThetaAngle:
  case ASTTypeInverseThetaAngle:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeThetaGate:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeThreadLocal:
    return nullptr;
    break;
  case ASTTypeTranslationUnit:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeTypedef:
  case ASTTypeTypedefName:
    return nullptr;
    break;
  case ASTTypeTypeQualifier:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUGate:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUnaryOp:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUnion:
    return nullptr;
    break;
  case ASTTypeUnsigned:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUnsignedChar:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUnsignedShort:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUnsignedInt:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeUnsignedLong:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeVarDecl:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeVariable:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeVoid:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeVolatile:
  case ASTTypeWhileLoop:
    return nullptr;
    break;
  case ASTTypeWhileStatement:
    try {
      return std::any_cast<const __To *>(From.first);
    } catch (const std::bad_any_cast &E) {
      return nullptr;
    }
    break;
  case ASTTypeXorAssign:
    return nullptr;
    break;
  default:
    return nullptr;
    break;
  }

  return nullptr;
}

} // namespace QASM
