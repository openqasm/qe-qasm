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

#include <qasm/AST/ASTTypeDiscovery.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTIdentifierBuilder.h>
#include <qasm/AST/ASTIdentifierTypeController.h>
#include <qasm/AST/ASTDeclarationBuilder.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTGateContextBuilder.h>
#include <qasm/AST/ASTDefcalContextBuilder.h>
#include <qasm/AST/ASTKernelContextBuilder.h>
#include <qasm/AST/ASTPragmaContextBuilder.h>
#include <qasm/AST/ASTCalContextBuilder.h>
#include <qasm/AST/ASTFunctionContextBuilder.h>
#include <qasm/AST/ASTKernelContextBuilder.h>
#include <qasm/AST/ASTAngleContextControl.h>
#include <qasm/AST/ASTRedeclarationController.h>
#include <qasm/AST/ASTOpenQASMVersionTracker.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTUtils.h>
#include <qasm/AST/ASTBraceMatcher.h>
#include <qasm/Diagnostic/DIAGLineBuffer.h>
#include <QasmParser.tab.h>

#include <iostream>
#include <sstream>
#include <cassert>

namespace QASM {

ASTTypeDiscovery ASTTypeDiscovery::TD;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTIdentifierNode*
ASTTypeDiscovery::CreateReservedFPConstantId(const std::string& N,
                                             const ASTDeclarationContext* DCX,
                                             const ASTToken* TK) const {
  assert(!N.empty() && "Invalid floating-point constant keyword!");
  assert(DCX && "Invalid ASTDeclarationContext argument!");
  assert(TK && "Invalid ASTToken argument!");

  if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(N)) {
    std::stringstream M;
    M << "Identifier " << N << " is not a reserved keyword.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::Error);
    return nullptr;
  }

  if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DCX)) {
    const ASTSymbolTableEntry* GSTE =
      ASTSymbolTable::Instance().FindGlobalSymbol(N, ASTDoubleNode::DoubleBits,
                                                  ASTTypeDouble);
    if (GSTE)
      return const_cast<ASTIdentifierNode*>(GSTE->GetIdentifier());

    GSTE = ASTSymbolTable::Instance().FindGlobal(N);

    if (GSTE && GSTE->GetValueType() != ASTTypeDouble &&
        GSTE->GetIdentifier()->GetSymbolType() != ASTTypeDouble &&
        GSTE->GetIdentifier()->GetBits() != ASTDoubleNode::DoubleBits) {
      std::stringstream M;
      M << "Identifier " << N << " already exists in the Global Symbol Table "
        << "with a different type and size: "
        << PrintTypeEnum(GSTE->GetValueType()) << '/'
        << GSTE->GetIdentifier()->GetBits() << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::Error);
      return nullptr;
    } else {
      return const_cast<ASTIdentifierNode*>(GSTE->GetIdentifier());
    }
  } else {
    const ASTSymbolTableEntry* LSTE =
      ASTSymbolTable::Instance().FindLocalSymbol(N, ASTDoubleNode::DoubleBits,
                                                 ASTTypeDouble);
    if (LSTE && LSTE->GetValueType() == ASTTypeDouble &&
        LSTE->GetIdentifier()->GetSymbolType() == ASTTypeDouble &&
        LSTE->GetIdentifier()->GetBits() == ASTDoubleNode::DoubleBits &&
        LSTE->GetIdentifier()->GetDeclarationContext() == DCX &&
        LSTE->GetContext() == DCX)
      return const_cast<ASTIdentifierNode*>(LSTE->GetIdentifier());

    LSTE = ASTSymbolTable::Instance().FindLocal(N);

    if (LSTE && LSTE->GetIdentifier()->GetSymbolType() != ASTTypeDouble &&
        LSTE->GetIdentifier()->GetBits() != ASTDoubleNode::DoubleBits) {
      std::stringstream M;
      M << "Identifier " << N << " already exists in the Local Symbol Table "
        << "with a different type and size ("
        << PrintTypeEnum(LSTE->GetValueType()) << '/'
        << LSTE->GetIdentifier()->GetBits() << ").";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::Error);
      return nullptr;
    }
  }

  ASTDoubleNode* DN = nullptr;

  if (N == u8"pi" || N == u8"π")
    DN = ASTDoubleNode::Pi();
  else if (N == u8"tau" || N == u8"τ")
    DN = ASTDoubleNode::Tau();
  else if (N == u8"euler" || N == u8"ε")
    DN = ASTDoubleNode::Euler();
  else if (N == u8"-pi" || N == u8"-π")
    DN = ASTDoubleNode::NegPi();
  else if (N == u8"-tau" || N == u8"-τ")
    DN = ASTDoubleNode::NegTau();
  else if (N == u8"-euler" || N == u8"-ε")
    DN = ASTDoubleNode::NegEuler();
  else {
    std::stringstream M;
    M << "Unknown floating-point constant " << N << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::Error);
    return nullptr;
  }

  assert(DN && "Could not create a valid ASTDoubleNode!");

  ASTIdentifierNode* IDN =
    ASTBuilder::Instance().CreateLocalScopeASTIdentifierNode(
                                      N, ASTDoubleNode::DoubleBits,
                                      ASTTypeDouble, DCX, TK);
  assert(IDN && "Could not create a valid ASTIdentifierNode!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().LookupLocal(IDN->GetName(), IDN->GetBits(),
                                           IDN->GetSymbolType());
  assert(STE && "ASTIdentifierNode has no SymbolTable Entry!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDouble), ASTTypeDouble);

  IDN->SetDeclarationContext(DCX);
  IDN->SetLocalScope();
  STE->SetContext(DCX);
  STE->SetLocalScope();
  IDN->SetSymbolTableEntry(STE);
  return IDN;
}

bool
ASTTypeDiscovery::InReDeclarationContext(const ASTIdentifierNode* Id, ASTType CTy,
                                         const ASTDeclarationContext* DCX) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DCX && "Invalid ASTDeclarationContext argument!");

  if (ASTIdentifierTypeController::Instance().SeenLBracket() &&
      ASTIdentifierTypeController::Instance().SeenRBracket() &&
      ASTExpressionValidator::Instance().IsSimpleIntegerType(Id->GetSymbolType()))
    return false;

  if (!ASTIdentifierTypeController::Instance().SeenLBracket() &&
      !ASTIdentifierTypeController::Instance().SeenRBracket() &&
      ASTIdentifierTypeController::Instance().SeenLParen() &&
      ASTExpressionValidator::Instance().IsIntegerType(Id->GetSymbolType())) {
    switch (DCX->GetContextType()) {
    case ASTTypeIfStatement:
    case ASTTypeElseIfStatement:
    case ASTTypeElseStatement:
    case ASTTypeWhileStatement:
    case ASTTypeSwitchStatement:
      return false;
      break;
    default:
      break;
    }
  }

  if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DCX) ||
      CTy == ASTTypeUndefined ||
      !ASTRedeclarationController::Instance().AllowsRedeclaration() ||
      !ASTRedeclarationController::Instance().TypeAllowsRedeclaration(CTy))
    return false;

  uint32_t DCIX;
  bool DAE = ASTDeclarationBuilder::Instance().ThisMayBeRedeclaration(Id, DCX,
                                                                      &DCIX);
  if (DCIX == DCX->GetIndex()) {
    const char* SC = DCIX == 0 ? "Global" : "Local";
    std::stringstream M;
    M << "Declaration " << Id->GetName() << " shadows a previous "
      << "declaration at " << SC << " Scope: (" << Id->GetName() << ", "
      << PrintTypeEnum(Id->GetSymbolType()) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return false;
  }

  bool SLB = false;

  switch (DCX->GetContextType()) {
  case ASTTypeIfStatement:
    SLB = (ASTIfBraceMatcher::Instance().GetLeft() >
           ASTIfBraceMatcher::Instance().GetRight()) ||
      ASTIfBraceMatcher::Instance().IsZero();
    break;
  case ASTTypeElseIfStatement:
    SLB = (ASTElseIfBraceMatcher::Instance().GetLeft() >
           ASTElseIfBraceMatcher::Instance().GetRight()) ||
      ASTElseIfBraceMatcher::Instance().IsZero();
    break;
  case ASTTypeElseStatement:
    SLB = (ASTElseBraceMatcher::Instance().GetLeft() >
           ASTElseBraceMatcher::Instance().GetRight()) ||
      ASTElseBraceMatcher::Instance().IsZero();
    break;
  case ASTTypeWhileStatement:
    SLB = (ASTWhileBraceMatcher::Instance().GetLeft() >
           ASTWhileBraceMatcher::Instance().GetRight()) ||
      ASTWhileBraceMatcher::Instance().IsZero();
    break;
  case ASTTypeForStatement:
    SLB = (ASTForBraceMatcher::Instance().GetLeft() >
           ASTForBraceMatcher::Instance().GetRight()) ||
      ASTForBraceMatcher::Instance().IsZero();
    break;
  case ASTTypeGate:
    SLB = (ASTGateBraceMatcher::Instance().GetLeft() >
           ASTGateBraceMatcher::Instance().GetRight()) ||
      ASTGateBraceMatcher::Instance().IsZero();
    break;
  case ASTTypeDefcal:
    SLB = (ASTDefcalBraceMatcher::Instance().GetLeft() >
           ASTDefcalBraceMatcher::Instance().GetRight()) ||
      ASTDefcalBraceMatcher::Instance().IsZero();
    break;
  case ASTTypeFunction:
    SLB = (ASTFunctionBraceMatcher::Instance().GetLeft() >
           ASTFunctionBraceMatcher::Instance().GetRight()) ||
      ASTFunctionBraceMatcher::Instance().IsZero();
    break;
  default:
    break;
  }

  return SLB && DAE;
}

bool
ASTTypeDiscovery::IsGateQubitParam(const ASTIdentifierNode* Id,
                                   ASTType CTy, ASTType PTy,
                                   const ASTDeclarationContext* DCX) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DCX && "Invalid ASTDeclarationContext argument!");

  if (DCX->GetContextType() != ASTTypeGate ||
      !ASTGateContextBuilder::Instance().InOpenContext())
    return false;

  if (ASTIdentifierTypeController::Instance().SeenLBrace() &&
      IsCallable(Id))
    return false;

  if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()))
    return false;

  if (ASTSymbolTable::Instance().FindLocal(Id)) {
    if (ASTIdentifierTypeController::Instance().SeenLBrace())
      return false;
  }

  if (PTy == ASTTypeGate &&
      ASTIdentifierTypeController::Instance().SeenLParen() &&
      ASTIdentifierTypeController::Instance().SeenRParen()) {
    uint32_t TIX = ASTTokenFactory::GetCurrentIndex() - 1;
    const ASTToken* ITK = ASTTokenFactory::GetToken(TIX);
    assert(ITK && "Could not obtain a valid ASTToken!");

    const std::string& TKS = ITK->GetString();

    if (TKS[0] == u8')')
      return false;

    if (TKS[0] == u8',' || TKS[0] == u8'{') {
      ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeGateQubitParam);
      return true;
    }
  }

  if ((CTy == ASTTypeUndefined && PTy == ASTTypeGate &&
       !ASTIdentifierTypeController::Instance().SeenLBrace()) &&
      (CTy == ASTTypeGateQubitParam &&
       (PTy == ASTTypeGateQubitParam || PTy == ASTTypeUndefined ||
        PTy == ASTTypeGate))) {
    ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeGateQubitParam);
    ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeGateQubitParam);
    return true;
  } else if (ASTIdentifierTypeController::Instance().SeenLBrace() &&
             ((CTy == ASTTypeUndefined && PTy == ASTTypeGate) ||
              (CTy == ASTTypeGateQubitParam && PTy == ASTTypeGateQubitParam))) {
    ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
    ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeGateQubitParam);
    return true;
  } else if (CTy == ASTTypeGateQubitParam && PTy == ASTTypeUndefined &&
             !ASTIdentifierTypeController::Instance().SeenLBrace()) {
    ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeGateQubitParam);
    ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeGateQubitParam);
    return true;
  }

  return false;
}

