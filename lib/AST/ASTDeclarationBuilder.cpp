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

#include <qasm/AST/ASTDeclarationBuilder.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTDeclarationList.h>
#include <qasm/AST/ASTRedeclarationController.h>
#include <qasm/AST/ASTReturn.h>
#include <qasm/AST/ASTScopeController.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

namespace QASM {

ASTDeclarationList ASTDeclarationBuilder::DL;
ASTDeclarationMap ASTDeclarationBuilder::DM;

std::map<const ASTIdentifierNode *, const ASTDeclarationNode *>
    ASTDeclarationBuilder::CDM;

ASTDeclarationBuilder ASTDeclarationBuilder::DB;
ASTRedeclarationController ASTRedeclarationController::RDC;
bool ASTRedeclarationController::ARD = false;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

void ASTDeclarationList::SetLocalScope() {
  for (ASTDeclarationList::iterator I = List.begin(); I != List.end(); ++I) {
    if (const ASTIdentifierNode *Id = (*I)->GetIdentifier()) {
      const_cast<ASTIdentifierNode *>(Id)->SetLocalScope();
      if (ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().Lookup(
              Id, Id->GetBits(), Id->GetSymbolType()))
        STE->SetLocalScope();
    }
  }
}

void ASTDeclarationList::SetDeclarationContext(
    const ASTDeclarationContext *CX) {
  for (ASTDeclarationList::iterator I = List.begin(); I != List.end(); ++I) {
    if (const ASTIdentifierNode *Id = (*I)->GetIdentifier()) {
      if (!ASTDeclarationContextTracker::Instance().IsGlobalContext(
              Id->GetDeclarationContext())) {
        const_cast<ASTIdentifierNode *>(Id)->SetDeclarationContext(CX);
        if (ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().Lookup(
                Id, Id->GetBits(), Id->GetSymbolType()))
          STE->SetLocalScope(CX);
      }
    }
  }
}

bool ASTDeclarationList::ExportQuantumParameters(ASTDeclarationList &QL) {
  if (List.empty())
    return true;

  uint64_t PO = 0UL;
  ASTDeclarationList PL;
  QL.Clear();

  for (ASTDeclarationList::iterator I = List.begin(); I != List.end(); ++I) {
    ASTDeclarationNode *DN = *I;
    assert(DN && "Invalid ASTDeclarationNode in ASTDeclarationList!");

    DN->SetParameterOrder(PO++);
    PL.Append(DN);

    if (const ASTExpressionNode *EXP =
            dynamic_cast<const ASTExpressionNode *>(DN->GetExpression())) {
      if (EXP->GetASTType() == ASTTypeQubit ||
          EXP->GetASTType() == ASTTypeQubitContainer) {
        QL.Append(DN);
      }
    }
  }

  List.clear();
  List = PL.List;

  return true;
}

void ASTDeclarationList::CreateBuiltinParameterSymbols(
    const ASTIdentifierNode *FId) {
  assert(FId && "Invalid ASTIdentifierNode argument!");

  if (ASTTypeSystemBuilder::Instance().IsBuiltinFunction(FId->GetName()) &&
      !List.empty()) {
    for (std::vector<ASTDeclarationNode *>::const_iterator DI = List.begin();
         DI != List.end(); ++DI) {
      const ASTIdentifierNode *DId = (*DI)->GetIdentifier();
      assert(DId && "Invalid ASTIIdentifierNode for ASTDeclarationNode!");

      const_cast<ASTIdentifierNode *>(DId)->SetLocalScope();
      ASTSymbolTableEntry *STE =
          const_cast<ASTSymbolTableEntry *>(DId->GetSymbolTableEntry());
      if (!STE) {
        STE = new ASTSymbolTableEntry(DId, DId->GetSymbolType());
        assert(STE && "Could not create a valid ASTSymbolTableEntry!");

        STE->SetLocalScope();
        STE->SetContext(DId->GetDeclarationContext());
        const_cast<ASTIdentifierNode *>(DId)->SetSymbolTableEntry(STE);
        assert(DId->HasSymbolTableEntry() &&
               "ASTIdentifierNode has no SymbolTable Entry!");
      } else {
        assert(DId->HasSymbolTableEntry() &&
               "ASTIdentifierNode has no SymbolTable Entry!");
        STE->SetLocalScope();
        STE->SetContext(DId->GetDeclarationContext());
      }
    }
  }
}

bool ASTDeclarationList::TransferSymbols(
    std::map<std::string, const ASTSymbolTableEntry *> &MM) const {
  if (List.empty())
    return true;

  for (std::vector<ASTDeclarationNode *>::const_iterator DI = List.begin();
       DI != List.end(); ++DI) {
    const ASTIdentifierNode *DId = (*DI)->GetIdentifier();
    assert(DId && "Invalid ASTIIdentifierNode for ASTDeclarationNode!");

    if (!ASTScopeController::Instance().CanHaveLocalScope(
            (*DI)->GetASTType())) {
      std::stringstream M;
      M << "A Declaration of Type " << PrintTypeEnum((*DI)->GetASTType())
        << " cannot have Local Scope.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(DId), M.str(),
          DiagLevel::Warning);
      continue;
    }

    const_cast<ASTIdentifierNode *>(DId)->SetLocalScope();
    const_cast<ASTSymbolTableEntry *>(DId->GetSymbolTableEntry())
        ->SetLocalScope();

    if (!ASTDeclarationBuilder::Instance().TransferSymbol(DId, MM))
      return false;
  }

