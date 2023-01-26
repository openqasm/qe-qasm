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

#ifndef __QASM_AST_BUILDER_H
#define __QASM_AST_BUILDER_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTStatement.h>
#include <qasm/AST/ASTFunction.h>
#include <qasm/AST/ASTDeclaration.h>
#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTPrimitives.h>
#include <qasm/AST/ASTBlock.h>
#include <qasm/AST/ASTProgramBlock.h>
#include <qasm/AST/ASTTypeSpecifier.h>
#include <qasm/AST/ASTRegisters.h>
#include <qasm/AST/ASTValue.h>
#include <qasm/AST/ASTArgument.h>
#include <qasm/AST/ASTGates.h>
#include <qasm/AST/ASTBarrier.h>
#include <qasm/AST/ASTMeasure.h>
#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTPopcount.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTQubitList.h>
#include <qasm/AST/ASTReset.h>
#include <qasm/AST/ASTRotate.h>
#include <qasm/AST/ASTFunctions.h>
#include <qasm/AST/ASTKernel.h>
#include <qasm/AST/ASTDefcal.h>
#include <qasm/AST/ASTLength.h>
#include <qasm/AST/ASTDelay.h>
#include <qasm/AST/ASTDuration.h>
#include <qasm/AST/ASTStretch.h>
#include <qasm/AST/ASTReturn.h>
#include <qasm/AST/ASTBox.h>
#include <qasm/AST/ASTGPhase.h>
#include <qasm/AST/ASTRoot.h>
#include <qasm/AST/ASTCallExpr.h>
#include <qasm/AST/ASTCastExpr.h>
#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTArray.h>
#include <qasm/AST/ASTCtrlAssocBuilder.h>
#include <qasm/AST/ASTInverseAssocBuilder.h>
#include <qasm/AST/ASTBoxStatementBuilder.h>

// OpenPulse
#include <qasm/AST/OpenPulse/ASTOpenPulsePlay.h>
#include <qasm/AST/OpenPulse/ASTOpenPulsePort.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseFrame.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseWaveform.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseCalibration.h>

namespace QASM {

class ASTBuilder {
private:
  static ASTBuilder Builder;
  static uint64_t IdentCounter;

private:
  ASTBuilder() = default;

  ASTBuilder(const ASTBuilder& RHS) = delete;
  ASTBuilder& operator=(const ASTBuilder& RHS) = delete;

public:
  static ASTBuilder& Instance() {
    return Builder;
  }

  ~ASTBuilder() = default;

  ASTOpenQASMStatementNode*
  CreateASTOpenQASMStatementNode(const std::string& OQS,
                                 const std::string& OQV);

  ASTArgumentNode* CreateASTArgumentNode(ASTIdentifierNode* Id,
                                         const std::any& Val = std::any(),
                                         ASTType Ty = ASTTypeUndefined);
  ASTArgumentNode* CreateASTArgumentNode(ASTIdentifierRefNode* Id,
                                         const std::any& Val = std::any(),
                                         ASTType Ty = ASTTypeUndefined);
  ASTExpressionNode* CreateASTExpressionNode(const ASTExpression* E,
                                             ASTType Ty);
  ASTExpressionNode* CreateASTExpressionNode(const std::string& Identifier,
                                             const ASTExpression* E,
                                             ASTType Ty);
  ASTExpressionNode*
  CreateASTIdentifierExpressionNode(const ASTIdentifierNode* Id);

  ASTOperatorNode* CreateASTOperatorNode(const ASTIdentifierNode* TId,
                                         ASTOpType OT);
  ASTOperatorNode* CreateASTOperatorNode(const ASTExpressionNode* TEx,
                                         ASTOpType OT);
  ASTOperandNode* CreateASTOperandNode(const ASTIdentifierNode* TId);
  ASTOperandNode* CreateASTOperandNode(const ASTExpressionNode* TEx);

  ASTBinaryOpNode* CreateASTBinaryOpNode(const std::string& Identifier,
                                         const ASTExpressionNode* L,
                                         const ASTExpressionNode* R,
                                         ASTOpType OT,
                                         bool Parens = false);
  ASTBinaryOpNode* CreateASTBinaryOpNode(const std::string& Identifier,
                                         const ASTIdentifierNode* L,
                                         const ASTExpressionNode* R,
                                         ASTOpType OT,
                                         bool Parens = false);
  ASTBinaryOpNode* CreateASTBinaryOpNode(const std::string& Identifier,
                                         const ASTExpressionNode* L,
                                         const ASTIdentifierNode* R,
                                         ASTOpType OT,
                                         bool Parens = false);
  ASTBinaryOpNode* CreateASTBinaryOpNode(const std::string& Identifier,
                                         const ASTIdentifierNode* L,
                                         const ASTIdentifierNode* R,
                                         ASTOpType OT,
                                         bool Parens = false);
  ASTBinaryOpNode* CreateASTBinaryOpNode(const std::string& Identifier,
                                         const std::string& L,
                                         const ASTExpressionNode* R,
                                         ASTOpType OT,
                                         bool Parens = false);
  ASTBinaryOpNode* CreateASTBinaryOpNode(const std::string& Identifier,
                                         const std::string& L,
                                         const std::string& R,
                                         ASTOpType OT,
                                         bool Parens = false);
  ASTBinaryOpNode*
  CreateASTArithNegateBinaryOpNode(const ASTIdentifierNode* Id,
                                   const std::string& BON);