bool
ASTTypeDiscovery::IsGateAngleParam(const ASTIdentifierNode* Id,
                                   ASTType CTy, ASTType PTy,
                                   const ASTDeclarationContext* DCX) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DCX && "Invalid ASTDeclarationContext argument!");

  if (DCX->GetContextType() != ASTTypeGate ||
      !ASTGateContextBuilder::Instance().InOpenContext())
    return false;

  if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()))
    return false;

  if (ASTGateContextBuilder::Instance().InOpenContext() &&
      CTy == ASTTypeGate && PTy == ASTTypeUndefined)
    return false;

  if (ASTGateBraceMatcher::Instance().GetLeft())
    return false;

  if (!ASTIdentifierTypeController::Instance().SeenLParen())
    return false;

  if (ASTIdentifierTypeController::Instance().SeenLParen() &&
      ASTIdentifierTypeController::Instance().SeenRParen())
    return false;

  if (ASTIdentifierTypeController::Instance().SeenLParen() &&
      !ASTIdentifierTypeController::Instance().SeenRParen() &&
      (CTy == ASTTypeGate || CTy == ASTTypeUndefined)) {
    ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeAngle);
    ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeGate);
    return true;
  } else if (ASTIdentifierTypeController::Instance().SeenLParen() &&
             ASTIdentifierTypeController::Instance().SeenRParen() &&
             (CTy == ASTTypeGate || CTy == ASTTypeGateQubitParam ||
              PTy == ASTTypeGate || PTy == ASTTypeGateQubitParam)) {
    uint32_t TIX = ASTTokenFactory::GetCurrentIndex() - 1;
    const ASTToken* ITK = ASTTokenFactory::GetToken(TIX);
    assert(ITK && "Could not obtain a valid ASTToken!");

    const std::string& TKS = ITK->GetString();
    return TKS[0] == u8')' || TKS[0] == u8',';
  }

  return ASTGateContextBuilder::Instance().InOpenContext() &&
         !ASTIdentifierTypeController::Instance().SeenLBrace();
}

bool
ASTTypeDiscovery::IsGateAngleParam(const std::string& S,
                                   ASTType CTy, ASTType PTy,
                                   const ASTDeclarationContext* DCX) const {
  assert(!S.empty() && "Invalid std::string argument!");
  assert(DCX && "Invalid ASTDeclarationContext argument!");

  if (DCX->GetContextType() != ASTTypeGate ||
      !ASTGateContextBuilder::Instance().InOpenContext())
    return false;

  if (ASTGateContextBuilder::Instance().InOpenContext() &&
      CTy == ASTTypeGate && PTy == ASTTypeUndefined)
    return false;

  if (ASTGateBraceMatcher::Instance().GetLeft())
    return false;

  if (!ASTIdentifierTypeController::Instance().SeenLParen())
    return false;

  uint32_t TIX = ASTTokenFactory::GetCurrentIndex() - 1;
  const ASTToken* ITK = ASTTokenFactory::GetToken(TIX);
  assert(ITK && "Could not obtain a valid ASTToken!");

  const std::string& TKS = ITK->GetString();
  return ASTGateContextBuilder::Instance().InOpenContext() &&
         !ASTIdentifierTypeController::Instance().SeenLBrace() &&
         (TKS[0] == u8')' || TKS[0] == u8',');
}

ASTIdentifierNode*
ASTTypeDiscovery::CreateLocalASTIdentifierNodeRedeclaration(
                             const ASTIdentifierNode* Id,
                             unsigned Bits, ASTType Ty,
                             const ASTDeclarationContext* DCX,
                             const ASTToken* TK) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DCX && "Invalid ASTDeclarationContext argument!");
  assert(TK && "Invalid ASTToken argument!");

  bool W = false;

  switch (DCX->GetContextType()) {
  case ASTTypeIfStatement:
    if (ASTIfBraceMatcher::Instance().IsZero())
      W = true;
    break;
  case ASTTypeElseIfStatement:
    if (ASTElseIfBraceMatcher::Instance().IsZero())
      W = true;
    break;
  case ASTTypeElseStatement:
    if (ASTElseBraceMatcher::Instance().IsZero())
      W = true;
    break;
  case ASTTypeWhileStatement:
    if (ASTWhileBraceMatcher::Instance().IsZero())
      W = true;
    break;
  case ASTTypeForStatement:
    if (ASTForBraceMatcher::Instance().IsZero())
      W = true;
    break;
  case ASTTypeGate:
    if (ASTGateBraceMatcher::Instance().IsZero() &&
        Ty != ASTTypeGateQubitParam)
      W = true;
    break;
  case ASTTypeDefcal:
    if (ASTDefcalBraceMatcher::Instance().IsZero())
      W = true;
    break;
  case ASTTypeFunction:
    if (ASTFunctionBraceMatcher::Instance().IsZero())
      W = true;
    break;
  default:
    break;
  }

  ASTIdentifierNode* IDN =
    ASTBuilder::Instance().CreateASTIdentifierNode(Id->GetName(), Bits, Ty, DCX);
  assert(IDN && "Could not create a valid ASTIdentifierNode!");

  IDN->SetRedeclaration(true);
  IDN->SetPredecessor(Id);
  IDN->SetLocation(TK->GetLocation());

  if (W) {
    std::stringstream M;
    M << "Redeclaration in a single-line statement block "
      << "is pointless.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(IDN), M.str(), DiagLevel::Warning);
  }

  ASTIdentifierTypeController::Instance().SetCurrentType(Ty);
  ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeUndefined);
  return IDN;
}

