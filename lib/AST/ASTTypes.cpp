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

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTPragma.h>
#include <qasm/AST/ASTReset.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTGates.h>
#include <qasm/AST/ASTGPhase.h>
#include <qasm/AST/ASTPopcount.h>
#include <qasm/AST/ASTRotate.h>
#include <qasm/AST/ASTCastExpr.h>
#include <qasm/AST/ASTImplicitConversionExpr.h>
#include <qasm/AST/ASTExpressionEvaluator.h>
#include <qasm/AST/ASTStringUtils.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <string>
#include <sstream>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTFloatNode* ASTFloatNode::DPi = nullptr;
ASTFloatNode* ASTFloatNode::DNegPi = nullptr;
ASTFloatNode* ASTFloatNode::DTau = nullptr;
ASTFloatNode* ASTFloatNode::DNegTau = nullptr;
ASTFloatNode* ASTFloatNode::DEuler = nullptr;
ASTFloatNode* ASTFloatNode::DNegEuler = nullptr;

ASTDoubleNode* ASTDoubleNode::DPi = nullptr;
ASTDoubleNode* ASTDoubleNode::DNegPi = nullptr;
ASTDoubleNode* ASTDoubleNode::DTau = nullptr;
ASTDoubleNode* ASTDoubleNode::DNegTau = nullptr;
ASTDoubleNode* ASTDoubleNode::DEuler = nullptr;
ASTDoubleNode* ASTDoubleNode::DNegEuler = nullptr;

ASTLongDoubleNode* ASTLongDoubleNode::DPi = nullptr;
ASTLongDoubleNode* ASTLongDoubleNode::DNegPi = nullptr;
ASTLongDoubleNode* ASTLongDoubleNode::DTau = nullptr;
ASTLongDoubleNode* ASTLongDoubleNode::DNegTau = nullptr;
ASTLongDoubleNode* ASTLongDoubleNode::DEuler = nullptr;
ASTLongDoubleNode* ASTLongDoubleNode::DNegEuler = nullptr;

ASTStringNode* ASTStringNode::TN = new ASTStringNode("true");
ASTStringNode* ASTStringNode::FN = new ASTStringNode("false");

ASTBoolNode* ASTBoolNode::TN = new ASTBoolNode(true);
ASTBoolNode* ASTBoolNode::FN = new ASTBoolNode(false);

ASTInputModifierNode ASTInputModifierNode::IM;
ASTOutputModifierNode ASTOutputModifierNode::OM;

ASTExpressionNode*
ASTExpressionNode::ExpressionError(const ASTIdentifierNode* Id,
                                   const std::string& ERM) {
  ASTStringNode* SN = new ASTStringNode(ERM);
  assert(SN && "Could not create a valid ASTStringNode!");

  ASTExpressionNode* ER = new ASTExpressionNode(Id, SN, ASTTypeExpressionError);
  assert(ER && "Could not create a valid ASTExpressionNode!");
  ER->SetLocation(Id->GetLocation());
  return ER;
}

ASTExpressionNode*
ASTExpressionNode::ExpressionError(const ASTIdentifierRefNode* IdR,
                                   const std::string& ERM) {
  ASTStringNode* SN = new ASTStringNode(ERM);
  assert(SN && "Could not create a valid ASTStringNode!");

  ASTExpressionNode* ER = new ASTExpressionNode(IdR, SN, ASTTypeExpressionError);
  assert(ER && "Could not create a valid ASTExpressionNode!");
  ER->SetLocation(IdR->GetLocation());
  return ER;
}

const std::string&
ASTExpressionNode::GetError() const {
  if (Type == ASTTypeExpressionError) {
    if (const ASTStringNode* SN = dynamic_cast<const ASTStringNode*>(Expr))
      return SN->GetValue();
  }

  return ASTStringUtils::Instance().EmptyString();
}

void ASTExpressionNode::print() const {
  std::cout << "<ASTExpressionNode>" << std::endl;
  std::cout << "<Type>" << QASM::PrintTypeEnum(Type) << "</Type>"
    << std::endl;
  std::cout << "<ExpressionType>" << PrintExpressionType(EXTy)
    << "</ExpressionType>" << std::endl;

  print_qualifiers();

  if (IsIdentifier())
    Ident->print();
  else if (IsStatement())
    Stmt->print();
  else if (IsExpression())
    Expr->print();

  if (HasInductionVariable()) {
    std::cout << "<InductionVariable>" << std::endl;
    std::cout << "<Name>" << IndVar->GetName() << "</Name>" << std::endl;
    std::cout << "</InductionVariable>" << std::endl;
  }

  if (HasIndexIdentifier()) {
    std::cout << "<IndexIdentifier>" << std::endl;
    std::cout << "<Name>" << IxInd->GetName() << "</Name>" << std::endl;
    std::cout << "</IndexIdentifier>" << std::endl;
  }

  std::cout << "</ASTExpressionNode>" << std::endl;
}

