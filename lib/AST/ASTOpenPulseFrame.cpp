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

#include <qasm/AST/ASTAngleNodeBuilder.h>
#include <qasm/AST/ASTArray.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseFrame.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <cassert>
#include <climits>
#include <vector>

namespace QASM {
namespace OpenPulse {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTOpenPulseFrameNodeResolver ASTOpenPulseFrameNodeResolver::FRN;

ASTAngleNode *
ASTOpenPulseFrameNodeResolver::ResolveAngle(ASTExpressionNode *E) {
  assert(E && "Invalid ASTExpression argument!");

  ASTAngleNode *AN = nullptr;

  switch (E->GetASTType()) {
  case ASTTypeDouble: {
    ASTDoubleNode *DN = dynamic_cast<ASTDoubleNode *>(E);
    if (!DN) {
      std::stringstream M;
      M << "Invalid second element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    AN = new ASTAngleNode(&ASTIdentifierNode::Angle, DN, ASTAngleTypeGeneric,
                          64);
  } break;
  case ASTTypeFloat: {
    ASTFloatNode *FN = dynamic_cast<ASTFloatNode *>(E);
    if (!FN) {
      std::stringstream M;
      M << "Invalid second element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    AN = new ASTAngleNode(&ASTIdentifierNode::Angle, FN, ASTAngleTypeGeneric,
                          32);
  } break;
  case ASTTypeAngle: {
    AN = dynamic_cast<ASTAngleNode *>(E);
    if (!AN) {
      std::stringstream M;
      M << "Invalid second element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }
  } break;
  case ASTTypeMPDecimal: {
    ASTMPDecimalNode *MPD = dynamic_cast<ASTMPDecimalNode *>(E);
    if (!MPD) {
      std::stringstream M;
      M << "Invalid second element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    AN = new ASTAngleNode(&ASTIdentifierNode::Angle, MPD, ASTAngleTypeGeneric,
                          MPD->GetBits());
  } break;
  case ASTTypeIdentifier: {
    ASTIdentifierNode *AId =
        const_cast<ASTIdentifierNode *>(E->GetIdentifier());
    if (!AId) {
      std::stringstream M;
      M << "Invalid second element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(AId->GetName())) {
      AN = ASTAngleNodeBuilder::Instance().FindInMap(AId->GetName(),
                                                     ASTAngleNode::AngleBits);
      if (AN)
        return AN;
    }

    std::vector<ASTSymbolTableEntry *> STV =
        ASTSymbolTable::Instance().LookupRange(AId);
    std::vector<std::vector<ASTSymbolTableEntry *>::iterator> EV;

    for (std::vector<ASTSymbolTableEntry *>::iterator SI = STV.begin();
         SI != STV.end(); ++SI) {
      if ((E)->GetIdentifier()->GetBits() != AId->GetBits())
        EV.push_back(SI);
    }

    for (std::vector<std::vector<ASTSymbolTableEntry *>::iterator>::iterator
             EI = EV.begin();
         EI != EV.end(); ++EI)
      STV.erase(*EI);

    for (std::vector<ASTSymbolTableEntry *>::iterator SI = STV.begin();
         SI != STV.end(); ++SI) {
      ASTSymbolTableEntry *ASTE = *SI;
      assert(ASTE && "Invalid ASTSymbolTableEntry in IdentifierRange!");

      switch (ASTE->GetValueType()) {
      case ASTTypeFloat: {
        ASTFloatNode *FN = ASTE->GetValue()->GetValue<ASTFloatNode *>();
        assert(FN &&
               "Could not obtain a valid ASTFloatNode from the SymbolTable!");
        AN = new ASTAngleNode(&ASTIdentifierNode::Angle, FN,
                              ASTAngleTypeGeneric, 32);
        break;
      } break;
      case ASTTypeDouble: {
        ASTDoubleNode *DN = ASTE->GetValue()->GetValue<ASTDoubleNode *>();
        assert(DN &&
               "Could not obtain a valid ASTDoubleNode from the SymbolTable!");
        AN = new ASTAngleNode(&ASTIdentifierNode::Angle, DN,
                              ASTAngleTypeGeneric, 64);
        break;
      } break;
      case ASTTypeMPDecimal: {
        ASTMPDecimalNode *MPD =
            ASTE->GetValue()->GetValue<ASTMPDecimalNode *>();
        assert(
            MPD &&
            "Could not obtain a valid ASTMPDecimalNode from the SymbolTable!");

        AN = new ASTAngleNode(&ASTIdentifierNode::Angle, MPD,
                              ASTAngleTypeGeneric, MPD->GetBits());
        break;
      } break;
      case ASTTypeAngle: {
        AN = ASTE->GetValue()->GetValue<ASTAngleNode *>();
        assert(AN &&
               "Could not obtain a valid ASTAngleNode from the SymbolTable!");
        break;
      } break;
      default: {
        std::stringstream M;
        M << "Impossible initialization of an ASTAngleNode from Type "
          << PrintTypeEnum(E->GetASTType()) << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(E), M.str(),
            DiagLevel::Error);
        return nullptr;
        break;
      } break;
      }
    }
  } break;
  case ASTTypeIdentifierRef: {
    ASTIdentifierRefNode *AIdR = dynamic_cast<ASTIdentifierRefNode *>(E);
    if (!AIdR) {
      std::stringstream M;
      M << "Invalid second element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    unsigned IX = AIdR->GetIndex();
    if (IX == static_cast<unsigned>(~0x0)) {
      std::stringstream M;
      M << "Invalid Index for ASTIdentifierRefNode indexed reference.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    const ASTIdentifierNode *AId = AIdR->GetIdentifier();
    assert(AId && "Could not obtain a valid ASTIdentifierNode "
                  "from the ASTIdentifierRefNode!");

    std::vector<ASTSymbolTableEntry *> STV =
        ASTSymbolTable::Instance().LookupRange(AId);
    std::vector<std::vector<ASTSymbolTableEntry *>::iterator> EV;

    for (std::vector<ASTSymbolTableEntry *>::iterator SI = STV.begin();
         SI != STV.end(); ++SI) {
      if ((E)->GetIdentifier()->GetBits() != AId->GetBits())
        EV.push_back(SI);
    }

    for (std::vector<std::vector<ASTSymbolTableEntry *>::iterator>::iterator
             EI = EV.begin();
         EI != EV.end(); ++EI)
      STV.erase(*EI);

    for (std::vector<ASTSymbolTableEntry *>::iterator SI = STV.begin();
         SI != STV.end(); ++SI) {
      ASTSymbolTableEntry *ASTE = *SI;
      assert(ASTE && "Invalid ASTSymbolTableEntry in IdentifierRange!");

      switch (ASTE->GetValueType()) {
      case ASTTypeFloatArray: {
        ASTFloatArrayNode *FAN =
            ASTE->GetValue()->GetValue<ASTFloatArrayNode *>();
        assert(FAN && "Could not obtain a valid ASTFloatArrayNode "
                      "from the SymbolTable!");
        ASTFloatNode *FN = FAN->GetElement(IX);
        assert(FN && "Could not obtain a valid ASTFloatNode "
                     "from the ASTFloat array!");
        AN = new ASTAngleNode(&ASTIdentifierNode::Angle, FN,
                              ASTAngleTypeGeneric, 32);
        break;
      } break;
      case ASTTypeMPDecimalArray: {
        ASTMPDecimalArrayNode *MPAN =
            ASTE->GetValue()->GetValue<ASTMPDecimalArrayNode *>();
        assert(MPAN && "Could not obtain a valid ASTMPDecimalArrayNode "
                       "from the SymbolTable!");
        ASTMPDecimalNode *MPD = MPAN->GetElement(IX);
        assert(MPD && "Could not obtain a valid ASTMPDecimalNode "
                      "from the ASTMPDecimal array!");
        AN = new ASTAngleNode(&ASTIdentifierNode::Angle, MPD,
                              ASTAngleTypeGeneric, MPD->GetBits());
        break;
      } break;
      case ASTTypeAngleArray: {
        ASTAngleArrayNode *AAN =
            ASTE->GetValue()->GetValue<ASTAngleArrayNode *>();
        assert(AAN && "Could not obtain a valid ASTAngleArrayNode "
                      "from the SymbolTable!");
        AN = AAN->GetElement(IX);
        assert(AN && "Could not obtain a valid ASTAngleNode "
                     "from the ASTAngle array!");
      } break;
      default: {
        std::stringstream M;
        M << "Impossible initialization of an ASTAngleNode from Type "
          << PrintTypeEnum(E->GetASTType()) << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(E), M.str(),
            DiagLevel::Error);
        return nullptr;
        break;
      } break;
      }
    }
  } break;
  case ASTTypeBinaryOp: {
    ASTBinaryOpNode *BOP = dynamic_cast<ASTBinaryOpNode *>(E);
    if (!BOP) {
      std::stringstream M;
      M << "Invalid second element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
    }

    AN = new ASTAngleNode(&ASTIdentifierNode::Angle, BOP, ASTAngleTypeGeneric,
                          32);
  } break;
  case ASTTypeUnaryOp: {
    ASTUnaryOpNode *UOP = dynamic_cast<ASTUnaryOpNode *>(E);
    if (!UOP) {
      std::stringstream M;
      M << "Invalid second element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    AN = new ASTAngleNode(&ASTIdentifierNode::Angle, UOP, ASTAngleTypeGeneric,
                          32);
  } break;
  default: {
    std::stringstream M;
    M << "Impossible instantiation of an ASTAngleNode from Type "
      << PrintTypeEnum(E->GetASTType()) << ".";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(E), M.str(), DiagLevel::Error);
    return nullptr;
  } break;
  }

  if (!AN) {
    std::stringstream M;
    M << "Could not instantiate a valid ASTAngleNode as second "
      << "frame parameter.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(E), M.str(), DiagLevel::Error);
    return nullptr;
  }

  return AN;
}

ASTMPDecimalNode *
ASTOpenPulseFrameNodeResolver::ResolveFrequency(ASTExpressionNode *E) {
  assert(E && "Invalid ASTExpression argument!");

  ASTMPDecimalNode *MPD = nullptr;

  switch (E->GetASTType()) {
  case ASTTypeInt: {
    ASTIntNode *I = dynamic_cast<ASTIntNode *>(E);
    if (!I) {
      std::stringstream M;
      M << "Invalid third element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 64,
                               static_cast<double>(I->GetSignedValue()));
  } break;
  case ASTTypeFloat: {
    ASTFloatNode *F = dynamic_cast<ASTFloatNode *>(E);
    if (!F) {
      std::stringstream M;
      M << "Invalid third element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 32, F->GetValue());
  } break;
  case ASTTypeDouble: {
    ASTDoubleNode *D = dynamic_cast<ASTDoubleNode *>(E);
    if (!D) {
      std::stringstream M;
      M << "Invalid third element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 64, D->GetValue());
    break;
  }
  case ASTTypeMPInteger: {
    ASTMPIntegerNode *MPI = dynamic_cast<ASTMPIntegerNode *>(E);
    if (MPI) {
      std::stringstream M;
      M << "Invalid third element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    unsigned Bits = MPI->GetBits();
    Bits += Bits / 4;
    MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, Bits, MPI);
  } break;
  case ASTTypeMPDecimal: {
    MPD = dynamic_cast<ASTMPDecimalNode *>(E);
    if (!MPD) {
      std::stringstream M;
      M << "Invalid third element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }
  } break;
  case ASTTypeIdentifier: {
    ASTIdentifierNode *DId =
        const_cast<ASTIdentifierNode *>(E->GetIdentifier());
    if (!DId) {
      std::stringstream M;
      M << "Invalid second element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    std::vector<ASTSymbolTableEntry *> STV =
        ASTSymbolTable::Instance().LookupRange(DId);
    std::vector<std::vector<ASTSymbolTableEntry *>::iterator> EV;

    for (std::vector<ASTSymbolTableEntry *>::iterator SI = STV.begin();
         SI != STV.end(); ++SI) {
      if ((E)->GetIdentifier()->GetBits() != DId->GetBits())
        EV.push_back(SI);
    }

    for (std::vector<std::vector<ASTSymbolTableEntry *>::iterator>::iterator
             EI = EV.begin();
         EI != EV.end(); ++EI)
      STV.erase(*EI);

    for (std::vector<ASTSymbolTableEntry *>::iterator SI = STV.begin();
         SI != STV.end(); ++SI) {
      ASTSymbolTableEntry *DSTE = *SI;
      assert(DSTE && "Invalid ASTSymbolTableEntry in IdentifierRange!");

      switch (DSTE->GetValueType()) {
      case ASTTypeInt: {
        ASTIntNode *I = DSTE->GetValue()->GetValue<ASTIntNode *>();
        assert(I &&
               "Could not obtain a valid ASTIntNode from the SymbolTable!");
        MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 32,
                                   static_cast<double>(I->GetSignedValue()));
      } break;
      case ASTTypeFloat: {
        ASTFloatNode *F = DSTE->GetValue()->GetValue<ASTFloatNode *>();
        assert(F &&
               "Could not obtain a valid ASTFloatNode from the SymbolTable!");
        MPD =
            new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 32, F->GetValue());
      } break;
      case ASTTypeDouble: {
        ASTDoubleNode *D = DSTE->GetValue()->GetValue<ASTDoubleNode *>();
        assert(D &&
               "Could not obtain a valid ASTDoubleNode from the SymbolTable!");
        MPD =
            new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 64, D->GetValue());
      } break;
      case ASTTypeMPInteger: {
        ASTMPIntegerNode *MPI =
            DSTE->GetValue()->GetValue<ASTMPIntegerNode *>();
        assert(MPI && "Could not obtain a valid ASTMPIntegerNode "
                      "from the SymbolTable!");
        unsigned Bits = MPI->GetBits();
        Bits += Bits / 4;
        MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, Bits, MPI);
      } break;
      case ASTTypeMPDecimal: {
        MPD = DSTE->GetValue()->GetValue<ASTMPDecimalNode *>();
        if (!MPD) {
          std::stringstream M;
          M << "Could not obtain a valid ASTMPDecimalNode from "
            << "the SymbolTable.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(E), M.str(),
              DiagLevel::Error);
          return nullptr;
        }
      } break;
      case ASTTypeIdentifier: {
        // FIXME: IMPLEMENT.
      } break;
      case ASTTypeIdentifierRef: {
        // FIXME: IMPLEMENT.
      } break;
      default: {
        std::stringstream M;
        M << "Impossible initialization of an ASTMPDecimalNode from Type "
          << PrintTypeEnum(E->GetASTType()) << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(E), M.str(),
            DiagLevel::Error);
        return nullptr;
      } break;
      }
    }
  } break;
  case ASTTypeIdentifierRef: {
    const ASTIdentifierRefNode *DIdR =
        dynamic_cast<const ASTIdentifierRefNode *>(E->GetIdentifier());
    if (!DIdR) {
      std::stringstream M;
      M << "Invalid third element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    unsigned IX = DIdR->GetIndex();
    if (IX == static_cast<unsigned>(~0x0)) {
      std::stringstream M;
      M << "Invalid Index for ASTIdentifierRefNode indexed reference.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    const ASTIdentifierNode *DId = DIdR->GetIdentifier();
    assert(DId && "Could not obtain a valid ASTIdentifierNode "
                  "from the ASTIdentifierRefNode!");

    std::vector<ASTSymbolTableEntry *> STV =
        ASTSymbolTable::Instance().LookupRange(DId);
    std::vector<std::vector<ASTSymbolTableEntry *>::iterator> EV;

    for (std::vector<ASTSymbolTableEntry *>::iterator SI = STV.begin();
         SI != STV.end(); ++SI) {
      if ((E)->GetIdentifier()->GetBits() != DId->GetBits())
        EV.push_back(SI);
    }

    for (std::vector<std::vector<ASTSymbolTableEntry *>::iterator>::iterator
             EI = EV.begin();
         EI != EV.end(); ++EI)
      STV.erase(*EI);

    for (std::vector<ASTSymbolTableEntry *>::iterator SI = STV.begin();
         SI != STV.end(); ++SI) {
      ASTSymbolTableEntry *DSTE = *SI;
      assert(DSTE && "Invalid ASTSymbolTableEntry in IdentifierRange!");

      switch (DSTE->GetValueType()) {
      case ASTTypeFloatArray: {
        ASTFloatArrayNode *FAN =
            DSTE->GetValue()->GetValue<ASTFloatArrayNode *>();
        assert(FAN && "Could not obtain a valid ASTFloatArrayNode "
                      "from the SymbolTable!");
        ASTFloatNode *FN = FAN->GetElement(IX);
        assert(FN && "Could not obtain a valid ASTFloatNode "
                     "from the ASTFloat array!");
        MPD =
            new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 32, FN->GetValue());
      } break;
      case ASTTypeMPDecimalArray: {
        ASTMPDecimalArrayNode *MPDA =
            DSTE->GetValue()->GetValue<ASTMPDecimalArrayNode *>();
        assert(MPDA && "Could not obtain a valid ASTMPDecimalArrayNode "
                       "from the SymbolTable!");
        ASTMPDecimalNode *MPDD = MPDA->GetElement(IX);
        assert(MPDD && "Could not obtain a valid ASTMPDecimalNode "
                       "from the ASTMPDecimal array!");
        MPD = MPDD;
      } break;
      case ASTTypeMPIntegerArray: {
        ASTMPIntegerArrayNode *MPIA =
            DSTE->GetValue()->GetValue<ASTMPIntegerArrayNode *>();
        assert(MPIA && "Could not obtain a valid ASTMPIntegerArrayNode "
                       "from the SymbolTable!");
        ASTMPIntegerNode *MPI = MPIA->GetElement(IX);
        assert(MPI && "Could not obtain a valid ASTMPIntegerNode "
                      "from the ASTMPInteger array!");
        unsigned Bits = MPI->GetBits();
        Bits += Bits / 4;
        MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, Bits, MPI);
      } break;
      case ASTTypeIdentifier: {
        // FIXME: IMPLEMENT.
      } break;
      case ASTTypeIdentifierRef: {
        // FIXME: IMPLEMENT.
      } break;
      default: {
        std::stringstream M;
        M << "Impossible initialization of an ASTMPDecimalNode from Type "
          << PrintTypeEnum(E->GetASTType()) << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(E), M.str(),
            DiagLevel::Error);
        return nullptr;
      } break;
      }
    }
  } break;
  case ASTTypeBinaryOp: {
    ASTBinaryOpNode *BOP = dynamic_cast<ASTBinaryOpNode *>(E);
    if (!BOP) {
      std::stringstream M;
      M << "Invalid third element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, BOP);
  } break;
  case ASTTypeUnaryOp: {
    ASTBinaryOpNode *UOP = dynamic_cast<ASTBinaryOpNode *>(E);
    if (!UOP) {
      std::stringstream M;
      M << "Invalid third element for the frame initialization "
        << "ExpressionList.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(E), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, UOP);
  } break;
  default: {
    std::stringstream M;
    M << "Impossible instantiation of an ASTAngleNode from Type "
      << PrintTypeEnum(E->GetASTType()) << ".";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(E), M.str(), DiagLevel::Error);
    return nullptr;
  } break;
  }

