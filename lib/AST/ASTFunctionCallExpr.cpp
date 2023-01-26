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

#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTScopeController.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <sstream>
#include <cassert>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTFunctionCallValidator ASTFunctionCallValidator::FCV;

void ASTFunctionCallNode::Mangle() {
  ASTMangler M;
  M.Start();

  switch (CallType) {
  case ASTTypeFunction:
  case ASTTypeFunctionCallExpression:
    M.TypeIdentifier(ASTTypeFunctionCall, GetIdentifier()->GetPolymorphicName());
    break;
  case ASTTypeDefcal:
  case ASTTypeDefcalCallExpression:
    M.TypeIdentifier(ASTTypeDefcalCall, GetIdentifier()->GetName());
    break;
  case ASTTypeKernel:
  case ASTTypeKernelCallExpression:
    M.TypeIdentifier(ASTTypeKernelCall, GetIdentifier()->GetName());
    break;
  default: {
    std::stringstream MM;
    MM << "Object of type " << PrintTypeEnum(CallType) << " is not callable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(CId), MM.str(),
                                                    DiagLevel::Error);
  }
    break;
  }

  switch (CallType) {
  case ASTTypeFunctionCallExpression:
  case ASTTypeFunction: {
    assert(FDN && "Invalid ASTFunctionDefinitionNode!");
    const ASTResultNode* RN = FDN->GetResult();
    assert(RN && "Invalid function result obtained from function definition!");

    switch (RN->GetResultType()) {
    case ASTTypeBool:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeVoid:
    case ASTTypeOpenPulseFrame:
    case ASTTypeOpenPulsePort:
    case ASTTypeOpenPulseWaveform:
    case ASTTypeDuration:
    case ASTTypeStretch:
    case ASTTypeBox:
      M.FuncReturn(RN->GetResultType());
      break;
    case ASTTypeAngle:
    case ASTTypeBitset:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeMPDecimal:
    case ASTTypeMPComplex:
    case ASTTypeAngleArray:
    case ASTTypeBoolArray:
    case ASTTypeCBitArray:
    case ASTTypeFloatArray:
    case ASTTypeIntArray:
    case ASTTypeMPIntegerArray:
    case ASTTypeMPDecimalArray:
    case ASTTypeMPComplexArray:
    case ASTTypeDurationArray:
    case ASTTypeOpenPulseFrameArray:
    case ASTTypeOpenPulsePortArray:
      M.FuncReturn(RN->GetResultType(), RN->GetResultBits());
      break;
    default: {
      std::stringstream MS;
      MS << "A function cannot return " << PrintTypeEnum(RN->GetResultType())
        << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), MS.str(), DiagLevel::Error);
    }
      break;
    }
  }
    break;
  case ASTTypeDefcalCallExpression:
  case ASTTypeDefcal: {
    assert(DDN && "Invalid ASTDefcalNode!");
    if (DDN->IsMeasure()) {
      const ASTMeasureNode* MN = DDN->GetMeasure();
      assert(MN && "Invalid ASTMeasureNode obtained from defcal!");

      if (MN->HasResult()) {
        const ASTCBitNode* CBN = MN->GetResult();
        assert(CBN && "Invalid ASTCBitNode result obtained from ASTMeasureNode!");

        M.FuncReturn(CBN->GetASTType());
        M.Underscore();
      } else if (MN->HasAngleResult()) {
        const ASTAngleNode* AN = MN->GetAngleResult();
        assert(AN && "Invalid ASTAngleNode result obtained from ASTMeasureNode!");

        M.FuncReturn(AN->GetASTType());
        M.Underscore();
      }
    }
  }
    break;
  case ASTTypeKernelCallExpression:
  case ASTTypeKernel: {
    assert(KDN && "Invalid ASTKernelNode!");
    const ASTResultNode* RN = KDN->GetResult();
    assert(RN && "Invalid function result obtained from kernel definition!");

    M.FuncReturn(RN->GetResultType());
    M.Underscore();
  }
    break;
  default: {
    std::stringstream MS;
    MS << "Invalid Call Type.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), MS.str(), DiagLevel::Error);
  }
    break;
  }

  for (std::map<unsigned, ASTSymbolTableEntry*>::iterator I = STEMap.begin();
       I != STEMap.end(); ++I) {
    ASTScopeController::Instance().CheckUndefined((*I).second->GetIdentifier());
    ASTScopeController::Instance().CheckOutOfScope((*I).second->GetIdentifier());

    switch (CallType) {
    case ASTTypeFunctionCallExpression:
    case ASTTypeFunction:
      assert((*I).second && "Invalid SymbolTable Entry for function call!");
      if ((*I).second->GetValueType() == ASTTypeInt) {
        (*I).second->GetValue()->GetValue<ASTIntNode*>()->MangleLiteral();
      } else if ((*I).second->GetValueType() == ASTTypeUInt) {
        (*I).second->GetValue()->GetValue<ASTIntNode*>()->MangleLiteral();
      } else if ((*I).second->GetValueType() == ASTTypeFloat) {
        (*I).second->GetValue()->GetValue<ASTFloatNode*>()->MangleLiteral();
      } else if ((*I).second->GetValueType() == ASTTypeDouble) {
        (*I).second->GetValue()->GetValue<ASTDoubleNode*>()->MangleLiteral();
      } else if ((*I).second->GetValueType() == ASTTypeMPInteger) {
        (*I).second->GetValue()->GetValue<ASTMPIntegerNode*>()->MangleLiteral();
      } else if ((*I).second->GetValueType() == ASTTypeMPUInteger) {
        (*I).second->GetValue()->GetValue<ASTMPIntegerNode*>()->MangleLiteral();
      } else if ((*I).second->GetValueType() == ASTTypeMPDecimal) {
        (*I).second->GetValue()->GetValue<ASTMPDecimalNode*>()->MangleLiteral();
      } else if ((*I).second->GetValueType() == ASTTypeMPComplex) {
        (*I).second->GetValue()->GetValue<ASTMPComplexNode*>()->MangleLiteral();
      } else if ((*I).second->GetValueType() == ASTTypeAngle) {
        (*I).second->GetValue()->GetValue<ASTAngleNode*>()->MangleLiteral();
      }

      switch ((*I).second->GetValueType()) {
      case ASTTypeInt:
      case ASTTypeUInt:
      case ASTTypeFloat:
      case ASTTypeDouble:
      case ASTTypeMPInteger:
      case ASTTypeMPUInteger:
      case ASTTypeMPDecimal:
      case ASTTypeMPComplex:
      case ASTTypeAngle:
        M.FuncArg((*I).first, ASTStringUtils::Instance().SanitizeMangled(
                              (*I).second->GetIdentifier()->GetMangledLiteralName()));
        break;
      default:
        M.FuncArg((*I).first, ASTStringUtils::Instance().SanitizeMangled(
                              (*I).second->GetIdentifier()->GetMangledName()));
        break;
      }
      break;
    case ASTTypeDefcalCallExpression:
    case ASTTypeDefcal:
      assert((*I).second && "Invalid SymbolTable Entry for defcal call!");
      M.DefcalArg((*I).first, (*I).second->GetValueType(),
                  (*I).second->GetIdentifier()->GetBits(),
                  (*I).second->GetIdentifier()->GetName());
      break;
    case ASTTypeKernelCallExpression:
    case ASTTypeKernel:
      assert((*I).second && "Invalid SymbolTable Entry for kernel call!");
      M.KernelArg((*I).first, (*I).second->GetValueType(),
                  (*I).second->GetIdentifier()->GetBits(),
                  (*I).second->GetIdentifier()->GetName());
      break;
    default:
      break;
    }
  }

  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTFunctionCallValidator::ValidateArgument(const ASTSymbolTableEntry* XSTE,
                                                unsigned XI,
                                                const std::string& AN,
                                                const std::string& FN) {
  assert(XSTE && "Invalid ASTSymbolTableEntry argument!");

  const ASTIdentifierNode* XId = XSTE->GetIdentifier();

  if (!XSTE->HasValue() || XSTE->GetValueType() == ASTTypeUndefined) {
    std::stringstream M;
    M << "Invalid argument " << XI << " (" << AN << ") to function call "
      << FN << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(XId), M.str(), DiagLevel::Error);
  }
}

