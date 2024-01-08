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

#include <qasm/AST/ASTAngleNodeBuilder.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTFunctionCallArgument.h>
#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTGPhase.h>
#include <qasm/AST/ASTGateContextBuilder.h>
#include <qasm/AST/ASTGateNodeBuilder.h>
#include <qasm/AST/ASTGateOpList.h>
#include <qasm/AST/ASTGateQubitTracker.h>
#include <qasm/AST/ASTGates.h>
#include <qasm/AST/ASTIdentifierBuilder.h>
#include <qasm/AST/ASTImplicitConversionExpr.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTUtils.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <set>
#include <sstream>
#include <string>

namespace QASM {

ASTGateQOpList ASTGateQOpList::EmptyDefault;

ASTGateContextBuilder ASTGateContextBuilder::GCB;
bool ASTGateContextBuilder::GCS = false;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

void ASTGateNode::ToGateParamSymbolTable(const ASTIdentifierNode *Id,
                                         const ASTSymbolTableEntry *STE) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  std::map<std::string, const ASTSymbolTableEntry *>::iterator I =
      GSTM.find(Id->GetName());
  if (I != GSTM.end())
    return;

  const ASTDeclarationContext *DCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(DCX && "Could not obtain a valid ASTDeclarationContext!");

  if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
    const_cast<ASTIdentifierNode *>(Id)->SetDeclarationContext(DCX);
    const_cast<ASTSymbolTableEntry *>(STE)->SetContext(DCX);
    const_cast<ASTIdentifierNode *>(Id)->SetLocalScope();
    const_cast<ASTSymbolTableEntry *>(STE)->SetLocalScope();
  }

  if (!GSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
    std::stringstream M;
    M << "Insertion failure to the local symbol table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
  }

  if (Id->GetSymbolType() == ASTTypeAngle) {
    std::stringstream S;
    for (unsigned J = 0; J < 3U; ++J) {
      S.str("");
      S.clear();
      S << Id->GetName() << '[' << J << ']';

      const ASTSymbolTableEntry *ASTE =
          ASTSymbolTable::Instance().FindLocal(S.str());
      if (!ASTE)
        ASTE = ASTSymbolTable::Instance().FindAngle(S.str());

      if (ASTE) {
        if (!GSTM.insert(std::make_pair(S.str(), ASTE)).second) {
          std::stringstream M;
          M << "Insertion failure to the local symbol table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
              DiagLevel::ICE);
        }

        const ASTIdentifierNode *AId = ASTE->GetIdentifier();
        assert(AId && "Invalid ASTIdentifierNode for ASTSymbolTableEntry!");

        if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
          const_cast<ASTIdentifierNode *>(AId)->SetDeclarationContext(DCX);
          const_cast<ASTSymbolTableEntry *>(ASTE)->SetContext(DCX);
          const_cast<ASTIdentifierNode *>(AId)->SetLocalScope();
          const_cast<ASTSymbolTableEntry *>(ASTE)->SetLocalScope();
        }

        ASTSymbolTable::Instance().EraseLocalSymbol(S.str(), AId->GetBits(),
                                                    AId->GetSymbolType());
        ASTSymbolTable::Instance().EraseLocalAngle(S.str(), AId->GetBits(),
                                                   AId->GetSymbolType());
      }
    }

    ASTSymbolTable::Instance().EraseLocalSymbol(Id->GetName(), Id->GetBits(),
                                                Id->GetSymbolType());
  }
}

ASTSymbolTableEntry *
ASTGateNode::MangleGateQubitParam(ASTIdentifierNode *Id,
                                  ASTSymbolTableEntry *&STE, unsigned IX,
                                  unsigned Bits, unsigned QBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (!STE) {
    STE = new ASTSymbolTableEntry(Id, Id->GetSymbolType());
    assert(STE && "Could not create a valid ASTSymbolTableEntry!");
  }

  if (Bits == 0U && QBits != 0U) {
    std::stringstream M;
    M << "A Qubit Parameter cannot have a Qubit size.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }

  std::stringstream QNS;
  QNS << (Bits == 0U ? "qubitparam" : "qubitarg") << IX;
  ASTType QTy = Id->GetSymbolType();

  switch (QTy) {
  case ASTTypeQubit: {
    ASTQubitNode *QN = nullptr;
    if (!STE->HasValue()) {
      QN = new ASTQubitNode(Id, IX, QNS.str());
      assert(QN && "Could not create a valid ASTQubitNode!");
      STE->ResetValue();
      STE->SetValue(new ASTValue<>(QN, QTy), QTy);
      assert(STE->HasValue() && "Qubit SymbolTable Entry has no Value!");
    } else {
      QN = STE->GetValue()->GetValue<ASTQubitNode *>();
      assert(QN && "Could not create a valid ASTQubitNode!");
    }

    QN->Mangle();
  } break;
  case ASTTypeGateQubitParam: {
    ASTGateQubitParamNode *QPN = nullptr;
    if (!STE->HasValue()) {
      QPN = new ASTGateQubitParamNode(Id, IX, Bits, QNS.str());
      assert(QPN && "Could not create a valid ASTQubitParamNode!");
      STE->ResetValue();
      STE->SetValue(new ASTValue<>(QPN, QTy), QTy);
      assert(STE->HasValue() && "Qubit SymbolTable Entry has no Value!");
    } else {
      QPN = STE->GetValue()->GetValue<ASTGateQubitParamNode *>();
      if (!QPN) {
        std::map<std::string, const ASTSymbolTableEntry *>::iterator QI =
            GSTM.find(Id->GetName());
        if (QI == GSTM.end()) {
          QPN = new ASTGateQubitParamNode(Id, IX, Bits, QNS.str());
          assert(QPN && "Could not create a valid ASTQubitParamNode!");
          STE->ResetValue();
          STE->SetValue(new ASTValue<>(QPN, QTy), QTy);
          assert(STE->HasValue() && "Qubit SymbolTable Entry has no Value!");
        } else if ((*QI).second->GetValueType() == ASTTypeGateQubitParam) {
          QPN = (*QI).second->GetValue()->GetValue<ASTGateQubitParamNode *>();
          assert(QPN && "Could not create a valid ASTGateQubitParamNode!");
        } else {
          std::stringstream M;
          M << "Identifier '" << Id->GetName() << "' already exists "
            << "in the Gate's SymbolTable but with a different Type ("
            << PrintTypeEnum((*QI).second->GetValueType()) << " vs. "
            << PrintTypeEnum(ASTTypeGateQubitParam) << ").";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
              DiagLevel::ICE);
        }
      }
    }

    QPN->Mangle();
  } break;
  case ASTTypeQubitContainer: {
    ASTQubitContainerNode *QCN = nullptr;
    if (!STE->HasValue()) {
      QCN = new ASTQubitContainerNode(Id, Bits, QBits, QNS.str());
      assert(QCN && "Could not create a valid ASTQubitContainerNode!");
      STE->ResetValue();
      STE->SetValue(new ASTValue<>(QCN, QTy), QTy);
      assert(STE->HasValue() && "Qubit SymbolTable Entry has no Value!");
    } else {
      QCN = STE->GetValue()->GetValue<ASTQubitContainerNode *>();
      assert(QCN && "Could not obtain a valid ASTQubitContainerNode!");
    }

    QCN->Mangle();
  } break;
  case ASTTypeQubitContainerAlias: {
    if (!STE->HasValue()) {
      std::stringstream M;
      M << "Impossible initialization of gate qubit parameter from "
        << "type " << PrintTypeEnum(QTy) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    ASTQubitContainerAliasNode *QCAN =
        STE->GetValue()->GetValue<ASTQubitContainerAliasNode *>();
    assert(QCAN && "Could not obtain a valid ASTQubitContainerAliasNode!");
    QCAN->Mangle();
  } break;
  default: {
    std::stringstream M;
    M << "Impossible initialization of gate qubit parameter from "
      << "type " << PrintTypeEnum(QTy) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return nullptr;
  } break;
  }

  return STE;
}

ASTAngleNode *
ASTGateNode::CreateAngleConversion(const ASTSymbolTableEntry *XSTE) const {
  assert(XSTE && "Invalid ASTSymbolTableEntry argument!");

  ASTAngleNode *XAN = nullptr;
  ASTImplicitConversionNode *ICE = nullptr;
  const ASTLocation &LC = XSTE->GetIdentifier()->GetLocation();

  switch (XSTE->GetValueType()) {
  case ASTTypeFloat: {
    const ASTFloatNode *FN = XSTE->GetValue()->GetValue<ASTFloatNode *>();
    assert(FN && "Could not obtain a valid ASTFloatNode!");

    XAN =
        new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), FN,
                         ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(FN, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  } break;
  case ASTTypeDouble: {
    const ASTDoubleNode *DN = XSTE->GetValue()->GetValue<ASTDoubleNode *>();
    assert(DN && "Could not obtain a valid ASTDoubleNode!");

    XAN =
        new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), DN,
                         ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(DN, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  } break;
  case ASTTypeInt:
  case ASTTypeUInt: {
    const ASTIntNode *IN = XSTE->GetValue()->GetValue<ASTIntNode *>();
    assert(IN && "Could not obtain a valid ASTIntNode!");

    XAN =
        new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), IN,
                         ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(IN, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  } break;
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger: {
    const ASTMPIntegerNode *MPI =
        XSTE->GetValue()->GetValue<ASTMPIntegerNode *>();
    assert(MPI && "Could not obtain a valid ASTMPIntegerNode!");

    XAN =
        new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), MPI,
                         ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(MPI, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  } break;
  case ASTTypeMPDecimal: {
    const ASTMPDecimalNode *MPD =
        XSTE->GetValue()->GetValue<ASTMPDecimalNode *>();
    assert(MPD && "Could not obtain a valid ASTMPDecimalNode!");

    XAN =
        new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), MPD,
                         ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(MPD, ASTTypeAngle,
                                        XSTE->GetIdentifier()->GetBits());
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  } break;
  case ASTTypeBitset: {
    const ASTCBitNode *CBN = XSTE->GetValue()->GetValue<ASTCBitNode *>();
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

    for (unsigned I = 0; I < SZ; ++I)
      if ((*CBN)[I])
        D += static_cast<double>(M_PI / 2);

    XAN =
        new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), D,
                         ASTAngleTypeGeneric, XSTE->GetIdentifier()->GetBits());
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(CBN, ASTTypeAngle, SZ);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  } break;
  default: {
    std::stringstream M;
    M << "Impossible implicit conversion to Angle Type from "
      << PrintTypeEnum(XSTE->GetValueType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(XSTE->GetIdentifier()), M.str(),
        DiagLevel::Error);
    return nullptr;
  } break;
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

