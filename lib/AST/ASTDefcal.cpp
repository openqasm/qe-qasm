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

#include <qasm/AST/ASTArgument.h>
#include <qasm/AST/ASTDefcal.h>
#include <qasm/AST/ASTDefcalBuilder.h>
#include <qasm/AST/ASTDefcalContextBuilder.h>
#include <qasm/AST/ASTCalContextBuilder.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTDefcalGrammarBuilder.h>
#include <qasm/AST/ASTDefcalStatementBuilder.h>
#include <qasm/AST/ASTObjectTracker.h>
#include <qasm/AST/ASTAngleNodeBuilder.h>
#include <qasm/AST/ASTGateQubitTracker.h>
#include <qasm/AST/ASTFunctionCallArgument.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTUtils.h>
#include <qasm/AST/ASTImplicitConversionExpr.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <vector>
#include <string>
#include <cassert>
#include <cctype>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

unsigned ASTDefcalNode::QIC = 0U;
ASTDefcalNodeList ASTDefcalBuilder::DL;
ASTDefcalNodeMap ASTDefcalBuilder::DM;
ASTDefcalBuilder ASTDefcalBuilder::DB;
uint32_t ASTDefcalBuilder::DC = 0U;
bool ASTDefcalBuilder::DCS = false;

ASTDefcalContextBuilder ASTDefcalContextBuilder::DCB;
bool ASTDefcalContextBuilder::DCS;

ASTCalContextBuilder ASTCalContextBuilder::CCB;
bool ASTCalContextBuilder::CCS;

ASTDefcalStatementBuilder ASTDefcalStatementBuilder::DSB;
ASTStatementList ASTDefcalStatementBuilder::DS;

ASTDefcalGrammarBuilder ASTDefcalGrammarBuilder::GB;
std::set<std::string> ASTDefcalGrammarBuilder::GS;
std::string ASTDefcalGrammarBuilder::CG;

ASTExpressionNodeList ASTDefcalParameterBuilder::EV;
ASTDefcalParameterBuilder ASTDefcalParameterBuilder::DPB;

