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

#ifndef __QASM_AST_PRODUCTION_FACTORY_H
#define __QASM_AST_PRODUCTION_FACTORY_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTStatement.h>
#include <qasm/AST/ASTStringList.h>
#include <qasm/AST/ASTFunction.h>
#include <qasm/AST/ASTDeclaration.h>
#include <qasm/AST/ASTDeclarationBuilder.h>
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
#include <qasm/AST/ASTQubitConcatBuilder.h>
#include <qasm/AST/ASTReset.h>
#include <qasm/AST/ASTRotate.h>
#include <qasm/AST/ASTFunctions.h>
#include <qasm/AST/ASTKernel.h>
#include <qasm/AST/ASTDefcal.h>
#include <qasm/AST/ASTDefcalBuilder.h>
#include <qasm/AST/ASTDefcalGrammarBuilder.h>
#include <qasm/AST/ASTDelay.h>
#include <qasm/AST/ASTDuration.h>
#include <qasm/AST/ASTLoops.h>
#include <qasm/AST/ASTStretch.h>
#include <qasm/AST/ASTReturn.h>
#include <qasm/AST/ASTBox.h>
#include <qasm/AST/ASTGPhase.h>
#include <qasm/AST/ASTRoot.h>
#include <qasm/AST/ASTPragma.h>
#include <qasm/AST/ASTAnnotation.h>
#include <qasm/AST/ASTCallExpr.h>
#include <qasm/AST/ASTCastExpr.h>
#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTDirectiveStatementNode.h>
#include <qasm/AST/ASTArray.h>
#include <qasm/AST/ASTArraySubscript.h>
#include <qasm/AST/ASTIfConditionals.h>
#include <qasm/AST/ASTCtrlAssocBuilder.h>
#include <qasm/AST/ASTInverseAssocBuilder.h>
#include <qasm/AST/ASTBoxStatementBuilder.h>
#include <qasm/AST/ASTForStatementBuilder.h>
#include <qasm/AST/ASTSwitchStatementBuilder.h>
#include <qasm/AST/ASTWhileStatementBuilder.h>
#include <qasm/AST/ASTDoWhileStatementBuilder.h>
#include <qasm/AST/ASTFlowControl.h>

// OpenPulse
#include <qasm/AST/OpenPulse/ASTOpenPulsePlay.h>
#include <qasm/AST/OpenPulse/ASTOpenPulsePort.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseFrame.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseWaveform.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseCalibration.h>

#include <variant>

namespace QASM {

class ASTProductionFactory {
private:
  static ASTProductionFactory APF;

protected:
  ASTProductionFactory() = default;

protected:
  unsigned GetVariantBits(const std::variant<const ASTIntNode*,
                                             const ASTIdentifierNode*>& V);

public:
  static const std::variant<const ASTIntNode*, const ASTIdentifierNode*> EVX;

public:
  static ASTProductionFactory& Instance() {
    return APF;
  }

  ~ASTProductionFactory() = default;

  ASTDeclarationNode* ProductionRule_100(const ASTToken* TK,
                                         const ASTIdentifierNode* DId) const;
  ASTDeclarationNode* ProductionRule_101(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const std::string& BSV) const;
  ASTDeclarationNode* ProductionRule_102(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const std::string& ISV) const;
  ASTDeclarationNode* ProductionRule_103(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTBinaryOpNode* BOP) const;
  ASTDeclarationNode* ProductionRule_104(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTUnaryOpNode* UOP) const;
  ASTDeclarationNode* ProductionRule_105(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIdentifierNode* Id) const;
  ASTDeclarationNode* ProductionRule_106(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTFunctionCallStatementNode* FC) const;

  // bit
  ASTDeclarationNode* ProductionRule_107(const ASTToken* TK,
                                         const ASTIdentifierNode* Id) const;
  ASTDeclarationNode* ProductionRule_108(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const std::string& IS) const;
  ASTDeclarationNode* ProductionRule_109(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const std::string& IS) const;
  ASTDeclarationNode* ProductionRule_110(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIdentifierNode* Id) const;
  ASTDeclarationNode* ProductionRule_111(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTGateQOpNode* GQO) const;
  ASTDeclarationNode* ProductionRule_112(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTFunctionCallStatementNode* FC) const;