ASTAngleNode *
ASTGateNode::CreateAngleTemporary(const ASTSymbolTableEntry *XSTE) const {
  assert(XSTE && "Invalid ASTSymbolTableEntry argument!");

  ASTAngleNode *XAN =
      new ASTAngleNode(ASTIdentifierNode::Angle.Clone(), ASTAngleTypeGeneric,
                       XSTE->GetIdentifier()->GetBits());
  assert(XAN && "Could not create a valid ASTAngleNode!");

  ASTObjectTracker::Instance().Unregister(XAN);
  return XAN;
}

ASTAngleNode *
ASTGateNode::CreateAngleSymbolTableEntry(ASTSymbolTableEntry *XSTE) const {
  assert(XSTE && "Invalid ASTSymbolTableEntry argument!");

  ASTAngleNode *XAN =
      new ASTAngleNode(XSTE->GetIdentifier(), ASTAngleTypeGeneric,
                       XSTE->GetIdentifier()->GetBits());
  assert(XAN && "Could not create a valid ASTAngleNode!");

  XSTE->ResetValue();
  XSTE->SetValue(new ASTValue<>(XAN, ASTTypeAngle), ASTTypeAngle);
  assert(XSTE->HasValue() && "ASTSymbolTable Entry has no Value!");

  return XAN;
}

void ASTGateNode::ClearLocalGateSymbols() const {
  std::map<std::string, const ASTSymbolTableEntry *>::const_iterator MI;
  for (MI = GSTM.begin(); MI != GSTM.end(); ++MI) {
    if ((*MI).second) {
      if ((*MI).second->GetValueType() == ASTTypeGateQubitParam)
        ASTSymbolTable::Instance().EraseGateQubitParam(
            (*MI).second->GetIdentifier());

      ASTSymbolTable::Instance().EraseLocalSymbol(
          (*MI).second->GetIdentifier(),
          (*MI).second->GetIdentifier()->GetBits(),
          (*MI).second->GetValueType());
    } else {
      ASTSymbolTable::Instance().EraseGateQubitParam((*MI).first);
      ASTSymbolTable::Instance().EraseLocalSymbol((*MI).first);
    }
  }
}

void ASTGateNode::ClearGateQubits() const {
  for (std::vector<ASTQubitNode *>::const_iterator QI = Qubits.begin();
       QI != Qubits.end(); ++QI) {
    switch ((*QI)->GetASTType()) {
    case ASTTypeQubit:
    case ASTTypeQubitContainer:
    case ASTTypeQubitContainerAlias:
    case ASTTypeGateQubitParam:
      ASTSymbolTable::Instance().EraseGateLocalQubit((*QI)->GetName());
      break;
    default:
      break;
    }
  }
}

void ASTGateNode::MaterializeBuiltinUGate(const ASTIdentifierNode *GId,
                                          const ASTParameterList &PL,
                                          const ASTIdentifierList &IL) {
  assert(GId && "Invalid ASTIdentifierNode argument!");

  if (PL.Size() != 3) {
    std::stringstream M;
    M << "Invalid number of angle parameters for builtin UGate.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(GId), M.str(), DiagLevel::ICE);
  }

  if (IL.Size() != 1) {
    std::stringstream M;
    M << "Invalid number of qubit parameters for builtin UGate.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(GId), M.str(), DiagLevel::ICE);
  }

  const ASTDeclarationContext *DCX =
      ASTDeclarationContextTracker::Instance().GetGlobalContext();
  assert(DCX && "Could not obtain a valid ASTDeclarationContext!");

  for (ASTParameterList::const_iterator I = PL.begin(); I != PL.end(); ++I) {
    ASTParameter *AP = dynamic_cast<ASTParameter *>(*I);
    assert(AP && "Could not dynamic_cast to an ASTParameter!");

    ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().Lookup(
        AP->GetName(), ASTAngleNode::AngleBits, ASTTypeAngle);
    assert(STE && "UGate Angle Parameter has no SymbolTable Entry!");
    assert(STE->HasValue() &&
           "UGate Angle Parameter SymbolTable Entry has no Value!");

    ASTAngleNode *AN = STE->GetValue()->GetValue<ASTAngleNode *>();
    assert(AN && "UGate Angle Parameter is invalid!");

    ASTIdentifierNode *AId =
        const_cast<ASTIdentifierNode *>(AN->GetIdentifier());
    assert(AId && "UGate Angle Parameter has an invalid ASTIdentifierNode!");

    AId->SetDeclarationContext(DCX);
    AId->SetLocalScope();
    STE->SetContext(DCX);
    STE->SetLocalScope();
    AId->SetSymbolTableEntry(STE);

    ToGateParamSymbolTable(AId, STE);
    Params.push_back(AN);
    ASTSymbolTable::Instance().EraseLocalAngle(AId);
  }

  for (ASTIdentifierList::const_iterator I = IL.begin(); I != IL.end(); ++I) {
    ASTIdentifierNode *QId = const_cast<ASTIdentifierNode *>(*I);
    assert(QId && "Invalid Qubit ASTIdentifierNode!");

    ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().Lookup(
        QId->GetName(), QId->GetBits(), ASTTypeGateQubitParam);
    assert(STE && "UGate Qubit Parameter has no SymbolTable Entry!");

    QId->SetDeclarationContext(DCX);
    QId->SetLocalScope();
    STE->SetContext(DCX);
    STE->SetLocalScope();
    QId->SetSymbolTableEntry(STE);

    MaterializeGateQubitParam(QId);
    QCParams.push_back(STE);
    STE = MangleGateQubitParam(QId, STE, 0, QId->GetBits(), QId->GetBits());
    ToGateParamSymbolTable(QId, STE);
    ASTSymbolTable::Instance().EraseLocalQubitParam(QId);
  }
}

