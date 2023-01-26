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
#include <qasm/AST/ASTDefcalGrammarBuilder.h>
#include <qasm/AST/ASTScopeController.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseCalibration.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <sstream>
#include <string>

namespace QASM {
namespace OpenPulse {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTOpenPulseCalibrationBuilder ASTOpenPulseCalibrationBuilder::CB;
bool ASTOpenPulseCalibrationBuilder::CX = false;

void ASTOpenPulseCalibrationBuilder::ValidateContext() const {
  ASTDefcalGrammarBuilder::Instance().ValidateContext(
    ASTDefcalGrammarBuilder::Instance().GetCurrent());

  if (!CX && !ASTCalContextBuilder::Instance().InOpenContext()) {
    std::stringstream M;
    M << "OpenPulse semantics are only allowed within an "
      << "OpenPulse Calibration context.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Error);
  }
}

void ASTOpenPulseCalibration::SetCalibrationContext() {
  ASTScopeController::Instance().SetCalBlockScope(&CSL, DCB);
}

void ASTOpenPulseCalibration::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace OpenPulse
} // namespace QASM

