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

#include <qasm/AST/ASTLiveRangeChecker.h>

namespace QASM {

ASTLiveRangeChecker ASTLiveRangeChecker::LRC;

void ASTLiveRangeChecker::LiveRangeCheck(ASTIdentifierList *IL,
                                         ASTExpressionNode *EN, ASTType Ty,
                                         std::set<std::string> &IDS) const {
  assert(IL && "Invalid ASTIdentifierList argument!");
  assert(EN && "Invalid ASTExpressionNode argument!");

  // Silence compiler warning about unused argument for now.
  // This will be used when checking for casts.
  (void)Ty;

  ASTType ETy = EN->GetASTType();

  switch (ETy) {
  case ASTTypeBool: {
    ASTBoolNode *BN = dynamic_cast<ASTBoolNode *>(EN);
    assert(BN && "Could not dynamic_cast to an ASTBoolNode!");
    IDS.insert(BN->GetIdentifier()->GetName());
  } break;
  case ASTTypeInt: {
    ASTIntNode *IN = dynamic_cast<ASTIntNode *>(EN);
    assert(IN && "Could not dynamic_cast to an ASTIntNode!");
    IDS.insert(IN->GetIdentifier()->GetName());
  } break;
  case ASTTypeFloat: {
    ASTFloatNode *FN = dynamic_cast<ASTFloatNode *>(EN);
    assert(FN && "Could not dynamic_cast to an ASTFloatNode!");
    IDS.insert(FN->GetIdentifier()->GetName());
  } break;
  case ASTTypeDouble: {
    ASTDoubleNode *DN = dynamic_cast<ASTDoubleNode *>(EN);
    assert(DN && "Could not dynamic_cast to an ASTDoubleNode!");
    IDS.insert(DN->GetIdentifier()->GetName());
  } break;
  case ASTTypeMPInteger: {
    ASTMPIntegerNode *MPI = dynamic_cast<ASTMPIntegerNode *>(EN);
    assert(MPI && "Could not dynamic_cast to an ASTIMPIntegerNode!");
    IDS.insert(MPI->GetIdentifier()->GetName());
  } break;
  case ASTTypeMPDecimal: {
    ASTMPDecimalNode *MPD = dynamic_cast<ASTMPDecimalNode *>(EN);
    assert(MPD && "Could not dynamic_cast to an ASTMPDecimalNode!");
    IDS.insert(MPD->GetIdentifier()->GetName());
  } break;
  case ASTTypeBinaryOp: {
    ASTBinaryOpNode *BOP = dynamic_cast<ASTBinaryOpNode *>(EN);
    assert(BOP && "Could not dynamic_cast to an ASTBinaryOpNode!");

    IDS.insert(BOP->GetIdentifier()->GetName());
    assert(BOP->GetLeft() && "Left ASTBinaryOpNode has no Left Node!");

    const ASTExpressionNode *BOPN = BOP->GetLeft();
    assert(BOPN && "ASTBinaryOpNode has no Left Node!");

    IDS.insert(BOP->GetLeftIdentifier()->GetName());

    while (BOPN->GetASTType() == ASTTypeBinaryOp) {
      const ASTBinaryOpNode *LBOP = dynamic_cast<const ASTBinaryOpNode *>(BOPN);
      assert(LBOP && "Could not dynamic_cast to a Left ASTBinaryOpNode!");

      IDS.insert(LBOP->GetLeftIdentifier()->GetName());
      BOPN = LBOP->GetLeft();
    }

    assert(BOP->GetRight() && "Left ASTBinaryOpNode has no Right Node!");

    BOPN = BOP->GetRight();
    assert(BOPN && "ASTBinaryOpNode has no Right Node!");

    IDS.insert(BOP->GetRightIdentifier()->GetName());

    while (BOPN->GetASTType() == ASTTypeBinaryOp) {
      const ASTBinaryOpNode *RBOP = dynamic_cast<const ASTBinaryOpNode *>(BOPN);
      assert(RBOP && "Could not dynamic_cast to a Right ASTBinaryOpNode!");

      IDS.insert(RBOP->GetRightIdentifier()->GetName());
      BOPN = RBOP->GetRight();
    }
  } break;
  case ASTTypeUnaryOp: {
    ASTUnaryOpNode *UOP = dynamic_cast<ASTUnaryOpNode *>(EN);
    assert(UOP && "Could not dynamic_cast to an ASTUnaryOpNode!");
    IDS.insert(UOP->GetIdentifier()->GetName());
    assert(UOP->GetExpression() && "ASTUnaryOpNode has no Right Node!");

    IDS.insert(UOP->GetRightIdentifier()->GetName());
    const ASTExpressionNode *RUOP = UOP->GetExpression();
    assert(RUOP && "ASTUnaryOpNode has no Right Expression!");

    while (RUOP->GetASTType() == ASTTypeUnaryOp) {
      const ASTUnaryOpNode *IUOP = dynamic_cast<const ASTUnaryOpNode *>(RUOP);
      assert(IUOP && "Could not dynamic_cast to an ASTUnaryOpNode!");
      IDS.insert(IUOP->GetIdentifier()->GetName());
      RUOP = IUOP;
    }
  } break;
  default:
    break;
  }
}

} // namespace QASM
