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

#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTFunctions.h>
#include <qasm/AST/ASTImplicitConversionExpr.h>
#include <qasm/AST/ASTFunctionParameterBuilder.h>
#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTQubitNodeBuilder.h>
#include <qasm/AST/ASTFunctionContextBuilder.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTReturn.h>
#include <qasm/AST/ASTIfConditionals.h>
#include <qasm/AST/ASTSwitchStatement.h>
#include <qasm/AST/ASTLoops.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <sstream>

namespace QASM {

ASTExpressionNodeList ASTFunctionParameterBuilder::EV;
ASTFunctionParameterBuilder ASTFunctionParameterBuilder::FPB;
ASTFunctionDeclarationMap ASTFunctionDeclarationMap::FDM;

ASTFunctionContextBuilder ASTFunctionContextBuilder::FCB;
bool ASTFunctionContextBuilder::FCS;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTFunctionDefinitionNode::
ASTFunctionDefinitionNode(const ASTIdentifierNode* Id,
                          const ASTDeclarationList& PDL,
                          const ASTStatementList& SL,
                          ASTResultNode *RES,
                          bool IsDefinition)
  : ASTExpressionNode(Id, ASTTypeFunction), Params(),
  STM(), Statements(SL), Result(RES), IsDeclADefinition(IsDefinition),
  Ellipsis(false), Builtin(false), Extern(false) {
    if (ASTTypeSystemBuilder::Instance().IsBuiltinFunction(Id->GetName())) {
      // MOVED TO ASTProductionFactory.cpp.
      // ASTSymbolTable::Instance().TransferLocalContextSymbols().
      std::vector<ASTDeclarationList::const_iterator> DVI;
      using erase_type = std::vector<ASTDeclarationList::const_iterator>;

      for (ASTDeclarationList::const_iterator I = PDL.begin();
           I != PDL.end(); ++I) {
        const ASTIdentifierNode* DId = (*I)->GetIdentifier();
        assert(DId && "Invalid ASTIdentifierNode for ASTDeclarationNode!");

        for (ASTStatementList::const_iterator SI = SL.begin();
             SI != SL.end(); ++SI) {
          const ASTStatementNode* SN = dynamic_cast<const ASTStatementNode*>(*SI);
          if (SN && SN->IsDeclaration() && (*I == SN)) {
            DVI.push_back(I);
          }
        }
      }

      if (DVI.size()) {
        for (erase_type::iterator DI = DVI.begin(); DI != DVI.end(); ++DI)
          PDL.Erase(*DI);
      }
    }

    unsigned PIX = 0U;
    unsigned EC = 0U;
    const ASTDeclarationContext* DCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
    assert(DCX && "Could not obtain a valid ASTDeclarationContext!");

    for (ASTDeclarationList::const_iterator I = PDL.begin(); I != PDL.end(); ++I) {
      if (Ellipsis) {
        std::stringstream M;
        M << "Ellipsis must be last in a function parameter list.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(*I), M.str(), DiagLevel::Error);
      }

      if ((*I)->GetASTType() == ASTTypeEllipsis) {
        Ellipsis = true;
        ++EC;
      }

      const ASTIdentifierNode* DId = (*I)->GetIdentifier();
      assert(DId && "Could not obtain a valid ASTIdentifierNode!");

      const ASTSymbolTableEntry* DSTE = ASTSymbolTable::Instance().FindLocal(DId);
      assert(DSTE && "Function parameter declaration has no SymbolTable Entry!");

      if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(DId->GetName()) &&
          !ASTStringUtils::Instance().IsBoundQubit(DId->GetName())) {
        const_cast<ASTIdentifierNode*>(DId)->SetDeclarationContext(DCX);
        const_cast<ASTIdentifierNode*>(DId)->SetLocalScope();
        const_cast<ASTSymbolTableEntry*>(DSTE)->SetContext(DCX);
        const_cast<ASTSymbolTableEntry*>(DSTE)->SetLocalScope();
      }

      Params.insert(std::make_pair(PIX++, *I));

      if (!STM.insert(std::make_pair((*I)->GetName(), DSTE)).second) {
        std::stringstream M;
        M << "Error inserting parameter symbol into the Function SymbolTable.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(DId), M.str(), DiagLevel::Error);
      }

      if (ASTUtils::Instance().IsQubitType(DId->GetSymbolType())) {
        std::stringstream QS;

        for (unsigned J = 0; J < DId->GetBits(); ++J) {
          QS.str("");
          QS.clear();
          QS << '%' << DId->GetName() << ':' << J;
          DSTE = ASTSymbolTable::Instance().FindLocal(QS.str());
          assert(DSTE && "Invalid ASTQubitContainerNode without Qubits!");

          if (!STM.insert(std::make_pair(QS.str(), DSTE)).second) {
            std::stringstream M;
            M << "Error inserting Qubit parameter symbol into the Kernel "
              << "SymbolTable.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(DId), M.str(),
                                                            DiagLevel::Error);
          }

          QS.str("");
          QS.clear();
          QS << DId->GetName() << '[' << J << ']';
          DSTE = ASTSymbolTable::Instance().FindLocal(QS.str());
          assert(DSTE && "Invalid ASTQubitContainerNode without Qubits!");

          if (!STM.insert(std::make_pair(QS.str(), DSTE)).second) {
            std::stringstream M;
            M << "Error inserting Qubit parameter symbol into the Kernel "
              << "SymbolTable.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(DId), M.str(),
                                                            DiagLevel::Error);
          }
        }
      } else if (ASTUtils::Instance().IsAngleType(DId->GetSymbolType())) {
        std::stringstream QS;

        for (unsigned J = 0; J < 3; ++J) {
          QS.str("");
          QS.clear();
          QS << DId->GetName() << '[' << J << ']';
          DSTE = ASTSymbolTable::Instance().FindLocal(QS.str());

          if (DSTE) {
            if (!STM.insert(std::make_pair(QS.str(), DSTE)).second) {
              std::stringstream M;
              M << "Error inserting Angle parameter symbol into the Kernel "
                << "SymbolTable.";
              QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(DId), M.str(),
                                                              DiagLevel::Error);
            }
          }
        }
      }

      if (ASTTypeSystemBuilder::Instance().IsBuiltinFunction(Id->GetName())) {
        // MOVED TO ASTProductionFactory.cpp.
        // ASTSymbolTable::Instance().TransferLocalContextSymbols().
        if (ASTUtils::Instance().IsQubitType(DId->GetSymbolType()))
          ASTSymbolTable::Instance().EraseLocalQubit(DId);

        if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(DId->GetName()) &&
            !ASTStringUtils::Instance().IsBoundQubit(DId->GetName())) {
          if (ASTUtils::Instance().IsAngleType(DId->GetSymbolType()))
            ASTSymbolTable::Instance().EraseLocalAngle(DId->GetName());
          else
            ASTSymbolTable::Instance().EraseLocalSymbol(DId, DId->GetBits(),
                                                        DId->GetSymbolType());
        }
      }
    }

    if (EC > 1) {
      std::stringstream M;
      M << "Ellipsis can only appear once in a function parameter list.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(&PDL), M.str(), DiagLevel::Error);
    }

    const ASTIdentifierNode* RId = RES->GetIdentifier();
    assert(RId && "Invalid ASTIdentifierNode for function ASTResultNode!");

    const ASTSymbolTableEntry* RSTE = ASTSymbolTable::Instance().FindLocal(RId);
    assert(RSTE && "Function ASTResultNode has no SymbolTable Entry!");

    RES->SetDeclarationContext(DCX);
    const_cast<ASTIdentifierNode*>(RId)->SetDeclarationContext(DCX);
    const_cast<ASTIdentifierNode*>(RId)->SetLocalScope();
    const_cast<ASTSymbolTableEntry*>(RSTE)->SetContext(DCX);
    const_cast<ASTSymbolTableEntry*>(RSTE)->SetLocalScope();

    if (!STM.insert(std::make_pair(RId->GetName(), RSTE)).second) {
      std::stringstream M;
      M << "Error inserting result symbol into the Function SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(RId), M.str(), DiagLevel::Error);
    }

    if (ASTTypeSystemBuilder::Instance().IsBuiltinFunction(Id->GetName())) {
      ASTSymbolTable::Instance().EraseLocalSymbol(RId, RId->GetBits(),
                                                  RId->GetSymbolType());
    }
}

