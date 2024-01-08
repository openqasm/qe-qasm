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

#include <qasm/AST/ASTDefcal.h>
#include <qasm/AST/ASTFunctions.h>
#include <qasm/AST/ASTKernel.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTResult.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <cassert>
#include <iostream>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

void ASTResultNode::print() const {
  std::cout << "<Result>" << std::endl;
  std::cout << "<Identifier>" << Ident->GetName() << "</Identifier>"
            << std::endl;
  std::cout << "<MangledName>" << Ident->GetMangledName() << "</MangledName>"
            << std::endl;
  std::cout << "<Type>" << QASM::PrintTypeEnum(Type) << "</Type>" << std::endl;

  switch (Type) {
  case ASTTypeAngle:
    if (Angle)
      Angle->print();
    break;
  case ASTTypeBool:
    if (Bool)
      Bool->print();
    break;
  case ASTTypeVoid:
    if (Void)
      Void->print();
    break;
  case ASTTypeBitset:
    if (CBit)
      CBit->print();
    break;
  case ASTTypeInt:
    if (Int)
      Int->print();
    break;
  case ASTTypeFloat:
    if (Float)
      Float->print();
    break;
  case ASTTypeDouble:
    if (Double)
      Double->print();
    break;
  case ASTTypeMPInteger:
    if (MPInt)
      MPInt->print();
    break;
  case ASTTypeMPDecimal:
    if (MPDec)
      MPDec->print();
    break;
  case ASTTypeMPComplex:
    if (MPComplex)
      MPComplex->print();
    break;
  case ASTTypeOpenPulseWaveform:
    if (Waveform)
      Waveform->print();
    break;
  case ASTTypeOpenPulseFrame:
    if (Frame)
      Frame->print();
    break;
  case ASTTypeBinaryOp:
    if (BinaryOp && !P) {
      P = true;
      BinaryOp->print();
      P = false;
    }
    break;
  case ASTTypeUnaryOp:
    if (UnaryOp && !P) {
      P = true;
      UnaryOp->print();
      P = false;
    }
    break;
  default:
    break;
  }

  if (IsFunction()) {
    std::cout << "<FunctionName>";
    if (Id)
      std::cout << Id->GetName();
    std::cout << "</FunctionName>" << std::endl;
  } else if (IsKernel()) {
    std::cout << "<KernelName>";
    if (Id)
      std::cout << Id->GetName();
    std::cout << "</KernelName>" << std::endl;
  }

  std::cout << "</Result>" << std::endl;
}

void ASTResultNode::Mangle() {
  ASTMangler M;
  M.Start();

  switch (GetResultType()) {
  case ASTTypeBool:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), Bool->GetName());
    break;
  case ASTTypeInt:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), Int->GetName());
    break;
  case ASTTypeFloat:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), Float->GetName());
    break;
  case ASTTypeDouble:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), Double->GetName());
    break;
  case ASTTypeLongDouble:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), LongDouble->GetName());
    break;
  case ASTTypeVoid:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), Void->GetName());
    break;
  case ASTTypeDuration:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), Duration->GetName());
    break;
  case ASTTypeOpenPulseFrame:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), Frame->GetName());
    break;
  case ASTTypeOpenPulseWaveform:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), Waveform->GetName());
    break;
  case ASTTypeOpenPulsePort:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), Port->GetName());
    break;
  case ASTTypeAngle:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), Angle->GetBits(), Angle->GetName());
    break;
  case ASTTypeMPInteger:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), MPInt->GetBits(), MPInt->GetName());
    break;
  case ASTTypeMPDecimal:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), MPDec->GetBits(), MPDec->GetName());
    break;
  case ASTTypeMPComplex:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), MPComplex->GetBits(),
                     MPComplex->GetName());
    break;
  case ASTTypeBitset:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(GetResultType(), CBit->Size(), CBit->GetName());
    break;
  default:
    M.TypeIdentifier(GetASTType(), GetPolymorphicName());
    M.Underscore();
    M.TypeIdentifier(ASTTypeVoid, "void");
    break;
  }

  M.EndExpression();

  if (IsKernel()) {
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
        GetKernel()->GetMangledName()));
    M.EndExpression();
  } else if (IsFunction()) {
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
        GetFunction()->GetMangledName()));
    M.EndExpression();
  } else if (IsDefcal()) {
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
        GetDefcal()->GetMangledName()));
    M.EndExpression();
  }

  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace QASM
