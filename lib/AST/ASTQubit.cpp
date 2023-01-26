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

#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <sstream>
#include <vector>
#include <string>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

unsigned ASTQubitContainerNode::AliasIndex = 0U;

ASTBoundQubitListBuilder ASTBoundQubitListBuilder::BQB;
ASTBoundQubitList ASTBoundQubitListBuilder::BQL;
ASTBoundQubitList* ASTBoundQubitListBuilder::BQP = nullptr;
std::vector<ASTBoundQubitList*> ASTBoundQubitListBuilder::BQV;

void ASTQubitNode::print() const {
  std::cout << "<Qubit>" << std::endl;
  std::cout << "<Identifier>" << GetName()
    << "</Identifier>" << std::endl;
  std::cout << "<MangledName>" << GetMangledName()
    << "</MangledName>" << std::endl;
  if (!GQN.empty())
    std::cout << "<GateQubitName>" << GQN << "</GateQubitName>" << std::endl;
  std::cout << "<Index>" << Index << "</Index>" << std::endl;
  std::cout << "</Qubit>" << std::endl;
}

static bool FillRangeQubitVector(const std::vector<int32_t>& IV,
                                 const std::vector<ASTQubitNode*>& QV,
                                 std::vector<ASTQubitNode*>& AV) {
  assert(!IV.empty() && "Invalid - empty - Index Vector argument!");
  assert(!QV.empty() && "Invalid - empty - Qubit Vector argument!");

  int S, B, E;

  switch (IV.size()) {
  case 2:
    S = 1;
    B = IV[0];
    E = IV[1];
    break;
  case 3:
    S = IV[1];
    B = IV[0];
    E = IV[2];
    assert(S > 0 && "Negative or zero stepping is not allowed!");

    if (S <= 0) {
      std::stringstream M;
      M << " Negative or zero stepping is not allowed.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return false;
    }
    break;
  default: {
    std::stringstream M;
    M << "Incomprehensible qubit alias vector initialization method.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return false;
  }
    break;
  }

  if (B == 0 && E == 0) {
    std::stringstream M;
    M << "Useless range [0,0).";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return false;
  }

  using vector_type = std::vector<ASTQubitNode*>;

  if (B >= 0 && E >= 0) {
    if (B >= E) {
      std::stringstream M;
      M << "Begin cannot be past or same as End.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return false;
    }

    vector_type::const_iterator BI = QV.begin() + B;
    vector_type::const_iterator EI = QV.begin() + E;
    AV.clear();

    for (vector_type::const_iterator I = BI; I <= EI; I += S)
      AV.push_back(*I);
  } else if (B < 0 && E >= 0) {
    std::stringstream M;
    M << "Invalid range [begin, end). Begin and end must "
      << "have the same sign.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return false;
  } else if (B >= 0 && E < 0) {
    std::stringstream M;
    M << "Invalid range [begin, end). Begin and end must "
      << "have the same sign.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return false;
  } else {
    if (std::abs(B) > static_cast<int32_t>(QV.size())) {
      std::stringstream M;
      M << "Reverse begin iterator points past the beginning of the vector.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return false;
    }

    vector_type::const_iterator BI = QV.end() + B + 1;
    vector_type::const_iterator EI = QV.end() + E + 1;
    AV.clear();

    for (vector_type::const_iterator I = BI; I < EI; I += S) {
      AV.push_back(*I);
    }
  }

  return true;
}

static bool
FillSequenceQubitVector(const std::vector<int32_t>& IV,
                        const std::vector<ASTQubitNode*>& QV,
                        std::vector<ASTQubitNode*>& AV) {
  assert(!IV.empty() && "Invalid - empty - Index Vector argument!");
  assert(!QV.empty() && "Invalid - empty - Qubit Vector argument!");

  AV.clear();

  for (std::vector<int>::const_iterator I = IV.begin(); I != IV.end(); ++I)
    AV.push_back(QV[*I]);

  return true;
}

void
ASTQubitContainerNode::EraseFromLocalSymbolTable() {
  if (!List.empty()) {

    const std::string& QNS = GetIdentifier()->GetName();
    std::stringstream QSS;

    for (std::vector<ASTQubitNode*>::iterator QI = List.begin();
         QI != List.end(); ++QI) {
      ASTSymbolTable::Instance().EraseLocalQubit(QNS);
    }
  }
}