  return true;
}

bool ASTDeclarationBuilder::TransferSymbol(
    const ASTIdentifierNode *Id,
    std::map<std::string, const ASTSymbolTableEntry *> &MM) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  const_cast<ASTIdentifierNode *>(Id)->SetLocalScope();
  if (Id->GetSymbolTableEntry())
    const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry())
        ->SetLocalScope();

  std::map<std::string, const ASTSymbolTableEntry *>::iterator MI =
      MM.find(Id->GetName());

  if (MI != MM.end()) {
    if (!(*MI).second) {
      // No SymbolTable Entry. Erase the entry, it will be re-inserted below.
      MM.erase(Id->GetName());
    } else if ((*MI).second == Id->GetSymbolTableEntry() &&
               (*MI).second->GetIdentifier() == Id &&
               (*MI).second->GetIdentifier()->GetSymbolType() ==
                   Id->GetSymbolType()) {
      goto Found;
    } else {
      std::stringstream M;
      M << "A symbol with Identifier " << (*MI).first << " already exists "
        << " in the declaration's symbol table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }
  }

  if (!Id->GetSymbolTableEntry()) {
    std::stringstream M;
    M << "Symbol " << Id->GetName() << " of Type "
      << PrintTypeEnum(Id->GetSymbolType()) << " does not have "
      << "a SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
        DiagLevel::Warning);
  }

  if (!MM.insert(std::make_pair(Id->GetName(), Id->GetSymbolTableEntry()))
           .second) {
    std::stringstream M;
    M << "Failure transferring symbol " << Id->GetName() << " to the "
      << "local Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    return false;
  } else if (!Id->GetSymbolTableEntry()) {
    ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(),
                                                Id->GetSymbolType());
  }