void ASTDefcalBuilder::ValidateDefcalContext() const {
  if (!DCS) {
    std::stringstream M;
    M << "play instruction is being issued outside an "
      << "open defcal context.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  }
}

ASTDefcalGroupNode::ASTDefcalGroupNode(const ASTIdentifierNode* Id,
                             const std::vector<const ASTSymbolTableEntry*>& DGV)
  : ASTExpressionNode(Id, ASTTypeDefcalGroup), DV() {
  for (std::vector<const ASTSymbolTableEntry*>::const_iterator I = DGV.begin();
       I != DGV.end(); ++I) {
    ASTDefcalNode* DN = (*I)->GetValue()->GetValue<ASTDefcalNode*>();
    assert(DN && "Could not obtain a valid ASTDefcalNode!");
    AddDefcal(DN);
  }
}

void ASTDefcalGroupNode::print() const {
  std::cout << "<DefcalGroup>" << std::endl;
  std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
  std::cout << "<MangledName>" << GetName() << "</MangledName>"
    << std::endl;

  if (!DV.empty()) {
    for (ASTDefcalGroupNode::const_iterator DI = begin();
         DI != end(); ++DI)
      (*DI)->print();
  }

  std::cout << "</DefcalGroup>" << std::endl;
}

void ASTDefcalGroupNode::Mangle() {
  ASTMangler M;
  M.Start();

  M.TypeIdentifier(GetASTType(), GetIdentifier()->GetName());
  M.Type(ASTTypeUInt);
  M.StringValue(std::to_string(Size()));
  M.EndExpression();
  M.End();

  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTDefcalGroupOpNode::print() const {
  std::cout << "<DefcalGroupOpNode>" << std::endl;
  ASTGateQOpNode::print();
  std::cout << "</DefcalGroupOpNode>" << std::endl;
}

void ASTDefcalGroupOpNode::Mangle() {
  // FIXME: IMPLEMENT.
}

void ASTDefcalGrammarBuilder::ValidateContext(const std::string& GN) const {
  if (!IsGrammar(GN)) {
    std::stringstream M;
    M << "Grammar " << GN << " is not a valid or known grammar.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Error);
  }

  if (GN != CG) {
    std::stringstream M;
    M << "Grammar " << GN << " is not the current grammar ("
      << CG << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Error);
  }
}

ASTAngleNode*
ASTDefcalNode::CreateAngleConversion(const ASTIdentifierNode* XId,
                                     const ASTSymbolTableEntry* XSTE) const {
  assert(XId && "Invalid ASTIdentifierNode argument!");
  assert(XSTE && "Invalid ASTSymbolTableEntry argument!");

  ASTAngleNode* XAN = nullptr;
  ASTImplicitConversionNode* ICE = nullptr;
  const ASTLocation& LC = XId->GetLocation();

  switch (XSTE->GetValueType()) {
  case ASTTypeFloat: {
    const ASTFloatNode* FN = XSTE->GetValue()->GetValue<ASTFloatNode*>();
    assert(FN && "Could not obtain a valid ASTFloatNode!");

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), FN,
                           ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(FN, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  }
    break;
  case ASTTypeDouble: {
    const ASTDoubleNode* DN = XSTE->GetValue()->GetValue<ASTDoubleNode*>();
    assert(DN && "Could not obtain a valid ASTDoubleNode!");

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), DN,
                           ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(DN, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  }
    break;
  case ASTTypeInt:
  case ASTTypeUInt: {
    const ASTIntNode* IN = XSTE->GetValue()->GetValue<ASTIntNode*>();
    assert(IN && "Could not obtain a valid ASTIntNode!");

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), IN,
                           ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(IN, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  }
    break;
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger: {
    const ASTMPIntegerNode* MPI = XSTE->GetValue()->GetValue<ASTMPIntegerNode*>();
    assert(MPI && "Could not obtain a valid ASTMPIntegerNode!");

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), MPI,
                           ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(MPI, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  }
    break;
  case ASTTypeMPDecimal: {
    const ASTMPDecimalNode* MPD = XSTE->GetValue()->GetValue<ASTMPDecimalNode*>();
    assert(MPD && "Could not obtain a valid ASTMPDecimalNode!");

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), MPD,
                           ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(MPD, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  }
    break;
  case ASTTypeBitset: {
    const ASTCBitNode* CBN = XSTE->GetValue()->GetValue<ASTCBitNode*>();
    assert(CBN && "Could not obtain a valid ASTCBitNode!");

    if (CBN->Size() > XSTE->GetIdentifier()->GetBits()) {
      std::stringstream M;
      M << "Conversion from " << PrintTypeEnum(XSTE->GetValueType())
        << " to Angle Type will result in truncation.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(XId), M.str(), DiagLevel::Warning);
    }

    unsigned SZ = std::min(static_cast<unsigned>(CBN->Size()), XId->GetBits());

    if (SZ >= 4U)
      SZ = SZ % 4;

    double D = 0.0;

    for (unsigned I = 0; I < SZ; ++I) {
      if ((*CBN)[I])
        D += static_cast<double>(M_PI / 2);
    }

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), D,
                           ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(CBN, ASTTypeAngle, SZ);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  }
    break;
  default: {
    std::stringstream M;
    M << "Impossible implicit conversion to Angle Type from "
      << PrintTypeEnum(XSTE->GetValueType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(XId), M.str(), DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  if (ICE && !ICE->IsValidConversion()) {
    std::stringstream M;
    M << "Impossible implicit conversion to Angle Type from "
      << PrintTypeEnum(XSTE->GetValueType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(XId), M.str(), DiagLevel::Error);
    return nullptr;
  }

  XAN->SetLocation(LC);
  XAN->Mangle();
  return XAN;
}

ASTAngleNode*
ASTDefcalNode::CreateAngleConversion(const ASTSymbolTableEntry* XSTE) const {
  assert(XSTE && "Invalid ASTSymbolTableEntry argument!");

  ASTAngleNode* XAN = nullptr;
  ASTImplicitConversionNode* ICE = nullptr;
  const ASTLocation& LC = XSTE->GetIdentifier()->GetLocation();

  switch (XSTE->GetValueType()) {
  case ASTTypeFloat: {
    const ASTFloatNode* FN = XSTE->GetValue()->GetValue<ASTFloatNode*>();
    assert(FN && "Could not obtain a valid ASTFloatNode!");

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), FN,
                           ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(FN, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  }
    break;
  case ASTTypeDouble: {
    const ASTDoubleNode* DN = XSTE->GetValue()->GetValue<ASTDoubleNode*>();
    assert(DN && "Could not obtain a valid ASTDoubleNode!");

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), DN,
                           ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(DN, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  }
    break;
  case ASTTypeInt:
  case ASTTypeUInt: {
    const ASTIntNode* IN = XSTE->GetValue()->GetValue<ASTIntNode*>();
    assert(IN && "Could not obtain a valid ASTIntNode!");

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), IN,
                           ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(IN, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  }
    break;
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger: {
    const ASTMPIntegerNode* MPI = XSTE->GetValue()->GetValue<ASTMPIntegerNode*>();
    assert(MPI && "Could not obtain a valid ASTMPIntegerNode!");

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), MPI,
                           ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(MPI, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  }
    break;
  case ASTTypeMPDecimal: {
    const ASTMPDecimalNode* MPD = XSTE->GetValue()->GetValue<ASTMPDecimalNode*>();
    assert(MPD && "Could not obtain a valid ASTMPDecimalNode!");

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), MPD,
                           ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(MPD, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  }
    break;
  case ASTTypeBitset: {
    const ASTCBitNode* CBN = XSTE->GetValue()->GetValue<ASTCBitNode*>();
    assert(CBN && "Could not obtain a valid ASTCBitNode!");

    if (CBN->Size() > XSTE->GetIdentifier()->GetBits()) {
      std::stringstream M;
      M << "Conversion from " << PrintTypeEnum(XSTE->GetValueType())
        << " to Angle Type will result in truncation.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(XSTE->GetIdentifier()),
        M.str(), DiagLevel::Warning);
    }

    unsigned SZ = std::min(static_cast<unsigned>(CBN->Size()),
                           XSTE->GetIdentifier()->GetBits());

    if (SZ >= 4U)
      SZ = SZ % 4;

    double D = 0.0;

    for (unsigned I = 0; I < SZ; ++I) {
      if ((*CBN)[I])
        D += static_cast<double>(M_PI / 2);
    }

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), D,
                           ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(CBN, ASTTypeAngle, SZ);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  }
    break;
  default: {
    std::stringstream M;
    M << "Impossible implicit conversion to Angle Type from "
      << PrintTypeEnum(XSTE->GetValueType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(XSTE->GetIdentifier()), M.str(),
                                                    DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  if (ICE && !ICE->IsValidConversion()) {
    std::stringstream M;
    M << "Impossible implicit conversion to Angle Type from "
      << PrintTypeEnum(XSTE->GetValueType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(XSTE->GetIdentifier()), M.str(),
                                                    DiagLevel::Error);
    return nullptr;
  }

  ASTObjectTracker::Instance().Unregister(XAN);

  XAN->SetLocation(LC);
  XAN->Mangle();
  return XAN;
}

ASTAngleNode*
ASTDefcalNode::CreateAngleTemporary(const ASTSymbolTableEntry* XSTE) const {
  assert(XSTE && "Invalid ASTSymbolTableEntry argument!");

  ASTAngleNode* XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(),
                                       ASTAngleTypeGeneric,
                                       XSTE->GetIdentifier()->GetBits());
  assert(XAN && "Could not create a valid ASTAngleNode!");

  ASTObjectTracker::Instance().Unregister(XAN);
  return XAN;
}

ASTAngleNode*
ASTDefcalNode::CreateAngleSymbolTableEntry(ASTSymbolTableEntry* XSTE) const {
  assert(XSTE && "Invalid ASTSymbolTableEntry argument!");

  ASTAngleNode* XAN = new ASTAngleNode(XSTE->GetIdentifier(),
                                       ASTAngleTypeGeneric,
                                       XSTE->GetIdentifier()->GetBits());
  assert(XAN && "Could not create a valid ASTAngleNode!");

  XSTE->ResetValue();
  XSTE->SetValue(new ASTValue<>(XAN, ASTTypeAngle), ASTTypeAngle);
  assert(XSTE->HasValue() && "ASTSymbolTable Entry has no Value!");

  return XAN;
}

ASTDefcalNode::ASTDefcalNode(const ASTIdentifierNode* Id)
  : ASTStatementNode(Id),
  Params(), Statements(), Void(nullptr), OTy(ASTTypeUndefined),
  DSTM(), QTarget(new ASTBoundQubitList()), QIL(), QK(Bound), Grammar(),
  Name(), IsCall(false) {
    Grammar = ASTDefcalGrammarBuilder::Instance().GetCurrent();
    ResolveQubits();
}

ASTDefcalNode::ASTDefcalNode(const ASTIdentifierNode* Id,
                             const ASTExpressionNodeList& EL,
                             const ASTStatementList& SL,
                             const ASTBoundQubitList* QT)
  : ASTStatementNode(Id),
  Params(EL), Statements(SL), Void(nullptr), OTy(ASTTypeUndefined),
  DSTM(), QTarget(QT), QIL(), QK(Bound), Grammar(), Name(), IsCall(false) {
    Grammar = ASTDefcalGrammarBuilder::Instance().GetCurrent();
    MangleParams();
    ResolveQubits();
    ResolveParams();
}

ASTDefcalNode::ASTDefcalNode(const ASTIdentifierNode* Id,
                             const std::string& GM,
                             const ASTExpressionNodeList& EL,
                             const ASTStatementList& SL,
                             const ASTBoundQubitList* QT)
  : ASTStatementNode(Id),
  Params(EL), Statements(SL), Void(nullptr), OTy(ASTTypeUndefined),
  DSTM(), QTarget(QT), QIL(), QK(Bound), Grammar(GM), Name(), IsCall(false) {
    MangleParams();
    ResolveQubits();
    ResolveParams();
}

ASTDefcalNode::ASTDefcalNode(const ASTIdentifierNode* Id,
                             const ASTStatementList& SL,
                             const ASTMeasureNode* M)
  : ASTStatementNode(Id),
  Params(), Statements(SL), Measure(M), OTy(M->GetASTType()),
  DSTM(), QTarget(new ASTBoundQubitList()), QIL(), QK(Bound), Grammar(),
  Name(), IsCall(false) {
    Grammar = ASTDefcalGrammarBuilder::Instance().GetCurrent();
    const std::string& S = M->GetTarget()->GetIdentifier()->GetName();
    const_cast<ASTBoundQubitList*>(QTarget)->Append(new ASTStringNode(S));
    ResolveQubits();
    const_cast<ASTMeasureNode*>(M)->Mangle();
    ResolveMeasure();
}

ASTDefcalNode::ASTDefcalNode(const ASTIdentifierNode* Id,
                             const std::string& GM,
                             const ASTStatementList& SL,
                             const ASTMeasureNode* M)
  : ASTStatementNode(Id),
  Params(), Statements(SL), Measure(M), OTy(M->GetASTType()),
  DSTM(), QTarget(new ASTBoundQubitList()), QIL(), QK(Bound), Grammar(GM),
  Name(), IsCall(false) {
    const std::string& S = M->GetTarget()->GetIdentifier()->GetName();
    const_cast<ASTBoundQubitList*>(QTarget)->Append(new ASTStringNode(S));
    ResolveQubits();
    const_cast<ASTMeasureNode*>(M)->Mangle();
    ResolveMeasure();
}

ASTDefcalNode::ASTDefcalNode(const ASTIdentifierNode* Id,
                             const ASTStatementList& SL,
                             const ASTResetNode* R)
  : ASTStatementNode(Id),
  Params(), Statements(SL), Reset(R), OTy(R->GetASTType()),
  DSTM(), QTarget(new ASTBoundQubitList()), QIL(), QK(Bound), Grammar(),
  Name(), IsCall(false) {
    Grammar = ASTDefcalGrammarBuilder::Instance().GetCurrent();
    const std::string& S = R->GetTarget()->GetName();
    const_cast<ASTBoundQubitList*>(QTarget)->Append(new ASTStringNode(S));
    ResolveQubits();
    const_cast<ASTResetNode*>(R)->Mangle();
    ResolveReset();
}

ASTDefcalNode::ASTDefcalNode(const ASTIdentifierNode* Id,
                             const std::string& GM,
                             const ASTStatementList& SL,
                             const ASTResetNode* R)
  : ASTStatementNode(Id),
  Params(), Statements(SL), Reset(R), OTy(R->GetASTType()),
  DSTM(), QTarget(new ASTBoundQubitList()), QIL(), QK(Bound), Grammar(GM),
  Name(), IsCall(false) {
    const std::string& S = R->GetTarget()->GetName();
    const_cast<ASTBoundQubitList*>(QTarget)->Append(new ASTStringNode(S));
    ResolveQubits();
    const_cast<ASTResetNode*>(R)->Mangle();
    ResolveReset();
}

ASTDefcalNode::ASTDefcalNode(const ASTIdentifierNode* Id,
                             const ASTDelayNode* D,
                             const ASTStatementList& SL,
                             const ASTBoundQubitList* QL)
  : ASTStatementNode(Id),
  Params(), Statements(SL), Delay(D), OTy(D->GetASTType()),
  DSTM(), QTarget(QL ? QL : new ASTBoundQubitList()), QIL(), QK(Bound),
  Grammar(), Name(), IsCall(false) {
    Grammar = ASTDefcalGrammarBuilder::Instance().GetCurrent();
    ResolveQubits();
    const_cast<ASTDelayNode*>(D)->Mangle();
}

ASTDefcalNode::ASTDefcalNode(const ASTIdentifierNode* Id,
                             const std::string& GM,
                             const ASTDelayNode* D,
                             const ASTStatementList& SL,
                             const ASTBoundQubitList* QL)
  : ASTStatementNode(Id),
  Params(), Statements(SL), Delay(D), OTy(D->GetASTType()),
  DSTM(), QTarget(QL ? QL : new ASTBoundQubitList()), QIL(), QK(Bound),
  Grammar(GM), Name(), IsCall(false) {
    ResolveQubits();
    const_cast<ASTDelayNode*>(D)->Mangle();
}


ASTDefcalNode::ASTDefcalNode(const ASTIdentifierNode* Id,
                             const ASTDurationOfNode* D,
                             const ASTStatementList& SL,
                             const ASTBoundQubitList* QL)
  : ASTStatementNode(Id),
  Params(), Statements(SL), Duration(D), OTy(D->GetASTType()),
  DSTM(), QTarget(QL ? QL : new ASTBoundQubitList()), QIL(), QK(Bound),
  Grammar(), Name(), IsCall(false) {
    Grammar = ASTDefcalGrammarBuilder::Instance().GetCurrent();
    ResolveQubits();
    const_cast<ASTDurationOfNode*>(D)->Mangle();
}

ASTDefcalNode::ASTDefcalNode(const ASTIdentifierNode* Id,
                             const std::string& GM,
                             const ASTDurationOfNode* D,
                             const ASTStatementList& SL,
                             const ASTBoundQubitList* QL)
  : ASTStatementNode(Id),
  Params(), Statements(SL), Duration(D), OTy(D->GetASTType()),
  DSTM(), QTarget(QL ? QL : new ASTBoundQubitList()), QIL(), QK(Bound),
  Grammar(GM), Name(), IsCall(false) {
    ResolveQubits();
    const_cast<ASTDurationOfNode*>(D)->Mangle();
}

void ASTDefcalNode::ResolveQubits() {
  if (!QTarget->Empty()) {
    unsigned X = 0U;

    for (ASTBoundQubitList::const_iterator I = QTarget->begin();
         I != QTarget->end(); ++I) {
      const std::string& QIR = (*I)->GetValue();
      const ASTSymbolTableEntry* QSTE = nullptr;

      if (ASTStringUtils::Instance().IsIndexed(QIR)) {
        QSTE = ASTSymbolTable::Instance().FindQubit(
          ASTStringUtils::Instance().IndexedIdentifierToQCElement(QIR));
        assert(QSTE && "Qubit has no valid SymbolTable Entry!");
      } else {
        QSTE = ASTSymbolTable::Instance().FindQubit(QIR);
        assert(QSTE && "Qubit has no valid SymbolTable Entry!");
      }

      if (!QSTE) {
        std::stringstream M;
        M << "Qubit " << QIR.c_str() << " has no SymbolTable Entry!";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                     DiagLevel::Error);
        return;
      }

      const ASTIdentifierNode* QId = QSTE->GetIdentifier();
      if (!QId) {
        std::stringstream M;
        M << "Could not resolve Bound Qubit " << (*I)->GetValue()
          << " at Index " << X << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                     DiagLevel::Error);
        return;
      }

      QId->SetGateParamName((*I)->GetValue());
      QIL.Append(const_cast<ASTIdentifierNode*>(QId));
      ToDefcalParamSymbolTable(QId, QSTE);
      ++X;
    }
  }
}

void ASTDefcalNode::ResolveMeasure() {
  if (OTy == ASTTypeMeasure && Measure) {
    const ASTIdentifierNode* MId = Measure->GetIdentifier();
    assert(MId && "Invalid ASTMeasureNode Identifier!");

    const ASTSymbolTableEntry* MSTE = ASTSymbolTable::Instance().FindLocal(MId);
    assert(MSTE && "Invalid Defcal Measure without an ASTSymbolTable Entry!");

    ASTMeasureNode* MN = MSTE->GetValue()->GetValue<ASTMeasureNode*>();
    assert(MN && "Invalid ASTMeasureNode obtained from the ASTSymbolTable!");

    const ASTIdentifierNode* RId = nullptr;
    const ASTSymbolTableEntry* RSTE = nullptr;
    const ASTDeclarationContext* DCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();

    switch (MN->GetResultType()) {
    case ASTTypeBitset: {
      ASTCBitNode* CBN = MN->GetResult();
      assert(CBN && "Invalid ASTCBitNode Measure Result!");

      RId = CBN->GetIdentifier();
      RId->SetDeclarationContext(DCX);
      CBN->SetDeclarationContext(DCX);
      RSTE = ASTSymbolTable::Instance().FindLocal(RId);
      assert(RSTE && "Invalid ASTSymbolTableEntry obtained for ASTCBitNode!");

      const_cast<ASTSymbolTableEntry*>(RSTE)->SetContext(DCX);
    }
      break;
    case ASTTypeAngle: {
      ASTAngleNode* AN = MN->GetAngleResult();
      assert(AN && "Invalid ASTAngleNode Measure Result!");

      RId = AN->GetIdentifier();
      RId->SetDeclarationContext(DCX);
      AN->SetDeclarationContext(DCX);
      RSTE = ASTSymbolTable::Instance().FindLocal(RId);
      assert(RSTE && "Invalid ASTSymbolTableEntry obtained for ASTAngleNode!");

      const_cast<ASTSymbolTableEntry*>(RSTE)->SetContext(DCX);
    }
      break;
    case ASTTypeMPComplex: {
      ASTMPComplexNode* MPC = MN->GetComplexResult();
      assert(MPC && "Invalid ASTMPComplexNode Measure Result!");

      RId = MPC->GetIdentifier();
      RId->SetDeclarationContext(DCX);
      MPC->SetDeclarationContext(DCX);
      RSTE = ASTSymbolTable::Instance().FindLocal(RId);
      assert(RSTE && "Invalid ASTSymbolTableEntry obtained for ASTMPComplexNode!");

      const_cast<ASTSymbolTableEntry*>(RSTE)->SetContext(DCX);
    }
      break;
    default:
      break;
    }

    if (RId && RSTE) {
      ToDefcalParamSymbolTable(RId, RSTE);
      ASTSymbolTable::Instance().EraseLocalSymbol(RId, RId->GetBits(),
                                                  RId->GetSymbolType());
    }

    ToDefcalParamSymbolTable(MId, MSTE);
    ASTSymbolTable::Instance().EraseLocalSymbol(MId, MId->GetBits(),
                                                MId->GetSymbolType());
  }
}

void ASTDefcalNode::ResolveReset() {
  if (OTy == ASTTypeReset && Reset) {
    const ASTDeclarationContext* DCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();

    const ASTIdentifierNode* RId = Reset->GetIdentifier();
    assert(RId && "Invalid ASTIdentifierNode for Defcal ASTResetNode!");

    const ASTSymbolTableEntry* RSTE = ASTSymbolTable::Instance().FindLocal(RId);
    assert(RSTE && "Invalid Defcal ASTResetNode without an ASTSymbolTable Entry!");

    ASTResetNode* RN = RSTE->GetValue()->GetValue<ASTResetNode*>();
    assert(RN && "Invalid ASTResetNode obtained from the ASTSymbolTable!");

    const_cast<ASTIdentifierNode*>(RId)->SetDeclarationContext(DCX);
    RN->SetDeclarationContext(DCX);
    const_cast<ASTSymbolTableEntry*>(RSTE)->SetContext(DCX);

    ToDefcalParamSymbolTable(RId, RSTE);
    ASTSymbolTable::Instance().EraseLocalSymbol(RId, RId->GetBits(),
                                                RId->GetSymbolType());
  }
}

const ASTExpressionNode*
ASTDefcalNode::AsExpression() const {
  ASTExpressionNode* EXN =
    new ASTExpressionNode(ASTStatementNode::GetIdentifier(), this,
                          ASTTypeDefcal);
  assert(EXN && "Could not create a valid Defcal ASTExpressionNode!");
  return EXN;
}

ASTDefcalNode*
ASTDefcalNode::CloneCall(const ASTIdentifierNode* Id,
                         const ASTArgumentNodeList& AL,
                         const ASTAnyTypeList& QL) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  std::stringstream IDS;
  IDS << "ast-defcal-call-" << Id->GetName()
    << DIAGLineCounter::Instance().GetIdentifierLocation();

  ASTDefcalNode* RDN = new ASTDefcalNode(Id);
  assert(RDN && "Could not create a valid ASTDefcalNode!");
  assert(RDN->QTarget && "Clone ASTDefcalNode has an invalid QTarget!");

  RDN->SetDefcalCall(true);
  RDN->Grammar = Grammar;
  RDN->QK = QK;
  *const_cast<ASTBoundQubitList*>(RDN->QTarget) = *QTarget;
  RDN->OTy = OTy;

  switch (OTy) {
  case ASTTypeMeasure:
    RDN->Measure = Measure;
    break;
  case ASTTypeReset:
    RDN->Reset = Reset;
    break;
  case ASTTypeDelay:
    RDN->Delay = Delay;
    break;
  case ASTTypeDurationOf:
    RDN->Duration = Duration;
    break;
  default:
    RDN->Void = Void;
    break;
  }

  RDN->Statements = Statements;
  RDN->CloneArgumentsFrom(AL);
  RDN->CloneQubitsFrom(QL);

  RDN->Mangle();
  return RDN;
}

