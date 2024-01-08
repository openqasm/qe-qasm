/* -*- coding: utf-8 -*-
 *
 * Copyright 2023 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_IF_CONDITIONALS_GRAPH_CONTROLLER_H
#define __QASM_AST_IF_CONDITIONALS_GRAPH_CONTROLLER_H

#include <qasm/AST/ASTIfConditionals.h>
#include <qasm/AST/ASTTypes.h>

#include <vector>

namespace QASM {

class ASTIfConditionalsGraphController {
private:
  static ASTIfConditionalsGraphController GC;

protected:
  ASTIfConditionalsGraphController() = default;

public:
  static ASTIfConditionalsGraphController &Instance() { return GC; }

  ~ASTIfConditionalsGraphController() = default;

  void ResolveIfChain(std::vector<ASTIfStatementNode *> &PV) const;

  void ResolveIfEdges(ASTStatementList &SL,
                      std::vector<ASTIfStatementNode *> &PV) const;
  void RemoveOutOfScope(ASTStatementList &SL,
                        const ASTDeclarationContext *DCX) const;
};

} // namespace QASM

#endif // __QASM_AST_IF_CONDITIONALS_GRAPH_CONTROLLER_H