Found:
  switch (Id->GetSymbolType()) {
  case ASTTypeAngle: {
    std::stringstream DSS;
    std::string DS;
    ASTSymbolTableEntry *STE;

    for (unsigned I = 0; I < 3; ++I) {
      DSS.str("");
      DSS.clear();
      DSS << Id->GetName() << '[' << I << ']';
      DS = DSS.str();

      if (MM.find(DS) == MM.end()) {
        STE = ASTSymbolTable::Instance().Lookup(DS, ASTIntNode::IntBits,
                                                Id->GetSymbolType());
        if (!STE) {
          ASTIdentifierNode *EId =
              new ASTIdentifierNode(DS, ASTTypeAngle, ASTIntNode::IntBits);
          assert(EId && "Could not create a valid ASTIdentifierNode!");

          STE = new ASTSymbolTableEntry(EId, ASTTypeAngle);
          assert(STE && "Could not create a valid ASTSymbolTableEntry!");

          EId->SetSymbolTableEntry(STE);
          assert(EId->HasSymbolTableEntry() &&
                 "ASTIdentifierNode has no SymbolTable Entry!");
        }

        assert(STE && "ASTAngleNode has no SymbolTable Entry!");

        if (!MM.insert(std::make_pair(DS, STE)).second) {
          std::stringstream M;
          M << "Failure transferring symbol " << DS << " to the "
            << "local Symbol Table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
              DiagLevel::ICE);
          return false;
        }
      }

      ASTSymbolTable::Instance().EraseLocalSymbol(DS, ASTIntNode::IntBits,
                                                  Id->GetSymbolType());
    }

    if (Id->GetSymbolScope() == ASTSymbolScope::Global)
      ASTSymbolTable::Instance().EraseGlobalSymbol(Id, Id->GetBits(),
                                                   Id->GetSymbolType());
    else
      ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(),
                                                  Id->GetSymbolType());
  } break;
  case ASTTypeQubit: {
    const ASTSymbolTableEntry *STE = Id->GetSymbolTableEntry();
    assert(STE && "Identifier without an ASTSymbolTableEntry!");

    if (MM.find(Id->GetName()) == MM.end()) {
      if (!MM.insert(std::make_pair(Id->GetName(), STE)).second) {
        std::stringstream M;
        M << "Failure transferring symbol " << Id->GetName() << " to the "
          << "local Symbol Table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
            DiagLevel::ICE);
        return false;
      }
    }

    ASTSymbolTable::Instance().EraseLocalSymbol(Id->GetName());
  } break;
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias: {
    const ASTSymbolTableEntry *STE = Id->GetSymbolTableEntry();
    assert(STE && "Identifier without an ASTSymbolTableEntry!");

    std::stringstream QSS;
    for (unsigned I = 0; I < Id->GetBits(); ++I) {
      QSS.str("");
      QSS.clear();
      QSS << '%' << Id->GetName() << ':' << I;

      if (MM.find(QSS.str()) == MM.end()) {
        if (!MM.insert(std::make_pair(QSS.str(), STE)).second) {
          std::stringstream M;
          M << "Failure transferring symbol " << QSS.str() << " to the "
            << "local Symbol Table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
              DiagLevel::ICE);
          return false;
        }
      }

      ASTSymbolTable::Instance().EraseLocalSymbol(QSS.str());

      QSS.str("");
      QSS.clear();
      QSS << Id->GetName() << '[' << I << ']';

      if (MM.find(QSS.str()) == MM.end()) {
        if (!MM.insert(std::make_pair(QSS.str(), STE)).second) {
          std::stringstream M;
          M << "Failure transferring symbol " << QSS.str() << " to the "
            << "local Symbol Table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
              DiagLevel::ICE);
          return false;
        }
      }

      ASTSymbolTable::Instance().EraseLocalSymbol(QSS.str());
    }

    ASTSymbolTable::Instance().EraseLocalSymbol(Id->GetName());
  } break;
  case ASTTypeBitset: {
    if (Id->GetSymbolType() != ASTTypeBitset) {
      std::stringstream M;
      M << "Identifier " << Id->GetName() << " does not represent an "
        << PrintTypeEnum(ASTTypeBitset) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }

    std::stringstream BIS;

    if (Id->GetSymbolTableEntry()) {
      for (unsigned I = 0; I < Id->GetBits(); ++I) {
        BIS << Id->GetName() << '[' << I << ']';
        if (const ASTSymbolTableEntry *STE =
                ASTSymbolTable::Instance().FindLocalSymbol(BIS.str(), 1U,
                                                           ASTTypeBitset)) {
          if (!MM.insert(std::make_pair(BIS.str(), STE)).second) {
            std::stringstream M;
            M << "Failure inserting Symbol '" << BIS.str() << "' to the Local "
              << "Symbol Table.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                DiagLevel::ICE);
            return false;
          }

          ASTSymbolTable::Instance().EraseLocalSymbol(BIS.str(), 1U,
                                                      ASTTypeBitset);
          BIS.clear();
          BIS.str("");
        }
      }
    }
  } break;
  default:
    if (Id->GetSymbolScope() == ASTSymbolScope::Global)
      ASTSymbolTable::Instance().EraseGlobalSymbol(Id, Id->GetBits(),
                                                   Id->GetSymbolType());
    else
      ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(),
                                                  Id->GetSymbolType());
    break;
  }

  return true;
}

