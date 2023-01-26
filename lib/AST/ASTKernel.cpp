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

#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTKernel.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTResult.h>
#include <qasm/AST/ASTKernelBuilder.h>
#include <qasm/AST/ASTKernelContextBuilder.h>
#include <qasm/AST/ASTKernelStatementBuilder.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <vector>
#include <sstream>
#include <cassert>

namespace QASM {

ASTKernelStatementBuilder ASTKernelStatementBuilder::KSB;
ASTStatementList ASTKernelStatementBuilder::VS;

std::map<std::string, ASTKernelNode*> ASTKernelBuilder::KM;
ASTKernelBuilder ASTKernelBuilder::KB;

ASTKernelContextBuilder ASTKernelContextBuilder::KCB;
bool ASTKernelContextBuilder::KCS;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTKernelNode::ASTKernelNode(const ASTIdentifierNode* Id,
                             const ASTDeclarationList& DL,
                             const ASTStatementList& SL,
                             ASTResultNode* RES)
  : ASTStatementNode(Id), Params(), Statements(SL), Result(RES),
  STM(), Extern(true), Ellipsis(false) {
    const ASTDeclarationContext* KCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
    assert(KCX && "Could not obtain a valid ASTDeclarationContext!");

    unsigned PIX = 0U;
    unsigned EC = 0U;

    for (ASTDeclarationList::const_iterator I = DL.begin(); I != DL.end(); ++I) {
      if (Ellipsis) {
        std::stringstream M;
        M << "Ellipsis must be last in a kernel parameter list.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(*I), M.str(), DiagLevel::Error);
      }

      if ((*I)->GetASTType() == ASTTypeEllipsis) {
        Ellipsis = true;
        ++EC;
      }

      const ASTIdentifierNode* DId = (*I)->GetIdentifier();
      assert(DId && "Could not obtain a valid ASTIdentifierNode!");

      const ASTSymbolTableEntry* DSTE = DId->GetSymbolTableEntry();
      if (!DSTE) {
        if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(DId->GetName()) &&
            !ASTStringUtils::Instance().IsBoundQubit(DId->GetName())) {
          DSTE = ASTSymbolTable::Instance().FindLocalSymbol(DId);
          if (!DSTE)
            DSTE = ASTSymbolTable::Instance().FindGlobalSymbol(DId);
          if (!DSTE) {
            DSTE = ASTSymbolTable::Instance().CreateSymbolTableEntry(
                                              DId, DId->GetSymbolType());
            const_cast<ASTIdentifierNode*>(DId)->SetSymbolTableEntry(
                                          const_cast<ASTSymbolTableEntry*>(DSTE));
          }
        }
      }

      assert(DSTE && "Could not obtain/create a valid ASTSymbolTableEntry!");

      if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(DId->GetName()) &&
          !ASTStringUtils::Instance().IsBoundQubit(DId->GetName())) {
        const_cast<ASTDeclarationNode*>(*I)->SetDeclarationContext(KCX);
        const_cast<ASTIdentifierNode*>(DId)->SetDeclarationContext(KCX);
        const_cast<ASTIdentifierNode*>(DId)->SetLocalScope();
        const_cast<ASTSymbolTableEntry*>(DSTE)->SetContext(KCX);
        const_cast<ASTSymbolTableEntry*>(DSTE)->SetLocalScope();
      }

      Params.insert(std::make_pair(PIX++, *I));

