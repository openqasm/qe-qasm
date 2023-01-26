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

#ifndef __QASM__AST_GATE_QUBIT_TRACKER_H
#define __QASM__AST_GATE_QUBIT_TRACKER_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTIdentifierBuilder.h>

#include <iostream>
#include <string>
#include <set>

namespace QASM {

class ASTGateQubitTracker {
private:
  static std::set<const ASTIdentifierNode*> QIS;
  static std::set<std::string> RS;
  static ASTGateQubitTracker QT;

protected:
  ASTGateQubitTracker() = default;

public:
  static ASTGateQubitTracker& Instance() {
    return QT;
  }

  ~ASTGateQubitTracker() = default;

  void Insert(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    char S = Id->GetName()[0];

    if (Id->IsGateLocal() && S != '$')
      QIS.insert(Id);
  }

  void Clear() {
    QIS.clear();
  }

  bool Exists(const ASTIdentifierNode* Id) const {
    return QIS.find(Id) != QIS.end();
  }

  const ASTIdentifierNode* Find(const ASTIdentifierNode* Id) const {
    std::set<const ASTIdentifierNode*>::const_iterator I = QIS.find(Id);
    return I == QIS.end() ? nullptr : *I;
  }

  unsigned Size() const {
    return static_cast<unsigned>(QIS.size());
  }

  void Erase(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    char S = Id->GetName()[0];

    if (Id->IsGateLocal() && S != '$' &&
        Id->GetSymbolType() == ASTTypeGateQubitParam)
      QIS.erase(Id);
  }

  void Erase() {
    for (std::set<const ASTIdentifierNode*>::iterator I = QIS.begin();
         I != QIS.end(); ++I) {
       const std::string& S = (*I)->GetName();

      if (RS.find(S) == RS.end() && (*I)->IsGateLocal() &&
          S[0] != '$' && S[0] != '%') {
        const ASTSymbolTableEntry* STE = (*I)->GetSymbolTableEntry();
        assert(STE && "ASTIdentifierNode has no SymbolTable Entry!");

        switch (STE->GetValueType()) {
        case ASTTypeGate:
        case ASTTypeDefcal:
        case ASTTypeQubit:
        case ASTTypeQubitContainer:
        case ASTTypeQubitContainerAlias:
          break;
        case ASTTypeGateQubitParam:
          ASTIdentifierBuilder::List()->Erase(const_cast<ASTIdentifierNode*>((*I)));
          break;
        default:
          break;
        }

        if ((*I)->IsReference()) {
          if (const ASTIdentifierRefNode* IdR =
              dynamic_cast<const ASTIdentifierRefNode*>(*I)) {
            STE = IdR->GetSymbolTableEntry();
            assert(STE && "ASTIdentifierNode has no SymbolTable Entry!");

            if (STE->HasValue()) {
              switch (STE->GetValueType()) {
              case ASTTypeGate:
              case ASTTypeDefcal:
              case ASTTypeQubit:
              case ASTTypeQubitContainer:
              case ASTTypeQubitContainerAlias:
                break;
              case ASTTypeGateQubitParam:
                ASTSymbolTable::Instance().EraseGateQubitParam(IdR->GetName(),
                                                               IdR->GetBits(),
                                                               STE->GetValueType());
                ASTSymbolTable::Instance().EraseGateLocalQubit((*I)->GetName());
                break;
              default:
                break;
              }
            }
          }
        }
      }
    }

    Clear();
  }

  void print() const {
    std::cout << "<GateQubitTracker>" << std::endl;
    for (std::set<const ASTIdentifierNode*>::iterator I = QIS.begin();
         I != QIS.end(); ++I) {
      (*I)->print();
    }
    std::cout << "</GateQubitTracker>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM__AST_GATE_QUBIT_TRACKER_H

