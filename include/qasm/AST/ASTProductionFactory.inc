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

#ifndef __QASM_AST_PRODUCTION_FACTORY_CPP
#error "Do not use this translation unit directly."
#error "Use #include <ASTProductionFactory.h> instead."
#endif

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <sstream>

namespace QASM {

using DiagLevel = QasmDiagnosticEmitter::DiagLevel;

template<typename __Type>
ASTUnaryOpNode*
ASTProductionFactory::ProductionRule_300(const ASTToken* TK,
                                         const __Type* TP,
                                         ASTOpType OTy) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(TP && "Invalid template parameter argument!");

  if (TP->GetASTType() == ASTTypeIdentifier ||
      TP->GetASTType() == ASTTypeIdentifierRef) {
    if (const ASTIdentifierNode* TId = dynamic_cast<const ASTIdentifierNode*>(TP)) {
      ASTScopeController::Instance().CheckOutOfScope(TId);
      ASTIdentifierTypeController::Instance().CheckUndefinedType(TId);
    }
  }

  ASTOperandNode* OPX = ASTBuilder::Instance().CreateASTOperandNode(TP);
  assert(OPX && "Could not create a valid ASTOperandNode!");
  if (!OPX) {
    std::stringstream M;
    M << "Could not create a valid ASTOperandNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TP), M.str(), DiagLevel::ICE);
    return ASTUnaryOpNode::ExpressionError(M.str(), OTy);
  }

  OPX->SetLocation(TK->GetLocation());
  OPX->Mangle();

  std::stringstream UOI;
  UOI << "unary-op-" << PrintOpTypeOperator(OTy) << '-'
    << DIAGLineCounter::Instance().GetIdentifierLocation(TK);

  ASTUnaryOpNode* UOP =
    ASTBuilder::Instance().CreateASTUnaryOpNode(UOI.str(), OPX, OTy, true);
  assert(UOP && "Could not create a valid ASTUnaryOpNode!");
  if (!UOP) {
    std::stringstream M;
    M << "Could not create a valid ASTUnaryOpNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TP), M.str(), DiagLevel::ICE);
    return ASTUnaryOpNode::ExpressionError(M.str(), OTy);
  }

  UOP->SetLocation(TK->GetLocation());
  UOP->Mangle();
  return UOP;
}

