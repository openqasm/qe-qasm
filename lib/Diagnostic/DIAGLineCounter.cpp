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
ASTLocation DIAGLineCounter::Location = {0,0};

const ASTLocation& DIAGLineCounter::GetLocation() const {
  Location.LineNo = LineNo;
  Location.ColNo = ColNo;
  return Location;
}

const ASTLocation& DIAGLineCounter::GetLocation(const ASTBase* LB) const {
  assert(LB && "Invalid ASTToken argument!");
  return LB->GetLocation();
}

const ASTLocation&  DIAGLineCounter::GetLocation(const ASTBase& LB) const {
  return LB.GetLocation();
}

const ASTLocation& DIAGLineCounter::GetLocation(const ASTToken* TK) const {
  assert(TK && "Invalid ASTToken argument!");
  return TK->GetLocation();
}

const ASTLocation& DIAGLineCounter::GetLocation(const ASTToken& TK) const {
  return TK.GetLocation();
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


} // namespace QASM