  ASTUnaryOpNode* CreateASTUnaryOpNode(const std::string& Identifier,
                                       const ASTExpressionNode* R,
                                       ASTOpType OT,
                                       bool HasParen = false,
                                       bool IsLValue = false);
  ASTUnaryOpNode* CreateASTUnaryOpNode(const std::string& Identifier,
                                       const ASTIdentifierNode* Id,
                                       ASTOpType OT,
                                       bool HasParen = false,
                                       bool IsLValue = false);

  ASTResetNode* CreateASTResetNode(const ASTIdentifierNode* Id,
                                   const ASTIdentifierNode* Target);

  ASTBarrierNode* CreateASTBarrierNode(const ASTIdentifierNode* Id,
                                       const ASTIdentifierList& List);

  ASTBoolNode* CreateASTBoolNode(bool Value);
  ASTBoolNode* CreateASTBoolNode(const ASTIdentifierNode* Id, bool Value);
  ASTBoolNode* CreateASTBoolNode(const ASTIdentifierNode* Id,
                                 const ASTBinaryOpNode* BOP);
  ASTBoolNode* CreateASTBoolNode(const ASTIdentifierNode* Id,
                                 const ASTUnaryOpNode* UOP);
  ASTBoolNode* CreateASTBoolNode(const ASTIdentifierNode* Id,
                                 const ASTExpressionNode* EN);

  ASTIntNode*
  CreateASTIntNode(int32_t Value,
                   const ASTCVRQualifiers& CVR = ASTCVRQualifiers());
  ASTIntNode*
  CreateASTIntNode(uint32_t Value,
                   const ASTCVRQualifiers& CVR = ASTCVRQualifiers());
  ASTIntNode*
  CreateASTIntNode(const ASTIdentifierNode* Id,
                   int32_t Value,
                   const ASTCVRQualifiers& CVR = ASTCVRQualifiers());
  ASTIntNode*
  CreateASTIntNode(const ASTIdentifierNode* Id,
                   uint32_t Value,
                   const ASTCVRQualifiers& CVR = ASTCVRQualifiers());
  ASTIntNode*
  CreateASTIntNode(const ASTIdentifierNode* Id,
                   const ASTExpressionNode* E,
                   bool Unsigned = false,
                   const ASTCVRQualifiers& CVR = ASTCVRQualifiers());
  ASTFloatNode*
  CreateASTFloatNode(float Value,
                     const ASTCVRQualifiers& CVR = ASTCVRQualifiers());
  ASTFloatNode*
  CreateASTFloatNode(const ASTIdentifierNode* Id, float Value,
                     const ASTCVRQualifiers& CVR = ASTCVRQualifiers());
  ASTFloatNode*
  CreateASTFloatNode(const ASTIdentifierNode* Id,
                     const ASTExpressionNode* E,
                     const ASTCVRQualifiers& CVR = ASTCVRQualifiers());

  ASTDoubleNode*
  CreateASTDoubleNode(double Value,
                      const ASTCVRQualifiers& CVR = ASTCVRQualifiers());
  ASTDoubleNode*
  CreateASTDoubleNode(const ASTIdentifierNode* Id, double Value,
                      const ASTCVRQualifiers& CVR = ASTCVRQualifiers());
  ASTDoubleNode*
  CreateASTDoubleNode(const ASTIdentifierNode* Id,
                      const ASTExpressionNode* E,
                      const ASTCVRQualifiers& CVR = ASTCVRQualifiers());

  ASTIdentifierNode* FindASTIdentifierNode(const ASTIdentifierNode* Id);
  ASTIdentifierNode* FindASTIdentifierNode(const std::string& Id);
  ASTIdentifierNode* FindASTIdentifierNode(const std::string& Id,
                                           unsigned Bits,
                                           ASTType Type);
  ASTIdentifierNode* FindASTIdentifierNode(const std::string& Id,
                                           ASTType Type);
  unsigned FindASTIdentifierNode(const std::string& Id,
                                 unsigned Bits,
                                 ASTType Type,
                                 std::vector<ASTSymbolTableEntry*>& STV);

  ASTIdentifierNode* CreateASTIdentifierNode(const std::string& Id);
  ASTIdentifierNode*
  CreateASTIdentifierNode(const std::string& Id,
                          unsigned Bits,
                          ASTType Type,
                          const ASTDeclarationContext* DCX = nullptr,
                          bool TypeSystemBuilder = false);
  ASTIdentifierNode*
  CreateLocalScopeASTIdentifierNode(const std::string& Id,
                                    unsigned Bits,
                                    ASTType Type,
                                    const ASTDeclarationContext* DCX,
                                    const ASTToken* TK);
  ASTIdentifierNode* CreateASTPragmaIdentifierNode(const std::string& Id);

  ASTIdentifierNode* FindOrCreateASTIdentifierNode(const std::string& Id);
  ASTIdentifierNode* FindOrCreateASTIdentifierNode(const std::string& Id,
                                                   unsigned Bits,
                                                   ASTType Type);
  ASTIdentifierNode* FindOrCreateASTIdentifierNode(const std::string& Id,
                                                   const ASTBinaryOpNode* BOP,
                                                   unsigned Bits,
                                                   ASTType Type);
  ASTIdentifierNode* FindOrCreateASTIdentifierNode(const std::string& Id,
                                                   const ASTUnaryOpNode* UOP,
                                                   unsigned Bits,
                                                   ASTType Type);