ASTGateNode::ASTGateNode(const ASTIdentifierNode *Id,
                         const ASTArgumentNodeList &AL,
                         const ASTAnyTypeList &QL, bool IsGateCall,
                         const ASTGateQOpList &OL)
    : ASTExpressionNode(Id, ASTTypeGate), Params(), Qubits(), QCParams(),
      OpList(OL), Ctrl(nullptr), GDId(IsGateCall ? nullptr : Id), GSTM(),
      ControlType(ASTTypeUndefined), Opaque(false), GateCall(IsGateCall) {
  unsigned C = 0;
  std::set<std::string> PNS;
  std::vector<const ASTIdentifierNode *> NQV;
  ASTType Ty = ASTTypeUndefined;

  for (ASTArgumentNodeList::const_iterator I = AL.begin(); I != AL.end(); ++I) {
    Ty = (*I)->GetValueType();
    ASTSymbolTableEntry *ASTE = nullptr;

    switch (Ty) {
    case ASTTypeExpression: {
      try {
        const ASTExpression *EN =
            std::any_cast<const ASTExpression *>((*I)->GetValue());
        (void)EN; // Quiet.
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeIdentifier: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTIdentifierNode *ID = EN->DynCast<const ASTIdentifierNode>();
        assert(ID && "Failed to dynamic_cast to an IdentifierNode!");

        const ASTSymbolTableEntry *XSTE =
            ASTSymbolTable::Instance().FindAngle(ID->GetName());
        if (!XSTE)
          XSTE = ASTSymbolTable::Instance().FindLocal(ID->GetName());
        if (!XSTE)
          XSTE = ASTSymbolTable::Instance().FindGlobal(ID->GetName());

        if (!XSTE) {
          std::stringstream M;
          M << "Non-existent angle " << ID->GetName() << " passed as "
            << "angle argument to Gate Call.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(ID), M.str(),
              DiagLevel::Error);
        }

        ASTAngleNode *XAN = nullptr;
        bool IMPC = false;

        if (XSTE->HasValue()) {
          if (ASTUtils::Instance().IsAngleType(XSTE->GetValueType()) ||
              ASTUtils::Instance().IsReservedAngleType(XSTE->GetValueType())) {
            XAN = XSTE->GetValue()->GetValue<ASTAngleNode *>();
          } else {
            XAN = CreateAngleConversion(XSTE);
            IMPC = true;
          }
        } else {
          XAN = CreateAngleTemporary(XSTE);
          IMPC = true;
        }

        assert(XAN && "Could not obtain a valid ASTAngleNode!");

        ASTAngleNode *AN = nullptr;
        ASTIdentifierNode *AId = nullptr;

        std::stringstream SAN;
        SAN << ASTDemangler::TypeName(ASTTypeAngle) << C;
        AId = ASTBuilder::Instance().CreateASTIdentifierNode(
            SAN.str(), ASTAngleNode::AngleBits, ASTTypeAngle);
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
        assert(ASTE &&
               "ASTAngleNode ASTIdentifierNode has no SymbolTable Entry!");

        AId->SetLocalScope();
        ASTE->SetLocalScope();
        AId->SetSymbolTableEntry(ASTE);
        AN->Mangle();
        AN->MangleLiteral();
        ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                       AId->GetSymbolType());
        ToGateParamSymbolTable(AId, ASTE);
        ASTSymbolTable::Instance().EraseLocalAngle(AId);
        ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                              AId->GetSymbolType());

        ASTAngleNodeBuilder::Instance().Insert(AN);
        ASTAngleNodeBuilder::Instance().Append(AN);

        Params.push_back(AN);

        if (XAN && IMPC)
          delete XAN;

        PNS.insert(AN->GetName());
        PNS.insert(ID->GetName());
        ID->SetNoQubit(true);
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeBinaryOp: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTBinaryOpNode *BOP = EN->DynCast<const ASTBinaryOpNode>();
        assert(BOP && "Failed to dynamic_cast to a BinaryOpNode!");

        const ASTIdentifierNode *BId = BOP->GetIdentifier();
        assert(BId && "Invalid ASTIdentifierNode for ASTBinaryOpNode!");

        ASTAngleNode *AN = ASTAngleNodeMap::Instance().Find(BId->GetName());

        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(BId->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), BId->GetBits(), ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, BOP, ATy,
                                                         AId->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(AId->GetName());
          AId->SetPolymorphicName(AId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          AId->GetSymbolTableEntry()->ResetValue();
          AId->GetSymbolTableEntry()->SetValue(new ASTValue<>(AN, ASTTypeAngle),
                                               ASTTypeAngle);
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        Params.push_back(AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeUnaryOp: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTUnaryOpNode *UOP = EN->DynCast<const ASTUnaryOpNode>();
        assert(UOP && "Failed to dynamic_cast to an UnaryOpNode!");

        const ASTIdentifierNode *UId = UOP->GetIdentifier();
        assert(UId && "Invalid ASTIdentifierNode for ASTUnaryOpNode!");

        ASTAngleNode *AN = ASTAngleNodeMap::Instance().Find(UId->GetName());

        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(UId->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), UId->GetBits(), ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, UOP, ATy,
                                                         AId->GetBits());
          assert(AN && "Could not create an ASTAngleNode!!");

          AN->SetGateParamName(UOP->GetName());
          AId->SetPolymorphicName(UId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        Params.push_back(AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeInt: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTIntNode *INT = dynamic_cast<const ASTIntNode *>(EN);
        assert(INT && "Failed to dynamic_cast to a IntNode!");

        const ASTIdentifierNode *ID = INT->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTIntNode!");

        if (INT->GetSignedValue() > static_cast<double>(M_PI * 2)) {
          std::stringstream M;
          M << "Angle value exceeds 2pi.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(ID), M.str(),
              DiagLevel::Warning);
        }

        ASTAngleNode *AN = ASTAngleNodeMap::Instance().Find(ID->GetName());
        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), ASTAngleNode::AngleBits, ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(
              AId, INT, ATy, ASTAngleNode::AngleBits);
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(AId->GetName());
          AId->SetPolymorphicName(AId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        Params.push_back(AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeFloat: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTFloatNode *FLT = dynamic_cast<const ASTFloatNode *>(EN);
        assert(FLT && "Failed to dynamic_cast to a FloatNode!");

        const ASTIdentifierNode *ID = FLT->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTFloatNode!");

        if (FLT->GetValue() > static_cast<float>(M_PI * 2)) {
          std::stringstream M;
          M << "Angle value exceeds 2pi.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(ID), M.str(),
              DiagLevel::Warning);
        }

        ASTAngleNode *AN = ASTAngleNodeMap::Instance().Find(ID->GetName());
        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), ASTAngleNode::AngleBits, ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, FLT, ATy,
                                                         AId->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(AId->GetName());
          AId->SetPolymorphicName(AId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        Params.push_back(AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeDouble: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTDoubleNode *DBL = dynamic_cast<const ASTDoubleNode *>(EN);
        assert(DBL && "Failed to dynamic_cast to a DoubleNode!");

        const ASTIdentifierNode *ID = DBL->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTDoubleNode!");

        if (DBL->GetValue() > static_cast<double>(M_PI * 2)) {
          std::stringstream M;
          M << "Angle value exceeds 2pi.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(ID), M.str(),
              DiagLevel::Warning);
        }

        ASTAngleNode *AN =
            ASTAngleNodeMap::Instance().Find(ID->GetName(), ID->GetBits());
        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), ASTAngleNode::AngleBits, ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, DBL, ATy,
                                                         AId->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(AId->GetName());
          AId->SetPolymorphicName(AId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        Params.push_back(AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeMPInteger: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTMPIntegerNode *MPI =
            dynamic_cast<const ASTMPIntegerNode *>(EN);
        assert(MPI && "Failed to dynamic_cast to a MPIntegerNode!");

        const ASTIdentifierNode *ID = MPI->GetIdentifier()->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTMPIntegerNode!");

        mpfr_t MPV;
        mpfr_init2(MPV, MPI->GetBits());
        (void)mpfr_set_z(MPV, MPI->GetMPValue(), MPFR_RNDN);
        mpfr_t MP2PI;
        mpfr_init2(MP2PI, MPI->GetBits());
        (void)mpfr_const_pi(MP2PI, MPFR_RNDN);
        (void)mpfr_mul_d(MP2PI, MP2PI, 2.0, MPFR_RNDN);
        if (mpfr_greater_p(MPV, MP2PI) != 0) {
          std::stringstream M;
          M << "Angle value exceeds 2pi.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(ID), M.str(),
              DiagLevel::Warning);
        }

        ASTAngleNode *AN =
            ASTAngleNodeMap::Instance().Find(ID->GetName(), ID->GetBits());
        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), MPI->GetBits(), ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, MPI, ATy,
                                                         MPI->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(AId->GetName());
          AId->SetPolymorphicName(AId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        Params.push_back(AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeMPDecimal: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTMPDecimalNode *MPD =
            dynamic_cast<const ASTMPDecimalNode *>(EN);
        assert(MPD && "Failed to dynamic_cast to a MPDecimalNode!");

        const ASTIdentifierNode *ID = MPD->GetIdentifier()->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTMPDecimalNode!");

        mpfr_t MP2PI;
        mpfr_init2(MP2PI, MPD->GetBits());
        (void)mpfr_const_pi(MP2PI, MPFR_RNDN);
        (void)mpfr_mul_d(MP2PI, MP2PI, 2.0, MPFR_RNDN);
        if (mpfr_greater_p(MPD->GetMPValue(), MP2PI) != 0) {
          std::stringstream M;
          M << "Angle value exceeds 2pi.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(ID), M.str(),
              DiagLevel::Warning);
        }

        ASTAngleNode *AN =
            ASTAngleNodeMap::Instance().Find(ID->GetName(), ID->GetBits());
        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), MPD->GetBits(), ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, MPD, ATy,
                                                         MPD->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(AId->GetName());
          AId->SetPolymorphicName(AId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        Params.push_back(AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeFunctionCallArgument: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTFunctionCallArgumentNode *FCA =
            dynamic_cast<const ASTFunctionCallArgumentNode *>(EN);
        assert(FCA &&
               "Failed to dynamic_cast to an ASTFunctionCallArgumentNode!");

        const ASTResultNode *RN = FCA->GetResult();
        assert(RN && "Could not obtain a valid function call ASTResultNode!");

        unsigned RBits = RN->GetResultBits() < ASTAngleNode::AngleBits
                             ? ASTAngleNode::AngleBits
                             : RN->GetResultBits();
        std::stringstream SAN;
        SAN << ASTDemangler::TypeName(RN->GetResultType()) << C;
        ASTIdentifierNode *AId = ASTBuilder::Instance().CreateASTIdentifierNode(
            SAN.str(), RBits, ASTTypeAngle);
        assert(AId && "Could not create an Angle ASTIdentifierNode!");

        ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
        ASTAngleNode *AN =
            ASTBuilder::Instance().CreateASTAngleNodeFromExpression(AId, FCA,
                                                                    ATy, RBits);
        assert(AN && "Could not create an ASTAngleNode!");

        AN->SetGateParamName(AId->GetName());
        AId->SetPolymorphicName(AId->GetName());
        AN->Mangle();
        AN->MangleLiteral();
        ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
        ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                       AId->GetSymbolType());
        ASTSymbolTable::Instance().EraseLocalAngle(AId);
        Params.push_back(AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    default:
      break;
    }

    ++C;
  }

  assert(C == Params.size() && "Inconsistent number of Params!");

  C = 0;
  Ty = ASTTypeUndefined;

  for (ASTAnyTypeList::const_iterator I = QL.begin(); I != QL.end(); ++I) {
    Ty = (*I).second;
    switch (Ty) {
    case ASTTypeExpression: {
      const ASTExpression *EN = nullptr;

      try {
        EN = std::any_cast<const ASTExpression *>((*I).first);
        (void)EN; // Quiet.
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeIdentifier: {
      try {
        ASTIdentifierNode *QId = std::any_cast<ASTIdentifierNode *>((*I).first);
        assert(QId && "Invalid ASTIdentifierNode!");

        std::stringstream M;
        ASTSymbolTableEntry *QSTE = QId->GetSymbolTableEntry();
        assert(QSTE && "Gate Qubit Argument has no SymbolTable Entry!");

        if (!QSTE->HasValue()) {
          ASTGateQubitParamNode *QPN =
              new ASTGateQubitParamNode(QId, C, QId->GetBits(), QId->GetName());
          assert(QPN && "Could not create a valid ASTGateQubitParamNode!");
          QPN->Mangle();
          QSTE->ResetValue();
          QSTE->SetValue(new ASTValue<>(QPN, ASTTypeGateQubitParam),
                         ASTTypeGateQubitParam);
        }

        assert(QSTE->HasValue() &&
               "Gate Qubit Parameter SymbolTable Entry has no Value!");

        QCParams.push_back(QSTE);
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeIdentifierRef: {
      try {
        ASTIdentifierRefNode *IdR =
            std::any_cast<ASTIdentifierRefNode *>((*I).first);
        assert(IdR && "Invalid ASTIdentifierNodeRef!");

        ASTIdentifierNode *QId =
            const_cast<ASTIdentifierNode *>(IdR->GetIdentifier());
        assert(QId && "Invalid ASTIdentifierNode!");

        ASTSymbolTableEntry *QSTE = QId->GetSymbolTableEntry();
        assert(QSTE && "Gate Qubit Argument has no SymbolTable Entry!");
        assert(QSTE->HasValue() &&
               "Gate Qubit Argument SymbolTable Entry has no Value!");

        QCParams.push_back(QSTE);
        if (IdR->IsInductionVariable())
          QCParamIds.insert(std::make_pair(C, IdR->GetInductionVariable()));
        else if (IdR->IsIndexedIdentifier())
          QCParamIds.insert(std::make_pair(C, IdR->GetIndexedIdentifier()));
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    default: {
      std::stringstream M;
      M << "Only Qubit Identifiers are allowed as Gate Qubit arguments.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
    } break;
    }

    ++C;
  }

  assert(C == QCParams.size() && "Inconsistent number of Qubit Parameters!");
  Mangle();
}

ASTGateNode::ASTGateNode(const ASTIdentifierNode *Id,
                         const ASTParameterList &PL,
                         const ASTIdentifierList &IL, bool IsGateCall,
                         const ASTGateQOpList &OL)
    : ASTExpressionNode(Id, ASTTypeGate), Params(), Qubits(), QCParams(),
      OpList(OL), Ctrl(nullptr), GDId(IsGateCall ? nullptr : Id), GSTM(),
      ControlType(ASTTypeUndefined), Opaque(false), GateCall(IsGateCall) {
  unsigned C = 0;
  std::set<std::string> PNS;
  std::vector<const ASTIdentifierNode *> NQV;

  if (!IsGateCall && Id->GetName() == u8"U") {
    MaterializeBuiltinUGate(Id, PL, IL);
    return;
  }

  for (ASTParameterList::const_iterator I = PL.begin(); I != PL.end(); ++I) {
    ASTParameter *AP = dynamic_cast<ASTParameter *>(*I);
    assert(AP && "Could not dynamic_cast to an ASTParameter!");

    ASTSymbolTableEntry *ASTE = nullptr;
    ASTAngleNode *AN = nullptr;

    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(AP->GetName())) {
      const ASTSymbolTableEntry *STE =
          ASTSymbolTable::Instance().FindAngle(AP->GetName());
      assert(STE &&
             "Could not obtain a valid reserved Angle SymbolTable Entry!");
      AN = STE->GetValue()->GetValue<ASTAngleNode *>();
    } else {
      const ASTSymbolTableEntry *STE =
          ASTSymbolTable::Instance().Lookup(AP->GetName(), ASTTypeAngle);
      if (STE) {
        if (!STE->HasValue()) {
          ASTAngleNode *XAN = CreateAngleSymbolTableEntry(
              const_cast<ASTSymbolTableEntry *>(STE));
          assert(XAN && "Could not create a valid ASTAngleNode!");
        }

        AN = STE->GetValue()->GetValue<ASTAngleNode *>();
      }
    }

    if (!AN) {
      ASTIdentifierNode *AId = ASTBuilder::Instance().CreateASTIdentifierNode(
          AP->GetName(), ASTAngleNode::AngleBits, ASTTypeAngle);
      assert(AId && "Could not create an Angle ASTIdentifierNode!");

      ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
      AN = ASTBuilder::Instance().CreateASTAngleNode(AId, ATy,
                                                     ASTAngleNode::AngleBits);
      assert(AN && "Could not create a valid ASTAngleNode!");

      AN->SetGateParamName(AId->GetName());
      AId->SetPolymorphicName(AP->GetName());
      ASTE = ASTSymbolTable::Instance().Lookup(AId, AId->GetBits(),
                                               AId->GetSymbolType());
      assert(ASTE &&
             "ASTAngleNode ASTIdentifierNode has no SymbolTable Entry!");

      AId->SetLocalScope();
      ASTE->SetLocalScope();
      AId->SetSymbolTableEntry(ASTE);
      AN->Mangle();
      AN->MangleLiteral();
      ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                     AId->GetSymbolType());
      ToGateParamSymbolTable(AId, ASTE);
      ASTSymbolTable::Instance().EraseLocalSymbol(AId, AId->GetBits(),
                                                  AId->GetSymbolType());
      ASTSymbolTable::Instance().EraseLocalAngle(AId);
      ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                            AId->GetSymbolType());
    } else {
      const ASTIdentifierNode *AId = AN->GetIdentifier();
      assert(AId && "ASTAngleNode has an invalid ASTIdentifierNode!");

      ASTE = ASTSymbolTable::Instance().Lookup(AId->GetName(), AId->GetBits(),
                                               ASTTypeAngle);
      assert(ASTE && "ASTIdentifierNode did not create a SymbolTable Entry!");

      if (const ASTIdentifierNode *PId = AP->GetIdentifier())
        if (AId->GetBits() == PId->GetBits())
          AN = AN->Clone(AId);
        else
          AN = AN->Clone(AId, PId->GetBits());
      else
        AN = AN->Clone(AId);

      assert(AN && "Could not clone a valid ASTAngleNode!");

      AN->SetGateParamName(AId->GetName());
      AId->SetPolymorphicName(AId->GetName());
      ASTE->ResetValue();
      ASTE->SetValue(new ASTValue<>(AN, ASTTypeAngle), ASTTypeAngle);
      assert(ASTE->HasValue() &&
             "ASTAngleNode SymbolTable Entry has no Value!");

      ASTE->SetLocalScope();
      AN->Mangle();
      AN->MangleLiteral();
      ToGateParamSymbolTable(AId, ASTE);
      ASTSymbolTable::Instance().EraseLocalSymbol(AId, AId->GetBits(),
                                                  AId->GetSymbolType());
      ASTSymbolTable::Instance().EraseLocalAngle(AId);
      ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                            AId->GetSymbolType());
      const_cast<ASTIdentifierNode *>(AId)->SetLocalScope();
      const_cast<ASTIdentifierNode *>(AId)->SetSymbolTableEntry(ASTE);
      const_cast<ASTIdentifierNode *>(AId)->SetHasSymbolTableEntry(true);
    }

    if (const ASTIdentifierNode *PId =
            ASTBuilder::Instance().FindASTIdentifierNode(AP->GetName())) {
      PId->SetNoQubit(true);
    }

    Params.push_back(AN);
    PNS.insert(AP->GetName());
    PNS.insert(AN->GetName());
    ++C;
  }

  assert(C == Params.size() && "Inconsistent number of Params!");

  for (std::vector<ASTAngleNode *>::const_iterator I = Params.begin();
       I != Params.end(); ++I) {
    const ASTIdentifierNode *AId = (*I)->GetIdentifier();
    assert(AId && "Invalid ASTIdentifierNode!");

    ASTSymbolTable::Instance().EraseLocalAngle(AId->GetName(), AId->GetBits(),
                                               ASTTypeAngle);
    ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                          AId->GetSymbolType());
  }

  C = 0;

  for (ASTIdentifierList::const_iterator I = IL.begin(); I != IL.end(); ++I) {
    ASTIdentifierNode *QId = const_cast<ASTIdentifierNode *>(*I);
    assert(QId && "Invalid Qubit ASTIdentifierNode!");

    ASTIdentifierRefNode *IdR = nullptr;

    if (QId->IsReference() &&
        (IdR = dynamic_cast<ASTIdentifierRefNode *>(QId))) {
      std::stringstream M;
      M << "Indexed Qubit register references cannot be used as Gate "
        << "Qubit Parameters.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
    }

    if ((PNS.find(QId->GetName()) != PNS.end()) || QId->IsNoQubit()) {
      NQV.push_back(QId);
      continue;
    }

    MaterializeGateQubitParam(QId);

    ASTSymbolTableEntry *STE = nullptr;
    std::map<std::string, const ASTSymbolTableEntry *>::iterator QI =
        GSTM.find(QId->GetName());

    if (QI != GSTM.end())
      STE = const_cast<ASTSymbolTableEntry *>((*QI).second);

    if (!STE) {
      STE = new ASTSymbolTableEntry(QId, QId->GetSymbolType());
      assert(STE && "Could not create a valid SymbolTable Entry!");
    }

    if (STE && STE->GetValueType() == ASTTypeUndefined) {
      if (GateCall) {
        std::stringstream M;
        M << "Undefined Types (" << QId->GetName() << ") are not "
          << "permitted in a Gate Call.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(QId), M.str(),
            DiagLevel::ICE);
      }

      QId->SetGateLocal();
      QId->SetLocalScope();
      QId->SetBits(1);
      STE->SetLocalScope();
      STE->SetValueType(ASTTypeGateQubitParam);
    }

    QId->SetLocalScope();

    if (!STE->HasValue() &&
        (QId->GetSymbolType() == ASTTypeQubitContainer ||
         QId->GetSymbolType() == ASTTypeQubitContainerAlias)) {
      std::stringstream M;
      M << "An uninitialized qubit container cannot be used as "
           "gate call argument.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(QId), M.str(),
          DiagLevel::Error);
    }

    unsigned Bits = 0U;
    unsigned QBits = 0U;

    switch (QId->GetSymbolType()) {
    case ASTTypeQubitContainer: {
      if (ASTQubitContainerNode *QCN =
              STE->GetValue()->GetValue<ASTQubitContainerNode *>()) {
        Bits = QCN->Size();
        QBits = 1U;
      }
    } break;
    case ASTTypeQubitContainerAlias: {
      if (ASTQubitContainerAliasNode *QCAN =
              STE->GetValue()->GetValue<ASTQubitContainerAliasNode *>()) {
        Bits = QCAN->Size();
        QBits = 1U;
      }
    } break;
    case ASTTypeQubit:
    case ASTTypeGateQubitParam:
      Bits = QBits = 1U;
      break;
    default:
      break;
    }

    if (STE) {
      STE->SetLocalScope();
      QCParams.push_back(STE);
    }

    STE = MangleGateQubitParam(QId, STE, C, Bits, QBits);
    ToGateParamSymbolTable(QId, STE);

    if (IdR)
      ToGateParamSymbolTable(IdR, STE);

    if (!QId->IsGateLocal()) {
      switch (STE->GetValueType()) {
      case ASTTypeQubit:
      case ASTTypeQubitContainer:
      case ASTTypeQubitContainerAlias:
      case ASTTypeGateQubitParam:
        break;
      default:
        continue;
        break;
      }
    }

    ASTGateQubitTracker::Instance().Insert(QId);

    // Do not insert the gate identifier into the qubit param's
    // ast identifier. It will cause scope resolution conflicts.
    std::stringstream QS;
    QS << "ast-gate-qubit-param-" << QId->GetName() << '-' << C;

    ASTIdentifierNode *QBId =
        new ASTIdentifierNode(QS.str(), ASTTypeGateQubitParam, 0U);
    assert(QBId && "Could not create a valid Qubit ASTIdentifierNode!");

    QBId->SetPolymorphicName(QId->GetName());
    Qubits.push_back(new ASTQubitNode(QBId, C, QId->GetName()));
    Qubits.back()->Mangle();
    ASTQubitNodeBuilder::Instance().Append(Qubits.back());
    ASTSymbolTable::Instance().EraseGateLocalQubit(QId->GetName());
    ++C;
  }

  assert(C == Qubits.size() && "Inconsistent number of Qubits!");

  OL.TransferToSymbolTable(GSTM);
  Mangle();
}

void ASTGateNode::print() const {
  std::cout << "<Gate>" << std::endl;
  std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
  std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
            << std::endl;
  std::cout << "<Opaque>" << std::boolalpha << Opaque << "</Opaque>"
            << std::endl;
  std::cout << "<GateCall>" << std::boolalpha << GateCall << "</GateCall>"
            << std::endl;

  if (GateCall) {
    std::cout << "<GateDefinitionName>" << GDId->GetName()
              << "</GateDefinitionName>" << std::endl;
  }

  if (!Params.empty()) {
    std::cout << "<Params>" << std::endl;
    for (std::vector<ASTAngleNode *>::const_iterator I = Params.begin();
         I != Params.end(); ++I)
      (*I)->print();
    std::cout << "</Params>" << std::endl;
  }

  if (!Qubits.empty()) {
    std::cout << "<Qubits>" << std::endl;
    for (std::vector<ASTQubitNode *>::const_iterator I = Qubits.begin();
         I != Qubits.end(); ++I)
      (*I)->print();
    std::cout << "</Qubits>" << std::endl;
  }

  if (!QCParams.empty() && Qubits.empty()) {
    std::cout << "<QubitParams>" << std::endl;
    unsigned XC = 0;
    std::map<unsigned, const ASTIdentifierNode *>::const_iterator MI;

    for (std::vector<const ASTSymbolTableEntry *>::const_iterator I =
             QCParams.begin();
         I != QCParams.end(); ++I) {
      const ASTIdentifierNode *QId = (*I)->GetIdentifier();
      assert(QId &&
             "Invalid ASTIdentifierNode obtained from the SymbolTable Entry!");
      std::cout << "<QubitParam>" << std::endl;
      const std::string &QN = QId->GetName();
      MI = QCParamIds.find(XC);
      if (MI != QCParamIds.end() && ASTStringUtils::Instance().IsIndexed(QN)) {
        std::string BN = ASTStringUtils::Instance().GetIdentifierBase(QN);
        std::cout << "<Name>" << BN << '[' << (*MI).second->GetName() << ']'
                  << "</Name>" << std::endl;
      } else {
        std::cout << "<Name>" << QId->GetName() << "</Name>" << std::endl;
      }
      std::cout << "</QubitParam>" << std::endl;
      ++XC;
    }

    std::cout << "</QubitParams>" << std::endl;
  }

  if (!OpList.Empty())
    OpList.print();

  switch (ControlType) {
  case ASTTypeGateControl:
  case ASTTypeGateNegControl:
  case ASTTypeGateInverse:
  case ASTTypeGatePower:
    std::cout << "<ControlType>";
    std::cout << PrintTypeEnum(ControlType);
    std::cout << "</ControlType>" << std::endl;
    break;
  default:
    break;
  }

  std::cout << "</Gate>" << std::endl;
}

ASTGateControlNode::ASTGateControlNode(const ASTGateNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GN(N),
      TType(ASTTypeGate), MV(nullptr), MType(ASTTypeUndefined), CBits(1U) {
  GN->SetControl(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GN(N),
      TType(ASTTypeGate), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  GN->SetControl(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateQOpNode *QN)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GQN(QN),
      TType(ASTTypeGateQOpNode), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {}

ASTGateControlNode::ASTGateControlNode(const ASTGateQOpNode *QN,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GQN(QN),
      TType(ASTTypeGateQOpNode), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {}

ASTGateControlNode::ASTGateControlNode(const ASTGateControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), CN(N),
      TType(ASTTypeGateControl), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateControlNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), CN(N),
      TType(ASTTypeGateControl), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateNegControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), NCN(N),
      TType(ASTTypeGateNegControl), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateNegControlNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), NCN(N),
      TType(ASTTypeGateNegControl), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGatePowerNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), PN(N),
      TType(ASTTypeGatePower), MV(nullptr), MType(ASTTypeUndefined), CBits(1U) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGatePowerNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), PN(N),
      TType(ASTTypeGatePower), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateInverseNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), IN(N),
      TType(ASTTypeGateInverse), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateInverseNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), IN(N),
      TType(ASTTypeGateInverse), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GPN(N),
      TType(ASTTypeGPhaseExpression), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {}

