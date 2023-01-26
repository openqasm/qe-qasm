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

#include <qasm/AST/ASTIfStatementTracker.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <sstream>

namespace QASM {

using DiagLevel = QasmDiagnosticEmitter::DiagLevel;

ASTIfStatementList ASTIfStatementTracker::IL;
ASTIfStatementTracker ASTIfStatementTracker::TR;
ASTIfStatementNode* ASTIfStatementTracker::CIF = nullptr;

ASTIfStatementList ASTElseIfStatementTracker::IL;
ASTElseIfStatementTracker ASTElseIfStatementTracker::EITR;
std::deque<unsigned> ASTElseIfStatementTracker::ISCQ;
unsigned ASTElseIfStatementTracker::PendingElse;
const ASTElseIfStatementNode* ASTElseIfStatementTracker::CEI;
bool ASTElseIfStatementTracker::POP;
unsigned ASTElseIfStatementTracker::C;

ASTIfStatementList ASTElseStatementTracker::IL;
ASTElseStatementTracker ASTElseStatementTracker::ETR;
std::deque<unsigned> ASTElseStatementTracker::ISCQ;
std::map<unsigned, ASTStatementList*> ASTElseStatementTracker::ESM;

bool
ASTElseStatementTracker::AddPendingElse(unsigned ISC, ASTStatementList* SL) {
  std::map<unsigned, ASTStatementList*>::iterator I = ESM.find(ISC);
  if (I == ESM.end())
    return ESM.insert(std::make_pair(ISC, SL)).second;

  if ((*I).second == SL)
    return true;

  std::stringstream M;
  M << "Only one else clause is allowed per if conditional!";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
  return false;
}

void
ASTElseStatementTracker::ClearPendingElse(unsigned ISC) {
  std::map<unsigned, ASTStatementList*>::iterator I = ESM.find(ISC);

  if (I != ESM.end())
    ESM.erase(I);
}

} // namespace QASM