template<typename __Base, typename __Exponent>
ASTBinaryOpNode*
ASTProductionFactory::ProductionRule_310(const ASTToken* TK,
                                         const __Base* B,
                                         const __Exponent* E) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(B && "Invalid Base template parameter argument!");
  assert(E && "Invalid Exponent template parameter argument!");

  ASTType BTy = ASTTypeUndefined;
  ASTType ETy = ASTTypeUndefined;

  if (B->GetASTType() == ASTTypeIdentifier ||
      B->GetASTType() == ASTTypeIdentifierRef) {
    if (const ASTIdentifierNode* TId = dynamic_cast<const ASTIdentifierNode*>(B)) {
      BTy = TId->GetSymbolType();
      ASTScopeController::Instance().CheckOutOfScope(TId);
      ASTIdentifierTypeController::Instance().CheckUndefinedType(TId);
    } else {
      std::stringstream M;
      M << "Could not dynamic_cast to an ASTIdentifierNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(B), M.str(), DiagLevel::ICE);
      return ASTBinaryOpNode::ExpressionError(M.str(), ASTOpTypePow);
    }
  }

  if (E->GetASTType() == ASTTypeIdentifier ||
      E->GetASTType() == ASTTypeIdentifierRef) {
    if (const ASTIdentifierNode* TId = dynamic_cast<const ASTIdentifierNode*>(E)) {
      ETy = TId->GetSymbolType();
      ASTScopeController::Instance().CheckOutOfScope(TId);
      ASTIdentifierTypeController::Instance().CheckUndefinedType(TId);
    } else {
      std::stringstream M;
      M << "Could not dynamic_cast to an ASTIdentifierNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(E), M.str(), DiagLevel::ICE);
      return ASTBinaryOpNode::ExpressionError(M.str(), ASTOpTypePow);
    }
  }

  if (BTy == ASTTypeUndefined) {
    switch (B->GetASTType()) {
    case ASTTypeBinaryOp:
      if (const ASTBinaryOpNode* BOP = dynamic_cast<const ASTBinaryOpNode*>(B))
        BTy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
      break;
    case ASTTypeUnaryOp:
      if (const ASTUnaryOpNode* UOP = dynamic_cast<const ASTUnaryOpNode*>(B))
        BTy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
      break;
    break;
    case ASTTypeCast:
      if (const ASTCastExpressionNode* CXE =
          dynamic_cast<const ASTCastExpressionNode*>(B))
        BTy = ASTExpressionEvaluator::Instance().EvaluatesTo(CXE);
      break;
    default:
      BTy = B->GetASTType();
      break;
    }
  }

  if (BTy == ASTTypeUndefined) {
    std::stringstream M;
    M << "An " << PrintTypeEnum(B->GetASTType()) << " type cannot be "
      << "used as a base operand for a pow arithmetic expression.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(E), M.str(), DiagLevel::Error);
    return ASTBinaryOpNode::ExpressionError(M.str(), ASTOpTypePow);
  }

  if (ETy == ASTTypeUndefined) {
    switch (E->GetASTType()) {
    case ASTTypeBinaryOp:
      if (const ASTBinaryOpNode* BOP = dynamic_cast<const ASTBinaryOpNode*>(E))
        ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
      break;
    case ASTTypeUnaryOp:
      if (const ASTUnaryOpNode* UOP = dynamic_cast<const ASTUnaryOpNode*>(E))
        ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
      break;
    case ASTTypeCast:
      if (const ASTCastExpressionNode* CXE =
          dynamic_cast<const ASTCastExpressionNode*>(E))
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(CXE);
      break;
    default:
      ETy = E->GetASTType();
      break;
    }
  }

  if (ETy == ASTTypeUndefined) {
    std::stringstream M;
    M << "An " << PrintTypeEnum(E->GetASTType()) << " type cannot be "
      << "used as an exponent operand for a pow arithmetic expression.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(E), M.str(), DiagLevel::Error);
    return ASTBinaryOpNode::ExpressionError(M.str(), ASTOpTypePow);
  }

  bool CVP = false;
  const ASTType CVTy = ASTTypeMPDecimal;

  if (!ASTExpressionValidator::Instance().IsUnPromotedIntegerType(BTy) &&
      !ASTExpressionValidator::Instance().IsFloatingPointType(BTy)) {
    if (ASTTypeCastController::Instance().CanImplicitConvert(BTy, CVTy)) {
      CVP = true;
    }
  } else {
    CVP = true;
  }

  if (!CVP) {
    std::stringstream M;
    M << "No possible implicit conversion was found from "
      << PrintTypeEnum(BTy) << " to an " << PrintTypeEnum(CVTy) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(E), M.str(), DiagLevel::Error);
    return ASTBinaryOpNode::ExpressionError(M.str(), ASTOpTypePow);
  }

  CVP = false;

  if (!ASTExpressionValidator::Instance().IsUnPromotedIntegerType(ETy) &&
      !ASTExpressionValidator::Instance().IsFloatingPointType(ETy)) {
    if (ASTTypeCastController::Instance().CanImplicitConvert(ETy, CVTy)) {
      CVP = true;
    }
  } else {
    CVP = true;
  }

  if (!CVP) {
    std::stringstream M;
    M << "No possible implicit conversion was found from "
      << PrintTypeEnum(ETy) << " to an " << PrintTypeEnum(CVTy) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(E), M.str(), DiagLevel::Error);
    return ASTBinaryOpNode::ExpressionError(M.str(), ASTOpTypePow);
  }

  const ASTImplicitConversionNode* BCIX = nullptr;
  const ASTImplicitConversionNode* ECIX = nullptr;

  if (BTy != ASTTypeMPDecimal) {
    BCIX = new ASTImplicitConversionNode(B, ASTTypeMPDecimal);
    assert(BCIX && "Could not create a valid ASTImplicitConversionNode!");
    if (!BCIX) {
      std::stringstream M;
      M << "Could not create a valid ASTImplicitConversionNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(B), M.str(), DiagLevel::Error);
      return ASTBinaryOpNode::ExpressionError(M.str(), ASTOpTypePow);
    }

    const_cast<ASTImplicitConversionNode*>(BCIX)->SetLocation(B->GetLocation());
    const_cast<ASTImplicitConversionNode*>(BCIX)->Mangle();
  }

  if (ETy != ASTTypeMPDecimal) {
    ECIX = new ASTImplicitConversionNode(E, ASTTypeMPDecimal);
    assert(ECIX && "Could not create a valid ASTImplicitConversionNode!");
    if (!ECIX) {
      std::stringstream M;
      M << "Could not create a valid ASTImplicitConversionNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(E), M.str(), DiagLevel::Error);
      return ASTBinaryOpNode::ExpressionError(M.str(), ASTOpTypePow);
    }

    const_cast<ASTImplicitConversionNode*>(ECIX)->SetLocation(E->GetLocation());
    const_cast<ASTImplicitConversionNode*>(ECIX)->Mangle();
  }

  const ASTOperandNode* BXOP = nullptr;

  if (BCIX)
    BXOP = ASTBuilder::Instance().CreateASTOperandNode(BCIX);
  else
    BXOP = ASTBuilder::Instance().CreateASTOperandNode(B);
  assert(BXOP && "Could not create a valid ASTOperandNode!");
  if (!BXOP) {
    std::stringstream M;
    M << "Could not create a valid ASTOperandNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(B), M.str(), DiagLevel::Error);
    return ASTBinaryOpNode::ExpressionError(M.str(), ASTOpTypePow);
  }

  const_cast<ASTOperandNode*>(BXOP)->SetLocation(B->GetLocation());
  const_cast<ASTOperandNode*>(BXOP)->Mangle();

  const ASTOperandNode* EXOP = nullptr;

  if (ECIX)
    EXOP = ASTBuilder::Instance().CreateASTOperandNode(ECIX);
  else
    EXOP = ASTBuilder::Instance().CreateASTOperandNode(E);
  assert(EXOP && "Could not create a valid ASTOperandNode!");
  if (!EXOP) {
    std::stringstream M;
    M << "Could not create a valid ASTOperandNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(E), M.str(), DiagLevel::Error);
    return ASTBinaryOpNode::ExpressionError(M.str(), ASTOpTypePow);
  }

  const_cast<ASTOperandNode*>(EXOP)->SetLocation(E->GetLocation());
  const_cast<ASTOperandNode*>(EXOP)->Mangle();

  std::stringstream BOS;
  BOS << "ast-binary-op-" << PrintDashOpTypeEnum(ASTOpTypePow) << '-'
    << DIAGLineCounter::Instance().GetIdentifierLocation(TK);

  ASTBinaryOpNode* BOP =
    ASTBuilder::Instance().CreateASTBinaryOpNode(BOS.str(), BXOP, EXOP, ASTOpTypePow);
  assert(BOP && "Could not create a valid ASTBinaryOpNode!");
  if (!BOP) {
    std::stringstream M;
    M << "Could not create a valid ASTBinaryOpNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::Error);
    return ASTBinaryOpNode::ExpressionError(M.str(), ASTOpTypePow);
  }

  BOP->SetLocation(TK->GetLocation());
  BOP->Mangle();
  return BOP;
}