void
ASTDefcalNode::CloneArgumentsFrom(const ASTArgumentNodeList& AL) {
  if (AL.Empty())
    return;

  unsigned C = 0U;
  ASTType Ty;

  for (ASTArgumentNodeList::const_iterator I = AL.begin(); I != AL.end(); ++I) {
    Ty = (*I)->GetValueType();
    ASTSymbolTableEntry* ASTE = nullptr;

    switch (Ty) {
    case ASTTypeExpression: {
      try {
        const ASTExpression* EN =
          std::any_cast<const ASTExpression*>((*I)->GetValue());
        (void) EN; // Quiet.
      } catch (const std::bad_any_cast& E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch ( ... ) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    }
      break;
    case ASTTypeIdentifier: {
      try {
        const ASTExpressionNode* EN =
          std::any_cast<const ASTExpressionNode*>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTIdentifierNode* ID = EN->DynCast<const ASTIdentifierNode>();
        assert(ID && "Failed to dynamic_cast to an IdentifierNode!");

        const ASTSymbolTableEntry* XSTE =
          ASTSymbolTable::Instance().FindAngle(ID->GetName());
        if (!XSTE)
          XSTE = ASTSymbolTable::Instance().FindLocal(ID->GetName());
        if (!XSTE)
          XSTE = ASTSymbolTable::Instance().FindGlobal(ID->GetName());

        if (!XSTE) {
          std::stringstream M;
          M << "Non-existent angle " << ID->GetName() << " passed as "
            << "defcal angle parameter.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(ID), M.str(),
                                                         DiagLevel::Error);
        }

        ASTAngleNode* XAN = nullptr;
        bool IMPC = false;

        if (XSTE->HasValue()) {
          if (ASTUtils::Instance().IsAngleType(XSTE->GetValueType()) ||
              ASTUtils::Instance().IsReservedAngleType(XSTE->GetValueType())) {
            XAN = XSTE->GetValue()->GetValue<ASTAngleNode*>();
          } else {
            XAN = CreateAngleConversion(XSTE);
            IMPC = true;
          }
        } else {
          XAN = CreateAngleTemporary(XSTE);
          IMPC = true;
        }

        assert(XAN && "Could not obtain a valid ASTAngleNode!");

        ASTAngleNode* AN = nullptr;
        ASTIdentifierNode* AId = nullptr;

        std::stringstream SAN;
        SAN << ASTDemangler::TypeName(ASTTypeAngle) << C;
        AId = ASTBuilder::Instance().CreateASTIdentifierNode(SAN.str(),
                                     ASTAngleNode::AngleBits, ASTTypeAngle);
        assert(AId && "Could not create an Angle ASTIdentifierNode!");

        ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
        AN = ASTBuilder::Instance().CreateASTAngleNode(AId, ATy,
                                                       ASTAngleNode::AngleBits);
        assert(AN && "Could not create a valid ASTAngleNode!");

        XAN->Clone(AN);
        AN->SetGateParamName(ID->GetName());
        AN->SetExpression(ID);
        AId->SetPolymorphicName(AId->GetName());
        ASTE = ASTSymbolTable::Instance().Lookup(AId, ASTAngleNode::AngleBits,
                                                 ASTTypeAngle);
        assert(ASTE && "ASTAngleNode ASTIdentifierNode has no SymbolTable Entry!");

        AId->SetLocalScope();
        ASTE->SetLocalScope();
        AId->SetSymbolTableEntry(ASTE);
        AN->Mangle();
        AN->MangleLiteral();
        ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                       AId->GetSymbolType());
        ToDefcalParamSymbolTable(AId, ASTE);
        ASTSymbolTable::Instance().EraseLocalAngle(AId);
        ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                              AId->GetSymbolType());

        ASTAngleNodeBuilder::Instance().Insert(AN);
        ASTAngleNodeBuilder::Instance().Append(AN);
        Params.Append(AN);

        if (XAN && IMPC)
          delete XAN;
      } catch (const std::bad_any_cast& E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch ( ... ) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    }
      break;
    case ASTTypeBinaryOp: {
      try {
        const ASTExpressionNode* EN =
          std::any_cast<const ASTExpressionNode*>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTBinaryOpNode* BOP = EN->DynCast<const ASTBinaryOpNode>();
        assert(BOP && "Failed to dynamic_cast to a BinaryOpNode!");

        const ASTIdentifierNode* BId = BOP->GetIdentifier();
        assert(BId && "Invalid ASTIdentifierNode for ASTBinaryOpNode!");

        ASTAngleNode* AN = ASTAngleNodeMap::Instance().Find(BId->GetName());

        if (!AN) {
          std::stringstream IAN;
          IAN << ASTDemangler::TypeName(BId->GetSymbolType()) << C;
          ASTIdentifierNode* AId =
            ASTBuilder::Instance().CreateASTIdentifierNode(IAN.str(),
                                                           BId->GetBits(),
                                                           ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, BOP, ATy,
                                                         AId->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(IAN.str());
          AId->SetPolymorphicName(IAN.str());
          AN->Mangle();
          AN->MangleLiteral();
          AId->GetSymbolTableEntry()->ResetValue();
          AId->GetSymbolTableEntry()->SetValue(new ASTValue<>(AN, ASTTypeAngle),
                                               ASTTypeAngle);
          ToDefcalParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
        }

        Params.Append(AN);
      } catch (const std::bad_any_cast& E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch ( ... ) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    }
      break;
    case ASTTypeUnaryOp: {
      try {
        const ASTExpressionNode* EN =
          std::any_cast<const ASTExpressionNode*>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTUnaryOpNode* UOP = EN->DynCast<const ASTUnaryOpNode>();
        assert(UOP && "Failed to dynamic_cast to an UnaryOpNode!");

        const ASTIdentifierNode* UId = UOP->GetIdentifier();
        assert(UId && "Invalid ASTIdentifierNode for ASTUnaryOpNode!");

        ASTAngleNode* AN = ASTAngleNodeMap::Instance().Find(UId->GetName());

        if (!AN) {
          std::stringstream IAN;
          IAN << ASTDemangler::TypeName(UId->GetSymbolType()) << C;
          ASTIdentifierNode* AId =
            ASTBuilder::Instance().CreateASTIdentifierNode(IAN.str(),
                                                           UId->GetBits(),
                                                           ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, UOP, ATy,
                                                         AId->GetBits());
          assert(AN && "Could not create an ASTAngleNode!!");

          AN->SetGateParamName(IAN.str());
          AId->SetPolymorphicName(IAN.str());
          AN->Mangle();
          AN->MangleLiteral();
          ToDefcalParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
        }

        Params.Append(AN);
      } catch (const std::bad_any_cast& E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch ( ... ) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    }
      break;
    case ASTTypeInt: {
      try {
        const ASTExpressionNode* EN =
          std::any_cast<const ASTExpressionNode*>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTIntNode* INT = dynamic_cast<const ASTIntNode*>(EN);
        assert(INT && "Failed to dynamic_cast to a IntNode!");

        const ASTIdentifierNode* ID = INT->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTIntNode!");

        ASTAngleNode* AN = ASTAngleNodeMap::Instance().Find(ID->GetName());
        if (!AN) {
          std::stringstream IAN;
          IAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode* AId =
            ASTBuilder::Instance().CreateASTIdentifierNode(IAN.str(),
                                                           ID->GetBits(),
                                                           ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, INT, ATy,
                                                         AId->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(IAN.str());
          AId->SetPolymorphicName(IAN.str());
          AN->Mangle();
          AN->MangleLiteral();
          ToDefcalParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
        }

        Params.Append(AN);
      } catch (const std::bad_any_cast& E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch ( ... ) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    }
      break;
    case ASTTypeFloat: {
      try {
        const ASTExpressionNode* EN =
          std::any_cast<const ASTExpressionNode*>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTFloatNode* FLT = dynamic_cast<const ASTFloatNode*>(EN);
        assert(FLT && "Failed to dynamic_cast to a FloatNode!");

        const ASTIdentifierNode* ID = FLT->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTFloatNode!");

        ASTAngleNode* AN = ASTAngleNodeMap::Instance().Find(ID->GetName());
        if (!AN) {
          std::stringstream IAN;
          IAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode* AId =
            ASTBuilder::Instance().CreateASTIdentifierNode(IAN.str(),
                                                           ID->GetBits(),
                                                           ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, FLT, ATy,
                                                         AId->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(IAN.str());
          AId->SetPolymorphicName(IAN.str());
          AN->Mangle();
          AN->MangleLiteral();
          ToDefcalParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
        }

        Params.Append(AN);
      } catch (const std::bad_any_cast& E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch ( ... ) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    }
      break;
    case ASTTypeDouble: {
      try {
        const ASTExpressionNode* EN =
          std::any_cast<const ASTExpressionNode*>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTDoubleNode* DBL = dynamic_cast<const ASTDoubleNode*>(EN);
        assert(DBL && "Failed to dynamic_cast to a DoubleNode!");

        const ASTIdentifierNode* ID = DBL->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTDoubleNode!");

        ASTAngleNode* AN = ASTAngleNodeMap::Instance().Find(ID->GetName(),
                                                            ID->GetBits());
        if (!AN) {
          std::stringstream IAN;
          IAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode* AId =
            ASTBuilder::Instance().CreateASTIdentifierNode(IAN.str(),
                                                           ID->GetBits(),
                                                           ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, DBL, ATy,
                                                         AId->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(IAN.str());
          AId->SetPolymorphicName(IAN.str());
          AN->Mangle();
          AN->MangleLiteral();
          ToDefcalParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
        }

        Params.Append(AN);
      } catch (const std::bad_any_cast& E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch ( ... ) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    }
      break;
    case ASTTypeMPInteger: {
      try {
        const ASTExpressionNode* EN =
          std::any_cast<const ASTExpressionNode*>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTMPIntegerNode* MPI =
          dynamic_cast<const ASTMPIntegerNode*>(EN);
        assert(MPI && "Failed to dynamic_cast to a MPIntegerNode!");

        const ASTIdentifierNode* ID = MPI->GetIdentifier()->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTMPIntegerNode!");

        ASTAngleNode* AN = ASTAngleNodeMap::Instance().Find(ID->GetName(),
                                                            ID->GetBits());
        if (!AN) {
          std::stringstream IAN;
          IAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode* AId =
            ASTBuilder::Instance().FindOrCreateASTIdentifierNode(IAN.str(),
                                                                 MPI->GetBits(),
                                                                 ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, MPI, ATy,
                                                         MPI->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(IAN.str());
          AId->SetPolymorphicName(IAN.str());
          AN->Mangle();
          AN->MangleLiteral();
          ToDefcalParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
        }

        Params.Append(AN);
      } catch (const std::bad_any_cast& E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch ( ... ) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    }
      break;
    case ASTTypeMPDecimal: {
      try {
        const ASTExpressionNode* EN =
          std::any_cast<const ASTExpressionNode*>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTMPDecimalNode* MPD =
          dynamic_cast<const ASTMPDecimalNode*>(EN);
        assert(MPD && "Failed to dynamic_cast to a MPDecimalNode!");

        const ASTIdentifierNode* ID = MPD->GetIdentifier()->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTMPDecimalNode!");

        ASTAngleNode* AN = ASTAngleNodeMap::Instance().Find(ID->GetName(),
                                                            ID->GetBits());
        if (!AN) {
          std::stringstream IAN;
          IAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode* AId =
            ASTBuilder::Instance().CreateASTIdentifierNode(IAN.str(),
                                                           MPD->GetBits(),
                                                           ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, MPD, ATy,
                                                         MPD->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(IAN.str());
          AId->SetPolymorphicName(IAN.str());
          AN->Mangle();
          AN->MangleLiteral();
          ToDefcalParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
        }

        Params.Append(AN);
      } catch (const std::bad_any_cast& E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch ( ... ) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    }
      break;
    case ASTTypeFunctionCallArgument: {
      try {
        const ASTExpressionNode* EN =
          std::any_cast<const ASTExpressionNode*>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTFunctionCallArgumentNode* FCA =
          dynamic_cast<const ASTFunctionCallArgumentNode*>(EN);
        assert(FCA && "Failed to dynamic_cast to an ASTFunctionCallArgumentNode!");

        const ASTResultNode* RN = FCA->GetResult();
        assert(RN && "Could not obtain a valid function call ASTResultNode!");

        unsigned RBits = RN->GetResultBits() < ASTAngleNode::AngleBits ?
                             ASTAngleNode::AngleBits : RN->GetResultBits();

        std::stringstream IAN;
        IAN << ASTDemangler::TypeName(RN->GetResultType()) << C;
        ASTIdentifierNode* AId =
          ASTBuilder::Instance().CreateASTIdentifierNode(IAN.str(), RBits,
                                                         ASTTypeAngle);
        assert(AId && "Could not create an Angle ASTIdentifierNode!");

        ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
        ASTAngleNode* AN =
          ASTBuilder::Instance().CreateASTAngleNodeFromExpression(AId, FCA, ATy,
                                                                  ASTAngleNode::AngleBits);
        assert(AN && "Could not create an ASTAngleNode!");

        AN->SetGateParamName(IAN.str());
        AId->SetGateParamName(IAN.str());
        AN->Mangle();
        AN->MangleLiteral();
        ToDefcalParamSymbolTable(AId, AId->GetSymbolTableEntry());
        ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                       AId->GetSymbolType());
        ASTSymbolTable::Instance().EraseLocalAngle(AId);
        Params.Append(AN);
      } catch (const std::bad_any_cast& E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch ( ... ) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    }
      break;
    default:
      break;
    } // switch

    ++C;
  } // for

  assert(C == Params.Size() && "Inconsistent numer of Params!");
}

void ASTDefcalNode::CloneQubitsFrom(const ASTAnyTypeList& QL) {
  if (QL.Empty())
    return;

  unsigned C = 0U;
  ASTType Ty;

  for (ASTAnyTypeList::const_iterator I = QL.begin(); I != QL.end(); ++I) {
    Ty = (*I).second;
    switch (Ty) {
    case ASTTypeExpression: {
      const ASTExpression* EN = nullptr;

      try {
        EN = std::any_cast<const ASTExpression*>((*I).first);
        (void) EN; // Quiet.
      } catch (const std::bad_any_cast& E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch ( ... ) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    }
      break;
    case ASTTypeIdentifier: {
      try {
        ASTIdentifierNode* QId = std::any_cast<ASTIdentifierNode*>((*I).first);
        assert(QId && "Invalid ASTIdentifierNode!");

        ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(QId);
        if (!STE) {
          std::stringstream M;
          M << "Identifier " << QId->GetName() << " does not reference "
            << "a known Symbol.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(QId), M.str(),
                                                          DiagLevel::Error);
        }

        if (STE && STE->GetValueType() != ASTTypeQubit &&
                   STE->GetValueType() != ASTTypeQubitContainer) {
          std::stringstream M;
          M << "Identifier " << QId->GetName() << " is not a Qubit "
            << "or a Qubit register bank.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(QId), M.str(),
                                                          DiagLevel::Error);
        } else if (STE && (STE->GetValueType() == ASTTypeQubit ||
                           STE->GetValueType() == ASTTypeQubitContainer) &&
                   QId->GetName()[0] == '$') {
          QId->SetGateLocal(false);
          QId->SetGlobalScope();
          STE->SetGlobalScope();
          if (STE->GetValueType() == ASTTypeQubit &&
              QId->GetBits() == static_cast<unsigned>(~0x0))
            QId->SetBits(1);

          QIL.Append(QId);

          if (STE->GetValueType() == ASTTypeQubitContainer) {
            ASTQubitContainerNode* QCN =
              STE->GetValue()->GetValue<ASTQubitContainerNode*>();
            assert(QCN && "Could not obtain a valid ASTQubitContainerNode!");
            QCN->Mangle();
          } else {
            ASTQubitNode* QN = STE->GetValue()->GetValue<ASTQubitNode*>();
            assert(QN && "Could not obtain a valid ASTQubitNode!");
            QN->Mangle();
          }
        }
      } catch (const std::bad_any_cast& E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch ( ... ) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    }
      break;
    case ASTTypeIdentifierRef: {
      try {
        ASTIdentifierRefNode* IdR =
          std::any_cast<ASTIdentifierRefNode*>((*I).first);
        assert(IdR && "Invalid ASTIdentifierNodeRef!");

        ASTIdentifierNode* QId =
          const_cast<ASTIdentifierNode*>(IdR->GetIdentifier());
        assert(QId && "Invalid ASTIdentifierNode!");

        ASTSymbolTableEntry* STE =
          ASTSymbolTable::Instance().Lookup(QId->GetName());

        if (STE && STE->GetValueType() != ASTTypeQubit &&
                   STE->GetValueType() != ASTTypeQubitContainer) {
          std::stringstream M;
          M << "Identifier " << QId->GetName() << " is not a Qubit "
            << "or a Qubit register bank.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(QId), M.str(),
                                                          DiagLevel::Error);
        } else if (STE && (STE->GetValueType() == ASTTypeQubit ||
                           STE->GetValueType() == ASTTypeQubitContainer) &&
                   QId->GetName()[0] == '$') {
          QId->SetGateLocal(false);
          QId->SetGlobalScope();
          STE->SetGlobalScope();
          if (STE->GetValueType() == ASTTypeQubit &&
              QId->GetBits() == static_cast<unsigned>(~0x0))
            QId->SetBits(1);

          QIL.Append(QId);

          if (STE->GetValueType() == ASTTypeQubitContainer) {
            ASTQubitContainerNode* QCN =
              STE->GetValue()->GetValue<ASTQubitContainerNode*>();
            assert(QCN && "Could not obtain a valid ASTQubitContainerNode!");
            QCN->Mangle();
          } else {
            ASTQubitNode* QN = STE->GetValue()->GetValue<ASTQubitNode*>();
            assert(QN && "Could not obtain a valid ASTQubitNode!");
            QN->Mangle();
          }
        }
      } catch (const std::bad_any_cast& E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch ( ... ) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    }
      break;
    default: {
      std::stringstream M;
      M << "Identifier of Type " << PrintTypeEnum(Ty)
        << " cannot be used as a Qubit or Qubit register bank argument.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                     DiagLevel::Error);
    }
      break;
    }

    ++C;
  }

  assert(C == QIL.Size() && "Inconsistent numer of Qubits!");
}

void ASTDefcalNode::ResolveParams() {
  for (ASTExpressionNodeList::iterator I = Params.begin();
       I != Params.end(); ++I) {
    switch ((*I)->GetASTType()) {
    case ASTTypeAngle:
    case ASTTypeInt:
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeMPInteger:
    case ASTTypeMPDecimal:
      if (!IsCall) {
        if (DSTM.insert(std::make_pair(
                        (*I)->GetIdentifier()->GetName(),
                        (*I)->GetIdentifier()->GetSymbolTableEntry())).second) {
          if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(
                                                (*I)->GetIdentifier()->GetName()))
              ASTSymbolTable::Instance().EraseLocalSymbol((*I)->GetIdentifier(),
                                               (*I)->GetIdentifier()->GetBits(),
                                               (*I)->GetIdentifier()->GetSymbolType());
          if ((*I)->GetIdentifier()->GetSymbolType() == ASTTypeAngle) {
            std::stringstream S;
            for (unsigned J = 0; J < 3U; ++J) {
              S.str("");
              S.clear();
              S << (*I)->GetIdentifier()->GetName() << '[' << J << ']';
              const ASTSymbolTableEntry* ASTE =
                ASTSymbolTable::Instance().FindLocal(S.str());
              if (ASTE) {
                const ASTIdentifierNode* AId = ASTE->GetIdentifier();
                if (DSTM.insert(std::make_pair(AId->GetName(), ASTE)).second) {
                  ASTSymbolTable::Instance().EraseLocalSymbol(AId,
                                               AId->GetBits(),
                                               AId->GetSymbolType());
                  if (ASTTypeSystemBuilder::Instance().IsImplicitAngle(AId->GetName())) {
                    ASTSymbolTable::Instance().Erase(AId, AId->GetBits(),
                                                     AId->GetSymbolType());
                  }
                }
              }
            }
          }
        }
      }
      break;
    case ASTTypeIdentifier: {
      if (!IsCall) {
        if (!ASTUtils::Instance().IsAngleType((*I)->GetIdentifier()->GetSymbolType())) {
          const ASTIdentifierNode* XId = (*I)->GetIdentifier();
          assert(XId && "Could not obtain a valid ASTIdentifierNode!");

          const ASTSymbolTableEntry* XSTE =
            ASTSymbolTable::Instance().Lookup(XId->GetName(), XId->GetBits(),
                                              XId->GetSymbolType());
          assert(XSTE && "ASTIdentifierNode without a SymbolTable Entry!");

          ASTAngleNode* XAN = CreateAngleConversion(XId, XSTE);
          if (!XAN) {
            std::stringstream M;
            M << "Could not create an implicit ASTAngleNode conversion "
              << "from " << PrintTypeEnum(XId->GetSymbolType()) << '.';
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(XId), M.str(),
                                                           DiagLevel::Error);
          }

          XId = XAN->GetIdentifier();
          assert(XId && "Invalid ASTIdentifierNode for the implicit ASTAngleNode!");

          XSTE = XId->GetSymbolTableEntry();
          assert(XSTE && "Implicit ASTAngleNode without a SymbolTable Entry!");
          assert(XSTE->HasValue() &&
                 "Implicit ASTAngleNode SymbolTable Entry has no Value!");

          ASTSymbolTable::Instance().TransferAngleToLSTM(XId, XId->GetBits(),
                                                         XId->GetSymbolType());
          ToDefcalParamSymbolTable(XId, XSTE);
          ASTSymbolTable::Instance().EraseLocalSymbol(XId, XId->GetBits(),
                                                      XId->GetSymbolType());
        } else {
          ASTSymbolTableEntry* ASTE = const_cast<ASTSymbolTableEntry*>(
            ASTSymbolTable::Instance().FindAngle((*I)->GetIdentifier()));

          if (ASTE && ASTE->HasValue()) {
            ASTAngleNode* AN = ASTE->GetValue()->GetValue<ASTAngleNode*>();
            AN->Mangle();
            ToDefcalParamSymbolTable(AN->GetIdentifier(), ASTE);
          } else {
            ASTIdentifierNode* AId = (*I)->GetIdentifier();
            AId->SetSymbolType(ASTTypeAngle);
            AId->SetBits(ASTAngleNode::AngleBits);
            ASTE = AId->GetSymbolTableEntry();

            if (!ASTE) {
              ASTE = new ASTSymbolTableEntry(AId, ASTTypeAngle);
              assert(ASTE && "Could not create a valid ASTSymbolTableEntry!");
              AId->SetSymbolTableEntry(ASTE);
            }

            ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
            ASTAngleNode* AN =
              ASTBuilder::Instance().CreateASTAngleNode(AId, ATy,
                                                        ASTAngleNode::AngleBits);
            assert(AN && "Could not create a valid ASTAngleNode!");

            ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                           AId->GetSymbolType());
            ToDefcalParamSymbolTable(AId, ASTE);
            if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(AId->GetName())) {
              ASTSymbolTable::Instance().EraseLocalSymbol(AId, AId->GetBits(),
                                                          AId->GetSymbolType());
            }
          }
        }
      }
    }
      break;
    case ASTTypeBinaryOp:
      if (!IsCall) {
        ASTBinaryOpNode* BOP = dynamic_cast<ASTBinaryOpNode*>(*I);
        assert(BOP && "Could not dynamic_cast to an ASTBinaryOpNode!");

        const ASTIdentifierNode* Id = BOP->GetLeft()->GetIdentifier();
        if (DSTM.insert(std::make_pair(Id->GetName(),
                                       Id->GetSymbolTableEntry())).second) {
          if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
            ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(),
                                                        Id->GetSymbolType());
          }
        }

        Id = BOP->GetRight()->GetIdentifier();
        if (DSTM.insert(std::make_pair(Id->GetName(),
                                       Id->GetSymbolTableEntry())).second) {
          if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
            ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(),
                                                        Id->GetSymbolType());
          }
        }

        Id = BOP->GetIdentifier();
        if (DSTM.insert(std::make_pair(Id->GetName(),
                                       Id->GetSymbolTableEntry())).second) {
          if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
            ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(),
                                                        Id->GetSymbolType());
          }
        }
      }
      break;
    case ASTTypeUnaryOp:
      if (!IsCall) {
        ASTUnaryOpNode* UOP = dynamic_cast<ASTUnaryOpNode*>(*I);
        assert(UOP && "Could not dynamimc_cast to an ASTUnaryOpNode!");

        const ASTIdentifierNode* Id = UOP->GetExpression()->GetIdentifier();
        if (DSTM.insert(std::make_pair(Id->GetName(),
                                       Id->GetSymbolTableEntry())).second) {
          if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
            ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(),
                                                        Id->GetSymbolType());
          }
        }

        Id = UOP->GetIdentifier();
        if (DSTM.insert(std::make_pair(Id->GetName(),
                                       Id->GetSymbolTableEntry())).second) {
          if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
            ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(),
                                                        Id->GetSymbolType());
          }
        }
      }
      break;
    default:
      break;
    }
  }
}

