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

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTGates.h>
#include <qasm/AST/ASTGateOpBuilder.h>

#include <cassert>

namespace QASM {

ASTGateQOpList* ASTGateOpBuilder::GLP;
ASTGateOpBuilder ASTGateOpBuilder::B;

ASTGateQOpNode*
ASTGateOpBuilder::CreateASTQGateOpNode(const ASTIdentifierNode* Id,
                                       const ASTGateNode* GateNode) {
  assert(Id && "Invalid QGate ASTIdentifierNode!");
  assert(GateNode && "Invalid QGate GateNode!");

  ASTGateQOpNode *QG = new ASTGateQOpNode(Id, GateNode);
  assert(QG && "Failed to create an ASTHGateOpNode!");

  ASTGateOpBuilder::Instance().Append(QG);
  return QG;
}

ASTGenericGateOpNode*
ASTGateOpBuilder::CreateASTGenericGateOpNode(const ASTIdentifierNode* Id,
                                             const ASTGateNode* GateNode) {
  assert(Id && "Invalid QGate ASTIdentifierNode!");
  assert(GateNode && "Invalid GGate GateNode!");

  ASTGenericGateOpNode *GG = new ASTGenericGateOpNode(Id, GateNode);
  assert(GG && "Failed to create an ASTGenericGateOpNode!");

  ASTGateOpBuilder::Instance().Append(GG);
  return GG;
}

ASTGenericGateOpNode*
ASTGateOpBuilder::CreateASTGenericDefcalOpNode(const ASTIdentifierNode* Id,
                                               const ASTDefcalNode* DefcalNode) {
  assert(Id && "Invalid Defcal ASTIdentifierNode argument!");
  assert(DefcalNode && "Invalid DefcalNode argument!");

  ASTGenericGateOpNode *GG = new ASTGenericGateOpNode(Id, DefcalNode);
  assert(GG && "Failed to create an ASTGenericGateOpNode!");

  ASTGateOpBuilder::Instance().Append(GG);
  return GG;
}

ASTHGateOpNode*
ASTGateOpBuilder::CreateASTHGateOpNode(const ASTIdentifierNode* Id,
                                       const ASTHadamardGateNode* GateNode) {
  assert(Id && "Invalid HGate ASTIdentifierNode!");
  assert(GateNode && "Invalid HGate GateNode!");

  ASTHGateOpNode* HG = new ASTHGateOpNode(Id, GateNode);
  assert(HG && "Failed to create an ASTHGateOpNode!");

  ASTGateOpBuilder::Instance().Append(HG);
  return HG;
}

ASTCXGateOpNode*
ASTGateOpBuilder::CreateASTCXGateOpNode(const ASTIdentifierNode* Id,
                                        const ASTCXGateNode* GateNode) {
  assert(Id && "Invalid CXGate ASTIdentifierNode!");
  assert(GateNode && "Invalid CXGate GateNode!");

  ASTCXGateOpNode* CXG = new ASTCXGateOpNode(Id, GateNode);
  assert(CXG && "Failed to create an ASTCXGateOpNode!");

  ASTGateOpBuilder::Instance().Append(CXG);
  return CXG;
}

ASTCCXGateOpNode*
ASTGateOpBuilder::CreateASTCCXGateOpNode(const ASTIdentifierNode* Id,
                                         const ASTCCXGateNode* GateNode) {
  assert(Id && "Invalid CCXGate ASTIdentifierNode!");
  assert(GateNode && "Invalid CCXGate GateNode!");

  ASTCCXGateOpNode* CCXG = new ASTCCXGateOpNode(Id, GateNode);
  assert(CCXG && "Failed to create an ASTCCXGateOpNode!");

  ASTGateOpBuilder::Instance().Append(CCXG);
  return CCXG;
}

ASTCNotGateOpNode*
ASTGateOpBuilder::CreateASTCNotGateOpNode(const ASTIdentifierNode* Id,
                                          const ASTCNotGateNode* GateNode) {
  assert(Id && "Invalid CNotGate ASTIdentifierNode!");
  assert(GateNode && "Invalid CNotGate GateNode!");

  ASTCNotGateOpNode* CNG = new ASTCNotGateOpNode(Id, GateNode);
  assert(CNG && "Failed to create an ASTCNotGateOpNode!");

  ASTGateOpBuilder::Instance().Append(CNG);
  return CNG;
}

ASTUGateOpNode*
ASTGateOpBuilder::CreateASTUGateOpNode(const ASTIdentifierNode* Id,
                                       const ASTUGateNode* GateNode) {
  assert(Id && "Invalid UGate ASTIdentifierNode!");
  assert(GateNode && "Invalid UGate GateNode!");

  ASTUGateOpNode* UG = new ASTUGateOpNode(Id, GateNode);
  assert(UG && "Failed to create an ASTUGateOpNode!");

  ASTGateOpBuilder::Instance().Append(UG);
  return UG;
}

} // namespace QASM