template<typename __Type>
ASTUnaryOpNode*
ASTProductionFactory::ProductionRule_340(const ASTToken* TK,
                                         const __Type* TP) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(TP && "Invalid template parameter argument!");

  if (!ASTExpressionValidator::Instance().IsLogicallyNegateType(TP)) {
    std::stringstream M;
    M << "Nonsensical logical negation.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::Error);
    return ASTUnaryOpNode::ExpressionError(M.str(), ASTOpTypeLogicalNot);
  }

  std::stringstream ES;
  ES << "unary-op-bool-neg-"
    << DIAGLineCounter::Instance().GetIdentifierLocation(TK);

  ASTOperatorNode* OPX =
    ASTBuilder::Instance().CreateASTOperatorNode(TP, ASTOpTypeLogicalNot);
  assert(OPX && "Could not create a valid ASTOperatorNode!");
  if (!OPX) {
    std::stringstream M;
    M << "Could not create a valid ASTOperatorNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::Error);
    return ASTUnaryOpNode::ExpressionError(M.str(), ASTOpTypeLogicalNot);
  }

  OPX->SetLocation(TK->GetLocation());
  OPX->Mangle();

  ASTUnaryOpNode* UOP =
    ASTBuilder::Instance().CreateASTUnaryOpNode(ES.str(), OPX,
                                                ASTOpTypeLogicalNot, true);
  assert(UOP && "Could not create a valid ASTUnaryOpNode!");
  if (!UOP) {
    std::stringstream M;
    M << "Could not create a valid ASTUnaryOpNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::Error);
    return ASTUnaryOpNode::ExpressionError(M.str(), ASTOpTypeLogicalNot);
  }

  UOP->SetLocation(TK->GetLocation());
  UOP->Mangle();
  return UOP;
}