ASTType ASTOperatorNode::GetEvaluatedTargetType() const {
  if (IsIdentifier())
    return TId->GetSymbolType();

  switch (TTy) {
  case ASTTypeBinaryOp: {
    if (const ASTBinaryOpNode* BOP = dynamic_cast<const ASTBinaryOpNode*>(TEx))
      return ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
  }
    break;
  case ASTTypeUnaryOp: {
    if (const ASTUnaryOpNode* UOP = dynamic_cast<const ASTUnaryOpNode*>(TEx))
      return ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
  }
    break;
  case ASTTypeCast: {
    if (const ASTCastExpressionNode* CXT =
        dynamic_cast<const ASTCastExpressionNode*>(TEx))
      return ASTExpressionEvaluator::Instance().EvaluatesTo(CXT);
  }
    break;
  case ASTTypeImplicitConversion: {
    if (const ASTImplicitConversionNode* ICX =
        dynamic_cast<const ASTImplicitConversionNode*>(TEx))
      return ASTExpressionEvaluator::Instance().EvaluatesTo(ICX);
  }
    break;
  default:
    break;
  }

  return TTy;
}

ASTType ASTOperandNode::GetEvaluatedTargetType() const {
  if (IsIdentifier())
    return TId->GetSymbolType();

  switch (TTy) {
  case ASTTypeBinaryOp: {
    if (const ASTBinaryOpNode* BOP = dynamic_cast<const ASTBinaryOpNode*>(TEx))
      return ASTExpressionEvaluator::Instance().EvaluatesTo(BOP);
  }
    break;
  case ASTTypeUnaryOp: {
    if (const ASTUnaryOpNode* UOP = dynamic_cast<const ASTUnaryOpNode*>(TEx))
      return ASTExpressionEvaluator::Instance().EvaluatesTo(UOP);
  }
    break;
  case ASTTypeCast: {
    if (const ASTCastExpressionNode* CXT =
        dynamic_cast<const ASTCastExpressionNode*>(TEx))
      return ASTExpressionEvaluator::Instance().EvaluatesTo(CXT);
  }
    break;
  case ASTTypeImplicitConversion: {
    if (const ASTImplicitConversionNode* ICX =
        dynamic_cast<const ASTImplicitConversionNode*>(TEx))
      return ASTExpressionEvaluator::Instance().EvaluatesTo(ICX);
  }
    break;
  default:
    break;
  }

  return TTy;
}

void ASTBoolNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

const ASTMPIntegerNode* ASTIntNode::GetMPInteger() const {
  return (Expr && (Expr->GetASTType() == ASTTypeMPInteger ||
                   Expr->GetASTType() == ASTTypeMPUInteger)) ?
          dynamic_cast<const ASTMPIntegerNode*>(Expr) :
          nullptr;
}

ASTMPIntegerNode* ASTIntNode::AsMPInteger(unsigned W) const {
  ASTMPIntegerNode* MPI = new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(),
                                               Signbit, W);
  assert(MPI && "Could not create a valid ASTMPIntegerNode!");

  MPI->InitFromString(GetString().c_str(), Signbit, W, 0);
  return MPI;
}

unsigned ASTIntNode::Size() const {
  if (Expr) {
    if (const ASTMPIntegerNode* MPI = dynamic_cast<const ASTMPIntegerNode*>(Expr))
      return MPI->GetBits() / CHAR_BIT;
  }

  return Bits / CHAR_BIT;
}

unsigned ASTIntNode::GetBits() const {
  if (Expr) {
    if (const ASTMPIntegerNode* MPI = dynamic_cast<const ASTMPIntegerNode*>(Expr))
      return MPI->GetBits();
  }

  return Bits;
}

void ASTIntNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (IsSigned()) {
    if (!GetIdentifier()->GetPolymorphicName().empty())
      M.TypeIdentifier(ASTTypeInt,
                       GetIdentifier()->GetPolymorphicName());
    else
      M.TypeIdentifier(ASTTypeInt, GetName());
  } else {
    if (!GetIdentifier()->GetPolymorphicName().empty())
      M.TypeIdentifier(ASTTypeUInt,
                       GetIdentifier()->GetPolymorphicName());
    else
      M.TypeIdentifier(ASTTypeUInt, GetName());
  }

  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTIntNode::MangleLiteral() {
  ASTMangler M;
  M.Start();
  if (IsSigned()) {
    M.NumericLiteral(GetSignedValue());
  } else {
    M.NumericLiteral(GetUnsignedValue());
  }

  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledLiteralName(M.AsString());
}

const ASTMPDecimalNode* ASTFloatNode::GetMPDecimal() const {
  return (Expr && Expr->GetASTType() == ASTTypeMPDecimal) ?
          dynamic_cast<const ASTMPDecimalNode*>(Expr) :
          nullptr;
}

