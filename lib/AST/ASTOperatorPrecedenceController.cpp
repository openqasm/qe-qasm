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

#include <qasm/AST/ASTOperatorPrecedenceController.h>
#include <qasm/AST/ASTExpressionValidator.h>
#include <qasm/AST/ASTExpressionEvaluator.h>
#include <qasm/AST/ASTTypeCastController.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

namespace QASM {

ASTOperatorPrecedenceController ASTOperatorPrecedenceController::OPC;

const std::map<ASTOpType, uint32_t> ASTOperatorPrecedenceController::OPM = {
  { ASTOpType::ASTOpTypePreDec, 1001U },
  { ASTOpType::ASTOpTypePreInc, 1001U },
  { ASTOpType::ASTOpTypePostDec, 1001U },
  { ASTOpType::ASTOpTypePostInc, 1001U },
  { ASTOpType::ASTOpTypeSin, 1001U },
  { ASTOpType::ASTOpTypeCos, 1001U },
  { ASTOpType::ASTOpTypeTan, 1001U },
  { ASTOpType::ASTOpTypeArcSin, 1001U },
  { ASTOpType::ASTOpTypeArcCos, 1001U },
  { ASTOpType::ASTOpTypeArcTan, 1001U },
  { ASTOpType::ASTOpTypeExp, 1001U },
  { ASTOpType::ASTOpTypeLn, 1001U },
  { ASTOpType::ASTOpTypePow, 1001U },
  { ASTOpType::ASTOpTypeSqrt, 1001U },
  { ASTOpType::ASTOpTypeRotation, 1001U },
  { ASTOpType::ASTOpTypeRotl, 1001U },
  { ASTOpType::ASTOpTypeRotr, 1001U },
  { ASTOpType::ASTOpTypePopcount, 1001U },
  { ASTOpType::ASTOpTypeArraySubscript, 1001U },
  { ASTOpType::ASTOpTypePositive, 1002U },
  { ASTOpType::ASTOpTypeNegative, 1002U },
  { ASTOpType::ASTOpTypeLogicalNot, 1002U },
  { ASTOpType::ASTOpTypeBitNot, 1002U },
  { ASTOpType::ASTOpTypeNegate, 1002U },
  { ASTOpType::ASTOpTypeCast, 1002U },
  { ASTOpType::ASTOpTypeImplicitConversion, 1002U },
  { ASTOpType::ASTOpTypeMul, 1003U },
  { ASTOpType::ASTOpTypeDiv, 1003U },
  { ASTOpType::ASTOpTypeMod, 1003U },
  { ASTOpType::ASTOpTypeAdd, 1004U },
  { ASTOpType::ASTOpTypeSub, 1004U },
  { ASTOpType::ASTOpTypeLeftShift, 1005U },
  { ASTOpType::ASTOpTypeRightShift, 1005U },
  { ASTOpType::ASTOpTypeLT, 1006U },
  { ASTOpType::ASTOpTypeGT, 1006U },
  { ASTOpType::ASTOpTypeLE, 1006U },
  { ASTOpType::ASTOpTypeGE, 1006U },
  { ASTOpType::ASTOpTypeCompEq, 1007U },
  { ASTOpType::ASTOpTypeCompNeq, 1007U },
  { ASTOpType::ASTOpTypeBitAnd, 1008U },
  { ASTOpType::ASTOpTypeXor, 1009U },
  { ASTOpType::ASTOpTypeBitOr, 1010U },
  { ASTOpType::ASTOpTypeLogicalAnd, 1011U },
  { ASTOpType::ASTOpTypeLogicalOr, 1012U },
  { ASTOpType::ASTOpTypeAssign, 1014U },
  { ASTOpType::ASTOpTypeAddAssign, 1014U },
  { ASTOpType::ASTOpTypeSubAssign, 1014U },
  { ASTOpType::ASTOpTypeDivAssign, 1014U },
  { ASTOpType::ASTOpTypeMulAssign, 1014U },
  { ASTOpType::ASTOpTypeModAssign, 1014U },
  { ASTOpType::ASTOpTypeXorAssign, 1014U },
  { ASTOpType::ASTOpTypeBitAndAssign, 1014U },
  { ASTOpType::ASTOpTypeBitOrAssign, 1014U },
  { ASTOpType::ASTOpTypeLeftShiftAssign, 1014U },
  { ASTOpType::ASTOpTypeRightShiftAssign, 1014U },
  { ASTOpType::ASTOpTypeAssociate, 1015U },
};

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

void
ASTOperatorPrecedenceController::Expand(const ASTBinaryOpNode* BOP,
                                        std::multimap<uint32_t,
                                                      ASTVariantOpNode>& MOP) const {
  assert(BOP && "Invalid ASTBinaryOpNode argument!");

  const ASTExpressionNode* L = BOP->GetLeft();
  const ASTExpressionNode* R = BOP->GetRight();
  uint32_t OP = GetOperatorPrecedence(BOP->GetOpType());
  assert(OP != static_cast<unsigned>(~0x0) &&
         "Invalid Operator Precedence Rank!");

  if (BOP->HasParens())
    OP -= RB;

  switch (L->GetASTType()) {
  case ASTTypeBinaryOp: {
    const ASTBinaryOpNode* LBOP = dynamic_cast<const ASTBinaryOpNode*>(L);
    if (!LBOP) {
      std::stringstream M;
      M << "Failed to dynamic_cast to an ASTBinaryOpNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(L), M.str(), DiagLevel::ICE);
      return;
    }

    Expand(LBOP, MOP);
  }
    break;
  case ASTTypeUnaryOp: {
    const ASTUnaryOpNode* LUOP = dynamic_cast<const ASTUnaryOpNode*>(L);
    if (!LUOP) {
      std::stringstream M;
      M << "Failed to dynamic_cast to an ASTUnaryOpNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(L), M.str(), DiagLevel::ICE);
      return;
    }

    Expand(LUOP, MOP);
  }
    break;
  case ASTTypeComplexExpression: {
    const ASTComplexExpressionNode* LCEX =
      dynamic_cast<const ASTComplexExpressionNode*>(L);
    if (!LCEX) {
      std::stringstream M;
      M << "Failed to dynamic_cast to an ASTComplexExpressionNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(L), M.str(), DiagLevel::ICE);
      return;
    }

    Expand(LCEX, MOP);
  }
    break;
  default:
    break;
  }

  switch (R->GetASTType()) {
  case ASTTypeBinaryOp: {
    const ASTBinaryOpNode* RBOP = dynamic_cast<const ASTBinaryOpNode*>(R);
    if (!RBOP) {
      std::stringstream M;
      M << "Failed to dynamic_cast to an ASTBinaryOpNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(R), M.str(), DiagLevel::ICE);
      return;
    }

    Expand(RBOP, MOP);
  }
    break;
  case ASTTypeUnaryOp: {
    const ASTUnaryOpNode* RUOP = dynamic_cast<const ASTUnaryOpNode*>(R);
    if (!RUOP) {
      std::stringstream M;
      M << "Failed to dynamic_cast to an ASTUnaryOpNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(R), M.str(), DiagLevel::ICE);
      return;
    }

    Expand(RUOP, MOP);
  }
    break;
  case ASTTypeComplexExpression: {
    const ASTComplexExpressionNode* RCEX =
      dynamic_cast<const ASTComplexExpressionNode*>(R);
    if (!RCEX) {
      std::stringstream M;
      M << "Failed to dynamic_cast to an ASTComplexExpressionNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(R), M.str(), DiagLevel::ICE);
      return;
    }

    Expand(RCEX, MOP);
  }
    break;
  default:
    break;
  }

  if (MOP.insert(std::make_pair(OP, BOP)) == MOP.end()) {
    std::stringstream M;
    M << "Failure inserting ASTBinaryOpNode into the Expression Evaluation "
      << "Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(BOP), M.str(), DiagLevel::ICE);
  }
}

void
ASTOperatorPrecedenceController::Expand(const ASTUnaryOpNode* UOP,
                                        std::multimap<uint32_t,
                                                      ASTVariantOpNode>& MOP) const {
  assert(UOP && "Invalid ASTUnaryOpNode argument!");

  const ASTExpressionNode* R = UOP->GetExpression();
  uint32_t OP = GetOperatorPrecedence(UOP->GetOpType());
  assert(OP != static_cast<unsigned>(~0x0) &&
         "Invalid Operator Precedence Rank!");

  if (UOP->HasParens())
    OP -= RB;

  switch (R->GetASTType()) {
  case ASTTypeBinaryOp: {
    const ASTBinaryOpNode* RBOP = dynamic_cast<const ASTBinaryOpNode*>(R);
    if (!RBOP) {
      std::stringstream M;
      M << "Failed to dynamic_cast to an ASTBinaryOpNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(R), M.str(), DiagLevel::ICE);
      return;
    }

    Expand(RBOP, MOP);
  }
    break;
  case ASTTypeUnaryOp: {
    const ASTUnaryOpNode* RUOP = dynamic_cast<const ASTUnaryOpNode*>(R);
    if (!RUOP) {
      std::stringstream M;
      M << "Failed to dynamic_cast to an ASTUnaryOpNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(R), M.str(), DiagLevel::ICE);
      return;
    }

    Expand(RUOP, MOP);
  }
    break;
  case ASTTypeComplexExpression: {
    const ASTComplexExpressionNode* RCEX =
      dynamic_cast<const ASTComplexExpressionNode*>(R);
    if (!RCEX) {
      std::stringstream M;
      M << "Failed to dynamic_cast to an ASTComplexExpressionNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(R), M.str(), DiagLevel::ICE);
      return;
    }

    Expand(RCEX, MOP);
  }
    break;
  default:
    break;
  }

  if (MOP.insert(std::make_pair(OP, UOP)) == MOP.end()) {
    std::stringstream M;
    M << "Failure inserting ASTBinaryOpNode into the Expression Evaluation "
      << "Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(UOP), M.str(), DiagLevel::ICE);
  }
}

void
ASTOperatorPrecedenceController::Expand(const ASTComplexExpressionNode* CEX,
                                        std::multimap<uint32_t,
                                                      ASTVariantOpNode>& MOP) const {
  assert(CEX && "Invalid ASTComplexExpressionNode argument!");

  switch (CEX->GetExpressionType()) {
  case ASTTypeBinaryOp: {
    const ASTBinaryOpNode* BOP = CEX->GetBinaryOp();
    if (!BOP) {
      std::stringstream M;
      M << "Failure obtaining a valid ASTBinaryOpNode from "
        << "the ASTComplexExpressionNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(CEX), M.str(), DiagLevel::ICE);
      return;
    }

    Expand(BOP, MOP);
  }
    break;
  case ASTTypeUnaryOp: {
    const ASTUnaryOpNode* UOP = CEX->GetUnaryOp();
    if (!UOP) {
      std::stringstream M;
      M << "Failure obtaining a valid ASTUnaryOpNode from "
        << "the ASTComplexExpressionNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(CEX), M.str(), DiagLevel::ICE);
      return;
    }

    Expand(UOP, MOP);
  }
    break;
  default: {
    std::stringstream M;
    M << "Inconsistent construction of an ASTComplexExpressionNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(CEX), M.str(), DiagLevel::ICE);
    return;
  }
    break;
  }
}

void
ASTOperatorPrecedenceController::print(const std::multimap<uint32_t,
                                                  ASTVariantOpNode>& MOP) const {
  std::cout << "<OperatorPrecedenceMap>" << std::endl;
  if (!MOP.empty()) {
    for (std::multimap<uint32_t, ASTVariantOpNode>::const_iterator MI = MOP.begin();
         MI != MOP.end(); ++MI) {
      std::cout << "<Subexpression>" << std::endl;
      std::cout << "<PrecedenceRank>" << (*MI).first
        << "</PrecedenceRank>" << std::endl;

      switch ((*MI).second.index()) {
      case 0:
        std::get<0>((*MI).second)->print();
        break;
      case 1:
        std::get<1>((*MI).second)->print();
        break;
      case 2:
        std::get<2>((*MI).second)->print();
        break;
      default:
        std::cout << "[Invalid Subexpression] [" << (*MI).second.index()
          << "]" << std::endl;
        break;
      }

      std::cout << "</Subexpression>" << std::endl;
    }
  }

  std::cout << "</OperatorPrecedenceMap>" << std::endl;
}

} // namespace QASM