template<typename __Type>
ASTUnaryOpNode*
ASTProductionFactory::ProductionRule_350(const ASTToken *TK, const __Type* TP,
                                         ASTOpType OTy, bool Parens) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(TP && "Invalid template parameter type argument!");

  assert((OTy == ASTOpTypeNegative || OTy == ASTOpTypePositive) &&
         "Invalid Operand type!");
  if (OTy != ASTOpTypeNegative && OTy != ASTOpTypePositive) {
    std::stringstream M;
    M << "Only arithmetic positive or arithmetic negative ops are allowed.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
    return ASTUnaryOpNode::ExpressionError(M.str(), OTy);
  }

  ASTType ETy;

  switch (TP->GetASTType()) {
  case ASTTypeIdentifier: {
    if (const ASTIdentifierNode* Id = dynamic_cast<const ASTIdentifierNode*>(TP))
      ETy = Id->GetSymbolType();
    else
      ETy = ASTTypeUndefined;
  }
    break;
  case ASTTypeIdentifierRef: {
    if (const ASTIdentifierRefNode* IdR =
        dynamic_cast<const ASTIdentifierRefNode*>(TP))
      ETy = IdR->GetSymbolType();
    else
      ETy = ASTTypeUndefined;
  }
    break;
  case ASTTypeBinaryOp: {
    if (const ASTBinaryOpNode* BOP = dynamic_cast<const ASTBinaryOpNode*>(TP))
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
    else
      ETy = ASTTypeUndefined;
  }
    break;
  case ASTTypeUnaryOp: {
    if (const ASTUnaryOpNode* UOP = dynamic_cast<const ASTUnaryOpNode*>(TP))
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
    else
      ETy = ASTTypeUndefined;
  }
    break;
  case ASTTypeCast: {
    if (const ASTCastExpressionNode* CXT =
        dynamic_cast<const ASTCastExpressionNode*>(TP))
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(CXT);
    else
      ETy = ASTTypeUndefined;
  }
    break;
  case ASTTypeImplicitConversion: {
    if (const ASTImplicitConversionNode* ICX =
        dynamic_cast<const ASTImplicitConversionNode*>(TP))
      ETy = ASTExpressionEvaluator::Instance().EvaluatesTo(ICX);
    else
      ETy = ASTTypeUndefined;
  }
    break;
    case ASTTypeFunctionCall: {
      if (const ASTFunctionCallNode* FCN =
          dynamic_cast<const ASTFunctionCallNode*>(TP))
        ETy = FCN->GetResultType();
      else
        ETy = ASTTypeUndefined;
    }
    break;
  default:
    ETy = TP->GetASTType();
    break;
  }

  if (!ASTExpressionValidator::Instance().CanDoArithmeticNegPos(ETy)) {
    std::stringstream M;
    M << "Type " << TP->GetASTType() << " cannot be an operand to "
      << "arithmetic negative / positive.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
    return ASTUnaryOpNode::ExpressionError(M.str(), OTy);
  }

  ASTOperandNode* OPN = new ASTOperandNode(TP);
  assert(OPN && "Could not create a valid ASTOperandNode!");
  if (!OPN) {
    std::stringstream M;
    M << "Could not create a valid ASTOperandNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
    return ASTUnaryOpNode::ExpressionError(M.str(), OTy);
  }

  OPN->SetLocation(TK->GetLocation());
  OPN->Mangle();

  std::stringstream OS;
  OS << "ast-unary-op-arith-" << PrintDashOpTypeEnum(OTy) << '-'
    << DIAGLineCounter::Instance().GetIdentifierLocation(TK);

  ASTUnaryOpNode* UOP =
    ASTBuilder::Instance().CreateASTUnaryOpNode(OS.str(), OPN, OTy, Parens);
  assert(UOP && "Could not create a valid ASTUnaryOpNode!");
  if (!UOP) {
    std::stringstream M;
    M << "Could not create a valid ASTUnaryOpNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
    return ASTUnaryOpNode::ExpressionError(M.str(), OTy);
  }

  UOP->SetLocation(TK->GetLocation());
  UOP->Mangle();
  return UOP;
}