ASTGateControlNode::ASTGateControlNode(const ASTGPhaseExpressionNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GPN(N),
      TType(ASTTypeGPhaseExpression), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {}

ASTGateControlNode::ASTGateControlNode(const ASTGateGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GGEN(N),
      TType(ASTTypeGateGPhaseExpression), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {}

ASTGateControlNode::ASTGateControlNode(const ASTGateGPhaseExpressionNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GGEN(N),
      TType(ASTTypeGateGPhaseExpression), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GN(N), TType(ASTTypeGate), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  GN->SetNegControl(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateNode *N,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GN(N), TType(ASTTypeGate), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  GN->SetNegControl(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateQOpNode *QN)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GQN(QN), TType(ASTTypeGateQOpNode), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateQOpNode *QN,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GQN(QN), TType(ASTTypeGateQOpNode), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      CN(N), TType(ASTTypeGateControl), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateControlNode *N,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      CN(N), TType(ASTTypeGateControl), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateNegControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      NCN(N), TType(ASTTypeGateNegControl), MV(nullptr),
      MType(ASTTypeUndefined), CBits(1U) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateNegControlNode *N,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      NCN(N), TType(ASTTypeGateNegControl), MV(nullptr),
      MType(ASTTypeUndefined), CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGatePowerNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      PN(N), TType(ASTTypeGatePower), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGatePowerNode *N,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      PN(N), TType(ASTTypeGatePower), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateInverseNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      IN(N), TType(ASTTypeGateInverse), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateInverseNode *N,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      IN(N), TType(ASTTypeGateInverse), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GPN(N), TType(ASTTypeGPhaseExpression), MV(nullptr),
      MType(ASTTypeUndefined), CBits(1U) {}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGPhaseExpressionNode *N,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GPN(N), TType(ASTTypeGPhaseExpression), MV(nullptr),
      MType(ASTTypeUndefined), CBits(CB->GetUnsignedValue()) {}

ASTGateNegControlNode::ASTGateNegControlNode(
    const ASTGateGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GGEN(N), TType(ASTTypeGateGPhaseExpression), MV(nullptr),
      MType(ASTTypeUndefined), CBits(1U) {}

ASTGateNegControlNode::ASTGateNegControlNode(
    const ASTGateGPhaseExpressionNode *N, const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GGEN(N), TType(ASTTypeGateGPhaseExpression), MV(nullptr),
      MType(ASTTypeUndefined), CBits(CB->GetUnsignedValue()) {}

ASTGateInverseNode::ASTGateInverseNode(const ASTGateNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), GN(N),
      TType(ASTTypeGate), MV(nullptr), MType(ASTTypeUndefined) {
  GN->SetInverse(this);
}

ASTGateInverseNode::ASTGateInverseNode(const ASTGateQOpNode *QN)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), GQN(QN),
      TType(ASTTypeGateQOpNode), MV(nullptr), MType(ASTTypeUndefined) {}

ASTGateInverseNode::ASTGateInverseNode(const ASTGateControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), CN(N),
      TType(ASTTypeGateControl), MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGateInverseNode::ASTGateInverseNode(const ASTGateNegControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), NCN(N),
      TType(ASTTypeGateNegControl), MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGateInverseNode::ASTGateInverseNode(const ASTGatePowerNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), PN(N),
      TType(ASTTypeGatePower), MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGateInverseNode::ASTGateInverseNode(const ASTGateInverseNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), IN(N),
      TType(ASTTypeGateInverse), MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGateInverseNode::ASTGateInverseNode(const ASTGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), GPN(N),
      TType(ASTTypeGPhaseExpression), MV(nullptr), MType(ASTTypeUndefined) {}

ASTGateInverseNode::ASTGateInverseNode(const ASTGateGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), GGEN(N),
      TType(ASTTypeGateGPhaseExpression), MV(nullptr), MType(ASTTypeUndefined) {
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNode *N, const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      GN(N), EType(Exp->GetASTType()), TType(ASTTypeGate), MV(nullptr),
      MType(ASTTypeUndefined) {
  GN->SetPower(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNode *N,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      GN(N), EType(Exp->GetASTType()), TType(ASTTypeGate), MV(nullptr),
      MType(ASTTypeUndefined) {
  GN->SetPower(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNode *N,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      GN(N), EType(Exp->GetASTType()), TType(ASTTypeGate), MV(nullptr),
      MType(ASTTypeUndefined) {
  GN->SetPower(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNode *N,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      GN(N), EType(Exp->GetASTType()), TType(ASTTypeGate), MV(nullptr),
      MType(ASTTypeUndefined) {
  GN->SetPower(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateQOpNode *QN)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), GQN(QN),
      EType(ASTTypeUndefined), TType(ASTTypeGateQOpNode), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateQOpNode *QN,
                                   const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      GQN(QN), EType(Exp->GetASTType()), TType(ASTTypeGateQOpNode), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateQOpNode *QN,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      GQN(QN), EType(Exp->GetASTType()), TType(ASTTypeGateQOpNode), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateQOpNode *QN,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      GQN(QN), EType(Exp->GetASTType()), TType(ASTTypeGateQOpNode), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateQOpNode *QN,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      GQN(QN), EType(Exp->GetASTType()), TType(ASTTypeGateQOpNode), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), CN(N),
      EType(ASTTypeUndefined), TType(ASTTypeGateControl), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateControlNode *N,
                                   const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      CN(N), EType(Exp->GetASTType()), TType(ASTTypeGateControl), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateControlNode *N,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      CN(N), EType(Exp->GetASTType()), TType(ASTTypeGateControl), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateControlNode *N,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      CN(N), EType(Exp->GetASTType()), TType(ASTTypeGateControl), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateControlNode *N,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      CN(N), EType(Exp->GetASTType()), TType(ASTTypeGateControl), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNegControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), NCN(N),
      EType(ASTTypeUndefined), TType(ASTTypeGateNegControl), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNegControlNode *N,
                                   const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      NCN(N), EType(Exp->GetASTType()), TType(ASTTypeGateNegControl),
      MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNegControlNode *N,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      NCN(N), EType(Exp->GetASTType()), TType(ASTTypeGateNegControl),
      MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNegControlNode *N,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      NCN(N), EType(Exp->GetASTType()), TType(ASTTypeGateNegControl),
      MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNegControlNode *N,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      NCN(N), EType(Exp->GetASTType()), TType(ASTTypeGateNegControl),
      MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGatePowerNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), PN(N),
      EType(ASTTypeUndefined), TType(ASTTypeGatePower), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGatePowerNode *N,
                                   const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      PN(N), EType(Exp->GetASTType()), TType(ASTTypeGatePower), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGatePowerNode *N,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      PN(N), EType(Exp->GetASTType()), TType(ASTTypeGatePower), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGatePowerNode *N,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      PN(N), EType(Exp->GetASTType()), TType(ASTTypeGatePower), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGatePowerNode *N,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      PN(N), EType(Exp->GetASTType()), TType(ASTTypeGatePower), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateInverseNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), IN(N),
      EType(ASTTypeUndefined), TType(ASTTypeGateInverse), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateInverseNode *N,
                                   const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      IN(N), EType(Exp->GetASTType()), TType(ASTTypeGateInverse), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateInverseNode *N,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      IN(N), EType(Exp->GetASTType()), TType(ASTTypeGateInverse), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateInverseNode *N,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      IN(N), EType(Exp->GetASTType()), TType(ASTTypeGateInverse), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateInverseNode *N,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      IN(N), EType(Exp->GetASTType()), TType(ASTTypeGateInverse), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), GPN(N),
      EType(ASTTypeUndefined), TType(ASTTypeGPhaseExpression), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), GGEN(N),
      EType(ASTTypeUndefined), TType(ASTTypeGateGPhaseExpression), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N,
                                   const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      GGEN(N), EType(Exp->GetASTType()), TType(ASTTypeGateGPhaseExpression),
      MV(nullptr), MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      GGEN(N), EType(Exp->GetASTType()), TType(ASTTypeGateGPhaseExpression),
      MV(nullptr), MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      GGEN(N), EType(Exp->GetASTType()), TType(ASTTypeGateGPhaseExpression),
      MV(nullptr), MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      GGEN(N), EType(Exp->GetASTType()), TType(ASTTypeGateGPhaseExpression),
      MV(nullptr), MType(ASTTypeUndefined) {}

void ASTGateControlNode::print() const {
  std::cout << "<GateControlNode>" << std::endl;
  std::cout << "<Target>" << std::endl;

  switch (TType) {
  case ASTTypeGate:
    GN->print();
    break;
  case ASTTypeGateQOpNode:
    GQN->print();
    break;
  case ASTTypeGPhaseExpression:
    GPN->print();
    break;
  case ASTTypeGateGPhaseExpression:
    GGEN->print();
    break;
  case ASTTypeGateControl:
    CN->print();
    break;
  case ASTTypeGateNegControl:
    NCN->print();
    break;
  case ASTTypeGatePower:
    PN->print();
    break;
  case ASTTypeGateInverse:
    IN->print();
    break;
  default:
    break;
  }

  std::cout << "</Target>" << std::endl;

  std::cout << "<HasModifier>" << std::boolalpha << HasModifier()
            << "</HasModifier>" << std::endl;
  std::cout << "<ControlBits>" << CBits << "</ControlBits>" << std::endl;

  std::cout << "</GateControlNode>" << std::endl;
}

void ASTGateNegControlNode::print() const {
  std::cout << "<GateNegControlNode>" << std::endl;
  std::cout << "<Target>" << std::endl;

  switch (TType) {
  case ASTTypeGate:
    GN->print();
    break;
  case ASTTypeGateQOpNode:
    GQN->print();
    break;
  case ASTTypeGPhaseExpression:
    GPN->print();
    break;
  case ASTTypeGateGPhaseExpression:
    GGEN->print();
    break;
  case ASTTypeGateControl:
    CN->print();
    break;
  case ASTTypeGateNegControl:
    NCN->print();
    break;
  case ASTTypeGatePower:
    PN->print();
    break;
  case ASTTypeGateInverse:
    IN->print();
    break;
  default:
    break;
  }

  std::cout << "</Target>" << std::endl;

  std::cout << "<HasModifier>" << std::boolalpha << HasModifier()
            << "</HasModifier>" << std::endl;
  std::cout << "<ControlBits>" << CBits << "</ControlBits>" << std::endl;

  std::cout << "</GateNegControlNode>" << std::endl;
}

void ASTGateInverseNode::print() const {
  std::cout << "<GateInverseNode>" << std::endl;
  std::cout << "<Target>" << std::endl;

  switch (TType) {
  case ASTTypeGate:
    GN->print();
    break;
  case ASTTypeGateQOpNode:
    GQN->print();
    break;
  case ASTTypeGPhaseExpression:
    GPN->print();
    break;
  case ASTTypeGateGPhaseExpression:
    GGEN->print();
    break;
  case ASTTypeGateControl:
    CN->print();
    break;
  case ASTTypeGateNegControl:
    NCN->print();
    break;
  case ASTTypeGatePower:
    PN->print();
    break;
  case ASTTypeGateInverse:
    IN->print();
    break;
  default:
    break;
  }

  std::cout << "</Target>" << std::endl;

  std::cout << "<HasModifier>" << std::boolalpha << HasModifier()
            << "</HasModifier>" << std::endl;

  std::cout << "</GateInverseNode>" << std::endl;
}

void ASTGatePowerNode::print() const {
  std::cout << "<GatePowerNode>" << std::endl;

  std::cout << "<Exponent>" << std::endl;
  if (EType == ASTTypeInt)
    I->print();
  else if (EType == ASTTypeIdentifier)
    ID->print();
  else if (EType == ASTTypeBinaryOp)
    BOP->print();
  else if (EType == ASTTypeUnaryOp)
    UOP->print();
  std::cout << "</Exponent>" << std::endl;

  std::cout << "<Target>" << std::endl;

  switch (TType) {
  case ASTTypeGate:
    std::cout << "<GateTarget>" << std::endl;
    GN->print();
    std::cout << "</GateTarget>" << std::endl;
    break;
  case ASTTypeGateQOpNode:
    GQN->print();
    break;
  case ASTTypeGPhaseExpression:
    GPN->print();
    break;
  case ASTTypeGateGPhaseExpression:
    GGEN->print();
    break;
  case ASTTypeGateControl:
    CN->print();
    break;
  case ASTTypeGateNegControl:
    NCN->print();
    break;
  case ASTTypeGatePower:
    PN->print();
    break;
  case ASTTypeGateInverse:
    IN->print();
    break;
  default:
    break;
  }

  std::cout << "</Target>" << std::endl;

  std::cout << "<HasModifier>" << std::boolalpha << HasModifier()
            << "</HasModifier>" << std::endl;

  std::cout << "</GatePowerNode>" << std::endl;
}

GateKind ASTGateNode::DetermineGateKind(const std::string &GN) {
  if (GN == u8"h")
    return ASTGateKindH;
  else if (GN == u8"U")
    return ASTGateKindU;

  std::string N = ASTStringUtils::Instance().ToLower(GN);

  if (N == u8"cx")
    return ASTGateKindCX;
  else if (N == u8"ccx")
    return ASTGateKindCCX;
  else if (N == u8"cnot")
    return ASTGateKindCNOT;

  return ASTGateKindGeneric;
}

ASTType ASTGateNode::DetermineGateType(const std::string &GN) {
  std::string N = ASTStringUtils::Instance().ToLower(GN);
  return ASTTypeGate;
}

void ASTGateNode::MaterializeGateQubitParam(ASTIdentifierNode *Id) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  const ASTDeclarationContext *CTX = Id->GetDeclarationContext();
  const ASTDeclarationContext *CCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
  ASTSymbolTableEntry *STE = Id->GetSymbolTableEntry();

  if (GateCall) {
    if (Id->GetSymbolType() == ASTTypeUndefined) {
      std::stringstream M;
      M << "Undefined Types (" << Id->GetName() << ") are not "
        << "permitted in a Gate Call.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    }

    if (CTX != CCX) {
      std::stringstream M;
      M << "Gate declaration context mismatch.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    }
  }

  if (Id->GetSymbolType() == ASTTypeUndefined) {
    Id->SetSymbolType(ASTTypeGateQubitParam);
    Id->SetBits(1U);
    Id->SetLocalScope();
    STE->SetValueType(ASTTypeGateQubitParam);
    STE->SetLocalScope();
  }

  if (Id->GetSymbolType() == ASTTypeGateQubitParam) {
    unsigned Bits = 1U;
    if (ASTStringUtils::Instance().IsIndexed(Id->GetName()))
      Bits = ASTStringUtils::Instance().GetIdentifierIndex(Id->GetName());
    assert(!ASTIdentifierNode::InvalidBits(Bits) &&
           "Invalid number of bits for ASTGateQubitParam!");

    if (!GSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
      std::stringstream M;
      M << "Failure inserting Qubit Param into the Gate Symbol Table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    }

    ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(),
                                                Id->GetSymbolType());
  } else {
    if (!GateCall) {
      std::stringstream M;
      M << "Only Qubit Parameters are permitted in a Gate declaration.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    }

    if (!GSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
      std::stringstream M;
      M << "Failure inserting Qubit Argument into the Gate Symbol Table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    }
  }
}

ASTIdentifierNode *ASTGateNode::GateCallIdentifier(const std::string &Name,
                                                   ASTType GTy,
                                                   unsigned Bits) const {
  assert(!Name.empty() && "Invalid Gate Name!");

  ASTIdentifierNode *GId = new ASTIdentifierNode(Name, GTy, Bits);
  assert(GId && "Could not create a valid GateCall ASTIdentifierNode!");

  return GId;
}

ASTGateNode *ASTGateNode::CloneCall(const ASTIdentifierNode *Id,
                                    const ASTArgumentNodeList &AL,
                                    const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTGateNode *RG = new ASTGateNode(GId, AL, QL, true);
  assert(RG && "Could not create a valid ASTGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTUGateNode *ASTUGateNode::CloneCall(const ASTIdentifierNode *Id,
                                      const ASTArgumentNodeList &AL,
                                      const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTUGateNode *RG = new ASTUGateNode(GId, AL, QL, true);
  assert(RG && "Could not create a valid ASTUGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTCXGateNode *ASTCXGateNode::CloneCall(const ASTIdentifierNode *Id,
                                        const ASTArgumentNodeList &AL,
                                        const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTCXGateNode *RG = new ASTCXGateNode(GId, AL, QL, true);
  assert(RG && "Could not create a valid ASTCXGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTCXGateNode *ASTCXGateNode::CloneCall(const ASTIdentifierNode *Id,
                                        const ASTParameterList &PL,
                                        const ASTIdentifierList &IL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTCXGateNode *RG = new ASTCXGateNode(GId, PL, IL, true);
  assert(RG && "Could not create a valid ASTCXGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTCCXGateNode *ASTCCXGateNode::CloneCall(const ASTIdentifierNode *Id,
                                          const ASTArgumentNodeList &AL,
                                          const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTCCXGateNode *RG = new ASTCCXGateNode(GId, AL, QL, true);
  assert(RG && "Could not create a valid ASTCCXGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTCCXGateNode *ASTCCXGateNode::CloneCall(const ASTIdentifierNode *Id,
                                          const ASTParameterList &PL,
                                          const ASTIdentifierList &IL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTCCXGateNode *RG = new ASTCCXGateNode(GId, PL, IL, true);
  assert(RG && "Could not create a valid ASTCXGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTHadamardGateNode *
ASTHadamardGateNode::CloneCall(const ASTIdentifierNode *Id,
                               const ASTArgumentNodeList &AL,
                               const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTHadamardGateNode *RG = new ASTHadamardGateNode(GId, AL, QL, true);
  assert(RG && "Could not create a valid ASTHadamardGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTCNotGateNode *ASTCNotGateNode::CloneCall(const ASTIdentifierNode *Id,
                                            const ASTArgumentNodeList &AL,
                                            const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTCNotGateNode *RG = new ASTCNotGateNode(GId, AL, QL, true);
  assert(RG && "Could not create a valid ASTCNotGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

void ASTGateNode::Mangle() {
  ASTMangler M;
  M.Start();

  if (GateCall)
    M.TypeIdentifier(ASTTypeGateCall, GetName());
  else
    M.TypeIdentifier(GetASTType(), GetName());

  if (GateCall) {
    unsigned X = 0;
    if (!Params.empty()) {
      for (unsigned I = 0; I < Params.size(); ++I) {
        if (Params[I]->IsExpression()) {
          if (const ASTExpressionNode *EXN = Params[I]->GetExpression()) {
            switch (EXN->GetASTType()) {
            case ASTTypeBinaryOp: {
              const ASTBinaryOpNode *BOP =
                  dynamic_cast<const ASTBinaryOpNode *>(EXN);
              assert(BOP && "Invalid dynamic_cast to an ASTBinaryOpNode!");
              M.GateArg(I, ASTStringUtils::Instance().SanitizeMangled(
                               BOP->GetMangledName()));
              X = I;
            } break;
            case ASTTypeUnaryOp: {
              const ASTUnaryOpNode *UOP =
                  dynamic_cast<const ASTUnaryOpNode *>(EXN);
              assert(UOP && "Invalid dynamic_cast to an ASTUnaryOpNode!");
              M.GateArg(I, ASTStringUtils::Instance().SanitizeMangled(
                               UOP->GetMangledName()));
              X = I;
            } break;
            default:
              M.GateArg(I, ASTStringUtils::Instance().SanitizeMangled(
                               Params[I]->GetMangledName()));
              X = I;
              break;
            }
          }
        } else {
          M.GateArg(I, ASTStringUtils::Instance().SanitizeMangled(
                           Params[I]->GetMangledName()));
          X = I;
        }
      }

      X += 1U;
    }

    if (!QCParams.empty()) {
      for (unsigned I = 0; I < QCParams.size(); ++I) {
        M.GateArg(X + I, ASTStringUtils::Instance().SanitizeMangled(
                             QCParams[I]->GetIdentifier()->GetMangledName()));
      }
    }
  } else {
    unsigned X = 0;
    if (!Params.empty()) {
      for (unsigned I = 0; I < Params.size(); ++I) {
        M.GateParam(I, ASTStringUtils::Instance().SanitizeMangled(
                           Params[I]->GetMangledName()));
        X = I;
      }

      X += 1U;
    }

    if (!Qubits.empty()) {
      for (unsigned I = 0; I < Qubits.size(); ++I) {
        M.GateParam(X + I, ASTTypeQubit, 1U,
                    Qubits[I]->GetIdentifier()->GetGateParamName());
      }
    }
  }

  if (GateCall)
    M.CallEnd();

  M.End();

  const_cast<ASTIdentifierNode *>(GetIdentifier())
      ->SetMangledName(M.AsString(), true);
}

ASTGateNode *ASTGateControlNode::Resolve() {
  // FIXME: IMPLEMENT.
  return GN->GetControlType() == ASTTypeUndefined
             ? const_cast<ASTGateNode *>(GN)
             : nullptr;
}

ASTGateNode *ASTGateNegControlNode::Resolve() {
  // FIXME: IMPLEMENT.
  return GN->GetControlType() == ASTTypeUndefined
             ? const_cast<ASTGateNode *>(GN)
             : nullptr;
}

ASTGateNode *ASTGateInverseNode::Resolve() {
  // FIXME: IMPLEMENT.
  return GN->GetControlType() == ASTTypeUndefined
             ? const_cast<ASTGateNode *>(GN)
             : nullptr;
}

std::vector<ASTGateNode *> ASTGatePowerNode::Resolve() {
  std::vector<ASTGateNode *> V;

  // FIXME: IMPLEMENT
  if (GN->GetControlType() == ASTTypeUndefined)
    V.push_back(const_cast<ASTGateNode *>(GN));

  return V;
}

bool ASTGateQOpList::TransferToSymbolTable(
    std::map<std::string, const ASTSymbolTableEntry *> &MM) const {
  if (List.empty())
    return true;

  for (std::vector<ASTGateQOpNode *>::const_iterator I = List.begin();
       I != List.end(); ++I) {
    const ASTIdentifierNode *Id = (*I)->GetIdentifier();
    assert(Id && "Could not obtain a valid ASTIdentifierNode!");

    if (ASTTypeSystemBuilder::Instance().IsBuiltinGate(Id->GetName()))
      continue;

    const ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().FindLocal(Id);
    if (STE) {
      if (!MM.insert(std::make_pair(Id->GetName(), STE)).second) {
        std::stringstream M;
        M << "Failed to insert ASTGateQOp " << Id->GetName() << " into the "
          << "Gate SymbolTable.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
            DiagLevel::ICE);
        return false;
      }

      ASTSymbolTable::Instance().EraseLocal(Id, Id->GetBits(),
                                            Id->GetSymbolType());
    }
  }

  return true;
}

} // namespace QASM
