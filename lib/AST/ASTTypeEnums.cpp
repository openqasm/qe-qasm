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

#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTLength.h>
#include "QasmParser.tab.h"

namespace QASM {

const char* PrintTypeEnum(ASTType Type) {
  switch (Type) {
    case ASTTypeUndefined:
      return "ASTTypeUndefined";
      break;
    case ASTTypeAddAssign:
      return "ASTTypeAddAssign";
      break;
    case ASTTypeAlignas:
      return "ASTTypeAlignas";
      break;
    case ASTTypeAlignof:
      return "ASTTypeAlignof";
      break;
    case ASTTypeAndAssign:
      return "ASTTypeAndAssign";
      break;
    case ASTTypeAndOp:
      return "ASTTypeAndOp";
      break;
    case ASTTypeAngle:
      return "ASTTypeAngle";
      break;
    case ASTTypeAngleArray:
      return "ASTTypeAngleArray";
      break;
    case ASTTypeAngleList:
      return "ASTTypeAngleList";
      break;
    case ASTTypeAngleMap:
      return "ASTTypeAngleMap";
      break;
    case ASTTypeAnnotation:
      return "ASTTypeAnnotation";
      break;
    case ASTTypeAnnotationStmt:
      return "ASTTypeAnnotationStmt";
      break;
    case ASTTypeArcCos:
      return "ASTTypeArcCos";
      break;
    case ASTTypeArcSin:
      return "ASTTypeArcSin";
      break;
    case ASTTypeArcTan:
      return "ASTTypeArcTan";
      break;
    case ASTTypeArgument:
      return "ASTTypeArgument";
      break;
    case ASTTypeArray:
      return "ASTTypeArray";
      break;
    case ASTTypeArraySubscript:
      return "ASTTypeArraySubscript";
      break;
    case ASTTypeArraySubscriptList:
      return "ASTTypeArraySubscriptList";
      break;
    case ASTTypeAssignment:
      return "ASTTypeAssignment";
      break;
    case ASTTypeAtomic:
      return "ASTTypeAtomic";
      break;
    case ASTTypeAuto:
      return "ASTTypeAuto";
      break;
    case ASTTypeBarrier:
      return "ASTTypeBarrier";
      break;
    case ASTTypeBinaryOp:
      return "ASTTypeBinaryOp";
      break;
    case ASTTypeBinaryOpStatement:
      return "ASTTypeBinaryOpStatement";
      break;
    case ASTTypeBitset:
      return "ASTTypeBitset";
      break;
    case ASTTypeBlock:
      return "ASTTypeBlock";
      break;
    case ASTTypeBool:
      return "ASTTypeBool";
      break;
    case ASTTypeBoolArray:
      return "ASTTypeBoolArray";
      break;
    case ASTTypeBoundQubit:
      return "ASTTypeBoundQubit";
      break;
    case ASTTypeBoundQubitList:
      return "ASTTypeBoundQubitList";
      break;
    case ASTTypeBox:
      return "ASTTypeBox";
      break;
    case ASTTypeBoxAs:
      return "ASTTypeBoxAs";
      break;
    case ASTTypeBoxTo:
      return "ASTTypeBoxTo";
      break;
    case ASTTypeBoxStatement:
      return "ASTTypeBoxStatement";
      break;
    case ASTTypeBracedInitializerList:
      return "ASTTypeBracedInitializerList";
      break;
    case ASTTypeBreak:
      return "ASTTypeBreak";
      break;
    case ASTTypeCal:
      return "ASTTypeCal";
      break;
    case ASTTypeCaseStatement:
      return "ASTTypeCaseStatement";
      break;
    case ASTTypeCast:
      return "ASTTypeCast";
      break;
    case ASTTypeBadCast:
      return "ASTTypeBadCast";
      break;
    case ASTTypeCBitArray:
      return "ASTTypeCBitArray";
      break;
    case ASTTypeCBitNArray:
      return "ASTTypeCBitNArray";
      break;
    case ASTTypeCBitNodeMap:
      return "ASTTypeCBitNodeMap";
      break;
    case ASTTypeCos:
      return "ASTTypeCos";
      break;
    case ASTTypeChar:
      return "ASTTypeChar";
      break;
    case ASTTypeCNotGate:
      return "ASTTypeCNotGate";
      break;
    case ASTTypeCNotGateOpNode:
      return "ASTTypeCNotGateOpNode";
      break;
    case ASTTypeComparison:
      return "ASTTypeComparison";
      break;
    case ASTTypeCommonDeclarator:
      return "ASTTypeCommonDeclarator";
      break;
    case ASTTypeComplexExpression:
      return "ASTTypeComplexExpression";
      break;
    case ASTTypeConst:
      return "ASTTypeConst";
      break;
    case ASTTypeControlExpression:
      return "ASTTypeControlExpression";
      break;
    case ASTTypeContinue:
      return "ASTTypeContinue";
      break;
    case ASTTypeCReg:
      return "ASTTypeCReg";
      break;
    case ASTTypeCXGate:
      return "ASTTypeCXGate";
      break;
    case ASTTypeCXGateOpNode:
      return "ASTTypeCXGateOpNode";
      break;
    case ASTTypeCCXGate:
      return "ASTTypeCCXGate";
      break;
    case ASTTypeCCXGateOpNode:
      return "ASTTypeCCXGateOpNode";
      break;
    case ASTTypeComplexList:
      return "ASTTypeComplexList";
      break;
    case ASTTypeCVRQualifier:
      return "ASTTypeCVRQualifier";
      break;
    case ASTTypeDeclaration:
      return "ASTTypeDeclaration";
      break;
    case ASTTypeDeclarationError:
      return "ASTTypeDeclarationError";
      break;
    case ASTTypeDeclarationContext:
      return "ASTTypeDeclarationContext";
      break;
    case ASTTypeDeclarationList:
      return "ASTTypeDeclarationList";
      break;
    case ASTTypeDecOp:
      return "ASTTypeDecOp";
      break;
    case ASTTypeDefaultStatement:
      return "ASTTypeDefaultStatement";
      break;
    case ASTTypeDefcal:
      return "ASTTypeDefcal";
      break;
    case ASTTypeDefcalMeasure:
      return "ASTTypeDefcalMeasure";
      break;
    case ASTTypeDefcalReset:
      return "ASTTypeDefcalReset";
      break;
    case ASTTypeDefcalArg:
      return "ASTTypeDefcalArg";
      break;
    case ASTTypeDefcalCall:
      return "ASTTypeDefcalCall";
      break;
    case ASTTypeDefcalDelay:
      return "ASTTypeDefcalDelay";
      break;
    case ASTTypeDefcalDelayCall:
      return "ASTTypeDefcalDelayCall";
      break;
    case ASTTypeDefcalGroup:
      return "ASTTypeDefcalGroup";
      break;
    case ASTTypeDefcalGroupOpNode:
      return "ASTTypeDefcalGroupOpNode";
      break;
    case ASTTypeDefcalMeasureCall:
      return "ASTTypeDefcalMeasureCall";
      break;
    case ASTTypeDefcalResetCall:
      return "ASTTypeDefcalResetCall";
      break;
    case ASTTypeDefcalCallExpression:
      return "ASTTypeDefcalCallExpression";
      break;
    case ASTTypeDefcalCallStatement:
      return "ASTTypeDefcalCallStatement";
      break;
    case ASTTypeDefcalDeclaration:
      return "ASTTypeDefcalDeclaration";
      break;
    case ASTTypeDefcalGrammar:
      return "ASTTypeDefcalGrammar";
      break;
    case ASTTypeDefcalParam:
      return "ASTTypeDefcalParam";
      break;
    case ASTTypeDelay:
      return "ASTTypeDelay";
      break;
    case ASTTypeDelayStatement:
      return "ASTTypeDelayStatement";
      break;
    case ASTTypeDirectiveStatement:
      return "ASTTypeDirectiveStatement";
      break;
    case ASTTypeDirty:
      return "ASTTypeDirty";
      break;
    case ASTTypeDivAssign:
      return "ASTTypeDivAssign";
      break;
    case ASTTypeDoWhileStatement:
      return "ASTTypeDoWhileStatement";
      break;
    case ASTTypeDouble:
      return "ASTTypeDouble";
      break;
    case ASTTypeDoWhileLoop:
      return "ASTTypeDoWhileLoop";
      break;
    case ASTTypeDuration:
      return "ASTTypeDuration";
      break;
    case ASTTypeDurationArray:
      return "ASTTypeDurationArray";
      break;
    case ASTTypeDurationOf:
      return "ASTTypeDurationOf";
      break;
    case ASTTypeEllipsis:
      return "ASTTypeEllipsis";
      break;
    case ASTTypeElse:
      return "ASTTypeElse";
      break;
    case ASTTypeEnd:
      return "ASTTypeEnd";
      break;
    case ASTTypeEnum:
      return "ASTTypeEnum";
      break;
    case ASTTypeEnumConstant:
      return "ASTTypeEnumConstant";
      break;
    case ASTTypeEqOp:
      return "ASTTypeEqOp";
      break;
    case ASTTypeEulerAngle:
      return "ASTTypeEulerAngle";
      break;
    case ASTTypeExp:
      return "ASTTypeExp";
      break;
    case ASTTypeExpression:
      return "ASTTypeExpression";
      break;
    case ASTTypeExpressionError:
      return "ASTTypeExpressionError";
      break;
    case ASTTypeExpressionList:
      return "ASTTypeExpressionList";
      break;
    case ASTTypeExpressionNodeList:
      return "ASTTypeExpressionNodeList";
      break;
    case ASTTypeExtern:
      return "ASTTypeExtern";
      break;
    case ASTTypeFloat:
      return "ASTTypeFloat";
      break;
    case ASTTypeFloatArray:
      return "ASTTypeFloatArray";
      break;
    case ASTTypeForLoop:
      return "ASTTypeForLoop";
      break;
    case ASTTypeForLoopRange:
      return "ASTTypeForLoopRange";
      break;
    case ASTTypeForStatement:
      return "ASTTypeForStatement";
      break;
    case ASTTypeFpConstant:
      return "ASTTypeFpConstant";
      break;
    case ASTTypeFuncArg:
      return "ASTTypeFuncArg";
      break;
    case ASTTypeFuncParam:
      return "ASTTypeFuncParam";
      break;
    case ASTTypeFunction:
      return "ASTTypeFunction";
      break;
    case ASTTypeFunctionCall:
      return "ASTTypeFunctionCall";
      break;
    case ASTTypeFunctionCallArgument:
      return "ASTTypeFunctionCallArgument";
      break;
    case ASTTypeFunctionCallArgumentList:
      return "ASTTypeFunctionCallArgumentList";
      break;
    case ASTTypeFunctionDeclaration:
      return "ASTTypeFunctionDeclaration";
      break;
    case ASTTypeFunctionCallExpression:
      return "ASTTypeFunctionCallExpression";
      break;
    case ASTTypeFunctionCallStatement:
      return "ASTTypeFunctionCallStatement";
      break;
    case ASTTypeFunctionGroup:
      return "ASTTypeFunctionGroup";
      break;
    case ASTTypeGate:
      return "ASTTypeGate";
      break;
    case ASTTypeGateAngleParam:
      return "ASTTypeGateAngleParam";
      break;
    case ASTTypeGateArg:
      return "ASTTypeGateArg";
      break;
    case ASTTypeGateCall:
      return "ASTTypeGateCall";
      break;
    case ASTTypeGateControl:
      return "ASTTypeGateControl";
      break;
    case ASTTypeGateControlStatement:
      return "ASTTypeGateControlStatement";
      break;
    case ASTTypeGateNegControl:
      return "ASTTypeGateNegControl";
      break;
    case ASTTypeGateNegControlStatement:
      return "ASTTypeGateNegControlStatement";
      break;
    case ASTTypeGateDeclaration:
      return "ASTTypeGateDeclaration";
      break;
    case ASTTypeGateGPhaseExpression:
      return "ASTTypeGateGPhaseExpression";
      break;
    case ASTTypeGateGPhaseStatement:
      return "ASTTypeGateGPhaseStatement";
      break;
    case ASTTypeGateInverse:
      return "ASTTypeGateInverse";
      break;
    case ASTTypeGateInverseStatement:
      return "ASTTypeGateInverseStatement";
      break;
    case ASTTypeGateParam:
      return "ASTTypeGateParam";
      break;
    case ASTTypeGatePower:
      return "ASTTypeGatePower";
      break;
    case ASTTypeGatePowerStatement:
      return "ASTTypeGatePowerStatement";
      break;
    case ASTTypeGateQubitParam:
      return "ASTTypeGateQubitParam";
      break;
    case ASTTypeGateQualifier:
      return "ASTTypeGateQualifier";
      break;
    case ASTTypeGateOpNode:
      return "ASTTypeGateOpNode";
      break;
    case ASTTypeGateOpList:
      return "ASTTypeGateOpList";
      break;
    case ASTTypeGateGenericOpNode:
      return "ASTTypeGateGenericOpNode";
      break;
    case ASTTypeGateHOpNode:
      return "ASTTypeGateHOpNode";
      break;
    case ASTTypeGateHOpList:
      return "ASTTypeGateHOpList";
      break;
    case ASTTypeGateQOpList:
      return "ASTTypeGateQOpList";
      break;
    case ASTTypeGateQOpNode:
      return "ASTTypeGateQOpNode";
      break;
    case ASTTypeGateUOpNode:
      return "ASTTypeGateUOpNode";
      break;
    case ASTTypeGateUOpList:
      return "ASTTypeGateUOpList";
      break;
    case ASTTypeGeneric:
      return "ASTTypeGeneric";
      break;
    case ASTTypeGeOp:
      return "ASTTypeGeOp";
      break;
    case ASTTypeGlobal:
      return "ASTTypeGlobal";
      break;
    case ASTTypeGoTo:
      return "ASTTypeGoTo";
      break;
    case ASTTypeGPhaseExpression:
      return "ASTTypeGPhaseExpression";
      break;
    case ASTTypeGPhaseStatement:
      return "ASTTypeGPhaseStatement";
      break;
    case ASTTypeHadamardGate:
      return "ASTTypeHadamardGate";
      break;
    case ASTTypeIdentifier:
      return "ASTTypeIdentifier";
      break;
    case ASTTypeIdentifierList:
      return "ASTTypeIdentifierList";
      break;
    case ASTTypeIdentifierRef:
      return "ASTTypeIdentifierRef";
      break;
    case ASTTypeIdentifierRefList:
      return "ASTTypeIdentifierRefList";
      break;
    case ASTTypeIfElse:
      return "ASTTypeIfElse";
      break;
    case ASTTypeIfStatement:
      return "ASTTypeIfStatement";
      break;
    case ASTTypeIfStatementList:
      return "ASTTypeIfStatementList";
      break;
    case ASTTypeIfStatementTracker:
      return "ASTTypeIfStatementTracker";
      break;
    case ASTTypeInvalidArray:
      return "ASTTypeInvalidArray";
      break;
    case ASTTypeElseIfStatement:
      return "ASTTypeElseIfStatement";
      break;
    case ASTTypeElseStatement:
      return "ASTTypeElseStatement";
      break;
    case ASTTypeImplicitConversion:
      return "ASTTypeImplicitConversion";
      break;
    case ASTTypeBadImplicitConversion:
      return "ASTTypeBadImplicitConversion";
      break;
    case ASTTypeHash:
      return "ASTTypeHash";
      break;
    case ASTTypeImaginary:
      return "ASTTypeImaginary";
      break;
    case ASTTypeIncOp:
      return "ASTTypeIncOp";
      break;
    case ASTTypeInitializerList:
      return "ASTTypeInitializerList";
      break;
    case ASTTypeInitializerNodeList:
      return "ASTTypeInitializerNodeList";
      break;
    case ASTTypeInitListElement:
      return "ASTTypeInitListElement";
      break;
    case ASTTypeInline:
      return "ASTTypeInline";
      break;
    case ASTTypeInputModifier:
      return "ASTTypeInputModifier";
      break;
    case ASTTypeInt:
      return "ASTTypeInt";
      break;
    case ASTTypeUInt:
      return "ASTTypeUInt";
      break;
    case ASTTypeIntArray:
      return "ASTTypeIntArray";
      break;
    case ASTTypeUIntArray:
      return "ASTTypeUIntArray";
      break;
    case ASTTypeIntegerConstant:
      return "ASTTypeIntegerConstant";
      break;
    case ASTTypeIntegerSequence:
      return "ASTTypeIntegerSequence";
      break;
    case ASTTypeInvalidCast:
      return "ASTTypeInvalidCast";
      break;
    case ASTTypeInverseExpression:
      return "ASTTypeInverseExpression";
      break;
    case ASTTypeKernel:
      return "ASTTypeKernel";
      break;
    case ASTTypeKernelArg:
      return "ASTTypeKernelArg";
      break;
    case ASTTypeKernelCall:
      return "ASTTypeKernelCall";
      break;
    case ASTTypeKernelCallExpression:
      return "ASTTypeKernelCallExpression";
      break;
    case ASTTypeKernelCallStatement:
      return "ASTTypeKernelCallStatement";
      break;
    case ASTTypeKernelGroup:
      return "ASTTypeKernelGroup";
      break;
    case ASTTypeKernelParam:
      return "ASTTypeKernelParam";
      break;
    case ASTTypeLambdaAngle:
      return "ASTTypeLambdaAngle";
      break;
    case ASTTypeLeftAssign:
      return "ASTTypeLeftAssign";
      break;
    case ASTTypeLeftOp:
      return "ASTTypeLeftOp";
      break;
    case ASTTypeLength:
      return "ASTTypeLength";
      break;
    case ASTTypeLengthArray:
      return "ASTTypeLengthArray";
      break;
    case ASTTypeLengthOf:
      return "ASTTypeLengthOf";
      break;
    case ASTTypeLeOp:
      return "ASTTypeLeOp";
      break;
    case ASTTypeLn:
      return "ASTTypeLn";
      break;
    case ASTTypeLong:
      return "ASTTypeLong";
      break;
    case ASTTypeLongDouble:
      return "ASTTypeLongDouble";
      break;
    case ASTTypeLet:
      return "ASTTypeLet";
      break;
    case ASTTypeLValueReference:
      return "ASTTypeLValueReference";
      break;
    case ASTTypeMeasure:
      return "ASTTypeMeasure";
      break;
    case ASTTypeModAssign:
      return "ASTTypeModAssign";
      break;
    case ASTTypeMPComplex:
      return "ASTTypeMPComplex";
      break;
    case ASTTypeMPComplexArray:
      return "ASTTypeMPComplexArray";
      break;
    case ASTTypeMPComplexRep:
      return "ASTTypeMPComplexRep";
      break;
    case ASTTypeMPCReal:
      return "ASTTypeMPCReal";
      break;
    case ASTTypeMPCImag:
      return "ASTTypeMPCImag";
      break;
    case ASTTypeMPDecimal:
      return "ASTTypeMPDecimal";
      break;
    case ASTTypeMPDecimalArray:
      return "ASTTypeMPDecimalArray";
      break;
    case ASTTypeMPInteger:
      return "ASTTypeMPInteger";
      break;
    case ASTTypeMPUInteger:
      return "ASTTypeMPUInteger";
      break;
    case ASTTypeMPIntegerArray:
      return "ASTTypeMPIntegerArray";
      break;
    case ASTTypeMPUIntegerArray:
      return "ASTTypeMPUIntegerArray";
      break;
    case ASTTypeMulAssign:
      return "ASTTypeMulAssign";
      break;
    case ASTTypeNegOp:
      return "ASTTypeNegOp";
      break;
    case ASTTypeNeqOp:
      return "ASTTypeNeqOp";
      break;
    case ASTTypeNoReturn:
      return "ASTTypeNoReturn";
      break;
    case ASTTypeOpaque:
      return "ASTTypeOpaque";
      break;
    case ASTTypeOpaqueDeclaration:
      return "ASTTypeOpaqueDeclaration";
      break;
    case ASTTypeOpenQASMDecl:
      return "ASTTypeOpenQASMDecl";
      break;
    case ASTTypeOpenQASMExpression:
      return "ASTTypeOpenQASMExpression";
      break;
    case ASTTypeOpenQASMStatement:
      return "ASTTypeOpenQASMStatement";
      break;
    case ASTTypeOpenPulseCalibration:
      return "ASTTypeOpenPulseCalibration";
      break;
    case ASTTypeOpenPulseCalibrationStmt:
      return "ASTTypeOpenPulseCalibrationStmt";
      break;
    case ASTTypeOpenPulseFrame:
      return "ASTTypeOpenPulseFrame";
      break;
    case ASTTypeOpenPulseFrameArray:
      return "ASTTypeOpenPulseFrameArray";
      break;
    case ASTTypeOpenPulsePlay:
      return "ASTTypeOpenPulsePlay";
      break;
    case ASTTypeOpenPulsePlayStatement:
      return "ASTTypeOpenPulsePlayStatement";
      break;
    case ASTTypeOpenPulsePort:
      return "ASTTypeOpenPulsePort";
      break;
    case ASTTypeOpenPulsePortArray:
      return "ASTTypeOpenPulsePortArray";
      break;
    case ASTTypeOpenPulseWaveform:
      return "ASTTypeOpenPulseWaveform";
      break;
    case ASTTypeOpenPulseWaveformArray:
      return "ASTTypeOpenPulseWaveformArray";
      break;
    case ASTTypeOpTy:
      return "ASTTypeOpTy";
      break;
    case ASTTypeOpndTy:
      return "ASTTypeOpndTy";
      break;
    case ASTTypeOrAssign:
      return "ASTTypeOrAssign";
      break;
    case ASTTypeOrOp:
      return "ASTTypeOrOp";
      break;
    case ASTTypeOutputModifier:
      return "ASTTypeOutputModifier";
      break;
    case ASTTypeParameter:
      return "ASTTypeParameter";
      break;
    case ASTTypeParameterList:
      return "ASTTypeParameterList";
      break;
    case ASTTypePhiAngle:
      return "ASTTypePhiAngle";
      break;
    case ASTTypePi:
      return "ASTTypePi";
      break;
    case ASTTypePiAngle:
      return "ASTTypePiAngle";
      break;
    case ASTTypePointer:
      return "ASTTypePointer";
      break;
    case ASTTypePow:
      return "ASTTypePow";
      break;
    case ASTTypePopcount:
      return "ASTTypePopcount";
      break;
    case ASTTypePopcountExpr:
      return "ASTTypePopcountExpr";
      break;
    case ASTTypePopcountStmt:
      return "ASTTypePopcountStmt";
      break;
    case ASTTypePragma:
      return "ASTTypePragma";
      break;
    case ASTTypePragmaStmt:
      return "ASTTypePragmaStmt";
      break;
    case ASTTypePtrOp:
      return "ASTTypePtrOp";
      break;
    case ASTTypeQReg:
      return "ASTTypeQReg";
      break;
    case ASTTypeQubit:
      return "ASTTypeQubit";
      break;
    case ASTTypeQubitArray:
      return "ASTTypeQubitArray";
      break;
    case ASTTypeQubitNArray:
      return "ASTTypeQubitNArray";
      break;
    case ASTTypeQubitContainer:
      return "ASTTypeQubitContainer";
      break;
    case ASTTypeQubitContainerAlias:
      return "ASTTypeQubitContainerAlias";
      break;
    case ASTTypeQubitMap:
      return "ASTTypeQubitMap";
      break;
    case ASTTypeQubits:
      return "ASTTypeQubits";
      break;
    case ASTTypeRegister:
      return "ASTTypeRegister";
      break;
    case ASTTypeReset:
      return "ASTTypeReset";
      break;
    case ASTTypeResult:
      return "ASTTypeResult";
      break;
    case ASTTypeRestrict:
      return "ASTTypeRestrict";
      break;
    case ASTTypeReturn:
      return "ASTTypeReturn";
      break;
    case ASTTypeRightAssign:
      return "ASTTypeRightAssign";
      break;
    case ASTTypeRightOp:
      return "ASTTypeRightOp";
      break;
    case ASTTypeRotl:
      return "ASTTypeRotl";
      break;
    case ASTTypeRotr:
      return "ASTTypeRotr";
      break;
    case ASTTypeRotateExpr:
      return "ASTTypeRotateExpr";
      break;
    case ASTTypeRotateStatement:
      return "ASTTypeRotateStatement";
      break;
    case ASTTypeRValueReference:
      return "ASTTypeRValueReference";
      break;
    case ASTTypeShort:
      return "ASTTypeShort";
      break;
    case ASTTypeSigned:
      return "ASTTypeSigned";
      break;
    case ASTTypeSignedChar:
      return "ASTTypeSignedChar";
      break;
    case ASTTypeSignedShort:
      return "ASTTypeSignedShort";
      break;
    case ASTTypeSignedInt:
      return "ASTTypeSignedInt";
      break;
    case ASTTypeSignedLong:
      return "ASTTypeSignedLong";
      break;
    case ASTTypeSin:
      return "ASTTypeSin";
      break;
    case ASTTypeSizeof:
      return "ASTTypeSizeof";
      break;
    case ASTTypeSqrt:
      return "ASTTypeSqrt";
      break;
    case ASTTypeStatic:
      return "ASTTypeStatic";
      break;
    case ASTTypeStaticAssert:
      return "ASTTypeStaticAssert";
      break;
    case ASTTypeStatement:
      return "ASTTypeStatement";
      break;
    case ASTTypeStatementError:
      return "ASTTypeStatementError";
      break;
    case ASTTypeStatementList:
      return "ASTTypeStatementList";
      break;
    case ASTTypeStorageQualifier:
      return "ASTTypeStorageQualifier";
      break;
    case ASTTypeStretch:
      return "ASTTypeStretch";
      break;
    case ASTTypeStretchStatement:
      return "ASTTypeStretchStatement";
      break;
    case ASTTypeStringLiteral:
      return "ASTTypeStringLiteral";
      break;
    case ASTTypeStringList:
      return "ASTTypeStringList";
      break;
    case ASTTypeStruct:
      return "ASTTypeStruct";
      break;
    case ASTTypeSubAssign:
      return "ASTTypeSubAssign";
      break;
    case ASTTypeSwitchStatement:
      return "ASTTypeSwitchStatement";
      break;
    case ASTTypeSyntaxError:
      return "ASTTypeSyntaxError";
      break;
    case ASTTypeTan:
      return "ASTTypeTan";
      break;
    case ASTTypeTauAngle:
      return "ASTTypeTauAngle";
      break;
    case ASTTypeThetaAngle:
      return "ASTTypeThetaAngle";
      break;
    case ASTTypeThreadLocal:
      return "ASTTypeThreadLocal";
      break;
    case ASTTypeTimeUnit:
      return "ASTTypeTimeUnit";
      break;
    case ASTTypeTranslationUnit:
      return "ASTTypeTranslationUnit";
      break;
    case ASTTypeTypedef:
      return "ASTTypeTypedef";
      break;
    case ASTTypeTypedefName:
      return "ASTTypeTypedefName";
      break;
    case ASTTypeTypeExpression:
      return "ASTTypeTypeExpression";
      break;
    case ASTTypeTypeQualifier:
      return "ASTTypeTypeQualifier";
      break;
    case ASTTypeUGate:
      return "ASTTypeUGate";
      break;
    case ASTTypeUnaryOp:
      return "ASTTypeUnaryOp";
      break;
    case ASTTypeUnaryOpStatement:
      return "ASTTypeUnaryOpStatement";
      break;
    case ASTTypeUnboundQubit:
      return "ASTTypeUnboundQubit";
      break;
    case ASTTypeUnboundQubitList:
      return "ASTTypeUnboundQubitList";
      break;
    case ASTTypeUnion:
      return "ASTTypeUnion";
      break;
    case ASTTypeUnsigned:
      return "ASTTypeUnsigned";
      break;
    case ASTTypeUnsignedChar:
      return "ASTTypeUnsignedChar";
      break;
    case ASTTypeUnsignedShort:
      return "ASTTypeUnsignedShort";
      break;
    case ASTTypeUnsignedInt:
      return "ASTTypeUnsignedInt";
      break;
    case ASTTypeUnsignedLong:
      return "ASTTypeUnsignedLong";
      break;
    case ASTTypeULong:
      return "ASTTypeULong";
      break;
    case ASTTypeUTF8:
      return "ASTTypeUTF8";
      break;
    case ASTTypeValue:
      return "ASTTypeValue";
      break;
    case ASTTypeVarDecl:
      return "ASTTypeVarDecl";
      break;
    case ASTTypeVariable:
      return "ASTTypeVariable";
      break;
    case ASTTypeVoid:
      return "ASTTypeVoid";
      break;
    case ASTTypeVolatile:
      return "ASTTypeVolatile";
      break;
    case ASTTypeWhileLoop:
      return "ASTTypeWhileLoop";
      break;
    case ASTTypeWhileStatement:
      return "ASTTypeWhileStatement";
      break;
    case ASTTypeXorAssign:
      return "ASTTypeXorAssign";
      break;
    default:
      return "Unknown";
      break;
  }

  return "Unknown";
}

const char* PrintOpTypeOperator(ASTOpType OpTy, const std::string& Op) {
  static std::string OPS;

  switch (OpTy) {
  case ASTOpTypeUndefined:
    return "Undefined";
    break;
  case ASTOpTypeAdd:
    return "+";
    break;
  case ASTOpTypeAddAssign:
    return "+=";
    break;
  case ASTOpTypeArraySubscript:
    return "<array-subscript>";
    break;
  case ASTOpTypeCast:
    return "<cast>";
    break;
  case ASTOpTypeImplicitConversion:
    return "<implicit-conversion>";
    break;
  case ASTOpTypeSub:
    return "-";
    break;
  case ASTOpTypeSubAssign:
    return "-=";
    break;
  case ASTOpTypeMul:
    return "*";
    break;
  case ASTOpTypeMulAssign:
    return "*=";
    break;
  case ASTOpTypeDiv:
    return "/";
    break;
  case ASTOpTypeDivAssign:
    return "/=";
    break;
  case ASTOpTypeMod:
    return "%";
    break;
  case ASTOpTypeModAssign:
    return "%=";
    break;
  case ASTOpTypeLogicalAnd:
    return "&&";
    break;
  case ASTOpTypeLogicalOr:
    return "||";
    break;
  case ASTOpTypeBitAnd:
    return "&";
    break;
  case ASTOpTypeBitAndAssign:
    return "&=";
    break;
  case ASTOpTypeBitOr:
    return "|";
    break;
  case ASTOpTypeBitOrAssign:
    return "|=";
    break;
  case ASTOpTypeXor:
    return "^";
    break;
  case ASTOpTypeXorAssign:
    return "^=";
    break;
  case ASTOpTypeAssign:
    return "=";
    break;
  case ASTOpTypeLT:
    return "<";
    break;
  case ASTOpTypeGT:
    return ">";
    break;
  case ASTOpTypeLE:
    return "<=";
    break;
  case ASTOpTypeGE:
    return ">=";
    break;
  case ASTOpTypeNegate:
    return "!";
    break;
  case ASTOpTypeBitNot:
    return "~";
    break;
  case ASTOpTypeCompEq:
    return "==";
    break;
  case ASTOpTypeCompNeq:
    return "!=";
    break;
  case ASTOpTypeLogicalNot:
    return "!";
    break;
  case ASTOpTypePreInc:
    assert(!Op.empty() && "Invalid Operand argument!");
    OPS = "++";
    OPS += Op;
    return OPS.c_str();
    break;
  case ASTOpTypePreDec:
    assert(!Op.empty() && "Invalid Operand argument!");
    OPS = "--";
    OPS += Op;
    return OPS.c_str();
    break;
  case ASTOpTypePostInc:
    assert(!Op.empty() && "Invalid Operand argument!");
    OPS = Op;
    OPS += "++";
    return OPS.c_str();
    break;
  case ASTOpTypePostDec:
    assert(!Op.empty() && "Invalid Operand argument!");
    OPS = Op;
    OPS += "--";
    return OPS.c_str();
    break;
  case ASTOpTypePow:
    return "**";
    break;
  case ASTOpTypeLeftShift:
    return "<<";
    break;
  case ASTOpTypeRightShift:
    return ">>";
    break;
  case ASTOpTypeLeftShiftAssign:
    return "<<=";
    break;
  case ASTOpTypeRightShiftAssign:
    return ">>=";
    break;
  case ASTOpTypePositive:
    return "+";
    break;
  case ASTOpTypeNegative:
    return "-";
    break;
  case ASTOpTypeUnaryLeftFold:
  case ASTOpTypeBinaryLeftFold:
    return "(";
    break;
  case ASTOpTypeUnaryRightFold:
  case ASTOpTypeBinaryRightFold:
    return ")";
    break;
  case ASTOpTypeSin:
    return "sin";
    break;
  case ASTOpTypeCos:
    return "cos";
    break;
  case ASTOpTypeTan:
    return "tan";
    break;
  case ASTOpTypeArcSin:
    return "arcsin";
    break;
  case ASTOpTypeArcCos:
    return "arccos";
    break;
  case ASTOpTypeArcTan:
    return "arctan";
    break;
  case ASTOpTypeExp:
    return "exp";
    break;
  case ASTOpTypeLn:
    return "ln";
    break;
  case ASTOpTypeSqrt:
    return "sqrt";
    break;
  default:
    return "<unknown>";
    break;
  }
}

const char* PrintSemaTypeEnum(ASTSemaType Type) {
  switch (Type) {
    case SemaTypeUndefined:
      return "SemaTypeUndefined";
      break;
    case SemaTypeArgument:
      return "SemaTypeArgument";
      break;
    case SemaTypeDeclaration:
      return "SemaTypeDeclaration";
      break;
    case SemaTypeExpression:
      return "SemaTypeExpression";
      break;
    case SemaTypeFunction:
      return "SemaTypeFunction";
      break;
    case SemaTypeParameter:
      return "SemaTypeParameter";
      break;
    case SemaTypeStatement:
      return "SemaTypeStatement";
      break;
    case SemaTypeTranslationUnit:
      return "SemaTypeTranslationUnit";
      break;
    case SemaTypeVariable:
      return "SemaTypeVariable";
      break;
    default:
      return "Unknown";
      break;
  }

  return "Unknown";
}

const char* PrintOpTypeEnum(ASTOpType Type) {
  switch (Type) {
    case ASTOpTypeUndefined:
      return "ASTOpTypeUndefined";
      break;
    case ASTOpTypeAdd:
      return "ASTOpTypeAdd";
      break;
    case ASTOpTypeAddAssign:
      return "ASTOpTypeAddAssign";
      break;
    case ASTOpTypeAssociate:
      return "ASTOpTypeAssociate";
      break;
    case ASTOpTypeSub:
      return "ASTOpTypeSub";
      break;
    case ASTOpTypeSubAssign:
      return "ASTOpTypeSubAssign";
      break;
    case ASTOpTypeMul:
      return "ASTOpTypeMul";
      break;
    case ASTOpTypeMulAssign:
      return "ASTOpTypeMulAssign";
      break;
    case ASTOpTypeDiv:
      return "ASTOpTypeDiv";
      break;
    case ASTOpTypeDivAssign:
      return "ASTOpTypeDivAssign";
      break;
    case ASTOpTypeMod:
      return "ASTOpTypeMod";
      break;
    case ASTOpTypeModAssign:
      return "ASTOpTypeModAssign";
      break;
    case ASTOpTypeLogicalAnd:
      return "ASTOpTypeLogicalAnd";
      break;
    case ASTOpTypeLogicalOr:
      return "ASTOpTypeLogicalOr";
      break;
    case ASTOpTypeBitAnd:
      return "ASTOpTypeBitAnd";
      break;
    case ASTOpTypeBitAndAssign:
      return "ASTOpTypeBitAndAssign";
      break;
    case ASTOpTypeBitOr:
      return "ASTOpTypeBitOr";
      break;
    case ASTOpTypeBitOrAssign:
      return "ASTOpTypeBitOrAssign";
      break;
    case ASTOpTypeXor:
      return "ASTOpTypeXor";
      break;
    case ASTOpTypeXorAssign:
      return "ASTOpTypeXorAssign";
      break;
    case ASTOpTypeAssign:
      return "ASTOpTypeAssign";
      break;
    case ASTOpTypeCompEq:
      return "ASTOpTypeCompEq";
      break;
    case ASTOpTypeCompNeq:
      return "ASTOpTypeCompNeq";
      break;
    case ASTOpTypeLT:
      return "ASTOpTypeLT";
      break;
    case ASTOpTypeGT:
      return "ASTOpTypeGT";
      break;
    case ASTOpTypeLE:
      return "ASTOpTypeLE";
      break;
    case ASTOpTypeGE:
      return "ASTOpTypeGE";
      break;
    case ASTOpTypeNegate:
      return "ASTOpTypeNegate";
      break;
    case ASTOpTypePositive:
      return "ASTOpTypePositive";
      break;
    case ASTOpTypeNegative:
      return "ASTOpTypeNegative";
      break;
    case ASTOpTypeLogicalNot:
      return "ASTOpTypeLogicalNot";
      break;
    case ASTOpTypeBitNot:
      return "ASTOpTypeBitNot";
      break;
    case ASTOpTypePreInc:
      return "ASTOpTypePreInc";
      break;
    case ASTOpTypePreDec:
      return "ASTOpTypePreDec";
      break;
    case ASTOpTypePostInc:
      return "ASTOpTypePostInc";
      break;
    case ASTOpTypePostDec:
      return "ASTOpTypePostDec";
      break;
    case ASTOpTypePow:
      return "ASTOpTypePow";
      break;
    case ASTOpTypeLeftShift:
      return "ASTOpTypeLeftShift";
      break;
    case ASTOpTypeLeftShiftAssign:
      return "ASTOpTypeLeftShiftAssign";
      break;
    case ASTOpTypeRightShift:
      return "ASTOpTypeRightShift";
      break;
    case ASTOpTypeRightShiftAssign:
      return "ASTOpTypeRightShiftAssign";
      break;
    case ASTOpTypeSin:
      return "ASTOpTypeSin";
      break;
    case ASTOpTypeCos:
      return "ASTOpTypeCos";
      break;
    case ASTOpTypeTan:
      return "ASTOpTypeTan";
      break;
    case ASTOpTypeArcSin:
      return "ASTOpTypeArcSin";
      break;
    case ASTOpTypeArcCos:
      return "ASTOpTypeArcCos";
      break;
    case ASTOpTypeArcTan:
      return "ASTOpTypeArcTan";
      break;
    case ASTOpTypeExp:
      return "ASTOpTypeExp";
      break;
    case ASTOpTypeLn:
      return "ASTOpTypeLn";
      break;
    case ASTOpTypeSqrt:
      return "ASTOpTypeSqrt";
      break;
    case ASTOpTypeNone:
      return "ASTOpTypeNone";
      break;
    case ASTOpTypeRotation:
      return "ASTOpTypeRotation";
      break;
    case ASTOpTypeRotl:
      return "ASTOpTypeRotl";
      break;
    case ASTOpTypeRotr:
      return "ASTOpTypeRotr";
      break;
    case ASTOpTypePopcount:
      return "ASTOpTypePopcount";
      break;
    case ASTOpTypeBinaryLeftFold:
      return "ASTOpTypeBinaryLeftFold";
      break;
    case ASTOpTypeBinaryRightFold:
      return "ASTOpTypeBinaryRightFold";
      break;
    case ASTOpTypeUnaryLeftFold:
      return "ASTOpTypeUnaryLeftFold";
      break;
    case ASTOpTypeUnaryRightFold:
      return "ASTOpTypeUnaryRightFold";
      break;
    default:
      break;
  }

  return "Unknown";
}

const char* PrintDashOpTypeEnum(ASTOpType OTy) {
  switch (OTy) {
    case ASTOpTypeUndefined:
      return "undefined";
      break;
    case ASTOpTypeAdd:
      return "add";
      break;
    case ASTOpTypeAddAssign:
      return "add-assign";
      break;
    case ASTOpTypeAssociate:
      return "associate";
      break;
    case ASTOpTypeSub:
      return "sub";
      break;
    case ASTOpTypeSubAssign:
      return "sub-assign";
      break;
    case ASTOpTypeMul:
      return "mul";
      break;
    case ASTOpTypeMulAssign:
      return "mul-assign";
      break;
    case ASTOpTypeDiv:
      return "div";
      break;
    case ASTOpTypeDivAssign:
      return "div-assign";
      break;
    case ASTOpTypeMod:
      return "mod";
      break;
    case ASTOpTypeModAssign:
      return "mod-assign";
      break;
    case ASTOpTypeLogicalAnd:
      return "logical-and";
      break;
    case ASTOpTypeLogicalOr:
      return "logical-or";
      break;
    case ASTOpTypeBitAnd:
      return "bit-and";
      break;
    case ASTOpTypeBitAndAssign:
      return "bit-and-assign";
      break;
    case ASTOpTypeBitOr:
      return "bit-or";
      break;
    case ASTOpTypeBitOrAssign:
      return "bit-or-assign";
      break;
    case ASTOpTypeXor:
      return "xor";
      break;
    case ASTOpTypeXorAssign:
      return "xor-assign";
      break;
    case ASTOpTypeAssign:
      return "assign";
      break;
    case ASTOpTypeCompEq:
      return "comp-eq";
      break;
    case ASTOpTypeCompNeq:
      return "comp-neq";
      break;
    case ASTOpTypeLT:
      return "lt";
      break;
    case ASTOpTypeGT:
      return "gt";
      break;
    case ASTOpTypeLE:
      return "le";
      break;
    case ASTOpTypeGE:
      return "ge";
      break;
    case ASTOpTypeNegate:
      return "negate";
      break;
    case ASTOpTypePositive:
      return "positive";
      break;
    case ASTOpTypeNegative:
      return "negative";
      break;
    case ASTOpTypeLogicalNot:
      return "logical-not";
      break;
    case ASTOpTypeBitNot:
      return "bit-not";
      break;
    case ASTOpTypePreInc:
      return "pre-inc";
      break;
    case ASTOpTypePreDec:
      return "pre-dec";
      break;
    case ASTOpTypePostInc:
      return "ASTOpTypePostInc";
      break;
    case ASTOpTypePostDec:
      return "ASTOpTypePostDec";
      break;
    case ASTOpTypePow:
      return "pow";
      break;
    case ASTOpTypeLeftShift:
      return "left-shift";
      break;
    case ASTOpTypeLeftShiftAssign:
      return "left-shift-assign";
      break;
    case ASTOpTypeRightShift:
      return "right-shift";
      break;
    case ASTOpTypeRightShiftAssign:
      return "right-shift-assign";
      break;
    case ASTOpTypeSin:
      return "sin";
      break;
    case ASTOpTypeCos:
      return "cos";
      break;
    case ASTOpTypeTan:
      return "tan";
      break;
    case ASTOpTypeArcSin:
      return "arcsin";
      break;
    case ASTOpTypeArcCos:
      return "arccos";
      break;
    case ASTOpTypeArcTan:
      return "arctan";
      break;
    case ASTOpTypeExp:
      return "exp";
      break;
    case ASTOpTypeLn:
      return "ln";
      break;
    case ASTOpTypeSqrt:
      return "sqrt";
      break;
    case ASTOpTypeNone:
      return "none";
      break;
    case ASTOpTypeRotation:
      return "rotation";
      break;
    case ASTOpTypeRotl:
      return "rotl";
      break;
    case ASTOpTypeRotr:
      return "rotr";
      break;
    case ASTOpTypePopcount:
      return "popcount";
      break;
    case ASTOpTypeBinaryLeftFold:
      return "binary-left-fold";
      break;
    case ASTOpTypeBinaryRightFold:
      return "binary-right-fold";
      break;
    case ASTOpTypeUnaryLeftFold:
      return "unary-left-fold";
      break;
    case ASTOpTypeUnaryRightFold:
      return "unary-right-fold";
      break;
    default:
      return "undefined";
      break;
  }

  return "undefined";
}

const char* PrintNumericConstant(ASTNumericConstant CTy) {
  switch (CTy) {
  case CNone:
    return "None";
    break;
  case CPi:
    return "Pi";
    break;
  case CTau:
    return "Tau";
    break;
  case CEuler:
    return "Euler";
    break;
  default:
    return "Unknown";
    break;
  }
}

const char* PrintEvalMethod(ASTEvaluationMethod M) {
  switch (M) {
  case Arithmetic:
    return "Arithmetic";
    break;
  case Radian:
    return "Radian";
    break;
  default:
    return "Unknown";
    break;
  }
}

const char* PrintRotationType(ASTRotationType Ty) {
  switch (Ty) {
  case ASTRotationTypeLeft:
    return "ASTRotationTypeLeft";
    break;
  case ASTRotationTypeRight:
    return "ASTRotationTypeRight";
    break;
  default:
    break;
  }

  return "ASTRotationTypeUnknown";
}

const char* PrintParserToken(unsigned Token) {
  using token = QASM::Parser::token;

  switch (Token) {
  case token::TOK_IDENTIFIER:
    return "TOK_IDENTIFIER";
    break;
  case token::TOK_INTEGER_CONSTANT:
    return "TOK_INTEGER_CONSTANT";
    break;
  case token::TOK_FP_CONSTANT:
    return "TOK_FP_CONSTANT";
    break;
  case token::TOK_STRING_LITERAL:
    return "TOK_STRING_LITERAL";
    break;
  case token::TOK_FUNC_NAME:
    return "TOK_FUNC_NAME";
    break;
  case token::TOK_SIZEOF:
    return "TOK_SIZEOF";
    break;
  case token::TOK_BOOLEAN_CONSTANT:
    return "TOK_BOOLEAN_CONSTANT";
    break;
  case token::TOK_BOOL:
    return "TOK_BOOL";
    break;
  case token::TOK_CHAR:
    return "TOK_CHAR";
    break;
  case token::TOK_SHORT:
    return "TOK_SHORT";
    break;
  case token::TOK_INT:
    return "TOK_INT";
    break;
  case token::TOK_UINT:
    return "TOK_UINT";
    break;
  case token::TOK_LONG:
    return "TOK_LONG";
    break;
  case token::TOK_SIGNED:
    return "TOK_SIGNED";
    break;
  case token::TOK_UNSIGNED:
    return "TOK_UNSIGNED";
    break;
  case token::TOK_FLOAT:
    return "TOK_FLOAT";
    break;
  case token::TOK_DOUBLE:
    return "TOK_DOUBLE";
    break;
  case token::TOK_LONG_DOUBLE:
    return "TOK_LONG_DOUBLE";
    break;
  case token::TOK_VOID:
    return "TOK_VOID";
    break;
  case token::TOK_TYPEDEF_NAME:
    return "TOK_TYPEDEF_NAME";
    break;
  case token::TOK_DEFCAL:
    return "TOK_DEFCAL";
    break;
  case token::TOK_CREG:
    return "TOK_CREG";
    break;
  case token::TOK_QREG:
    return "TOK_QREG";
    break;
  case token::TOK_CNOT:
    return "TOK_CNOT";
    break;
  case token::TOK_HADAMARD:
    return "TOK_HADAMARD";
    break;
  case token::TOK_CCX:
    return "TOK_CCX";
    break;
  case token::TOK_CX:
    return "TOK_CX";
    break;
  case token::TOK_QUBIT:
    return "TOK_QUBIT";
    break;
  case token::TOK_BOUND_QUBIT:
    return "TOK_BOUND_QUBIT";
    break;
  case token::TOK_UNBOUND_QUBIT:
    return "TOK_UNBOUND_QUBIT";
    break;
  case token::TOK_QUBITS:
    return "TOK_QUBITS";
    break;
  case token::TOK_U:
    return "TOK_U";
    break;
  case token::TOK_ANGLE:
    return "TOK_ANGLE";
    break;
  case token::TOK_FIXED:
    return "TOK_FIXED";
    break;
  case token::TOK_GATE:
    return "TOK_GATE";
    break;
  case token::TOK_BARRIER:
    return "TOK_BARRIER";
    break;
  case token::TOK_MEASURE:
    return "TOK_MEASURE";
    break;
  case token::TOK_KERNEL:
    return "TOK_KERNEL";
    break;
  case token::TOK_DIRTY:
    return "TOK_DIRTY";
    break;
  case token::TOK_OPAQUE:
    return "TOK_OPAQUE";
    break;
  case token::TOK_RESET:
    return "TOK_RESET";
    break;
  case token::TOK_ALPHA:
    return "TOK_ALPHA";
    break;
  case token::TOK_BETA:
    return "TOK_BETA";
    break;
  case token::TOK_GAMMA:
    return "TOK_GAMMA";
    break;
  case token::TOK_DELTA:
    return "TOK_DELTA";
    break;
  case token::TOK_EPSILON:
    return "TOK_EPSILON";
    break;
  case token::TOK_ZETA:
    return "TOK_ZETA";
    break;
  case token::TOK_ETA:
    return "TOK_ETA";
    break;
  case token::TOK_THETA:
    return "TOK_THETA";
    break;
  case token::TOK_IOTA:
    return "TOK_IOTA";
    break;
  case token::TOK_KAPPA:
    return "TOK_KAPPA";
    break;
  case token::TOK_LAMBDA:
    return "TOK_LAMBDA";
    break;
  case token::TOK_MU:
    return "TOK_MU";
    break;
  case token::TOK_NU:
    return "TOK_NU";
    break;
  case token::TOK_XI:
    return "TOK_XI";
    break;
  case token::TOK_OMICRON:
    return "TOK_OMICRON";
    break;
  case token::TOK_PI:
    return "TOK_PI";
    break;
  case token::TOK_RHO:
    return "TOK_RHO";
    break;
  case token::TOK_SIGMA:
    return "TOK_SIGMA";
    break;
  case token::TOK_TAU:
    return "TOK_TAU";
    break;
  case token::TOK_UPSILON:
    return "TOK_UPSILON";
    break;
  case token::TOK_PHI:
    return "TOK_PHI";
    break;
  case token::TOK_CHI:
    return "TOK_CHI";
    break;
  case token::TOK_PSI:
    return "TOK_PSI";
    break;
  case token::TOK_OMEGA:
    return "TOK_OMEGA";
    break;
  case token::TOK_EULER:
    return "TOK_EULER";
    break;
  case token::TOK_SIN:
    return "TOK_SIN";
    break;
  case token::TOK_COS:
    return "TOK_COS";
    break;
  case token::TOK_TAN:
    return "TOK_TAN";
    break;
  case token::TOK_EXP:
    return "TOK_EXP";
    break;
  case token::TOK_LN:
    return "TOK_LN";
    break;
  case token::TOK_SQRT:
    return "TOK_SQRT";
    break;
  case token::TOK_IBMQASM:
    return "TOK_IBMQASM";
    break;
  case token::TOK_PTR_OP:
    return "TOK_PTR_OP";
    break;
  case token::TOK_INC_OP:
    return "TOK_INC_OP";
    break;
  case token::TOK_DEC_OP:
    return "TOK_DEC_OP";
    break;
  case token::TOK_LEFT_SHIFT_OP:
    return "TOK_LEFT_SHIFT_OP";
    break;
  case token::TOK_RIGHT_SHIFT_OP:
    return "TOK_RIGHT_SHIFT_OP";
    break;
  case token::TOK_LE_OP:
    return "TOK_LE_OP";
    break;
  case token::TOK_GE_OP:
    return "TOK_GE_OP";
    break;
  case token::TOK_EQ_OP:
    return "TOK_EQ_OP";
    break;
  case token::TOK_NE_OP:
    return "TOK_NE_OP";
    break;
  case token::TOK_AND_OP:
    return "TOK_AND_OP";
    break;
  case token::TOK_OR_OP:
    return "TOK_OR_OP";
    break;
  case token::TOK_MUL_ASSIGN:
    return "TOK_MUL_ASSIGN";
    break;
  case token::TOK_DIV_ASSIGN:
    return "TOK_DIV_ASSIGN";
    break;
  case token::TOK_MOD_ASSIGN:
    return "TOK_MOD_ASSIGN";
    break;
  case token::TOK_ADD_ASSIGN:
    return "TOK_ADD_ASSIGN";
    break;
  case token::TOK_SUB_ASSIGN:
    return "TOK_SUB_ASSIGN";
    break;
  case token::TOK_LEFT_SHIFT_ASSIGN:
    return "TOK_LEFT_SHIFT_ASSIGN";
    break;
  case token::TOK_RIGHT_SHIFT_ASSIGN:
    return "TOK_RIGHT_SHIFT_ASSIGN";
    break;
  case token::TOK_AND_ASSIGN:
    return "TOK_AND_ASSIGN";
    break;
  case token::TOK_XOR_ASSIGN:
    return "TOK_XOR_ASSIGN";
    break;
  case token::TOK_OR_ASSIGN:
    return "TOK_OR_ASSIGN";
    break;
  case token::TOK_RIGHT_ARROW:
    return "TOK_RIGHT_ARROW";
    break;
  case token::TOK_LEFT_ARROW:
    return "TOK_LEFT_ARROW";
    break;
  case token::TOK_CONST:
    return "TOK_CONST";
    break;
  case token::TOK_STRETCH:
    return "TOK_STRETCH";
    break;
  case token::TOK_STRETCH_N:
    return "TOK_STRETCH_N";
    break;
  case token::TOK_STRETCHINF:
    return "TOK_STRETCHINF";
    break;
  case token::TOK_LENGTH:
    return "TOK_LENGTH";
    break;
  case token::TOK_LENGTHOF:
    return "TOK_LENGTHOF";
    break;
  case token::TOK_DELAY:
    return "TOK_DELAY";
    break;
  case token::TOK_IN:
    return "TOK_IN";
    break;
  case token::TOK_BIT:
    return "TOK_BIT";
    break;
  case token::TOK_COMPLEX:
    return "TOK_COMPLEX";
    break;
  case token::TOK_IMAGINARY:
    return "TOK_IMAGINARY";
    break;
  case token::TOK_UNION:
    return "TOK_UNION";
    break;
  case token::TOK_ENUM:
    return "TOK_ENUM";
    break;
  case token::TOK_ELLIPSIS:
    return "TOK_ELLIPSIS";
    break;
  case token::TOK_CASE:
    return "TOK_CASE";
    break;
  case token::TOK_IF:
    return "TOK_IF";
    break;
  case token::TOK_ELSEIF:
    return "TOK_ELSEIF";
    break;
  case token::TOK_ELSE:
    return "TOK_ELSE";
    break;
  case token::TOK_WHILE:
    return "TOK_WHILE";
    break;
  case token::TOK_DO:
    return "TOK_DO";
    break;
  case token::TOK_FOR:
    return "TOK_FOR";
    break;
  case token::TOK_CONTINUE:
    return "TOK_CONTINUE";
    break;
  case token::TOK_BREAK:
    return "TOK_BREAK";
    break;
  case token::TOK_RETURN:
    return "TOK_RETURN";
    break;
  case token::TOK_LEFT_CURLY:
    return "TOK_LEFT_CURLY";
    break;
  case token::TOK_RIGHT_CURLY:
    return "TOK_RIGHT_CURLY";
    break;
  case token::TOK_COMMA:
    return "TOK_COMMA";
    break;
  case token::TOK_SEMICOLON:
    return "TOK_SEMICOLON";
    break;
  case token::TOK_EQUAL_ASSIGN:
    return "TOK_EQUAL_ASSIGN";
    break;
  case token::TOK_LEFT_PAREN:
    return "TOK_LEFT_PAREN";
    break;
  case token::TOK_RIGHT_PAREN:
    return "TOK_RIGHT_PAREN";
    break;
  case token::TOK_LEFT_BRACKET:
    return "TOK_LEFT_BRACKET";
    break;
  case token::TOK_RIGHT_BRACKET:
    return "TOK_RIGHT_BRACKET";
    break;
  case token::TOK_PERIOD:
    return "TOK_PERIOD";
    break;
  case token::TOK_AMPERSAND:
    return "TOK_AMPERSAND";
    break;
  case token::TOK_BANG:
    return "TOK_BANG";
    break;
  case token::TOK_MINUS:
    return "TOK_MINUS";
    break;
  case token::TOK_PLUS:
    return "TOK_PLUS";
    break;
  case token::TOK_MUL_OP:
    return "TOK_MUL";
    break;
  case token::TOK_DIV_OP:
    return "TOK_DIV";
    break;
  case token::TOK_MOD_OP:
    return "TOK_MOD";
    break;
  case token::TOK_LT_OP:
    return "TOK_LT";
    break;
  case token::TOK_GT_OP:
    return "TOK_GT";
    break;
  case token::TOK_XOR_OP:
    return "TOK_XOR";
    break;
  case token::TOK_QUESTION:
    return "TOK_QUESTION";
    break;
  case token::TOK_END:
    return "TOK_END";
    break;
  case token::TOK_POW:
    return "TOK_POW";
    break;
  case token::TOK_GPHASE:
    return "TOK_GPHASE";
    break;
  default:
    return "UNKNOWN";
    break;
  }

  return "UNKNOWN";
}

const char* PrintQubitKind(QubitKind QK) {
  switch (QK) {
  case Undetermined:
    return "Undetermined";
    break;
  case Bound:
    return "Bound";
    break;
  case Unbound:
    return "Unbound";
  default:
    break;
  }

  return "Undetermined";
}

const char* PrintLengthUnit(LengthUnit LU) {
  switch (LU) {
  case LengthUnspecified:
    return "LengthUnspecified";
    break;
  case Nanoseconds:
    return "Nanoseconds";
    break;
  case Microseconds:
    return "Microseconds";
    break;
  case Milliseconds:
    return "Milliseconds";
    break;
  case Seconds:
    return "Seconds";
    break;
  case DT:
    return "DT";
    break;
  case LengthOfDependent:
    return "LengthOfDependent";
    break;
  case BinaryOpDependent:
    return "BinaryOpDependent";
    break;
  case FunctionCallDependent:
    return "FunctionCallDependent";
    break;
  default:
    break;
  }

  return "Unspecified";
}

const char* PrintGateKind(GateKind GK) {
  switch (GK) {
  case ASTGateKindGeneric:
    return "Generic";
    break;
  case ASTGateKindCX:
    return "CX";
    break;
  case ASTGateKindCCX:
    return "CCX";
    break;
  case ASTGateKindCNOT:
    return "CNOT";
    break;
  case ASTGateKindH:
    return "Hadamard";
    break;
  case ASTGateKindU:
    return "U";
    break;
  case ASTGateKindLambda:
    return "Lambda";
    break;
  case ASTGateKindPhi:
    return "Phi";
    break;
  case ASTGateKindTheta:
    return "Theta";
    break;
  default:
    break;
  }

  return "Generic";
}

const char* PrintAngleType(ASTAngleType AT) {
  switch (AT) {
  case ASTAngleTypeAlpha:
    return "Alpha";
    break;
  case ASTAngleTypeBeta:
    return "Beta";
    break;
  case ASTAngleTypeGamma:
    return "Gamma";
    break;
  case ASTAngleTypeDelta:
    return "Delta";
    break;
  case ASTAngleTypeEpsilon:
    return "Epsilon";
    break;
  case ASTAngleTypeZeta:
    return "Zeta";
    break;
  case ASTAngleTypeEta:
    return "Eta";
    break;
  case ASTAngleTypeTheta:
    return "Theta";
    break;
  case ASTAngleTypeIota:
    return "Iota";
    break;
  case ASTAngleTypeKappa:
    return "Kappa";
    break;
  case ASTAngleTypeLambda:
    return "Lambda";
    break;
  case ASTAngleTypeMu:
    return "Mu";
    break;
  case ASTAngleTypeNu:
    return "Nu";
    break;
  case ASTAngleTypeXi:
    return "Xi";
    break;
  case ASTAngleTypeOmicron:
    return "Omicron";
    break;
  case ASTAngleTypePi:
    return "Pi";
    break;
  case ASTAngleTypeRho:
    return "Rho";
    break;
  case ASTAngleTypeSigma:
      return "Sigma";
    break;
  case ASTAngleTypeTau:
    return "Tau";
    break;
  case ASTAngleTypeUpsilon:
    return "Upsilon";
    break;
  case ASTAngleTypePhi:
    return "Phi";
    break;
  case ASTAngleTypeChi:
    return "Chi";
    break;
  case ASTAngleTypePsi:
    return "Psi";
    break;
  case ASTAngleTypeOmega:
    return "Omega";
    break;
  case ASTAngleTypeEuler:
    return "Euler";
    break;
  default:
    break;
  }

  return "Generic";
}

const char* PrintSymbolScope(ASTSymbolScope S) {
  switch (S) {
  case ScopeUnspecified:
    return "Unspecified";
    break;
  case Global:
    return "Global";
    break;
  case Local:
    return "Local";
    break;
  case Extern:
    return "Extern";
    break;
  default:
    break;
  }

  return "Unspecified";
}

const char* PrintScopeState(ASTScopeState S) {
  switch (S) {
  case Alive:
    return "Alive";
    break;
  case Dead:
    return "Dead";
    break;
  default:
    break;
  }

  return "Unspecified";
}

const char* PrintTypeConversionMethod(ASTTypeConversionMethod M) {
  switch (M) {
  case ASTConvMethodUnknown:
    return "ASTConvMethodUnknown";
    break;
  case ASTConvMethodBadCast:
    return "ASTConvMethodBadCast";
    break;
  case ASTConvMethodBitcast:
    return "ASTConvMethodBitcast";
    break;
  case ASTConvMethodConversion:
    return "ASTConvMethodConversion";
    break;
  case ASTConvMethodPromotion:
    return "ASTConvMethodPromotion";
    break;
  case ASTConvMethodTruncation:
    return "ASTConvMethodTruncation";
    break;
  default:
    break;
  }

  return "ASTConvMethodUnknown";
}

} // namespace QASM