ASTIdentifierNode*
ASTTypeDiscovery::CreateLocalASTIdentifierNodeRedeclaration(
                             const std::string& Id,
                             unsigned Bits, ASTType Ty,
                             const ASTDeclarationContext* DCX,
                             const ASTToken* TK) const {
  assert(!Id.empty() && "Invalid std::string argument!");
  assert(DCX && "Invalid ASTDeclarationContext argument!");
  assert(TK && "Invalid ASTToken argument!");

  bool W = false;

  switch (DCX->GetContextType()) {
  case ASTTypeGate:
    break;
  case ASTTypeDefcal:
    if (ASTDefcalBraceMatcher::Instance().IsZero())
      W = true;
    break;
  case ASTTypeFunction:
    if (ASTFunctionBraceMatcher::Instance().IsZero())
      W = true;
    break;
  default: {
    std::stringstream M;
    M << "This method should never be used in an ASTDeclarationContext "
      << "of Type " << PrintTypeEnum(DCX->GetContextType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
    return nullptr;
  }
    break;
  }

  ASTIdentifierNode* IDN =
    ASTBuilder::Instance().CreateASTIdentifierNode(Id, Bits, Ty, DCX);
  assert(IDN && "Could not create a valid ASTIdentifierNode!");

  IDN->SetRedeclaration(false);
  IDN->SetLocation(TK->GetLocation());

  if (W) {
    std::stringstream M;
    M << "Redeclaration in a single-line statement block "
      << "is pointless.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(IDN), M.str(), DiagLevel::Warning);
  }

  return IDN;
}

ASTIdentifierNode*
ASTTypeDiscovery::ResolveASTIdentifier(const ASTToken* TK,
                                       const std::string& S) const {
  assert(TK && "Invalid ASTToken argument!");

  if (S.empty())
    return nullptr;

  bool Found = false;
  ASTIdentifierNode* Id = nullptr;

  if (ASTPragmaContextBuilder::Instance().InOpenContext()) {
    Id = ASTBuilder::Instance().CreateASTPragmaIdentifierNode(S);
    assert(Id && "Could not create a valid pragma ASTIdentifierNode!");
    return Id;
  }

  if (ASTTypeSystemBuilder::Instance().IsReservedAngle(S)) {
    Id = ASTBuilder::Instance().FindASTIdentifierNode(S, ASTAngleNode::AngleBits,
                                                      ASTTypeAngle);
  } else if (ASTTypeSystemBuilder::Instance().IsImplicitAngle(S) &&
             ASTIdentifierTypeController::Instance().InAngleList()) {
    Id = ASTBuilder::Instance().FindASTIdentifierNode(S, ASTAngleNode::AngleBits,
                                                      ASTTypeAngle);
  } else {
    Id = ASTBuilder::Instance().FindASTIdentifierNode(S);
  }

  const ASTToken* LTK = ASTTokenFactory::GetLastToken();
  const ASTToken* PTK = ASTTokenFactory::GetPreviousToken();
  const ASTDeclarationContext* DCX =
    ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(DCX && "Could not obtain a valid ASTDeclarationContext!");

  const ASTDeclarationContext* GCX =
    ASTDeclarationContextTracker::Instance().GetGlobalContext();
  assert(GCX && "Could not obtain a valid Global ASTDeclarationContext!");

  if (Id) {
    ASTType CTy = ASTIdentifierTypeController::Instance().GetCurrentType();
    ASTType PTy = ASTIdentifierTypeController::Instance().GetPreviousType();

    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()) &&
        CTy != ASTTypeMPDecimal)
      return Id;

    if (ASTTypeSystemBuilder::Instance().IsReservedConstant(Id->GetName())) {
      const ASTSymbolTableEntry* STE =
        ASTSymbolTable::Instance().FindGlobalSymbol(S, ASTMPDecimalNode::DefaultBits,
                                                    ASTTypeMPDecimal);
      if (STE) {
        Id = const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
        return Id;
      }
    }

    if (ASTIdentifierTypeController::Instance().IsGateType(Id->GetSymbolType())) {
      if (ASTSymbolTable::Instance().FindGate(Id))
        return Id;
    }

    if (Id->IsInductionVariable() && DCX->GetContextType() == ASTTypeForStatement) {
      return Id;
    }

    if (CTy == ASTTypeForStatement && PTy == ASTTypeForStatement) {
      const ASTSymbolTableEntry* ISTE =
        ASTSymbolTable::Instance().FindLocalSymbol(Id);
      if (ISTE && ISTE->GetValueType() == Id->GetSymbolType() &&
          (Id->GetSymbolType() == ASTTypeInt ||
           Id->GetSymbolType() == ASTTypeMPInteger ||
           Id->GetSymbolType() == ASTTypeMPUInteger)) {
        std::stringstream M;
        M << "Implicit re-declaration of loop induction variable '"
          << Id->GetName() << "'.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
        return nullptr;
      }
    }

    if (InReDeclarationContext(Id, CTy, DCX)) {
      uint32_t DCIX;
      if (ASTDeclarationBuilder::Instance().ThisMayBeRedeclaration(Id, DCX, &DCIX) &&
          !ASTIdentifierTypeController::Instance().TypeScopeIsAlwaysGlobal(Id) &&
          DCX->GetIndex() > GCX->GetIndex() &&
          ASTRedeclarationController::Instance().AllowsRedeclaration() &&
          ASTRedeclarationController::Instance().TypeAllowsRedeclaration(Id)) {
        // This is a variable redeclaration at block scope.
        ASTIdentifierNode* RId =
          CreateLocalASTIdentifierNodeRedeclaration(Id,
                          ASTTypeSystemBuilder::Instance().GetTypeBits(CTy),
                          CTy, DCX, TK);
        assert(RId && "Could not create a valid ASTIdentifierNode!");
        assert(RId != Id && "New ASTIdentifierNode cannot be the same as the old!");
        assert(RId->GetSymbolTableEntry() != Id->GetSymbolTableEntry() &&
               "Redeclaration Identifier cannot have the same SymbolTable Entry!");

        ASTRedeclarationController::Instance().AllowRedeclarations(false);
        if (LTK && LTK->GetInt() == Parser::token::TOK_IN &&
            CTy == ASTTypeForStatement && PTy == ASTTypeForStatement)
          RId->SetInductionVariable(true);
        return RId;
      }
    } else if (IsGateQubitParam(Id, CTy, PTy, DCX)) {
      ASTType QTy = ASTTypeGateQubitParam;
      ASTIdentifierNode* RId =
        CreateLocalASTIdentifierNodeRedeclaration(Id,
                         ASTTypeSystemBuilder::Instance().GetTypeBits(QTy),
                         QTy, DCX, TK);
      assert(RId && "Could not create a valid ASTIdentifierNode!");
      assert(RId != Id && "New ASTIdentifierNode cannot be the same as the old!");
      assert(RId->GetSymbolTableEntry() != Id->GetSymbolTableEntry() &&
             "Redeclaration Identifier cannot have the same SymbolTable Entry!");

      CTy = ASTIdentifierTypeController::Instance().GetCurrentType();
      PTy = ASTIdentifierTypeController::Instance().GetPreviousType();
      if (CTy == QTy)
        ASTRedeclarationController::Instance().AllowRedeclarations(true);
      else
        ASTRedeclarationController::Instance().AllowRedeclarations(false);
      return RId;
    } else if (IsGateAngleParam(Id, CTy, PTy, DCX)) {
      ASTType ATy = ASTTypeAngle;
      ASTIdentifierNode* RId =
        CreateLocalASTIdentifierNodeRedeclaration(Id,
                         ASTTypeSystemBuilder::Instance().GetTypeBits(ATy),
                         ATy, DCX, TK);
      assert(RId && "Could not create a valid ASTIdentifierNode!");
      assert(RId != Id && "New ASTIdentifierNode cannot be the same as the old!");
      assert(RId->GetSymbolTableEntry() != Id->GetSymbolTableEntry() &&
             "Redeclaration Identifier cannot have the same SymbolTable Entry!");
      CTy = ASTIdentifierTypeController::Instance().GetCurrentType();
      PTy = ASTIdentifierTypeController::Instance().GetPreviousType();
      if (CTy == ATy)
        ASTRedeclarationController::Instance().AllowRedeclarations(true);
      else
        ASTRedeclarationController::Instance().AllowRedeclarations(false);
      return RId;
    }
  }

  if (Id) {
    ASTType CTy = ASTIdentifierTypeController::Instance().GetCurrentType();
    ASTType PTy = ASTIdentifierTypeController::Instance().GetPreviousType();
    ASTRedeclarationController::Instance().AllowRedeclarations(false);

    if (Id->GetSymbolType() == ASTTypeAngle &&
        (ASTTypeSystemBuilder::Instance().IsReservedAngle(S) ||
        ASTTypeSystemBuilder::Instance().IsImplicitAngle(S))) {
      ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
      ASTIdentifierTypeController::Instance().SetCurrentType(Id->GetSymbolType());
      CTy = ASTIdentifierTypeController::Instance().GetCurrentType();
      PTy = ASTIdentifierTypeController::Instance().GetPreviousType();
    } else if (Id->GetSymbolType() == ASTTypeAngle &&
        (CTy == ASTTypeGate ||
         CTy == ASTTypeCXGate ||
         CTy == ASTTypeCCXGate ||
         CTy == ASTTypeCNotGate ||
         CTy == ASTTypeHadamardGate ||
         CTy == ASTTypeUGate)) {
      ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
      ASTIdentifierTypeController::Instance().SetCurrentType(Id->GetSymbolType());
      CTy = ASTIdentifierTypeController::Instance().GetCurrentType();
      PTy = ASTIdentifierTypeController::Instance().GetPreviousType();
    }

    switch (CTy) {
    case ASTTypeDefcal: {
      ASTMapSymbolTableEntry* MSTE =
        ASTSymbolTable::Instance().FindDefcalGroup(Id->GetName());
      if (!MSTE) {
        ASTIdentifierNode* DGId =
          ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                                    ASTDefcalNode::DefcalBits,
                                                    ASTTypeDefcalGroup);
        assert(DGId && "Could not create a valid defcal group ASTIdentifierNode!");

        ASTDefcalGroupNode* DGN =
          ASTBuilder::Instance().CreateASTDefcalGroupNode(DGId);
        assert(DGN && "Could not create a valid ASTDefcalGroupNode!");

        MSTE = ASTSymbolTable::Instance().FindDefcalGroup(DGId->GetName());
        if (!MSTE) {
          std::stringstream M;
          M << "A defcal group identifier must have a SymbolTable Entry.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
          return nullptr;
        }
      }

      ASTIdentifierNode* DId =
        ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTDefcalNode::DefcalBits,
                                                       ASTTypeDefcal);
      assert(DId && "Could not create a valid defcal ASTIdentifierNode!");
      Id = DId;
      return Id;
    }
      break;
    case ASTTypeGate:
    case ASTTypeCXGate:
    case ASTTypeCCXGate:
    case ASTTypeCNotGate:
    case ASTTypeHadamardGate:
    case ASTTypeUGate: {
      const ASTSymbolTableEntry* STE =
        ASTSymbolTable::Instance().FindGate(Id->GetName());

      if (STE && STE->GetIdentifier()->GetSymbolType() == CTy &&
          DCX->GetContextType() == CTy &&
          STE->GetIdentifier()->GetDeclarationContext() != DCX) {
        std::stringstream M;
        M << "Re-declaration of " << PrintTypeEnum(CTy) << " '" << Id->GetName()
          << "' shadows a previous declaration.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
        return nullptr;
      }

      if (Id->GetSymbolType() == CTy) {
        assert(Id->GetSymbolTableEntry() &&
               "Invalid SymbolTable Entry for Gate ASTIdentifierNode!");
        return Id;
      } else {
        switch (Id->GetSymbolType()) {
        case ASTTypeAngle:
          assert(Id->GetSymbolTableEntry() &&
                 "Invalid SymbolTable Entry for Gate ASTIdentifierNode!");
          ASTIdentifierTypeController::Instance().SetCurrentType(Id->GetSymbolType());
          return Id;
          break;
        case ASTTypeGateQubitParam:
          assert(Id->GetSymbolTableEntry() &&
                 "Invalid SymbolTable Entry for Gate ASTIdentifierNode!");
          ASTIdentifierTypeController::Instance().SetCurrentType(Id->GetSymbolType());
          Id->SetLocalScope();
          if (ASTSymbolTableEntry* XSTE = Id->GetSymbolTableEntry())
            XSTE->SetLocalScope();
          return Id;
          break;
        default:
          break;
        }
      }

      if (STE) {
        std::stringstream M;
        M << "Re-declaration of " << PrintTypeEnum(CTy) << " '" << Id->GetName()
          << "' shadows a previous declaration.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
        return nullptr;
      }
    }
      break;
    case ASTTypeAngle:
      if (Id->GetSymbolType() == ASTTypeGateQubitParam) {
        assert(Id->GetSymbolTableEntry() &&
               "Invalid SymbolTable Entry for Gate ASTIdentifierNode!");
        ASTIdentifierTypeController::Instance().SetCurrentType(Id->GetSymbolType());
        return Id;
      } else if (Id->GetSymbolType() == ASTTypeAngle) {
        assert(Id->GetSymbolTableEntry() &&
               "Invalid SymbolTable Entry for Gate ASTIdentifierNode!");
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        return Id;
      } else if (Id->GetSymbolType() != CTy && CTy != ASTTypeUndefined) {
        if (ASTFunctionContextBuilder::Instance().InOpenContext() &&
            ASTIdentifierTypeController::Instance().SeenLParen()) {
          unsigned Bits = ASTTypeSystemBuilder::Instance().GetTypeBits(CTy);
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(Id->GetName(), Bits,
                                                              CTy);
          assert(Id && "Could not create a valid ASTIdentifierNode!");

          if (!Id->GetSymbolTableEntry()) {
            ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(Id, Id->GetSymbolType());
            assert(STE && "Could not create a valid ASTSymbolTableEntry!");

            Id->SetLocalScope();
            STE->SetLocalScope();
            Id->SetSymbolTableEntry(STE);

            if (!ASTSymbolTable::Instance().InsertLocal(Id, STE)) {
              std::stringstream M;
              M << "Failure inserting local SymbolTable Entry for ASTIdentifierNode "
                << Id->GetName() << '.';
              QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
              return nullptr;
            }
          }

          return Id;
        }
      }
      break;
    case ASTTypeQubit:
    case ASTTypeQubitContainer: {
      if (PTy == ASTTypeFunction || PTy == ASTTypeKernel ||
          ASTFunctionContextBuilder::Instance().InOpenContext() ||
          ASTKernelContextBuilder::Instance().InOpenContext()) {
        if (ASTIdentifierTypeController::Instance().InQubitList()) {
          if (Id->GetSymbolType() == ASTTypeGateQubitParam) {
            if (!Id->GetSymbolTableEntry()) {
              ASTSymbolTableEntry* STE =
                new ASTSymbolTableEntry(Id, Id->GetSymbolType());
              assert(STE && "Could not create a valid ASTSymbolTable Entry!");

              Id->SetSymbolTableEntry(STE);

              ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(),
                                                          Id->GetSymbolType());
              if (!ASTSymbolTable::Instance().InsertLocalSymbol(Id, STE)) {
                std::stringstream M;
                M << "Failure inserting SymbolTable Entry for ASTIdentifierNode "
                  << Id->GetName() << '.';
                QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                  DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
                return nullptr;
              }
            }

            return Id;
          }

          ASTIdentifierNode* QId =
            ASTBuilder::Instance().CreateASTIdentifierNode(Id->GetName(),
                                                           0U, CTy);
          assert(QId && "Could not create a valid ASTIdentifierNode!");

          ASTSymbolTableEntry* STE =
            new ASTSymbolTableEntry(QId, QId->GetSymbolType());
          assert(STE && "Could not create a valid ASTSymbolTableEntry!");

          ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(),
                                                      Id->GetSymbolType());
          if (!ASTSymbolTable::Instance().InsertLocalSymbol(QId, STE)) {
            std::stringstream M;
            M << "Failure inserting SymbolTable Entry for ASTIdentifierNode "
              << QId->GetName() << '.';
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(QId), M.str(), DiagLevel::ICE);
            return nullptr;
          }

          QId->SetSymbolTableEntry(STE);
          Id = QId;
        } else {
          if (ASTIdentifierTypeController::Instance().SeenLParen()) {
          }
          return Id;
        }
      }
    }
      break;
    case ASTTypeForStatement:
      if (PTy == ASTTypeForStatement)
        Id->SetInductionVariable(true);
      break;
    default: {
      if (ASTGateContextBuilder::Instance().InOpenContext() &&
          (Id->GetSymbolType() == ASTTypeGateQubitParam ||
           Id->GetSymbolType() == ASTTypeAngle ||
           Id->GetSymbolType() == ASTTypeGate)) {
        return Id;
      } else if (ASTGateContextBuilder::Instance().InOpenContext() &&
                 (Id->GetSymbolType() == ASTTypeQubit ||
                  Id->GetSymbolType() == ASTTypeQubitContainer)) {
        std::string YB = DIAGLineBuffer::Instance().GetBuffer();

        if (ASTFunctionContextBuilder::Instance().InOpenContext() ||
            ASTKernelContextBuilder::Instance().InOpenContext() ||
            ASTDefcalContextBuilder::Instance().InOpenContext()) {
          const ASTType ITy = Id->GetSymbolType();
          if (ASTIdentifierTypeController::Instance().MustHaveASymbolTableEntry(ITy) &&
              Id->GetSymbolTableEntry() == nullptr) {
            std::stringstream M;
            M << "An Identifier for Type " << PrintTypeEnum(ITy) << " must "
              << "have created a SymbolTable Entry." << std::endl;
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
            return nullptr;
          }

          return Id;
        }

        size_t LP = YB.find(u8'(');
        size_t RP = YB.find(u8')');

        if (LP == std::string::npos && RP == std::string::npos) {
          ASTSymbolTable::Instance().EraseLocalSymbol(Id->GetName());
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(Id->GetName(), 1U,
                                                              ASTTypeGateQubitParam);
          assert(Id && "Could not create a valid ASTIdentifierNode!");

          Id->SetLocalScope();
          return Id;
        } else if (LP != std::string::npos && RP != std::string::npos) {
          ASTSymbolTable::Instance().EraseLocalSymbol(Id->GetName());
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(Id->GetName(), 1U,
                                                              ASTTypeGateQubitParam);
          assert(Id && "Could not create a valid ASTIdentifierNode!");

          Id->SetLocalScope();
          return Id;
        }
      } else if (ASTDefcalContextBuilder::Instance().InOpenContext() &&
               (Id->GetSymbolType() == ASTTypeQubit ||
                Id->GetSymbolType() == ASTTypeQubitContainer ||
                Id->GetSymbolType() == ASTTypeAngle ||
                Id->GetSymbolType() == ASTTypeGate ||
                Id->GetSymbolType() == ASTTypeDefcal)) {
        return Id;
    }

      if ((ASTFunctionContextBuilder::Instance().InOpenContext() ||
           ASTKernelContextBuilder::Instance().InOpenContext()) &&
          ASTIdentifierTypeController::Instance().SeenLParen()) {
        if (CTy != ASTTypeUndefined && CTy != Id->GetSymbolType()) {
          if (!ASTIdentifierTypeController::Instance().TypeScopeIsAlwaysGlobal(CTy)) {
            if (ASTIdentifierTypeController::Instance().SeenLBracket() &&
                ASTIdentifierTypeController::Instance().SeenRBracket() &&
                ASTUtils::Instance().IsIntegralType(Id->GetSymbolType())) {
              return Id;
            } else if (ASTUtils::Instance().IsQubitType(Id->GetSymbolType()) &&
                       (CTy == ASTTypeReset || CTy == ASTTypeMeasure)) {
              return Id;
            } else if (CTy == ASTTypeReturn || CTy == ASTTypeForStatement ||
                       CTy == ASTTypeWhileStatement) {
              return Id;
            }

            Id = ASTBuilder::Instance().CreateASTIdentifierNode(Id->GetName(),
                             ASTTypeSystemBuilder::Instance().GetTypeBits(CTy), CTy);
          } else {
            if (ASTIdentifierTypeController::Instance().SeenLBracket() &&
                ASTIdentifierTypeController::Instance().CanReadIndexedIdentifier(CTy) &&
                ASTUtils::Instance().IsIntegralType(Id->GetSymbolType())) {
              return Id;
            } else if (ASTIdentifierTypeController::Instance().SeenLBracket() &&
                       ASTIdentifierTypeController::Instance().SeenRBracket() &&
                       ASTUtils::Instance().IsIntegralType(Id->GetSymbolType())) {
              return Id;
            } else if (Id->GetDeclarationContext()->GetIndex() == DCX->GetIndex() ||
                       Id->GetDeclarationContext()->GetHash() == DCX->GetHash()) {
              return Id;
            } else if (Id->GetDeclarationContext()->GetIndex() < DCX->GetIndex() &&
                       !Id->GetDeclarationContext()->IsDead()) {
              return Id;
            } else if (CTy == ASTTypeReturn) {
              return Id;
            }

            unsigned Bits = ASTTypeSystemBuilder::Instance().GetTypeBits(CTy);
            Id = ASTBuilder::Instance().CreateASTIdentifierNode(Id->GetName(), Bits,
                                                                CTy);
          }

          assert(Id && "Could not create a valid ASTIdentifierNode!");

          if (!Id->GetSymbolTableEntry()) {
            ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(Id, Id->GetSymbolType());
            assert(STE && "Could not create a valid ASTSymbolTableEntry!");

            Id->SetLocalScope();
            STE->SetLocalScope();
            Id->SetSymbolTableEntry(STE);

            if (!ASTSymbolTable::Instance().InsertLocal(Id, STE)) {
              std::stringstream M;
              M << "Failure inserting local SymbolTable Entry for ASTIdentifierNode "
                << Id->GetName() << '.';
              QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
              return nullptr;
            }
          }

          return Id;
        } else if (Id->GetSymbolType() != ASTTypeUndefined &&
                   CTy == ASTTypeUndefined) {
          CTy = Id->GetSymbolType();
          return Id;
        }
      }
    }
      break;
    }

    ASTSymbolTableEntry* STE = Id->GetSymbolTableEntry();
    if (!STE) {
      ASTType ITy = Id->GetSymbolType();
      if (ASTIdentifierTypeController::Instance().MustHaveASymbolTableEntry(ITy)) {
        std::stringstream M;
        M << "An Identifier for Type " << PrintTypeEnum(ITy) << " must "
          << "have created a SymbolTable Entry." << std::endl;
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
        return nullptr;
      }

      STE = new ASTSymbolTableEntry(Id, Id->GetSymbolType());
      assert(STE && "Could not create a valid SymbolTable Entry!");

      STE->SetContext(Id->GetDeclarationContext());

      if (Id->IsGlobalScope())
        STE->SetGlobalScope();
      else
        STE->SetLocalScope();

      Id->SetSymbolTableEntry(STE);

      if (Id->IsGlobalScope()) {
        if (!ASTSymbolTable::Instance().UpdateGlobal(Id, STE)) {
          std::stringstream M;
          M << "Failure updating Global SymbolTable Entry for "
            << "ASTIdentifierNode " << Id->GetName() << '.';
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
          return nullptr;
        }
      } else {
        if (!ASTSymbolTable::Instance().UpdateLocal(Id, STE)) {
          std::stringstream M;
          M << "Failure updating Local SymbolTable Entry for "
            << "ASTIdentifierNode " << Id->GetName() << '.';
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
          return nullptr;
        }
      }
    }

    if (Id->GetSymbolType() == ASTTypeGate ||
        Id->GetSymbolType() == ASTTypeDefcal) {
      ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
      ASTIdentifierTypeController::Instance().SetPreviousType(Id->GetSymbolType());
    } else {
      switch (CTy) {
      case ASTTypeArray:
      case ASTTypeCBitArray:
      case ASTTypeCBitNArray:
      case ASTTypeQubitArray:
      case ASTTypeQubitNArray:
      case ASTTypeAngleArray:
      case ASTTypeBoolArray:
      case ASTTypeIntArray:
      case ASTTypeMPIntegerArray:
      case ASTTypeFloatArray:
      case ASTTypeMPDecimalArray:
      case ASTTypeMPComplexArray:
      case ASTTypeDurationArray:
      case ASTTypeOpenPulseFrameArray:
      case ASTTypeOpenPulsePortArray:
        break;
      case ASTTypeMPInteger:
      case ASTTypeMPDecimal:
      case ASTTypeMPComplex:
      case ASTTypeAngle:
        if (Id->GetSymbolType() == ASTTypeInt) {
        } else {
          ASTIdentifierTypeController::Instance().SetCurrentType(Id->GetSymbolType());
          ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        }
        break;
      default:
        ASTIdentifierTypeController::Instance().SetCurrentType(Id->GetSymbolType());
        ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        break;
      }
    }

    Found = true;
  } else {
    ASTType CTy = ASTIdentifierTypeController::Instance().GetCurrentType();
    ASTType PTy = ASTIdentifierTypeController::Instance().GetPreviousType();
    ASTRedeclarationController::Instance().AllowRedeclarations(false);

    if (CTy == ASTTypeThetaAngle || CTy == ASTTypeLambdaAngle ||
        CTy == ASTTypePhiAngle || CTy == ASTTypeAngle) {
      const ASTSymbolTableEntry* ASTE = ASTSymbolTable::Instance().FindLocal(S);
      if (ASTE) {
        return const_cast<ASTIdentifierNode*>(ASTE->GetIdentifier());
      }
    }

    if (!ASTExpressionValidator::Instance().IsFunctionType(PTy)) {
      if (ASTIdentifierTypeController::Instance().SeenLParen() &&
          ASTIdentifierTypeController::Instance().SeenRParen() &&
          ASTIdentifierTypeController::Instance().IsGateType(PTy) &&
          !ASTTypeSystemBuilder::Instance().IsImplicitAngle(S) &&
          !ASTTypeSystemBuilder::Instance().IsReservedAngle(S)) {
        uint32_t TIX = ASTTokenFactory::GetCurrentIndex() - 1;
        const ASTToken* ITK = ASTTokenFactory::GetToken(TIX);
        assert(ITK && "Could not obtain a valid ASTToken!");

        const std::string& TKS = ITK->GetString();
        if (TKS[0] == u8'{' || TKS[0] == u8',') {
          Id = ASTBuilder::Instance().CreateLocalScopeASTIdentifierNode(S, 1U,
                                               ASTTypeGateQubitParam, DCX, TK);
          assert(Id && "Could not create a valid ASTIdentifierNode!");
          Id->SetLocation(TK->GetLocation());
          Id->SetDeclarationContext(DCX);
          Id->SetGateLocal(true);
          Id->SetLocalScope();

          if (TKS[0] == u8'{')
            ASTIdentifierTypeController::Instance().StopQubitList();

          return Id;
        }
      } else if (!ASTIdentifierTypeController::Instance().SeenLParen() &&
                 !ASTIdentifierTypeController::Instance().SeenRParen() &&
                 ASTIdentifierTypeController::Instance().IsGateType(PTy) &&
                 ASTIdentifierTypeController::Instance().IsGateType(CTy) &&
                 !ASTTypeSystemBuilder::Instance().IsImplicitAngle(S) &&
                 !ASTTypeSystemBuilder::Instance().IsReservedAngle(S)) {
        uint32_t TIX = ASTTokenFactory::GetCurrentIndex() - 1;
        const ASTToken* ITK = ASTTokenFactory::GetToken(TIX);
        assert(ITK && "Could not obtain a valid ASTToken!");

        const std::string& TKS = ITK->GetString();
        if (TKS[0] == u8'{' || TKS[0] == u8',') {
          Id = ASTBuilder::Instance().CreateLocalScopeASTIdentifierNode(S, 1U,
                                               ASTTypeGateQubitParam, DCX, TK);
          assert(Id && "Could not create a valid ASTIdentifierNode!");
          Id->SetLocation(TK->GetLocation());
          Id->SetDeclarationContext(DCX);
          Id->SetGateLocal(true);
          Id->SetLocalScope();

          if (TKS[0] == u8'{')
            ASTIdentifierTypeController::Instance().StopQubitList();

          return Id;
        }
      } else if (!ASTIdentifierTypeController::Instance().SeenLParen() &&
                 !ASTIdentifierTypeController::Instance().SeenRParen() &&
                 (ASTIdentifierTypeController::Instance().IsGateType(PTy) ||
                  PTy == ASTTypeUndefined) &&
                 (ASTIdentifierTypeController::Instance().IsGateType(CTy) ||
                  CTy == ASTTypeGateQubitParam) &&
                 !ASTTypeSystemBuilder::Instance().IsImplicitAngle(S) &&
                 !ASTTypeSystemBuilder::Instance().IsReservedAngle(S)) {
        uint32_t TIX = ASTTokenFactory::GetCurrentIndex() - 1;
        const ASTToken* ITK = ASTTokenFactory::GetToken(TIX);
        assert(ITK && "Could not obtain a valid ASTToken!");

        const std::string& TKS = ITK->GetString();
        if (TKS[0] == u8'{' || TKS[0] == u8',') {
          Id = ASTBuilder::Instance().CreateLocalScopeASTIdentifierNode(S, 1U,
                                               ASTTypeGateQubitParam, DCX, TK);
          assert(Id && "Could not create a valid ASTIdentifierNode!");
          Id->SetLocation(TK->GetLocation());
          Id->SetDeclarationContext(DCX);
          Id->SetGateLocal(true);
          Id->SetLocalScope();

          if (TKS[0] == u8'{')
            ASTIdentifierTypeController::Instance().StopQubitList();

          return Id;
        }
      } else if (ASTIdentifierTypeController::Instance().SeenLParen() &&
                 ASTIdentifierTypeController::Instance().SeenRParen() &&
                 (ASTIdentifierTypeController::Instance().IsGateType(PTy) ||
                  PTy == ASTTypeUndefined) &&
                 (ASTIdentifierTypeController::Instance().IsGateType(CTy) ||
                  CTy == ASTTypeGateQubitParam) &&
                 !ASTTypeSystemBuilder::Instance().IsImplicitAngle(S) &&
                 !ASTTypeSystemBuilder::Instance().IsReservedAngle(S)) {
        uint32_t TIX = ASTTokenFactory::GetCurrentIndex() - 1;
        const ASTToken* ITK = ASTTokenFactory::GetToken(TIX);
        assert(ITK && "Could not obtain a valid ASTToken!");

        const std::string& TKS = ITK->GetString();
        if (TKS[0] == u8'{' || TKS[0] == u8',') {
          Id = ASTBuilder::Instance().CreateLocalScopeASTIdentifierNode(S, 1U,
                                               ASTTypeGateQubitParam, DCX, TK);
          assert(Id && "Could not create a valid ASTIdentifierNode!");
          Id->SetLocation(TK->GetLocation());
          Id->SetDeclarationContext(DCX);
          Id->SetGateLocal(true);
          Id->SetLocalScope();

          if (TKS[0] == u8'{')
            ASTIdentifierTypeController::Instance().StopQubitList();

          return Id;
        }
      } else if (DCX->GetContextType() == ASTTypeGate &&
                 ASTIdentifierTypeController::Instance().SeenLParen() &&
                 ASTIdentifierTypeController::Instance().SeenRParen() &&
                 !ASTTypeSystemBuilder::Instance().IsImplicitAngle(S) &&
                 !ASTTypeSystemBuilder::Instance().IsReservedAngle(S)) {
        uint32_t TIX = ASTTokenFactory::GetCurrentIndex() - 1;
        const ASTToken* ITK = ASTTokenFactory::GetToken(TIX);
        assert(ITK && "Could not obtain a valid ASTToken!");

        const std::string& TKS = ITK->GetString();
        if (TKS[0] == u8'{' || TKS[0] == u8',') {
          Id = ASTBuilder::Instance().CreateLocalScopeASTIdentifierNode(S, 1U,
                                               ASTTypeGateQubitParam, DCX, TK);
          assert(Id && "Could not create a valid ASTIdentifierNode!");
          Id->SetLocation(TK->GetLocation());
          Id->SetDeclarationContext(DCX);
          Id->SetGateLocal(true);
          Id->SetLocalScope();

          if (TKS[0] == u8'{')
            ASTIdentifierTypeController::Instance().StopQubitList();

          return Id;
        }
      }
    }

    if (IsGateAngleParam(S, CTy, PTy, DCX)) {
      ASTType ATy = ASTTypeAngle;
      ASTIdentifierNode* RId =
        CreateLocalASTIdentifierNodeRedeclaration(S,
                                ASTTypeSystemBuilder::Instance().GetTypeBits(ATy),
                                ATy, DCX, TK);
      assert(RId && "Could not create a valid ASTIdentifierNode!");
      assert(RId->GetSymbolTableEntry() &&
             "Gate Angle Param does not have a SymbolTable Entry!");

      ASTSymbolTable::Instance().TransferAngleToLSTM(RId, RId->GetBits(),
                                                     RId->GetSymbolType());

      if (ASTIdentifierTypeController::Instance().SeenLParen()) {
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeGate);
      } else {
        ASTIdentifierTypeController::Instance().SetCurrentType(ATy);
        ASTIdentifierTypeController::Instance().SetPreviousType(ATy);
      }

      if (!ASTIdentifierTypeController::Instance().SeenLBrace()) {
        ASTRedeclarationController::Instance().AllowRedeclarations(true);
      } else {
        ASTIdentifierTypeController::Instance().StopAngleList();
        ASTRedeclarationController::Instance().AllowRedeclarations(false);
      }

      RId->SetLocation(TK->GetLocation());

      uint32_t TIX = ASTTokenFactory::GetCurrentIndex() - 1;
      const ASTToken* ITK = ASTTokenFactory::GetToken(TIX);
      assert(ITK && "Could not obtain a valid ASTToken!");
      const std::string& TKS = ITK->GetString();

      if (TKS[0] == u8')')
        ASTIdentifierTypeController::Instance().StopAngleList();

      return RId;
    }

    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(S)) {
      Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                           ASTAngleNode::AngleBits,
                                           ASTTypeAngle);
      assert(Id && "Could not create a valid ASTIdentifierNode!");
    } else if (ASTGateContextBuilder::Instance().InOpenContext() &&
               ASTTypeSystemBuilder::Instance().IsImplicitAngle(S)) {
      if (ASTIdentifierTypeController::Instance().InAngleList() &&
          !ASTIdentifierTypeController::Instance().SeenLBrace()) {
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                             ASTAngleNode::AngleBits,
                                             ASTTypeAngle);
        assert(Id && "Could not create a valid ASTIdentifierNode!");

        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeAngle);
        ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeAngle);

        if (ASTIdentifierTypeController::Instance().ParensMatched()) {
          uint32_t TIX = ASTTokenFactory::GetCurrentIndex() - 1;
          const ASTToken* ITK = ASTTokenFactory::GetToken(TIX);
          assert(ITK && "Could not obtain a valid ASTToken!");
          const std::string& TKS = ITK->GetString();

          if (TKS[0] == u8')')
            ASTIdentifierTypeController::Instance().StopAngleList();

          ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeGateQubitParam);
        }
      } else if (ASTIdentifierTypeController::Instance().InQubitList()) {
        Id = ASTBuilder::Instance().CreateLocalScopeASTIdentifierNode(S, 1U,
                                                    ASTTypeGateQubitParam, DCX, TK);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        if (ASTIdentifierTypeController::Instance().SeenLBrace()) {
          uint32_t TIX = ASTTokenFactory::GetCurrentIndex() - 1;
          const ASTToken* ITK = ASTTokenFactory::GetToken(TIX);
          assert(ITK && "Could not obtain a valid ASTToken!");
          const std::string& TKS = ITK->GetString();

          if (TKS[0] == u8'{')
            ASTIdentifierTypeController::Instance().StopQubitList();

          ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeGateQubitParam);
          ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeGateQubitParam);
        }
      } else if (ASTGateContextBuilder::Instance().InOpenContext() &&
                 ASTIdentifierTypeController::Instance().IsGateType(CTy)) {
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTGateNode::GateBits,
                                                            ASTTypeGate);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetGlobalScope();
        if (ASTSymbolTableEntry* XSTE = Id->GetSymbolTableEntry())
          XSTE->SetGlobalScope();

        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeGate);
      } else {
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0U,
                                                            ASTTypeUndefined);
        assert(Id && "Could not create a valid ASTIdentifierNode!");

        Id->SetLocation(TK->GetLocation());
        if (!Id->GetSymbolTableEntry()) {
          ASTSymbolTableEntry *STE = new ASTSymbolTableEntry(Id,
                                                             Id->GetSymbolType());
          assert(STE && "Could not create a valid SymbolTable Entry!");
          Id->SetSymbolTableEntry(STE);
        }
      }
    } else if (ASTIdentifierTypeController::Instance().InQubitList()) {
      switch (CTy) {
      case ASTTypeGateQubitParam:
        Id = ASTBuilder::Instance().CreateLocalScopeASTIdentifierNode(S, 1U,
                                                    ASTTypeGateQubitParam, DCX, TK);
        assert(Id && "Could not create a valid ASTIdentifierNode!");

        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeGateQubitParam);
        ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeGateQubitParam);

        if (ASTIdentifierTypeController::Instance().SeenLBrace()) {
          uint32_t TIX = ASTTokenFactory::GetCurrentIndex() - 1;
          const ASTToken* ITK = ASTTokenFactory::GetToken(TIX);
          assert(ITK && "Could not obtain a valid ASTToken!");
          const std::string& TKS = ITK->GetString();

          if (TKS[0] == u8'{')
            ASTIdentifierTypeController::Instance().StopQubitList();

          ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        }
        break;
      case ASTTypeQubit:
      case ASTTypeQubitContainer: {
        std::string QS = S;
        unsigned Bits = 0U;

        if (ASTStringUtils::Instance().IsIndexed(S)) {
          Bits = ASTStringUtils::Instance().GetIdentifierIndex(S);
          QS = ASTStringUtils::Instance().GetIdentifierBase(S);
        }

        Id = ASTBuilder::Instance().CreateASTIdentifierNode(QS, Bits,
                                                       ASTTypeQubitContainer);
        assert(Id && "Could not create a valid ASTIdentifierNode!");

        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeQubitContainer);
        ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeQubitContainer);

        if (ASTIdentifierTypeController::Instance().ParensMatched()) {
          ASTIdentifierTypeController::Instance().StopQubitList();
          ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        }
      }
        break;
      case ASTTypeDefcal: {
        ASTMapSymbolTableEntry* MSTE =
          ASTSymbolTable::Instance().FindDefcalGroup(S);
        if (!MSTE) {
          ASTIdentifierNode* GId =
            ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTDefcalNode::DefcalBits,
                                                           ASTTypeDefcalGroup);
          assert(GId && "Could not create a valid ASTIdentifierNode!");
        }

        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTDefcalNode::DefcalBits,
                                                            ASTTypeDefcal);
        assert(Id && "Could not create a valid ASTIdentifierNode!");

        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeDefcal);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        assert(Id->GetSymbolTableEntry() && "ASTIdentifierNode has no SymbolTable Entry!");
      }
        break;
      case ASTTypeFunction:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                    ASTFunctionDefinitionNode::FunctionBits,
                                    ASTTypeFunction);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        assert(Id->GetSymbolTableEntry() && "ASTIdentifierNode has no SymbolTable Entry!");

        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        break;
      case ASTTypeGate:
      case ASTTypeCNotGate:
      case ASTTypeCCXGate:
      case ASTTypeCXGate:
      case ASTTypeHadamardGate:
      case ASTTypeUGate:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTGateNode::GateBits,
                                                            ASTTypeGate);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        assert(Id->GetSymbolTableEntry() && "ASTIdentifierNode has no SymbolTable Entry!");

        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        break;
      case ASTTypeEulerAngle:
      case ASTTypeLambdaAngle:
      case ASTTypePhiAngle:
      case ASTTypePiAngle:
      case ASTTypeTauAngle:
      case ASTTypeThetaAngle:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTAngleNode::AngleBits,
                                                            ASTTypeAngle);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        assert(Id->GetSymbolTableEntry() && "ASTIdentifierNode has no SymbolTable Entry!");

        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeAngle);
        ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeAngle);
        break;
      case ASTTypeAngle:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0U, ASTTypeAngle);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        assert(Id->GetSymbolTableEntry() && "ASTIdentifierNode has no SymbolTable Entry!");

        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeAngle);
        ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeAngle);
        break;
      case ASTTypeOpenPulseFrame:
        if (ASTCalContextBuilder::Instance().InOpenContext() ||
            ASTDefcalContextBuilder::Instance().InOpenContext()) {
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                      OpenPulse::ASTOpenPulseFrameNode::FrameBits,
                                      ASTTypeOpenPulseFrame);
          assert(Id && "Could not create a valid ASTIdentifierNode!");
          assert(Id->GetSymbolTableEntry() &&
                 "ASTIdentifierNode has no SymbolTable Entry!");

          Id->SetLocation(TK->GetLocation());
          ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
          ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        }
        break;
      case ASTTypeOpenPulsePort:
        if (ASTCalContextBuilder::Instance().InOpenContext() ||
            ASTDefcalContextBuilder::Instance().InOpenContext()) {
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                      OpenPulse::ASTOpenPulsePortNode::PortBits,
                                      ASTTypeOpenPulsePort);
          ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
          ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        }
        break;
      case ASTTypeOpenPulseWaveform:
        if (ASTCalContextBuilder::Instance().InOpenContext() ||
            ASTDefcalContextBuilder::Instance().InOpenContext()) {
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                      OpenPulse::ASTOpenPulseWaveformNode::WaveformBits,
                                      ASTTypeOpenPulseWaveform);
          assert(Id && "Could not create a valid ASTIdentifierNode!");
          assert(Id->GetSymbolTableEntry() &&
                 "ASTIdentifierNode has no SymbolTable Entry!");

          Id->SetLocation(TK->GetLocation());
          ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
          ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        }
        break;
      default:
        if (ASTFunctionContextBuilder::Instance().InOpenContext() &&
            ASTIdentifierTypeController::Instance().SeenLParen()) {
          switch (CTy) {
          case ASTTypeBool:
            Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTBoolNode::BoolBits,
                                                                CTy);
            assert(Id && "Could not create a valid ASTIdentifierNode!");
            Id->SetLocation(TK->GetLocation());
            break;
          case ASTTypeInt:
            Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTIntNode::IntBits,
                                                                CTy);
            assert(Id && "Could not create a valid ASTIdentifierNode!");
            Id->SetLocation(TK->GetLocation());
            break;
          case ASTTypeFloat:
            Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTFloatNode::FloatBits,
                                                                CTy);
            assert(Id && "Could not create a valid ASTIdentifierNode!");
            Id->SetLocation(TK->GetLocation());
            break;
          case ASTTypeMPInteger:
            Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                                 ASTMPIntegerNode::DefaultBits,
                                                 CTy);
            assert(Id && "Could not create a valid ASTIdentifierNode!");
            Id->SetLocation(TK->GetLocation());
            break;
          case ASTTypeMPDecimal:
            Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                                 ASTMPDecimalNode::DefaultBits,
                                                 CTy);
            assert(Id && "Could not create a valid ASTIdentifierNode!");
            Id->SetLocation(TK->GetLocation());
            break;
          case ASTTypeQubitContainer:
            Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0, CTy);
          assert(Id && "Could not create a valid ASTIdentifierNode!");
            break;
          case ASTTypeAngle:
            Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                                 ASTAngleNode::AngleBits,
                                                 CTy);
            assert(Id && "Could not create a valid ASTIdentifierNode!");
            Id->SetLocation(TK->GetLocation());
            break;
          default:
            break;
          }
        } else if (ASTTypeSystemBuilder::Instance().IsImplicitAngle(S)) {
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTAngleNode::AngleBits,
                                                              ASTTypeAngle);
        } else if ((PTy == ASTTypeGate ||
                    ASTGateContextBuilder::Instance().InOpenContext()) &&
                   ASTIdentifierTypeController::Instance().SeenLParen()) {
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTAngleNode::AngleBits,
                                                              ASTTypeAngle);
          assert(Id && "Could not create a valid ASTIdentifierNode!");
        } else {
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 1U,
                                                              ASTTypeGateQubitParam);
        }

        assert(Id && "Could not create a valid ASTIdentifierNode!");
        assert(Id->GetSymbolTableEntry() && "ASTIdentifierNode has no SymbolTable Entry!");
        Id->SetLocation(TK->GetLocation());
        break;
      }

      assert(Id && "Could not create a valid ASTIdentifierNode!");
      if (ASTIdentifierTypeController::Instance().SeenLBrace()) {
        ASTIdentifierTypeController::Instance().StopQubitList();
        ASTIdentifierTypeController::Instance().StopAngleList();
      }
    } else if (ASTTypeSystemBuilder::Instance().IsBuiltinFunction(S)) {
      Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                           ASTFunctionDefinitionNode::FunctionBits,
                                           ASTTypeFunction);
      assert(Id && "Could not create a valid ASTIdentifierNode!");
      Id->SetLocation(TK->GetLocation());
    } else if (CTy == ASTTypeQubitContainer && (PTy == ASTTypeFunction ||
                                                PTy == ASTTypeKernel)) {
      Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0U,
                                                          ASTTypeQubitContainer);
      assert(Id && "Could not create a valid ASTIdentifierNode!");

      Id->SetLocation(TK->GetLocation());
      ASTSymbolTableEntry* STE = Id->GetSymbolTableEntry();
      if (!STE) {
        STE = new ASTSymbolTableEntry(Id, Id->GetSymbolType());
        assert(STE && "Could not create a valid SymbolTable Entry!");
      }

      ASTIdentifierTypeController::Instance().SetCurrentType(PTy);
      ASTIdentifierTypeController::Instance().SetPreviousType(PTy);
    } else if (CTy == ASTTypeExtern && PTy == ASTTypeUndefined) {
      ASTKernelContextBuilder::Instance().OpenContext();
      ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
      ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeKernel);
      Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTKernelNode::KernelBits,
                                                          ASTTypeKernel);
      assert(Id && "Could not create a valid ASTIdentifierNode!");

      Id->SetLocation(TK->GetLocation());
      ASTSymbolTableEntry* STE = Id->GetSymbolTableEntry();
      if (!STE) {
        STE = new ASTSymbolTableEntry(Id, Id->GetSymbolType());
        assert(STE && "Could not create a valid SymbolTable Entry!");
      }
    } else if (PTy == ASTTypeArray) {
      ASTType NTy = ASTTypeUndefined;

      switch (CTy) {
      case ASTTypeAngle:
        NTy = ASTTypeAngleArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeBool:
        NTy = ASTTypeBoolArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeInt:
      case ASTTypeUInt:
        NTy = ASTTypeIntArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeMPInteger:
      case ASTTypeMPUInteger:
        NTy = ASTTypeMPIntegerArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeFloat:
      case ASTTypeDouble:
        NTy = ASTTypeFloatArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeMPDecimal:
        NTy = ASTTypeMPDecimalArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeMPComplex:
        NTy = ASTTypeMPComplexArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeDuration:
      case ASTTypeDurationOf:
        NTy = ASTTypeDurationArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeQubit:
      case ASTTypeQubitContainer:
        NTy = ASTTypeQubitArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeBitset:
        NTy = ASTTypeCBitArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeOpenPulseFrame:
        NTy = ASTTypeOpenPulseFrameArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeOpenPulsePort:
        NTy = ASTTypeOpenPulsePortArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeOpenPulseWaveform:
        NTy = ASTTypeOpenPulseWaveformArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeAngleArray:
        NTy = CTy;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeBoolArray:
        NTy = CTy;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeCBitArray:
        NTy = CTy;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeCBitNArray:
        NTy = ASTTypeCBitArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeIntArray:
        NTy = CTy;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeFloatArray:
        NTy = CTy;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeMPIntegerArray:
        NTy = CTy;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeMPDecimalArray:
        NTy = CTy;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeMPComplexArray:
        NTy = CTy;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeDurationArray:
        NTy = CTy;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeQubitArray:
        NTy = CTy;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeQubitNArray:
        NTy = ASTTypeQubitArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeOpenPulseFrameArray:
        NTy = CTy;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeOpenPulsePortArray:
        NTy = CTy;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTArrayNode::ArrayBits,
                                                            NTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      default:
        // We haven't been able to infer any concrete type.
        Id = nullptr;
        break;
      }

      CTy = NTy;
    } else if (CTy == ASTTypeForStatement && PTy == ASTTypeForStatement &&
               DCX->GetContextType() == ASTTypeForStatement) {
      CTy = ASTTypeInt;
      Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTIntNode::IntBits,
                                                          CTy);
      assert(Id && "Could not create a valid ASTIdentifierNode!");

      Id->SetLocation(TK->GetLocation());
      Id->SetInductionVariable(true);
      ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
      ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
    } else if (CTy == ASTTypeWhileStatement && PTy == ASTTypeWhileStatement &&
               DCX->GetContextType() == ASTTypeWhileStatement) {
      ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
      ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
      std::stringstream M;
      M << "Unknown identifier '" << S << "' at current scope.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return nullptr;
    } else {
      switch (CTy) {
      case ASTTypeFunction:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                    ASTFunctionDefinitionNode::FunctionBits,
                                    CTy);
        break;
      case ASTTypeKernel:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                             ASTKernelNode::KernelBits,
                                             CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeQubitContainer:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0U, CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeQubit:
        CTy = ASTTypeQubitContainer;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 1U, CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeQubitContainerAlias:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0U, CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeBool:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTBoolNode::BoolBits,
                                                            CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        break;
      case ASTTypeDouble:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTDoubleNode::DoubleBits,
                                                            CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        break;
      case ASTTypeAngle:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0U, CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        break;
      case ASTTypeDelay:
        if (S == "dt" || S == "DT")
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTDelayNode::DelayBits,
                                                              ASTTypeTimeUnit);
        else
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTDelayNode::DelayBits,
                                                              CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        break;
      case ASTTypeDuration:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                            ASTDurationNode::DurationBits, CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        break;
      case ASTTypeInt:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTIntNode::IntBits,
                                                            CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        break;
      case ASTTypeFloat:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, ASTFloatNode::FloatBits,
                                                            CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        break;
      case ASTTypeMPInteger:
      case ASTTypeMPDecimal:
      case ASTTypeMPComplex:
        if ((CTy == ASTTypeMPDecimal || CTy == ASTTypeMPInteger) &&
            PTy == ASTTypeMPComplex) {
          const unsigned Bits = ASTMPComplexNode::DefaultBits;
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, Bits, ASTTypeMPComplex);
          assert(Id && "Could not create a valid ASTIdentifierNode!");
          Id->SetLocation(TK->GetLocation());
          ASTIdentifierTypeController::Instance().SetPreviousType(ASTTypeMPComplex);
          ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        } else if ((CTy == ASTTypeMPDecimal || CTy == ASTTypeMPInteger) &&
                   PTy == ASTTypeUndefined) {
          const unsigned Bits = ASTMPDecimalNode::DefaultBits;
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, Bits, CTy);
          assert(Id && "Could not create a valid ASTIdentifierNode!");
          Id->SetLocation(TK->GetLocation());
          ASTIdentifierTypeController::Instance().SetCurrentType(CTy);
          ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
          return Id;
        } else {
          Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0U, CTy);
          assert(Id && "Could not create a valid ASTIdentifierNode!");
          Id->SetLocation(TK->GetLocation());
          ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
          ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        }
        break;
      case ASTTypeOpenPulseFrame:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                    OpenPulse::ASTOpenPulseFrameNode::FrameBits,
                                    CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        break;
      case ASTTypeOpenPulsePort:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                    OpenPulse::ASTOpenPulsePortNode::PortBits,
                                    CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        break;
      case ASTTypeOpenPulseWaveform:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                         OpenPulse::ASTOpenPulseWaveformNode::WaveformBits,
                         CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
        ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
        break;
      case ASTTypeIntArray:
      case ASTTypeFloatArray:
      case ASTTypeMPIntegerArray:
      case ASTTypeMPDecimalArray:
      case ASTTypeMPComplexArray:
      case ASTTypeAngleArray:
      case ASTTypeDurationArray:
      case ASTTypeLengthArray:
      case ASTTypeCBitArray:
      case ASTTypeCBitNArray:
      case ASTTypeBoolArray:
      case ASTTypeQubitArray:
      case ASTTypeOpenPulseFrameArray:
      case ASTTypeOpenPulsePortArray:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0U, CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeQubitNArray:
        CTy = ASTTypeQubitArray;
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0U, CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeBitset:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0U, CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeBarrier:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                             ASTBarrierNode::BarrierBits, CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeBox:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                          ASTBoxExpressionNode::BoxBits, CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeBoxTo:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                          ASTBoxToExpressionNode::BoxToBits, CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeBoxAs:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S,
                                          ASTBoxAsExpressionNode::BoxAsBits, CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeVoid:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0U, CTy);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      case ASTTypeLet:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0U,
                                                            ASTTypeQubitContainerAlias);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      default:
        Id = ASTBuilder::Instance().CreateASTIdentifierNode(S, 0U,
                                                            ASTTypeUndefined);
        assert(Id && "Could not create a valid ASTIdentifierNode!");
        Id->SetLocation(TK->GetLocation());
        break;
      }

      ASTIdentifierTypeController::Instance().SetPreviousType(CTy);
      ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUndefined);
    }

    assert(Id && "Could not create a valid ASTIdentifierNode!");

    ASTSymbolTableEntry* STE = Id->GetSymbolTableEntry();
    if (!STE) {
      STE = new ASTSymbolTableEntry(Id, Id->GetSymbolType());
      assert(STE && "Could not create a valid SymbolTable Entry!");

      Id->SetSymbolTableEntry(STE);

      if (!ASTSymbolTable::Instance().Insert(Id, STE)) {
        std::stringstream M;
        M << "Failure inserting SymbolTable Entry for ASTIdentifierNode "
          << Id->GetName() << " Type " << PrintTypeEnum(Id->GetSymbolType())
          << '.';
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
        return nullptr;
      }
    }
  }

  if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()) &&
      Id->GetBits() == 0U)
    Id->SetBits(ASTAngleNode::AngleBits);

  if (!Found)
    ASTIdentifierBuilder::Instance().Append(Id);

  return Id;
}