unsigned ASTFloatNode::Size() const {
  if (Expr) {
    if (const ASTMPDecimalNode* MPD = dynamic_cast<const ASTMPDecimalNode*>(Expr))
      return MPD->GetBits() / CHAR_BIT;
  }

  return sizeof(float) / CHAR_BIT;
}

unsigned ASTFloatNode::GetBits() const {
  if (Expr) {
    if (const ASTMPDecimalNode* MPD = dynamic_cast<const ASTMPDecimalNode*>(Expr))
      return MPD->GetBits();
  }

  return sizeof(float) * CHAR_BIT;
}

const ASTMPDecimalNode* ASTDoubleNode::GetMPDecimal() const {
  return (Expr && Expr->GetASTType() == ASTTypeMPDecimal) ?
          dynamic_cast<const ASTMPDecimalNode*>(Expr) :
          nullptr;
}

unsigned ASTDoubleNode::Size() const {
  if (Expr) {
    if (const ASTMPDecimalNode* MPD = dynamic_cast<const ASTMPDecimalNode*>(Expr))
      return MPD->GetBits() / CHAR_BIT;
  }

  return sizeof(double) / CHAR_BIT;
}

unsigned ASTDoubleNode::GetBits() const {
  if (Expr) {
    if (const ASTMPDecimalNode* MPD = dynamic_cast<const ASTMPDecimalNode*>(Expr))
      return MPD->GetBits();
  }

  return sizeof(double) * CHAR_BIT;
}

void ASTFloatNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (!GetIdentifier()->GetPolymorphicName().empty())
    M.TypeIdentifier(GetASTType(),
                     GetIdentifier()->GetPolymorphicName());
  else
    M.TypeIdentifier(GetASTType(), GetName());

  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTFloatNode::MangleLiteral() {
  ASTMangler M;
  M.Start();
  M.NumericLiteral(GetValue());
  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledLiteralName(M.AsString());
}

ASTMPDecimalNode* ASTDoubleNode::AsMPDecimal(unsigned W) const {
  ASTMPDecimalNode* MPD = new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                                               W, GetString().c_str());
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");
  return MPD;
}

void ASTDoubleNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (!GetIdentifier()->GetPolymorphicName().empty())
    M.TypeIdentifier(GetASTType(),
                     GetIdentifier()->GetPolymorphicName());
  else
    M.TypeIdentifier(GetASTType(), GetName());

  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTDoubleNode::MangleLiteral() {
  ASTMangler M;
  M.Start();
  M.NumericLiteral(GetValue());
  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledLiteralName(M.AsString());
}

void ASTLongDoubleNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (!GetIdentifier()->GetPolymorphicName().empty())
    M.TypeIdentifier(GetASTType(),
                     GetIdentifier()->GetPolymorphicName());
  else
    M.TypeIdentifier(GetASTType(), GetName());
  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTLongDoubleNode::MangleLiteral() {
  ASTMangler M;
  M.Start();
  M.NumericLiteral(GetValue());
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledLiteralName(M.AsString());
}

void ASTVoidNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTStringNode::Mangle() {
  ASTMangler M;
  M.Start();

  if (IsLiteral())
    M.StringLiteral(GetValue());
  else
    M.StringValue(GetValue());

  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTOperatorNode::Mangle() {
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetPolymorphicName("operator");

  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(),
                   GetIdentifier()->GetPolymorphicName());
  M.OpIdentifier(OTy);
  M.EndExpression();

  if (TTy == ASTTypeIdentifier) {
    if (!TId->GetMangledName().empty())
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(TId->GetMangledName()));
    else
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                    ASTMangler::MangleIdentifier(TId)));
  } else {
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(TEx->GetMangledName()));
  }

  M.EndExpression();
  M.End();

  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTOperandNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), "operand");
  M.EndExpression();

  if (IsIdentifier()) {
    if (!TId->GetMangledName().empty())
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(TId->GetMangledName()));
    else
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                    ASTMangler::MangleIdentifier(TId)));
  } else {
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                  GetExpression()->GetMangledName()));
  }

  M.EndExpression();
  M.End();
  // ASTOperandNode sets the mangled name in a different way.
  SetMangledName(M.AsString());
}

