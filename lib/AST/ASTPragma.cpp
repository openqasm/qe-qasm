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

#include <qasm/AST/ASTPragma.h>
#include <qasm/AST/ASTPragmaContextBuilder.h>
#include <qasm/AST/ASTMangler.h>

namespace QASM {

ASTPragmaContextBuilder ASTPragmaContextBuilder::PCB;
bool ASTPragmaContextBuilder::PCS;
std::vector<std::string> ASTPragmaContextBuilder::PXV;

void ASTPragmaNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), PN);

  if (!EL.Empty()) {
    for (ASTExpressionNodeList::iterator EI = EL.begin();
         EI != EL.end(); ++EI) {
      if (ASTStringNode* SN = dynamic_cast<ASTStringNode*>(*EI)) {
        SN->Mangle();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                                 SN->GetMangledName()));
      }
    }
  }

  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

} // namespace QASM

