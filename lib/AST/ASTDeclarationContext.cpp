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

#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

bool ASTDeclarationContextTracker::II = false;
unsigned ASTDeclarationContextTracker::CIX = 1U;
unsigned ASTDeclarationContextTracker::RSL = 24U;
ASTDeclarationContextTracker ASTDeclarationContextTracker::DCT;

const ASTDeclarationContext __attribute__((init_priority(101)))
ASTDeclarationContextTracker::GCX("GlobalContext", 0U, ASTTypeGlobal, nullptr);

const ASTDeclarationContext __attribute__((init_priority(104)))
ASTDeclarationContextTracker::CCX("DefaultCalibrationContext",
                                  static_cast<unsigned>(~0x0),
                                  ASTTypeOpenPulseCalibration,
                                  &ASTDeclarationContextTracker::GCX);

std::map<unsigned, const ASTDeclarationContext *> __attribute__((
    init_priority(102))) ASTDeclarationContextTracker::M = {
    {0U, &ASTDeclarationContextTracker::GCX},
};

std::vector<const ASTDeclarationContext *> __attribute__((init_priority(103)))
ASTDeclarationContextTracker::CCV = {
    &ASTDeclarationContextTracker::GCX,
};

void ASTDeclarationContextTracker::Init() {
  if (M.empty() || M.find(0U) == M.end()) {
    if (!M.insert(std::make_pair(0U, &ASTDeclarationContextTracker::GCX))
             .second) {
      std::stringstream MM;
      MM << "Failure inserting the Global Declaration Context into the "
         << "global map.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::ICE);
      abort();
    }

    ASTDeclarationContextTracker::CIX += 1;
  }

  if (CCV.empty())
    CCV.push_back(&ASTDeclarationContextTracker::GCX);

  II = true;
}

} // namespace QASM
