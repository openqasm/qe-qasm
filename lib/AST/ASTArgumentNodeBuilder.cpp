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

#include <qasm/AST/ASTArgumentNodeBuilder.h>

#include <cassert>

namespace QASM {

ASTArgumentNodeList ASTArgumentNodeBuilder::AL;
ASTArgumentNodeBuilder ASTArgumentNodeBuilder::B;
ASTArgumentNodeList *ASTArgumentNodeBuilder::ALP = nullptr;
std::vector<ASTArgumentNodeList *> ASTArgumentNodeBuilder::ALV;

ASTArgumentNodeList &
ASTArgumentNodeList::operator=(const ASTExpressionList *EL) {
  assert(EL && "Invalid ASTExpressionList!");

  Graph.clear();

  for (ASTExpressionList::const_iterator I = EL->begin(); I != EL->end(); ++I)
    if (const ASTExpressionNode *EN =
            dynamic_cast<const ASTExpressionNode *>(*I))
      Graph.push_back(new ASTArgumentNode(EN));

  return *this;
}

} // namespace QASM
