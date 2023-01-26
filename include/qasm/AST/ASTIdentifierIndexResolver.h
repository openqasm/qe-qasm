/* -*- coding: utf-8 -*-
 *
 * Copyright 2021 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_IDENTIFIER_INDEX_RESOLVER_H
#define __QASM_AST_IDENTIFIER_INDEX_RESOLVER_H

#include <qasm/AST/ASTTypes.h>

#include <string>
#include <cassert>

namespace QASM {

class ASTIdentifierIndexResolver {
private:
  static ASTIdentifierIndexResolver IIR;

protected:
  ASTIdentifierIndexResolver() = default;

public:
  static ASTIdentifierIndexResolver& Instance() {
    return IIR;
  }

  ~ASTIdentifierIndexResolver() = default;

  unsigned ResolveIndex(const std::string& Id) const;

  unsigned ResolveIndex(const ASTIdentifierNode* Id) const;

  unsigned ResolveIndex(const ASTIntNode* I) const {
    assert(I && "Invalid ASTIntNode argument!");

    return I->IsSigned() ? static_cast<unsigned>(I->GetSignedValue()) :
                           I->GetUnsignedValue();
  }

  unsigned ResolveIndex(const ASTMPIntegerNode* MPI) const {
    assert(MPI && "Invalid ASTMPIntegerNode argument!");

    return MPI->IsSigned() ? static_cast<unsigned>(MPI->ToSignedInt()) :
                             MPI->ToUnsignedInt();
  }
};

} // namespace QASM

#endif // __QASM_AST_IDENTIFIER_INDEX_RESOLVER_H