void
ASTQubitContainerNode::LocalFunctionArgument() {
  if (!List.empty()) {
    unsigned LS = Size();
    std::stringstream QSS;
    const std::string& QN = GetIdentifier()->GetName();

    for (std::vector<ASTQubitNode*>::iterator QI = List.begin();
         QI != List.end(); ++QI) {
      ASTSymbolTable::Instance().TransferQubitToLSTM((*QI)->GetIdentifier(),
                                                     (*QI)->GetIdentifier()->GetBits(),
                                                     (*QI)->GetASTType());
    }

    for (unsigned I = 0; I < LS; ++I) {
      QSS.str("");
      QSS.clear();
      QSS << QN << '[' << I << ']';

      ASTSymbolTable::Instance().TransferQubitToLSTM(QSS.str(), 1U, ASTTypeQubit);

      QSS.str("");
      QSS.clear();
      QSS << '%' << GetIdentifier()->GetName() << ':' << I;
      ASTSymbolTable::Instance().TransferQubitToLSTM(QSS.str(), 1U, ASTTypeQubit);
    }
  }

  ASTSymbolTable::Instance().TransferQubitToLSTM(GetIdentifier(), Size(),
                                                 GetASTType());
}

ASTQubitContainerAliasNode*
ASTQubitContainerNode::CreateAlias(const ASTIdentifierNode* AId,
                                   const ASTIntegerList& IL,
                                   char M) const {
  assert((M == ':' || M == ',') && "Invalid Qubit Alias construction Method!");

  std::vector<ASTQubitNode*> AV;

  if (M == ':') {
    switch (IL.Size()) {
    case 2:
    case 3:
      if (!FillRangeQubitVector(IL.List, List, AV)) {
        std::stringstream MM;
        MM << "Could not fill a range-based Alias Vector.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(AId), MM.str(), DiagLevel::Error);
        return nullptr;
      }
      break;
    default: {
      std::stringstream MM;
      MM << "Invalid Qubit Alias vector initialization method.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(AId), MM.str(), DiagLevel::Error);
      return nullptr;
    }
      break;
    }
  } else if (M == ',') {
    if (!FillSequenceQubitVector(IL.List, List, AV)) {
      std::stringstream MM;
      MM << "Could not fill a sequence-based Alias Vector.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(AId), MM.str(), DiagLevel::Error);
      return nullptr;
    }
  }

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(AId);
  assert(STE && "Resulting QubitContainer Alias has no SymbolTable Entry!");

  AId->SetBits(AV.size());
  STE->GetIdentifier()->SetBits(AV.size());

  ASTQubitContainerAliasNode* CAN =
    new ASTQubitContainerAliasNode(AId, Ident, AliasIndex++, AV);
  assert(CAN && "Could not create a valid ASTQubitContainerAliasNode!");
  return CAN;
}

ASTQubitContainerAliasNode*
ASTQubitContainerAliasNode::CreateAlias(const ASTIdentifierNode* AId,
                                        const ASTIntegerList& IL,
                                        char M) const {
  assert((M == ':' || M == ',') &&
         "Invalid Qubit Alias construction Method!");

  std::vector<ASTQubitNode*> AV;

  if (M == ':') {
    switch (IL.Size()) {
    case 2:
    case 3:
      if (!FillRangeQubitVector(IL.List, QAL, AV)) {
        std::stringstream MM;
        MM << "Could not fill a range-based Alias Vector.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(AId), MM.str(), DiagLevel::Error);
        return nullptr;
      }
      break;
    default: {
      std::stringstream MM;
      MM << "Invalid Qubit Alias Vector initialization method.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(AId), MM.str(), DiagLevel::Error);
      return nullptr;
    }
      break;
    }
  } else if (M == ',') {
    if (!FillSequenceQubitVector(IL.List, QAL, AV)) {
      std::stringstream MM;
      MM << "Could not fill a sequence-based Alias Vector.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(AId), MM.str(), DiagLevel::Error);
      return nullptr;
    }
  }

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(AId);
  assert(STE && "Resulting QubitContainer Alias has no SymbolTable Entry!");

  AId->SetBits(AV.size());
  STE->GetIdentifier()->SetBits(AV.size());

  ASTQubitContainerAliasNode* CAN =
    new ASTQubitContainerAliasNode(AId, Ident, AliasIndex++, AV);
  assert(CAN && "Could not create a valid ASTQubitContainerAliasNode!");
  return CAN;
}

ASTQubitContainerAliasNode*
ASTQubitContainerNode::CreateAlias(const ASTIdentifierNode* AId,
                                   int32_t Index) const {
  std::vector<ASTQubitNode*> AV;

  if (Index < 0) {
    list_type::const_iterator I = List.end() + Index;
    AV.push_back(*I);
  } else {
    list_type::const_iterator I = List.begin() + Index;
    AV.push_back(*I);
  }

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(AId);
  assert(STE && "Resulting QubitContainer Alias has no SymbolTable Entry!");

  AId->SetBits(AV.size());
  STE->GetIdentifier()->SetBits(AV.size());

  ASTQubitContainerAliasNode* CAN =
    new ASTQubitContainerAliasNode(AId, Ident, AliasIndex++, AV);
  assert(CAN && "Could not create a valid ASTQubitContainerAliasNode!");
  return CAN;
}

