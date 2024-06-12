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

#include <qasm/AST/ASTArray.h>
#include <qasm/AST/ASTMathUtils.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTUtils.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <sstream>
#include <string>

namespace QASM {

ASTUtils ASTUtils::Utils;
ASTMathUtils ASTMathUtils::MU;

using DiagLevel = QasmDiagnosticEmitter::DiagLevel;

unsigned ASTUtils::GetUnsignedValue(const ASTIdentifierNode *Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry *STE =
      ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), Id->GetSymbolType());
  if (!STE) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " has no SymbolTable Entry!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return static_cast<unsigned>(~0x0);
  }

  if (!STE->HasValue()) {
    std::stringstream M;
    M << "SymbolTable Entry for Identifier " << Id->GetName()
      << " has no Value!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return static_cast<unsigned>(~0x0);
  }

  if (Id->IsReference()) {
    const ASTIdentifierRefNode *IdR =
        dynamic_cast<const ASTIdentifierRefNode *>(Id);
    assert(IdR && "Could not dynamic_cast to an ASTIdentifierRefNode!");

    const ASTIdentifierNode *RId = IdR->GetIdentifier();
    assert(RId && "Could not obtain a valid ASTIdentifierNode!");

    switch (RId->GetSymbolType()) {
    case ASTTypeBoolArray: {
      ASTBoolArrayNode *BAN = dynamic_cast<ASTBoolArrayNode *>(
          STE->GetValue()->GetValue<ASTArrayNode *>());
      assert(BAN && "Could not dynamic_cast to an ASTBoolArrayNode!");

      auto Index = IdR->GetIndex();
      BAN->ValidateIndex(Index, IdR->GetLocation());
      ASTBoolNode *B = BAN->GetElement(Index);
      assert(B && "Could not obtain a valid SymbolTable Entry Value!");

      return static_cast<unsigned>(B->GetValue());
    } break;
    case ASTTypeIntArray: {
      ASTIntArrayNode *IAN = dynamic_cast<ASTIntArrayNode *>(
          STE->GetValue()->GetValue<ASTArrayNode *>());
      assert(IAN && "Could not dynamic_cast to an ASTIntArrayNode!");

      auto Index = IdR->GetIndex();
      IAN->ValidateIndex(Index, IdR->GetLocation());
      ASTIntNode *I = IAN->GetElement(Index);
      assert(I && "Could not obtain a valid SymbolTable Entry Value!");

      return I->IsSigned() ? static_cast<unsigned>(I->GetSignedValue())
                           : I->GetUnsignedValue();
    } break;
    case ASTTypeFloatArray: {
      ASTFloatArrayNode *FAN = dynamic_cast<ASTFloatArrayNode *>(
          STE->GetValue()->GetValue<ASTArrayNode *>());
      assert(FAN && "Could not dynamic_cast to an ASTFloatArrayNode!");

      auto Index = IdR->GetIndex();
      FAN->ValidateIndex(Index, IdR->GetLocation());
      ASTFloatNode *F = FAN->GetElement(Index);
      assert(F && "Could not obtain a valid SymbolTable Entry Value!");

      return static_cast<unsigned>(F->GetValue());
    } break;
    case ASTTypeMPIntegerArray: {
      ASTMPIntegerArrayNode *MIAN = dynamic_cast<ASTMPIntegerArrayNode *>(
          STE->GetValue()->GetValue<ASTArrayNode *>());
      assert(MIAN && "Could not dynamic_cast to an ASTMPIntegerArrayNode!");

      auto Index = IdR->GetIndex();
      MIAN->ValidateIndex(Index, IdR->GetLocation());
      ASTMPIntegerNode *MPI = MIAN->GetElement(Index);
      assert(MPI && "Could not obtain a valid SymbolTable Entry Value!");

      return MPI->ToUnsignedInt();
    } break;
    case ASTTypeMPDecimalArray: {
      ASTMPDecimalArrayNode *MDAN = dynamic_cast<ASTMPDecimalArrayNode *>(
          STE->GetValue()->GetValue<ASTArrayNode *>());
      assert(MDAN && "Could not dynamic_cast to an ASTMPDecimalArrayNode!");

      auto Index = IdR->GetIndex();
      MDAN->ValidateIndex(Index, IdR->GetLocation());
      ASTMPDecimalNode *MPD = MDAN->GetElement(Index);
      assert(MPD && "Could not obtain a valid SymbolTable Entry Value!");

      return static_cast<unsigned>(MPD->ToDouble());
    } break;
    default: {
      std::stringstream M;
      M << "Indexed Identifier " << Id->GetName() << " is not an Integer "
        << "Constant Expression.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return static_cast<unsigned>(~0x0);
    } break;
    }
  }

  if (STE->GetValueType() != ASTTypeInt &&
      STE->GetValueType() != ASTTypeMPInteger) {
    std::stringstream M;
    M << "Index Identifier " << Id->GetName() << " is not an Integer "
      << "Constant Expression.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return static_cast<unsigned>(~0x0);
  }

  unsigned Bits = static_cast<unsigned>(~0x0);

  if (STE->GetValueType() == ASTTypeInt) {
    ASTIntNode *INT = nullptr;
    try {
      INT = STE->GetValue()->GetValue<ASTIntNode *>();
    } catch (const std::bad_any_cast &E) {
      INT = nullptr;
      std::stringstream M;
      M << "Impossible any_cast to an Index ASTIntNode!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return static_cast<unsigned>(~0x0);
    }

    Bits = INT->IsSigned() ? static_cast<unsigned>(INT->GetSignedValue())
                           : INT->GetUnsignedValue();
  } else if (STE->GetValueType() == ASTTypeMPInteger) {
    ASTMPIntegerNode *MPI = nullptr;
    try {
      MPI = STE->GetValue()->GetValue<ASTMPIntegerNode *>();
    } catch (const std::bad_any_cast &E) {
      MPI = nullptr;
      std::stringstream M;
      M << "Impossible any_cast to an Index ASTMPIntegerNode!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return static_cast<unsigned>(~0x0);
    }

    if (MPI->IsSigned())
      Bits = static_cast<unsigned>(std::stol(MPI->GetValue()));
    else
      Bits = static_cast<unsigned>(std::stoul(MPI->GetValue()));
  }

  return Bits;
}