bool ASTDeclarationBuilder::TransferResult(
    const ASTResultNode *RN,
    std::map<std::string, const ASTSymbolTableEntry *> &MM) {
  assert(RN && "Invalid ASTResultNode argument!");

  bool RT = false;

  switch (RN->GetResultType()) {
  case ASTTypeBool: {
    if (const ASTBoolNode *BN = RN->GetBoolNode())
      RT = TransferSymbol(BN->GetIdentifier(), MM);
  } break;
  case ASTTypeInt: {
    if (const ASTIntNode *IN = RN->GetIntNode())
      RT = TransferSymbol(IN->GetIdentifier(), MM);
  } break;
  case ASTTypeFloat: {
    if (const ASTFloatNode *FN = RN->GetFloatNode())
      RT = TransferSymbol(FN->GetIdentifier(), MM);
  } break;
  case ASTTypeDouble: {
    if (const ASTDoubleNode *DN = RN->GetDoubleNode())
      RT = TransferSymbol(DN->GetIdentifier(), MM);
  } break;
  case ASTTypeLongDouble: {
    if (const ASTLongDoubleNode *LN = RN->GetLongDoubleNode())
      RT = TransferSymbol(LN->GetIdentifier(), MM);
  } break;
  case ASTTypeVoid: {
    if (const ASTVoidNode *VN = RN->GetVoidNode())
      RT = TransferSymbol(VN->GetIdentifier(), MM);
  } break;
  case ASTTypeDuration: {
    if (const ASTDurationNode *DN = RN->GetDurationNode())
      RT = TransferSymbol(DN->GetIdentifier(), MM);
  } break;
  case ASTTypeOpenPulseFrame: {
    if (const OpenPulse::ASTOpenPulseFrameNode *FN = RN->GetFrame())
      RT = TransferSymbol(FN->GetIdentifier(), MM);
  } break;
  case ASTTypeOpenPulseWaveform: {
    if (const OpenPulse::ASTOpenPulseWaveformNode *WN = RN->GetWaveform())
      RT = TransferSymbol(WN->GetIdentifier(), MM);
  } break;
  case ASTTypeOpenPulsePort: {
    if (const OpenPulse::ASTOpenPulsePortNode *PN = RN->GetPort())
      RT = TransferSymbol(PN->GetIdentifier(), MM);
  } break;
  case ASTTypeAngle: {
    if (const ASTAngleNode *AN = RN->GetAngleNode())
      RT = TransferSymbol(AN->GetIdentifier(), MM);
  } break;
  case ASTTypeMPInteger: {
    if (const ASTMPIntegerNode *MPI = RN->GetMPInteger())
      RT = TransferSymbol(MPI->GetIdentifier(), MM);
  } break;
  case ASTTypeMPDecimal: {
    if (const ASTMPDecimalNode *MPD = RN->GetMPDecimal())
      RT = TransferSymbol(MPD->GetIdentifier(), MM);
  } break;
  case ASTTypeMPComplex: {
    if (const ASTMPComplexNode *MPC = RN->GetMPComplex())
      RT = TransferSymbol(MPC->GetIdentifier(), MM);
  } break;
  case ASTTypeBitset: {
    if (const ASTCBitNode *CBN = RN->GetCBitNode())
      RT = TransferSymbol(CBN->GetIdentifier(), MM);
  } break;
  case ASTTypeBinaryOp: {
    if (const ASTBinaryOpNode *BOP = RN->GetBinaryOp())
      RT = TransferSymbol(BOP->GetIdentifier(), MM);
  } break;
  case ASTTypeUnaryOp: {
    if (const ASTUnaryOpNode *UOP = RN->GetUnaryOp())
      RT = TransferSymbol(UOP->GetIdentifier(), MM);
  } break;
  case ASTTypeComplexList:
    RT = true;
    break;
  default:
    break;
  }

  if (!RT) {
    std::stringstream M;
    M << "Impossible Result of Type " << PrintTypeEnum(RN->GetResultType())
      << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(RN), M.str(), DiagLevel::Error);
    return false;
  }

  return TransferSymbol(RN->GetIdentifier(), MM);
}

