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

#include <qasm/AST/ASTMPComplexList.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <cassert>
#include <iostream>
#include <string>

namespace QASM {

using DiagLevel = QasmDiagnosticEmitter::DiagLevel;

void ASTMPComplexList::ConstructFromExpressionList(
    const ASTExpressionList &EL) {
  if (EL.Empty())
    return;

  for (ASTExpressionList::const_iterator I = EL.begin(); I != EL.end(); ++I) {
    const ASTExpressionNode *EN = dynamic_cast<const ASTExpressionNode *>(*I);
    if (EN) {
      switch (EN->GetASTType()) {
      case ASTTypeMPComplex: {
        if (const ASTMPComplexNode *MPC =
                dynamic_cast<const ASTMPComplexNode *>(EN)) {
          List.push_back(
              new ASTMPComplexRep(const_cast<ASTMPComplexNode *>(MPC)));
        }
      } break;
      case ASTTypeBinaryOp: {
        if (const ASTBinaryOpNode *BOP =
                dynamic_cast<const ASTBinaryOpNode *>(EN)) {
          ASTComplexExpressionNode *CEN =
              new ASTComplexExpressionNode(&ASTIdentifierNode::MPComplex, BOP);
          assert(CEN && "Could not create a valid ASTComplexExpression from "
                        "an ASTBinaryOpNode!");
          ASTMPComplexNode *MPC =
              new ASTMPComplexNode(&ASTIdentifierNode::MPComplex, CEN, 128);
          List.push_back(
              new ASTMPComplexRep(const_cast<ASTBinaryOpNode *>(BOP), MPC));
        }
      } break;
      case ASTTypeUnaryOp: {
        if (const ASTUnaryOpNode *UOP =
                dynamic_cast<const ASTUnaryOpNode *>(EN)) {
          ASTComplexExpressionNode *CEN =
              new ASTComplexExpressionNode(&ASTIdentifierNode::MPComplex, UOP);
          assert(UOP && "Could not create a valid ASTComplexExpression from "
                        "an ASTUnaryOpNode!");
          ASTMPComplexNode *MPC =
              new ASTMPComplexNode(&ASTIdentifierNode::MPComplex, CEN, 128);
          List.push_back(
              new ASTMPComplexRep(const_cast<ASTUnaryOpNode *>(UOP), MPC));
        }
      } break;
      case ASTTypeIdentifier: {
        ASTSymbolTableEntry *STE = nullptr;
        const ASTIdentifierNode *Id = EN->GetIdentifier();
        assert(Id && "Could not obtain a valid ASTIdentifierNode from "
                     "the ASTExpressionNode!");
        unsigned NumBits = 32;
        while (!STE && NumBits < 512) {
          STE =
              ASTSymbolTable::Instance().Lookup(Id, NumBits, ASTTypeMPComplex);
          if (STE) {
            ASTMPComplexNode *MPC =
                STE->GetValue()->GetValue<ASTMPComplexNode *>();
            assert(MPC && "Could not obtain a valid ASTMPComplexNode from "
                          "the SymbolTable!");
            List.push_back(new ASTMPComplexRep(Id, MPC));
            break;
          } else {
            NumBits *= 2U;
            continue;
          }
        }
      } break;
      case ASTTypeIdentifierRef: {
        const ASTIdentifierNode *Id = EN->GetIdentifier();
        assert(Id && "Could not obtain a valid ASTIdentifierNode from "
                     "the ASTExpressionNode!");
        if (Id->IsReference()) {
          const ASTIdentifierRefNode *IdR =
              dynamic_cast<const ASTIdentifierRefNode *>(Id);
          assert(IdR && "Could not dynamic_cast to an ASTIdentifierRefNode!");
          ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().Lookup(IdR);
          assert(STE && "ASTIdentifierRefNode has no SymbolTable Entry!");

          if (STE->GetValueType() == ASTTypeMPComplex) {
            ASTMPComplexNode *MPC =
                STE->GetValue()->GetValue<ASTMPComplexNode *>();
            assert(MPC && "Could not obtain a valid ASTMPComplexNode "
                          "from the SymbolTable!");
            List.push_back(new ASTMPComplexRep(MPC));
          } else if (STE->GetValueType() == ASTTypeMPComplexArray) {
            // FIXME: IMPLEMENT COMPLEX ARRAYS.
            std::stringstream M;
            M << "Complex arrays are not implemented yet.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(IdR), M.str(),
                DiagLevel::Warning);
          } else {
            std::stringstream M;
            M << "Impossible initialization of an ASTMPComplexNode from "
              << "type " << PrintTypeEnum(STE->GetValueType()) << ".";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(IdR), M.str(),
                DiagLevel::Error);
          }
        } else {
          std::stringstream M;
          M << "Expression does not evaluate to an Identifier reference.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
              DiagLevel::Error);
        }
      } break;
      case ASTTypeComplexExpression: {
        if (const ASTComplexExpressionNode *CEN =
                dynamic_cast<const ASTComplexExpressionNode *>(EN)) {
          ASTMPComplexNode *MPC =
              new ASTMPComplexNode(&ASTIdentifierNode::MPComplex, CEN, 128);
          List.push_back(
              new ASTMPComplexRep(const_cast<ASTMPComplexNode *>(MPC)));
        }
      } break;
      default: {
        std::stringstream M;
        M << "Impossible initialization of an ASTMPComplexNode from "
          << "type " << PrintTypeEnum(EN->GetASTType()) << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(EN), M.str(),
            DiagLevel::Error);
      } break;
      }
    }
  }
}