template<typename __AT, typename __ET>
ASTIdentifierRefNode*
ResolveASTIdentifierRef(const ASTToken* TK,
                        const std::string& S, const std::string& US,
                        unsigned IX, ASTType ATy,
                        const ASTDeclarationContext* DCX,
                        const ASTArraySubscriptNode* ASN,
                        const ASTArraySubscriptList* ASL) {
  assert(TK && "Invalid ASTToken argument!");
  assert(DCX && "Invalid ASTDeclarationContext argument!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(US, ATy);
  assert(STE && "Could not obtain a valid ASTSymbolTableEntry!");

  __AT* A = dynamic_cast<__AT*>(STE->GetValue()->GetValue<ASTArrayNode*>());
  assert(A && "Could not dynamic_cast to a valid array node!");

  __ET* E = A->GetElement(IX);
  assert(E && "Could not obtain a valid array element!");

  unsigned Bits = static_cast<unsigned>(~0x0);

  switch (A->GetElementType()) {
  case ASTTypeBool:
  case ASTTypeInt:
  case ASTTypeUInt:
  case ASTTypeFloat:
  case ASTTypeDouble:
  case ASTTypeLongDouble:
  case ASTTypeDuration:
  case ASTTypeOpenPulseFrame:
  case ASTTypeOpenPulsePort:
  case ASTTypeOpenPulseWaveform:
  case ASTTypeAngle:
  case ASTTypeBitset:
  case ASTTypeQubit:
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias:
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
  case ASTTypeMPDecimal:
  case ASTTypeMPComplex:
    Bits = A->GetElementSize();
    break;
  default: {
    std::stringstream M;
    M << "Invalid array type " << PrintTypeEnum(A->GetElementType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
    return ASTIdentifierRefNode::IdentifierError(M.str());
  }
    break;
  }

  assert(!ASTIdentifierNode::InvalidBits(Bits) &&
         "Invalid number of bits for array element!");

  ASTSymbolTableEntry* XSTE =
    ASTSymbolTable::Instance().Lookup(S, Bits, A->GetElementType());

  if (XSTE && XSTE->GetIdentifier()->IsReference()) {
    if (ASN->IsInductionVariable()) {
      ASTIdentifierRefNode* IdR =
        new ASTIdentifierRefNode(US, S, A->GetElementType(),
                                 XSTE->GetIdentifier(), IX, true,
                                 XSTE, ASN, ASL);
      assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
      IdR->SetSymbolTableEntry(XSTE);
      IdR->SetPolymorphicName(S);
      IdR->SetDeclarationContext(DCX);
      IdR->SetMangledName(ASTMangler::MangleIdentifier(IdR));
      return IdR;
    }

    return dynamic_cast<ASTIdentifierRefNode*>(XSTE->GetIdentifier());
  }

  ASTIdentifierRefNode* IdR =
    new ASTIdentifierRefNode(US, S, A->GetElementType(), A->GetIdentifier(),
                             IX, true, STE, ASN, ASL);
  assert(IdR && "Could not create a valid ASTIdentifierRefNode!");

  IdR->SetPolymorphicName(S);
  IdR->SetDeclarationContext(DCX);
  IdR->SetMangledName(ASTMangler::MangleIdentifier(IdR));
  IdR->SetArraySubscriptNode(ASN);
  IdR->SetArraySubscriptList(ASL);

  XSTE = new ASTSymbolTableEntry(IdR, A->GetElementType());
  assert(XSTE && "Could not create a valid ASTSymbolTableEntry!");

  XSTE->SetContext(DCX);

  if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DCX))
    XSTE->SetGlobalScope();
  else
    XSTE->SetLocalScope();

  XSTE->SetValue(new ASTValue<>(E, A->GetElementType()), A->GetElementType(), true);
  IdR->SetSymbolTableEntry(XSTE);

  if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DCX)) {
    if (!ASTSymbolTable::Instance().InsertGlobal(IdR, XSTE)) {
      std::stringstream M;
      M << "Failure inserting identifier reference into the "
        << "Global SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(A->GetIdentifier()), M.str(),
                                                DiagLevel::ICE);
      return ASTIdentifierRefNode::IdentifierError(M.str());
    }
  } else {
    if (!ASTSymbolTable::Instance().Insert(IdR, XSTE)) {
      std::stringstream M;
      M << "Failure inserting identifier reference into the "
        << "Local SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(A->GetIdentifier()), M.str(),
                                                DiagLevel::Error);
      return ASTIdentifierRefNode::IdentifierError(M.str());
    }
  }

  return IdR;
}