void ASTFunctionCallValidator::ValidateArgument(const ASTFunctionDefinitionNode* FDN,
                                                const ASTSymbolTableEntry* XSTE,
                                                unsigned XI,
                                                const std::string& AN,
                                                const std::string& FN) {
  assert(XSTE && "Invalid ASTSymbolTableEntry argument!");

  const ASTIdentifierNode* XId = XSTE->GetIdentifier();

  if (FDN->GetNumParameters() > 0) {
    if (!XSTE->HasValue() || XSTE->GetValueType() == ASTTypeUndefined) {
      std::stringstream M;
      M << "Invalid argument " << XI << " (" << AN << ") to function call "
        << FN << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(XId), M.str(), DiagLevel::Error);
    }

    if (XI >= FDN->GetNumParameters()) {
      std::stringstream M;
      M << "Argument index exceeds the number of function arguments.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(XId), M.str(), DiagLevel::Error);
    }

    ASTType ATy = XSTE->GetValueType();
    ASTType PTy = ASTTypeUndefined;

    if (ATy == ASTTypeIdentifier) {
      const ASTIdentifierNode* Id = XSTE->GetValue()->GetValue<const ASTIdentifierNode*>();
      assert(Id && "Could not obtain a valid ASTIdentifierNode from the SymbolTable!");

      if (Id->IsReference()) {
        if (const ASTIdentifierRefNode* IdR =
            dynamic_cast<const ASTIdentifierRefNode*>(Id))
          ATy = IdR->GetReferenceType();
      } else {
        ATy = Id->GetSymbolType();
      }
    }

    const ASTSymbolTableEntry* PSTE = nullptr;
    if (const ASTDeclarationNode* PDN = FDN->GetParameter(XI)) {
      PSTE = FDN->GetParameterSymbol(PDN->GetName());
      if (!PSTE) {
        std::stringstream M;
        M << "Function " << FN << " has no symbol at parameter index " << XI << " ("
          << PDN->GetName() << ").";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(XId), M.str(), DiagLevel::Error);
      }

      PTy = PSTE->GetValueType();
    } else {
      std::stringstream M;
      M << "Function " << FN << " has no parameter at index " << XI << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(XId), M.str(), DiagLevel::Error);
    }

    // FIXME: THESE ARE NECESSARY FOR CHECKING N-DIMENSIONAL ARRAYS.
    (void) ATy;
    (void) PTy;
  }
}

} // namespace QASM

