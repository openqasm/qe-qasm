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

#ifndef __QASM_AST_GATE_FOR_H
#define __QASM_AST_GATE_FOR_H

#include <qasm/AST/ASTLoops.h>
#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <iostream>

namespace QASM {

/// Gate-body `for` wrapped as a listable quantum op (`ASTGateQOpList`).
class ASTGateForOpNode : public ASTGateQOpNode {
private:
  const ASTForStatementNode *ForStmt;

private:
  ASTGateForOpNode() = delete;

public:
  ASTGateForOpNode(const ASTIdentifierNode *Id, const ASTForStatementNode *FS)
      : ASTGateQOpNode(Id), ForStmt(FS) {
    assert(FS && "Invalid ASTForStatementNode argument!");
  }

  virtual ~ASTGateForOpNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGateFor; }

  virtual const ASTForStatementNode *GetForStatement() const { return ForStmt; }

  virtual void Mangle() override {
    // Induction / body already mangled when the ForStatement was built.
  }

  virtual void print() const override {
    std::cout << "<GateForOpNode>" << std::endl;
    if (ForStmt)
      ForStmt->print();
    std::cout << "</GateForOpNode>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_GATE_FOR_H