void ASTBinaryOpNode::Mangle() {
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetPolymorphicName("binaryop");
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetPolymorphicName());

  if (HasParens())
    M.OpType(ASTOpTypeBinaryLeftFold);

  switch (GetLeft()->GetASTType()) {
  case ASTTypeBinaryOp: {
    if (const ASTBinaryOpNode* CB =
        dynamic_cast<const ASTBinaryOpNode*>(GetLeft())) {
      if (!CB->IsMangled())
        const_cast<ASTBinaryOpNode*>(CB)->Mangle();
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                               CB->GetMangledName()));
    }
  }
    break;
  case ASTTypeUnaryOp: {
    if (const ASTUnaryOpNode* CU =
        dynamic_cast<const ASTUnaryOpNode*>(GetLeft())) {
      if (!CU->IsMangled())
        const_cast<ASTUnaryOpNode*>(CU)->Mangle();
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                               CU->GetMangledName()));
    }
  }
    break;
  case ASTTypeIdentifier: {
    if (!GetLeft()->GetIdentifier()->GetMangledName().empty())
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                    GetLeft()->GetIdentifier()->GetMangledName()));
    else
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                             ASTMangler::MangleIdentifier(GetLeft()->GetIdentifier())));
  }
    break;
  default: {
    if (!GetLeft()->GetIdentifier()->GetMangledLiteralName().empty()) {
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                    GetLeft()->GetIdentifier()->GetMangledLiteralName()));
    } else {
      if (GetLeft()->GetMangledName().empty())
        const_cast<ASTExpressionNode*>(GetLeft())->Mangle();
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                               GetLeft()->GetMangledName()));
    }
  }
    break;
  }

  M.OpType(GetOpType());

  switch (GetRight()->GetASTType()) {
  case ASTTypeBinaryOp: {
    if (const ASTBinaryOpNode* CB =
        dynamic_cast<const ASTBinaryOpNode*>(GetRight())) {
      if (!CB->IsMangled())
        const_cast<ASTBinaryOpNode*>(CB)->Mangle();
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                               CB->GetMangledName()));
    }
  }
    break;
  case ASTTypeUnaryOp: {
    if (const ASTUnaryOpNode* CU =
        dynamic_cast<const ASTUnaryOpNode*>(GetRight())) {
      if (!CU->IsMangled())
        const_cast<ASTUnaryOpNode*>(CU)->Mangle();
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                               CU->GetMangledName()));
    }
  }
    break;
  case ASTTypeIdentifier: {
    if (!GetRight()->GetIdentifier()->GetMangledName().empty())
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                    GetRight()->GetIdentifier()->GetMangledName()));
    else
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                             ASTMangler::MangleIdentifier(GetRight()->GetIdentifier())));
  }
    break;
  default: {
    if (!GetRight()->GetIdentifier()->GetMangledLiteralName().empty()) {
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                    GetRight()->GetIdentifier()->GetMangledLiteralName()));
    } else {
      if (GetRight()->GetMangledName().empty())
        const_cast<ASTExpressionNode*>(GetRight())->Mangle();
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                               GetRight()->GetMangledName()));
    }
  }
    break;
  }

  if (HasParens())
    M.OpType(ASTOpTypeBinaryRightFold);

  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTUnaryOpNode::Mangle() {
  GetIdentifier()->SetPolymorphicName("unaryop");
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetPolymorphicName());
  M.OpType(GetOpType());

  if (HasParens())
    M.OpType(ASTOpTypeUnaryLeftFold);

  switch (GetExpression()->GetASTType()) {
  case ASTTypeBinaryOp: {
    if (const ASTBinaryOpNode* CB =
        dynamic_cast<const ASTBinaryOpNode*>(GetExpression())) {
      if (CB->HasParens())
        M.OpType(ASTOpTypeBinaryLeftFold);

      ASTBinaryOpNode* BOP = const_cast<ASTBinaryOpNode*>(CB);
      BOP->Mangle();
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                               BOP->GetMangledName()));
    }
  }
    break;
  case ASTTypeUnaryOp: {
    if (const ASTUnaryOpNode* CU =
        dynamic_cast<const ASTUnaryOpNode*>(GetExpression())) {
      if (CU->HasParens())
        M.OpType(ASTOpTypeUnaryLeftFold);

      ASTUnaryOpNode* UOP = const_cast<ASTUnaryOpNode*>(CU);
      UOP->Mangle();
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                               UOP->GetMangledName()));
    }
  }
    break;
  case ASTTypeOpndTy: {
    ASTOperandNode* OPX = const_cast<ASTOperandNode*>(
                          dynamic_cast<const ASTOperandNode*>(GetExpression()));
    if (OPX) {
      if (OPX->GetIdentifier()->GetMangledName().empty())
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                      ASTMangler::MangleIdentifier(OPX->GetIdentifier())));
      else
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                      OPX->GetIdentifier()->GetMangledName()));
    }
  }
    break;
  default: {
    if (!GetExpression()->GetIdentifier()->GetMangledLiteralName().empty()) {
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                    GetExpression()->GetIdentifier()->GetMangledLiteralName()));
    } else {
      if (GetExpression()->GetMangledName().empty())
        const_cast<ASTExpressionNode*>(GetExpression())->Mangle();
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                               GetExpression()->GetMangledName()));
    }
  }
    break;
  }

  if (HasParens())
    M.OpType(ASTOpTypeUnaryRightFold);

  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTCBitNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (!GetIdentifier()->GetGateParamName().empty())
    M.TypeIdentifier(GetASTType(), static_cast<unsigned>(Size()),
                     GetIdentifier()->GetGateParamName());
  else
    M.TypeIdentifier(GetASTType(), static_cast<unsigned>(Size()), GetName());

  if (IsBinaryOp()) {
    M.TypeIdentifier(ASTTypeBinaryOp, BOP->GetName());
  } else if (IsUnaryOp()) {
    M.TypeIdentifier(ASTTypeUnaryOp, UOP->GetName());
  } else if (IsGateQOp()) {
    M.TypeIdentifier(QOP->GetASTType(), QOP->GetName());
  }

  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTComplexExpressionNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (!GetIdentifier()->GetPolymorphicName().empty())
    M.TypeIdentifier(GetASTType(), GetIdentifier()->GetPolymorphicName());
  else
    M.TypeIdentifier(GetASTType(), GetName());

  if (GetExpressionType() == ASTTypeBinaryOp) {
    assert(BOP && "Invalid ASTBinaryOpNode Element!");
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                             BOP->GetMangledName()));
  } else if (GetExpressionType() == ASTTypeUnaryOp) {
    assert(UOP && "Invalid ASTUnaryOpNode Element!");
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                             UOP->GetMangledName()));
  }

  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTResetNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetIdentifier()->GetPolymorphicName());

  if (ASTStringUtils::Instance().IsBoundQubit(Target->GetName()))
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                             Target->GetMangledName()));
  else if (Target->GetSymbolTableEntry()->HasValue())
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                             Target->GetMangledName()));
  else {
    M.TypeIdentifier(ASTTypeQubitContainer, Target->GetBits(),
                     Target->GetName());
  }

  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTEllipsisNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTGateControlNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());

  switch (TType) {
  case ASTTypeGate:
  case ASTTypeCNotGate:
  case ASTTypeCXGate:
  case ASTTypeCCXGate:
  case ASTTypeHadamardGate:
  case ASTTypeUGate:
    assert(GN && "Invalid ASTGateNode target!");
    M.Identifier(GN->GetMangledName());
    break;
  case ASTTypeGateQOpNode:
    assert(GQN && "Invalid ASTGateQOpNode target!");
    M.Identifier(GQN->GetMangledName());
    break;
  case ASTTypeGateControl:
    assert(CN && "Invalid ASTGateControlNode target!");
    M.Identifier(CN->GetMangledName());
    break;
  case ASTTypeGateNegControl:
    assert(NCN && "Invalid ASTGateNegControlNode target!");
    M.Identifier(NCN->GetMangledName());
    break;
  case ASTTypeGatePower:
    assert(PN && "Invalid ASTGatePowerNode target!");
    M.Identifier(PN->GetMangledName());
    break;
  case ASTTypeGateInverse:
    assert(IN && "Invalid ASTGateInverseNode target!");
    M.Identifier(IN->GetMangledName());
    break;
  case ASTTypeGPhaseExpression:
    assert(GPN && "Invalid ASTGPhaseExpressionNode target!");
    M.Identifier(GPN->GetMangledName());
    break;
  case ASTTypeGateGPhaseExpression:
    assert(GGEN && "Invalid ASTGateGPhaseExpressionNode target!");
    M.Identifier(GGEN->GetMangledName());
    break;
  default: {
    std::stringstream MM;
    MM << "Invalid gate control target type " << PrintTypeEnum(TType)
      << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::Error);
  }
    break;
  }

  if (MV && MType != ASTTypeUndefined) {
    switch (MType) {
    case ASTTypeGateControl:
      assert(MCN && "Invalid ASTTypeGateControl control modifier!");
      M.Identifier(MCN->GetMangledName());
      break;
    case ASTTypeGateNegControl:
      assert(MNCN && "Invalid ASTGateNegControlNode control modifier!");
      M.Identifier(MNCN->GetMangledName());
      break;
    case ASTTypeGatePower:
      assert(MPN && "Invalid ASTGatePowerNode control modifier!");
      M.Identifier(MPN->GetMangledName());
      break;
    case ASTTypeGateInverse:
      assert(MIN && "Invalid ASTGateInverseNode control modifier!");
      M.Identifier(MIN->GetMangledName());
      break;
    default: {
      std::stringstream MM;
      MM << "Invalid gate modifier type " << PrintTypeEnum(MType) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::Error);
    }
      break;
    }
  }

  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTGateNegControlNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());

  switch (TType) {
  case ASTTypeGate:
  case ASTTypeCNotGate:
  case ASTTypeCXGate:
  case ASTTypeCCXGate:
  case ASTTypeHadamardGate:
  case ASTTypeUGate:
    assert(GN && "Invalid ASTGateNode target!");
    M.Identifier(GN->GetMangledName());
    break;
  case ASTTypeGateQOpNode:
    assert(GQN && "Invalid ASTGateQOpNode target!");
    M.Identifier(GQN->GetMangledName());
    break;
  case ASTTypeGateControl:
    assert(CN && "Invalid ASTGateControlNode target!");
    M.Identifier(CN->GetMangledName());
    break;
  case ASTTypeGateNegControl:
    assert(NCN && "Invalid ASTGateNegControlNode target!");
    M.Identifier(NCN->GetMangledName());
    break;
  case ASTTypeGatePower:
    assert(PN && "Invalid ASTGatePowerNode target!");
    M.Identifier(PN->GetMangledName());
    break;
  case ASTTypeGateInverse:
    assert(IN && "Invalid ASTGateInverseNode target!");
    M.Identifier(IN->GetMangledName());
    break;
  case ASTTypeGPhaseExpression:
    assert(GPN && "Invalid ASTGPhaseExpressionNode target!");
    M.Identifier(GPN->GetMangledName());
    break;
  case ASTTypeGateGPhaseExpression:
    assert(GGEN && "Invalid ASTGateGPhaseExpressionNode target!");
    M.Identifier(GGEN->GetMangledName());
    break;
  default: {
    std::stringstream MM;
    MM << "Invalid gate control target type " << PrintTypeEnum(TType)
      << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::Error);
  }
    break;
  }

  if (MV && MType != ASTTypeUndefined) {
    switch (MType) {
    case ASTTypeGateControl:
      assert(MCN && "Invalid ASTTypeGateControl control modifier!");
      M.Identifier(MCN->GetMangledName());
      break;
    case ASTTypeGateNegControl:
      assert(MNCN && "Invalid ASTGateNegControlNode control modifier!");
      M.Identifier(MNCN->GetMangledName());
      break;
    case ASTTypeGatePower:
      assert(MPN && "Invalid ASTGatePowerNode control modifier!");
      M.Identifier(MPN->GetMangledName());
      break;
    case ASTTypeGateInverse:
      assert(MIN && "Invalid ASTGateInverseNode control modifier!");
      M.Identifier(MIN->GetMangledName());
      break;
    default: {
      std::stringstream MM;
      MM << "Invalid gate modifier type " << PrintTypeEnum(MType) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::Error);
    }
      break;
    }
  }

  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTGatePowerNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());

  switch (TType) {
  case ASTTypeGate:
  case ASTTypeCNotGate:
  case ASTTypeCXGate:
  case ASTTypeCCXGate:
  case ASTTypeHadamardGate:
  case ASTTypeUGate:
    assert(GN && "Invalid ASTGateNode target!");
    M.Identifier(GN->GetMangledName());
    break;
  case ASTTypeGateQOpNode:
    assert(GQN && "Invalid ASTGateQOpNode target!");
    M.Identifier(GQN->GetMangledName());
    break;
  case ASTTypeGateControl:
    assert(CN && "Invalid ASTGateControlNode target!");
    M.Identifier(CN->GetMangledName());
    break;
  case ASTTypeGateNegControl:
    assert(NCN && "Invalid ASTGateNegControlNode target!");
    M.Identifier(NCN->GetMangledName());
    break;
  case ASTTypeGatePower:
    assert(PN && "Invalid ASTGatePowerNode target!");
    M.Identifier(PN->GetMangledName());
    break;
  case ASTTypeGateInverse:
    assert(IN && "Invalid ASTGateInverseNode target!");
    M.Identifier(IN->GetMangledName());
    break;
  case ASTTypeGPhaseExpression:
    assert(GPN && "Invalid ASTGPhaseExpressionNode target!");
    M.Identifier(GPN->GetMangledName());
    break;
  case ASTTypeGateGPhaseExpression:
    assert(GGEN && "Invalid ASTGateGPhaseExpressionNode target!");
    M.Identifier(GGEN->GetMangledName());
    break;
  default: {
    std::stringstream MM;
    MM << "Invalid gate control target type " << PrintTypeEnum(TType)
      << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::Error);
  }
    break;
  }

  if (MV && MType != ASTTypeUndefined) {
    switch (MType) {
    case ASTTypeGateControl:
      assert(MCN && "Invalid ASTTypeGateControl control modifier!");
      M.Identifier(MCN->GetMangledName());
      break;
    case ASTTypeGateNegControl:
      assert(MNCN && "Invalid ASTGateNegControlNode control modifier!");
      M.Identifier(MNCN->GetMangledName());
      break;
    case ASTTypeGatePower:
      assert(MPN && "Invalid ASTGatePowerNode control modifier!");
      M.Identifier(MPN->GetMangledName());
      break;
    case ASTTypeGateInverse:
      assert(MIN && "Invalid ASTGateInverseNode control modifier!");
      M.Identifier(MIN->GetMangledName());
      break;
    default: {
      std::stringstream MM;
      MM << "Invalid gate modifier type " << PrintTypeEnum(MType) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::Error);
    }
      break;
    }
  }

  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTGateInverseNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());

  switch (TType) {
  case ASTTypeGate:
  case ASTTypeCNotGate:
  case ASTTypeCXGate:
  case ASTTypeCCXGate:
  case ASTTypeHadamardGate:
  case ASTTypeUGate:
    assert(GN && "Invalid ASTGateNode target!");
    M.Identifier(GN->GetMangledName());
    break;
  case ASTTypeGateQOpNode:
    assert(GQN && "Invalid ASTGateQOpNode target!");
    M.Identifier(GQN->GetMangledName());
    break;
  case ASTTypeGateControl:
    assert(CN && "Invalid ASTGateControlNode target!");
    M.Identifier(CN->GetMangledName());
    break;
  case ASTTypeGateNegControl:
    assert(NCN && "Invalid ASTGateNegControlNode target!");
    M.Identifier(NCN->GetMangledName());
    break;
  case ASTTypeGatePower:
    assert(PN && "Invalid ASTGatePowerNode target!");
    M.Identifier(PN->GetMangledName());
    break;
  case ASTTypeGateInverse:
    assert(IN && "Invalid ASTGateInverseNode target!");
    M.Identifier(IN->GetMangledName());
    break;
  case ASTTypeGPhaseExpression:
    assert(GPN && "Invalid ASTGPhaseExpressionNode target!");
    M.Identifier(GPN->GetMangledName());
    break;
  case ASTTypeGateGPhaseExpression:
    assert(GGEN && "Invalid ASTGateGPhaseExpressionNode target!");
    M.Identifier(GGEN->GetMangledName());
    break;
  default: {
    std::stringstream MM;
    MM << "Invalid gate control target type " << PrintTypeEnum(TType)
      << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::Error);
  }
    break;
  }

  if (MV && MType != ASTTypeUndefined) {
    switch (MType) {
    case ASTTypeGateControl:
      assert(MCN && "Invalid ASTTypeGateControl control modifier!");
      M.Identifier(MCN->GetMangledName());
      break;
    case ASTTypeGateNegControl:
      assert(MNCN && "Invalid ASTGateNegControlNode control modifier!");
      M.Identifier(MNCN->GetMangledName());
      break;
    case ASTTypeGatePower:
      assert(MPN && "Invalid ASTGatePowerNode control modifier!");
      M.Identifier(MPN->GetMangledName());
      break;
    case ASTTypeGateInverse:
      assert(MIN && "Invalid ASTGateInverseNode control modifier!");
      M.Identifier(MIN->GetMangledName());
      break;
    default: {
      std::stringstream MM;
      MM << "Invalid gate modifier type " << PrintTypeEnum(MType) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::Error);
    }
      break;
    }
  }

  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTInputModifierNode::Mangle() {
  if (GetIdentifier()->GetMangledName().empty()) {
    ASTMangler M;
    M.Start();
    M.TypeIdentifier(GetASTType(), GetName());
    M.End();
    const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
  }
}