      if (!STM.insert(std::make_pair((*I)->GetName(), DSTE)).second) {
        std::stringstream M;
        M << "Error inserting parameter symbol into the Kernel Symbol Table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(DId), M.str(),
                                                        DiagLevel::Error);
      }

      if (ASTUtils::Instance().IsQubitType(DId->GetSymbolType())) {
        std::stringstream QS;

        for (unsigned J = 0; J < DId->GetBits(); ++J) {
          QS.str("");
          QS.clear();
          QS << '%' << DId->GetName() << ':' << J;
          DSTE = ASTSymbolTable::Instance().FindLocal(QS.str());
          assert(DSTE && "Invalid ASTQubitContainerNode without Qubits!");

          if (!STM.insert(std::make_pair(QS.str(), DSTE)).second) {
            std::stringstream M;
            M << "Error inserting Qubit parameter symbol into the Kernel "
              << "SymbolTable.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(DId), M.str(),
                                                            DiagLevel::Error);
          }

          QS.str("");
          QS.clear();
          QS << DId->GetName() << '[' << J << ']';
          DSTE = ASTSymbolTable::Instance().FindLocal(QS.str());
          assert(DSTE && "Invalid ASTQubitContainerNode without Qubits!");

          if (!STM.insert(std::make_pair(QS.str(), DSTE)).second) {
            std::stringstream M;
            M << "Error inserting Qubit parameter symbol into the Kernel "
              << "SymbolTable.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(DId), M.str(),
                                                            DiagLevel::Error);
          }
        }
      } else if (ASTUtils::Instance().IsAngleType(DId->GetSymbolType())) {
        std::stringstream AS;

        for (unsigned J = 0; J < 3; ++J) {
          AS.str("");
          AS.clear();
          AS << DId->GetName() << '[' << J << ']';
          const ASTSymbolTableEntry* ASTE =
            ASTSymbolTable::Instance().FindLocal(AS.str());

          if (!ASTE)
            ASTE = new ASTSymbolTableEntry(DId, DId->GetSymbolType());
          assert(ASTE && "Could not create/obtain a valid ASTSymbolTable Entry!");

          if (!STM.insert(std::make_pair(AS.str(), ASTE)).second) {
            std::stringstream M;
            M << "Error inserting Angle parameter symbol into the Kernel "
              << "SymbolTable.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(DId), M.str(),
                                                            DiagLevel::Error);
          }
        }
      }

      if (ASTUtils::Instance().IsQubitType(DId->GetSymbolType()))
        ASTSymbolTable::Instance().EraseLocalQubit(DId);

      if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(DId->GetName()) &&
          !ASTStringUtils::Instance().IsBoundQubit(DId->GetName())) {
        if (ASTUtils::Instance().IsAngleType(DId->GetSymbolType()))
          ASTSymbolTable::Instance().EraseLocalAngle(DId->GetName());
        else {
          if (ASTSymbolTable::Instance().FindLocal(DId))
            ASTSymbolTable::Instance().EraseLocalSymbol(DId, DId->GetBits(),
                                                        DId->GetSymbolType());
          else if (ASTSymbolTable::Instance().FindGlobal(DId))
            ASTSymbolTable::Instance().EraseGlobalSymbol(DId, DId->GetBits(),
                                                         DId->GetSymbolType());
        }
      }
    }

    if (EC > 1) {
      std::stringstream M;
      M << "Ellipsis can only appear once in a kernel parameter list.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(&DL), M.str(), DiagLevel::Error);
    }

    const ASTIdentifierNode* RId = RES->GetIdentifier();
    assert(RId && "Invalid ASTIdentifierNode for function ASTResultNode!");

    const ASTSymbolTableEntry* RSTE = ASTSymbolTable::Instance().FindLocal(RId);
    assert(RSTE && "Function ASTResultNode has no SymbolTable Entry!");

    RES->SetDeclarationContext(KCX);
    const_cast<ASTIdentifierNode*>(RId)->SetDeclarationContext(KCX);
    const_cast<ASTIdentifierNode*>(RId)->SetLocalScope();
    const_cast<ASTSymbolTableEntry*>(RSTE)->SetContext(KCX);
    const_cast<ASTSymbolTableEntry*>(RSTE)->SetLocalScope();

    if (!STM.insert(std::make_pair(RId->GetName(), RSTE)).second) {
      std::stringstream M;
      M << "Error inserting result symbol into the Kernel SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(RId), M.str(), DiagLevel::Error);
    }

    ASTSymbolTable::Instance().EraseLocalSymbol(RId, RId->GetBits(),
                                                RId->GetSymbolType());
}

void ASTKernelNode::TransferLocalSymbolTable() {
  std::map<std::string, ASTSymbolTableEntry*>& LSTM =
    ASTSymbolTable::Instance().GetLSTM();

  if (!LSTM.empty()) {
    std::vector<std::string> SV;
    const ASTDeclarationContext* DCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
    assert(DCX && "Could not obtain a valid current DeclarationContext!");

    for (std::map<std::string, ASTSymbolTableEntry*>::iterator I = LSTM.begin();
         I != LSTM.end(); ++I) {
      if ((*I).second && ((*I).second->GetContext() == DCX)) {
        if (!STM.insert(std::make_pair((*I).first, (*I).second)).second) {
          std::stringstream M;
          M << "Kernel symbol table insertion failure for local symbol "
            << (*I).first.c_str() << '.';
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
        }

        SV.push_back((*I).first);
      }
    }

    for (std::vector<std::string>::const_iterator I = SV.begin();
         I != SV.end(); ++I)
      LSTM.erase(*I);
  }
}

void ASTKernelNode::print() const {
  std::cout << "<Kernel>" << std::endl;
  std::cout << "<Identifier>" << this->GetName() << "</Identifier>"
    << std::endl;
  std::cout << "<MangledName>" << GetMangledName()
    << "</MangledName>" << std::endl;
  std::cout << "<Extern>" << std::boolalpha << Extern << "</Extern>"
    << std::endl;

  if (!Params.empty()) {
    std::cout << "<KernelParameters>" << std::endl;
    for (std::map<unsigned, ASTDeclarationNode*>::const_iterator I = Params.begin();
         I != Params.end(); ++I) {
      std::cout << "<KernelParameter>" << std::endl;
      std::cout << "<ParameterIndex>" << (*I).first << "</ParameterIndex>"
        << std::endl;
      (*I).second->print();
      std::cout << "</KernelParameter>" << std::endl;
    }
    std::cout << "</KernelParameters>" << std::endl;
  }

  Statements.print();
  if (Result)
    Result->print();

  std::cout << "<HasEllipsis>" << std::boolalpha << Ellipsis
    << "</HasEllipsis>" << std::endl;
  std::cout << "</Kernel>" << std::endl;
}

void ASTKernelNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (Extern)
    M.Type(ASTTypeExtern);
  M.TypeIdentifier(GetASTType(), GetName());
  M.FuncReturn(Result->GetResultType());

  for (std::map<unsigned, ASTDeclarationNode*>::iterator I = Params.begin();
       I != Params.end(); ++I) {
      const ASTIdentifierNode* Id = (*I).second->GetIdentifier();
      assert(Id && "Invalid kernel parameter ASTIdentifierNode!");

      M.KernelParam((*I).first, Id->GetSymbolType(), Id->GetBits());
  }

  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace QASM