ASTIdentifierRefNode*
ASTTypeDiscovery::ResolveASTIdentifierRef(const ASTToken* TK,
                                          const std::string& S,
                                          const ASTArraySubscriptNode* ASN,
                                          const ASTArraySubscriptList* ASL) const {
  assert(TK && "Invalid ASTToken argument!");

  if (S.empty()) {
    std::stringstream M;
    M << "Invalid (empty) identifier name.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(ASN), M.str(), DiagLevel::Error);
    return ASTIdentifierRefNode::IdentifierError(M.str());
  }

  // FIXME: REMOVE THIS DIAGNOSTIC WHEN N-DIMENSIONAL ARRAYS ARE IMPLEMENTED.
  if (ASL && !ASL->Empty()) {
    std::stringstream M;
    M << "N-Dimensional array subscripts are not supported yet.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(ASN), M.str(), DiagLevel::Error);
    return ASTIdentifierRefNode::IdentifierError(M.str());
  }

  ASTType CTy = ASTIdentifierTypeController::Instance().GetCurrentType();
  ASTType PTy = ASTIdentifierTypeController::Instance().GetPreviousType();

  const ASTDeclarationContext* DC =
    ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(DC && "Could not obtain a valid ASTDeclarationContext!");

  if (CTy == ASTTypeUndefined) {
    switch (PTy) {
    case ASTTypeAngle:
      CTy = ASTTypeAngleArray;
      break;
    case ASTTypeBitset:
      CTy = ASTTypeCBitArray;
      break;
    case ASTTypeBool:
      CTy = ASTTypeBoolArray;
      break;
    case ASTTypeInt:
      CTy = ASTTypeIntArray;
      break;
    case ASTTypeFloat:
      CTy =ASTTypeFloatArray;
      break;
    case ASTTypeMPInteger:
      CTy = ASTTypeMPIntegerArray;
      break;
    case ASTTypeMPDecimal:
      CTy = ASTTypeMPDecimalArray;
      break;
    case ASTTypeMPComplex:
      CTy = ASTTypeMPComplexArray;
      break;
    case ASTTypeDuration:
      CTy = ASTTypeDurationArray;
      break;
    case ASTTypeLength:
      CTy = ASTTypeLengthArray;
      break;
    case ASTTypeQubit:
    case ASTTypeQubitContainer:
      CTy = ASTTypeQubitArray;
      break;
    case ASTTypeOpenPulseFrame:
      CTy = ASTTypeOpenPulseFrameArray;
      break;
    case ASTTypeOpenPulsePort:
      CTy = ASTTypeOpenPulsePortArray;
    default:
      break;
    }
  }

  std::string US;
  std::string IS;
  unsigned IX = ASN->GetUnsignedIndexValue();

  if (ASTStringUtils::Instance().IsIndexed(S)) {
    US = ASTStringUtils::Instance().GetIdentifierBase(S);
    IS = S;
  } else {
    US = S;
    std::stringstream ISS;
    ISS << US << '[' << IX << ']';
    IS = ISS.str();
  }

  if (CTy == ASTTypeQReg)
    CTy = ASTTypeQubitContainer;
  else if (CTy == ASTTypeCReg)
    CTy = ASTTypeBitset;

  ASTIdentifierNode* Id = ASTBuilder::Instance().FindASTIdentifierNode(US);
  if (Id) {
    CTy = Id->GetSymbolType();
    DC = Id->GetDeclarationContext();
  }

  switch (CTy) {
  case ASTTypeCBitArray: {
    return QASM::ResolveASTIdentifierRef<ASTCBitArrayNode, ASTCBitNode>(
                                         TK, IS, US, IX, ASTTypeCBitArray, DC,
                                         ASN, ASL);
  }
    break;
  case ASTTypeCBitNArray: {
    assert(0 && "ASTTypeCBitNArray SHOULD NEVER BE USED!");
    return QASM::ResolveASTIdentifierRef<ASTCBitNArrayNode, ASTCBitNode>(
                                         TK, IS, US, IX, ASTTypeCBitArray, DC,
                                         ASN, ASL);
  }
    break;
  case ASTTypeQubitArray: {
    return QASM::ResolveASTIdentifierRef<ASTQubitArrayNode, ASTQubitContainerNode>(
                                         TK, IS, US, IX, ASTTypeQubitArray, DC,
                                         ASN, ASL);
  }
    break;
  case ASTTypeQubitNArray: {
    assert(0 && "ASTTypeQubitNArray SHOULD NEVER BE USED!");
    return QASM::ResolveASTIdentifierRef<ASTQubitNArrayNode, ASTQubitContainerNode>(
                                         TK, IS, US, IX, ASTTypeQubitArray, DC,
                                         ASN, ASL);
  }
    break;
  case ASTTypeAngleArray: {
    return QASM::ResolveASTIdentifierRef<ASTAngleArrayNode, ASTAngleNode>(
                                         TK, IS, US, IX, ASTTypeAngleArray, DC,
                                         ASN, ASL);
  }
    break;
  case ASTTypeBoolArray: {
    return QASM::ResolveASTIdentifierRef<ASTBoolArrayNode, ASTBoolNode>(
                                         TK, IS, US, IX, ASTTypeBoolArray, DC,
                                         ASN, ASL);
  }
    break;
  case ASTTypeIntArray: {
    return QASM::ResolveASTIdentifierRef<ASTIntArrayNode, ASTIntNode>(
                                         TK, IS, US, IX, ASTTypeIntArray, DC,
                                         ASN, ASL);
  }
    break;
  case ASTTypeFloatArray: {
    return QASM::ResolveASTIdentifierRef<ASTFloatArrayNode, ASTFloatNode>(
                                         TK, IS, US, IX, ASTTypeFloatArray, DC,
                                         ASN ,ASL);
  }
    break;
  case ASTTypeMPIntegerArray: {
    return QASM::ResolveASTIdentifierRef<ASTMPIntegerArrayNode, ASTMPIntegerNode>(
                                         TK, IS, US, IX, ASTTypeMPIntegerArray,
                                         DC, ASN, ASL);
  }
    break;
  case ASTTypeMPDecimalArray: {
    return QASM::ResolveASTIdentifierRef<ASTMPDecimalArrayNode, ASTMPDecimalNode>(
                                         TK, IS, US, IX, ASTTypeMPDecimalArray,
                                         DC, ASN, ASL);
  }
    break;
  case ASTTypeMPComplexArray: {
    return QASM::ResolveASTIdentifierRef<ASTMPComplexArrayNode, ASTMPComplexNode>(
                                         TK, IS, US, IX, ASTTypeMPComplexArray,
                                         DC, ASN, ASL);
  }
    break;
  case ASTTypeDurationArray: {
    return QASM::ResolveASTIdentifierRef<ASTDurationArrayNode, ASTDurationNode>(
                                         TK, IS, US, IX, ASTTypeDurationArray,
                                         DC, ASN, ASL);
  }
    break;
  case ASTTypeOpenPulseFrameArray: {
    return QASM::ResolveASTIdentifierRef<ASTOpenPulseFrameArrayNode,
                                         OpenPulse::ASTOpenPulseFrameNode>(
                                         TK, IS, US, IX,
                                         ASTTypeOpenPulseFrameArray, DC,
                                         ASN, ASL);
  }
    break;
  case ASTTypeOpenPulsePortArray: {
    return QASM::ResolveASTIdentifierRef<ASTOpenPulsePortArrayNode,
                                         OpenPulse::ASTOpenPulsePortNode>(
                                         TK, IS, US, IX,
                                         ASTTypeOpenPulsePortArray, DC,
                                         ASN, ASL);
  }
    break;
  case ASTTypeAngle: {
    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(US, ASTTypeAngle);
    assert(STE && "Could not obtain a valid ASTSymbolTable Entry!");

    ASTAngleNode* AN = STE->GetValue()->GetValue<ASTAngleNode*>();
    assert(AN && "Could not obtain a valid ASTAngleNode!");

    ASTSymbolTableEntry* ASTE =
      ASTSymbolTable::Instance().Lookup(IS, ASTIntNode::IntBits, ASTTypeAngle);
    if (ASTE)
      return dynamic_cast<ASTIdentifierRefNode*>(ASTE->GetIdentifier());

    ASTIdentifierRefNode* IdR =
      new ASTIdentifierRefNode(IS, AN->GetIdentifier(), ASTIntNode::IntBits);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");

    IdR->SetBits(ASTIntNode::IntBits);
    IdR->SetMangledName(ASTMangler::MangleIdentifier(IdR));
    IdR->SetArraySubscriptNode(ASN);
    IdR->SetArraySubscriptList(ASL);

    ASTE = new ASTSymbolTableEntry(IdR, AN->GetASTType());
    assert(ASTE && "Could not create a valid ASTSymbolTableEntry!");

    ASTE->SetValue(new ASTValue<>(AN, AN->GetASTType()),
                   AN->GetASTType(), true);

    IdR->SetSymbolTableEntry(ASTE);

    if (!ASTSymbolTable::Instance().Insert(IdR, ASTE)) {
      std::stringstream M;
      M << "Failure inserting into the SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(AN->GetIdentifier()), M.str(),
                                                DiagLevel::Error);
      return ASTIdentifierRefNode::IdentifierError(M.str());
    }

    return IdR;
  }
    break;
  case ASTTypeBitset: {
    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(US, ASTTypeBitset);
    if (!STE) {
      if (ASTOpenQASMVersionTracker::Instance().GetVersion() == 2.0) {
        if (ASTStringUtils::Instance().IsIndexed(S)) {
          unsigned Bits = ASTStringUtils::Instance().GetIdentifierIndex(S);
          ASTIdentifierNode* RId =
            ASTBuilder::Instance().CreateASTIdentifierNode(US, Bits,
                                                           ASTTypeBitset);
          assert(RId && "Could not create a valid ASTIdentifierNode!");
          // FIXME.
        }
      } else {
        std::stringstream M;
        M << "Invalid syntax for the declaration of an ASTTypeBitset.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
        return ASTIdentifierRefNode::IdentifierError(M.str());
      }
    }

    assert(STE && "Could not obtain a valid ASTSymbolTable Entry!");

    ASTCBitNode* CBN = STE->GetValue()->GetValue<ASTCBitNode*>();
    assert(CBN && "Could not obtain a valid ASTCBitNode!");

    ASTSymbolTableEntry* XSTE =
      ASTSymbolTable::Instance().Lookup(IS, 1U, CBN->GetASTType());
    if (XSTE)
      return dynamic_cast<ASTIdentifierRefNode*>(XSTE->GetIdentifier());

    ASTIdentifierRefNode* IdR =
      new ASTIdentifierRefNode(IS, CBN->GetIdentifier(), CBN->GetASTType());
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");

    IdR->SetBits(1U);
    IdR->SetPolymorphicName(US);
    IdR->SetDeclarationContext(DC);
    IdR->SetMangledName(ASTMangler::MangleIdentifier(IdR));
    IdR->SetArraySubscriptNode(ASN);
    IdR->SetArraySubscriptList(ASL);

    XSTE = new ASTSymbolTableEntry(IdR, CBN->GetASTType());
    assert(XSTE && "Could not create a valid ASTSymbolTableEntry!");

    ASTCBitNode* RCBN = new ASTCBitNode(IdR, 1U, 0UL);
    assert(RCBN && "Could not create a valid ASTCBitNode!");

    RCBN->Mangle();
    XSTE->SetContext(DC);

    if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DC))
      XSTE->SetGlobalScope();
    else
      XSTE->SetLocalScope();

    XSTE->SetValue(new ASTValue<>(RCBN, RCBN->GetASTType()),
                   RCBN->GetASTType(), true);

    IdR->SetSymbolTableEntry(XSTE);

    if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DC)) {
      if (!ASTSymbolTable::Instance().InsertGlobal(IdR, XSTE)) {
        std::stringstream M;
        M << "Failure inserting identifier reference into the "
          << "Global SymbolTable.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(CBN->GetIdentifier()), M.str(),
                                                  DiagLevel::ICE);
        return ASTIdentifierRefNode::IdentifierError(M.str());
      }
    } else {
      if (!ASTSymbolTable::Instance().Insert(IdR, XSTE)) {
        std::stringstream M;
        M << "Failure inserting identifier reference into the "
          << "Local SymbolTable.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(CBN->GetIdentifier()), M.str(),
                                                  DiagLevel::ICE);
        return ASTIdentifierRefNode::IdentifierError(M.str());
      }
    }

    return IdR;
  }
    break;
  case ASTTypeQubit: {
    bool QCC = false;
    ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(US, ASTTypeQubit);
    if (!STE) {
      QCC = true;
      STE = ASTSymbolTable::Instance().Lookup(US, ASTTypeQubitContainer);
    }

    assert(STE && "Could not obtain a valid ASTSymbolTableEntry!");

    ASTSymbolTableEntry* XSTE = nullptr;
    ASTQubitNode* QN = nullptr;
    ASTQubitContainerNode* QCN = nullptr;

    if (QCC) {
      QCN = STE->GetValue()->GetValue<ASTQubitContainerNode*>();
      assert(QCN && "Could not obtain a valid ASTQubitContainerNode!");
      std::string QIS = "%";
      QIS += ASTStringUtils::Instance().IndexedIdentifierToQCElement(IS);
      XSTE = ASTSymbolTable::Instance().Lookup(QIS, 1U, ASTTypeQubit);
    } else {
      QN = STE->GetValue()->GetValue<ASTQubitNode*>();
      assert(QN && "Could not obtain a valid ASTQubitNode!");
      XSTE = ASTSymbolTable::Instance().Lookup(IS, 1U, QN->GetASTType());
    }

    if (XSTE) {
      if (XSTE->GetIdentifier()->IsReference())
        return dynamic_cast<ASTIdentifierRefNode*>(XSTE->GetIdentifier());

      if (ASTSymbolTableEntry* RSTE =
          ASTSymbolTable::Instance().Lookup(IS, 1U, ASTTypeQubit)) {
        return dynamic_cast<ASTIdentifierRefNode*>(RSTE->GetIdentifier());
      }

      ASTIdentifierRefNode* IdR =
        new ASTIdentifierRefNode(IS, XSTE->GetIdentifier(), 1U);
      assert(IdR && "Could not create a valid ASTIdentifierRefNode!");

      IdR->SetBits(1U);
      IdR->SetMangledName(ASTMangler::MangleIdentifier(IdR));
      IdR->SetArraySubscriptList(ASL);
      IdR->SetSymbolTableEntry(XSTE);

      if (!ASTSymbolTable::Instance().Insert(IdR, XSTE)) {
        std::stringstream M;
        M << "Failure inserting into the SymbolTable.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(XSTE->GetIdentifier()), M.str(),
                                                  DiagLevel::Error);
        return ASTIdentifierRefNode::IdentifierError(M.str());
      }

      return IdR;
    }

    ASTIdentifierRefNode* IdR = nullptr;
    if (QCC)
      IdR = new ASTIdentifierRefNode(IS, QCN->GetIdentifier(), 1U);
    else
      IdR = new ASTIdentifierRefNode(IS, QN->GetIdentifier(), 1U);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");

    IdR->SetBits(1U);
    IdR->SetMangledName(ASTMangler::MangleIdentifier(IdR));
    IdR->SetArraySubscriptNode(ASN);
    IdR->SetArraySubscriptList(ASL);

    if (QCC) {
      XSTE = new ASTSymbolTableEntry(IdR, QCN->GetASTType());
      assert(XSTE && "Could not create a valid ASTSymbolTableEntry!");

      XSTE->SetValue(new ASTValue<>(QCN, QCN->GetASTType()),
                     QCN->GetASTType(), true);
      IdR->SetSymbolTableEntry(XSTE);
    } else {
      XSTE = new ASTSymbolTableEntry(IdR, QN->GetASTType());
      assert(XSTE && "Could not create a valid ASTSymbolTableEntry!");
      XSTE->SetValue(new ASTValue<>(QN, QN->GetASTType()),
                     QN->GetASTType(), true);
      IdR->SetSymbolTableEntry(XSTE);
    }

    if (!ASTSymbolTable::Instance().Insert(IdR, XSTE)) {
      std::stringstream M;
      M << "Failure inserting into the SymbolTable.";
      if (QCC)
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(QCN->GetIdentifier()), M.str(),
                                                  DiagLevel::Error);
      else
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(QN->GetIdentifier()), M.str(),
                                                  DiagLevel::Error);
      return ASTIdentifierRefNode::IdentifierError(M.str());
    }

    return IdR;
  }
    break;
  case ASTTypeQubitContainer: {
    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(US, ASTTypeQubitContainer);
    if (!STE) {
      STE = ASTSymbolTable::Instance().Lookup(IS, 1U, ASTTypeQubitContainer);
      if (STE && STE->GetIdentifier()->IsReference()) {
        if (ASN->IsInductionVariable() || ASN->IsIndexIdentifier()) {
          ASTIdentifierRefNode* IdR =
            new ASTIdentifierRefNode(US, IS, ASTTypeQubitContainer,
                                     STE->GetIdentifier(), IX, true,
                                     STE, ASN, ASL);
          assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
          IdR->SetSymbolTableEntry(STE);
          return IdR;
        }

        return dynamic_cast<ASTIdentifierRefNode*>(STE->GetIdentifier());
      } else {
        STE = ASTSymbolTable::Instance().Lookup(IS, 1U, ASTTypeQubit);
        if (STE && STE->GetIdentifier()->IsReference()) {
          if (ASN->IsInductionVariable() || ASN->IsIndexIdentifier()) {
            ASTIdentifierRefNode* IdR =
              new ASTIdentifierRefNode(US, IS, ASTTypeQubitContainer,
                                       STE->GetIdentifier(), IX, true,
                                       STE, ASN, ASL);
            assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
            IdR->SetSymbolTableEntry(STE);
            return IdR;
          }

          return dynamic_cast<ASTIdentifierRefNode*>(STE->GetIdentifier());
        }
      }
    } else {
      STE = ASTSymbolTable::Instance().Lookup(IS, 1U, ASTTypeQubitContainer);
      if (STE && STE->GetIdentifier()->IsReference()) {
        if (ASN->IsInductionVariable() || ASN->IsIndexIdentifier()) {
          ASTIdentifierRefNode* IdR =
            new ASTIdentifierRefNode(US, IS, ASTTypeQubitContainer,
                                     STE->GetIdentifier(), IX, true,
                                     STE, ASN, ASL);
          assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
          IdR->SetSymbolTableEntry(STE);
          return IdR;
        }

        return dynamic_cast<ASTIdentifierRefNode*>(STE->GetIdentifier());
      } else {
        STE = ASTSymbolTable::Instance().Lookup(IS, 1U, ASTTypeQubit);
        if (STE && STE->GetIdentifier()->IsReference()) {
          if (ASN->IsInductionVariable() || ASN->IsIndexIdentifier()) {
            ASTIdentifierRefNode* IdR =
              new ASTIdentifierRefNode(US, IS, ASTTypeQubitContainer,
                                       STE->GetIdentifier(), IX, true,
                                       STE, ASN, ASL);
            assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
            IdR->SetSymbolTableEntry(STE);
            return IdR;
          }

          return dynamic_cast<ASTIdentifierRefNode*>(STE->GetIdentifier());
        }
      }
    }

    Id = ASTBuilder::Instance().CreateASTIdentifierNode(US, IX,
                                                        ASTTypeQubitContainer);
    assert(Id && "Could not create a valid ASTIdentifierNode!");
    assert(Id->GetSymbolTableEntry() &&
           "ASTIdentifierNode without an ASTSymbolTableEntry!");

    ASTIdentifierRefNode* XIdR =
      new ASTIdentifierRefNode(US, IS, ASTTypeQubitContainer, Id, IX,
                               true, Id->GetSymbolTableEntry(), ASN, ASL);
    assert(XIdR && "Could not create a valid ASTIdentifierRefNode!");

    XIdR->SetMangledName(ASTMangler::MangleIdentifier(XIdR));
    if (!ASTSymbolTable::Instance().Insert(Id, Id->GetSymbolTableEntry())) {
      std::stringstream M;
      M << "Failure inserting into the SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return ASTIdentifierRefNode::IdentifierError(M.str());
    }

    return XIdR;
  }
    break;
  case ASTTypeQubitContainerAlias: {
    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(US, ASTTypeQubitContainerAlias);
    assert(STE && "Could not obtain a valid ASTSymbolTableEntry!");

    ASTQubitContainerAliasNode* QCN =
      STE->GetValue()->GetValue<ASTQubitContainerAliasNode*>();
    assert(QCN && "Could not obtain a valid ASTQubitContainerAliasNode!");

    ASTSymbolTableEntry* XSTE =
      ASTSymbolTable::Instance().Lookup(IS, 1U, QCN->GetASTType());
    if (XSTE)
      return dynamic_cast<ASTIdentifierRefNode*>(XSTE->GetIdentifier());

    ASTIdentifierRefNode* IdR =
      new ASTIdentifierRefNode(IS, QCN->GetIdentifier(), QCN->GetASTType());
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");

    IdR->SetBits(1U);
    IdR->SetMangledName(ASTMangler::MangleIdentifier(IdR));
    IdR->SetArraySubscriptNode(ASN);
    IdR->SetArraySubscriptList(ASL);

    XSTE = new ASTSymbolTableEntry(IdR, QCN->GetASTType());
    assert(XSTE && "Could not create a valid ASTSymbolTableEntry!");

    XSTE->SetValue(new ASTValue<>(QCN, QCN->GetASTType()),
                   QCN->GetASTType(), true);

    IdR->SetSymbolTableEntry(XSTE);

    if (!ASTSymbolTable::Instance().Insert(IdR, XSTE)) {
      std::stringstream M;
      M << "Failure inserting into the SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(QCN->GetIdentifier()), M.str(),
                                                DiagLevel::Error);
      return ASTIdentifierRefNode::IdentifierError(M.str());
    }

    return IdR;
  }
    break;
  default: {
    std::stringstream M;
    if (!Id) {
      M << "Unknown Identifier '" << US << "'.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    } else {
      M << "Type " << PrintTypeEnum(CTy) << " cannot be indexed "
        << "with C-style array index operator.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    }

    return ASTIdentifierRefNode::IdentifierError(M.str());
  }
    break;
  }

  std::stringstream M;
  M << "Failure resolving a valid ASTIdentifierRefNode.";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
    DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::Error);
  return ASTIdentifierRefNode::IdentifierError(M.str());
}