void ASTDefcalNode::MangleParams() {
  if (!Params.Empty()) {
    for (ASTExpressionNodeList::iterator I = Params.begin();
         I != Params.end(); ++I) {
      (*I)->Mangle();
    }
  }
}

void ASTDefcalNode::ToDefcalParamSymbolTable(const ASTIdentifierNode* Id,
                                             const ASTSymbolTableEntry* STE) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  std::map<std::string, const ASTSymbolTableEntry*>::iterator I =
    DSTM.find(Id->GetName());
  if (I != DSTM.end())
    return;

  if (!DSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
    std::stringstream M;
    M << "Insertion failure to the local symbol table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
  }

  if (Id->GetSymbolType() == ASTTypeAngle) {
    ASTSymbolTable::Instance().EraseLocalSymbol(Id->GetName(),
                                                Id->GetBits(),
                                                Id->GetSymbolType());
  } else {
    if (Id->GetSymbolType() == ASTTypeGateQubitParam)
      ASTSymbolTable::Instance().EraseGateQubitParam(Id->GetName(),
                                                     Id->GetBits(),
                                                     Id->GetSymbolType());
  }

  if (Id->GetSymbolType() == ASTTypeAngle) {
    std::stringstream S;
    for (unsigned J = 0; J < 3U; ++J) {
      S.str("");
      S.clear();
      S << Id->GetName() << '[' << J << ']';

      const ASTSymbolTableEntry* ASTE =
        ASTSymbolTable::Instance().FindLocal(S.str());
      if (ASTE) {
        if (!DSTM.insert(std::make_pair(S.str(), ASTE)).second) {
          std::stringstream M;
          M << "Insertion failure to the local symbol table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
        }

        const ASTIdentifierNode* AId = ASTE->GetIdentifier();
        ASTSymbolTable::Instance().EraseLocalSymbol(S.str(),
                                                    AId->GetBits(),
                                                    AId->GetSymbolType());
      }
    }

    ASTSymbolTable::Instance().EraseLocalSymbol(Id->GetName(), Id->GetBits(),
                                                Id->GetSymbolType());
  } else if (Id->GetSymbolType() == ASTTypeQubitContainer) {
    std::stringstream S;
    const ASTSymbolTableEntry* QSTE = nullptr;

    for (unsigned J = 0; J < Id->GetBits(); ++J) {
      S.str("");
      S.clear();
      S << Id->GetName() << ':' << J;

      QSTE = ASTSymbolTable::Instance().FindQubit(S.str());
      if (QSTE) {
        if (!DSTM.insert(std::make_pair(S.str(), QSTE)).second) {
          std::stringstream M;
          M << "Insertion failure to the local symbol table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
        }
      } else {
        std::stringstream M;
        M << "Invalid ASTQubitContainerNode without a SymbolTable Entry.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
      }

      S.str("");
      S.clear();
      S << Id->GetName() << '[' << J << ']';

      QSTE = ASTSymbolTable::Instance().FindQubit(S.str());
      if (QSTE) {
        if (!DSTM.insert(std::make_pair(S.str(), QSTE)).second) {
          std::stringstream M;
          M << "Insertion failure to the local symbol table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
        }
      } else {
        std::stringstream M;
        M << "Invalid ASTQubitContainerNode without a SymbolTable Entry.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
      }
    }
  } else if (Id->GetSymbolType() == ASTTypeQubit) {
    if (DSTM.find(Id->GetName()) == DSTM.end()) {
      const ASTSymbolTableEntry* QSTE = ASTSymbolTable::Instance().FindQubit(Id);
      if (!QSTE) {
        std::stringstream M;
        M << "Invalid ASTQubitContainerNode without a SymbolTable Entry.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      }

      if (!DSTM.insert(std::make_pair(Id->GetName(), QSTE)).second) {
        std::stringstream M;
        M << "Insertion failure to the local symbol table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      }
    }
  }
}

void
ASTDefcalNode::ToDefcalParamSymbolTable(const std::string& Id,
                                        const ASTSymbolTableEntry* STE) {
  assert(!Id.empty() && "Invalid ASTIdentifierNode argument!");

  std::map<std::string, const ASTSymbolTableEntry*>::iterator I =
    DSTM.find(Id);
  if (I != DSTM.end())
    return;

  const ASTIdentifierNode* IId = STE->GetIdentifier();
  assert(IId && "Invalid ASTIdentifierNode for the SymbolTable Entry!");

  if (!DSTM.insert(std::make_pair(Id, STE)).second) {
    std::stringstream M;
    M << "Insertion failure to the local symbol table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
  }

  if (STE->GetValueType() == ASTTypeAngle) {
    std::stringstream S;
    for (unsigned J = 0; J < 3U; ++J) {
      S.str("");
      S.clear();
      S << IId->GetName() << '[' << J << ']';

      const ASTSymbolTableEntry* ASTE =
        ASTSymbolTable::Instance().FindLocal(S.str());
      if (ASTE) {
        if (!DSTM.insert(std::make_pair(S.str(), ASTE)).second) {
          std::stringstream M;
          M << "Insertion failure to the local symbol table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(IId), M.str(), DiagLevel::ICE);
        }

        const ASTIdentifierNode* AId = ASTE->GetIdentifier();
        ASTSymbolTable::Instance().EraseLocalSymbol(S.str(),
                                                    AId->GetBits(),
                                                    AId->GetSymbolType());
      }
    }

    ASTSymbolTable::Instance().EraseLocalSymbol(IId->GetName(), IId->GetBits(),
                                                IId->GetSymbolType());
  } else if (STE->GetValueType() == ASTTypeQubitContainer) {
    std::stringstream S;
    const ASTSymbolTableEntry* QSTE = nullptr;

    for (unsigned J = 0; J < IId->GetBits(); ++J) {
      S.str("");
      S.clear();
      S << IId->GetName() << ':' << J;

      QSTE = ASTSymbolTable::Instance().FindQubit(S.str());
      if (QSTE) {
        if (!DSTM.insert(std::make_pair(S.str(), QSTE)).second) {
          std::stringstream M;
          M << "Insertion failure to the local symbol table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(IId), M.str(),
                                                          DiagLevel::ICE);
        }
      } else {
        std::stringstream M;
        M << "Invalid ASTQubitContainerNode without a SymbolTable Entry.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(IId), M.str(),
                                                        DiagLevel::ICE);
      }

      S.str("");
      S.clear();
      S << IId->GetName() << '[' << J << ']';

      QSTE = ASTSymbolTable::Instance().FindQubit(S.str());
      if (QSTE) {
        if (!DSTM.insert(std::make_pair(S.str(), QSTE)).second) {
          std::stringstream M;
          M << "Insertion failure to the local symbol table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(IId), M.str(),
                                                          DiagLevel::ICE);
        }
      } else {
        std::stringstream M;
        M << "Invalid ASTQubitContainerNode without a SymbolTable Entry.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(IId), M.str(),
                                                        DiagLevel::ICE);
      }
    }
  } else if (STE->GetValueType() == ASTTypeQubit) {
    if (DSTM.find(Id) == DSTM.end()) {
      const ASTSymbolTableEntry* QSTE = ASTSymbolTable::Instance().FindQubit(Id);
      if (!QSTE) {
        std::stringstream M;
        M << "Invalid ASTQubitContainerNode without a SymbolTable Entry.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(IId), M.str(), DiagLevel::ICE);
      }

      if (!DSTM.insert(std::make_pair(Id, QSTE)).second) {
        std::stringstream M;
        M << "Insertion failure to the local symbol table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(IId), M.str(), DiagLevel::ICE);
      }
    }
  }
}

