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

#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTParameterBuilder.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <cassert>

namespace QASM {

ASTParameterList ASTParameterBuilder::BL;
ASTParameterList* ASTParameterBuilder::BLP;
ASTParameterBuilder ASTParameterBuilder::B;
std::vector<ASTParameterList*> ASTParameterBuilder::BLV;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTParameter::ASTParameter(const ASTExpression* E, const std::string& Id)
  : ID(Id), Ident(nullptr), Expr(E), Decl(nullptr) {
  if (Id.empty()) {
    const ASTExpressionNode* EN = dynamic_cast<const ASTExpressionNode*>(E);
    if (EN) {
      Ident = EN->GetIdentifier();
      ID = Ident->GetName();
    }
  }
}

ASTType
ASTParameter::GetASTType() const {
  if (this->Expr)
    return this->Expr->GetASTType();
  else if (this->Decl)
    return this->Decl->GetASTType();

  return ASTTypeUndefined;
}

void ASTParameterList::SetLocalScope() {
  if (!Graph.empty()) {
    for (iterator I = Graph.begin(); I != Graph.end(); ++I) {
      if (ASTParameter* P = dynamic_cast<ASTParameter*>(*I)) {
        if (!P->GetName().empty()) {
          ASTSymbolTableEntry* STE =
            ASTSymbolTable::Instance().Lookup(P->GetName());
          if (STE) {
            const ASTIdentifierNode* Id = STE->GetIdentifier();
            switch (Id->GetSymbolType()) {
            case ASTTypeQubit:
            case ASTTypeQubitContainer:
            case ASTTypeQubitContainerAlias:
              if (ASTStringUtils::Instance().IsBoundQubit(Id->GetName()))
                continue;
              break;
            case ASTTypeAngle:
              if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()))
                continue;
              break;
            case ASTTypeGate:
            case ASTTypeDefcal:
            case ASTTypeFunction:
              continue;
              break;
            default:
              STE->SetLocalScope();
              break;
            }
          }
        }
      }
    }
  }
}

void ASTParameterList::DeleteSymbols() {
  if (!Graph.empty()) {
    for (iterator I = Graph.begin(); I != Graph.end(); ++I) {
      if (ASTParameter* P = dynamic_cast<ASTParameter*>(*I)) {
        const ASTIdentifierNode* Id = P->GetIdentifier();
        if (!P->GetName().empty() && Id) {
          ASTSymbolTableEntry* STE =
            const_cast<ASTSymbolTableEntry*>(Id->GetSymbolTableEntry());
          if (STE) {
            ASTType STy = STE->GetValueType();
            if (STy == ASTTypeQubitContainer) {
              ASTQubitContainerNode* QCN =
                STE->GetValue()->GetValue<ASTQubitContainerNode*>();
              assert(QCN && "Invalid ASTQubitContainerNode obtained "
                            "from the SymbolTable!");

              QCN->EraseFromLocalSymbolTable();
              continue;
            } else if (STy == ASTTypeQubit) {
              ASTSymbolTable::Instance().EraseLocalSymbol(Id->GetName(), Id->GetBits(),
                                                          Id->GetSymbolType());
              continue;
            } else if (STy == ASTTypeAngle) {
              ASTSymbolTable::Instance().EraseLocalSymbol(Id->GetName(), Id->GetBits(),
                                                          Id->GetSymbolType());
              continue;
            }

            ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(), STy);
            ASTSymbolTable::Instance().EraseLocal(Id, Id->GetBits(), STy);
          }
        }
      }
    }
  }
}

void
ASTParameterList::TransferSymbols(std::map<std::string,
                                           const ASTSymbolTableEntry*>& MM) {
  MM.clear();
  if (!Graph.empty()) {
    for (iterator I = Graph.begin(); I != Graph.end(); ++I) {
      if (ASTParameter* P = dynamic_cast<ASTParameter*>(*I)) {
        const ASTIdentifierNode* Id = P->GetIdentifier();
        if (!P->GetName().empty() && Id) {
          const ASTSymbolTableEntry* STE = Id->GetSymbolTableEntry();
          assert(STE && "Invalid SymbolTable Entry for ASTParameter!");

          if (!MM.insert(std::make_pair(Id->GetMangledName(), STE)).second) {
            std::stringstream M;
            M << "Failure transferring parameter " << Id->GetName()
              << " to the local Symbol Table.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                           DiagLevel::ICE);
          }
        }
      }
    }
  }
}

void
ASTParameterList::TransferGlobalSymbolsToLocal() {
  if (!Graph.empty()) {
    for (iterator I = Graph.begin(); I != Graph.end(); ++I) {
      if (ASTParameter* P = dynamic_cast<ASTParameter*>(*I)) {
        const ASTIdentifierNode* Id = P->GetIdentifier();
        if (!P->GetName().empty() && Id) {
          const ASTSymbolTableEntry* STE = Id->GetSymbolTableEntry();
          assert(STE && "Invalid SymbolTable Entry for ASTParameter!");

          if (!ASTSymbolTable::Instance().TransferGlobalSymbolToLocal(Id, STE)) {
            std::stringstream M;
            M << "Failure transferring symbol " << Id->GetName() << " to the "
              << "Local Symbol Table.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                           DiagLevel::ICE);
          }
        }
      }
    }
  }
}