  ASTIdentifierNode* FindOrCreateIndexedASTIdentifierNode(const std::string& Id,
                                                          unsigned Bits,
                                                          ASTType Type);
  ASTIdentifierNode*
  FindUnindexedASTIdentifierNode(const ASTIdentifierNode* Id);

  ASTIdentifierNode*
  CreateASTIdentifierRValueReference(const ASTIdentifierNode* Id);

  ASTMPIntegerNode* CreateASTMPIntegerNode(const ASTIdentifierNode* Id,
                                           unsigned Bits, bool Unsigned = false);
  ASTMPIntegerNode* CreateASTMPIntegerNode(const ASTIdentifierNode* Id,
                                           unsigned Bits, const char* String,
                                           bool Unsigned = false);
  ASTMPIntegerNode* CreateASTMPIntegerNode(const ASTIdentifierNode* Id,
                                           unsigned Bits,
                                           const ASTExpressionNode* Expr,
                                           bool Unsigned = false);
  ASTMPIntegerNode* CreateASTMPIntegerNode(const ASTIdentifierNode* Id,
                                           unsigned Bits, const mpz_t& Value,
                                           bool Unsigned = false);
  ASTMPIntegerNode* CreateASTMPIntegerNode(const ASTIdentifierNode* Id,
                                           unsigned Bits,
                                           const ASTIntNode* I);
  ASTMPIntegerNode*
  CreateASTMPIntegerNodeFromExpression(const ASTIdentifierNode* Id,
                                       unsigned Bits,
                                       ASTExpressionNode* EN,
                                       bool Unsigned = false);

