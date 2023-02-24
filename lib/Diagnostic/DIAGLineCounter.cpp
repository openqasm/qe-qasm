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

#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/AST/ASTBase.h>

namespace QASM {

DIAGLineCounter DIAGLineCounter::DLC;
uint64_t DIAGLineCounter::ILC = 0ULL;

FileLineColLoc DIAGLineCounter::GetLocation(const ASTBase* LB) const {
  if (LB) {
    return {File, LB->GetLineNo(), LB->GetColNo()};
  } else {
    return GetLocation();
  }
}

FileLineColLoc DIAGLineCounter::GetLocation(const ASTBase& LB) const {
  return {File, LB.GetLineNo(), LB.GetColNo()};
}

FileLineColLoc DIAGLineCounter::GetLocation(const ASTToken* TK) const {
  assert(TK && "Invalid ASTToken argument!");

  return {File, TK->GetLocation().GetLineNo(), TK->GetLocation().GetColNo()};
}

FileLineColLoc DIAGLineCounter::GetLocation(const ASTToken& TK) const {
  return {File, TK.GetLocation().GetLineNo(), TK.GetLocation().GetColNo()};
}

std::string DIAGLineCounter::GetIdentifierLocation(const ASTBase* LB) const {
  if (LB) {
    std::stringstream S;
    S << LB->GetLineNo() << '-' << LB->GetColNo() << '-' << ++ILC;
    return S.str();
  } else {
    return GetIdentifierLocation();
  }
}

std::string DIAGLineCounter::GetIdentifierLocation(const ASTToken* TK) const {
  if (TK) {
    std::stringstream S;
    S << TK->GetLocation().GetLineNo() << '-' << TK->GetLocation().GetColNo()
      << '-' << ++ILC;
    return S.str();
  } else {
    return GetIdentifierLocation();
  }
}

std::ostream &operator<<(std::ostream &o, FileLineColLoc Loc) {
  o << "File: " << Loc.Filename << ", Line: " << Loc.Line << ", Col: " << Loc.Col;
  return o;
}

} // namespace QASM