void ASTDefcalNode::TransferLocalSymbolTable() {
  std::map<std::string, ASTSymbolTableEntry*>& LSTM =
    ASTSymbolTable::Instance().GetLSTM();

  if (!LSTM.empty()) {
    std::vector<std::string> SV;

    for (std::map<std::string, ASTSymbolTableEntry*>::iterator I = LSTM.begin();
         I != LSTM.end(); ++I) {
      if (!DSTM.insert(std::make_pair((*I).first, (*I).second)).second) {
        std::stringstream M;
        M << "Defcal symbol table insertion failure for local symbol "
          << (*I).first.c_str() << '.';
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }

      SV.push_back((*I).first);
    }

    for (std::vector<std::string>::const_iterator I = SV.begin();
         I != SV.end(); ++I)
      LSTM.erase(*I);
  }
}

void ASTDefcalNode::print_qubits() const {
  if (!QIL.Empty()) {
    std::cout << "<DefcalQubits>" << std::endl;

    for (ASTIdentifierList::const_iterator I = QIL.begin();
         I != QIL.end(); ++I) {
      std::cout << "<Qubit>" << std::endl;
      (*I)->print();
      std::cout << "</Qubit>" << std::endl;
    }

    std::cout << "</DefcalQubits>" << std::endl;
  }
}