void
ASTParameterList::TransferLocalSymbolsToGlobal() {
  if (!Graph.empty()) {
    for (iterator I = Graph.begin(); I != Graph.end(); ++I) {
      if (ASTParameter* P = dynamic_cast<ASTParameter*>(*I)) {
        const ASTIdentifierNode* Id = P->GetIdentifier();
        if (!P->GetName().empty() && Id) {
          const ASTSymbolTableEntry* STE = Id->GetSymbolTableEntry();
          assert(STE && "Invalid SymbolTable Entry for ASTParameter!");

          if (!ASTSymbolTable::Instance().TransferLocalSymbolToGlobal(Id, STE)) {
            std::stringstream M;
            M << "Failure transferring symbol " << Id->GetName() << " to the "
              << "Global Symbol Table.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                           DiagLevel::ICE);
          }
        }
      }
    }
  }
}



void
ASTParameter::print() const {
  std::cout << "<Parameter>" << std::endl;
  std::cout << "<Name>" << ID << "</Name>" << std::endl;

  if (Ident) {
    Ident->print();
  }

  if (Expr) {
    std::cout << "<Expression>" << std::endl;
    if (const ASTExpressionNode* EN =
                                 dynamic_cast<const ASTExpressionNode*>(Expr))
      EN->print();
    else
      Expr->print();
    std::cout << "</Expression>" << std::endl;
  }

  if (Decl) {
    Decl->print();
  }

  std::cout << "</Parameter>" << std::endl;
}

ASTParameterList&
ASTParameterList::operator=(const ASTIdentifierList* IL) {
  assert(IL && "Invalid ASTIdentifierList!");

  Clear();

  for (ASTIdentifierList::const_iterator I = IL->begin();
       I != IL->end(); ++I)
    Graph.push_back(new ASTParameter(*I));

  return *this;
}

ASTParameterList&
ASTParameterList::operator=(const ASTIdentifierList& IL) {
  Clear();

  if (!IL.Empty()) {
    for (ASTIdentifierList::const_iterator I = IL.begin();
         I != IL.end(); ++I)
      Graph.push_back(new ASTParameter(*I));
  }

  return *this;
}

ASTParameterList&
ASTParameterList::operator=(const ASTDeclarationList* DDL) {
  assert(DDL && "Invalid ASTDeclarationList argument!");

  Clear();

  DL = DDL;

  if (!DDL->Empty()) {
    for (ASTDeclarationList::const_iterator I = DL->begin();
         I != DL->end(); ++I)
      Graph.push_back(new ASTParameter((*I)->GetIdentifier(), *I));
  }

  return *this;
}

ASTParameterList&
ASTParameterList::operator=(const ASTDeclarationList& DDL) {
  Clear();

  if (!DDL.Empty()) {
    for (ASTDeclarationList::const_iterator I = DDL.begin();
         I != DDL.end(); ++I)
      Graph.push_back(new ASTParameter((*I)->GetIdentifier(), *I));
  }

  return *this;
}

void
ASTParameterList::Append(const ASTExpressionNode* EN) {
  assert(EN && "Invalid ASTExpressionNode!");

  const ASTIdentifierNode* IDN = EN->DynCast<const ASTIdentifierNode>();
  assert(IDN && "Invalid ASTIdentifierNode for ASTExpressionNode!");

  std::string Id = IDN->GetName();
  Graph.push_back(new ASTParameter(EN, Id));
}

void ASTParameterList::Append(const ASTDeclarationNode* DN) {
  assert(DN && "Invalid ASTDeclarationNode!");

  Graph.push_back(new ASTParameter(DN->GetIdentifier(), DN));
}

void
ASTParameterList::Append(const ASTIdentifierList* IL) {
  assert(IL && "Invalid ASTIdentifierList!");

  for (ASTIdentifierList::const_iterator I = IL->begin();
       I != IL->end(); ++I)
    Graph.push_back(new ASTParameter((*I)->GetIdentifier()));
}

void ASTParameterList::Append(const ASTDeclarationList* DDL) {
  assert(DDL && "Invalid ASTDeclarationList argument!");

  for (ASTDeclarationList::const_iterator I = DDL->begin();
       I != DDL->end(); ++I)
    Graph.push_back(new ASTParameter((*I)->GetIdentifier(), *I));

  if (DL) {
    for (ASTDeclarationList::const_iterator I = DDL->begin();
         I != DDL->end(); ++I) {
      const_cast<ASTDeclarationList*>(DL)->Append(*I);
    }
  }
}

ASTParameterList&
ASTParameterList::operator=(const std::vector<std::string>& VS) {
  Clear();

  if (!VS.size())
    return *this;

  for (std::vector<std::string>::const_iterator I = VS.begin();
       I != VS.end(); ++I)
    Graph.push_back(new ASTParameter(*I));

  return *this;
}

void
ASTParameterList::Erase(const std::string& Id) {
  if (Id.empty())
    return;

  for (ASTParameterList::iterator I = Graph.begin();
       I != Graph.end(); ++I) {
    if (ASTParameter* AP = dynamic_cast<ASTParameter*>(*I)) {
      if (AP->GetName() == Id) {
        Graph.erase(I);
        break;
      }
    }
  }
}

} // namespace QASM

