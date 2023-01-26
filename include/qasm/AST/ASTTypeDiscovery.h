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

#ifndef __QASM_AST_TYPE_DISCOVERY_H
#define __QASM_AST_TYPE_DISCOVERY_H

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
#include <qasm/AST/ASTArraySubscript.h>
#include <qasm/AST/ASTCtrlAssocBuilder.h>
#include <qasm/AST/ASTBraceMatcher.h>
#include <qasm/AST/ASTInverseAssocBuilder.h>
#include <qasm/AST/ASTBoxStatementBuilder.h>
#include <qasm/AST/ASTExpressionValidator.h>
#include <qasm/AST/ASTRedeclarationController.h>

// OpenPulse
#include <qasm/AST/OpenPulse/ASTOpenPulsePlay.h>
#include <qasm/AST/OpenPulse/ASTOpenPulsePort.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseFrame.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseWaveform.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseCalibration.h>

#include <string>

namespace QASM {

class ASTTypeDiscovery {
private:
  static ASTTypeDiscovery TD;

private:
  ASTTypeDiscovery(const ASTTypeDiscovery& RHS) = delete;
  ASTTypeDiscovery& operator=(const ASTTypeDiscovery& RHS) = delete;

protected:
  ASTTypeDiscovery() = default;

protected:
  bool InReDeclarationContext(const ASTIdentifierNode* Id,
                              ASTType CTy, const ASTDeclarationContext* DCX) const;
  bool IsGateQubitParam(const ASTIdentifierNode* Id,
                        ASTType CTy, ASTType PTy,
                        const ASTDeclarationContext* DCX) const;
  bool IsGateAngleParam(const ASTIdentifierNode* Id,
                        ASTType CTy, ASTType PTy,
                        const ASTDeclarationContext* DCX) const;
  bool IsGateAngleParam(const std::string& S,
                        ASTType CTy, ASTType PTy,
                        const ASTDeclarationContext* DCX) const;

  ASTIdentifierNode*
  CreateLocalASTIdentifierNodeRedeclaration(const ASTIdentifierNode* Id,
                                            unsigned Bits,
                                            ASTType Ty,
                                            const ASTDeclarationContext* DCX,
                                            const ASTToken* TK) const;
  ASTIdentifierNode*
  CreateLocalASTIdentifierNodeRedeclaration(const std::string& S,
                                            unsigned Bits,
                                            ASTType Ty,
                                            const ASTDeclarationContext* DCX,
                                            const ASTToken* TK) const;

  ASTIdentifierNode*
  CreateReservedFPConstantId(const std::string& N,
                             const ASTDeclarationContext* DCX,
                             const ASTToken* TK) const;

  bool IsCallable(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeGate:
    case ASTTypeGateCall:
    case ASTTypeCXGate:
    case ASTTypeCCXGate:
    case ASTTypeCNotGate:
    case ASTTypeHadamardGate:
    case ASTTypeUGate:
    case ASTTypeDefcal:
    case ASTTypeDefcalGroup:
    case ASTTypeDefcalCall:
    case ASTTypeDefcalCallExpression:
    case ASTTypeDefcalCallStatement:
    case ASTTypeDefcalDeclaration:
    case ASTTypeFunction:
    case ASTTypeFunctionDeclaration:
    case ASTTypeFunctionCall:
    case ASTTypeFunctionCallExpression:
    case ASTTypeFunctionCallStatement:
      return true;
      break;
    default:
      return false;
      break;
    }

    return false;
  }

  bool IsCallable(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return IsCallable(Id->GetSymbolType());
  }

public:
  static ASTTypeDiscovery& Instance() {
    return TD;
  }

  ~ASTTypeDiscovery() = default;

  ASTIdentifierNode* ResolveASTIdentifier(const ASTToken* TK,
                                          const std::string& S) const;

  ASTIdentifierRefNode* ResolveASTIdentifierRef(const ASTToken* TK,
                                                const std::string& S,
                                                const ASTArraySubscriptNode* ASN,
                                                const ASTArraySubscriptList* ASL) const;

  void ValidateGateQubitArgs(const ASTAnyTypeList* ATL,
                             const ASTGateNode* G) const;

  void ValidateDefcalQubitArgs(const ASTAnyTypeList* ATL,
                               const ASTDefcalGroupNode* DG) const;
};

} // namespace QASM

#endif // __QASM_AST_TYPE_DISCOVERY_H