void ASTOutputModifierNode::Mangle() {
  if (GetIdentifier()->GetMangledName().empty()) {
    ASTMangler M;
    M.Start();
    M.TypeIdentifier(GetASTType(), GetName());
    M.End();
    const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
  }
}

void ASTPopcountNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(ASTTypeUnaryOp, "unaryop");
  M.OpType(ASTOpTypePopcount);
  M.OpType(ASTOpTypeUnaryLeftFold);

  switch (IType) {
  case ASTTypeInt:
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(I->GetMangledName()));
    break;
  case ASTTypeMPInteger:
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(MPI->GetMangledName()));
    break;
  case ASTTypeBitset:
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(CBI->GetMangledName()));
    break;
  default:
    break;
  }

  M.OpType(ASTOpTypeUnaryRightFold);
  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTRotateNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(ASTTypeUnaryOp, "unaryop");

  switch (OpType) {
  case ASTRotationTypeLeft:
    M.OpType(ASTOpTypeRotl);
    break;
  case ASTRotationTypeRight:
    M.OpType(ASTOpTypeRotr);
    break;
  default: {
    std::stringstream MM;
    MM << "Invalid rotation type " << PrintRotationType(OpType)
      << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::Error);
  }
    break;
  }

  M.OpType(ASTOpTypeUnaryLeftFold);

  switch (IType) {
  case ASTTypeInt:
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(I->GetMangledName()));
    break;
  case ASTTypeMPInteger:
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(MPI->GetMangledName()));
    break;
  case ASTTypeBitset:
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(CBI->GetMangledName()));
    break;
  default: {
    std::stringstream MM;
    MM << "Invalid type " << PrintTypeEnum(IType) << " for rotate op.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::Error);
  }
    break;
  }

  M.Type(ASTTypeInt);
  M.Underscore();
  M.StringValue(std::to_string(S));
  M.OpType(ASTOpTypeUnaryRightFold);
  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTAngleNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeSize(ASTTypeAngle, GetBits());
  if (!GetIdentifier()->GetPolymorphicName().empty())
    M.Identifier(GetIdentifier()->GetPolymorphicName());
  else
    M.Identifier(GetIdentifier()->GetName());

  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledName(M.AsString());
}