template<typename __Type>
ASTDeclarationNode*
ASTProductionFactory::ProductionRule_3800(const ASTToken* TK, const __Type* TP,
                                          ASTType Ty, bool Const) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(TP && "Invalid template parameter argument!");

  ASTDeclarationNode* DN = new ASTDeclarationNode(TP->GetIdentifier(), TP, Ty);
  if (!DN) {
    std::stringstream M;
    M << "Failure creating an ASTDeclarationNode Parameter.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TP), M.str(), DiagLevel::ICE);
    return ASTDeclarationNode::DeclarationError(TP->GetIdentifier(), M.str());
  }

  DN->SetLocation(TK->GetLocation());
  DN->SetConst(Const);
  return DN;
}

template<typename __Type>
ASTGateControlNode*
ASTProductionFactory::ProductionRule_3850(const ASTToken* TK,
                                          const __Type* TP) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(TP && "Invalid template parameter argument!");

  ASTGateControlNode* GCN = new ASTGateControlNode(TP);
  assert(GCN && "Could not create a valid ASTGateControlNode!");

  if (!GCN) {
    std::stringstream M;
    M << "Could not create a valid ASTGateControlNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TP), M.str(), DiagLevel::ICE);
    return ASTGateControlNode::ExpressionError(M.str());
  }

  GCN->SetLocation(TK->GetLocation());
  GCN->Mangle();
  return GCN;
}

template<typename __Type>
ASTGateControlNode*
ASTProductionFactory::ProductionRule_3850(const ASTToken* TK,
                                          const __Type* TP,
                                          const ASTIntNode* I) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(TP && "Invalid template parameter argument!");
  assert(I && "Invalid ASTIntNode argument!");

  ASTGateControlNode* GCN = new ASTGateControlNode(TP, I);
  assert(GCN && "Could not create a valid ASTGateControlNode!");

  if (!GCN) {
    std::stringstream M;
    M << "Could not create a valid ASTGateControlNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TP), M.str(), DiagLevel::ICE);
    return ASTGateControlNode::ExpressionError(M.str());
  }

  GCN->SetLocation(TK->GetLocation());
  GCN->Mangle();
  return GCN;
}

template<typename __Type>
ASTGateControlNode*
ASTProductionFactory::ProductionRule_3850(const ASTToken* TK,
                                          const ASTGateQOpNode* GQN) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(GQN && "Invalid ASTGateQOpNode argument!");

  ASTGateControlNode* GCN = new ASTGateControlNode(GQN);
  assert(GCN && "Could not create a valid ASTGateControlNode!");

  if (!GCN) {
    std::stringstream M;
    M << "Could not create a valid ASTGateControlNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
    return ASTGateControlNode::ExpressionError(M.str());
  }

  GCN->SetLocation(TK->GetLocation());
  GCN->Mangle();
  const_cast<ASTGateQOpNode*>(GQN)->SetModifier(GCN);
  return GCN;
}