  if (!MPD) {
    std::stringstream M;
    M << "Could not instantiate a valid ASTMPDecimalNode as third "
      << "frame parameter.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(E), M.str(), DiagLevel::Error);
    return nullptr;
  }

  return MPD;
}

ASTMPDecimalNode *
ASTOpenPulseFrameNodeResolver::ResolveFrequency(const ASTIdentifierNode *Id) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  const ASTSymbolTableEntry *STE = Id->GetSymbolTableEntry();

  if (!STE) {
    STE = ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                            Id->GetValueType());
    if (!STE) {
      std::stringstream M;
      M << "Identifier " << Id->GetName() << " does not reference a known "
        << "symbol.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return nullptr;
    }
  }

  ASTMPDecimalNode *MPD = nullptr;

  switch (STE->GetValueType()) {
  case ASTTypeInt: {
    ASTIntNode *I = STE->GetValue()->GetValue<ASTIntNode *>();
    if (I) {
      MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec,
                                 ASTMPDecimalNode::DefaultBits,
                                 static_cast<double>(I->GetSignedValue()));
      assert(MPD && "Could not create a valid ASTMPDecimalNode!");
    }
  } break;
  case ASTTypeFloat: {
    ASTFloatNode *F = STE->GetValue()->GetValue<ASTFloatNode *>();
    if (F) {
      MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec,
                                 ASTMPDecimalNode::DefaultBits, F->GetValue());
      assert(MPD && "Could not create a valid ASTMPDecimalNode!");
    }
  } break;
  case ASTTypeDouble: {
    ASTDoubleNode *D = STE->GetValue()->GetValue<ASTDoubleNode *>();
    if (D) {
      MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec,
                                 ASTMPDecimalNode::DefaultBits, D->GetValue());
      assert(MPD && "Could not create a valid ASTMPDecimalNode!");
    }
  } break;
  case ASTTypeMPInteger: {
    ASTMPIntegerNode *MPI = STE->GetValue()->GetValue<ASTMPIntegerNode *>();
    if (MPI) {
      MPD =
          new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, MPI->GetBits(), MPI);
      assert(MPD && "Could not create a valid ASTMPDecimalNode!");
    }
  } break;
  case ASTTypeMPDecimal: {
    ASTMPDecimalNode *MPDD = STE->GetValue()->GetValue<ASTMPDecimalNode *>();
    if (MPDD) {
      MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, MPDD->GetBits(),
                                 MPDD->GetMPValue());
      assert(MPD && "Could not create a valid ASTMPDecimalNode!");
    }
  } break;
  case ASTTypeBinaryOp: {
    ASTBinaryOpNode *BOP = STE->GetValue()->GetValue<ASTBinaryOpNode *>();
    if (BOP) {
      MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec,
                                 ASTMPDecimalNode::DefaultBits, BOP);
      assert(MPD && "Could not create a valid ASTMPDecimalNode!");
    }
  } break;
  case ASTTypeUnaryOp: {
    ASTUnaryOpNode *UOP = STE->GetValue()->GetValue<ASTUnaryOpNode *>();
    if (UOP) {
      MPD = new ASTMPDecimalNode(&ASTIdentifierNode::MPDec,
                                 ASTMPDecimalNode::DefaultBits, UOP);
      assert(MPD && "Could not create a valid ASTMPDecimalNode!");
    }
  } break;
  default: {
    std::stringstream M;
    M << "Impossible ASTMPDecimal initialization from Type "
      << PrintTypeEnum(STE->GetValueType()) << ".";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  } break;
  }

  return MPD;
}