void ASTAngleNode::MangleLiteral() {
  ASTMangler M;
  M.Start();

  M.TypeSize(ASTTypeAngle, GetBits());
  if (!GetIdentifier()->GetPolymorphicName().empty())
    M.Identifier(GetIdentifier()->GetPolymorphicName());
  else
    M.Identifier(GetIdentifier()->GetName());

  if (IsExpression()) {
    switch (GetExprType()) {
    case ASTTypeBinaryOp:
      const_cast<ASTBinaryOpNode*>(BOP)->Mangle();
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(BOP->GetMangledName()));
      break;
    case ASTTypeUnaryOp:
      const_cast<ASTUnaryOpNode*>(UOP)->Mangle();
      M.StringValue(ASTStringUtils::Instance().SanitizeMangled(UOP->GetMangledName()));
      break;
    case ASTTypeIdentifier: {
      if (ID) {
        if (ID->IsReference()) {
          if (const ASTIdentifierRefNode* IdR =
              dynamic_cast<const ASTIdentifierRefNode*>(ID)) {
            if (!IdR->GetMangledName().empty())
              M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                            IdR->GetMangledName()));
            else
              M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                            ASTMangler::MangleIdentifier(IdR)));
          }
        } else {
          if (!ID->GetMangledName().empty())
            M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                          ID->GetMangledName()));
          else {
            ASTMangler MM;
            MM.Start();
            MM.TypeSize(ASTTypeAngle, ID->GetBits());
            MM.Identifier(ID->GetName());
            MM.EndExpression();
            MM.End();
            M.StringValue(ASTStringUtils::Instance().SanitizeMangled(MM.AsString()));
          }
        }
      }
    }
      break;
    default:
      if (EX) {
        const_cast<ASTExpressionNode*>(EX)->Mangle();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(EX->GetMangledName()));
      }
      break;
    }
  } else {
    mpz_t MI;
    switch (GetExprType()) {
    case ASTTypeInt:
      if (I) {
        const_cast<ASTIntNode*>(I)->MangleLiteral();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                      I->GetMangledLiteralName()));
      } else {
        ASTIntNode IN(&ASTIdentifierNode::Int,
                      static_cast<int32_t>(mpfr_get_flt(MPValue, MPFR_RNDN)));
        IN.MangleLiteral();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                      IN.GetMangledLiteralName()));
      }
      break;
    case ASTTypeFloat:
      if (F) {
        const_cast<ASTFloatNode*>(F)->MangleLiteral();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                      F->GetMangledLiteralName()));
      } else {
        ASTFloatNode FN(&ASTIdentifierNode::Float,
                        mpfr_get_flt(MPValue, MPFR_RNDN));
        FN.MangleLiteral();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                      FN.GetMangledLiteralName()));
      }
      break;
    case ASTTypeDouble:
      if (D) {
        const_cast<ASTDoubleNode*>(D)->MangleLiteral();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                      D->GetMangledLiteralName()));
      } else {
        ASTDoubleNode DN(&ASTIdentifierNode::Double,
                         mpfr_get_d(MPValue, MPFR_RNDN));
        DN.MangleLiteral();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                      DN.GetMangledLiteralName()));
      }
      break;
    case ASTTypeLongDouble:
      if (LD) {
        const_cast<ASTLongDoubleNode*>(LD)->MangleLiteral();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                      LD->GetMangledLiteralName()));
      } else {
        ASTLongDoubleNode LN(&ASTIdentifierNode::LongDouble,
                             mpfr_get_ld(MPValue, MPFR_RNDN));
        LN.MangleLiteral();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                      LN.GetMangledLiteralName()));
      }
      break;
    case ASTTypeMPInteger:
      if (MPI) {
        const_cast<ASTMPIntegerNode*>(MPI)->MangleLiteral();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                      MPI->GetMangledLiteralName()));
      } else {
        mpz_init2(MI, GetBits());
        mpfr_get_z(MI, MPValue, MPFR_RNDN);
        ASTMPIntegerNode MD(&ASTIdentifierNode::MPInt,
                            ASTMPIntegerNode::DefaultBits, MI, false);
        MD.MangleLiteral();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                      MD.GetMangledLiteralName()));
      }
      break;
    case ASTTypeMPDecimal:
      if (MPD) {
        const_cast<ASTMPDecimalNode*>(MPD)->MangleLiteral();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                      MPD->GetMangledLiteralName()));
      } else {
        ASTMPDecimalNode MD(&ASTIdentifierNode::MPDec, Bits, MPValue);
        MD.MangleLiteral();
        M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                      MD.GetMangledLiteralName()));
      }
      break;
    default:
      break;
    }
  }

  M.EndExpression();
  M.End();
  const_cast<ASTIdentifierNode*>(GetIdentifier())->SetMangledLiteralName(M.AsString());
}

} // namespace QASM