template<typename __Type>
ASTGateControlNode*
ASTProductionFactory::ProductionRule_3850(const ASTToken* TK,
                                          const ASTGateQOpNode* GQN,
                                          const ASTIntNode* I) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(I && "Invalid ASTIntNode argument!");
  assert(GQN && "Invalid ASTGateQOpNode argument!");

  ASTGateControlNode* GCN = new ASTGateControlNode(GQN, I);
  assert(GCN && "Could not create a valid ASTGateControlNode!");

  if (!GCN) {
    std::stringstream M;
    M << "Could not create a valid ASTGateControlNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
    return ASTGateControlNode::ExpressionError(M.str());
  }

  GCN->SetLocation(TK->GetLocation());
  GCN->Mangle();
  const_cast<ASTGateQOpNode*>(GQN)->SetModifier(GCN);
  return GCN;
}

template<typename __Type>
ASTGateNegControlNode*
ASTProductionFactory::ProductionRule_3851(const ASTToken* TK,
                                          const __Type* TP) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(TP && "Invalid template parameter argument!");

  ASTGateNegControlNode* GNCN = new ASTGateNegControlNode(TP);
  assert(GNCN && "Could not create a valid ASTGateNegControlNode!");

  if (!GNCN) {
    std::stringstream M;
    M << "Could not create a valid ASTGateNegControlNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TP), M.str(), DiagLevel::ICE);
    return ASTGateNegControlNode::ExpressionError(M.str());
  }

  GNCN->SetLocation(TK->GetLocation());
  GNCN->Mangle();
  return GNCN;
}

template<typename __Type>
ASTGateNegControlNode*
ASTProductionFactory::ProductionRule_3851(const ASTToken* TK,
                                          const __Type* TP,
                                          const ASTIntNode* I) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(TP && "Invalid template parameter argument!");
  assert(I && "Invalid ASTIntNode argument!");

  ASTGateNegControlNode* GNCN = new ASTGateNegControlNode(TP, I);
  assert(GNCN && "Could not create a valid ASTGateNegControlNode!");

  if (!GNCN) {
    std::stringstream M;
    M << "Could not create a valid ASTGateNegControlNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TP), M.str(), DiagLevel::ICE);
    return ASTGateNegControlNode::ExpressionError(M.str());
  }

  GNCN->SetLocation(TK->GetLocation());
  GNCN->Mangle();
  return GNCN;
}

template<typename __Type>
ASTGateNegControlNode*
ASTProductionFactory::ProductionRule_3851(const ASTToken* TK,
                                          const ASTGateQOpNode* GQN) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(GQN && "Invalid ASTGateQOpNode argument!");

  ASTGateNegControlNode* GNCN = new ASTGateNegControlNode(GQN);
  assert(GNCN && "Could not create a valid ASTGateNegControlNode!");

  if (!GNCN) {
    std::stringstream M;
    M << "Could not create a valid ASTGateNegControlNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
    return ASTGateNegControlNode::ExpressionError(M.str());
  }

  GNCN->SetLocation(TK->GetLocation());
  GNCN->Mangle();
  const_cast<ASTGateQOpNode*>(GQN)->SetModifier(GNCN);
  return GNCN;
}

template<typename __Type>
ASTGateNegControlNode*
ASTProductionFactory::ProductionRule_3851(const ASTToken* TK,
                                          const ASTGateQOpNode* GQN,
                                          const ASTIntNode* I) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(GQN && "Invalid ASTGateQOpNode argument!");

  ASTGateNegControlNode* GNCN = new ASTGateNegControlNode(GQN, I);
  assert(GNCN && "Could not create a valid ASTGateNegControlNode!");

  if (!GNCN) {
    std::stringstream M;
    M << "Could not create a valid ASTGateNegControlNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
    return ASTGateNegControlNode::ExpressionError(M.str());
  }

  GNCN->SetLocation(TK->GetLocation());
  GNCN->Mangle();
  const_cast<ASTGateQOpNode*>(GQN)->SetModifier(GNCN);
  return GNCN;
}

