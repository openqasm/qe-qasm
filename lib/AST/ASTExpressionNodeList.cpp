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

#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTExpressionNodeList.h>

#include <cassert>

namespace QASM {

ASTExpressionNodeList::ASTExpressionNodeList(const ASTDeclarationList& DL)
  : ASTBase(), List() {
  for (ASTDeclarationList::const_iterator I = DL.begin();
       I != DL.end(); ++I) {
    if (const ASTExpressionNode* EXN = (*I)->GetExpression()) {
      Append(const_cast<ASTExpressionNode*>(EXN));
    }
  }
}

ASTExpressionNodeList::ASTExpressionNodeList(const ASTParameterList& PL)
  : ASTBase(), List() {
  for (ASTParameterList::const_iterator I = PL.begin();
       I != PL.end(); ++I) {
    if (const ASTExpressionNode* EXN =
        dynamic_cast<const ASTExpressionNode*>(*I)) {
      Append(const_cast<ASTExpressionNode*>(EXN));
    }
  }
}

ASTExpressionNodeList::ASTExpressionNodeList(const ASTExpressionList& EL)
  : ASTBase(), List() {
  for (ASTExpressionList::const_iterator I = EL.begin();
       I != EL.end(); ++I) {
    if (const ASTExpressionNode* EXN =
        dynamic_cast<const ASTExpressionNode*>(*I)) {
      Append(const_cast<ASTExpressionNode*>(EXN));
    } else {
      if (const ASTExpression* EX = dynamic_cast<const ASTExpression*>(*I)) {
        ASTExpressionNode* EN = new ASTExpressionNode(EX, EX->GetIdentifier(),
                                                      EX->GetASTType());
        assert(EN && "Could not create a valid ASTExpressionNode!");
        Append(EN);
      }
    }
  }
}

ASTExpressionNodeList::ASTExpressionNodeList(const ASTStringList& SL)
  : ASTBase(), List() {
  if (!SL.Empty()) {
    for (ASTStringList::const_iterator I = SL.begin(); I != SL.end(); ++I) {
      Append(const_cast<ASTStringNode*>(*I));
    }
  }
}

} // namespace QASM

