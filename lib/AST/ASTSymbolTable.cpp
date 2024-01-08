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

#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTSymbolTable.h>

#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

std::multimap<std::string, ASTSymbolTableEntry *> ASTSymbolTable::STM;
std::map<std::string, ASTSymbolTableEntry *> ASTSymbolTable::ASTM;
std::map<std::string, ASTSymbolTableEntry *> ASTSymbolTable::QSTM;

std::map<std::string, ASTSymbolTableEntry *> ASTSymbolTable::GSTM;
std::map<std::string, ASTSymbolTableEntry *> ASTSymbolTable::SGSTM;
std::map<uint64_t, ASTSymbolTableEntry *> ASTSymbolTable::HGSTM;

std::map<std::string, ASTSymbolTableEntry *> ASTSymbolTable::GPSTM;

std::map<std::string, ASTSymbolTableEntry *> ASTSymbolTable::DSTM;
std::map<std::string, ASTSymbolTableEntry *> ASTSymbolTable::SDSTM;
std::map<uint64_t, ASTSymbolTableEntry *> ASTSymbolTable::HDSTM;

std::map<std::string, ASTSymbolTableEntry *> ASTSymbolTable::FSTM;
std::map<std::string, ASTSymbolTableEntry *> ASTSymbolTable::SFSTM;
std::map<uint64_t, ASTSymbolTableEntry *> ASTSymbolTable::HFSTM;

std::map<std::string, ASTSymbolTableEntry *> ASTSymbolTable::CSTM;

std::map<std::string, ASTSymbolTableEntry *> ASTSymbolTable::GLSTM;
std::map<std::string, ASTSymbolTableEntry *> ASTSymbolTable::LSTM;
std::map<std::string, ASTSymbolTableEntry *> ASTSymbolTable::USTM;

ASTSymbolTable ASTSymbolTable::ST;

ASTMapSymbolTableEntry *
ASTSymbolTable::CreateDefcalGroup(const std::string &Id) {
  assert(!Id.empty() && "Invalid defcal group identifier argument!");

  if (ASTStringUtils::Instance().IsMangled(Id)) {
    std::string M = "A defcal group cannot have a mangled identifier.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M, DiagLevel::Error);
    return nullptr;
  }

  map_iterator DMI = DSTM.find(Id);
  if (DMI == DSTM.end()) {
    ASTIdentifierNode *DId = ASTBuilder::Instance().CreateASTIdentifierNode(
        Id, ASTDefcalNode::DefcalBits, ASTTypeDefcalGroup);
    assert(DId && "Could not create a valid ASTIdentifierNode!");

    ASTMapSymbolTableEntry *MSTE =
        dynamic_cast<ASTMapSymbolTableEntry *>(DId->GetSymbolTableEntry());
    assert(MSTE && "Could not create a valid ASTMapSymbolTableEntry!");
    assert(MSTE->GetValueType() == ASTTypeDefcalGroup &&
           "Invalid ASTSymbolTableEntry Type created for ASTDefcalGroup!");

    DMI = DSTM.find(Id);
  } else if ((*DMI).second->GetValueType() != ASTTypeDefcalGroup &&
             !(*DMI).second->HasMap()) {
    DSTM.erase(DMI);
    ASTIdentifierNode *DId = ASTBuilder::Instance().CreateASTIdentifierNode(
        Id, ASTDefcalNode::DefcalBits, ASTTypeDefcalGroup);
    assert(DId && "Could not create a valid ASTIdentifierNode!");

    ASTMapSymbolTableEntry *MSTE =
        dynamic_cast<ASTMapSymbolTableEntry *>(DId->GetSymbolTableEntry());
    assert(MSTE && "Could not create a valid ASTMapSymbolTableEntry!");
    assert(MSTE->GetValueType() == ASTTypeDefcalGroup &&
           "Invalid ASTSymbolTableEntry Type created for ASTDefcalGroup!");

    map_iterator LI = LSTM.find(Id);
    if (LI != LSTM.end())
      LSTM.erase(LI);

    DMI = DSTM.find(Id);
  }

  return dynamic_cast<ASTMapSymbolTableEntry *>((*DMI).second);
}

void ASTSymbolTable::Release() {
  for (map_iterator MI = ASTM.begin(); MI != ASTM.end(); ++MI) {
    if (!ASTStringUtils::Instance().IsIndexed((*MI).first) &&
        !IsComplexPart((*MI).first)) {
      delete (*MI).second;
    }
  }

  for (map_iterator MI = QSTM.begin(); MI != QSTM.end(); ++MI) {
    if (!ASTStringUtils::Instance().IsIndexed((*MI).first) &&
        !IsComplexPart((*MI).first)) {
      delete (*MI).second;
    }
  }

  for (map_iterator MI = GSTM.begin(); MI != GSTM.end(); ++MI) {
    if (!ASTStringUtils::Instance().IsIndexed((*MI).first) &&
        !IsComplexPart((*MI).first)) {
      delete (*MI).second;
    }
  }

  for (map_iterator MI = DSTM.begin(); MI != DSTM.end(); ++MI) {
    std::map<uint64_t, ASTSymbolTableEntry *> &MM =
        dynamic_cast<ASTMapSymbolTableEntry *>((*MI).second)->GetMap();

    for (ASTMapSymbolTableEntry::map_iterator MMI = MM.begin(); MMI != MM.end();
         ++MMI) {
      if (!ASTStringUtils::Instance().IsIndexed((*MI).first))
        delete (*MMI).second;
    }

    if (!IsComplexPart((*MI).first))
      delete (*MI).second;
  }

  for (map_iterator MI = FSTM.begin(); MI != FSTM.end(); ++MI) {
    if (!ASTStringUtils::Instance().IsIndexed((*MI).first) &&
        !IsComplexPart((*MI).first)) {
      delete (*MI).second;
    }
  }

  for (map_iterator MI = CSTM.begin(); MI != CSTM.end(); ++MI) {
    if (!ASTStringUtils::Instance().IsIndexed((*MI).first) &&
        !IsComplexPart((*MI).first)) {
      delete (*MI).second;
    }
  }

  for (map_iterator MI = GLSTM.begin(); MI != GLSTM.end(); ++MI) {
    if (!ASTStringUtils::Instance().IsIndexed((*MI).first) &&
        !IsComplexPart((*MI).first)) {
      delete (*MI).second;
    }
  }

  for (map_iterator MI = LSTM.begin(); MI != LSTM.end(); ++MI) {
    if (!ASTStringUtils::Instance().IsIndexed((*MI).first) &&
        !IsComplexPart((*MI).first)) {
      delete (*MI).second;
    }
  }

  for (map_iterator MI = USTM.begin(); MI != USTM.end(); ++MI) {
    if (!ASTStringUtils::Instance().IsIndexed((*MI).first) &&
        !IsComplexPart((*MI).first)) {
      delete (*MI).second;
    }
  }

  for (multimap_iterator MMI = STM.begin(); MMI != STM.end(); ++MMI) {
    if (!ASTStringUtils::Instance().IsIndexed((*MMI).first) &&
        !IsComplexPart((*MMI).first)) {
      delete (*MMI).second;
    }
  }
}

} // namespace QASM
