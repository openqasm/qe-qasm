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

#ifndef __QASM_AST_RETURN_STATEMENT_NODE_H
#define __QASM_AST_RETURN_STATEMENT_NODE_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTValue.h>
#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTCastExpr.h>
#include <qasm/AST/ASTImplicitConversionExpr.h>
#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTImplicitConversionExpr.h>

namespace QASM {

class ASTReturnStatementNode : public ASTStatementNode {
private:
  union {
    ASTVoidNode* Void;
    ASTBoolNode* Bool;
    ASTIntNode* Int;
    ASTFloatNode* Float;
    ASTDoubleNode* Double;
    ASTMPIntegerNode* MPInt;
    ASTMPDecimalNode* MPDec;
    ASTMPComplexNode* MPCplx;
    ASTAngleNode* Angle;
    ASTMeasureNode* Measure;
    ASTCBitNode* CBit;
    ASTQubitNode* QBit;
    ASTQubitContainerNode* QBitContainer;
    ASTBinaryOpNode* BinaryOp;
    ASTUnaryOpNode* UnaryOp;
    ASTValue<>* Value;
    ASTExpressionNode* Expr;
    ASTStatementNode* Stmt;
    ASTFunctionCallStatementNode* FuncStmt;
    ASTIdentifierNode* Ident;
    ASTCastExpressionNode* Cast;
    ASTImplicitConversionNode* ImplConv;
  };

  ASTType Type;
  mutable const ASTImplicitConversionNode* ICE;

protected:
  ASTReturnStatementNode(const ASTIdentifierNode* Id, const std::string& ERM)
  : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)),
  Void(nullptr), Type(ASTTypeStatementError), ICE(nullptr) { }

public:
  static const unsigned ReturnBits = 64U;