bool ASTDeclarationBuilder::TransferReturn(
    const ASTReturnStatementNode *RN,
    std::map<std::string, const ASTSymbolTableEntry *> &MM) {
  assert(RN && "Invalid ASTResultNode argument!");

  bool RT = false;

  switch (RN->GetReturnType()) {
  case ASTTypeVoid: {
    if (const ASTVoidNode *VN = RN->GetVoid())
      RT = TransferSymbol(VN->GetIdentifier(), MM);
    else
      RT = true;
  } break;
  case ASTTypeBool: {
    if (const ASTBoolNode *BN = RN->GetBool())
      RT = TransferSymbol(BN->GetIdentifier(), MM);
  } break;
  case ASTTypeInt:
  case ASTTypeUInt: {
    if (const ASTIntNode *IN = RN->GetInt())
      RT = TransferSymbol(IN->GetIdentifier(), MM);
  } break;
  case ASTTypeFloat: {
    if (const ASTFloatNode *FN = RN->GetFloat())
      RT = TransferSymbol(FN->GetIdentifier(), MM);
  } break;
  case ASTTypeDouble: {
    if (const ASTDoubleNode *DN = RN->GetDouble())
      RT = TransferSymbol(DN->GetIdentifier(), MM);
  } break;
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger: {
    if (const ASTMPIntegerNode *MPI = RN->GetMPInteger())
      RT = TransferSymbol(MPI->GetIdentifier(), MM);
  } break;
  case ASTTypeMPDecimal: {
    if (const ASTMPDecimalNode *MPD = RN->GetMPDecimal())
      RT = TransferSymbol(MPD->GetIdentifier(), MM);
  } break;
  case ASTTypeMPComplex: {
    if (const ASTMPComplexNode *MPC = RN->GetMPComplex())
      RT = TransferSymbol(MPC->GetIdentifier(), MM);
  } break;
  case ASTTypeAngle: {
    if (const ASTAngleNode *AN = RN->GetAngle()) {
      if (ASTTypeSystemBuilder::Instance().IsReservedAngle(
              AN->GetIdentifier()->GetName()))
        RT = true;
      else
        RT = TransferSymbol(AN->GetIdentifier(), MM);
    }
  } break;
  case ASTTypeBitset: {
    if (const ASTCBitNode *CB = RN->GetCBit())
      RT = TransferSymbol(CB->GetIdentifier(), MM);
  } break;
  case ASTTypeQubit: {
    if (const ASTQubitNode *QB = RN->GetQubit())
      RT = TransferSymbol(QB->GetIdentifier(), MM);
  } break;
  case ASTTypeQubitContainer: {
    if (const ASTQubitContainerNode *QCN = RN->GetQubitContainer())
      RT = TransferSymbol(QCN->GetIdentifier(), MM);
  } break;
  case ASTTypeMeasure: {
    bool MRT = true;

    if (const ASTMeasureNode *MN = RN->GetMeasure()) {
      switch (MN->GetResultType()) {
      case ASTTypeBitset: {
        if (const ASTCBitNode *CBN = MN->GetResult())
          MRT = TransferSymbol(CBN->GetIdentifier(), MM);
      } break;
      case ASTTypeAngle: {
        if (const ASTAngleNode *AN = MN->GetAngleResult()) {
          if (ASTTypeSystemBuilder::Instance().IsReservedAngle(
                  AN->GetIdentifier()->GetName()))
            MRT = true;
          else
            MRT = TransferSymbol(AN->GetIdentifier(), MM);
        }
      } break;
      case ASTTypeMPComplex: {
        if (const ASTMPComplexNode *MPC = MN->GetComplexResult())
          MRT = TransferSymbol(MPC->GetIdentifier(), MM);
      } break;
      default:
        break;
      }

      RT = TransferSymbol(MN->GetIdentifier(), MM) && MRT;
    }
  } break;
  case ASTTypeBinaryOp: {
    if (const ASTBinaryOpNode *BOP = RN->GetBinaryOp())
      RT = TransferSymbol(BOP->GetIdentifier(), MM);
  } break;
  case ASTTypeUnaryOp: {
    if (const ASTUnaryOpNode *UOP = RN->GetUnaryOp())
      RT = TransferSymbol(UOP->GetIdentifier(), MM);
  } break;
  case ASTTypeValue:
    RT = true;
    break;
  case ASTTypeExpression: {
    if (const ASTExpressionNode *EX = RN->GetExpression()) {
      const ASTDeclarationContext *DCX =
          ASTDeclarationContextTracker::Instance().GetCurrentContext();
      assert(DCX && "Could not obtain a valid ASTDeclarationContext!");

      if (EX->GetDeclarationContext() == DCX)
        RT = TransferSymbol(EX->GetIdentifier(), MM);
    }
  } break;
  case ASTTypeCast: {
    if (const ASTCastExpressionNode *CE = RN->GetCast()) {
      const ASTDeclarationContext *DCX =
          ASTDeclarationContextTracker::Instance().GetCurrentContext();
      assert(DCX && "Could not obtain a valid ASTDeclarationContext!");

      if (CE->GetDeclarationContext() == DCX)
        RT = TransferSymbol(CE->GetIdentifier(), MM);
    }
  } break;
  case ASTTypeStatement: {
    if (const ASTStatementNode *SN = RN->GetStatement()) {
      const ASTDeclarationContext *DCX =
          ASTDeclarationContextTracker::Instance().GetCurrentContext();
      assert(DCX && "Could not obtain a valid ASTDeclarationContext!");

      if (SN->GetDeclarationContext() == DCX)
        RT = TransferSymbol(SN->GetIdentifier(), MM);
    }
  } break;
  case ASTTypeFunctionCallStatement: {
    if (const ASTFunctionCallStatementNode *FSN = RN->GetFunctionStatement()) {
      const ASTDeclarationContext *DCX =
          ASTDeclarationContextTracker::Instance().GetCurrentContext();
      assert(DCX && "Could not obtain a valid ASTDeclarationContext!");

      if (FSN->GetDeclarationContext() == DCX)
        RT = TransferSymbol(FSN->GetIdentifier(), MM);
    }
  } break;
  case ASTTypeIdentifier: {
    if (const ASTIdentifierNode *ID = RN->GetIdent()) {
      const ASTDeclarationContext *DCX =
          ASTDeclarationContextTracker::Instance().GetCurrentContext();
      assert(DCX && "Could not obtain a valid ASTDeclarationContext!");

      // An Identifier may have been declared at a dominating
      // Declaration Context.
      if (ID->GetDeclarationContext()->GetIndex() < DCX->GetIndex()) {
        const ASTSymbolTableEntry *STE =
            ASTSymbolTable::Instance().FindLocal(ID);
        if (!STE || ID->GetDeclarationContext()->IsDead()) {
          std::stringstream M;
          M << "Unknown identifier '" << ID->GetName() << "' at current scope.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(ID), M.str(),
              DiagLevel::Error);
          return false;
        }
      } else if (ID->GetDeclarationContext()->GetIndex() == DCX->GetIndex()) {
        RT = TransferSymbol(ID, MM);
      }
    }
  } break;
  default:
    break;
  }

  if (!RT) {
    std::stringstream M;
    M << "Impossible Return Statement of Type "
      << PrintTypeEnum(RN->GetReturnType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(RN), M.str(), DiagLevel::Error);
    return false;
  }

  return TransferSymbol(RN->GetIdentifier(), MM);
}