template<typename __Type, typename __OpType>
ASTGatePowerNode*
ASTProductionFactory::ProductionRule_3852(const ASTToken* TK,
                                          const __Type* TP,
                                          const __OpType* OTP) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(TP && "Invalid template parameter argument!");
  assert(OTP && "Invalid template parameter argument!");

  ASTGatePowerNode* GPN = new ASTGatePowerNode(TP, OTP);
  assert(GPN && "Could not create a valid ASTGatePowerNode!");

  if (!GPN) {
    std::stringstream M;
    M << "Could not create a valid ASTGatePowerNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TP), M.str(), DiagLevel::ICE);
    return ASTGatePowerNode::ExpressionError(M.str());
  }

  GPN->SetLocation(TK->GetLocation());
  GPN->Mangle();
  return GPN;
}

template<typename __OpType>
ASTGatePowerNode*
ASTProductionFactory::ProductionRule_3852(const ASTToken* TK,
                                          const ASTGateQOpNode* GQN,
                                          const __OpType* OTP) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(GQN && "Invalid ASTGateQOpNode argument!");
  assert(OTP && "Invalid template parameter argument!");

  ASTGatePowerNode* GPN = new ASTGatePowerNode(GQN->GetGateNode(), OTP);
  assert(GPN && "Could not create a valid ASTGatePowerNode!");

  if (!GPN) {
    std::stringstream M;
    M << "Could not create a valid ASTGatePowerNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
    return ASTGatePowerNode::ExpressionError(M.str());
  }

  GPN->SetLocation(TK->GetLocation());
  GPN->Mangle();
  const_cast<ASTGateQOpNode*>(GQN)->SetModifier(GPN);
  return GPN;
}

template<typename __Type>
ASTGateInverseNode*
ASTProductionFactory::ProductionRule_3853(const ASTToken* TK,
                                          const __Type* TP) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(TP && "Invalid template parameter argument!");

  ASTGateInverseNode* GIN = new ASTGateInverseNode(TP);
  assert(GIN && "Could not create a valid ASTGateInverseNode!");

  if (!GIN) {
    std::stringstream M;
    M << "Could not create a valid ASTGateInverseNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TP), M.str(), DiagLevel::ICE);
    return ASTGateInverseNode::ExpressionError(M.str());
  }

  GIN->SetLocation(TK->GetLocation());
  GIN->Mangle();
  return GIN;
}

template<typename __Type>
ASTGateInverseNode*
ASTProductionFactory::ProductionRule_3853(const ASTToken* TK,
                                          const ASTGateQOpNode* GQN) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(GQN && "Invalid ASTGateQOpNode argument!");

  ASTGateInverseNode* GIN = new ASTGateInverseNode(GQN);
  assert(GIN && "Could not create a valid ASTGateInverseNode!");

  if (!GIN) {
    std::stringstream M;
    M << "Could not create a valid ASTGateInverseNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::ICE);
    return ASTGateInverseNode::ExpressionError(M.str());
  }

  GIN->SetLocation(TK->GetLocation());
  GIN->Mangle();
  const_cast<ASTGateQOpNode*>(GQN)->SetModifier(GIN);
  return GIN;
}

