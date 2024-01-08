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
#include <qasm/AST/ASTTypeCastController.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <sstream>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

bool ASTCBitNode::ResolveCast() {
  if (OTy == ASTTypeCast && CST) {
    if (!ASTTypeCastController::Instance().CanCast(CST)) {
      std::stringstream M;
      M << "Impossible cast from " << PrintTypeEnum(CST->GetCastFrom())
        << " to " << PrintTypeEnum(CST->GetCastTo()) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(this), M.str(),
          DiagLevel::Error);
      return false;
    }

    ASTCBitNode *CBX = CST->CastToBitset();
    if (!CBX) {
      std::stringstream M;
      M << "Failure casting " << PrintTypeEnum(CST->GetCastTo())
        << " to an ASTCBitNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(this), M.str(),
          DiagLevel::ICE);
      return false;
    }

    BV = CBX->BV;
    NR = CBX->NR;
    SR = CBX->SR;
    SZ = CBX->SZ;
    OTy = ASTTypeUndefined;
    NBC = false;
    return true;
  }

  return false;
}

} // namespace QASM