void ASTDeclarationBuilder::CheckLoopInductionVariable(
    const ASTIdentifierNode *Id, const ASTStatementList *SL,
    const ASTDeclarationContext *DCX) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(SL && "Invalid ASTStatementList argument!");

  for (ASTStatementList::const_iterator I = SL->begin(); I != SL->end(); ++I) {
    const ASTStatementNode *SN = dynamic_cast<const ASTStatementNode *>(*I);
    if (const ASTDeclarationNode *DN =
            dynamic_cast<const ASTDeclarationNode *>(SN)) {
      if (DN->IsDeclaration() && DN->GetName() == Id->GetName() &&
          DN->GetDeclarationContext() == Id->GetDeclarationContext() &&
          DN->GetDeclarationContext() == DCX) {
        std::stringstream M;
        if (DN->GetASTType() == Id->GetSymbolType())
          M << "Redeclaration of loop induction variable '" << Id->GetName()
            << "' at enclosed scope.";
        else
          M << "Redeclaration of symbol '" << Id->GetName()
            << "' at enclosed scope.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(DN), M.str(),
            DiagLevel::Error);
      }
    }
  }
}

void ASTDeclarationBuilder::CheckLoopInductionVariable(
    const ASTIdentifierNode *Id, const ASTStatement *ST,
    const ASTDeclarationContext *DCX) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(ST && "Invalid ASTStatement argument!");

  const ASTStatementNode *SN = dynamic_cast<const ASTStatementNode *>(ST);
  if (const ASTDeclarationNode *DN =
          dynamic_cast<const ASTDeclarationNode *>(SN)) {
    if (DN->IsDeclaration() && DN->GetName() == Id->GetName() &&
        DN->GetDeclarationContext() == Id->GetDeclarationContext() &&
        DN->GetDeclarationContext() == DCX) {
      std::stringstream M;
      if (DN->GetASTType() == Id->GetSymbolType())
        M << "Redeclaration of loop induction variable '" << Id->GetName()
          << "' at enclosed scope.";
      else
        M << "Redeclaration of symbol '" << Id->GetName()
          << "' at enclosed scope.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(DN), M.str(),
          DiagLevel::Error);
    }
  }
}

bool ASTDeclarationBuilder::ThisMayBeRedeclaration(
    const ASTIdentifierNode *Id, const ASTDeclarationContext *DCX,
    uint32_t *DCIX) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DCX && "Invalid ASTDeclarationContext argument!");

  uint32_t F = 0U;
  *DCIX = static_cast<unsigned>(~0x0);

  if (DCX && ASTRedeclarationController::Instance().TypeAllowsRedeclaration(
                 Id->GetSymbolType())) {
    const ASTDeclarationContext *GCX =
        ASTDeclarationContextTracker::Instance().GetGlobalContext();
    if (Id->GetDeclarationContext()->GetIndex() >= GCX->GetIndex()) {
      std::vector<ASTDeclarationNode *> DV = DM.FindRange(Id);
      const ASTIdentifierNode *DId = nullptr;

      for (std::vector<ASTDeclarationNode *>::const_iterator I = DV.begin();
           I != DV.end(); ++I) {
        DId = (*I)->GetIdentifier();

        if (Id->GetDeclarationContext() == DId->GetDeclarationContext() ||
            DId->GetDeclarationContext() == DCX) {
          F += 1U;
          *DCIX = DId->GetDeclarationContext()->GetIndex();
          break;
        }
      }
    }
  }

  return F;
}