  // int
  ASTDeclarationNode* ProductionRule_120(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         bool Unsigned = false) const;
  ASTDeclarationNode* ProductionRule_121(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_122(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTStatementNode* SN) const;

  // const declaration
  ASTDeclarationNode* ProductionRule_170(ASTDeclarationNode* DN) const;

  // int[n]
  ASTDeclarationNode* ProductionRule_220(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIntNode* I,
                                         bool Unsigned = false) const;
  ASTDeclarationNode* ProductionRule_221(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIdentifierNode* IId,
                                         bool Unsigned = false) const;
  ASTDeclarationNode* ProductionRule_222(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIntNode* I,
                                         const ASTExpressionNode* EN,
                                         bool Unsigned = false) const;
  ASTDeclarationNode* ProductionRule_223(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIntNode* I,
                                         const ASTFunctionCallStatementNode* FC,
                                         bool Unsigned = false) const;
  ASTDeclarationNode* ProductionRule_224(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIdentifierNode* BId,
                                         const ASTExpressionNode* EN,
                                         bool Unsigned = false) const;
  ASTDeclarationNode* ProductionRule_225(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIdentifierNode* BId,
                                         const ASTFunctionCallStatementNode* FC,
                                         bool Unsigned = false) const;

  // uint
  ASTDeclarationNode* ProductionRule_226(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_227(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTStatementNode* SN) const;

  // Integer
  ASTIntNode* ProductionRule_800(const ASTToken* TK,
                                 const std::string& IS) const;

  // Real
  ASTDoubleNode* ProductionRule_801(const ASTToken* TK,
                                    const std::string& RS) const;

  // IntScalarType
  ASTIntNode* ProductionRule_802(const ASTToken* TK,
                                 ASTType ITy) const;

  // FloatScalarType
  ASTFloatNode* ProductionRule_803(const ASTToken* TK) const;

  // MPIntegerType
  ASTMPIntegerNode* ProductionRule_804(const ASTToken* TK,
                                       const ASTIntNode* II,
                                       ASTType ITy) const;
  ASTMPIntegerNode* ProductionRule_805(const ASTToken* TK,
                                       const ASTIdentifierNode* IId,
                                       ASTType ITy) const;

  // MPDecimalType
  ASTMPDecimalNode* ProductionRule_806(const ASTToken* TK,
                                       const ASTIntNode* II) const;
  ASTMPDecimalNode* ProductionRule_807(const ASTToken* TK,
                                       const ASTIdentifierNode* IId) const;

  // MPComplexType
  ASTMPComplexNode* ProductionRule_808(const ASTToken* TK,
                                       const ASTIntNode* II) const;
  ASTMPComplexNode* ProductionRule_809(const ASTToken* TK,
                                       const ASTIdentifierNode* IId) const;

  // Ellipsis
  ASTEllipsisNode* ProductionRule_810(const ASTToken* TK) const;

  // String
  ASTStringNode* ProductionRule_811(const ASTToken* TK,
                                    const std::string& S) const;

  // TimeUnit
  ASTStringNode* ProductionRule_812(const ASTToken* TK,
                                    const std::string& TS) const;

  // Bool literal constant
  ASTBoolNode* ProductionRule_813(const ASTToken* TK) const;

  // Bound / Unbound Qubit
  ASTStringNode* ProductionRule_814(const ASTToken* TK,
                                    ASTType QTy) const;

  // Indexed Bound / Unbound Qubit
  ASTStringNode* ProductionRule_815(const ASTToken* TK,
                                    const ASTArraySubscriptNode* ASN,
                                    ASTType QTy) const;

  // Aggregate Types suffix.
  ASTStringNode* ProductionRule_816(const ASTToken* ITK,
                                    const ASTToken* STK,
                                    bool D = true) const;

  // Bitset
  ASTCBitNode* ProductionRule_817(const ASTToken* TK,
                                  const ASTIdentifierNode* IId) const;
  ASTCBitNode* ProductionRule_817(const ASTToken* TK,
                                  const ASTIntNode* II) const;
  ASTCBitNode* ProductionRule_817(const ASTToken* TK) const;

  // Angle
  ASTAngleNode* ProductionRule_818(const ASTToken* TK,
                                   const ASTIdentifierNode* IId) const;
  ASTAngleNode* ProductionRule_818(const ASTToken* TK,
                                   const ASTIntNode* II) const;
  ASTAngleNode* ProductionRule_818(const ASTToken* TK) const;

  // Qubit
  ASTQubitContainerNode* ProductionRule_819(const ASTToken* TK,
                                            const ASTIdentifierNode* IId) const;
  ASTQubitContainerNode* ProductionRule_819(const ASTToken* TK,
                                            const ASTIntNode* II) const;
  ASTQubitContainerNode* ProductionRule_819(const ASTToken* TK) const;

  // Duration
  ASTDurationNode* ProductionRule_820(const ASTToken* TK) const;
  ASTDurationNode* ProductionRule_820(const ASTToken* TK,
                                      const ASTStringNode* TU) const;

  // Array parameters
  ASTArrayNode* ProductionRule_821(const ASTToken* TK,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   ASTType Ty, bool Unsigned = false) const;

  ASTArrayNode* ProductionRule_821(const ASTToken* TK,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   const ASTStringNode* TS,
                                   ASTType Ty) const;

  ASTArrayNode* ProductionRule_821(const ASTToken* TK,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& DX,
                                   const ASTStringNode* TS,
                                   ASTType Ty) const;

  ASTArrayNode* ProductionRule_821(const ASTToken* TK,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   const ASTDurationOfNode* DON,
                                   ASTType Ty) const;

  ASTArrayNode* ProductionRule_821(const ASTToken* TK,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& DX,
                                   const ASTDurationOfNode* DON,
                                   ASTType Ty) const;

  ASTArrayNode* ProductionRule_821(const ASTToken* TK,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& CX,
                                   ASTType Ty, bool Unsigned = false) const;

  ASTArrayNode* ProductionRule_821(const ASTToken* TK,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& CX,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& DX,
                                   ASTType Ty, bool Unsigned = false) const;

  // Array expressions
  ASTArrayNode* ProductionRule_822(const ASTToken* TK,
                                   const ASTIdentifierNode* Id,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   ASTType Ty, bool Unsigned = false) const;

  ASTArrayNode* ProductionRule_822(const ASTToken* TK,
                                   const ASTIdentifierNode* Id,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   const ASTStringNode* TS,
                                   ASTType Ty) const;

  ASTArrayNode* ProductionRule_822(const ASTToken* TK,
                                   const ASTIdentifierNode* Id,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& DX,
                                   const ASTStringNode* TS,
                                   ASTType Ty) const;

  ASTArrayNode* ProductionRule_822(const ASTToken* TK,
                                   const ASTIdentifierNode* Id,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   const ASTDurationOfNode* DON,
                                   ASTType Ty) const;

  ASTArrayNode* ProductionRule_822(const ASTToken* TK,
                                   const ASTIdentifierNode* Id,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& DX,
                                   const ASTDurationOfNode* DON,
                                   ASTType Ty) const;

  ASTArrayNode* ProductionRule_822(const ASTToken* TK,
                                   const ASTIdentifierNode* Id,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& CX,
                                   ASTType Ty, bool Unsigned = false) const;

  ASTArrayNode* ProductionRule_822(const ASTToken* TK,
                                   const ASTIdentifierNode* Id,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& II,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& CX,
                                   const std::variant<const ASTIntNode*,
                                                      const ASTIdentifierNode*>& DX,
                                   ASTType Ty, bool Unsigned = false) const;

  // Named Type Declaration
  ASTDeclarationNode* ProductionRule_850(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         ASTType Ty,
                                         bool Const = false) const;
  ASTDeclarationNode* ProductionRule_850(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTStringNode* TS,
                                         ASTType Ty,
                                         bool Const = false) const;
  ASTDeclarationNode* ProductionRule_850(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const std::variant<const ASTIntNode*,
                                               const ASTIdentifierNode*>& II,
                                         ASTType Ty,
                                         bool Const = false) const;
  ASTDeclarationNode* ProductionRule_850(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         unsigned Size, ASTArrayNode* ARN,
                                         ASTType Ty,
                                         bool Const = false) const;
  ASTDeclarationNode* ProductionRule_850(const ASTToken* TK,
                                         ASTEllipsisNode* EL) const;
  ASTDeclarationNode* ProductionRule_850(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         bool Const = false) const;

  // For Loop Range Expression
  ASTForLoopRangeExpressionNode*
  ProductionRule_860(const ASTToken* TK, const ASTIdentifierNode* Id,
                     const ASTIntegerList* IL,
                     const std::variant<const ASTIntNode*,
                                const ASTIdentifierNode*>& II,
                     ASTOpType OTy) const;

  // GPhase Expression
  ASTGPhaseExpressionNode* ProductionRule_870(const ASTToken* TK,
                                              const ASTIdentifierNode* TId) const;
  ASTGPhaseExpressionNode* ProductionRule_870(const ASTToken* TK,
                                              const ASTBinaryOpNode* BOP) const;
  ASTGPhaseExpressionNode* ProductionRule_870(const ASTToken* TK,
                                              const ASTUnaryOpNode* UOP) const;

  // Complex Initializer Expression
  ASTComplexExpressionNode* ProductionRule_880(const ASTToken* TK,
                                               const ASTBinaryOpNode* BOP) const;
  ASTComplexExpressionNode* ProductionRule_880(const ASTToken* TK,
                                               const ASTUnaryOpNode* UOP) const;

  ASTDeclarationNode* ProductionRule_1000(const ASTToken* TK,
                                          const ASTIdentifierNode* QId) const;
  ASTDeclarationNode* ProductionRule_1001(const ASTToken* TK,
                                          const ASTIdentifierNode* Id) const;
  ASTDeclarationNode* ProductionRule_1002(const ASTToken* TK,
                                          const ASTIdentifierNode* Id,
                                          const ASTIntNode* I) const;
  ASTDeclarationNode* ProductionRule_1003(const ASTToken* TK,
                                          const ASTIdentifierNode* Id,
                                          const ASTIntNode* I) const;
  ASTDeclarationNode* ProductionRule_1004(const ASTToken* TK,
                                          const ASTIdentifierNode* Id,
                                          const ASTIntNode* BI,
                                          const ASTIntNode* I) const;
  ASTDeclarationNode* ProductionRule_1005(const ASTToken* TK,
                                          const ASTIdentifierNode* Id,
                                          const ASTIntNode* I,
                                          const std::string& SBM) const;

  // qubit
  ASTDeclarationNode* ProductionRule_1100(const ASTToken* TK,
                                          const ASTIdentifierNode* DId) const;
  ASTDeclarationNode* ProductionRule_1103(const ASTToken* TK,
                                          const ASTIdentifierNode* DId,
                                          const ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_1104(const ASTToken* TK,
                                          const ASTIdentifierNode* DId,
                                          const ASTFunctionCallStatementNode* FC) const;

  // qubit[n]
  ASTDeclarationNode* ProductionRule_1101(const ASTToken* TK,
                                          const ASTIdentifierNode* DId,
                                          const ASTIntNode* BI) const;
  ASTDeclarationNode* ProductionRule_1102(const ASTToken* TK,
                                          const ASTIdentifierNode* DId,
                                          const ASTIdentifierNode* IXd) const;
  ASTDeclarationNode* ProductionRule_1105(const ASTToken* TK,
                                          const ASTIdentifierNode* DId,
                                          const ASTIntNode* IXd,
                                          const ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_1106(const ASTToken* TK,
                                          const ASTIdentifierNode* DId,
                                          const ASTIdentifierNode* IXd,
                                          const ASTExpressionNode* EN) const;

  // qubit aliases
  ASTDeclarationNode* ProductionRule_1150(const ASTIdentifierNode* AId,
                                          const std::string* QN,
                                          const std::string* FIX,
                                          ASTIntegerList* IL) const;
  ASTDeclarationNode* ProductionRule_1151(const ASTIdentifierNode* AId,
                                          const std::string* QN,
                                          const std::string* FIX,
                                          ASTIntegerList* IL) const;
  ASTDeclarationNode* ProductionRule_1152(const ASTIdentifierNode* AId,
                                          const std::string* QN,
                                          ASTArraySubscriptNode* ASN) const;
  ASTDeclarationNode* ProductionRule_1153(const ASTIdentifierNode* AId,
                                          const ASTIdentifierNode* QId,
                                          ASTQubitConcatList* QCL) const;

  // float
  ASTDeclarationNode* ProductionRule_140(const ASTToken* TK,
                                         const ASTIdentifierNode* Id) const;
  ASTDeclarationNode* ProductionRule_141(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_142(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTFunctionCallStatementNode* FC) const;

  // float[n]
  ASTDeclarationNode* ProductionRule_143(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTIntNode* II) const;
  ASTDeclarationNode* ProductionRule_144(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTIdentifierNode* IId) const;
  ASTDeclarationNode* ProductionRule_145(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIntNode* II,
                                         const ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_146(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIntNode* II,
                                         const ASTFunctionCallStatementNode* FC) const;
  ASTDeclarationNode* ProductionRule_147(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIdentifierNode* IId,
                                         const ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_148(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIdentifierNode* BId,
                                         const ASTFunctionCallStatementNode* FC) const;

  // double
  ASTDeclarationNode* ProductionRule_150(const ASTToken* TK,
                                         const ASTIdentifierNode* Id) const;
  ASTDeclarationNode* ProductionRule_151(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTExpressionNode* EN) const;

  // const
  ASTDeclarationNode* ProductionRule_170(const ASTToken* TK,
                                         const std::string& SId,
                                         ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_171(const ASTToken* TK,
                                         const std::string& SId,
                                         const ASTIntNode* II,
                                         ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_172(const ASTToken* TK,
                                         const std::string& SId,
                                         const ASTIdentifierNode* IId,
                                         ASTExpressionNode* EN) const;

  // angle
  ASTDeclarationNode* ProductionRule_180(const ASTToken* TK,
                                         const ASTIdentifierNode* Id) const;
  ASTDeclarationNode* ProductionRule_181(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_182(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTFunctionCallStatementNode* FC) const;

  // bit[n]
  ASTDeclarationNode* ProductionRule_200(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIntNode* II) const;
  ASTDeclarationNode* ProductionRule_201(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTIdentifierNode* IId) const;
  ASTDeclarationNode* ProductionRule_202(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTIntNode* II,
                                         const ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_203(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIntNode* IIX,
                                         const ASTFunctionCallStatementNode* FC) const;
  ASTDeclarationNode* ProductionRule_204(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTIdentifierNode* IXd,
                                         const ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_205(const ASTToken* TK,
                                         const ASTIdentifierNode* DId,
                                         const ASTIdentifierNode* IXd,
                                         const ASTFunctionCallStatementNode* FC) const;

  // angle[n]
  ASTDeclarationNode* ProductionRule_240(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTIntNode* II) const;
  ASTDeclarationNode* ProductionRule_241(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTIdentifierNode* IId) const;
  ASTDeclarationNode* ProductionRule_242(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTIntNode* II,
                                         const ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_243(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTIntNode* II,
                                         const ASTFunctionCallStatementNode* FC) const;
  ASTDeclarationNode* ProductionRule_244(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTIdentifierNode* IId,
                                         const ASTExpressionNode* EN) const;
  ASTDeclarationNode* ProductionRule_245(const ASTToken* TK,
                                         const ASTIdentifierNode* Id,
                                         const ASTIdentifierNode* IId,
                                         const ASTFunctionCallStatementNode* FC) const;

  // complex[n]
  ASTMPComplexNode* ProductionRule_250(const ASTToken* TK,
                                       const ASTIdentifierNode* Id) const;
  ASTMPComplexNode* ProductionRule_251(const ASTToken* TK,
                                       const ASTIdentifierNode* Id,
                                       const ASTIntNode* SI) const;
  ASTMPComplexNode* ProductionRule_252(const ASTToken* TK,
                                       const ASTIdentifierNode* Id,
                                       const ASTIdentifierNode* SId) const;
  ASTMPComplexNode* ProductionRule_253(const ASTToken* TK,
                                       const ASTIdentifierNode* Id,
                                       const ASTIntNode* SI,
                                       const ASTComplexExpressionNode* CEN) const;
  ASTMPComplexNode* ProductionRule_254(const ASTToken* TK,
                                       const ASTIdentifierNode* Id,
                                       const ASTIntNode* SI,
                                       ASTBinaryOpNode* BOP) const;
  ASTMPComplexNode* ProductionRule_255(const ASTToken* TK,
                                       const ASTIdentifierNode* Id,
                                       const ASTIntNode* SI,
                                       ASTUnaryOpNode* UOP) const;
  ASTMPComplexNode* ProductionRule_256(const ASTToken* TK,
                                       const ASTIdentifierNode* Id,
                                       const ASTIdentifierNode* SId,
                                       const ASTComplexExpressionNode* CEN) const;
  ASTMPComplexNode* ProductionRule_257(const ASTToken* TK,
                                       const ASTIdentifierNode* Id,
                                       const ASTIdentifierNode* SId,
                                       ASTBinaryOpNode* BOP) const;
  ASTMPComplexNode* ProductionRule_258(const ASTToken* TK,
                                       const ASTIdentifierNode* Id,
                                       const ASTIdentifierNode* SId,
                                       ASTUnaryOpNode* UOP) const;
  ASTMPComplexNode* ProductionRule_259(const ASTToken* TK,
                                       const ASTIdentifierNode* Id,
                                       const ASTIntNode* SI,
                                       const ASTIdentifierNode* RId) const;
  ASTMPComplexNode* ProductionRule_260(const ASTToken* TK,
                                       const ASTIdentifierNode* Id,
                                       const ASTIdentifierNode* SId,
                                       const ASTIdentifierNode* RId) const;
  ASTMPComplexNode* ProductionRule_261(const ASTToken* TK,
                                       const ASTIdentifierNode* Id,
                                       const ASTIntNode* SI,
                                       const ASTFunctionCallStatementNode* FC) const;
  ASTMPComplexNode* ProductionRule_262(const ASTToken* TK,
                                       const ASTIdentifierNode* Id,
                                       const ASTIdentifierNode* SId,
                                       const ASTFunctionCallStatementNode* FC) const;

  // Unary Ops
  template<typename __Type>
  ASTUnaryOpNode* ProductionRule_300(const ASTToken* TK,
                                     const __Type* TP,
                                     ASTOpType OTy) const;

  // Pow Binary Ops
  template<typename __Base, typename __Exponent>
  ASTBinaryOpNode* ProductionRule_310(const ASTToken* TK,
                                      const __Base* B,
                                      const __Exponent* E) const;

  // Logical Not
  template<typename __Type>
  ASTUnaryOpNode* ProductionRule_340(const ASTToken* TK,
                                     const __Type* TP) const;

  // Arithmetic Negative / Positive
  template<typename __Type>
  ASTUnaryOpNode* ProductionRule_350(const ASTToken* TK,
                                     const __Type* TP,
                                     ASTOpType OTy,
                                     bool Parens = false) const;

  // Bit Rotation
  ASTRotateNode* ProductionRule_310(const ASTToken* TK,
                                    const std::variant<const ASTIntNode*,
                                               const ASTIdentifierNode*,
                                               const ASTCBitNode*>& LII,
                                    const std::variant<const ASTIntNode*,
                                               const ASTIdentifierNode*>& RII,
                                    ASTRotationType RoTy) const;
  ASTRotateStatementNode* ProductionRule_310(const ASTToken* TK,
                                             const ASTRotateNode* RN) const;

  // Popcount
  ASTPopcountNode* ProductionRule_320(const ASTToken* TK,
                                      const ASTIntNode* I) const;
  ASTPopcountNode* ProductionRule_320(const ASTToken* TK,
                                      const ASTIdentifierNode* TId) const;
  ASTPopcountStatementNode* ProductionRule_320(const ASTToken* TK,
                                               const ASTPopcountNode* PN) const;

  // Binary Op Assignment
  ASTBinaryOpNode* ProductionRule_450(const ASTToken* TK,
                                      const ASTIdentifierNode* Id,
                                      const ASTExpressionNode* EX) const;
  ASTBinaryOpNode* ProductionRule_451(const ASTToken* TK,
                                      const ASTIdentifierNode* Id,
                                      const ASTFunctionCallNode* FCN) const;
  ASTBinaryOpNode* ProductionRule_452(const ASTToken* TK,
                                      const ASTIdentifierNode* Id,
                                      const ASTComplexExpressionNode* CEX) const;
  ASTBinaryOpNode* ProductionRule_453(const ASTToken* TK,
                                      const ASTIdentifierNode* Id,
                                      const ASTBinaryOpNode* BOP) const;
  ASTBinaryOpNode* ProductionRule_454(const ASTToken* TK,
                                      const ASTIdentifierNode* Id,
                                      const ASTRotateNode* RN) const;
  ASTBinaryOpNode* ProductionRule_455(const ASTToken* TK,
                                      const ASTIdentifierNode* Id,
                                      const ASTPopcountNode* PN) const;
  ASTBinaryOpNode* ProductionRule_456(const ASTToken* TK,
                                      const ASTIdentifierNode* Id,
                                      const ASTStringNode* TS) const;
  ASTBinaryOpNode* ProductionRule_457(const ASTToken* TK,
                                      const ASTIdentifierNode* Id,
                                      const ASTBoolNode* BN) const;

  // binary op arithmetic
  ASTBinaryOpNode* ProductionRule_580(const ASTToken* TK,
                                      const ASTExpressionNode* LXN,
                                      const ASTExpressionNode* RXN,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_581(const ASTToken* TK,
                                      const ASTIdentifierNode* LId,
                                      const ASTExpressionNode* RXN,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_582(const ASTToken* TK,
                                      const ASTExpressionNode* LXN,
                                      const ASTIdentifierNode* RId,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_583(const ASTToken* TK,
                                      const ASTIdentifierNode* LId,
                                      const ASTIdentifierNode* RId,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_584(const ASTToken* TK,
                                      const ASTExpressionNode* LXN,
                                      const ASTFunctionCallNode* FCN,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_585(const ASTToken* TK,
                                      const ASTIdentifierNode* LId,
                                      const ASTFunctionCallNode* FCN,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_586(const ASTToken* TK,
                                      const ASTIdentifierNode* LId,
                                      const ASTBinaryOpNode* RBOP,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_587(const ASTToken* TK,
                                      const ASTIdentifierNode* LId,
                                      const ASTUnaryOpNode* RUOP,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_588(const ASTToken* TK,
                                      const ASTExpressionNode* LXN,
                                      const ASTBinaryOpNode* RBOP,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_589(const ASTToken* TK,
                                      const ASTExpressionNode* LXN,
                                      const ASTUnaryOpNode* RUOP,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_590(const ASTToken* TK,
                                      const ASTBinaryOpNode* LBOP,
                                      const ASTBinaryOpNode* RBOP,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_591(const ASTToken* TK,
                                      const ASTBinaryOpNode* LBOP,
                                      const ASTUnaryOpNode* RUOP,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_592(const ASTToken* TK,
                                      const ASTUnaryOpNode* LUOP,
                                      const ASTBinaryOpNode* RBOP,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_593(const ASTToken* TK,
                                      const ASTUnaryOpNode* LUOP,
                                      const ASTUnaryOpNode* RUOP,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_594(const ASTToken* TK,
                                      const ASTBinaryOpNode* LBOP,
                                      const ASTExpressionNode* RXN,
                                      ASTOpType OTy) const;

  // binary op pre-dec / pre-inc
  ASTBinaryOpNode* ProductionRule_600(const ASTToken* TK,
                                      const ASTIdentifierNode* Id,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_601(const ASTToken* TK,
                                      const ASTExpressionNode* EX,
                                      ASTOpType OTy) const;

  // binary op post-dec / post-inc
  ASTBinaryOpNode* ProductionRule_602(const ASTToken* TK,
                                      const ASTIdentifierNode* Id,
                                      ASTOpType OTy) const;
  ASTBinaryOpNode* ProductionRule_603(const ASTToken* TK,
                                      const ASTExpressionNode* EX,
                                      ASTOpType OTy) const;

  // binary op statement
  ASTBinaryOpStatementNode*
  ProductionRule_605(const ASTToken* TK, ASTBinaryOpNode* BOP) const;

  // duration
  ASTDurationNode* ProductionRule_1200(const ASTToken* TK,
                                       ASTIdentifierNode* DId,
                                       ASTStringNode* TS) const;
  ASTDurationNode* ProductionRule_1201(const ASTToken* TK,
                                       ASTIdentifierNode* DId,
                                       ASTDurationOfNode* DON) const;
  ASTDurationNode* ProductionRule_1202(const ASTToken* TK,
                                       ASTIdentifierNode* DId,
                                       ASTIdentifierNode* Id,
                                       ASTStringNode* TS) const;
  ASTDurationNode* ProductionRule_1203(const ASTToken* TK,
                                       ASTIdentifierNode* DId,
                                       ASTIdentifierNode* Id) const;
  ASTDurationNode* ProductionRule_1204(const ASTToken* TK,
                                       ASTIdentifierNode* DId,
                                       ASTBinaryOpNode* BOP) const;
  ASTDurationNode* ProductionRule_1205(const ASTToken* TK,
                                       ASTIdentifierNode* DId,
                                       ASTFunctionCallStatementNode* FC) const;
  ASTDeclarationNode* ProductionRule_1206(const ASTToken* TK,
                                          ASTDurationNode* DN) const;
  ASTDeclarationNode* ProductionRule_1207(const ASTToken*TK,
                                          ASTDurationOfNode* DON) const;

  // implicit duration
  ASTDurationNode* ProductionRule_1209(const ASTToken* TK,
                                       const ASTStringNode* DSN) const;

  // durationof
  ASTDurationOfNode* ProductionRule_1250(const ASTToken* TK,
                                         const ASTIdentifierNode* TId) const;
  ASTDurationOfNode* ProductionRule_1251(const ASTToken* TK,
                                         ASTGateQOpNode* QOP) const;

  // Delay
  ASTDelayNode* ProductionRule_1300(const ASTToken* TK,
                                    const ASTIdentifierNode* Id) const;
  ASTDelayNode* ProductionRule_1300(const ASTToken* TK,
                                    const ASTStringNode* TS,
                                    const ASTIdentifierList* IL) const;
  ASTDelayNode* ProductionRule_1300(const ASTToken* TK,
                                    const ASTStringNode* TS) const;
  ASTDelayNode* ProductionRule_1300(const ASTToken* TK,
                                    const ASTDurationOfNode* DON) const;
  ASTDelayNode* ProductionRule_1300(const ASTToken* TK,
                                    const ASTDurationOfNode* DON,
                                    const ASTIdentifierList* IL) const;
  ASTDelayNode* ProductionRule_1300(const ASTToken* TK,
                                    const ASTIdentifierNode* DId,
                                    const ASTIdentifierList* IL) const;
  ASTDelayNode* ProductionRule_1300(const ASTToken* TK,
                                    const ASTBinaryOpNode* BOP) const;
  ASTDelayNode* ProductionRule_1300(const ASTToken* TK,
                                    const ASTUnaryOpNode* UOP) const;
  ASTDelayNode* ProductionRule_1300(const ASTToken* TK,
                                    const ASTIdentifierNode* TId,
                                    const ASTIdentifierNode* QId) const;
  ASTDelayStatementNode* ProductionRule_1300(const ASTToken* TK,
                                             const ASTDelayNode* DN) const;

  // Stretch
  ASTStretchNode* ProductionRule_1350(const ASTToken* TK,
                                      const ASTIdentifierNode* DId) const;
  template<typename __Type>
  ASTStretchNode* ProductionRule_1350(const ASTToken* TK,
                                      const ASTIdentifierNode* DId,
                                      const __Type* TP) const;
  template<typename __Type>
  ASTStretchNode* ProductionRule_1350(const ASTToken* TK,
                                      const ASTIdentifierNode* DId,
                                      const ASTIntNode* II) const;
  ASTStretchStatementNode* ProductionRule_1350(const ASTToken* TK,
                                               const ASTStretchNode* SN) const;

  // Box
  ASTBoxStatementNode* ProductionRule_1400(const ASTToken* TK,
                                           const ASTIdentifierNode* Id,
                                           const ASTStatementList* SL) const;
  // BoxAs
  ASTBoxStatementNode* ProductionRule_1401(const ASTToken* TK,
                                           const ASTIdentifierNode* Id,
                                           const ASTStatementList* SL) const;
  // BoxTo
  ASTBoxStatementNode* ProductionRule_1402(const ASTToken* TK,
                                           const ASTStringNode* TS,
                                           const ASTStatementList* SL) const;

  // Gates
  ASTGateDeclarationNode* ProductionRule_1430(const ASTToken* TK,
                                              const ASTIdentifierNode* GId,
                                              ASTDeclarationList* DL,
                                              ASTIdentifierList* QIL,
                                              ASTGateQOpList* GOL) const;
  ASTGateDeclarationNode* ProductionRule_1431(const ASTToken* TK,
                                              const ASTIdentifierNode* GId,
                                              ASTIdentifierList* QIL,
                                              ASTGateQOpList* GOL) const;
  ASTGateDeclarationNode* ProductionRule_1432(const ASTToken* TK,
                                              ASTIdentifierList* QIL,
                                              const ASTLocation& Loc,
                                              ASTGateQOpList* GL) const;
  ASTGateDeclarationNode* ProductionRule_1433(const ASTToken* TK,
                                              const std::string* GS,
                                              const ASTLocation& Loc,
                                              ASTIdentifierList* QIL,
                                              ASTGateQOpList* GL) const;
  ASTGateDeclarationNode* ProductionRule_1434(const ASTToken* TK,
                                              const std::string* GS,
                                              const ASTLocation& Loc,
                                              ASTDeclarationList* DL,
                                              ASTIdentifierList* QIL,
                                              ASTGateQOpList* GOL) const;

  // Opaque
  ASTGateDeclarationNode* ProductionRule_1435(const ASTToken* TK,
                                              const ASTIdentifierNode* GId,
                                              ASTDeclarationList* DL,
                                              ASTIdentifierList* QIL) const;
  ASTGateDeclarationNode* ProductionRule_1436(const ASTToken* TK,
                                              const ASTIdentifierNode* GId,
                                              ASTIdentifierList* QIL) const;

  // Defcals
  ASTDefcalDeclarationNode* ProductionRule_1440(const ASTToken* TK,
                                                const ASTIdentifierNode* Id,
                                                ASTExpressionList* DEL,
                                                ASTBoundQubitList* BQL,
                                                ASTStatementList* DSL,
                                                const ASTStringNode* GS = nullptr) const;
  ASTDefcalDeclarationNode* ProductionRule_1441(const ASTToken* TK,
                                                const ASTIdentifierNode* Id,
                                                ASTBoundQubitList* BQL,
                                                ASTStatementList* DSL,
                                                const ASTStringNode* GS = nullptr) const;
  ASTDefcalDeclarationNode* ProductionRule_1442(const ASTMeasureNode* MN,
                                                const ASTToken* TK,
                                                ASTStatementList* DSL,
                                                const ASTStringNode* GS = nullptr) const;
  ASTDefcalDeclarationNode* ProductionRule_1443(const ASTToken* DTK,
                                                const ASTToken* RTK,
                                                ASTIdentifierNode* QId,
                                                ASTStatementList* DSL,
                                                const ASTStringNode* GS = nullptr) const;
  ASTDefcalDeclarationNode* ProductionRule_1444(const ASTToken* DTK,
                                                const ASTToken* DLTK,
                                                const ASTStringNode* TS,
                                                ASTBoundQubitList* QL,
                                                ASTStatementList* DSL,
                                                const ASTStringNode* GS = nullptr) const;
  ASTDefcalDeclarationNode* ProductionRule_1445(const ASTToken* DTK,
                                                const ASTToken* DLTK,
                                                ASTDurationOfNode* DRN,
                                                ASTBoundQubitList* QL,
                                                ASTStatementList* DSL,
                                                const ASTStringNode* GS = nullptr) const;
  ASTDefcalDeclarationNode* ProductionRule_1445(const ASTToken* DTK,
                                                const ASTToken* DRTK,
                                                const ASTIdentifierNode* DId,
                                                ASTBoundQubitList* QL,
                                                ASTStatementList* DSL,
                                                const ASTStringNode* GS = nullptr) const;

  // Defcal Grammar
  ASTDefcalGrammarNode* ProductionRule_1445(const ASTToken* TK,
                                            ASTStringNode* GS) const;

  // Barrier
  ASTBarrierNode* ProductionRule_1450(const ASTToken* TK,
                                      const ASTIdentifierList* IL) const;
  ASTBarrierNode* ProductionRule_1451(const ASTToken* TK) const;

  // Measure
  ASTMeasureNode* ProductionRule_1460(const ASTToken* TK,
                                      const ASTIdentifierNode* Id) const;
  ASTMeasureNode* ProductionRule_1461(const ASTToken* TK,
                                      const ASTIdentifierNode* TId,
                                      const ASTIdentifierNode* RId) const;
  ASTMeasureNode* ProductionRule_1462(const ASTToken* TK,
                                      const ASTIdentifierNode* TId) const;
  ASTMeasureNode* ProductionRule_1463(const ASTToken* TK,
                                      const std::string& TS,
                                      const ASTToken* TSTK,
                                      const ASTIntNode* TIMin,
                                      const ASTIntNode* TIMax,
                                      const std::string& RS,
                                      const ASTToken* RSTK,
                                      const ASTIntNode* RIMin,
                                      const ASTIntNode* RIMax) const;
  ASTMeasureNode* ProductionRule_1464(const ASTToken* TK,
                                      const ASTIdentifierNode* TId,
                                      const std::string& BS) const;

  // Reset
  ASTResetNode* ProductionRule_1480(const ASTToken* TK,
                                    const ASTIdentifierNode* TId) const;

  // Identifiers
  ASTIdentifierNode* ProductionRule_1500(const ASTToken* TK,
                                         const std::string& TS) const;
  ASTIdentifierNode* ProductionRule_1501(const ASTToken* TK,
                                         const ASTStringNode* BQS) const;
  ASTIdentifierNode* ProductionRule_1502(const ASTToken* TK,
                                         const ASTStringNode* UQS) const;
  ASTIdentifierNode* ProductionRule_1503(const ASTToken* TK,
                                         const ASTStringNode* SN,
                                         const char* P) const;
  ASTIdentifierNode* ProductionRule_1504(const ASTToken* TK,
                                         const ASTStringNode* SN) const;
  ASTIdentifierNode* ProductionRule_1505(const ASTToken* TK,
                                         const ASTArraySubscriptNode* ASN,
                                         const ASTArraySubscriptList* ASL,
                                         const std::string& IS) const;
  ASTIdentifierNode* ProductionRule_1506(const ASTToken* TK,
                                         const ASTStringNode* BQS,
                                         const ASTArraySubscriptNode* ASN,
                                         const ASTArraySubscriptList* ASL) const;
  ASTIdentifierNode* ProductionRule_1507(const ASTToken* TK,
                                         const std::string& N) const;

  // Array Subscript.
  ASTArraySubscriptNode* ProductionRule_1520(const ASTToken* TK,
                                             const ASTIntNode* II) const;
  ASTArraySubscriptNode* ProductionRule_1520(const ASTToken* TK,
                                             const ASTIdentifierNode* IId) const;
  ASTArraySubscriptNode* ProductionRule_1520(const ASTToken* TK,
                                             const ASTBinaryOpNode* BOP) const;
  ASTArraySubscriptNode* ProductionRule_1520(const ASTToken* TK,
                                             const ASTUnaryOpNode* UOP) const;

  // OpenPulse Frame
  OpenPulse::ASTOpenPulseFrameNode*
  ProductionRule_1700(const ASTToken* TKF,
                      const ASTIdentifierNode* Id,
                      const ASTExpressionList* EL) const;
  OpenPulse::ASTOpenPulseFrameNode*
  ProductionRule_1701(const ASTToken* TKF,
                      const ASTIdentifierNode* Id) const;
  OpenPulse::ASTOpenPulseFrameNode*
  ProductionRule_1702(const ASTToken* TKF,
                      const ASTExpressionList* EL) const;

  // OpenPulse Port
  OpenPulse::ASTOpenPulsePortNode*
  ProductionRule_1710(const ASTToken* TK,
                      const ASTIdentifierNode* Id) const;

  // OpenPulse Waveform
  OpenPulse::ASTOpenPulseWaveformNode*
  ProductionRule_1720(const ASTToken* TK,
                      const ASTIdentifierNode* Id,
                      const ASTExpressionList* EL) const;
  OpenPulse::ASTOpenPulseWaveformNode*
  ProductionRule_1721(const ASTToken* TK,
                      const ASTIdentifierNode* Id,
                      const ASTFunctionCallStatementNode* FC) const;

  // OpenPulse Play
  OpenPulse::ASTOpenPulsePlayNode*
  ProductionRule_1730(const ASTToken* TK,
                      const ASTIdentifierNode* FId,
                      const ASTExpressionList* EL) const;
  OpenPulse::ASTOpenPulsePlayNode*
  ProductionRule_1731(const ASTToken* TK,
                      const ASTExpressionList* EL,
                      const OpenPulse::ASTOpenPulseFrameNode* FN) const;
  OpenPulse::ASTOpenPulsePlayNode*
  ProductionRule_1732(const ASTToken* TK,
                      const OpenPulse::ASTOpenPulseWaveformNode* WN,
                      const ASTIdentifierNode* FId) const;
  OpenPulse::ASTOpenPulsePlayNode*
  ProductionRule_1733(const ASTToken* TK,
                      const ASTIdentifierNode* WId,
                      const ASTIdentifierNode* FId) const;
  OpenPulse::ASTOpenPulsePlayNode*
  ProductionRule_1734(const ASTToken* TK,
                      const ASTIdentifierNode* WId,
                      const OpenPulse::ASTOpenPulseFrameNode* FN) const;
  OpenPulse::ASTOpenPulsePlayNode*
  ProductionRule_1735(const ASTToken* TK,
                      const OpenPulse::ASTOpenPulseWaveformNode* WN,
                      const OpenPulse::ASTOpenPulseFrameNode* FN) const;
  OpenPulse::ASTOpenPulsePlayNode*
  ProductionRule_1736(const ASTToken* TK,
                      const ASTFunctionCallNode* FCN,
                      const ASTIdentifierNode* FId) const;
  OpenPulse::ASTOpenPulsePlayNode*
  ProductionRule_1737(const ASTToken* TK,
                      const ASTFunctionCallNode* FCN,
                      const OpenPulse::ASTOpenPulseFrameNode* FN) const;

  // OpenPulse Calibration
  OpenPulse::ASTOpenPulseCalibration*
  ProductionRule_1740(const ASTToken* TK,
                      ASTStatementList* OPSL) const;

  // OpenPulse Declaration
  ASTDeclarationNode*
  ProductionRule_1750(const ASTToken* TK,
                      const OpenPulse::ASTOpenPulseFrameNode* FN) const;
  ASTDeclarationNode*
  ProductionRule_1751(const ASTToken* TK,
                      const OpenPulse::ASTOpenPulsePortNode* PN) const;
  ASTDeclarationNode*
  ProductionRule_1752(const ASTToken* TK,
                      const OpenPulse::ASTOpenPulseWaveformNode* WFN) const;

  // OpenPulse Statement
  OpenPulse::ASTOpenPulsePlayStatementNode*
  ProductionRule_1760(const ASTToken* TK,
                      const OpenPulse::ASTOpenPulsePlayNode* PN) const;
  OpenPulse::ASTOpenPulseCalibrationStmt*
  ProductionRule_1761(const ASTToken* TK,
                      const OpenPulse::ASTOpenPulseCalibration* CAL) const;

  // Return Statement
  ASTReturnStatementNode*
  ProductionRule_2000(const ASTToken* TK, ASTExpressionNode* EN) const;

  ASTReturnStatementNode*
  ProductionRule_2001(const ASTToken* TK, ASTIdentifierNode* Id) const;

  ASTReturnStatementNode*
  ProductionRule_2002(const ASTToken* TK, ASTBinaryOpNode* BOP) const;

  ASTReturnStatementNode*
  ProductionRule_2003(const ASTToken* TK,
                      ASTFunctionCallStatementNode* FSN) const;

  ASTReturnStatementNode*
  ProductionRule_2004(const ASTToken* TK, ASTMeasureNode* MN) const;

  ASTReturnStatementNode* ProductionRule_2005(const ASTToken* TK) const;

  ASTReturnStatementNode*
  ProductionRule_2006(const ASTToken* TK, const std::string& B) const;

  // Result
  ASTResultNode* ProductionRule_2100(const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2101(const ASTIdentifierNode* IId,
                                     const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2102(const ASTIntNode* II,
                                     const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2103(const ASTBinaryOpNode* BOP,
                                     const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2104(const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2105(const ASTIdentifierNode* IId,
                                     const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2106(const ASTBinaryOpNode* BOP,
                                     const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2107(const std::string* IS,
                                     const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2108(const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2109(const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2110(const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2111(const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2112(const ASTMPIntegerNode* MPI) const;
  ASTResultNode* ProductionRule_2113(const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2114(const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2115(const ASTMPDecimalNode* MPD) const;
  ASTResultNode* ProductionRule_2116(const ASTMPComplexNode* MPC) const;
  ASTResultNode* ProductionRule_2117(const ASTToken* TK) const;
  ASTResultNode* ProductionRule_2118(const ASTToken* TK) const;
  ASTDeclarationNode* ProductionRule_2150(const ASTToken* TK,
                                          const ASTResultNode* RN) const;

  // Functions
  ASTFunctionDeclarationNode* ProductionRule_2300(const ASTToken* TK,
                                                  const ASTIdentifierNode* Id,
                                                  ASTDeclarationList* DL,
                                                  ASTResultNode* Res,
                                                  ASTStatementList* SL) const;
  ASTFunctionDeclarationNode* ProductionRule_2301(const ASTToken* TK,
                                                  const ASTIdentifierNode* Id,
                                                  ASTResultNode* Res,
                                                  ASTStatementList* SL) const;
  ASTFunctionDeclarationNode* ProductionRule_2302(const ASTToken* TK,
                                                  const ASTIdentifierNode* Id,
                                                  ASTDeclarationList* DL,
                                                  ASTStatementList* SL) const;
  ASTFunctionDeclarationNode* ProductionRule_2303(const ASTToken* TK,
                                                  const ASTIdentifierNode* Id,
                                                  ASTStatementList* SL) const;

  // Extern
  ASTKernelDeclarationNode* ProductionRule_2500(const ASTToken* ETK,
                                                ASTIdentifierNode* Id,
                                                ASTDeclarationList* DL,
                                                ASTResultNode* Res,
                                                const ASTToken* RTK) const;
  ASTKernelDeclarationNode* ProductionRule_2501(const ASTToken* ETK,
                                                ASTIdentifierNode* Id,
                                                ASTDeclarationList* DL,
                                                const ASTToken* RTK) const;
  ASTKernelDeclarationNode* ProductionRule_2502(const ASTToken* ETK,
                                                const ASTIdentifierNode* Id,
                                                const ASTToken* RTK) const;
  ASTKernelDeclarationNode* ProductionRule_2503(const ASTToken* ETK,
                                                ASTIdentifierNode* Id,
                                                ASTResultNode* Res,
                                                const ASTToken* RTK) const;
  ASTKernelDeclarationNode* ProductionRule_2504(const ASTToken* ETK,
                                                ASTIdentifierNode* Id,
                                                ASTDeclarationList* DL,
                                                ASTStatementList* SL,
                                                ASTResultNode* Res,
                                                const ASTToken* RTK) const;

  // Function Call Expression
  ASTFunctionCallNode* ProductionRule_2700(const ASTToken* TK,
                                           const ASTIdentifierNode* Id,
                                           const ASTExpressionList* EL) const;
  ASTFunctionCallStatementNode*
  ProductionRule_2700(const ASTToken* TK, const ASTFunctionCallNode* FC) const;

  // Pragma
  ASTPragmaNode* ProductionRule_2600(const ASTToken* TK,
                                     const std::string* PS) const;
  ASTPragmaStatementNode* ProductionRule_2601(const ASTPragmaNode* PN) const;

  // Annotations
  ASTAnnotationNode* ProductionRule_2610(const ASTToken* TK,
                                         const ASTStringList* SL) const;
  ASTAnnotationStatementNode*
  ProductionRule_2610(const ASTToken* TK, const ASTAnnotationNode* AN) const;

  // If Statement
  ASTIfStatementNode* ProductionRule_3000(const ASTToken* TK,
                                          ASTExpressionNode* EN,
                                          ASTStatementNode* SN) const;
  ASTIfStatementNode* ProductionRule_3001(const ASTToken* TK,
                                          ASTExpressionNode* EN,
                                          ASTStatementList* SL) const;

  // ElseIf
  ASTElseIfStatementNode* ProductionRule_3010(const ASTToken* TK,
                                              ASTExpressionNode* EN,
                                              ASTStatementNode* SN) const;
  ASTElseIfStatementNode* ProductionRule_3011(const ASTToken* TK,
                                              ASTExpressionNode* EN,
                                              ASTStatementList* SL) const;

  // Else
  ASTElseStatementNode* ProductionRule_3020(const ASTToken* TK,
                                            ASTStatementNode* SN) const;
  ASTElseStatementNode* ProductionRule_3021(const ASTToken* TK,
                                            ASTStatementList* SL) const;

  // Switch
  ASTSwitchStatementNode* ProductionRule_3100(const ASTToken* TK,
                                              const ASTIntNode* DI,
                                              ASTStatementList* SL) const;
  ASTSwitchStatementNode* ProductionRule_3101(const ASTToken* TK,
                                              const ASTBinaryOpNode* BOP,
                                              ASTStatementList* SL) const;
  ASTSwitchStatementNode* ProductionRule_3102(const ASTToken* TK,
                                              const ASTUnaryOpNode* UOP,
                                              ASTStatementList* SL) const;
  ASTSwitchStatementNode* ProductionRule_3103(const ASTToken* TK,
                                              const ASTIdentifierNode* DId,
                                              ASTStatementList* SL) const;
  ASTSwitchStatementNode* ProductionRule_3104(const ASTToken* TK,
                                              const ASTFunctionCallNode* FC,
                                              ASTStatementList* SL) const;
  ASTSwitchStatementNode* ProductionRule_3105(const ASTToken* TK,
                                              const ASTMPIntegerNode* DI,
                                              ASTStatementList* SL) const;
  ASTSwitchStatementNode* ProductionRule_3106(const ASTToken* TK,
                                              const ASTExpressionNode* DE,
                                              ASTStatementList* SL) const;

  // Switch Case
  ASTCaseStatementNode* ProductionRule_3110(const ASTToken* TK,
                                            ASTExpressionNode* EN,
                                            ASTStatementList* SL,
                                            bool SC = true) const;
  ASTCaseStatementNode* ProductionRule_3111(const ASTToken* TK,
                                            ASTExpressionNode* EN,
                                            ASTStatementList* SL,
                                            ASTBreakStatementNode* BK,
                                            bool SC = true) const;

  // Switch Default
  ASTDefaultStatementNode* ProductionRule_3120(const ASTToken* TK,
                                               ASTStatementList* SL,
                                               bool SC = true) const;
  ASTDefaultStatementNode* ProductionRule_3121(const ASTToken* TK,
                                               ASTStatementList* SL,
                                               ASTBreakStatementNode* BK,
                                               bool SC = true) const;

  // break/continue
  ASTBreakStatementNode* ProductionRule_3150(const ASTToken* TK) const;
  ASTContinueStatementNode* ProductionRule_3151(const ASTToken* TK) const;

  // For Loop
  ASTForStatementNode* ProductionRule_3200(const ASTToken* TK,
                                           ASTIdentifierNode* LId,
                                           ASTIntegerList* IL,
                                           ASTStatementList* SL) const;
  ASTForStatementNode* ProductionRule_3201(const ASTToken* TK,
                                           ASTIdentifierNode* LId,
                                           ASTIntegerList* IL,
                                           ASTStatement* ST) const;
  ASTForStatementNode* ProductionRule_3202(const ASTToken* TK,
                                           ASTIdentifierNode* LId,
                                           ASTIntegerList* IL,
                                           ASTStatement* ST) const;
  ASTForStatementNode* ProductionRule_3203(const ASTToken* TK,
                                           ASTIdentifierNode* LId,
                                           ASTIntegerList* IL,
                                           ASTStatementList* SL) const;
  ASTForStatementNode* ProductionRule_3204(const ASTToken* TK,
                                           ASTIdentifierNode* LId,
                                           ASTForLoopRangeExpressionNode* FLR,
                                           ASTStatementList* SL) const;
  ASTForStatementNode* ProductionRule_3205(const ASTToken* TK,
                                           ASTIdentifierNode* LId,
                                           ASTForLoopRangeExpressionNode* FLR,
                                           ASTStatement* ST) const;
  ASTForStatementNode* ProductionRule_3206(const ASTToken* TK,
                                           ASTIntNode* IVT,
                                           ASTIdentifierNode* LId,
                                           ASTIntegerList* IL,
                                           ASTStatementList* SL) const;
  ASTForStatementNode* ProductionRule_3207(const ASTToken* TK,
                                           ASTIntNode* IVT,
                                           ASTIdentifierNode* LId,
                                           ASTIntegerList* IL,
                                           ASTStatement* ST) const;
  ASTForStatementNode* ProductionRule_3208(const ASTToken* TK,
                                           ASTIntNode* IVT,
                                           ASTIdentifierNode* LId,
                                           ASTIntegerList* IL,
                                           ASTStatementList* SL) const;
  ASTForStatementNode* ProductionRule_3209(const ASTToken* TK,
                                           ASTIntNode* IVT,
                                           ASTIdentifierNode* LId,
                                           ASTIntegerList* IL,
                                           ASTStatement* ST) const;
  ASTForStatementNode* ProductionRule_3210(const ASTToken* TK,
                                           ASTIntNode* IVT,
                                           ASTIdentifierNode* LId,
                                           ASTForLoopRangeExpressionNode* FLR,
                                           ASTStatementList* SL) const;
  ASTForStatementNode* ProductionRule_3211(const ASTToken* TK,
                                           ASTIntNode* IVT,
                                           ASTIdentifierNode* LId,
                                           ASTForLoopRangeExpressionNode* FLR,
                                           ASTStatement* ST) const;

  // While Loop
  ASTWhileStatementNode* ProductionRule_3300(const ASTToken* TK,
                                             ASTExpressionNode* EN,
                                             ASTStatementList* SL) const;

  // Do/While Loop
  ASTDoWhileStatementNode* ProductionRule_3400(const ASTToken* TK,
                                               ASTExpressionNode* EN,
                                               ASTStatementList* SL) const;

  // Gate Ops
  ASTGateQOpNode* ProductionRule_3500(const ASTToken* TK,
                                      const ASTIdentifierNode* Id,
                                      const ASTArgumentNodeList* ANL,
                                      const ASTAnyTypeList* ATL) const;

  ASTGateQOpNode* ProductionRule_3502(const ASTToken* TK,
                                      const ASTArgumentNodeList* ANL,
                                      const ASTAnyTypeList* ATL) const;

  ASTGateQOpNode* ProductionRule_3503(const ASTToken* TK,
                                      const ASTIdentifierList* IL) const;

  ASTGateQOpNode* ProductionRule_3504(const ASTToken* TK,
                                      const ASTArgumentNodeList* ANL,
                                      const ASTAnyTypeList* ATL) const;

  ASTGateQOpNode* ProductionRule_3505(const ASTToken* TK,
                                      const ASTArgumentNodeList* ANL,
                                      const ASTAnyTypeList* ATL) const;

  ASTGateQOpNode* ProductionRule_3506(const ASTToken* TK,
                                      const ASTArgumentNodeList* ANL,
                                      const ASTAnyTypeList* ATL) const;

  // GPhase Statement
  ASTGateQOpNode* ProductionRule_3507(const ASTToken* TK,
                                      const ASTGPhaseExpressionNode* GPE) const;

  // Cast Expression
  ASTCastExpressionNode* ProductionRule_3600(const ASTToken* TK,
                                             const ASTExpressionNode* EXN,
                                             ASTType Ty) const;
  ASTCastExpressionNode*
  ProductionRule_3600(const ASTToken* TK, const std::variant<const ASTIntNode*,
                                                const ASTIdentifierNode*>& II,
                      const ASTExpressionNode* EXN, ASTType Ty) const;


  // Scalar Types
  ASTIntNode* ProductionRule_3700(const ASTToken* TK,
                                  ASTSignbit SB) const;
  ASTFloatNode* ProductionRule_3701(const ASTToken* TK) const;
  ASTDoubleNode* ProductionRule_3702(const ASTToken* TK) const;
  ASTMPIntegerNode* ProductionRule_3703(const ASTToken* TK,
                                        const ASTIntNode* II,
                                        ASTSignbit SB) const;
  ASTMPIntegerNode* ProductionRule_3703(const ASTToken* TK,
                                        const ASTIdentifierNode* IId,
                                        ASTSignbit SB) const;
  ASTMPIntegerNode* ProductionRule_3704(const ASTToken* TK,
                                        const ASTExpressionNode* EX) const;
  ASTMPDecimalNode* ProductionRule_3705(const ASTToken* TK,
                                        const ASTIntNode* II) const;
  ASTMPDecimalNode* ProductionRule_3705(const ASTToken* TK,
                                        const ASTIdentifierNode* IId) const;
  ASTMPDecimalNode* ProductionRule_3706(const ASTToken* TK,
                                        const ASTExpressionNode* EX) const;
  ASTMPComplexNode* ProductionRule_3707(const ASTToken* TK,
                                        const ASTIntNode* II) const;
  ASTMPComplexNode* ProductionRule_3708(const ASTToken* TK,
                                        const ASTIdentifierNode* IId) const;
  ASTCBitNode* ProductionRule_3709(const ASTToken* TK,
                                   const ASTIntNode* II) const;
  ASTCBitNode* ProductionRule_3710(const ASTToken* TK,
                                   const ASTIdentifierNode* IId) const;
  ASTCBitNode* ProductionRule_3711(const ASTToken* TK) const;
  ASTAngleNode* ProductionRule_3712(const ASTToken* TK) const;
  ASTAngleNode* ProductionRule_3713(const ASTToken* TK,
                                    const ASTIntNode* II) const;
  ASTAngleNode* ProductionRule_3714(const ASTToken* TK,
                                    const ASTIdentifierNode* IId) const;
  ASTAngleNode* ProductionRule_3715(const ASTToken* TK,
                                    const ASTExpressionNode* EX) const;
  ASTQubitContainerNode* ProductionRule_3716(const ASTToken* TK);
  ASTQubitContainerNode* ProductionRule_3717(const ASTToken* TK,
                                             const ASTIntNode* II) const;
  ASTQubitContainerNode* ProductionRule_3718(const ASTToken* TK,
                                             const ASTIdentifierNode* II) const;
  ASTDurationNode* ProductionRule_3718(const ASTToken* TK);
  ASTDurationNode* ProductionRule_3719(const ASTToken* TK,
                                       const ASTStringNode* TS) const;

  // Param Type Decl
  template<typename __Type>
  ASTDeclarationNode* ProductionRule_3800(const ASTToken* TK,
                                          const __Type* TP, ASTType Ty,
                                          bool Const = false) const;

  // Gate Control Expression
  template<typename __Type>
  ASTGateControlNode* ProductionRule_3850(const ASTToken* TK,
                                          const __Type* TP) const;
  template<typename __Type>
  ASTGateControlNode* ProductionRule_3850(const ASTToken* TK,
                                          const __Type* TP,
                                          const ASTIntNode* I) const;
  template<typename __Type>
  ASTGateControlNode* ProductionRule_3850(const ASTToken* TK,
                                          const ASTGateQOpNode* GQN) const;
  template<typename __Type>
  ASTGateControlNode* ProductionRule_3850(const ASTToken* TK,
                                          const ASTGateQOpNode* GQN,
                                          const ASTIntNode* I) const;
  ASTGateQOpNode* ProductionRule_3850(const ASTToken* TK,
                                      const ASTGateControlNode* GCN,
                                      bool UseASTBuilder) const;

  // Gate Neg Control Expression
  template<typename __Type>
  ASTGateNegControlNode* ProductionRule_3851(const ASTToken* TK,
                                             const __Type* TP) const;
  template<typename __Type>
  ASTGateNegControlNode* ProductionRule_3851(const ASTToken* TK,
                                             const __Type* TP,
                                             const ASTIntNode* I) const;
  template<typename __Type>
  ASTGateNegControlNode* ProductionRule_3851(const ASTToken* TK,
                                             const ASTGateQOpNode* GQN) const;
  template<typename __Type>
  ASTGateNegControlNode* ProductionRule_3851(const ASTToken* TK,
                                             const ASTGateQOpNode* GQN,
                                             const ASTIntNode* I) const;
  ASTGateQOpNode* ProductionRule_3851(const ASTToken* TK,
                                      const ASTGateNegControlNode* GNCN,
                                      bool UseASTBuilder) const;


  template<typename __Type, typename __OpType>
  ASTGatePowerNode* ProductionRule_3852(const ASTToken* TK,
                                        const __Type* TP,
                                        const __OpType* OTP) const;
  // Gate Pow Control Expression
  template<typename __OpType>
  ASTGatePowerNode* ProductionRule_3852(const ASTToken* TK,
                                        const ASTGateQOpNode* GQN,
                                        const __OpType* OTP) const;
  ASTGateQOpNode* ProductionRule_3852(const ASTToken* TK,
                                      const ASTGatePowerNode* GPN,
                                      bool UseASTBuilder) const;

  // Gate Inverse Control Expression
  template<typename __Type>
  ASTGateInverseNode* ProductionRule_3853(const ASTToken* TK,
                                          const __Type* TP) const;
  template<typename __Type>
  ASTGateInverseNode* ProductionRule_3853(const ASTToken* TK,
                                          const ASTGateQOpNode* GQN) const;
  ASTGateQOpNode* ProductionRule_3853(const ASTToken* TK,
                                      const ASTGateInverseNode* GIN,
                                      bool UseASTBuilder) const;

  // Gate GPhase Control Expression
  ASTGateGPhaseExpressionNode*
  ProductionRule_3854(const ASTToken* TK, const ASTGPhaseExpressionNode* GPN,
                      const ASTIdentifierList* IL) const;
  ASTGateQOpNode* ProductionRule_3854(const ASTToken* TK,
                                      const ASTGateGPhaseExpressionNode* GEN) const;

  // Defcal Grammar
  ASTDeclarationNode* ProductionRule_5000(const ASTDefcalGrammarNode* DG) const;

  // Line Directive
  ASTDirectiveStatementNode* ProductionRule_5300(const ASTToken* TK,
                                                 const std::string* S) const;
  // File Directive
  ASTDirectiveStatementNode* ProductionRule_5301(const ASTToken* TK,
                                                 const std::string* S) const;

  // Arrays
  ASTDeclarationNode* ProductionRule_3000(const ASTArrayNode* AN) const;

  // Scalars
  ASTDeclarationNode* ProductionRule_9000(const ASTMPComplexNode* MPC) const;

  // Expressions
  ASTExpressionNode* ProductionRule_8000(const ASTToken* TK,
                                         ASTExpressionNode* EXN) const;
  ASTExpressionNode* ProductionRule_8001(const ASTToken* TK,
                                         const ASTIdentifierNode* Id) const;


  // Miscellaneous Warnings
  void ProductionRule_6000(const ASTStatementNode* SN) const;

  void ProductionRule_6001(const ASTStatement* SN) const;

  // Miscellaneous Errors
  ASTDeclarationNode* ProductionRule_7000(const ASTDeclarationNode* DN) const;
  ASTBinaryOpNode* ProductionRule_7001(const ASTToken* TK,
                                       const ASTExpressionNode* EN) const;
  ASTBinaryOpNode* ProductionRule_7002(const ASTToken* TK) const;
  ASTBinaryOpNode* ProductionRule_7003(const ASTToken* TK) const;
  ASTMPIntegerNode* ProductionRule_7004(const ASTToken* TK) const;
  ASTMPDecimalNode* ProductionRule_7005(const ASTToken* TK) const;
  ASTArrayNode* ProductionRule_7006(const ASTToken* TK) const;
};

} // namespace QASM

#define __QASM_AST_PRODUCTION_FACTORY_CPP
#include "qasm/AST/ASTProductionFactory.inc"
#undef __QASM_AST_PRODUCTION_FACTORY_CPP

#endif // __QASM_AST_PRODUCTION_FACTORY_H