ASTMPDecimalNode *ASTOpenPulseFrameNodeResolver::ResolveFrequency(
    const ASTIdentifierRefNode *IdR) {
  assert(IdR && "Invalid ASTIdentifierRefNode argument!");

  const ASTSymbolTableEntry *STE = IdR->GetSymbolTableEntry();
  if (!STE) {
  }

  // FIXME: IMPLEMENT
  return nullptr;
}

ASTDurationNode *
ASTOpenPulseFrameNodeResolver::ResolveDuration(ASTExpressionNode *E) {
  assert(E && "Invalid ASTExpression argument!");

  ASTDurationNode *DN = dynamic_cast<ASTDurationNode *>(E);
  if (!DN) {
    std::stringstream M;
    M << "Expression of Type " << PrintTypeEnum(E->GetASTType())
      << " does not instantiate a Duration.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(E), M.str(), DiagLevel::Error);
    return nullptr;
  }

  return DN;
}

ASTDurationNode *
ASTOpenPulseFrameNodeResolver::ResolveDuration(ASTExpressionNode *E0,
                                               ASTExpressionNode *E1) {
  assert(E0 && "Invalid first ASTExpression argument!");
  assert(E1 && "Invalid second ASTExpression argument!");

  ASTIdentifierNode *I0 = dynamic_cast<ASTIdentifierNode *>(E0);
  ASTIdentifierNode *I1 = dynamic_cast<ASTIdentifierNode *>(E1);

  if (!I0) {
    std::stringstream M;
    M << "First expression of duration instantiation is invalid.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(E0), M.str(), DiagLevel::Error);
    return nullptr;
  }

  bool CE = false;
  uint64_t XV;

  try {
    XV = std::stoul(I0->GetName());
  } catch (const std::invalid_argument &E) {
    (void)E;
    (void)XV;
    CE = true;
  } catch (const std::out_of_range &E) {
    (void)E;
    (void)XV;
    CE = true;
  } catch (...) {
    (void)XV;
    CE = true;
  }

  if (CE) {
    std::stringstream M;
    M << "First expression of duration instantiation is not a valid "
      << "numeric string literal.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(E0), M.str(), DiagLevel::Error);
    return nullptr;
  }

  if (!I1) {
    std::stringstream M;
    M << "Second expression of duration instantiation is invalid.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(E1), M.str(), DiagLevel::Error);
    return nullptr;
  }

  std::string DS = I0->GetName();
  DS += I1->GetName();

  ASTSymbolTable::Instance().Erase(I0);
  ASTSymbolTable::Instance().Erase(I1);

  ASTDurationNode *DN = new ASTDurationNode(&ASTIdentifierNode::Duration, DS);
  assert(DN && "Could not create a valid ASTDurationNode!");

  return DN;
}

void ASTOpenPulseFrameNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (EXT)
    M.Type(ASTTypeExtern);

  M.TypeIdentifier(GetASTType(), GetName());

  if (PRT) {
    const_cast<ASTOpenPulsePortNode *>(PRT)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(PRT->GetMangledName()));
  }

  if (FRQ) {
    const_cast<ASTMPDecimalNode *>(FRQ)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(FRQ->GetMangledName()));
  }

  if (PHS) {
    const_cast<ASTAngleNode *>(PHS)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(PHS->GetMangledName()));
    M.EndExpression();
  }

  if (DUR) {
    const_cast<ASTDurationNode *>(DUR)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(DUR->GetMangledName()));
  }

  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTOpenPulsePortNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  if (EXT)
    M.Type(ASTTypeExtern);
  M.Underscore();

  std::stringstream PS;
  PS << 'L' << PID;
  M.TypeIdentifier(ASTTypeUInt, CHAR_BIT * sizeof(uint64_t), PS.str());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace OpenPulse
} // namespace QASM