ASTMPComplexList::ASTMPComplexList(const ASTExpressionList &EL)
    : ASTBase(), List() {
  ConstructFromExpressionList(EL);
}

std::string
ASTMPComplexRep::MangleIdentifier(const ASTIdentifierNode *Id) const {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  switch (Id->GetSymbolType()) {
  case ASTTypeBool: {
    if (ASTBoolNode *B =
            Id->GetSymbolTableEntry()->GetValue()->GetValue<ASTBoolNode *>()) {
      B->Mangle();
      return ASTStringUtils::Instance().SanitizeMangled(B->GetMangledName());
    }
  } break;
  case ASTTypeInt:
  case ASTTypeUInt: {
    if (ASTIntNode *I =
            Id->GetSymbolTableEntry()->GetValue()->GetValue<ASTIntNode *>()) {
      I->Mangle();
      return ASTStringUtils::Instance().SanitizeMangled(I->GetMangledName());
    }
  } break;
  case ASTTypeFloat: {
    if (ASTFloatNode *F =
            Id->GetSymbolTableEntry()->GetValue()->GetValue<ASTFloatNode *>()) {
      F->Mangle();
      return ASTStringUtils::Instance().SanitizeMangled(F->GetMangledName());
    }
  } break;
  case ASTTypeMPInteger: {
    if (ASTMPIntegerNode *MPI = Id->GetSymbolTableEntry()
                                    ->GetValue()
                                    ->GetValue<ASTMPIntegerNode *>()) {
      MPI->Mangle();
      return ASTStringUtils::Instance().SanitizeMangled(MPI->GetMangledName());
    }
  } break;
  case ASTTypeMPDecimal: {
    if (ASTMPDecimalNode *MPD = Id->GetSymbolTableEntry()
                                    ->GetValue()
                                    ->GetValue<ASTMPDecimalNode *>()) {
      MPD->Mangle();
      return ASTStringUtils::Instance().SanitizeMangled(MPD->GetMangledName());
    }
  } break;
  case ASTTypeMPComplex: {
    if (ASTMPComplexNode *MMPC = Id->GetSymbolTableEntry()
                                     ->GetValue()
                                     ->GetValue<ASTMPComplexNode *>()) {
      MMPC->Mangle();
      return ASTStringUtils::Instance().SanitizeMangled(MMPC->GetMangledName());
    }
  } break;
  case ASTTypeAngle: {
    if (ASTAngleNode *A =
            Id->GetSymbolTableEntry()->GetValue()->GetValue<ASTAngleNode *>()) {
      A->Mangle();
      return ASTStringUtils::Instance().SanitizeMangled(A->GetMangledName());
    }
  } break;
  default:
    break;
  }

  ASTMangler M;
  M.Start();
  M.TypeIdentifier(Id->GetSymbolType(), Id->GetName());
  M.EndExpression();
  M.End();
  return ASTStringUtils::Instance().SanitizeMangled(M.AsString());
}

std::string ASTMPComplexRep::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(ASTTypeMPComplexRep, "complexrep");

  if (MPC) {
    MPC->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(MPC->GetMangledName()));
  }

  switch (RType) {
  case ASTTypeBinaryOp:
    assert(BOP && "Invalid ASTBinaryOpNode!");
    const_cast<ASTBinaryOpNode *>(BOP)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(BOP->GetMangledName()));
    break;
  case ASTTypeUnaryOp:
    assert(UOP && "Invalid ASTUnaryOpNode!");
    const_cast<ASTUnaryOpNode *>(UOP)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(UOP->GetMangledName()));
    break;
  case ASTTypeIdentifier:
    assert(ID && "Invalid ASTIdentifierNode!");
    M.StringValue(MangleIdentifier(ID));
    break;
  case ASTTypeIdentifierRef:
    assert(IDR && "Invalid ASTIdentifierRefNode!");
    M.StringValue(MangleIdentifier(IDR->GetIdentifier()));
    break;
  default:
    break;
  }

  M.EndExpression();
  M.End();
  return M.AsString();
}

std::string ASTMPComplexList::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(ASTTypeComplexList, "complexlist");

  if (List.empty()) {
    M.StringValue("E");
  } else {
    for (ASTMPComplexList::const_iterator I = List.begin(); I != List.end();
         ++I) {
      // M.Identifier((*I)->Mangle());
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled((*I)->Mangle()));
    }
  }

  M.EndExpression();
  M.End();
  return M.AsString();
}

} // namespace QASM