unsigned ASTUtils::GetUnsignedValue(const ASTIntNode *Int) const {
  assert(Int && "Invalid ASTIntNode argument!");

  unsigned Bits;

  if (Int->IsSigned())
    Bits = static_cast<unsigned>(Int->GetSignedValue());
  else
    Bits = Int->GetUnsignedValue();

  return Bits;
}

bool ASTUtils::GetBooleanValue(const ASTIntNode *Int) const {
  assert(Int && "Invalid ASTIntNode argument!");

  if (Int->IsSigned())
    return Int->GetSignedValue() != 0;

  return Int->GetUnsignedValue() != 0;
}

bool ASTUtils::GetBooleanValue(const ASTMPIntegerNode *MPI) const {
  assert(MPI && "Invalid ASTMPIntegerNode argument!");

  return !MPI->IsZero();
}

bool ASTUtils::GetVariantIntegerValue(
    const ASTIdentifierNode *IId,
    std::variant<const ASTIntNode *, const ASTMPIntegerNode *,
                 const ASTCBitNode *> &IIV) const {
  assert(IId && "Invalid ASTIdentifierNode argument!");

  switch (IId->GetSymbolType()) {
  case ASTTypeInt:
  case ASTTypeUInt: {
    if (const ASTSymbolTableEntry *STE = IId->GetSymbolTableEntry()) {
      assert(STE->GetValueType() == IId->GetSymbolType() &&
             "Type Mismatch ASTIdentifierNode <-> ASTSymbolTableEntry!");
      if (STE->HasValue()) {
        ASTIntNode *II = STE->GetValue()->GetValue<ASTIntNode *>();
        assert(II && "Could not obtain a valid ASTIntNode!");

        if (II->IsMPInteger())
          IIV = II->GetMPInteger();
        else
          IIV = II;
      } else {
        ASTIntNode *II = new ASTIntNode(IId, IId->GetSymbolType() == ASTTypeInt
                                                 ? int32_t(0)
                                                 : uint32_t(0U));
        assert(II && "Could not create a valid ASTIntNode!");
        const_cast<ASTSymbolTableEntry *>(STE)->SetValue(
            new ASTValue<>(II, ASTTypeInt), ASTTypeInt);
        IIV = II;
      }

      return true;
    }
  } break;
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger: {
    if (const ASTSymbolTableEntry *STE = IId->GetSymbolTableEntry()) {
      assert(STE->GetValueType() == IId->GetSymbolType() &&
             "Type Mismatch ASTIdentifierNode <-> ASTSymbolTableEntry!");
      if (STE->HasValue()) {
        ASTMPIntegerNode *MPI = STE->GetValue()->GetValue<ASTMPIntegerNode *>();
        assert(MPI && "Could not obtain a valid ASTMPIntegerNode!");

        IIV = MPI;
      } else {
        ASTSignbit SB = IId->GetSymbolType() == ASTTypeMPInteger
                            ? ASTSignbit::Signed
                            : ASTSignbit::Unsigned;
        ASTMPIntegerNode *MPI = new ASTMPIntegerNode(IId, SB, IId->GetBits());
        assert(MPI && "Could not create a valid ASTMPIntegerNode!");
        const_cast<ASTSymbolTableEntry *>(STE)->SetValue(
            new ASTValue<>(MPI, ASTTypeMPInteger), ASTTypeMPInteger);
        IIV = MPI;
      }

      return true;
    }
  } break;
  case ASTTypeBitset: {
    if (const ASTSymbolTableEntry *STE = IId->GetSymbolTableEntry()) {
      assert(STE->GetValueType() == IId->GetSymbolType() &&
             "Type Mismatch ASTIdentifierNode <-> ASTSymbolTableEntry!");
      if (STE->HasValue()) {
        ASTCBitNode *CBN = STE->GetValue()->GetValue<ASTCBitNode *>();
        assert(CBN && "Could not obtain a valid ASTCBitNode!");

        IIV = CBN;
      } else {
        ASTCBitNode *CBN = new ASTCBitNode(IId, IId->GetBits());
        assert(CBN && "Could not create a valid ASTCBitNode!");
        const_cast<ASTSymbolTableEntry *>(STE)->SetValue(
            new ASTValue<>(CBN, ASTTypeBitset), ASTTypeBitset);
        IIV = CBN;
      }

      return true;
    }
  } break;
  default:
    break;
  }

  return false;
}

unsigned ASTUtils::AdjustZeroBitWidth(const ASTIdentifierNode *Id,
                                      unsigned NumBits) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->GetBits() == 0)
    Id->SetBits(NumBits);

  return Id->GetBits();
}

} // namespace QASM
