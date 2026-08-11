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

#ifndef __QASM_AST_GATE_OP_BUILDER_H
#define __QASM_AST_GATE_OP_BUILDER_H

#include <qasm/AST/ASTDefcal.h>
#include <qasm/AST/ASTGateOpList.h>
#include <qasm/AST/ASTGates.h>

#include <vector>

namespace QASM {

class ASTHGateOpNode;
class ASTCXGateOpNode;
class ASTCCXGateOpNode;
class ASTCNotGateOpNode;
class ASTUGateOpNode;
class ASTHadamardGateOpNode;
class ASTLambdaGateOpNode;
class ASTPhiGateOpNode;
class ASTThetaGateOpNode;

class ASTGateOpBuilder {
private:
  static ASTGateQOpList *GLP;
  static ASTGateOpBuilder B;
  static std::vector<ASTGateQOpList *> GLV;

protected:
  ASTGateOpBuilder() {}

public:
  using list_type = std::vector<ASTGateQOpNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  static ASTGateOpBuilder &Instance() { return ASTGateOpBuilder::B; }

  static void Init() {
    if (!GLP)
      GLP = new ASTGateQOpList();
  }

  ~ASTGateOpBuilder() = default;

  static ASTGateQOpList *List() { return ASTGateOpBuilder::GLP; }

  static ASTGateQOpList *NewList() { return GLP = new ASTGateQOpList(); }

  /// Nest a gate-op list (e.g. `for` body). Outer list restored by PopList.
  static void PushList() {
    GLV.push_back(GLP);
    GLP = nullptr;
  }

  static ASTGateQOpList *PopList() {
    ASTGateQOpList *Inner = GLP;
    assert(!GLV.empty() && "GateOpBuilder PopList without PushList!");
    GLP = GLV.back();
    GLV.pop_back();
    return Inner;
  }

  static ASTGateQOpNode *CreateASTQGateOpNode(const ASTIdentifierNode *Id,
                                              const ASTGateNode *GateNode);

  static ASTGenericGateOpNode *
  CreateASTGenericGateOpNode(const ASTIdentifierNode *Id,
                             const ASTGateNode *GateNode);

  static ASTGenericGateOpNode *
  CreateASTGenericDefcalOpNode(const ASTIdentifierNode *Id,
                               const ASTDefcalNode *DefcalNode);

  static ASTHGateOpNode *
  CreateASTHGateOpNode(const ASTIdentifierNode *Id,
                       const ASTHadamardGateNode *GateNode);

  static ASTCXGateOpNode *CreateASTCXGateOpNode(const ASTIdentifierNode *Id,
                                                const ASTCXGateNode *GateNode);

  static ASTCCXGateOpNode *
  CreateASTCCXGateOpNode(const ASTIdentifierNode *Id,
                         const ASTCCXGateNode *GateNode);

  static ASTCNotGateOpNode *
  CreateASTCNotGateOpNode(const ASTIdentifierNode *Id,
                          const ASTCNotGateNode *GateNode);

  static ASTUGateOpNode *CreateASTUGateOpNode(const ASTIdentifierNode *Id,
                                              const ASTUGateNode *GateNode);

  void Append(ASTGateQOpNode *Node) {
    assert(Node && "Invalid ASTGateQOpNode argument!");
    if (!Node->IsDirective())
      GLP->push(Node);
  }

  // Remove Node from the active list when a ctrl/negctrl/inv/pow wraps a
  // GateEOp that CreateAST*GateOpNode already appended as a bare sibling.
  bool PopIfBack(ASTGateQOpNode *Node) { return GLP && GLP->PopIfBack(Node); }

  void Clear() { GLP->Clear(); }

  std::size_t Size() { return GLP->Size(); }

  static ASTGateQOpNode *Root() { return GLP->front(); }

  iterator begin() { return GLP->begin(); }

  const_iterator begin() const { return GLP->begin(); }

  iterator end() { return GLP->end(); }

  const_iterator end() const { return GLP->end(); }
};

} // namespace QASM

#endif // __QASM_AST_GATE_OP_BUILDER_H