bool
ASTFunctionDefinitionNode::CheckMeasureReturnType(const ASTReturnStatementNode* RSN,
                                       std::pair<ASTType, ASTType>& OP) const {
  assert(RSN && "Invalid ASTReturnStatementNode argument!");

  const ASTMeasureNode* MN = RSN->GetMeasure();
  assert(MN && "Could not obtain a valid ASTMeasureNode!");
  if (!MN) {
    std::stringstream M;
    M << "Could not obtain a valid ASTMeasureNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(RSN), M.str(), DiagLevel::ICE);
    return false;
  }

  const ASTImplicitConversionNode* ICE = nullptr;

  switch (OP.second) {
  case ASTTypeBitset: {
    if (MN->GetResultType() == OP.second && MN->HasResult()) {
      ICE = new ASTImplicitConversionNode(MN->GetResult(), OP.first);
      assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    } else {
      ICE = ASTImplicitConversionNode::InvalidConversion(OP.first, OP.second);
      assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    }

    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeMPComplex: {
    if (MN->GetResultType() == OP.second && MN->HasComplexResult()) {
      ICE = new ASTImplicitConversionNode(MN->GetComplexResult(), OP.first);
      assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    } else {
      ICE = ASTImplicitConversionNode::InvalidConversion(OP.first, OP.second);
      assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    }

    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeAngle: {
    if (MN->GetResultType() == OP.second && MN->HasAngleResult()) {
      ICE = new ASTImplicitConversionNode(MN->GetAngleResult(), OP.first);
      assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    } else {
      ICE = ASTImplicitConversionNode::InvalidConversion(OP.first, OP.second);
      assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    }

    RSN->AddImplicitConversion(ICE);
  }
    break;
  default:
    ICE = ASTImplicitConversionNode::InvalidConversion(OP.first, OP.second);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  }

  return ICE->IsValidConversion();
}

bool
ASTFunctionDefinitionNode::CheckFunctionReturnType(const ASTReturnStatementNode* RSN,
                                        std::pair<ASTType, ASTType>& OP) const {
  assert(RSN && "Invalid ASTReturnStatementNode argument!");

  const ASTFunctionCallNode* FCN =
    RSN->GetFunctionStatement()->GetFunctionCall();
  assert(FCN && "Could not obtain a valid ASTFunctionCallNode!");
  if (!FCN) {
    std::stringstream M;
    M << "Could not obtain a valid ASTFunctionCallNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(RSN), M.str(), DiagLevel::ICE);
    return false;
  }

  const ASTFunctionDefinitionNode* FDN = FCN->GetFunctionDefinition();
  assert(FDN && "Could not obtain a valid ASTFunctionDefinitionNode!");
  if (!FDN) {
    std::stringstream M;
    M << "Could not obtain a valid ASTFunctionDefinitionNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(FCN), M.str(), DiagLevel::ICE);
    return false;
  }

  const ASTResultNode* FRN = FDN->GetResult();
  assert(FRN && "Could not obtain a valid ASTResultNode!");
  if (!FRN) {
    std::stringstream M;
    M << "Could not obtain a valid ASTResultNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(FDN), M.str(), DiagLevel::ICE);
    return false;
  }

  const ASTImplicitConversionNode* ICE = nullptr;

  switch (FRN->GetResultType()) {
  case ASTTypeVoid:
    ICE = new ASTImplicitConversionNode(FRN->GetVoidNode(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeBool:
    ICE = new ASTImplicitConversionNode(FRN->GetBoolNode(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeInt:
  case ASTTypeUInt:
    ICE = new ASTImplicitConversionNode(FRN->GetIntNode(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeFloat:
    ICE = new ASTImplicitConversionNode(FRN->GetFloatNode(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeDouble:
    ICE = new ASTImplicitConversionNode(FRN->GetDoubleNode(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeDuration:
    ICE = new ASTImplicitConversionNode(FRN->GetDurationNode(),
                                        OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeOpenPulseFrame:
    ICE = new ASTImplicitConversionNode(FRN->GetFrame(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeOpenPulseWaveform:
    ICE = new ASTImplicitConversionNode(FRN->GetWaveform(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeOpenPulsePort:
    ICE = new ASTImplicitConversionNode(FRN->GetPort(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeAngle:
    ICE = new ASTImplicitConversionNode(FRN->GetAngleNode(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
    ICE = new ASTImplicitConversionNode(FRN->GetMPInteger(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeMPDecimal:
    ICE = new ASTImplicitConversionNode(FRN->GetMPDecimal(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeMPComplex:
    ICE = new ASTImplicitConversionNode(FRN->GetMPComplex(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeBitset:
    ICE = new ASTImplicitConversionNode(FRN->GetCBitNode(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  case ASTTypeMeasure:
    return CheckMeasureReturnType(RSN, OP);
    break;
  case ASTTypeFunctionCallStatement:
    return CheckFunctionReturnType(RSN, OP);
    break;
  default:
    ICE = ASTImplicitConversionNode::InvalidConversion(OP.first, OP.second);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  }

  return ICE->IsValidConversion();
}

bool
ASTFunctionDefinitionNode::CheckReturnType(const ASTReturnStatementNode* RSN,
                                           std::pair<ASTType, ASTType>& OP) const {
  assert(RSN && "Invalid ASTReturnStatementNode argument!");

  if (RSN->IsTransitive()) {
    if (RSN->IsMeasure())
      return CheckMeasureReturnType(RSN, OP);
    else if (RSN->IsFunctionCall())
      return CheckFunctionReturnType(RSN, OP);
  }

  const ASTImplicitConversionNode* ICE = nullptr;

  switch (OP.second) {
  case ASTTypeBinaryOp: {
    ICE = new ASTImplicitConversionNode(RSN->GetBinaryOp(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeUnaryOp: {
    ICE = new ASTImplicitConversionNode(RSN->GetUnaryOp(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeBool: {
    ICE = new ASTImplicitConversionNode(RSN->GetBool(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeInt:
  case ASTTypeUInt: {
    ICE = new ASTImplicitConversionNode(RSN->GetInt(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeFloat: {
    ICE = new ASTImplicitConversionNode(RSN->GetFloat(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeDouble: {
    ICE = new ASTImplicitConversionNode(RSN->GetDouble(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger: {
    ICE = new ASTImplicitConversionNode(RSN->GetMPInteger(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeMPDecimal: {
    ICE = new ASTImplicitConversionNode(RSN->GetMPDecimal(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeMPComplex: {
    ICE = new ASTImplicitConversionNode(RSN->GetMPComplex(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeAngle: {
    ICE = new ASTImplicitConversionNode(RSN->GetAngle(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeBitset: {
    ICE = new ASTImplicitConversionNode(RSN->GetCBit(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeIdentifier: {
    ASTScopeController::Instance().CheckIdentifier(RSN->GetIdent());
    ICE = new ASTImplicitConversionNode(RSN->GetIdent(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeFunctionCallStatement:
    return CheckFunctionReturnType(RSN, OP);
    break;
  case ASTTypeMeasure:
    return CheckMeasureReturnType(RSN, OP);
    break;
  case ASTTypeStatement: {
    ICE = new ASTImplicitConversionNode(RSN->GetStatement(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  case ASTTypeExpression: {
    ICE = new ASTImplicitConversionNode(RSN->GetExpression(), OP.first);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
  }
    break;
  default:
    ICE = ASTImplicitConversionNode::InvalidConversion(OP.first, OP.second);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    RSN->AddImplicitConversion(ICE);
    break;
  }

  return ICE->IsValidConversion();
}

bool
ASTFunctionDefinitionNode::CheckReturnType(std::pair<ASTType, ASTType>& OP) const {
  assert(Result && "Invalid ASTResultNode!");

  for (std::map<std::string, const ASTSymbolTableEntry*>::const_iterator MI = STM.begin();
       MI != STM.end(); ++MI) {
    // Result nodes do not have a SymbolTable Entry.
    if (!(*MI).second)
      continue;

    if ((*MI).second->GetValueType() == ASTTypeReturn) {
      const ASTReturnStatementNode* RSN =
        (*MI).second->GetValue()->GetValue<ASTReturnStatementNode*>();
      assert(RSN && "Could not obtain a valid Function ASTReturnStatementNode!");

      OP.first = Result->GetResultType();

      switch (RSN->GetReturnType()) {
      case ASTTypeCast:
        OP.second = RSN->GetCastReturnType();
        break;
      case ASTTypeMeasure:
        OP.second = RSN->GetMeasure()->GetResultType();
        break;
      default:
        OP.second = RSN->GetReturnType();
        break;
      }

      if (OP.first != OP.second) {
        if (!CheckReturnType(RSN, OP))
          return false;
      }
    }
  }

  return true;
}

const ASTReturnStatementNode*
ASTFunctionDefinitionNode::CheckReturnStatements(
                                      std::pair<ASTType, ASTType>& OP) const {
  OP.first = Result->GetResultType();

  for (ASTStatementList::const_iterator SI = Statements.begin();
       SI != Statements.end(); ++SI) {
    const ASTStatementNode* SN = dynamic_cast<const ASTStatementNode*>(*SI);
    assert(SN && "Could not obtain a valid ASTStatementNode!");

    switch (SN->GetASTType()) {
    case ASTTypeIfStatement: {
      if (const ASTIfStatementNode* IFS =
          dynamic_cast<const ASTIfStatementNode*>(SN)) {
        const ASTStatementList* ISL = IFS->GetOpList();
        assert(ISL && "Could not obtain a valid ASTStatementList!");

        for (ASTStatementList::const_iterator ISI = ISL->begin();
             ISI != ISL->end(); ++ISI) {
          if ((*ISI)->GetASTType() == ASTTypeReturn) {
            if (const ASTReturnStatementNode* RSN =
                dynamic_cast<const ASTReturnStatementNode*>(*ISI)) {
              switch (RSN->GetReturnType()) {
              case ASTTypeCast:
                OP.second = RSN->GetCastReturnType();
                break;
              case ASTTypeMeasure:
                OP.second = RSN->GetMeasure()->GetResultType();
                break;
              default:
                if (RSN->IsImplicitConversion())
                  OP.second = RSN->GetImplicitConversionType();
                else
                  OP.second = RSN->GetReturnType();
                break;
              }

              if (OP.first != OP.second) {
                if (!CheckReturnType(RSN, OP))
                  return RSN;
              }
            }
          } else {
            switch ((*ISI)->GetASTType()) {
            case ASTTypeIfStatement: {
              if (const ASTIfStatementNode* IIFS =
                  dynamic_cast<const ASTIfStatementNode*>(*ISI)) {
                const ASTStatementList* IISL = IIFS->GetOpList();
                assert(IISL && "Could not obtain a valid ASTStatementList!");

                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(*IISL, OP, IFS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeElseIfStatement: {
              if (const ASTElseIfStatementNode* EIFS =
                  dynamic_cast<const ASTElseIfStatementNode*>(*ISI)) {
                const ASTStatementList* EISL = EIFS->GetOpList();
                assert(EISL && "Could not obtain a valid ASTStatementList!");

                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(*EISL, OP, EIFS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeElseStatement: {
              if (const ASTElseStatementNode* ES =
                  dynamic_cast<const ASTElseStatementNode*>(*ISI)) {
                const ASTStatementList* ESL = ES->GetOpList();
                assert(ESL && "Could not obtain a valid ASTStatementList!");

                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(*ESL, OP, ES->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeForStatement: {
              if (const ASTForStatementNode* FOS =
                  dynamic_cast<const ASTForStatementNode*>(*ISI)) {
                const ASTForLoopNode* FOL = FOS->GetLoop();
                assert(FOL && "Could not obtain a valid ASTForLoopNode!");

                const ASTStatementList& FSL = FOL->GetStatementList();
                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(FSL, OP, FOS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeWhileStatement: {
              if (const ASTWhileStatementNode* WS =
                  dynamic_cast<const ASTWhileStatementNode*>(*ISI)) {
                const ASTWhileLoopNode* WL = WS->GetLoop();
                assert(WL && "Could not obtain a valid ASTWhileLoopNode!");

                const ASTStatementList& WSL = WL->GetStatementList();
                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(WSL, OP, WS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeDoWhileStatement: {
              if (const ASTDoWhileStatementNode* DWS =
                  dynamic_cast<const ASTDoWhileStatementNode*>(*ISI)) {
                const ASTDoWhileLoopNode* DWL = DWS->GetLoop();
                assert(DWL && "Could not obtain a valid ASTDoWhileLoopNode!");

                const ASTStatementList& DWSL = DWL->GetStatementList();
                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(DWSL, OP, DWS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeSwitchStatement: {
              if (const ASTSwitchStatementNode* SWS =
                  dynamic_cast<const ASTSwitchStatementNode*>(*ISI)) {
                if (SWS->GetNumCaseStatements() == 0) {
                  std::stringstream M;
                  M << "Switch statement with no case labels.";
                  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                    DIAGLineCounter::Instance().GetLocation(SWS), M.str(),
                                                                  DiagLevel::Error);
                  return ASTReturnStatementNode::StatementError(M.str());
                }

                for (unsigned I = 0; I < SWS->GetNumCaseStatements(); ++I) {
                  const ASTCaseStatementNode* CSN = SWS->GetCaseStatement(I);
                  assert(CSN && "Could not obtain a valid ASTCaseStatementNode!");

                  const ASTStatementList* CSL = CSN->GetStatementList();
                  assert(CSL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*CSL, OP, CSN->GetASTType()))
                    return RSN;
                }

                const ASTDefaultStatementNode* DSN = SWS->GetDefaultStatement();
                if (!DSN) {
                  std::stringstream M;
                  M << "Switch statement without a default label.";
                  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                    DIAGLineCounter::Instance().GetLocation(SWS), M.str(),
                                                                  DiagLevel::Warning);
                } else {
                  const ASTStatementList* DSL = DSN->GetStatementList();
                  assert(DSL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*DSL, OP, DSN->GetASTType()))
                    return RSN;

                }
              }
            }
              break;
            default:
              break;
            }
          }
        }
      }
    }
      break;
    case ASTTypeElseIfStatement: {
      if (const ASTElseIfStatementNode* EIFS =
          dynamic_cast<const ASTElseIfStatementNode*>(SN)) {
        const ASTStatementList* ISL = EIFS->GetOpList();
        assert(ISL && "Could not obtain a valid ASTStatementList!");

        for (ASTStatementList::const_iterator ISI = ISL->begin();
             ISI != ISL->end(); ++ISI) {
          if ((*ISI)->GetASTType() == ASTTypeReturn) {
            if (const ASTReturnStatementNode* RSN =
                dynamic_cast<const ASTReturnStatementNode*>(*ISI)) {
              switch (RSN->GetReturnType()) {
              case ASTTypeCast:
                OP.second = RSN->GetCastReturnType();
                break;
              case ASTTypeMeasure:
                OP.second = RSN->GetMeasure()->GetResultType();
                break;
              default:
                if (RSN->IsImplicitConversion())
                  OP.second = RSN->GetImplicitConversionType();
                else
                  OP.second = RSN->GetReturnType();
                break;
              }

              if (OP.first != OP.second) {
                if (!CheckReturnType(RSN, OP))
                  return RSN;
              }
            }
          } else {
            switch ((*ISI)->GetASTType()) {
            case ASTTypeIfStatement: {
              if (const ASTIfStatementNode* IFS =
                  dynamic_cast<const ASTIfStatementNode*>(*ISI)) {
                const ASTStatementList* IISL = IFS->GetOpList();
                assert(IISL && "Could not obtain a valid ASTStatementList!");

                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(*IISL, OP, IFS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeElseIfStatement: {
              if (const ASTElseIfStatementNode* EEIFS =
                  dynamic_cast<const ASTElseIfStatementNode*>(*ISI)) {
                const ASTStatementList* EISL = EEIFS->GetOpList();
                assert(EISL && "Could not obtain a valid ASTStatementList!");

                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(*EISL, OP, EIFS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeElseStatement: {
              if (const ASTElseStatementNode* ES =
                  dynamic_cast<const ASTElseStatementNode*>(*ISI)) {
                const ASTStatementList* ESL = ES->GetOpList();
                assert(ESL && "Could not obtain a valid ASTStatementList!");

                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(*ESL, OP, ES->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeForStatement: {
              if (const ASTForStatementNode* FOS =
                  dynamic_cast<const ASTForStatementNode*>(*ISI)) {
                const ASTForLoopNode* FOL = FOS->GetLoop();
                assert(FOL && "Could not obtain a valid ASTForLoopNode!");

                const ASTStatementList& FSL = FOL->GetStatementList();
                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(FSL, OP, FOS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeWhileStatement: {
              if (const ASTWhileStatementNode* WS =
                  dynamic_cast<const ASTWhileStatementNode*>(*ISI)) {
                const ASTWhileLoopNode* WL = WS->GetLoop();
                assert(WL && "Could not obtain a valid ASTWhileLoopNode!");

                const ASTStatementList& WSL = WL->GetStatementList();
                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(WSL, OP, WS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeDoWhileStatement: {
              if (const ASTDoWhileStatementNode* DWS =
                  dynamic_cast<const ASTDoWhileStatementNode*>(*ISI)) {
                const ASTDoWhileLoopNode* DWL = DWS->GetLoop();
                assert(DWL && "Could not obtain a valid ASTDoWhileLoopNode!");

                const ASTStatementList& DWSL = DWL->GetStatementList();
                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(DWSL, OP, DWS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeSwitchStatement: {
              if (const ASTSwitchStatementNode* SWS =
                  dynamic_cast<const ASTSwitchStatementNode*>(*ISI)) {
                if (SWS->GetNumCaseStatements() == 0) {
                  std::stringstream M;
                  M << "Switch statement with no case labels.";
                  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                    DIAGLineCounter::Instance().GetLocation(SWS), M.str(),
                                                                  DiagLevel::Error);
                  return ASTReturnStatementNode::StatementError(M.str());
                }

                for (unsigned I = 0; I < SWS->GetNumCaseStatements(); ++I) {
                  const ASTCaseStatementNode* CSN = SWS->GetCaseStatement(I);
                  assert(CSN && "Could not obtain a valid ASTCaseStatementNode!");

                  const ASTStatementList* CSL = CSN->GetStatementList();
                  assert(CSL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*CSL, OP, CSN->GetASTType()))
                    return RSN;
                }

                const ASTDefaultStatementNode* DSN = SWS->GetDefaultStatement();
                if (!DSN) {
                  std::stringstream M;
                  M << "Switch statement without a default label.";
                  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                    DIAGLineCounter::Instance().GetLocation(SWS), M.str(),
                                                                  DiagLevel::Warning);
                } else {
                  const ASTStatementList* DSL = DSN->GetStatementList();
                  assert(DSL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*DSL, OP, DSN->GetASTType()))
                    return RSN;

                }
              }
            }
              break;
            default:
              break;
            }
          }
        }
      }
    }
      break;
    case ASTTypeElseStatement: {
      if (const ASTElseStatementNode* EFS =
          dynamic_cast<const ASTElseStatementNode*>(SN)) {
        const ASTStatementList* ISL = EFS->GetOpList();
        assert(ISL && "Could not obtain a valid ASTStatementList!");

        for (ASTStatementList::const_iterator ISI = ISL->begin();
             ISI != ISL->end(); ++ISI) {
          if ((*ISI)->GetASTType() == ASTTypeReturn) {
            if (const ASTReturnStatementNode* RSN =
                dynamic_cast<const ASTReturnStatementNode*>(*ISI)) {
              switch (RSN->GetReturnType()) {
              case ASTTypeCast:
                OP.second = RSN->GetCastReturnType();
                break;
              case ASTTypeMeasure:
                OP.second = RSN->GetMeasure()->GetResultType();
                break;
              default:
                if (RSN->IsImplicitConversion())
                  OP.second = RSN->GetImplicitConversionType();
                else
                  OP.second = RSN->GetReturnType();
                break;
              }

              if (OP.first != OP.second) {
                if (!CheckReturnType(RSN, OP))
                  return RSN;
              }
            }
          } else {
            switch ((*ISI)->GetASTType()) {
            case ASTTypeIfStatement: {
              if (const ASTIfStatementNode* IFS =
                  dynamic_cast<const ASTIfStatementNode*>(*ISI)) {
                const ASTStatementList* IISL = IFS->GetOpList();
                assert(IISL && "Could not obtain a valid ASTStatementList!");

                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(*IISL, OP, IFS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeElseIfStatement: {
              if (const ASTElseIfStatementNode* EIFS =
                  dynamic_cast<const ASTElseIfStatementNode*>(*ISI)) {
                const ASTStatementList* EISL = EIFS->GetOpList();
                assert(EISL && "Could not obtain a valid ASTStatementList!");

                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(*EISL, OP, EIFS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeElseStatement: {
              if (const ASTElseStatementNode* ES =
                  dynamic_cast<const ASTElseStatementNode*>(*ISI)) {
                const ASTStatementList* ESL = ES->GetOpList();
                assert(ESL && "Could not obtain a valid ASTStatementList!");

                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(*ESL, OP, ES->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeForStatement: {
              if (const ASTForStatementNode* FOS =
                  dynamic_cast<const ASTForStatementNode*>(*ISI)) {
                const ASTForLoopNode* FOL = FOS->GetLoop();
                assert(FOL && "Could not obtain a valid ASTForLoopNode!");

                const ASTStatementList& FSL = FOL->GetStatementList();
                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(FSL, OP, FOS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeWhileStatement: {
              if (const ASTWhileStatementNode* WS =
                  dynamic_cast<const ASTWhileStatementNode*>(*ISI)) {
                const ASTWhileLoopNode* WL = WS->GetLoop();
                assert(WL && "Could not obtain a valid ASTWhileLoopNode!");

                const ASTStatementList& WSL = WL->GetStatementList();
                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(WSL, OP, WS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeDoWhileStatement: {
              if (const ASTDoWhileStatementNode* DWS =
                  dynamic_cast<const ASTDoWhileStatementNode*>(*ISI)) {
                const ASTDoWhileLoopNode* DWL = DWS->GetLoop();
                assert(DWL && "Could not obtain a valid ASTDoWhileLoopNode!");

                const ASTStatementList& DWSL = DWL->GetStatementList();
                if (const ASTReturnStatementNode* RSN =
                    CheckReturnStatements(DWSL, OP, DWS->GetASTType()))
                  return RSN;
              }
            }
              break;
            case ASTTypeSwitchStatement: {
              if (const ASTSwitchStatementNode* SWS =
                  dynamic_cast<const ASTSwitchStatementNode*>(*ISI)) {
                if (SWS->GetNumCaseStatements() == 0) {
                  std::stringstream M;
                  M << "Switch statement with no case labels.";
                  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                    DIAGLineCounter::Instance().GetLocation(SWS), M.str(),
                                                                  DiagLevel::Error);
                  return ASTReturnStatementNode::StatementError(M.str());
                }

                for (unsigned I = 0; I < SWS->GetNumCaseStatements(); ++I) {
                  const ASTCaseStatementNode* CSN = SWS->GetCaseStatement(I);
                  assert(CSN && "Could not obtain a valid ASTCaseStatementNode!");

                  const ASTStatementList* CSL = CSN->GetStatementList();
                  assert(CSL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*CSL, OP, CSN->GetASTType()))
                    return RSN;
                }

                const ASTDefaultStatementNode* DSN = SWS->GetDefaultStatement();
                if (!DSN) {
                  std::stringstream M;
                  M << "Switch statement without a default label.";
                  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                    DIAGLineCounter::Instance().GetLocation(SWS), M.str(),
                                                                  DiagLevel::Warning);
                } else {
                  const ASTStatementList* DSL = DSN->GetStatementList();
                  assert(DSL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*DSL, OP, DSN->GetASTType()))
                    return RSN;

                }
              }
            }
              break;
            default:
              break;
            }
          }
        }
      }
    }
      break;
    case ASTTypeForStatement: {
      if (const ASTForStatementNode* FOS =
          dynamic_cast<const ASTForStatementNode*>(SN)) {
        if (const ASTForLoopNode* FOL = FOS->GetLoop()) {
          const ASTStatementList& FSL = FOL->GetStatementList();

          for (ASTStatementList::const_iterator FI = FSL.begin();
               FI != FSL.end(); ++FI) {
            if ((*FI)->GetASTType() == ASTTypeReturn) {
              if (const ASTReturnStatementNode* RSN =
                  dynamic_cast<const ASTReturnStatementNode*>(*FI)) {
                switch (RSN->GetReturnType()) {
                case ASTTypeCast:
                  OP.second = RSN->GetCastReturnType();
                  break;
                case ASTTypeMeasure:
                  OP.second = RSN->GetMeasure()->GetResultType();
                  break;
                default:
                  if (RSN->IsImplicitConversion())
                    OP.second = RSN->GetImplicitConversionType();
                  else
                    OP.second = RSN->GetReturnType();
                  break;
                }

                if (OP.first != OP.second) {
                  if (!CheckReturnType(RSN, OP))
                    return RSN;
                }
              } else {
              }
            } else {
              switch ((*FI)->GetASTType()) {
              case ASTTypeIfStatement: {
                if (const ASTIfStatementNode* IFS =
                    dynamic_cast<const ASTIfStatementNode*>(*FI)) {
                  const ASTStatementList* ISL = IFS->GetOpList();
                  assert(ISL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*ISL, OP, IFS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeElseIfStatement: {
                if (const ASTElseIfStatementNode* EIFS =
                    dynamic_cast<const ASTElseIfStatementNode*>(*FI)) {
                  const ASTStatementList* EISL = EIFS->GetOpList();
                  assert(EISL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*EISL, OP, EIFS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeElseStatement: {
                if (const ASTElseStatementNode* ES =
                    dynamic_cast<const ASTElseStatementNode*>(*FI)) {
                  const ASTStatementList* ESL = ES->GetOpList();
                  assert(ESL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*ESL, OP, ES->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeForStatement: {
                if (const ASTForStatementNode* FFOS =
                    dynamic_cast<const ASTForStatementNode*>(*FI)) {
                  const ASTForLoopNode* FFOL = FFOS->GetLoop();
                  assert(FFOL && "Could not obtain a valid ASTForLoopNode!");

                  const ASTStatementList& FFSL = FFOL->GetStatementList();
                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(FFSL, OP, FFOS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeWhileStatement: {
                if (const ASTWhileStatementNode* WS =
                    dynamic_cast<const ASTWhileStatementNode*>(*FI)) {
                  const ASTWhileLoopNode* WL = WS->GetLoop();
                  assert(WL && "Could not obtain a valid ASTWhileLoopNode!");

                  const ASTStatementList& WSL = WL->GetStatementList();
                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(WSL, OP, WS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeDoWhileStatement: {
                if (const ASTDoWhileStatementNode* DWS =
                    dynamic_cast<const ASTDoWhileStatementNode*>(*FI)) {
                  const ASTDoWhileLoopNode* DWL = DWS->GetLoop();
                  assert(DWL && "Could not obtain a valid ASTDoWhileLoopNode!");

                  const ASTStatementList& DWSL = DWL->GetStatementList();
                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(DWSL, OP, DWS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeSwitchStatement: {
                if (const ASTSwitchStatementNode* SWS =
                    dynamic_cast<const ASTSwitchStatementNode*>(*FI)) {
                  if (SWS->GetNumCaseStatements() == 0) {
                    std::stringstream M;
                    M << "Switch statement with no case labels.";
                    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                      DIAGLineCounter::Instance().GetLocation(SWS), M.str(),
                                                                    DiagLevel::Error);
                    return ASTReturnStatementNode::StatementError(M.str());
                  }

                  for (unsigned I = 0; I < SWS->GetNumCaseStatements(); ++I) {
                    const ASTCaseStatementNode* CSN = SWS->GetCaseStatement(I);
                    assert(CSN && "Could not obtain a valid ASTCaseStatementNode!");

                    const ASTStatementList* CSL = CSN->GetStatementList();
                    assert(CSL && "Could not obtain a valid ASTStatementList!");

                    if (const ASTReturnStatementNode* RSN =
                        CheckReturnStatements(*CSL, OP, CSN->GetASTType()))
                      return RSN;
                  }

                  const ASTDefaultStatementNode* DSN = SWS->GetDefaultStatement();
                  if (!DSN) {
                    std::stringstream M;
                    M << "Switch statement without a default label.";
                    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                      DIAGLineCounter::Instance().GetLocation(SWS), M.str(),
                                                                    DiagLevel::Warning);
                  } else {
                    const ASTStatementList* DSL = DSN->GetStatementList();
                    assert(DSL && "Could not obtain a valid ASTStatementList!");

                    if (const ASTReturnStatementNode* RSN =
                        CheckReturnStatements(*DSL, OP, DSN->GetASTType()))
                      return RSN;

                  }
                }
              }
                break;
              default:
                break;
              }
            }
          }
        }
      }
    }
      break;
    case ASTTypeWhileStatement: {
      if (const ASTWhileStatementNode* WHS =
          dynamic_cast<const ASTWhileStatementNode*>(SN)) {
        if (const ASTWhileLoopNode* WOL = WHS->GetLoop()) {
          const ASTStatementList& WSL = WOL->GetStatementList();

          for (ASTStatementList::const_iterator WI = WSL.begin();
               WI != WSL.end(); ++WI) {
            if ((*WI)->GetASTType() == ASTTypeReturn) {
              if (const ASTReturnStatementNode* RSN =
                  dynamic_cast<const ASTReturnStatementNode*>(*WI)) {
                switch (RSN->GetReturnType()) {
                case ASTTypeCast:
                  OP.second = RSN->GetCastReturnType();
                  break;
                case ASTTypeMeasure:
                  OP.second = RSN->GetMeasure()->GetResultType();
                  break;
                default:
                  if (RSN->IsImplicitConversion())
                    OP.second = RSN->GetImplicitConversionType();
                  else
                    OP.second = RSN->GetReturnType();
                  break;
                }

                if (OP.first != OP.second) {
                  if (!CheckReturnType(RSN, OP))
                    return RSN;
                }
              }
            } else {
              switch ((*WI)->GetASTType()) {
              case ASTTypeIfStatement: {
                if (const ASTIfStatementNode* IFS =
                    dynamic_cast<const ASTIfStatementNode*>(*WI)) {
                  const ASTStatementList* ISL = IFS->GetOpList();
                  assert(ISL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*ISL, OP, IFS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeElseIfStatement: {
                if (const ASTElseIfStatementNode* EIFS =
                    dynamic_cast<const ASTElseIfStatementNode*>(*WI)) {
                  const ASTStatementList* EISL = EIFS->GetOpList();
                  assert(EISL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*EISL, OP, EIFS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeElseStatement: {
                if (const ASTElseStatementNode* ES =
                    dynamic_cast<const ASTElseStatementNode*>(*WI)) {
                  const ASTStatementList* ESL = ES->GetOpList();
                  assert(ESL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*ESL, OP, ES->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeForStatement: {
                if (const ASTForStatementNode* FOS =
                    dynamic_cast<const ASTForStatementNode*>(*WI)) {
                  const ASTForLoopNode* FOL = FOS->GetLoop();
                  assert(FOL && "Could not obtain a valid ASTForLoopNode!");

                  const ASTStatementList& FSL = FOL->GetStatementList();
                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(FSL, OP, FOS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeWhileStatement: {
                if (const ASTWhileStatementNode* WS =
                    dynamic_cast<const ASTWhileStatementNode*>(*WI)) {
                  const ASTWhileLoopNode* WL = WS->GetLoop();
                  assert(WL && "Could not obtain a valid ASTWhileLoopNode!");

                  const ASTStatementList& WWSL = WL->GetStatementList();
                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(WWSL, OP, WS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeDoWhileStatement: {
                if (const ASTDoWhileStatementNode* DWS =
                    dynamic_cast<const ASTDoWhileStatementNode*>(*WI)) {
                  const ASTDoWhileLoopNode* DWL = DWS->GetLoop();
                  assert(DWL && "Could not obtain a valid ASTDoWhileLoopNode!");

                  const ASTStatementList& DWSL = DWL->GetStatementList();
                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(DWSL, OP, DWS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeSwitchStatement: {
                if (const ASTSwitchStatementNode* SWS =
                    dynamic_cast<const ASTSwitchStatementNode*>(*WI)) {
                  if (SWS->GetNumCaseStatements() == 0) {
                    std::stringstream M;
                    M << "Switch statement with no case labels.";
                    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                      DIAGLineCounter::Instance().GetLocation(SWS), M.str(),
                                                                    DiagLevel::Error);
                    return ASTReturnStatementNode::StatementError(M.str());
                  }

                  for (unsigned I = 0; I < SWS->GetNumCaseStatements(); ++I) {
                    const ASTCaseStatementNode* CSN = SWS->GetCaseStatement(I);
                    assert(CSN && "Could not obtain a valid ASTCaseStatementNode!");

                    const ASTStatementList* CSL = CSN->GetStatementList();
                    assert(CSL && "Could not obtain a valid ASTStatementList!");

                    if (const ASTReturnStatementNode* RSN =
                        CheckReturnStatements(*CSL, OP, CSN->GetASTType()))
                      return RSN;
                  }

                  const ASTDefaultStatementNode* DSN = SWS->GetDefaultStatement();
                  if (!DSN) {
                    std::stringstream M;
                    M << "Switch statement without a default label.";
                    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                      DIAGLineCounter::Instance().GetLocation(SWS), M.str(),
                                                                    DiagLevel::Warning);
                  } else {
                    const ASTStatementList* DSL = DSN->GetStatementList();
                    assert(DSL && "Could not obtain a valid ASTStatementList!");

                    if (const ASTReturnStatementNode* RSN =
                        CheckReturnStatements(*DSL, OP, DSN->GetASTType()))
                      return RSN;

                  }
                }
              }
                break;
              default:
                break;
              }
            }
          }
        }
      }
    }
      break;
    case ASTTypeDoWhileStatement: {
      if (const ASTDoWhileStatementNode* DWHS =
          dynamic_cast<const ASTDoWhileStatementNode*>(SN)) {
        if (const ASTDoWhileLoopNode* DWOL = DWHS->GetLoop()) {
          const ASTStatementList& DWSL = DWOL->GetStatementList();

          for (ASTStatementList::const_iterator WI = DWSL.begin();
               WI != DWSL.end(); ++WI) {
            if ((*WI)->GetASTType() == ASTTypeReturn) {
              if (const ASTReturnStatementNode* RSN =
                  dynamic_cast<const ASTReturnStatementNode*>(*WI)) {
                switch (RSN->GetReturnType()) {
                case ASTTypeCast:
                  OP.second = RSN->GetCastReturnType();
                  break;
                case ASTTypeMeasure:
                  OP.second = RSN->GetMeasure()->GetResultType();
                  break;
                default:
                  if (RSN->IsImplicitConversion())
                    OP.second = RSN->GetImplicitConversionType();
                  else
                    OP.second = RSN->GetReturnType();
                  break;
                }

                if (OP.first != OP.second) {
                  if (!CheckReturnType(RSN, OP))
                    return RSN;
                }
              }
            } else {
              switch ((*WI)->GetASTType()) {
              case ASTTypeIfStatement: {
                if (const ASTIfStatementNode* IFS =
                    dynamic_cast<const ASTIfStatementNode*>(*WI)) {
                  const ASTStatementList* ISL = IFS->GetOpList();
                  assert(ISL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*ISL, OP, IFS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeElseIfStatement: {
                if (const ASTElseIfStatementNode* EIFS =
                    dynamic_cast<const ASTElseIfStatementNode*>(*WI)) {
                  const ASTStatementList* EISL = EIFS->GetOpList();
                  assert(EISL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*EISL, OP, EIFS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeElseStatement: {
                if (const ASTElseStatementNode* ES =
                    dynamic_cast<const ASTElseStatementNode*>(*WI)) {
                  const ASTStatementList* ESL = ES->GetOpList();
                  assert(ESL && "Could not obtain a valid ASTStatementList!");

                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(*ESL, OP, ES->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeForStatement: {
                if (const ASTForStatementNode* FOS =
                    dynamic_cast<const ASTForStatementNode*>(*WI)) {
                  const ASTForLoopNode* FOL = FOS->GetLoop();
                  assert(FOL && "Could not obtain a valid ASTForLoopNode!");

                  const ASTStatementList& FSL = FOL->GetStatementList();
                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(FSL, OP, FOS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeWhileStatement: {
                if (const ASTWhileStatementNode* WS =
                    dynamic_cast<const ASTWhileStatementNode*>(*WI)) {
                  const ASTWhileLoopNode* WL = WS->GetLoop();
                  assert(WL && "Could not obtain a valid ASTWhileLoopNode!");

                  const ASTStatementList& WSL = WL->GetStatementList();
                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(WSL, OP, WS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeDoWhileStatement: {
                if (const ASTDoWhileStatementNode* DWS =
                    dynamic_cast<const ASTDoWhileStatementNode*>(*WI)) {
                  const ASTDoWhileLoopNode* DWL = DWS->GetLoop();
                  assert(DWL && "Could not obtain a valid ASTDoWhileLoopNode!");

                  const ASTStatementList& DDWSL = DWL->GetStatementList();
                  if (const ASTReturnStatementNode* RSN =
                      CheckReturnStatements(DDWSL, OP, DWS->GetASTType()))
                    return RSN;
                }
              }
                break;
              case ASTTypeSwitchStatement: {
                if (const ASTSwitchStatementNode* SWS =
                    dynamic_cast<const ASTSwitchStatementNode*>(*WI)) {
                  if (SWS->GetNumCaseStatements() == 0) {
                    std::stringstream M;
                    M << "Switch statement with no case labels.";
                    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                      DIAGLineCounter::Instance().GetLocation(SWS), M.str(),
                                                                    DiagLevel::Error);
                    return ASTReturnStatementNode::StatementError(M.str());
                  }

                  for (unsigned I = 0; I < SWS->GetNumCaseStatements(); ++I) {
                    const ASTCaseStatementNode* CSN = SWS->GetCaseStatement(I);
                    assert(CSN && "Could not obtain a valid ASTCaseStatementNode!");

                    const ASTStatementList* CSL = CSN->GetStatementList();
                    assert(CSL && "Could not obtain a valid ASTStatementList!");

                    if (const ASTReturnStatementNode* RSN =
                        CheckReturnStatements(*CSL, OP, CSN->GetASTType()))
                      return RSN;
                  }

                  const ASTDefaultStatementNode* DSN = SWS->GetDefaultStatement();
                  if (!DSN) {
                    std::stringstream M;
                    M << "Switch statement without a default label.";
                    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                      DIAGLineCounter::Instance().GetLocation(SWS), M.str(),
                                                                    DiagLevel::Warning);
                  } else {
                    const ASTStatementList* DSL = DSN->GetStatementList();
                    assert(DSL && "Could not obtain a valid ASTStatementList!");

                    if (const ASTReturnStatementNode* RSN =
                        CheckReturnStatements(*DSL, OP, DSN->GetASTType()))
                      return RSN;

                  }
                }
              }
                break;
              default:
                break;
              }
            }
          }
        }
      }
    }
      break;
    case ASTTypeSwitchStatement: {
      if (const ASTSwitchStatementNode* SWS =
          dynamic_cast<const ASTSwitchStatementNode*>(SN)) {
        for (unsigned I = 0; I < SWS->GetNumCaseStatements(); ++I) {
          if (const ASTCaseStatementNode* CSN = SWS->GetCaseStatement(I)) {
            if (const ASTStatementList* CSL = CSN->GetStatementList()) {
              if (const ASTReturnStatementNode* RSN =
                  CheckReturnStatements(*CSL, OP, CSN->GetASTType()))
                return RSN;
            }
          }
        }

        if (const ASTDefaultStatementNode* DSN = SWS->GetDefaultStatement()) {
          if (const ASTStatementList* DSL = DSN->GetStatementList()) {
            if (const ASTReturnStatementNode* RSN =
                CheckReturnStatements(*DSL, OP, DSN->GetASTType()))
              return RSN;
          }
        }
      }
    }
      break;
    case ASTTypeReturn: {
      if (const ASTReturnStatementNode* RSN =
          dynamic_cast<const ASTReturnStatementNode*>(SN)) {
        switch (RSN->GetReturnType()) {
        case ASTTypeCast:
          OP.second = RSN->GetCastReturnType();
          break;
        case ASTTypeMeasure:
          OP.second = RSN->GetMeasure()->GetResultType();
          break;
        default:
          if (RSN->IsImplicitConversion())
            OP.second = RSN->GetImplicitConversionType();
          else
            OP.second = RSN->GetReturnType();
          break;
        }

        if (OP.first != OP.second) {
          if (!CheckReturnType(RSN, OP))
            return RSN;
        }
      }
    }
      break;
    default:
      break;
    }
  }

  return nullptr;
}

const ASTReturnStatementNode*
ASTFunctionDefinitionNode::CheckReturnStatements(const ASTStatementList& SL,
                                      std::pair<ASTType, ASTType>& OP,
                                      ASTType STy) const {
  OP.first = Result->GetResultType();

  for (ASTStatementList::const_iterator SI = SL.begin(); SI != SL.end(); ++SI) {
    const ASTStatementNode* SN = dynamic_cast<const ASTStatementNode*>(*SI);
    assert(SN && "Could not obtain a valid ASTStatementNode!");

    switch (SN->GetASTType()) {
    case ASTTypeReturn: {
      if (const ASTReturnStatementNode* RSN =
          dynamic_cast<const ASTReturnStatementNode*>(SN)) {
        switch (RSN->GetReturnType()) {
        case ASTTypeCast:
          OP.second = RSN->GetCastReturnType();
          break;
        case ASTTypeMeasure:
          OP.second = RSN->GetMeasure()->GetResultType();
          break;
        default:
          if (RSN->IsImplicitConversion())
            OP.second = RSN->GetImplicitConversionType();
          else
            OP.second = RSN->GetReturnType();
          break;
        }

        if (OP.first != OP.second) {
          if (!CheckReturnType(RSN, OP))
            return RSN;
        }
      } else {
        std::stringstream M;
        M << "Could not dynamic_cast to a valid ASTReturnStatementNode.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(SN), M.str(), DiagLevel::ICE);
        return ASTReturnStatementNode::StatementError(M.str());
      }
    }
      break;
    case ASTTypeIfStatement: {
      if (const ASTIfStatementNode* IFS =
          dynamic_cast<const ASTIfStatementNode*>(SN)) {
        const ASTStatementList* ISL = IFS->GetOpList();
        assert(ISL && "Could not obtain a valid ASTStatementList!");

        for (ASTStatementList::const_iterator ISI = ISL->begin();
             ISI != ISL->end(); ++ISI) {
          if ((*ISI)->GetASTType() == ASTTypeReturn) {
            if (const ASTReturnStatementNode* RSN =
                dynamic_cast<const ASTReturnStatementNode*>(*ISI)) {
              switch (RSN->GetReturnType()) {
              case ASTTypeCast:
                OP.second = RSN->GetCastReturnType();
                break;
              case ASTTypeMeasure:
                OP.second = RSN->GetMeasure()->GetResultType();
                break;
              default:
                if (RSN->IsImplicitConversion())
                  OP.second = RSN->GetImplicitConversionType();
                else
                  OP.second = RSN->GetReturnType();
                break;
              }

              if (OP.first != OP.second) {
                if (!CheckReturnType(RSN, OP))
                  return RSN;
              }
            }
          }
        }
      }
    }
      break;
    case ASTTypeElseIfStatement: {
      if (const ASTElseIfStatementNode* EIFS =
          dynamic_cast<const ASTElseIfStatementNode*>(SN)) {
        const ASTStatementList* ISL = EIFS->GetOpList();
        assert(ISL && "Could not obtain a valid ASTStatementList!");

        for (ASTStatementList::const_iterator ISI = ISL->begin();
             ISI != ISL->end(); ++ISI) {
          if ((*ISI)->GetASTType() == ASTTypeReturn) {
            if (const ASTReturnStatementNode* RSN =
                dynamic_cast<const ASTReturnStatementNode*>(*ISI)) {
              switch (RSN->GetReturnType()) {
              case ASTTypeCast:
                OP.second = RSN->GetCastReturnType();
                break;
              case ASTTypeMeasure:
                OP.second = RSN->GetMeasure()->GetResultType();
                break;
              default:
                if (RSN->IsImplicitConversion())
                  OP.second = RSN->GetImplicitConversionType();
                else
                  OP.second = RSN->GetReturnType();
                break;
              }

              if (OP.first != OP.second) {
                if (!CheckReturnType(RSN, OP))
                  return RSN;
              }
            }
          }
        }
      }
    }
      break;
    case ASTTypeElseStatement: {
      if (const ASTElseStatementNode* EFS =
          dynamic_cast<const ASTElseStatementNode*>(SN)) {
        const ASTStatementList* ISL = EFS->GetOpList();
        assert(ISL && "Could not obtain a valid ASTStatementList!");

        for (ASTStatementList::const_iterator ISI = ISL->begin();
             ISI != ISL->end(); ++ISI) {
          if ((*ISI)->GetASTType() == ASTTypeReturn) {
            if (const ASTReturnStatementNode* RSN =
                dynamic_cast<const ASTReturnStatementNode*>(*ISI)) {
              switch (RSN->GetReturnType()) {
              case ASTTypeCast:
                OP.second = RSN->GetCastReturnType();
                break;
              case ASTTypeMeasure:
                OP.second = RSN->GetMeasure()->GetResultType();
                break;
              default:
                if (RSN->IsImplicitConversion())
                  OP.second = RSN->GetImplicitConversionType();
                else
                  OP.second = RSN->GetReturnType();
                break;
              }

              if (OP.first != OP.second) {
                if (!CheckReturnType(RSN, OP))
                  return RSN;
              }
            }
          }
        }
      }
    }
      break;
    case ASTTypeForStatement: {
      if (const ASTForStatementNode* FOS =
          dynamic_cast<const ASTForStatementNode*>(SN)) {
        if (const ASTForLoopNode* FOL = FOS->GetLoop()) {
          const ASTStatementList& FSL = FOL->GetStatementList();

          for (ASTStatementList::const_iterator FI = FSL.begin();
               FI != FSL.end(); ++FI) {
            if ((*FI)->GetASTType() == ASTTypeReturn) {
              if (const ASTReturnStatementNode* RSN =
                  dynamic_cast<const ASTReturnStatementNode*>(*FI)) {
                switch (RSN->GetReturnType()) {
                case ASTTypeCast:
                  OP.second = RSN->GetCastReturnType();
                  break;
                case ASTTypeMeasure:
                  OP.second = RSN->GetMeasure()->GetResultType();
                  break;
                default:
                  if (RSN->IsImplicitConversion())
                    OP.second = RSN->GetImplicitConversionType();
                  else
                    OP.second = RSN->GetReturnType();
                  break;
                }

                if (OP.first != OP.second) {
                  if (!CheckReturnType(RSN, OP))
                    return RSN;
                }
              }
            }
          }
        }
      }
    }
      break;
    case ASTTypeWhileStatement: {
      if (const ASTWhileStatementNode* WHS =
          dynamic_cast<const ASTWhileStatementNode*>(SN)) {
        if (const ASTWhileLoopNode* WOL = WHS->GetLoop()) {
          const ASTStatementList& WSL = WOL->GetStatementList();

          for (ASTStatementList::const_iterator WI = WSL.begin();
               WI != WSL.end(); ++WI) {
            if ((*WI)->GetASTType() == ASTTypeReturn) {
              if (const ASTReturnStatementNode* RSN =
                  dynamic_cast<const ASTReturnStatementNode*>(*WI)) {
                switch (RSN->GetReturnType()) {
                case ASTTypeCast:
                  OP.second = RSN->GetCastReturnType();
                  break;
                case ASTTypeMeasure:
                  OP.second = RSN->GetMeasure()->GetResultType();
                  break;
                default:
                  if (RSN->IsImplicitConversion())
                    OP.second = RSN->GetImplicitConversionType();
                  else
                    OP.second = RSN->GetReturnType();
                  break;
                }

                if (OP.first != OP.second) {
                  if (!CheckReturnType(RSN, OP))
                    return RSN;
                }
              }
            }
          }
        }
      }
    }
      break;
    case ASTTypeSwitchStatement: {
      if (const ASTSwitchStatementNode* SWS =
          dynamic_cast<const ASTSwitchStatementNode*>(SN)) {
        for (unsigned I = 0; I < SWS->GetNumCaseStatements(); ++I) {
          if (const ASTCaseStatementNode* CSN = SWS->GetCaseStatement(I)) {
            if (const ASTStatementList* CSL = CSN->GetStatementList()) {
              if (const ASTReturnStatementNode* RSN =
                  CheckReturnStatements(*CSL, OP, SWS->GetASTType()))
                return RSN;
            }
          }
        }

        if (const ASTDefaultStatementNode* DSN = SWS->GetDefaultStatement()) {
          if (const ASTStatementList* DSL = DSN->GetStatementList()) {
            if (const ASTReturnStatementNode* RSN =
                CheckReturnStatements(*DSL, OP, SWS->GetASTType()))
              return RSN;
          }
        }
      }
    }
      break;
    case ASTTypeCaseStatement: {
      if (STy != ASTTypeSwitchStatement) {
        std::stringstream M;
        M << "A case label is only allowed inside a Switch statement.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(SN), M.str(),
                                                       DiagLevel::Error);
        return ASTReturnStatementNode::StatementError(M.str());
      }

      if (const ASTCaseStatementNode* CSN =
          dynamic_cast<const ASTCaseStatementNode*>(SN)) {
        if (const ASTReturnStatementNode* RSN = CheckReturnStatement(CSN, OP))
          return RSN;
      }
    }
      break;
    case ASTTypeDefaultStatement: {
      if (STy != ASTTypeSwitchStatement) {
        std::stringstream M;
        M << "A default label is only allowed inside a Switch statement.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(SN), M.str(),
                                                       DiagLevel::Error);
        return ASTReturnStatementNode::StatementError(M.str());
      }

      if (const ASTDefaultStatementNode* DSN =
          dynamic_cast<const ASTDefaultStatementNode*>(SN)) {
        if (const ASTReturnStatementNode* RSN = CheckReturnStatement(DSN, OP))
          return RSN;
      }
    }
      break;
    default:
      break;
    }
  }

  return nullptr;
}

const ASTReturnStatementNode*
ASTFunctionDefinitionNode::CheckReturnStatement(const ASTIfStatementNode* IFS,
                                      std::pair<ASTType, ASTType>& OP) const {
  assert(IFS && "Invalid ASTIfStatementNode argument!");

  if (const ASTStatementList* CSL = IFS->GetOpList()) {
    if (const ASTReturnStatementNode* RSN =
        CheckReturnStatements(*CSL, OP, IFS->GetASTType())) {
      return RSN;
    }
  }

  return nullptr;
}

const ASTReturnStatementNode*
ASTFunctionDefinitionNode::CheckReturnStatement(const ASTElseIfStatementNode* EIS,
                                      std::pair<ASTType, ASTType>& OP) const {
  assert(EIS && "Invalid ASTElseIfStatementNode argument!");

  if (const ASTStatementList* CSL = EIS->GetOpList()) {
    if (const ASTReturnStatementNode* RSN =
        CheckReturnStatements(*CSL, OP, EIS->GetASTType())) {
      return RSN;
    }
  }

  return nullptr;
}

const ASTReturnStatementNode*
ASTFunctionDefinitionNode::CheckReturnStatement(const ASTElseStatementNode* ES,
                                      std::pair<ASTType, ASTType>& OP) const {
  assert(ES && "Invalid ASTElseStatementNode argument!");

  if (const ASTStatementList* CSL = ES->GetOpList()) {
    if (const ASTReturnStatementNode* RSN =
        CheckReturnStatements(*CSL, OP, ES->GetASTType())) {
      return RSN;
    }
  }

  return nullptr;
}

const ASTReturnStatementNode*
ASTFunctionDefinitionNode::CheckReturnStatement(const ASTForStatementNode* FS,
                                      std::pair<ASTType, ASTType>& OP) const {
  assert(FS && "Invalid ASTForStatementNode argument!");

  const ASTForLoopNode* FLN = FS->GetLoop();
  assert(FLN && "Could not obtain a valid ASTForLoopNode!");

  const ASTStatementList& CSL = FLN->GetStatementList();

  if (!CSL.Empty()) {
    if (const ASTReturnStatementNode* RSN =
        CheckReturnStatements(CSL, OP, FS->GetASTType())) {
      return RSN;
    }
  }

  return nullptr;
}

const ASTReturnStatementNode*
ASTFunctionDefinitionNode::CheckReturnStatement(const ASTWhileStatementNode* WS,
                                      std::pair<ASTType, ASTType>& OP) const {
  assert(WS && "Invalid ASTWhileStatementNode argument!");

  const ASTWhileLoopNode* WLN = WS->GetLoop();
  assert(WLN && "Could not obtain a valid ASTWhileLoopNode!");

  const ASTStatementList& CSL = WLN->GetStatementList();

  if (!CSL.Empty()) {
    if (const ASTReturnStatementNode* RSN =
        CheckReturnStatements(CSL, OP, WS->GetASTType())) {
      return RSN;
    }
  }

  return nullptr;
}

const ASTReturnStatementNode*
ASTFunctionDefinitionNode::CheckReturnStatement(const ASTDoWhileStatementNode* DWS,
                                      std::pair<ASTType, ASTType>& OP) const {
  assert(DWS && "Invalid ASTDoWhileStatementNode argument!");

  const ASTDoWhileLoopNode* DWLN = DWS->GetLoop();
  assert(DWLN && "Could not obtain a valid ASTDoWhileLoopNode!");

  const ASTStatementList& CSL = DWLN->GetStatementList();

  if (!CSL.Empty()) {
    if (const ASTReturnStatementNode* RSN =
        CheckReturnStatements(CSL, OP, DWS->GetASTType())) {
      return RSN;
    }
  }

  return nullptr;
}

const ASTReturnStatementNode*
ASTFunctionDefinitionNode::CheckReturnStatement(const ASTCaseStatementNode* CSN,
                                      std::pair<ASTType, ASTType>& OP) const {
  assert(CSN && "Invalid ASTCaseStatementNode argument!");

  if (const ASTStatementList* CSL = CSN->GetStatementList()) {
    if (!CSL->Empty()) {
      if (const ASTReturnStatementNode* RSN =
          CheckReturnStatements(*CSL, OP, CSN->GetASTType())) {
        return RSN;
      }
    }
  }

  return nullptr;
}

const ASTReturnStatementNode*
ASTFunctionDefinitionNode::CheckReturnStatement(const ASTDefaultStatementNode* DSN,
                                      std::pair<ASTType, ASTType>& OP) const {
  assert(DSN && "Invalid ASTDefaultStatementNode argument!");

  if (const ASTStatementList* CSL = DSN->GetStatementList()) {
    if (!CSL->Empty()) {
      if (const ASTReturnStatementNode* RSN =
          CheckReturnStatements(*CSL, OP, DSN->GetASTType())) {
        return RSN;
      }
    }
  }

  return nullptr;
}

const ASTReturnStatementNode*
ASTFunctionDefinitionNode::CheckReturnStatement(const ASTSwitchStatementNode* SWS,
                                      std::pair<ASTType, ASTType>& OP) const {
  assert(SWS && "Invalid ASTSwitchStatementNode argument!");

  using map_type = typename std::map<unsigned, const ASTCaseStatementNode*>;
  using map_iterator = typename map_type::const_iterator;

  const map_type& CSM = SWS->GetCaseStatementsMap();

  if (CSM.empty()) {
    std::stringstream M;
    M << "Empty Switch statement without case labels.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(SWS), M.str(), DiagLevel::Warning);
  } else {
    ASTStatementList CSL;

    for (map_iterator MI = CSM.begin(); MI != CSM.end(); ++MI) {
      if (const ASTStatementNode* CSN =
          dynamic_cast<const ASTStatementNode*>((*MI).second)) {
        CSL.Append(const_cast<ASTStatementNode*>(CSN));
      }
    }

    if (const ASTReturnStatementNode* RSN =
        CheckReturnStatements(CSL, OP, SWS->GetASTType())) {
      return RSN;
    }
  }

  const ASTDefaultStatementNode* DSN = SWS->GetDefaultStatement();
  if (DSN) {
    if (const ASTStatementList* DSL = DSN->GetStatementList()) {
      if (!DSL->Empty()) {
        if (const ASTReturnStatementNode* RSN =
            CheckReturnStatements(*DSL, OP, SWS->GetASTType()))
          return RSN;
      }
    }
  } else {
    std::stringstream M;
    M << "Switch statement without a default case label.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(SWS), M.str(), DiagLevel::Warning);
    return nullptr;
  }

  return nullptr;
}

void ASTFunctionDefinitionNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());

  if (Extern)
    M.Type(ASTTypeExtern);

  M.FuncReturn(Result->GetResultType(), Result->GetResultBits());

  for (std::map<unsigned, ASTDeclarationNode*>::const_iterator PI = Params.begin();
       PI != Params.end(); ++PI) {
    const ASTIdentifierNode* DId = (*PI).second->GetIdentifier();
    assert(DId && "Invalid ASTIdentifierNode for function parameter!");
    M.FuncParam((*PI).first, DId->GetSymbolType(), DId->GetBits(),
                DId->GetName());
  }

  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTFunctionDeclarationNode::Mangle() {
  const ASTFunctionDefinitionNode* FDN = GetDefinition();
  if (!FDN) {
    std::stringstream M;
    M << "A function declaration without a valid definition "
      << "is not allowed.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  }

  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(),
                   ASTStringUtils::Instance().SanitizeMangled(FDN->GetMangledName()));
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTReturnStatementNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace QASM