  ASTMPDecimalNode* CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                           unsigned Bits);
  ASTMPDecimalNode* CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                           unsigned Bits,
                                           const ASTExpressionNode* Expr);
  ASTMPDecimalNode* CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                           unsigned Bits, int Mantissa);
  ASTMPDecimalNode* CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                           unsigned Bits,
                                           const ASTExpressionNode* Expr,
                                           int Mantissa);
  ASTMPDecimalNode* CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                           unsigned Bits,
                                           const char* String);
  ASTMPDecimalNode* CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                           unsigned Bits,
                                           const mpfr_t& MPFR);
  ASTMPDecimalNode* CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                           unsigned Bits, double D);
  ASTMPDecimalNode* CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                           unsigned Bits, long double LD);
  ASTMPDecimalNode* CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                           unsigned Bits, float F);
  ASTMPDecimalNode*
  CreateASTMPDecimalNodeFromExpression(const ASTIdentifierNode* Id,
                                       unsigned Bits,
                                       const ASTExpressionNode* EN);

  ASTMPComplexNode*
  CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                         unsigned NumBits = ASTMPComplexNode::DefaultBits);
  ASTMPComplexNode*
  CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                         const ASTComplexExpressionNode* E,
                         unsigned NumBits = ASTMPComplexNode::DefaultBits);
  ASTMPComplexNode*
  CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                         const ASTMPDecimalNode* R,
                         const ASTMPDecimalNode* I,
                         ASTOpType OT,
                         unsigned NumBits = ASTMPComplexNode::DefaultBits);
  ASTMPComplexNode*
  CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                         const ASTMPIntegerNode* R,
                         const ASTMPIntegerNode* I,
                         ASTOpType OT,
                         unsigned NumBits = ASTMPComplexNode::DefaultBits);
  ASTMPComplexNode*
  CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                         const ASTMPDecimalNode* R,
                         const ASTMPIntegerNode* I,
                         ASTOpType OT,
                         unsigned NumBits = ASTMPComplexNode::DefaultBits);
  ASTMPComplexNode*
  CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                         const ASTMPIntegerNode* R,
                         const ASTMPDecimalNode* I,
                         ASTOpType OT,
                         unsigned NumBits = ASTMPComplexNode::DefaultBits);
  ASTMPComplexNode*
  CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                         const std::string& REP,
                         unsigned NumBits = ASTMPComplexNode::DefaultBits);
  ASTMPComplexNode*
  CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                         const ASTFunctionCallNode* FC,
                         unsigned Bits = ASTMPComplexNode::DefaultBits);

  ASTMPComplexNode*
  CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                         const ASTMPComplexNode* MPC,
                         unsigned Bits = ASTMPComplexNode::DefaultBits);

  ASTComplexExpressionNode*
  CreateASTComplexExpressionNode(const ASTIdentifierNode* Id,
                                 const ASTBinaryOpNode* BOp);
  ASTComplexExpressionNode*
  CreateASTComplexExpressionNode(const ASTIdentifierNode* Id,
                                 const ASTUnaryOpNode* UOp);

  ASTMeasureNode*
  CreateASTMeasureNode(const ASTIdentifierNode* Id,
                       ASTQubitContainerNode* Target,
                       ASTCBitNode* Result,
                       unsigned RIX = static_cast<unsigned>(~0x0));
  ASTMeasureNode*
  CreateASTMeasureNode(const ASTIdentifierNode* Id,
                       ASTQubitContainerNode* Target,
                       ASTAngleNode* Result,
                       unsigned RIX = static_cast<unsigned>(~0x0));
  ASTMeasureNode*
  CreateASTMeasureNode(const ASTIdentifierNode* Id,
                       ASTQubitContainerNode* Target,
                       ASTMPComplexNode* Result,
                       unsigned RIX = static_cast<unsigned>(~0x0));

  ASTQRegNode* CreateASTQRegNode(const ASTIdentifierNode* Id,
                                 unsigned NumQubits);

  ASTCRegNode* CreateASTCRegNode(const ASTIdentifierNode* Id,
                                 unsigned NumBits);
  ASTCRegNode* CreateASTCRegNode(const ASTIdentifierNode* Id,
                                 const std::vector<unsigned>& BV);

  ASTAngleNode* CreateASTAngleNode(const std::string& Id,
                                   ASTAngleType ATy,
                                   unsigned NumBits = ASTAngleNode::AngleBits);
  ASTAngleNode* CreateASTAngleNode(const ASTIdentifierNode* Id,
                                   ASTAngleType ATy,
                                   unsigned NumBits = ASTAngleNode::AngleBits);
  ASTAngleNode* CreateASTAngleNode(const ASTIdentifierNode* Id,
                                   uint64_t W, uint64_t X, uint64_t Y,
                                   uint64_t Z, ASTAngleType ATy,
                                   unsigned NumBits = ASTAngleNode::AngleBits);
  ASTAngleNode* CreateASTAngleNode(const ASTIdentifierNode* Id,
                                   const ASTExpressionNode* E,
                                   ASTAngleType ATy,
                                   unsigned NumBits = ASTAngleNode::AngleBits);
  ASTAngleNode* CreateASTAngleNode(const ASTIdentifierNode* Id,
                                   ASTAngleType ATy,
                                   const mpfr_t& MPV,
                                   unsigned NumBits = ASTAngleNode::AngleBits);
  void MaterializeASTAngleNode(ASTAngleNode* AN);

  ASTAngleNode* CloneASTAngleNode(const ASTIdentifierNode* Id,
                                  const ASTAngleNode* AN);
  ASTAngleNode* CloneASTAngleNode(const ASTIdentifierNode* Id,
                                  const ASTAngleNode* AN,
                                  unsigned NumBits);

  ASTAngleNode* CreateASTAngleNodeFromExpression(const ASTIdentifierNode* Id,
                                                 const ASTExpressionNode* E,
                                                 ASTAngleType ATy,
                                                 unsigned NumBits);

  ASTQubitContainerNode*
  CreateASTQubitContainerNode(const ASTIdentifierNode* Id, unsigned NumBits);

  ASTQubitContainerNode*
  CreateASTQubitContainerNode(const ASTIdentifierNode* Id,
                              const std::vector<ASTQubitNode*>& QV);

  ASTQubitContainerNode*
  CreateASTQubitContainerNode(const ASTIdentifierNode* Id,
                              unsigned NumBits,
                              const std::vector<ASTQubitNode*>& QV);

  ASTQubitNode*
  CreateASTQubitNode(const ASTIdentifierNode* Id, unsigned Index);

  ASTQubitContainerAliasNode*
  CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                   int32_t Index);

  ASTQubitContainerAliasNode*
  CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                   const ASTQubitContainerNode* QCN,
                                   int32_t Index);

  ASTQubitContainerAliasNode*
  CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                   const ASTQubitContainerAliasNode* QCA,
                                   int32_t Index);

  ASTQubitContainerAliasNode*
  CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                   const ASTQubitContainerNode* QCN,
                                   const ASTIntegerList& IL,
                                   char Method);

  ASTQubitContainerAliasNode*
  CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                   const ASTQubitContainerAliasNode* QCA,
                                   const ASTIntegerList& IL,
                                   char Method);

  ASTQubitContainerAliasNode*
  CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                   const ASTQubitContainerNode* QCN);

  ASTQubitContainerAliasNode*
  CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                   const ASTQubitContainerAliasNode* QCA);

  ASTCBitNode*
  CreateASTCBitNode(const ASTIdentifierNode* Id, std::size_t Size,
                    std::size_t Bitmask);
  ASTCBitNode*
  CreateASTCBitNode(const ASTIdentifierNode* Id, std::size_t Size,
                    const std::string& Bitmask);
  ASTCBitNode*
  CreateASTCBitNode(const ASTIdentifierNode* Id, std::size_t Size,
                    const ASTCastExpressionNode* C);
  ASTCBitNode*
  CreateASTCBitNode(const ASTIdentifierNode* Id, std::size_t Size,
                    const ASTBinaryOpNode* B);
  ASTCBitNode*
  CreateASTCBitNode(const ASTIdentifierNode* Id, std::size_t Size,
                    const ASTUnaryOpNode* U);
  ASTCBitNode*
  CreateASTCBitNode(const ASTIdentifierNode* Id, std::size_t Size,
                    const ASTImplicitConversionNode* IC);

  ASTResultNode*
  CreateASTResultNode(const ASTIdentifierNode* Id, ASTType RTy,
                      unsigned NumBits = 0U,
                      ASTSignbit SB = ASTSignbit::Signed);

  ASTFunctionDefinitionNode*
  CreateASTFunctionDefinition(const ASTIdentifierNode* Id,
                              const ASTDeclarationList& PDL,
                              const ASTStatementList& SL,
                              ASTResultNode *RES,
                              bool IsDefinition = true);


  ASTFunctionDeclarationNode*
  CreateASTFunctionDeclaration(const ASTIdentifierNode* Id,
                               const ASTFunctionDefinitionNode* FDN);

  ASTKernelNode*
  CreateASTKernelNode(const ASTIdentifierNode* Id,
                      const ASTDeclarationList& DL,
                      const ASTStatementList& SL,
                      ASTResultNode *RES);
  ASTKernelDeclarationNode*
  CreateASTKernelDeclaration(const ASTIdentifierNode* Id,
                             const ASTKernelNode* KN);

  ASTReturnStatementNode*
  CreateASTReturnStatementNode(const ASTIdentifierNode* Id);

  ASTReturnStatementNode*
  CreateASTReturnStatementNode(const ASTIdentifierNode* Id,
                               ASTIdentifierNode* RId);
  ASTReturnStatementNode*
  CreateASTReturnStatementNode(const ASTIdentifierNode* Id,
                               ASTExpressionNode* EN);
  ASTReturnStatementNode*
  CreateASTReturnStatementNode(const ASTIdentifierNode* Id,
                               ASTStatementNode* SN);
  ASTReturnStatementNode*
  CreateASTReturnStatementNode(const ASTIdentifierNode* Id,
                               ASTFunctionCallStatementNode* FSN);
  ASTReturnStatementNode*
  CreateASTReturnStatementNode(const ASTIdentifierNode* Id,
                               ASTMeasureNode* MN);
  ASTPopcountNode*
  CreateASTPopcountNode(const ASTIdentifierNode* Id,
                        const ASTIntNode* I);
  ASTPopcountNode*
  CreateASTPopcountNode(const ASTIdentifierNode* Id,
                        const ASTMPIntegerNode* MPI);
  ASTPopcountNode*
  CreateASTPopcountNode(const ASTIdentifierNode* Id,
                        const ASTCBitNode* CBI);

  ASTRotateNode*
  CreateASTRotateNode(const ASTIdentifierNode* Id,
                      const ASTIntNode* I,
                      const ASTIntNode* S,
                      ASTRotationType OpTy);
  ASTRotateNode*
  CreateASTRotateNode(const ASTIdentifierNode* Id,
                      const ASTMPIntegerNode* MPI,
                      const ASTIntNode* S,
                      ASTRotationType OpTy);
  ASTRotateNode*
  CreateASTRotateNode(const ASTIdentifierNode* Id,
                      const ASTCBitNode* CBI,
                      const ASTIntNode* S,
                      ASTRotationType OpTy);

  ASTDefcalNode*
  CreateASTDefcalNode(const ASTIdentifierNode* Id,
                      const ASTExpressionNodeList& EL,
                      const ASTStatementList& SL,
                      const ASTBoundQubitList* BQL);
  ASTDefcalNode*
  CreateASTDefcalNode(const ASTIdentifierNode* Id,
                      const std::string& GM,
                      const ASTExpressionNodeList& EL,
                      const ASTStatementList& SL,
                      const ASTBoundQubitList* BQL);
  ASTDefcalNode*
  CreateASTDefcalNode(const ASTIdentifierNode* Id,
                      const ASTStatementList& SL,
                      const ASTMeasureNode* M);
  ASTDefcalNode*
  CreateASTDefcalNode(const ASTIdentifierNode* Id,
                      const std::string& GM,
                      const ASTStatementList& SL,
                      const ASTMeasureNode* M);
  ASTDefcalNode*
  CreateASTDefcalNode(const ASTIdentifierNode* Id,
                      const ASTStatementList& SL,
                      const ASTResetNode* R);
  ASTDefcalNode*
  CreateASTDefcalNode(const ASTIdentifierNode* Id,
                      const std::string& GM,
                      const ASTStatementList& SL,
                      const ASTResetNode* R);
  ASTDefcalNode*
  CreateASTDefcalNode(const ASTIdentifierNode* Id,
                      const ASTDelayNode* DN,
                      const ASTStatementList& SL,
                      const ASTBoundQubitList* BQL);
  ASTDefcalNode*
  CreateASTDefcalNode(const ASTIdentifierNode* Id,
                      const std::string& GS,
                      const ASTDelayNode* DN,
                      const ASTStatementList& SL,
                      const ASTBoundQubitList* BQL);
  ASTDefcalNode*
  CreateASTDefcalNode(const ASTIdentifierNode* Id,
                      const ASTDurationOfNode* DN,
                      const ASTStatementList& SL,
                      const ASTBoundQubitList* BQL);
  ASTDefcalNode*
  CreateASTDefcalNode(const ASTIdentifierNode* Id,
                      const std::string& GS,
                      const ASTDurationOfNode* DN,
                      const ASTStatementList& SL,
                      const ASTBoundQubitList* BQL);

  ASTDefcalGroupNode*
  CreateASTDefcalGroupNode(const ASTIdentifierNode* Id);

  ASTDefcalDeclarationNode*
  CreateASTDefcalDeclaration(const ASTIdentifierNode* Id,
                             const ASTDefcalNode* DN);


  ASTDurationNode*
  CreateASTDurationNode(const ASTIdentifierNode* Id,
                        const std::string& Units);
  ASTDurationNode*
  CreateASTDurationNode(const ASTIdentifierNode* Id,
                        const ASTDurationOfNode* DON);
  ASTDurationNode*
  CreateASTDurationNode(const ASTIdentifierNode* Id,
                        const ASTBinaryOpNode* BOP);
  ASTDurationNode*
  CreateASTDurationNode(const ASTIdentifierNode* Id,
                        const ASTExpressionNode* EXP);

  ASTDurationOfNode*
  CreateASTDurationOfNode(const ASTIdentifierNode* Id,
                          const ASTIdentifierNode* TId);
  ASTDurationOfNode*
  CreateASTDurationOfNode(const ASTIdentifierNode* Id,
                          const ASTIdentifierNode* TId,
                          const ASTIdentifierList* QIL);

  ASTDurationOfNode*
  CreateASTDurationOfNode(const ASTIdentifierNode* Id,
                          const ASTIdentifierNode* TId,
                          const ASTExpressionList* AIL,
                          const ASTIdentifierList* QIL);
  ASTDurationOfNode*
  CreateASTDurationOfNode(const ASTIdentifierNode* Id,
                          const ASTGateQOpNode* QOP);

  ASTLengthNode*
  CreateASTLengthNode(const ASTIdentifierNode* Id,
                      const std::string& Units);
  ASTLengthNode*
  CreateASTLengthNode(const ASTIdentifierNode* Id,
                      const ASTLengthOfNode* LON);
  ASTLengthNode*
  CreateASTLengthNode(const ASTIdentifierNode* Id,
                      const ASTBinaryOpNode* BOP);

  ASTLengthOfNode*
  CreateASTLengthOfNode(const ASTIdentifierNode* Id,
                        const ASTIdentifierNode* TId);

  ASTCallExpressionNode*
  CreateASTCallExpressionNode(const ASTIdentifierNode* Id,
                              const ASTIdentifierNode* CId,
                              const ASTParameterList& PL,
                              const ASTIdentifierList& IL);
  ASTControlExpressionNode*
  CreateASTControlExpressionNode(const ASTIdentifierNode* Id,
                                 const ASTCtrlAssocList* CL);

  ASTInverseExpressionNode*
  CreateASTInverseExpressionNode(const ASTIdentifierNode* Id,
                                 const ASTInverseAssocList* IL);

  ASTGateNode*
  CreateASTGateNode(const ASTIdentifierNode* Id,
                    GateKind GK,
                    const ASTParameterList& PL,
                    const ASTIdentifierList& IL,
                    const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault);
  ASTGateNode*
  CreateASTGateNode(const ASTIdentifierNode* Id,
                    GateKind GK,
                    const ASTArgumentNodeList& AL,
                    const ASTAnyTypeList& QL,
                    const ASTGateQOpList& OL = ASTGateQOpList::EmptyDefault);

  ASTCastExpressionNode*
  CreateASTCastExpressionNode(const ASTExpressionNode* From, ASTType To,
                              unsigned Bits = 0);

  ASTDelayNode*
  CreateASTDelayNode(const ASTIdentifierNode* Id,
                     const ASTIdentifierNode* TId,
                     const std::string& Time = "");
  ASTDelayNode*
  CreateASTDelayNode(const ASTIdentifierNode* Id,
                     const ASTBinaryOpNode* BOP);
  ASTDelayNode*
  CreateASTDelayNode(const ASTIdentifierNode* Id,
                     const ASTUnaryOpNode* UOP);
  ASTDelayNode*
  CreateASTDelayNode(const ASTIdentifierNode* Id,
                     const ASTIdentifierNode* TId,
                     const ASTIdentifierNode* QId,
                     const std::string& Time = "");
  ASTDelayNode*
  CreateASTDelayNode(const ASTIdentifierNode* Id,
                     const ASTIdentifierNode* TId,
                     const ASTIdentifierList& IL,
                     const std::string& Time = "");
  ASTDelayNode*
  CreateASTDelayNode(const ASTIdentifierNode* DId,
                     const ASTIdentifierList& IL,
                     const std::string& Time = "");
  ASTDelayNode*
  CreateASTDelayNode(const ASTIdentifierNode* Id,
                     const ASTDurationOfNode* DON,
                     const ASTIdentifierList& IL);

  ASTDelayNode*
  CreateASTDelayNode(const ASTIdentifierNode* Id,
                     const ASTDurationNode* DON,
                     const ASTIdentifierList& IL);

  ASTStretchNode*
  CreateASTStretchNode(const ASTIdentifierNode* Id);
  ASTStretchNode*
  CreateASTStretchNode(const ASTIdentifierNode* Id,
                       const std::string& LU);
  ASTStretchNode*
  CreateASTStretchNode(const ASTIdentifierNode* Id,
                       uint64_t Duration, LengthUnit LU);
  ASTStretchNode*
  CreateASTStretchNode(const ASTIdentifierNode* Id,
                       const ASTIntNode* I);
  ASTStretchNode*
  CreateASTStretchNode(const ASTIdentifierNode* Id,
                       const ASTFloatNode* F);
  ASTStretchNode*
  CreateASTStretchNode(const ASTIdentifierNode* Id,
                       const ASTDoubleNode* D);
  ASTStretchNode*
  CreateASTStretchNode(const ASTIdentifierNode* Id,
                       const ASTMPIntegerNode* MPI);
  ASTStretchNode*
  CreateASTStretchNode(const ASTIdentifierNode* Id,
                       const ASTMPDecimalNode* MPD);
  ASTStretchNode*
  CreateASTStretchNode(const ASTIdentifierNode* Id,
                       const ASTBinaryOpNode* BOP);
  ASTStretchNode*
  CreateASTStretchNode(const ASTIdentifierNode* Id,
                       const ASTUnaryOpNode* UOP);

  ASTBoxExpressionNode*
  CreateASTBoxExpressionNode(const ASTIdentifierNode* Id,
                             const ASTStatementList& SL);
  ASTBoxAsExpressionNode*
  CreateASTBoxAsExpressionNode(const ASTIdentifierNode* Id,
                               const ASTStatementList& SL);
  ASTBoxToExpressionNode*
  CreateASTBoxToExpressionNode(const ASTIdentifierNode* Id,
                               const std::string& Timing,
                               const ASTStatementList& SL);

  ASTGPhaseExpressionNode*
  CreateASTGPhaseExpressionNode(const ASTIdentifierNode* GId,
                                const ASTIdentifierNode* TId);
  ASTGPhaseExpressionNode*
  CreateASTGPhaseExpressionNode(const ASTIdentifierNode* GId,
                                const ASTBinaryOpNode* BOp);
  ASTGPhaseExpressionNode*
  CreateASTGPhaseExpressionNode(const ASTIdentifierNode* GId,
                                const ASTUnaryOpNode* UOp);

  ASTGateQOpNode*
  CreateASTGateControlStatement(const ASTIdentifierNode* Id,
                                const ASTGateControlNode* GCN);
  ASTGateQOpNode*
  CreateASTGateNegControlStatement(const ASTIdentifierNode* Id,
                                   const ASTGateNegControlNode* GNCN);
  ASTGateQOpNode*
  CreateASTGateInverseStatement(const ASTIdentifierNode* Id,
                                const ASTGateInverseNode* GIN);
  ASTGateQOpNode*
  CreateASTGatePowerStatement(const ASTIdentifierNode* Id,
                              const ASTGatePowerNode* GPN);
  ASTGateQOpNode*
  CreateASTGateGPhaseStatement(const ASTIdentifierNode* Id,
                               const ASTGateGPhaseExpressionNode* GGN);

  ASTFunctionCallNode*
  CreateASTFunctionCallNode(const ASTIdentifierNode* Id,
                            const ASTArgumentNodeList* ANL,
                            const ASTAnyTypeList* ATL);
  ASTFunctionCallNode*
  CreateASTFunctionCallNode(const std::string& EId,
                            const ASTFunctionDefinitionNode* FDN,
                            const ASTExpressionList* EL);

  ASTFunctionCallNode*
  CreateASTFunctionCallNode(const std::string& EId,
                            const ASTFunctionDefinitionNode* FDN,
                            const ASTExpressionList* EL,
                            const ASTIdentifierList* IL);

  ASTFunctionCallNode*
  CreateASTFunctionCallNode(const std::string& EId,
                            const ASTKernelNode* KN,
                            const ASTExpressionList* EL);

  ASTFunctionCallNode*
  CreateASTFunctionCallNode(const std::string& EId,
                            const ASTDefcalNode* DN,
                            const ASTExpressionList* EL);

  ASTArrayNode*
  CreateASTCBitArrayNode(const ASTIdentifierNode* Id,
                         const ASTIntNode* IX);
  ASTArrayNode*
  CreateASTCBitArrayNode(const ASTIdentifierNode* Id,
                         unsigned Bits);
  ASTArrayNode*
  CreateASTCBitArrayNode(const ASTIdentifierNode* Id,
                         const ASTIntNode* IX,
                         const ASTIntNode* CX);
  ASTArrayNode*
  CreateASTCBitArrayNode(const ASTIdentifierNode* Id,
                         unsigned Bits, unsigned CBits);
  ASTArrayNode*
  CreateASTQubitArrayNode(const ASTIdentifierNode* Id,
                          const ASTIntNode* IX);
  ASTArrayNode*
  CreateASTQubitArrayNode(const ASTIdentifierNode* Id,
                          unsigned Bits);
  ASTArrayNode*
  CreateASTQubitArrayNode(const ASTIdentifierNode* Id,
                          const ASTIntNode* IX,
                          const ASTIntNode* QX);
  ASTArrayNode*
  CreateASTQubitArrayNode(const ASTIdentifierNode* Id,
                          unsigned Bits, unsigned QBits);
  ASTArrayNode*
  CreateASTAngleArrayNode(const ASTIdentifierNode* Id,
                          const ASTIntNode* IX);
  ASTArrayNode*
  CreateASTAngleArrayNode(const ASTIdentifierNode* Id,
                          unsigned Bits);
  ASTArrayNode*
  CreateASTAngleArrayNode(const ASTIdentifierNode* Id,
                          const ASTIntNode* IX,
                          const ASTIntNode* AX);
  ASTArrayNode*
  CreateASTAngleArrayNode(const ASTIdentifierNode* Id,
                          unsigned Bits, unsigned ABits);
  ASTArrayNode*
  CreateASTBoolArrayNode(const ASTIdentifierNode* Id,
                         const ASTIntNode* IX);
  ASTArrayNode*
  CreateASTBoolArrayNode(const ASTIdentifierNode* Id,
                         unsigned Bits);
  ASTArrayNode*
  CreateASTIntArrayNode(const ASTIdentifierNode* Id,
                        const ASTIntNode* IX,
                        bool Unsigned = false);
  ASTArrayNode*
  CreateASTIntArrayNode(const ASTIdentifierNode* Id,
                        unsigned Bits,
                        bool Unsigned = false);
  ASTArrayNode*
  CreateASTMPIntegerArrayNode(const ASTIdentifierNode* Id,
                              const ASTIntNode* IX,
                              const ASTIntNode* IIX,
                              bool Unsigned = false);
  ASTArrayNode*
  CreateASTMPIntegerArrayNode(const ASTIdentifierNode* Id,
                              unsigned Bits, unsigned MPBits,
                              bool Unsigned = false);
  ASTArrayNode*
  CreateASTFloatArrayNode(const ASTIdentifierNode* Id,
                          const ASTIntNode* IX);
  ASTArrayNode*
  CreateASTFloatArrayNode(const ASTIdentifierNode* Id,
                          unsigned Bits);
  ASTArrayNode*
  CreateASTMPDecimalArrayNode(const ASTIdentifierNode* Id,
                              const ASTIntNode* IX,
                              const ASTIntNode* FX);
  ASTArrayNode*
  CreateASTMPDecimalArrayNode(const ASTIdentifierNode* Id,
                              unsigned Bits, unsigned MPBits);

  ASTArrayNode*
  CreateASTMPComplexArrayNode(const ASTIdentifierNode* Id,
                              const ASTIntNode* IX,
                              const ASTIntNode* CX);
  ASTArrayNode*
  CreateASTMPComplexArrayNode(const ASTIdentifierNode* Id,
                              unsigned Size, unsigned Bits);
  ASTArrayNode*
  CreateASTMPComplexArrayNode(const ASTIdentifierNode* Id,
                              unsigned Size,
                              unsigned Bits,
                              const ASTComplexExpressionNode* CE);