bool ASTDeclarationBuilder::DeclAlreadyExists(
    const ASTIdentifierNode *Id, const ASTDeclarationContext *DCX) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  uint32_t DCIX;
  if (DCX && Id->IsRedeclaration() && ThisMayBeRedeclaration(Id, DCX, &DCIX)) {
    if (DCIX == DCX->GetIndex()) {
      const char *SC = DCIX == 0 ? "Global" : "Local";
      std::stringstream M;
      M << "Declaration " << Id->GetName() << " shadows a previous "
        << "declaration at " << SC << " Scope: (" << Id->GetName() << ", "
        << PrintTypeEnum(Id->GetSymbolType()) << ").";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return true;
    }
  }

  const ASTSymbolTableEntry *STE = nullptr;

  switch (Id->GetSymbolType()) {
  case ASTTypeGate:
  case ASTTypeCNotGate:
  case ASTTypeCXGate:
  case ASTTypeCCXGate:
  case ASTTypeHadamardGate:
  case ASTTypeUGate: {
    STE = ASTSymbolTable::Instance().FindGate(Id);
    if (STE && STE->GetIdentifier() != Id &&
        STE->GetIdentifier()->GetName() == Id->GetName()) {
      std::stringstream M;
      M << "Declaration " << Id->GetName() << " shadows a previous "
        << "declaration at Global Scope: (" << STE->GetIdentifier()->GetName()
        << ", " << PrintTypeEnum(STE->GetValueType()) << ").";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return true;
    }

    return false;
  } break;
  case ASTTypeFunction: {
    STE = ASTSymbolTable::Instance().FindFunction(Id);
    if (STE && STE->GetIdentifier() != Id &&
        STE->GetIdentifier()->GetName() == Id->GetName()) {
      std::stringstream M;
      M << "Declaration " << Id->GetName() << " shadows a previous "
        << "declaration at Global Scope: (" << STE->GetIdentifier()->GetName()
        << ", " << PrintTypeEnum(STE->GetValueType()) << ").";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return true;
    }

    return false;
  } break;
  case ASTTypeDefcal: {
    STE = ASTSymbolTable::Instance().FindDefcal(Id->GetDefcalGroupName(),
                                                Id->GetMangledName());
    if (STE && STE->GetIdentifier() != Id &&
        STE->GetIdentifier()->GetName() == Id->GetName()) {
      std::stringstream M;
      M << "Declaration " << Id->GetName() << " shadows a previous "
        << "declaration at Global Scope: (" << STE->GetIdentifier()->GetName()
        << ", " << PrintTypeEnum(STE->GetValueType()) << ").";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return true;
    }

    return false;
  } break;
  case ASTTypeQubit:
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias: {
    STE = ASTSymbolTable::Instance().FindQubit(Id);
    if (STE && STE->GetIdentifier() != Id) {
      std::stringstream M;
      M << "Declaration " << Id->GetName() << " shadows a previous "
        << "declaration at Global Scope: (" << STE->GetIdentifier()->GetName()
        << ", " << PrintTypeEnum(STE->GetValueType()) << ").";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return true;
    }

    return false;
  } break;
  case ASTTypeEulerAngle:
  case ASTTypeLambdaAngle:
  case ASTTypePhiAngle:
  case ASTTypePiAngle:
  case ASTTypeTauAngle:
  case ASTTypeThetaAngle:
  case ASTTypeAngle: {
    STE = ASTSymbolTable::Instance().FindAngle(Id);
    if (STE && STE->GetIdentifier()->GetName() == Id->GetName() &&
        (STE == Id->GetSymbolTableEntry() ||
         STE->GetIdentifier()->GetBits() == Id->GetBits()) &&
        STE->GetContext() == Id->GetDeclarationContext()) {
      std::vector<std::string> SVN;
      std::vector<const ASTSymbolTableEntry *> SVS;

      std::stringstream SSN;
      for (unsigned I = 0; I < 4; ++I) {
        SSN << Id->GetName() << '[' << I << ']';
        if (const ASTSymbolTableEntry *SVSE =
                ASTSymbolTable::Instance().FindAngle(SSN.str())) {
          SVS.push_back(SVSE);
        }

        SSN.clear();
        SSN.str("");
      }

      if (SVS.size() == 4) {
        const char *SC = STE->IsGlobalScope() ? "Global" : "Local";
        std::stringstream M;
        M << "Declaration '" << Id->GetName() << "' shadows a previous "
          << "declaration at " << SC << " Scope: ("
          << PrintTypeEnum(STE->GetValueType()) << ").";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
            DiagLevel::Error);
        return true;
      }
    }

    return false;
  } break;
  default: {
    STE = ASTSymbolTable::Instance().FindLocal(Id);
    if (STE && (STE->GetIdentifier() != Id &&
                STE->GetIdentifier()->GetDeclarationContext() ==
                    Id->GetDeclarationContext())) {
      std::stringstream M;
      M << "Declaration " << Id->GetName() << " shadows a previous "
        << "declaration at Local Scope: (" << STE->GetIdentifier()->GetName()
        << ", " << PrintTypeEnum(STE->GetValueType()) << ").";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return true;
    }

    STE = ASTSymbolTable::Instance().FindGlobal(Id);
    if (STE && STE->GetIdentifier() != Id) {
      if (ASTRedeclarationController::Instance().TypeAllowsRedeclaration(Id) &&
          !ASTIdentifierTypeController::Instance().TypeScopeIsAlwaysGlobal(
              Id) &&
          Id->IsRedeclaration() && Id->IsLocalScope())
        return false;

      std::stringstream M;
      M << "Declaration " << Id->GetName() << " shadows a previous "
        << "declaration at Global Scope: (" << STE->GetIdentifier()->GetName()
        << ", " << PrintTypeEnum(STE->GetValueType()) << ").";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return true;
    }
  } break;
  }

  STE = nullptr;

  if (DM.Find(Id) != nullptr) {
    STE = ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                            Id->GetSymbolType());
    if (STE && STE != Id->GetSymbolTableEntry()) {
      switch (Id->GetSymbolType()) {
      case ASTTypeGate:
      case ASTTypeDefcal:
      case ASTTypeFunction: {
        std::stringstream M;
        M << "Declaration " << Id->GetName() << " shadows a previous "
          << "declaration at Global Scope: (" << STE->GetIdentifier()->GetName()
          << ", " << PrintTypeEnum(STE->GetValueType()) << ").";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
            DiagLevel::Error);
        return true;
      } break;
      case ASTTypeQubit:
      case ASTTypeQubitContainer:
      case ASTTypeQubitContainerAlias: {
        if (Id->GetName()[0] == '$') {
          std::stringstream M;
          M << "Bound Qubit declaration " << Id->GetName() << " shadows "
            << "a previous Bound Qubit declaration.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
              DiagLevel::Error);
          return true;
        }
      } break;
      default:
        break;
      }
    }

    std::vector<ASTDeclarationNode *> DV =
        ASTDeclarationBuilder::Instance().FindRange(Id);
    STE = Id->GetSymbolTableEntry();
    if (STE) {
      ASTSymbolTableEntry *DSTE = ASTSymbolTable::Instance().Lookup(
          Id, Id->GetBits(), STE->GetValueType());

      if (STE == DSTE && STE->GetIdentifier() == DSTE->GetIdentifier() &&
          STE->GetValueType() == DSTE->GetValueType() &&
          STE->GetContext() == DSTE->GetContext() &&
          STE->GetScope() == DSTE->GetScope())
        return false;

      if (DSTE &&
          (DSTE->GetContext()->GetIndex() > STE->GetContext()->GetIndex())) {
        std::stringstream M;
        if (ASTDeclarationContextTracker::Instance().IsGlobalContext(
                STE->GetContext())) {
          M << "Declaration shadows a previous declaration at Global Scope: ("
            << DSTE->GetIdentifier()->GetName() << ", "
            << PrintTypeEnum(DSTE->GetValueType()) << ").";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
              DiagLevel::Error);
          return true;
        } else {
          for (std::vector<ASTDeclarationNode *>::const_iterator DI =
                   DV.begin();
               DI != DV.end(); ++DI) {
            const ASTIdentifierNode *DId = (*DI)->GetIdentifier();
            if (Id == DId) {
              M << "Re-declaration of the same variable (" << Id->GetName();
              if (STE->GetValueType() == DSTE->GetValueType())
                M << ") of the same Type.";
              else
                M << ") with a different Type ("
                  << PrintTypeEnum(STE->GetValueType()) << " vs. "
                  << PrintTypeEnum(DSTE->GetValueType()) << ").";
              QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                  DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                  DiagLevel::Error);
              return true;
            } else {
              const ASTDeclarationContext *CTX = Id->GetDeclarationContext();
              while (CTX) {
                if (CTX == DId->GetDeclarationContext() ||
                    ASTDeclarationContextTracker::Instance().IsGlobalContext(
                        DId->GetDeclarationContext())) {
                  M << "Declaration shadows a previous declaration: ("
                    << DId->GetName() << ", "
                    << PrintTypeEnum(DId->GetSymbolType()) << ").";
                  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                      DIAGLineCounter::Instance().GetLocation(DId), M.str(),
                      DiagLevel::Error);
                  return true;
                }

                CTX = CTX->GetParentContext();
              }
            }
          }

          return false;
        }
      } else if (DSTE && DSTE->GetContext()->GetIndex() ==
                             STE->GetContext()->GetIndex()) {
        std::stringstream M;
        M << "Re-declaration of the same variable (" << Id->GetName();
        if (STE->GetValueType() == DSTE->GetValueType())
          M << ") of the same Type.";
        else
          M << ") with a different Type (" << PrintTypeEnum(STE->GetValueType())
            << " vs. " << PrintTypeEnum(DSTE->GetValueType()) << ").";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
            DiagLevel::Error);
        return true;
      }
    } else {
      std::stringstream M;
      M << "A declaration with the same identifier but without "
        << "a SymbolTable Entry exists.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    }
  }

  return false;
}

} // namespace QASM