template<typename __Type>
ASTStretchNode*
ASTProductionFactory::ProductionRule_1350(const ASTToken* TK,
                                          const ASTIdentifierNode* Id,
                                          const __Type* TP) const {
  assert(TK && "Invalid ASTToken argument!");
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(TP && "Invalid template parameter argument!");

  if (!ASTSymbolTable::Instance().TransferUndefinedSymbol(Id,
                                                  ASTStretchNode::StretchBits,
                                                  ASTTypeStretch)) {
    std::stringstream M;
    M << "Could not transfer Symbol Table Entry for "
      << PrintTypeEnum(Id->GetSymbolType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    return ASTStretchNode::ExpressionError(Id, M.str());
  }

  if (ASTDeclarationBuilder::Instance().DeclAlreadyExists(Id)) {
    std::stringstream M;
    M << "Declaration " << Id->GetName() << " shadows a "
      << "previous declaration.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return ASTStretchNode::ExpressionError(Id, M.str());
  }

  unsigned SV = 0U;

  switch (TP->GetASTType()) {
  case ASTTypeInt: {
    if (const ASTIntNode* II = dynamic_cast<const ASTIntNode*>(TP)) {
      SV = II->IsSigned() ?
           static_cast<unsigned>(II->GetSignedValue()) :
           II->GetUnsignedValue();
    } else {
      SV = static_cast<unsigned>(~0x0);
    }
  }
    break;
  case ASTTypeFloat: {
    if (const ASTFloatNode* FF = dynamic_cast<const ASTFloatNode*>(TP)) {
      SV = static_cast<unsigned>(FF->GetValue());
    } else {
      SV = static_cast<unsigned>(~0x0);
    }
  }
    break;
  case ASTTypeDouble: {
    if (const ASTDoubleNode* DD = dynamic_cast<const ASTDoubleNode*>(TP)) {
      SV = static_cast<unsigned>(DD->GetValue());
    } else {
      SV = static_cast<unsigned>(~0x0);
    }
  }
    break;
  case ASTTypeMPInteger: {
    if (const ASTMPIntegerNode* MPI = dynamic_cast<const ASTMPIntegerNode*>(TP)) {
      SV = static_cast<unsigned>(MPI->ToUnsignedInt());
    } else {
      SV = static_cast<unsigned>(~0x0);
    }
  }
    break;
  case ASTTypeMPDecimal: {
    if (const ASTMPDecimalNode* MPD = dynamic_cast<const ASTMPDecimalNode*>(TP)) {
      SV = static_cast<unsigned>(MPD->ToMPInteger()->ToUnsignedInt());
    } else {
      SV = static_cast<unsigned>(~0x0);
    }
  }
    break;
  default:
    break;
  }

  if (SV == static_cast<unsigned>(~0x0)) {
    std::stringstream M;
    M << "Invalid stretch value.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return ASTStretchNode::ExpressionError(Id, M.str());
  } else if (SV > ASTStretchNode::MaxValue) {
    std::stringstream M;
    M << "Stretch value cannot exceed 255.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return ASTStretchNode::ExpressionError(Id, M.str());
  }

  ASTStretchNode* SN = ASTBuilder::Instance().CreateASTStretchNode(Id, TP);
  assert(SN && "Could not create a valid ASTStretchNode!");
  if (!SN) {
    std::stringstream M;
    M << "Could not create a valid ASTStretchNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    return ASTStretchNode::ExpressionError(Id, M.str());
  }

  SN->SetDuration(SV);
  SN->SetLengthUnit(LengthUnspecified);
  SN->SetLocation(TK->GetLocation());
  SN->Mangle();

  ASTDeclarationNode* DSN =
    new ASTDeclarationNode(Id, SN, ASTTypeStretch, false);
  assert(DSN && "Could not create a valid Stretch ASTDeclarationNode!");
  if (!DSN) {
    std::stringstream M;
    M << "Could not create a valid Stretch ASTDeclarationNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    return ASTStretchNode::ExpressionError(Id, M.str());
  }

  ASTDeclarationBuilder::Instance().Append(DSN);

  ASTStretchStatementNode* SSN = new ASTStretchStatementNode(SN);
  assert(SSN && "Could not create a valid ASTStretchStatementNode!");
  if (!SSN) {
    std::stringstream M;
    M << "Could not create a valid ASTStretchStatementNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    return ASTStretchNode::ExpressionError(Id, M.str());
  }

  ASTStatementBuilder::Instance().Append(SSN);
  return SN;
}

} // namespace QASM