ASTQubitContainerAliasNode*
ASTQubitContainerAliasNode::CreateAlias(const ASTIdentifierNode* AId,
                                        int32_t Index) const {
  std::vector<ASTQubitNode*> AV;

  if (Index < 0) {
    list_type::const_iterator I = QAL.end() + Index;
    AV.push_back(*I);
  } else {
    list_type::const_iterator I = QAL.begin() + Index;
    AV.push_back(*I);
  }

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(AId);
  assert(STE && "Resulting QubitContainer Alias has no SymbolTable Entry!");

  AId->SetBits(AV.size());
  STE->GetIdentifier()->SetBits(AV.size());

  ASTQubitContainerAliasNode* CAN =
    new ASTQubitContainerAliasNode(AId, Ident, AliasIndex++, AV);
  assert(CAN && "Could not create a valid ASTQubitContainerAliasNode!");
  return CAN;
}

ASTQubitContainerAliasNode*
ASTQubitContainerNode::CreateAlias(const ASTIdentifierNode* AId) const {
  std::vector<ASTQubitNode*> AV;

  ASTQubitContainerAliasNode* CAN =
    new ASTQubitContainerAliasNode(AId, Ident, AliasIndex++, AV);
  assert(CAN && "Could not create a valid ASTQubitContainerAliasNode!");

  CAN->QAL = List;
  CAN->Bits = Bits;

  return CAN;
}

ASTQubitContainerAliasNode*
ASTQubitContainerAliasNode::CreateAlias(const ASTIdentifierNode* AId) const {
  std::vector<ASTQubitNode*> AV;

  ASTQubitContainerAliasNode* CAN =
    new ASTQubitContainerAliasNode(AId, Ident, AliasIndex++, AV);
  assert(CAN && "Could not create a valid ASTQubitContainerAliasNode!");

  CAN->QAL = QAL;
  CAN->Bits = Bits;
  CAN->IdAliasList = IdAliasList;
  IdAliasList.push_back(AId);

  return CAN;
}

void
ASTQubitContainerAliasNode::operator+=(const ASTQubitContainerAliasNode& Op) {
  unsigned Size = QAL.size() + Op.Size();
  QAL.insert(QAL.end(), Op.begin(), Op.end());

  const ASTIdentifierNode* AId = GetIdentifier();
  assert(AId && "Invalid QubitContainer ASTIdentifierNode!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(AId);
  assert(STE && "QubitContainer has no SymbolTable Entry!");

  AId->SetBits(Size);
  STE->GetIdentifier()->SetBits(Size);
}

void
ASTQubitContainerAliasNode::operator+=(const ASTQubitContainerNode& Op) {
  unsigned Size = QAL.size() + Op.Size();
  QAL.insert(QAL.end(), Op.begin(), Op.end());

  const ASTIdentifierNode* AId = GetIdentifier();
  assert(AId && "Invalid QubitContainer ASTIdentifierNode!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(AId);
  assert(STE && "QubitContainer has no SymbolTable Entry!");

  AId->SetBits(Size);
  STE->GetIdentifier()->SetBits(Size);
}

void ASTQubitNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (!GetIdentifier()->GetGateParamName().empty()) {
    if (ASTStringUtils::Instance().IsIndexedQubit(
                                   GetIdentifier()->GetPolymorphicName()))
      M.TypeIdentifier(GetASTType(),
                       ASTStringUtils::Instance().BracketedQubit(
                                       GetIdentifier()->GetGateParamName()));
    else
      M.TypeIdentifier(GetASTType(), GetIdentifier()->GetGateParamName());
  } else {
    if (ASTStringUtils::Instance().IsIndexedQubit(GetName()))
      M.TypeIdentifier(GetASTType(),
                       ASTStringUtils::Instance().BracketedQubit(GetName()));
    else
      M.TypeIdentifier(GetASTType(), GetName());
  }

  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTQubitContainerNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (!GetIdentifier()->GetGateParamName().empty())
    M.TypeIdentifier(GetASTType(), Size(), GetIdentifier()->GetGateParamName());
  else
    M.TypeIdentifier(GetASTType(), Size(), GetName());

  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTQubitContainerAliasNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (!GetIdentifier()->GetGateParamName().empty())
    M.TypeIdentifier(GetASTType(), Size(), GetIdentifier()->GetGateParamName());
  else
    M.TypeIdentifier(GetASTType(), Size(), GetName());

  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTGateQubitParamNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetBits(), GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace QASM