public:
  ASTReturnStatementNode(const ASTIdentifierNode* Id)
  : ASTStatementNode(Id), Void(nullptr), Type(ASTTypeVoid),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTVoidNode* V)
  : ASTStatementNode(Id), Void(V), Type(V->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTBoolNode* B)
  : ASTStatementNode(Id), Bool(B), Type(B->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTIntNode* IN)
  : ASTStatementNode(Id), Int(IN), Type(IN->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTFloatNode* FN)
  : ASTStatementNode(Id), Float(FN), Type(FN->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTDoubleNode* DN)
  : ASTStatementNode(Id), Double(DN), Type(DN->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTMPIntegerNode* MPI)
  : ASTStatementNode(Id), MPInt(MPI), Type(MPI->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTMPDecimalNode* MPD)
  : ASTStatementNode(Id), MPDec(MPD), Type(MPD->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTMPComplexNode* MPC)
  : ASTStatementNode(Id), MPCplx(MPC), Type(MPC->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTAngleNode* AN)
  : ASTStatementNode(Id), Angle(AN), Type(AN->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTCBitNode* CB)
  : ASTStatementNode(Id), CBit(CB), Type(CB->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTQubitNode* QB)
  : ASTStatementNode(Id), QBit(QB), Type(QB->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTQubitContainerNode* QBC)
  : ASTStatementNode(Id), QBitContainer(QBC), Type(QBC->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTMeasureNode* MN)
  : ASTStatementNode(Id), Measure(MN), Type(MN->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTBinaryOpNode* BOP)
  : ASTStatementNode(Id), BinaryOp(BOP), Type(BOP->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTUnaryOpNode* UOP)
  : ASTStatementNode(Id), UnaryOp(UOP), Type(UOP->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTValue<>* V)
  : ASTStatementNode(Id), Value(V), Type(ASTTypeValue),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTExpressionNode* E)
  : ASTStatementNode(Id), Expr(E), Type(ASTTypeExpression),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTFunctionCallStatementNode* FSN)
  : ASTStatementNode(Id), FuncStmt(FSN), Type(FSN->GetASTType()),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTStatementNode* S)
  : ASTStatementNode(Id), Stmt(S), Type(ASTTypeStatement),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTIdentifierNode* RId)
  : ASTStatementNode(Id), Ident(RId), Type(ASTTypeIdentifier),
  ICE(nullptr) { }

  ASTReturnStatementNode(const ASTIdentifierNode* Id,
                         ASTCastExpressionNode* CE)
  : ASTStatementNode(Id), Cast(CE), Type(ASTTypeCast),
  ICE(nullptr) { }

  virtual ~ASTReturnStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeReturn;
  }

  virtual ASTSemaType GetSemaType() const override {
    return ASTStatementNode::GetSemaType();
  }

  virtual ASTType GetReturnType() const {
    return Type;
  }

  virtual ASTType GetCastReturnType() const {
    if (Type == ASTTypeCast)
      return Cast->GetCastTo();

    return ASTTypeUndefined;
  }

  virtual ASTType GetImplicitConversionType() const {
    if (IsImplicitConversion())
      return ICE->GetConvertTo();

    return ASTTypeUndefined;
  }

  virtual ASTType GetFunctionCallReturnType() const {
    if (Type == ASTTypeFunctionCallStatement)
      return FuncStmt->GetFunctionCall()->GetResultType();

    return ASTTypeUndefined;
  }

  virtual ASTType GetMeasureReturnType() const {
    if (Type == ASTTypeMeasure)
      return Measure->GetResultType();

    return ASTTypeUndefined;
  }

  // Implemented in ASTFunctions.cpp.
  virtual void Mangle() override;

  virtual void
  AddImplicitConversion(const ASTImplicitConversionNode* CE) {
    assert(CE && "Invalid ASTImplicitConversionNode argument!");
    ICE = CE;
  }

  virtual void
  AddImplicitConversion(const ASTImplicitConversionNode* CE) const {
    assert(CE && "Invalid ASTImplicitConversionNode argument!");
    ICE = CE;
  }

  virtual bool IsVoid() const {
    return Type == ASTTypeVoid || Void == nullptr;
  }

  virtual bool IsCast() const {
    return Type == ASTTypeCast;
  }

  virtual bool IsImplicitConversion() const {
    return ICE;
  }

  virtual bool IsFunctionCall() const {
    return Type == ASTTypeFunctionCallStatement;
  }

  virtual bool IsMeasure() const {
    return Type == ASTTypeMeasure;
  }

  virtual bool IsTransitive() const {
    return Type == ASTTypeFunctionCallStatement ||
           Type == ASTTypeMeasure;
  }

  virtual const ASTImplicitConversionNode* GetImplicitConversion() const {
    return ICE;
  }

  virtual ASTBoolNode* GetBool() {
    return Type == ASTTypeBool ? Bool : nullptr;
  }

  virtual const ASTBoolNode* GetBool() const {
    return Type == ASTTypeBool ? Bool : nullptr;
  }

  virtual ASTIntNode* GetInt() {
    return Type == ASTTypeInt ? Int : nullptr;
  }

  virtual const ASTIntNode* GetInt() const {
    return Type == ASTTypeInt ? Int : nullptr;
  }

  virtual ASTFloatNode* GetFloat() {
    return Type == ASTTypeFloat ? Float : nullptr;
  }

  virtual const ASTFloatNode* GetFloat() const {
    return Type == ASTTypeFloat ? Float : nullptr;
  }

  virtual ASTDoubleNode* GetDouble() {
    return Type == ASTTypeDouble ? Double : nullptr;
  }

  virtual const ASTDoubleNode* GetDouble() const {
    return Type == ASTTypeDouble ? Double : nullptr;
  }

  virtual ASTMPIntegerNode* GetMPInteger() {
    return Type == ASTTypeMPInteger ? MPInt : nullptr;
  }

  virtual const ASTMPIntegerNode* GetMPInteger() const {
    return Type == ASTTypeMPInteger ? MPInt : nullptr;
  }

  virtual ASTMPDecimalNode* GetMPDecimal() {
    return Type == ASTTypeMPDecimal ? MPDec : nullptr;
  }

  virtual const ASTMPDecimalNode* GetMPDecimal() const {
    return Type == ASTTypeMPDecimal ? MPDec : nullptr;
  }

  virtual ASTMPComplexNode* GetMPComplex() {
    return Type == ASTTypeMPComplex ? MPCplx : nullptr;
  }

  virtual const ASTMPComplexNode* GetMPComplex() const {
    return Type == ASTTypeMPComplex ? MPCplx : nullptr;
  }

  virtual ASTAngleNode* GetAngle() {
    return Type == ASTTypeAngle ? Angle : nullptr;
  }

  virtual const ASTAngleNode* GetAngle() const {
    return Type == ASTTypeAngle ? Angle : nullptr;
  }

  virtual ASTCBitNode* GetCBit() {
    return Type == ASTTypeBitset ? CBit : nullptr;
  }

  virtual ASTQubitNode* GetQubit() {
    return Type == ASTTypeQubit ? QBit : nullptr;
  }

  virtual const ASTQubitNode* GetQubit() const {
    return Type == ASTTypeQubit ? QBit : nullptr;
  }

  virtual ASTQubitContainerNode* GetQubitContainer() {
    return Type == ASTTypeQubitContainer ? QBitContainer : nullptr;
  }

  virtual const ASTQubitContainerNode* GetQubitContainer() const {
    return Type == ASTTypeQubitContainer ? QBitContainer : nullptr;
  }

  virtual const ASTCBitNode* GetCBit() const {
    return Type == ASTTypeBitset ? CBit : nullptr;
  }

  virtual ASTMeasureNode* GetMeasure() {
    return Type == ASTTypeMeasure ? Measure : nullptr;
  }

  virtual const ASTMeasureNode* GetMeasure() const {
    return Type == ASTTypeMeasure ? Measure : nullptr;
  }

  virtual const ASTBinaryOpNode* GetBinaryOp() const {
    return Type == ASTTypeBinaryOp ? BinaryOp : nullptr;
  }

  virtual ASTBinaryOpNode* GetBinaryOp() {
    return Type == ASTTypeBinaryOp ? BinaryOp : nullptr;
  }

  virtual const ASTUnaryOpNode* GetUnaryOp() const {
    return Type == ASTTypeUnaryOp ? UnaryOp : nullptr;
  }

  virtual ASTUnaryOpNode* GetUnaryOp() {
    return Type == ASTTypeUnaryOp ? UnaryOp : nullptr;
  }

  virtual const ASTVoidNode* GetVoid() const {
    return Type == ASTTypeVoid ? Void : nullptr;
  }

  virtual ASTVoidNode* GetVoid() {
    return Type == ASTTypeVoid ? Void : nullptr;
  }

  virtual const ASTCastExpressionNode* GetCast() const {
    return Type == ASTTypeCast ? Cast : nullptr;
  }

  virtual ASTCastExpressionNode* GetCast() {
    return Type == ASTTypeCast ? Cast : nullptr;
  }

  virtual ASTValue<>* GetValue() {
    return Type == ASTTypeValue ? Value : nullptr;
  }

  virtual const ASTValue<>* GetValue() const {
    return Type == ASTTypeValue ? Value : nullptr;
  }

  virtual ASTExpressionNode* GetExpression() override {
    return Type == ASTTypeExpression ? Expr : nullptr;
  }

  virtual const ASTExpressionNode* GetExpression() const override {
    return Type == ASTTypeExpression ? Expr : nullptr;
  }

  virtual ASTFunctionCallStatementNode* GetFunctionStatement() {
    return Type == ASTTypeFunctionCallStatement ? FuncStmt : nullptr;
  }

  virtual const ASTFunctionCallStatementNode* GetFunctionStatement() const {
    return Type == ASTTypeFunctionCallStatement ? FuncStmt : nullptr;
  }

  virtual ASTStatementNode* GetStatement() {
    return Type == ASTTypeStatement ? Stmt : nullptr;
  }

  virtual const ASTStatementNode* GetStatement() const {
    return Type == ASTTypeStatement ? Stmt : nullptr;
  }

  virtual const ASTIdentifierNode* GetIdent() const {
    return Type == ASTTypeIdentifier ? Ident : nullptr;
  }

  virtual bool IsError() const override {
    return Type == ASTTypeStatementError ||
           ASTStatementNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTReturnStatementNode* StatementError(const std::string& ERM) {
    return new ASTReturnStatementNode(ASTIdentifierNode::Return.Clone(), ERM);
  }

  static ASTReturnStatementNode* StatementError(const ASTIdentifierNode* Id,
                                                const std::string& ERM) {
    return new ASTReturnStatementNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<ReturnStatement>" << std::endl;

    switch (Type) {
    case ASTTypeBool:
      Bool->print();
      break;
    case ASTTypeInt:
      Int->print();
      break;
    case ASTTypeFloat:
      Float->print();
      break;
    case ASTTypeDouble:
      Double->print();
      break;
    case ASTTypeMPInteger:
      MPInt->print();
      break;
    case ASTTypeMPDecimal:
      MPDec->print();
      break;
    case ASTTypeAngle:
      Angle->print();
      break;
    case ASTTypeBitset:
      CBit->print();
      break;
    case ASTTypeQubit:
      QBit->print();
      break;
    case ASTTypeMeasure:
      Measure->print();
      break;
    case ASTTypeBinaryOp:
      BinaryOp->print();
      break;
    case ASTTypeUnaryOp:
      UnaryOp->print();
      break;
    case ASTTypeExpression:
      Expr->print();
      break;
    case ASTTypeStatement:
      Stmt->print();
      break;
    case ASTTypeValue:
      Value->print();
      break;
    case ASTTypeIdentifier:
      Ident->print();
      break;
    case ASTTypeVoid:
      if (Void)
        Void->print();
      else
        std::cout << "<Void></Void>" << std::endl;
      break;
    case ASTTypeCast:
      Cast->print();
      break;
    default:
      break;
    }

    std::cout << "</ReturnStatement>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM

#endif // __QASM_AST_RETURN_STATEMENT_NODE_H