void ASTTypeDiscovery::ValidateGateQubitArgs(const ASTAnyTypeList* ATL,
                                             const ASTGateNode* G) const {
  assert(ATL && "Invalid ASTAnyTypeList argument!");
  assert(G && "Invalid ASTGateNode argument!");

  if ((G->GetNumQubits() == 0 || G->GetNumQCParams() == 0) ||
      (G->GetNumQubits() > ATL->Size() && G->GetNumQCParams() > ATL->Size())) {
    std::stringstream M;
    M << "Number of Gate Qubit params (" << G->GetName()
      << ") does not match the number of Qubit arguments provided ("
      << G->GetNumQubits() << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(G), M.str(), DiagLevel::Warning);
  }
}

void
ASTTypeDiscovery::ValidateDefcalQubitArgs(const ASTAnyTypeList* ATL,
                                          const ASTDefcalGroupNode* DG) const {
  assert(ATL && "Invalid ASTAnyTypeList argument!");
  assert(DG && "Invalid ASTDefcalGroupNode argument!");

  for (unsigned I = 0; I < DG->Size(); ++I) {
    const ASTDefcalNode* D = DG->GetDefcalNode(I);
    assert(D && "Could not obtain a valid ASTDefcalNode!");

    if (D->GetQubitTargets().Size() != ATL->Size()) {
      std::stringstream M;
      M << "Number of Defcal Qubit arguments does not match the number "
        << "of Qubit arguments provided.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(D), M.str(), DiagLevel::Warning);
    }
  }
}

} // namespace QASM

