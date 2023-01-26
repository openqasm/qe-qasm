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

#include <qasm/AST/ASTGateQubitParamBuilder.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTSymbolTable.h>

namespace QASM {

ASTIdentifierList ASTGateQubitParamBuilder::IL;
ASTGateQubitParamBuilder ASTGateQubitParamBuilder::GQB;
ASTIdentifierList* ASTGateQubitParamBuilder::ILP;
std::vector<ASTIdentifierList*> ASTGateQubitParamBuilder::ILV;

void ASTGateQubitParamBuilder::ReleaseQubits() {
  for (ASTIdentifierList::const_iterator I = ILP->begin(); I != ILP->end(); ++I) {
    const ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().LookupLocal(*I);
    if (STE && (*I)->IsGateLocal() && (*I)->IsLocalScope()) {
      switch (STE->GetValueType()) {
      default:
        break;
      case ASTTypeGateQubitParam:
        ASTSymbolTable::Instance().EraseLocalQubit((*I)->GetName());
        break;
      }
    }
  }
}

void ASTGateQubitParamBuilder::SetGateLocal() {
  for (ASTIdentifierList::iterator I = ILP->begin(); I != ILP->end(); ++I) {
    ASTSymbolTableEntry* STE =
      const_cast<ASTSymbolTableEntry*>(ASTSymbolTable::Instance().LookupLocal(*I));
    if (STE) {
      switch (STE->GetValueType()) {
      default:
        break;
      case ASTTypeUndefined:
      case ASTTypeQubit:
      case ASTTypeQubitContainer:
        (*I)->SetBits(1);
        (*I)->SetGateLocal();
        (*I)->SetLocalScope();
        (*I)->SetSymbolType(ASTTypeGateQubitParam);
        STE->SetLocalScope();
        STE->SetValueType(ASTTypeGateQubitParam);
        break;
      }
    }
  }
}

} // namespace QASM

