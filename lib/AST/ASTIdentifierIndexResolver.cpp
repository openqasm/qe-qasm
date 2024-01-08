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

#include <qasm/AST/ASTIdentifierIndexResolver.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <iostream>
#include <sstream>

namespace QASM {

ASTIdentifierIndexResolver ASTIdentifierIndexResolver::IIR;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

unsigned ASTIdentifierIndexResolver::ResolveIndex(const std::string &Id) const {
  if (Id.empty()) {
    std::stringstream M;
    M << "Invalid (empty) Identifier";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return static_cast<unsigned>(~0x0);
  }

  std::vector<ASTSymbolTableEntry *> VSTE =
      ASTSymbolTable::Instance().LookupRange(Id);

  if (VSTE.empty()) {
    std::stringstream M;
    M << "Identifier " << Id << " does not have a SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return static_cast<unsigned>(~0x0);
  }

  ASTSymbolTableEntry *STE = nullptr;

  for (std::vector<ASTSymbolTableEntry *>::iterator I = VSTE.begin();
       I != VSTE.end(); ++I) {
    switch ((*I)->GetValueType()) {
    case ASTTypeInt:
    case ASTTypeMPInteger:
    case ASTTypeBinaryOp:
    case ASTTypeUnaryOp:
    case ASTTypeUndefined: {
      STE = *I;
      ASTIdentifierNode *SId = STE->GetIdentifier();
      if (ASTIdentifierNode::InvalidBits(SId->GetBits()))
        SId->SetBits(32);
      break;
      break;
    }
    default:
      break;
    }
  }

  if (!STE) {
    std::stringstream M;
    M << "No Integer Constant Expression Entry was found in the SymbolTable "
      << "for Identifier " << Id << ".";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return static_cast<unsigned>(~0x0);
  }

  switch (STE->GetValueType()) {
  case ASTTypeInt: {
    ASTIntNode *I = STE->GetValue()->GetValue<ASTIntNode *>();
    assert(I && "Invalid ASTIntNode obtained from the SymbolTable!");

    return I->IsSigned() ? static_cast<unsigned>(I->GetSignedValue())
                         : I->GetUnsignedValue();
  } break;
  case ASTTypeMPInteger: {
    ASTMPIntegerNode *MPI = STE->GetValue()->GetValue<ASTMPIntegerNode *>();
    assert(MPI && "Invalid ASTMPIntegerNode obtained from the SymbolTable!");

    return MPI->IsSigned() ? static_cast<unsigned>(MPI->ToSignedInt())
                           : MPI->ToUnsignedInt();
  } break;
  default:
    break;
  }

  return static_cast<unsigned>(~0x0);
}

unsigned
ASTIdentifierIndexResolver::ResolveIndex(const ASTIdentifierNode *Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (const ASTIdentifierRefNode *IdR =
          dynamic_cast<const ASTIdentifierRefNode *>(Id)) {
    return IdR->GetIndex();
  } else if (Id->HasSymbolTableEntry()) {
    if (Id->GetSymbolTableEntry()->HasValue() &&
        Id->GetSymbolTableEntry()->GetValueType() != ASTTypeUndefined)
      return Id->GetBits();
  }

  return ResolveIndex(Id->GetName());
}

} // namespace QASM