#if 0
  ASTArrayNode*
  CreateASTLengthArrayNode(const ASTIdentifierNode* Id,
                           const ASTIntNode* IX,
                           const std::string& Length);
  ASTArrayNode*
  CreateASTLengthArrayNode(const ASTIdentifierNode* Id,
                           unsigned Bits,
                           const std::string& Length);
#endif

  ASTArrayNode*
  CreateASTDurationArrayNode(const ASTIdentifierNode* Id,
                             const ASTIntNode* IX,
                             const std::string& Length);
  ASTArrayNode*
  CreateASTDurationArrayNode(const ASTIdentifierNode* Id,
                             unsigned Bits,
                             const std::string& Length);
  ASTArrayNode*
  CreateASTDurationArrayNode(const ASTIdentifierNode* Id,
                             unsigned Bits,
                             const ASTDurationOfNode* DON);
  ASTArrayNode*
  CreateASTOpenPulseFrameArrayNode(const ASTIdentifierNode* Id,
                                   unsigned Size);
  ASTArrayNode*
  CreateASTOpenPulsePortArrayNode(const ASTIdentifierNode* Id,
                                  unsigned Size);
  ASTArrayNode*
  CreateASTOpenPulseWaveformArrayNode(const ASTIdentifierNode* Id,
                                      unsigned Size);

  ASTIdentifierRefNode*
  CreateArrayASTIdentifierRefNode(const ASTIdentifierNode* Id,
                                  ASTType AType,
                                  ASTSymbolTableEntry* STE,
                                  unsigned Index);

  ASTEllipsisNode*
  CreateASTEllipsisNode(const ASTIdentifierNode* Id);

  // OpenPulse
  OpenPulse::ASTOpenPulseFrameNode*
  CreateASTOpenPulseFrameNode(const ASTIdentifierNode* Id,
                              const ASTExpressionList* EL,
                              bool Extern = false);

  OpenPulse::ASTOpenPulsePortNode*
  CreateASTOpenPulsePortNode(const ASTIdentifierNode* Id,
                             bool Extern = false);
  OpenPulse::ASTOpenPulsePortNode*
  CreateASTOpenPulsePortNode(const ASTIdentifierNode* Id,
                             uint64_t PId,
                             bool Extern = false);

  OpenPulse::ASTOpenPulseWaveformNode*
  CreateASTOpenPulseWaveformNode(const ASTIdentifierNode* Id,
                                 const ASTMPComplexList& CXL);

  OpenPulse::ASTOpenPulsePlayNode*
  CreateASTOpenPulsePlayNode(const ASTIdentifierNode* Id,
                             const OpenPulse::ASTOpenPulseWaveformNode* W,
                             const OpenPulse::ASTOpenPulseFrameNode* F);
  OpenPulse::ASTOpenPulsePlayNode*
  CreateASTOpenPulsePlayNode(const ASTIdentifierNode* Id,
                             const ASTFunctionCallNode* C,
                             const OpenPulse::ASTOpenPulseFrameNode* F);
  OpenPulse::ASTOpenPulsePlayNode*
  CreateASTOpenPulsePlayNode(const ASTIdentifierNode* Id,
                             const ASTMPComplexList& CXL,
                             const OpenPulse::ASTOpenPulseFrameNode* F);

  OpenPulse::ASTOpenPulseCalibration*
  CreateASTOpenPulseCalibration(const ASTIdentifierNode* Id,
                                const ASTStatementList& OSL);
};

} // namespace QASM

#endif // __QASM_AST_BUILDER_H

