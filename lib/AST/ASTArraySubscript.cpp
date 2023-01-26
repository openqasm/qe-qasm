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

#include <qasm/AST/ASTArraySubscript.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <sstream>
#include <climits>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

std::string
ASTArraySubscriptNode::AsIndexedString() const {
  std::stringstream S;
  S << '[' << IX << ']';
  return S.str();
}

unsigned ASTArraySubscriptNode::GetIdentifierIndexValue() const {
  const ASTIdentifierNode* IId = ID;
  assert(IId && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* ISTE = ASTSymbolTable::Instance().Lookup(IId);
  assert(ISTE && "Index ASTIdentifierNode has no SymbolTable Entry!");

  if (ISTE->GetValueType() == ASTTypeUndefined || !ISTE->HasValue()) {
    ASTIntNode* I = ASTBuilder::Instance().CreateASTIntNode(IId, int32_t(0));
    assert(I && "Could not create a valid ASTIntNode!");
    IId->SetBits(ASTIntNode::IntBits);
    ISTE = ASTSymbolTable::Instance().Lookup(IId);
  }

  if (ISTE->GetValueType() != ASTTypeInt &&
      ISTE->GetValueType() != ASTTypeMPInteger) {
    std::stringstream M;
    M << "Invalid SymbolTableEntry Type for Index " << IId->GetName() << "!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return static_cast<unsigned>(~0x0);
  }

  unsigned Index = static_cast<unsigned>(~0x0);

  // Do NOT use the Identifier Index Resolver here.
  // The semantics of the number of bits is context-dependent.
  if (ISTE->GetValueType() == ASTTypeInt) {
    ASTIntNode* Int = ISTE->GetValue()->GetValue<ASTIntNode*>();
    assert(Int && "Could not retrieve a valid Index ASTIntNode!");
    Index = Int->IsSigned() ? static_cast<unsigned>(Int->GetSignedValue()) :
                              Int->GetUnsignedValue();
  } else {
    ASTMPIntegerNode* MPII = ISTE->GetValue()->GetValue<ASTMPIntegerNode*>();
    assert(MPII && "Could not retrieve a valid Index ASTMPIntegerNode!");
    Index = MPII->IsSigned() ?
            static_cast<unsigned>(MPII->ToSignedInt()) :
            MPII->ToUnsignedInt();
  }

  assert(Index != static_cast<unsigned>(~0x0) &&
         "Invalid number of bits for Index Identifier!");

  return Index;
}

int32_t ASTArraySubscriptNode::GetSignedIndexValue() const {
  bool IWNS = false;

  switch (EType) {
  case ASTTypeInt:
  case ASTTypeMPInteger:
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
    return IX;
    break;
  case ASTTypeBinaryOp:
  case ASTTypeUnaryOp:
  case ASTTypeExpression:
    IWNS = true;
    break;
  default:
    IWNS = true;
    break;
  }

  if (IWNS) {
    std::stringstream M;
    M << "Expression of type " << PrintTypeEnum(EType)
      << " does not evaluate to an Integer Constant Expression.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Error);
  }

  return static_cast<int32_t>(INT_MAX);
}

uint32_t ASTArraySubscriptNode::GetUnsignedIndexValue() const {
  bool IWNS = false;

  switch (EType) {
  case ASTTypeInt:
  case ASTTypeMPInteger:
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
    return static_cast<uint32_t>(IX);
    break;
  case ASTTypeBinaryOp:
  case ASTTypeUnaryOp:
  case ASTTypeExpression:
    IWNS = true;
    break;
  default:
    IWNS = true;
    break;
  }

  if (IWNS) {
    std::stringstream M;
    M << "Expression of type " << PrintTypeEnum(EType)
      << " does not evaluate to an Integer Constant Expression.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Error);
  }

  return static_cast<uint32_t>(UINT_MAX);
}

std::string ASTArraySubscriptList::AsIndexedString() const {
  if (SV.empty())
    return "";

  std::stringstream IS;

  for (ASTArraySubscriptList::const_iterator I = SV.begin();
       I != SV.end(); ++I) {
    IS << '[' << (*I)->IX << ']';
  }

  return IS.str();
}

void ASTArraySubscriptList::Append(const ASTArraySubscriptNode* ASN) {
  assert(ASN && "Invalid ASTArraySubscriptNode argument!");

  switch (ASN->GetExpressionType()) {
  case ASTTypeInt:
  case ASTTypeMPInteger:
  case ASTTypeBinaryOp:
  case ASTTypeUnaryOp:
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
  case ASTTypeArraySubscriptList:
    SV.push_back(ASN);
    return;
    break;
  default:
    break;
  }

  std::stringstream M;
  M << "Expression of type " << PrintTypeEnum(ASN->GetExpressionType())
    << " does not evaluate to an Integer Constant Expression.";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
    DIAGLineCounter::Instance().GetLocation(ASN), M.str(),
                                                  DiagLevel::Error);
}

void ASTArraySubscriptList::Prepend(const ASTArraySubscriptNode* ASN) {
  assert(ASN && "Invalid ASTArraySubscriptNode argument!");

  switch (ASN->GetExpressionType()) {
  case ASTTypeInt:
  case ASTTypeMPInteger:
  case ASTTypeBinaryOp:
  case ASTTypeUnaryOp:
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef:
  case ASTTypeArraySubscriptList:
    SV.insert(SV.begin(), ASN);
    return;
    break;
  default:
    break;
  }

  std::stringstream M;
  M << "Expression of type " << PrintTypeEnum(ASN->GetExpressionType())
    << " does not evaluate to an Integer Constant Expression.";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
    DIAGLineCounter::Instance().GetLocation(ASN), M.str(),
                                                  DiagLevel::Error);
}

} // namespace QASM