void ASTDefcalNode::print() const {
  std::cout << "<Defcal>" << std::endl;
  std::cout << "<Identifier>" << GetName() << "</Identifier>" << std::endl;
  std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
  std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
    << std::endl;
  std::cout << "<IsDefcalCall>" << std::boolalpha << IsCall
    << "</IsDefcalCall>" << std::endl;
  if (!Params.Empty()) {
    std::cout << "<DefcalParameters>" << std::endl;
    Params.print();
    std::cout << "</DefcalParameters>" << std::endl;
  }

  if (!Statements.Empty()) {
    std::cout << "<DefcalStatements>" << std::endl;
    Statements.print();
    std::cout << "</DefcalStatements>" << std::endl;
  }

  print_qubits();

  switch (OTy) {
  case ASTTypeMeasure:
    if (Measure)
      Measure->print();
    break;
  case ASTTypeReset:
    if (Reset)
      Reset->print();
    break;
  case ASTTypeDelay:
    if (Delay)
      Delay->print();
    break;
  case ASTTypeDurationOf:
    if (Duration)
      Duration->print();
    break;
  default:
    break;
  }

  std::cout << "<Grammar>" << Grammar << "</Grammar>" << std::endl;
  std::cout << "</Defcal>" << std::endl;
}

void ASTDefcalNode::Mangle() {
  ASTMangler M;
  M.Start();

  if (IsCall) {
    if (IsMeasure()) {
      M.TypeIdentifier(ASTTypeDefcalMeasureCall,
                       GetIdentifier()->GetGateParamName());
    } else if (IsReset()) {
      M.TypeIdentifier(ASTTypeDefcalResetCall,
                       GetIdentifier()->GetGateParamName());
    } else {
      M.TypeIdentifier(ASTTypeDefcalCall, GetName());
    }
  } else {
    if (IsMeasure()) {
      const_cast<ASTMeasureNode*>(Measure)->Mangle();
      M.TypeIdentifier(ASTTypeDefcalMeasure,
                       GetIdentifier()->GetGateParamName());
    } else if (IsReset()) {
      const_cast<ASTResetNode*>(Reset)->Mangle();
      M.TypeIdentifier(ASTTypeDefcalReset,
                       GetIdentifier()->GetGateParamName());
    } else if (IsDelay()) {
      const_cast<ASTDelayNode*>(Delay)->Mangle();
      M.TypeIdentifier(ASTTypeDefcalDelay,
                       GetIdentifier()->GetGateParamName());
    } else {
      M.TypeIdentifier(GetASTType(), GetName());
    }
  }

  if (!Grammar.empty())
    M.TypeIdentifier(ASTTypeDefcalGrammar, Grammar);

  if (OTy == ASTTypeMeasure) {
    assert(Measure && "Invalid ASTMeasureNode for measure defcal!");

    const_cast<ASTMeasureNode*>(Measure)->Mangle();
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(Measure->GetMangledName()));

    if (Measure->HasResult()) {
      const ASTCBitNode* CBN = Measure->GetResult();
      assert(CBN && "Invalid ASTCBitNode obtained from ASTMeasureNode!");
      M.FuncReturn(CBN->GetASTType(), CBN->GetBits());
    } else if (Measure->HasAngleResult()) {
      const ASTAngleNode* AN = Measure->GetAngleResult();
      assert(AN && "Invalid ASTAngleNode obtained from ASTMeasureNode!");
      M.FuncReturn(AN->GetASTType(), AN->GetBits());
    } else if (Measure->HasComplexResult()) {
      const ASTMPComplexNode* MPC = Measure->GetComplexResult();
      assert(MPC && "Invalid ASTMPComplexNode obtained from ASTMeasureNode!");
      M.FuncReturn(MPC->GetASTType(), MPC->GetBits());
    }
  } else if (OTy == ASTTypeReset) {
    assert(Reset && "Invalid ASTResetNode for measure reset!");
    const_cast<ASTResetNode*>(Reset)->Mangle();
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(Reset->GetMangledName()));
  } else if (OTy == ASTTypeDelay) {
    assert(Delay && "Invalid ASTDelayNode for delay defcal!");
    const_cast<ASTDelayNode*>(Delay)->Mangle();
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(Delay->GetMangledName()));
  }

  unsigned IX = 0U;

  if (IsCall) {
    for (ASTExpressionNodeList::iterator EI = Params.begin();
         EI != Params.end(); ++EI) {
      if (const ASTAngleNode* AN = dynamic_cast<const ASTAngleNode*>(*EI)) {
        if (AN->IsExpression()) {
          switch (AN->GetExprType()) {
          case ASTTypeUnaryOp: {
            if (const ASTUnaryOpNode* UOP =
                dynamic_cast<const ASTUnaryOpNode*>(AN->GetExpression())) {
              M.DefcalArg(IX++, ASTStringUtils::Instance().SanitizeMangled(
                                                UOP->GetMangledName()));
            }
          }
            break;
          case ASTTypeBinaryOp: {
            if (const ASTBinaryOpNode* BOP =
                dynamic_cast<const ASTBinaryOpNode*>(AN->GetExpression())) {
              M.DefcalArg(IX++, ASTStringUtils::Instance().SanitizeMangled(
                                                BOP->GetMangledName()));
            }
          }
            break;
          default: {
            const_cast<ASTAngleNode*>(AN)->MangleLiteral();
            M.DefcalArg(IX++, ASTStringUtils::Instance().SanitizeMangled(
                                              AN->GetMangledLiteralName()));
          }
            break;
          }
        } else {
            M.DefcalArg(IX++, ASTStringUtils::Instance().SanitizeMangled(
                                              (*EI)->GetMangledLiteralName()));
        }

        continue;
      }

      if ((*EI)->IsIdentifier()) {
        if (const ASTIdentifierNode* Id =
            dynamic_cast<const ASTIdentifierNode*>((*EI)->GetExpression())) {
          if (Id->IsReference()) {
            if (const ASTIdentifierRefNode* IdR =
                dynamic_cast<const ASTIdentifierRefNode*>(Id)) {
              M.DefcalArg(IX++, IdR->GetSymbolType(), IdR->GetBits(), IdR->GetName());
            }
          } else {
            if (!Id->GetMangledLiteralName().empty())
              M.DefcalArg(IX++, ASTStringUtils::Instance().SanitizeMangled(
                                                Id->GetMangledLiteralName()));
            else
              M.DefcalArg(IX++, Id->GetSymbolType(), Id->GetBits(), Id->GetName());
          }
        }
      } else {
        if ((*EI)->GetASTType() == ASTTypeBinaryOp ||
            (*EI)->GetASTType() == ASTTypeUnaryOp) {
          M.DefcalArg(IX++, ASTStringUtils::Instance().SanitizeMangled(
                                            (*EI)->GetMangledLiteralName()));
        } else {
          if (const ASTIdentifierNode* Id = (*EI)->GetIdentifier()) {
            if (!Id->GetMangledLiteralName().empty())
              M.DefcalArg(IX++, ASTStringUtils::Instance().SanitizeMangled(
                                                Id->GetMangledLiteralName()));
            else if (!Id->GetPolymorphicName().empty())
              M.DefcalArg(IX++, Id->GetSymbolType(), Id->GetBits(),
                                                     Id->GetPolymorphicName());
            else
              M.DefcalArg(IX++, Id->GetSymbolType(), Id->GetBits(), Id->GetName());
          }
        }
      }
    }
  } else {
    for (ASTExpressionNodeList::iterator EI = Params.begin();
         EI != Params.end(); ++EI) {
      if ((*EI)->IsIdentifier()) {
        if (const ASTIdentifierNode* Id =
            dynamic_cast<const ASTIdentifierNode*>((*EI)->GetExpression())) {
          if (Id->IsReference()) {
            if (const ASTIdentifierRefNode* IdR =
                dynamic_cast<const ASTIdentifierRefNode*>(Id)) {
              M.DefcalParam(IX++, IdR->GetSymbolType(), IdR->GetBits(),
                                                        IdR->GetName());
            }
          } else {
            M.DefcalParam(IX++, Id->GetSymbolType(), Id->GetBits(), Id->GetName());
          }
        }
      } else {
        if ((*EI)->GetASTType() == ASTTypeBinaryOp ||
            (*EI)->GetASTType() == ASTTypeUnaryOp) {
          M.DefcalParam(IX++, ASTStringUtils::Instance().SanitizeMangled(
                                              (*EI)->GetMangledName()));
        } else {
          if (const ASTIdentifierNode* Id = (*EI)->GetIdentifier()) {
            if (!Id->GetMangledLiteralName().empty())
              M.DefcalParam(IX++, Id->GetSymbolType(), Id->GetBits(),
                            ASTStringUtils::Instance().SanitizeMangled(
                                                       Id->GetMangledLiteralName()));
            else if (!Id->GetPolymorphicName().empty())
              M.DefcalParam(IX++, Id->GetSymbolType(), Id->GetBits(),
                                                       Id->GetPolymorphicName());
            else
              M.DefcalParam(IX++, Id->GetSymbolType(), Id->GetBits(), Id->GetName());
          }
        }
      }
    }
  }

  if (!QIL.Empty()) {
    for (ASTIdentifierList::const_iterator QI = QIL.begin();
         QI != QIL.end(); ++QI) {
      const ASTIdentifierNode* QId = *QI;
      assert(QId && "Invalid defcal qubit ASTIdentifierNode!");

      if (IsCall) {
        if (QId->IsReference()) {
          if (const ASTIdentifierRefNode* QIdR =
              dynamic_cast<const ASTIdentifierRefNode*>(QId)) {
            M.DefcalArg(IX++, ASTStringUtils::Instance().SanitizeMangled(
                                              QIdR->GetMangledName()));
          }
        } else {
          if (ASTStringUtils::Instance().IsIndexedQubit(QId->GetName()))
            M.DefcalArg(IX++, ASTStringUtils::Instance().SanitizeMangled(
                                              QId->GetMangledName()));
          else
            M.DefcalArg(IX++, ASTStringUtils::Instance().SanitizeMangled(
                                              QId->GetMangledName()));
        }
      } else {
        if (QId->IsReference()) {
          if (const ASTIdentifierRefNode* QIdR =
              dynamic_cast<const ASTIdentifierRefNode*>(QId)) {
            if (ASTStringUtils::Instance().IsIndexedQubit(QIdR->GetName()))
              M.DefcalParam(IX++, QIdR->GetSymbolType(), QIdR->GetBits(),
                            QIdR->GetName());
            else
              M.DefcalParam(IX++, QIdR->GetSymbolType(), QIdR->GetBits(),
                            ASTStringUtils::Instance().BracketedQubit(QIdR->GetName()));
          }
        } else {
          if (ASTStringUtils::Instance().IsIndexedQubit(QId->GetName()))
            M.DefcalParam(IX++, QId->GetSymbolType(), QId->GetBits(),
                          ASTStringUtils::Instance().BracketedQubit(QId->GetName()));
          else
            M.DefcalParam(IX++, QId->GetSymbolType(), QId->GetBits(),
                          QId->GetName());
        }
      }
    }
  }

  if (IsCall)
    M.CallEnd();

  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace QASM

