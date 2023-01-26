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

#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTStringUtils.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTPragma.h>
#include <qasm/AST/ASTAnyTypeList.h>
#include <qasm/AST/ASTDynamicTypeCast.h>
#include <qasm/AST/ASTAngleNodeList.h>
#include <qasm/AST/ASTAngleContextControl.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTExpressionBuilder.h>
#include <qasm/AST/ASTDeclarationBuilder.h>
#include <qasm/AST/ASTFunctionDefinitionBuilder.h>
#include <qasm/AST/ASTKernelBuilder.h>
#include <qasm/AST/ASTDefcalBuilder.h>
#include <qasm/AST/ASTGateNodeBuilder.h>
#include <qasm/AST/ASTGateOpBuilder.h>
#include <qasm/AST/ASTPragmaContextBuilder.h>
#include <qasm/AST/ASTObjectTracker.h>
#include <qasm/AST/ASTScopeController.h>
#include <qasm/AST/ASTUtils.h>
#include <qasm/AST/ASTGateControl.h>
#include <qasm/AST/ASTExpressionValidator.h>
#include <qasm/AST/ASTExpressionEvaluator.h>
#include <qasm/AST/ASTTypeCastController.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <iomanip>
#include <cassert>

namespace QASM {

ASTBuilder ASTBuilder::Builder;
uint64_t ASTBuilder::IdentCounter;

ASTStringUtils ASTStringUtils::SU;
std::regex ASTStringUtils::ZRD("[0]+.[0]+");
std::regex ASTStringUtils::ZRZ("[0]+");
std::regex ASTStringUtils::MPNAN("@?[Nn]a[Nn]@?");
std::regex ASTStringUtils::MPINF("@?[Ii]nf@?");
std::string ASTStringUtils::ES("");

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTOpenQASMStatementNode*
ASTBuilder::CreateASTOpenQASMStatementNode(const std::string& OQS,
                                           const std::string& OQV) {
  assert(!OQS.empty() && "Invalid OpenQASM Declaration argument!");
  assert(!OQV.empty() && "Invalid OpenQASM Version argument!");

  ASTIdentifierNode* Id =
    FindOrCreateASTIdentifierNode(OQS, 64, ASTTypeOpenQASMExpression);
  assert(Id && "Could not create a valid ASTIdentifierNode!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not obtain a valid OpenQASM SymbolTable Entry!");

  ASTOpenQASMExpressionNode* OQE =
    new ASTOpenQASMExpressionNode(Id, std::stod(OQV));
  assert(OQE && "Could not create a valid ASTOpenQASMExpressionNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(OQE, ASTTypeOpenQASMExpression),
                ASTTypeOpenQASMExpression);
  assert(STE->HasValue() && "SymbolTable Entry for OPENQASM Declaration "
                            "has no Value!");

  return new ASTOpenQASMStatementNode(OQE);
}

ASTArgumentNode*
ASTBuilder::CreateASTArgumentNode(ASTIdentifierNode* Id,
                                  const std::any& Val,
                                  ASTType Ty) {
  return new ASTArgumentNode(Id, Val, Ty);
}

ASTArgumentNode*
ASTBuilder::CreateASTArgumentNode(ASTIdentifierRefNode* IdRef,
                                  const std::any& Val,
                                  ASTType Ty) {
  const ASTIdentifierNode* Id = IdRef->GetIdentifier();
  return CreateASTArgumentNode(const_cast<ASTIdentifierNode*>(Id), Val, Ty);
}

ASTExpressionNode*
ASTBuilder::CreateASTExpressionNode(const ASTExpression* E, ASTType Ty) {
  assert(E && "Invalid ASTExpression argument!");

  if (const ASTIdentifierNode* Id =
      dynamic_cast<const ASTIdentifierNode*>(E)) {
    return new ASTExpressionNode(Id, Ty);
  } else if (const ASTIdentifierRefNode* IdR =
             dynamic_cast<const ASTIdentifierRefNode*>(E)) {
    return new ASTExpressionNode(IdR, Ty);
  }

  std::stringstream SID;
  SID << "ast-expression-node-" << ++IdentCounter;

  const ASTIdentifierNode* IDN =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(SID.str());
  assert(IDN && "Failed to create an ASTExpression ASTIdentifierNode!");

  return new ASTExpressionNode(E, IDN, Ty);
}

ASTExpressionNode*
ASTBuilder::CreateASTExpressionNode(const std::string& Identifier,
                                    const ASTExpression* E, ASTType Ty) {
  assert(E && "Invalid ASTExpression argument!");

  if (const ASTIdentifierNode* Id =
      dynamic_cast<const ASTIdentifierNode*>(E)) {
    return new ASTExpressionNode(Id, Ty);
  } else if (const ASTIdentifierRefNode* IdR =
             dynamic_cast<const ASTIdentifierRefNode*>(E)) {
    return new ASTExpressionNode(IdR, Ty);
  }

  const ASTIdentifierNode* IDN =
    ASTBuilder::Instance().FindASTIdentifierNode(Identifier);
  assert(IDN && "Failed to create an ASTExpression ASTIdentifierNode!");

  return new ASTExpressionNode(E, IDN, Ty);
}

ASTExpressionNode*
ASTBuilder::CreateASTIdentifierExpressionNode(const ASTIdentifierNode* Id) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTExpressionNode* EX = nullptr;
  const ASTExpressionNode* IEX = Id->GetExpression();

  if (IEX) {
    if (Id->IsReference()) {
      if (const ASTIdentifierRefNode* IdR =
          dynamic_cast<const ASTIdentifierRefNode*>(Id)) {
        EX = new ASTExpressionNode(IdR, IEX, Id->GetASTType());
      }
    } else {
      EX = new ASTExpressionNode(Id, IEX, Id->GetASTType());
    }
  } else {
    if (Id->IsReference()) {
      if (const ASTIdentifierRefNode* IdR =
          dynamic_cast<const ASTIdentifierRefNode*>(Id)) {
        EX = new ASTExpressionNode(IdR, Id->GetASTType());
      }
    } else {
      EX = new ASTExpressionNode(Id, Id->GetASTType());
    }
  }

  assert(EX && "Could not create a valid Identifier ASTExpressionNode!");
  return EX;
}

ASTOperatorNode*
ASTBuilder::CreateASTOperatorNode(const ASTIdentifierNode* TId,
                                  ASTOpType OTy) {
  assert(TId && "Invalid ASTIdentifierNode argument!");

  ASTOperatorNode* OPX = new ASTOperatorNode(TId, OTy);
  assert(OPX && "Could not create a valid ASTOperatorNode!");

  OPX->Mangle();
  return OPX;
}

ASTOperatorNode*
ASTBuilder::CreateASTOperatorNode(const ASTExpressionNode* TEx,
                                  ASTOpType OTy) {
  assert(TEx && "Invalid ASTExpressionNode argument!");

  ASTOperatorNode* OPX = nullptr;
  if (TEx->GetASTType() == ASTTypeIdentifier)
    OPX = new ASTOperatorNode(TEx->GetIdentifier(), OTy);
  else
    OPX = new ASTOperatorNode(TEx, OTy);
  assert(OPX && "Could not create a valid ASTOperatorNode!");

  OPX->Mangle();
  return OPX;
}

ASTOperandNode*
ASTBuilder::CreateASTOperandNode(const ASTIdentifierNode* TId) {
  assert(TId && "Invalid ASTIdentifierNode argument!");

  ASTOperandNode* OPX = new ASTOperandNode(TId);
  assert(OPX && "Could not create a valid ASTOperandNode!");

  OPX->Mangle();
  return OPX;
}

ASTOperandNode*
ASTBuilder::CreateASTOperandNode(const ASTExpressionNode* TEx) {
  assert(TEx && "Invalid ASTExpressionNode argument!");

  ASTOperandNode* OPX = nullptr;
  if (TEx->GetASTType() == ASTTypeIdentifier)
    OPX = new ASTOperandNode(TEx->GetIdentifier());
  else
    OPX = new ASTOperandNode(TEx);
  assert(OPX && "Could not create a valid ASTOperandNode!");

  OPX->Mangle();
  return OPX;
}

static ASTBinaryOpNode*
FinalizeASTBinaryOpNode(const ASTIdentifierNode* Id,
                        ASTBinaryOpNode* BOp) {
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), Id->GetSymbolType());
  assert(STE && "Invalid SymbolTable Entry for ASTBinaryOpNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(BOp, ASTTypeBinaryOp), ASTTypeBinaryOp);
  assert(STE->HasValue() && "BinaryOpNode SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);

  if (!BOp->IsMangled())
    BOp->Mangle();

  return BOp;
}

ASTBinaryOpNode*
ASTBuilder::CreateASTBinaryOpNode(const std::string& Identifier,
                                  const ASTExpressionNode* L,
                                  const ASTExpressionNode* R,
                                  ASTOpType OT,
                                  bool Parens) {
  assert(!Identifier.empty() && "ASTExpression Identifier cannot be empty!");
  assert(L && "Invalid Left ASTExpression Operand!");
  assert(R && "Invalid Right ASTExpression Operand!");

  const ASTIdentifierNode* IDN =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(Identifier,
                                                         ASTBinaryOpNode::BinaryOpBits,
                                                         ASTTypeBinaryOp);
  assert(IDN && "Failed to create an ASTBinaryOpNode ASTIdentifierNode!");

  ASTBinaryOpNode* BOp = new ASTBinaryOpNode(IDN, L, R, OT);
  assert(BOp && "Invalid ASTBinaryOpNode argument!");

  if (Parens)
    BOp->AddParens();

  if (!ASTExpressionValidator::Instance().Validate(BOp)) {
    std::stringstream M;
    M << "Invalid binary expression " << PrintTypeEnum(L->GetASTType())
      << ' ' << PrintOpTypeEnum(OT) << ' ' << PrintTypeEnum(R->GetASTType())
      << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(IDN), M.str(), DiagLevel::Error);
    return nullptr;
  }

  return FinalizeASTBinaryOpNode(IDN, BOp);
}

ASTBinaryOpNode*
ASTBuilder::CreateASTBinaryOpNode(const std::string& Identifier,
                                  const ASTIdentifierNode* L,
                                  const ASTExpressionNode* R,
                                  ASTOpType OT,
                                  bool Parens) {
  assert(!Identifier.empty() && "ASTExpression Identifier cannot be empty!");
  assert(L && "Invalid Left ASTIdentifierNode Operand!");
  assert(R && "Invalid Right ASTExpression Operand!");

  const ASTIdentifierNode* IDN =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(Identifier,
                                                         ASTBinaryOpNode::BinaryOpBits,
                                                         ASTTypeBinaryOp);
  assert(IDN && "Failed to create an ASTBinaryOpNode ASTIdentifierNode!");

  ASTExpressionNode* LEN = new ASTExpressionNode(L, ASTTypeIdentifier);
  assert(LEN && "Could not create a Left Identifier ASTExpressionNode!");

  ASTBinaryOpNode* BOp = new ASTBinaryOpNode(IDN, LEN, R, OT);
  assert(BOp && "Invalid ASTBinaryOpNode argument!");

  if (Parens)
    BOp->AddParens();

  if (!ASTExpressionValidator::Instance().Validate(BOp)) {
    std::stringstream M;
    M << "Invalid binary expression " << PrintTypeEnum(L->GetSymbolType())
      << ' ' << PrintOpTypeEnum(OT) << ' ' << PrintTypeEnum(R->GetASTType())
      << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(IDN), M.str(), DiagLevel::Error);
    return nullptr;
  }

  return FinalizeASTBinaryOpNode(IDN, BOp);
}

ASTBinaryOpNode*
ASTBuilder::CreateASTBinaryOpNode(const std::string& Identifier,
                                  const ASTExpressionNode* L,
                                  const ASTIdentifierNode* R,
                                  ASTOpType OT,
                                  bool Parens) {
  assert(!Identifier.empty() && "ASTExpression Identifier cannot be empty!");
  assert(L && "Invalid Left ASTIdentifierNode Operand!");
  assert(R && "Invalid Right ASTExpression Operand!");

  const ASTIdentifierNode* IDN =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(Identifier,
                                       ASTBinaryOpNode::BinaryOpBits,
                                       ASTTypeBinaryOp);
  assert(IDN && "Failed to create an ASTBinaryOpNode ASTIdentifierNode!");

  ASTExpressionNode* REN = new ASTExpressionNode(R, ASTTypeIdentifier);
  assert(REN && "Could not create a Right Identifier ASTExpressionNode!");

  ASTBinaryOpNode* BOp = new ASTBinaryOpNode(IDN, L, REN, OT);
  assert(BOp && "Invalid ASTBinaryOpNode argument!");

  if (Parens)
    BOp->AddParens();

  if (!ASTExpressionValidator::Instance().Validate(BOp)) {
    std::stringstream M;
    M << "Invalid binary expression " << PrintTypeEnum(L->GetASTType())
      << ' ' << PrintOpTypeEnum(OT) << ' ' << PrintTypeEnum(R->GetSymbolType())
      << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(IDN), M.str(), DiagLevel::Error);
    return nullptr;
  }

  return FinalizeASTBinaryOpNode(IDN, BOp);
}

ASTBinaryOpNode*
ASTBuilder::CreateASTBinaryOpNode(const std::string& Identifier,
                                  const ASTIdentifierNode* L,
                                  const ASTIdentifierNode* R,
                                  ASTOpType OT,
                                  bool Parens) {
  assert(!Identifier.empty() && "ASTExpression Identifier cannot be empty!");
  assert(L && "Invalid Left ASTIdentifierNode Operand!");
  assert(R && "Invalid Right ASTIdentifierNode Operand!");

  const ASTIdentifierNode* IDN =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(Identifier,
                                                         ASTBinaryOpNode::BinaryOpBits,
                                                         ASTTypeBinaryOp);
  assert(IDN && "Failed to create an ASTBinaryOpNode ASTIdentifierNode!");

  ASTExpressionNode* LEN = nullptr;
  if (L->HasExpression())
    LEN = const_cast<ASTExpressionNode*>(L->GetExpression());
  else
    LEN = new ASTExpressionNode(L, ASTTypeIdentifier);
  assert(LEN && "Could not create a Left Identifier ASTExpressionNode!");

  ASTExpressionNode* REN = nullptr;
  if (R->HasExpression())
    REN = const_cast<ASTExpressionNode*>(R->GetExpression());
  else
    REN = new ASTExpressionNode(R, ASTTypeIdentifier);
  assert(REN && "Could not create a Right Identifier ASTExpressionNode!");

  ASTBinaryOpNode* BOp = new ASTBinaryOpNode(IDN, LEN, REN, OT);
  assert(BOp && "Invalid ASTBinaryOpNode argument!");

  if (Parens)
    BOp->AddParens();

  if (!ASTExpressionValidator::Instance().Validate(BOp)) {
    std::stringstream M;
    M << "Invalid binary expression " << PrintTypeEnum(L->GetSymbolType())
      << ' ' << PrintOpTypeEnum(OT) << ' ' << PrintTypeEnum(R->GetSymbolType())
      << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(IDN), M.str(), DiagLevel::Error);
    return nullptr;
  }

  return FinalizeASTBinaryOpNode(IDN, BOp);
}

ASTBinaryOpNode*
ASTBuilder::CreateASTBinaryOpNode(const std::string& Identifier,
                                  const std::string& L,
                                  const ASTExpressionNode* R,
                                  ASTOpType OT,
                                  bool Parens) {
  assert(!Identifier.empty() && "ASTExpression Identifier cannot be empty!");
  assert(!L.empty() && "LHS Identifier cannot be empty!");
  assert(R && "Invalid Right ASTExpressionNode Operand!");

  const ASTIdentifierNode* LDN =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(L, ASTBinaryOpNode::BinaryOpBits,
                                                         ASTTypeBinaryOp);
  assert(LDN && "Failed to create a Left ASTBinaryOpNode ASTIdentifierNode!");

  return ASTBuilder::Instance().CreateASTBinaryOpNode(Identifier, LDN, R,
                                                      OT, Parens);
}

ASTBinaryOpNode*
ASTBuilder::CreateASTBinaryOpNode(const std::string& Identifier,
                                  const std::string& L,
                                  const std::string& R,
                                  ASTOpType OT,
                                  bool Parens) {
  assert(!Identifier.empty() && "ASTExpression Identifier cannot be empty!");
  assert(!L.empty() && "LHS Identifier cannot be empty!");
  assert(!R.empty() && "RHS Identifier cannot be empty!");

  const ASTIdentifierNode* LDN =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(L);
  assert(LDN && "Failed to create a Left ASTBinaryOpNode ASTIdentifierNode!");

  const ASTIdentifierNode* RDN =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(R);
  assert(RDN && "Failed to create a Right ASTBinaryOpNode ASTIdentifierNode!");

  return ASTBuilder::Instance().CreateASTBinaryOpNode(Identifier, LDN,
                                                      RDN, OT, Parens);
}

static ASTUnaryOpNode*
FinalizeASTUnaryOpNode(const ASTIdentifierNode* Id,
                       ASTUnaryOpNode* UOp) {
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Invalid SymbolTable Entry for ASTUnaryOpNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(UOp, ASTTypeUnaryOp), ASTTypeUnaryOp);
  assert(STE->HasValue() && "UnaryOpNode SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);

  UOp->Mangle();
  return UOp;
}

ASTUnaryOpNode*
ASTBuilder::CreateASTUnaryOpNode(const std::string& Identifier,
                                 const ASTExpressionNode* R,
                                 ASTOpType OT,
                                 bool HasParens,
                                 bool IsLValue) {
  assert(!Identifier.empty() && "ASTExpression Identifier cannot be empty!");
  assert(R && "Invalid Right ASTExpression Operand!");

  const ASTIdentifierNode* IDN =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(Identifier,
                                                         ASTUnaryOpNode::UnaryOpBits,
                                                         ASTTypeUnaryOp);
  assert(IDN && "Failed to create an ASTBinaryOpNode ASTIdentifierNode!");

  ASTUnaryOpNode* UOp = new ASTUnaryOpNode(IDN, R, OT, IsLValue);
  assert(UOp && "Failed to create an ASTUnaryOpNode!");

  if (HasParens)
    UOp->AddParens();

  if (!ASTExpressionValidator::Instance().Validate(UOp)) {
    std::stringstream M;
    M << "Invalid unary expression " << PrintOpTypeEnum(OT) << ' '
      << PrintTypeEnum(R->GetASTType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(IDN), M.str(), DiagLevel::Error);
    return nullptr;
  }

  return FinalizeASTUnaryOpNode(IDN, UOp);
}

ASTUnaryOpNode*
ASTBuilder::CreateASTUnaryOpNode(const std::string& Identifier,
                                 const ASTIdentifierNode* Id,
                                 ASTOpType OT,
                                 bool HasParens,
                                 bool IsLValue) {
  assert(Id && "Invalid ASTIdentifierNode Argument!");

  ASTExpressionNode* REN = new ASTExpressionNode(Id, ASTTypeIdentifier);
  assert(REN && "Could not create a Right Identifier ASTExpressionNode!");

  return CreateASTUnaryOpNode(Identifier, REN, OT, HasParens, IsLValue);
}

ASTMPIntegerNode*
ASTBuilder::CreateASTMPIntegerNode(const ASTIdentifierNode* Id,
                                   unsigned Bits, bool Unsigned) {
  assert(Id && "Invalid ASTIdentifierRefNode!");

  Id->SetBits(Bits);
  ASTSymbolTableEntry* STE = nullptr;
  if (!(STE = ASTSymbolTable::Instance().Lookup(Id->GetName(), Bits,
                                                Id->GetSymbolType()))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeMPInteger &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeMPInteger) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTMPIntegerNode* MPI =
    new ASTMPIntegerNode(Id, Unsigned ?
                             ASTSignbit::Unsigned : ASTSignbit::Signed,
                         Bits);
  assert(MPI && "Could not create an ASTMPIntegerNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPI, ASTTypeMPInteger), ASTTypeMPInteger);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTMPIntegerNode has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPI->Mangle();
  MPI->MangleLiteral();
  return MPI;
}

ASTMPIntegerNode*
ASTBuilder::CreateASTMPIntegerNode(const ASTIdentifierNode* Id,
                                   unsigned Bits, const char* String,
                                   bool Unsigned) {
  assert(Id && "Invalid ASTIdentifierRefNode!");

  Id->SetBits(Bits);
  ASTSymbolTableEntry* STE = nullptr;
  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Bits,
                                                Id->GetSymbolType()))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeMPInteger &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeMPInteger) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  ASTMPIntegerNode* MPI =
    new ASTMPIntegerNode(Id, Unsigned ?
                              ASTSignbit::Unsigned : ASTSignbit::Signed,
                         Bits, String);
  assert(MPI && "Could not create an ASTMPIntegerNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPI, ASTTypeMPInteger), ASTTypeMPInteger);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTMPIntegerNode has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPI->Mangle();
  MPI->MangleLiteral();
  return MPI;
}

ASTMPIntegerNode*
ASTBuilder::CreateASTMPIntegerNode(const ASTIdentifierNode* Id,
                                   unsigned Bits,
                                   const ASTExpressionNode* Expr,
                                   bool Unsigned) {
  assert(Id && "Invalid ASTIdentifierNode!");

  Id->SetBits(Bits);
  ASTSymbolTableEntry* STE = nullptr;
  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Bits,
                                                Id->GetSymbolType()))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeMPInteger &&
      STE->GetValueType() != ASTTypeInt &&
      STE->GetValueType() != ASTTypeBool &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeMPInteger) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  ASTMPIntegerNode* MPI = nullptr;

  if (STE->GetValueType() == ASTTypeInt) {
    if (const ASTIntNode* I = dynamic_cast<const ASTIntNode*>(Expr))
      MPI = new ASTMPIntegerNode(Id, I, Bits);
    else
      MPI = new ASTMPIntegerNode(Id, Unsigned ? ASTSignbit::Unsigned : ASTSignbit::Signed,
                                 Bits, Expr);
  } else if (STE->GetValueType() == ASTTypeMPInteger) {
    if (const ASTMPIntegerNode* MI = dynamic_cast<const ASTMPIntegerNode*>(Expr))
      MPI = new ASTMPIntegerNode(Id, Bits, MI->GetMPValue(),
                                 Unsigned ? ASTSignbit::Unsigned : ASTSignbit::Signed);
    else
      MPI = new ASTMPIntegerNode(Id, Unsigned ? ASTSignbit::Unsigned : ASTSignbit::Signed,
                                 Bits, Expr);
  } else if (STE->GetValueType() == ASTTypeBool) {
    if (const ASTBoolNode* B = dynamic_cast<const ASTBoolNode*>(Expr))
      MPI = new ASTMPIntegerNode(Id,
                                 Unsigned ? ASTSignbit::Unsigned : ASTSignbit::Signed,
                                 Bits, B->GetValue() ? "1" : "0");
    else
      MPI = new ASTMPIntegerNode(Id, Unsigned ? ASTSignbit::Unsigned : ASTSignbit::Signed,
                                 Bits, Expr);
  } else {
    MPI = new ASTMPIntegerNode(Id, Unsigned ? ASTSignbit::Unsigned : ASTSignbit::Signed,
                                   Bits, Expr);
  }

  assert(MPI && "Could not create an ASTMPIntegerNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPI, ASTTypeMPInteger), ASTTypeMPInteger);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTMPIntegerNode has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPI->Mangle();
  MPI->MangleLiteral();
  return MPI;
}

ASTMPIntegerNode*
ASTBuilder::CreateASTMPIntegerNode(const ASTIdentifierNode* Id,
                                   unsigned Bits,
                                   const ASTIntNode* I) {
  assert(Id && "Invalid ASTIdentifierNode!");

  Id->SetBits(Bits);
  ASTSymbolTableEntry* STE = nullptr;
  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Bits,
                                                Id->GetSymbolType()))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeMPInteger &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeMPInteger) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTMPIntegerNode* MPI =
    new ASTMPIntegerNode(Id, I, static_cast<unsigned>(Bits));
  assert(MPI && "Could not create an ASTMPIntegerNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPI, ASTTypeMPInteger), ASTTypeMPInteger);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTMPIntegerNode has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPI->Mangle();
  MPI->MangleLiteral();
  return MPI;
}

ASTMPIntegerNode*
ASTBuilder::CreateASTMPIntegerNodeFromExpression(const ASTIdentifierNode* Id,
                                                 unsigned Bits,
                                                 ASTExpressionNode* EN,
                                                 bool Unsigned) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(EN && "Invalid ASTExpressionNode argument!");

  Id->SetBits(Bits);
  ASTMPIntegerNode* DMPI = nullptr;
  ASTSymbolTableEntry* STE = nullptr;

  switch (EN->GetASTType()) {
    case ASTTypeInt: {
      ASTIntNode* Int = dynamic_cast<ASTIntNode*>(EN);
      assert(Int && "Could not dynamic_cast to an ASTIntNode!");
      DMPI = ASTBuilder::Instance().CreateASTMPIntegerNode(Id, Bits, Int);
      assert(DMPI && "Could not create a valid ASTMPIntegerNode!");
    }
      break;
    case ASTTypeBool: {
      ASTBoolNode* BN = dynamic_cast<ASTBoolNode*>(EN);
      assert(BN && "Could not dynamic_cast to an ASTBoolNode!");
      const char* BS = BN->GetValue() ? "1" : "0";
      DMPI = ASTBuilder::Instance().CreateASTMPIntegerNode(Id, Bits, BS,
                                                           true);
      assert(DMPI && "Could not create a valid ASTMPIntegerNode!");
    }
      break;
    case ASTTypeFloat: {
      ASTFloatNode* FN = dynamic_cast<ASTFloatNode*>(EN);
      assert(FN && "Could not dynamic_cast to an ASTFloatNode!");
      std::stringstream F;
      F << static_cast<int32_t>(FN->GetValue());
      std::string FS = F.str();
      DMPI = ASTBuilder::Instance().CreateASTMPIntegerNode(Id, Bits,
                                                           FS.c_str(),
                                                           false);
      assert(DMPI && "Could not create a valid ASTMPIntegerNode!");
    }
      break;
    case ASTTypeDouble: {
      ASTDoubleNode* DN = dynamic_cast<ASTDoubleNode*>(EN);
      assert(DN && "Could not dynamic_cast to an ASTDoubleNode!");
      std::stringstream D;
      D << static_cast<int64_t>(DN->GetValue());
      std::string DS = D.str();
      DMPI = ASTBuilder::Instance().CreateASTMPIntegerNode(Id, Bits,
                                                           DS.c_str(),
                                                           false);
      assert(DMPI && "Could not create a valid ASTMPIntegerNode!");
    }
      break;
    case ASTTypeMPInteger: {
      ASTMPIntegerNode* MPI = dynamic_cast<ASTMPIntegerNode*>(EN);
      assert(MPI && "Could not dynamic_cast to an ASTMPIntegerNode!");
      const std::string& MPS = MPI->GetValue();
      DMPI = ASTBuilder::Instance().CreateASTMPIntegerNode(Id, Bits,
                                                           MPS.c_str(),
                                                           !MPI->IsSigned());
      assert(DMPI && "Could not create a valid ASTMPIntegerNode!");
    }
      break;
    case ASTTypeMPDecimal: {
      ASTMPDecimalNode* MPD = dynamic_cast<ASTMPDecimalNode*>(EN);
      assert(MPD && "Could not dynamic_cast to an ASTMPDecimalNode!");
      mpz_t MPZ;
      mpz_init2(MPZ, MPD->GetBits());
      mpfr_get_z(MPZ, MPD->GetMPValue(), MPFR_RNDN);
      const std::string& MPS = ASTMPIntegerNode::GetValue(MPZ, 10);
      DMPI = ASTBuilder::Instance().CreateASTMPIntegerNode(Id, Bits,
                                                           MPS.c_str(),
                                                           false);
      assert(DMPI && "Could not create a valid ASTMPIntegerNode!");
      const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
    }
      break;
    case ASTTypeBinaryOp: {
      ASTBinaryOpNode* BOP = dynamic_cast<ASTBinaryOpNode*>(EN);
      assert(BOP && "Could not dynamic_cast to an ASTBinaryOpNode!");
      DMPI = ASTBuilder::Instance().CreateASTMPIntegerNode(Id, Bits, BOP,
                                                           Unsigned);
      assert(DMPI && "Could not create a valid ASTMPIntegerNode!");
      const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
    }
      break;
    case ASTTypeUnaryOp: {
      ASTUnaryOpNode* UOP = dynamic_cast<ASTUnaryOpNode*>(EN);
      assert(UOP && "Could not dynamic_cast to an ASTUnaryOpNode!");
      DMPI = ASTBuilder::Instance().CreateASTMPIntegerNode(Id, Bits, UOP,
                                                           Unsigned);
      assert(DMPI && "Could not create a valid ASTMPIntegerNode!");
      const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
    }
      break;
    case ASTTypeBitset: {
      ASTCBitNode* CBN = dynamic_cast<ASTCBitNode*>(EN);
      assert(CBN && "Could not dynamic_cast to an ASTCBitNode!");
      const std::string& CBS = CBN->AsString();
      DMPI = ASTBuilder::Instance().CreateASTMPIntegerNode(Id, Bits, "0",
                                                           true);
      assert(DMPI && "Could not create a valid ASTMPIntegerNode!");
      DMPI->SetValue(CBS.c_str(), 2);
      const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
    }
      break;
    case ASTTypeExpression: {
      DMPI = ASTBuilder::Instance().CreateASTMPIntegerNode(Id, Bits, EN,
                                                           Unsigned);
      assert(DMPI && "Could not create a valid ASTMPIntegerNode!");
      const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
    }
      break;
    default: {
      std::stringstream M;
      M << "Impossible initialization for an ASTMPIntegerNode from an "
        << PrintTypeEnum(STE->GetValueType());
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Error);
      DMPI = nullptr;
    }
      break;
  }

  DMPI->Mangle();
  DMPI->MangleLiteral();
  return DMPI;
}

ASTMPIntegerNode*
ASTBuilder::CreateASTMPIntegerNode(const ASTIdentifierNode* Id,
                                   unsigned Bits,
                                   const mpz_t& MPZ,
                                   bool Unsigned) {
  assert(Id && "Invalid ASTIdentifierNode!");

  Id->SetBits(Bits);
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, Id->GetSymbolType());
  if (!STE) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeMPInteger &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeMPInteger) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  ASTMPIntegerNode* MPI = new ASTMPIntegerNode(Id, Bits, MPZ, Unsigned);
  assert(MPI && "Could not create a valid ASTMPIntegerNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPI, ASTTypeMPInteger), ASTTypeMPInteger);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTMPIntegerNode has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPI->Mangle();
  MPI->MangleLiteral();
  return MPI;
}

ASTMPDecimalNode*
ASTBuilder::CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                   unsigned Bits) {
  assert(Id && "Invalid ASTIdentifierNode!");

  Id->SetBits(Bits);
  ASTSymbolTableEntry* STE = nullptr;
  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Bits,
                                                Id->GetSymbolType()))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeMPDecimal &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeMPDecimal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  ASTMPDecimalNode* MPD = new ASTMPDecimalNode(Id, Bits);
  assert(MPD && "Could not create an ASTMPDecimalNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPD, ASTTypeMPDecimal), ASTTypeMPDecimal);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTMPDecimalNode has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPD->Mangle();
  MPD->MangleLiteral();
  return MPD;
}

ASTMPDecimalNode*
ASTBuilder::CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                   unsigned Bits,
                                   int Mantissa) {
  assert(Id && "Invalid ASTIdentifierRefNode!");

  Id->SetBits(Bits);
  ASTSymbolTableEntry* STE = nullptr;
  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Bits,
                                                Id->GetSymbolType()))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeMPDecimal &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeMPDecimal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  ASTMPDecimalNode* MPD =
    new ASTMPDecimalNode(Id, Bits, static_cast<unsigned>(Mantissa));
  assert(MPD && "Could not create an ASTMPDecimalNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPD, ASTTypeMPDecimal), ASTTypeMPDecimal);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTMPDecimalNode has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPD->Mangle();
  MPD->MangleLiteral();
  return MPD;
}

ASTMPDecimalNode*
ASTBuilder::CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                   unsigned Bits,
                                   const ASTExpressionNode* Expr) {
  return CreateASTMPDecimalNode(Id, Bits, Expr,
                                ASTMPDecimalNode::GetDefaultPrecision());
}

ASTMPDecimalNode*
ASTBuilder::CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                   unsigned Bits,
                                   const ASTExpressionNode* Expr,
                                   int Mantissa) {
  assert(Id && "Invalid ASTIdentifierRefNode Argument!");
  assert(Expr && "Invalid ASTExpressionNode Argument!");

  Id->SetBits(Bits);
  ASTSymbolTableEntry* STE = nullptr;
  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Bits,
                                                Id->GetSymbolType()))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeMPDecimal &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeMPDecimal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  ASTMPDecimalNode* MPD = nullptr;

  switch (Expr->GetASTType()) {
  case ASTTypeInt: {
    const ASTIntNode* EI = dynamic_cast<const ASTIntNode*>(Expr);
    assert(EI && "Could not dynamic_cast to an ASTIntNode!");

    std::string IR;
    if (EI->IsSigned())
      IR = std::to_string(EI->GetSignedValue());
    else
      IR = std::to_string(EI->GetUnsignedValue());

    IR += ".";
    IR.append(static_cast<size_t>(Mantissa), '0');
    MPD = new ASTMPDecimalNode(Id, Bits,
                               static_cast<unsigned>(Mantissa), IR.c_str());
    assert(MPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeFloat: {
    const ASTFloatNode* EF = dynamic_cast<const ASTFloatNode*>(Expr);
    assert(EF && "Could not dynamic_cast to an ASTFloatNode!");

    // Bug in Gold Linker.
    unsigned FBits = ASTFloatNode::FloatBits;
    Bits = static_cast<unsigned>(std::max(Bits, FBits));
    MPD = new ASTMPDecimalNode(Id, Bits,
                               static_cast<double>(EF->GetValue()));
    assert(MPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeDouble: {
    const ASTDoubleNode* ED = dynamic_cast<const ASTDoubleNode*>(Expr);
    assert(ED && "Could not dynamic_cast to an ASTDoubleNode!");

    // Bug in Gold Linker.
    unsigned DBits = ASTDoubleNode::DoubleBits;
    Bits = static_cast<unsigned>(std::max(Bits, DBits));
    MPD = new ASTMPDecimalNode(Id, Bits, ED->GetValue());
    assert(MPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeMPInteger: {
    const ASTMPIntegerNode* MPI = dynamic_cast<const ASTMPIntegerNode*>(Expr);
    assert(MPI && "Could not dynamic_cast to an ASTMPIntegerNode!");

    if (MPI->IsExpression()) {
      const ASTExpressionNode* MPE = MPI->GetExpression();
      assert(MPE && "Could not obtain a valid MPInteger ASTExpressionNode!");

      MPD = new ASTMPDecimalNode(Id, Bits, MPE);
      assert(MPD && "Could not create a valid ASTMPDecimalNode!");
    } else {
      std::string IR = MPI->GetValue();
      IR += ".";
      IR.append(static_cast<size_t>(Mantissa), '0');
      MPD = new ASTMPDecimalNode(Id, Bits, IR.c_str());
      assert(MPD && "Could not create a valid ASTMPDecimalNode!");
    }
  }
    break;
  case ASTTypeMPDecimal: {
    const ASTMPDecimalNode* CMPD = dynamic_cast<const ASTMPDecimalNode*>(Expr);
    assert(CMPD && "Could not dynamic_cast to an ASTMPDecimalNode!");

    if (CMPD->IsExpression()) {
      const ASTExpressionNode* MPE = CMPD->GetExpression();
      assert(MPE && "Could not obtain a valid MPDecimal ASTExpressionNode!");

      MPD = new ASTMPDecimalNode(Id, Bits, MPE);
      assert(MPD && "Could not create a valid ASTMPDecimalNode!");
    } else {
      std::string IR = CMPD->GetValue();
      MPD = new ASTMPDecimalNode(Id, Bits, IR.c_str());
      assert(MPD && "Could not create a valid ASTMPDecimalNode!");
    }
  }
    break;
  case ASTTypeIdentifier: {
    const ASTIdentifierNode* IDN =
      dynamic_cast<const ASTIdentifierNode*>(Expr->GetIdentifier());
    assert(IDN && "Could not dynamic_cast to an ASTIdentifierNode!");

    ASTScopeController::Instance().CheckUndefined(IDN);
    ASTScopeController::Instance().CheckOutOfScope(IDN);

    const std::string& IS = IDN->GetName();

    if (IS == "pi") {
      MPD = ASTMPDecimalNode::Pi(Bits, Mantissa);
      assert(MPD && "Could not create a valid Pi ASTMPDecimalNode!");
    } else if (IS == "tau") {
      MPD = ASTMPDecimalNode::Tau(Bits, Mantissa);
      assert(MPD && "Could not create a valid Tau ASTMPDecimalNode!");
    } else if (IS == "euler" || IS == "euler_gamma") {
      MPD = ASTMPDecimalNode::Euler(Bits, Mantissa);
      assert(MPD && "Could not create a valid EulerGamma ASTMPDecimalNode!");
    } else {
      // FIXME: Implicit Casts.
      if (ASTTypeCastController::Instance().CanCast(IDN, ASTTypeMPDecimal)) {
        ASTCastExpressionNode* CX =
          new ASTCastExpressionNode(IDN, ASTTypeMPDecimal, Bits);
        assert(CX && "Could not create a valid ASTCastExpressionNode!");
        if (CX->IsBadCast()) {
          std::stringstream M;
          M << "Impossible cast from " << PrintTypeEnum(IDN->GetSymbolType())
            << " to " << PrintTypeEnum(ASTTypeMPDecimal) << " requested.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
          MPD = nullptr;
        } else {
          MPD = new ASTMPDecimalNode(Id, Bits, Bits, CX);
          assert(MPD && "Could not create a valid ASTMPDecimalNode!");
        }
      } else {
        std::stringstream M;
        M << "Impossible cast requested: " << PrintTypeEnum(IDN->GetSymbolType())
          << '.';
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
        MPD = nullptr;
      }
    }
  }
    break;
  case ASTTypeBinaryOp: {
    const ASTBinaryOpNode* BOP = dynamic_cast<const ASTBinaryOpNode*>(Expr);
    assert(BOP && "Could not dynamic_cast to an ASTBinaryOpNode!");

    MPD = new ASTMPDecimalNode(Id, Bits, BOP);
    assert(MPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeUnaryOp: {
    const ASTUnaryOpNode* UOP = dynamic_cast<const ASTUnaryOpNode*>(Expr);
    assert(UOP && "Could not dynamic_cast to an ASTUnaryOpNode!");

    MPD = new ASTMPDecimalNode(Id, Bits, UOP);
    assert(MPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeCast: {
    const ASTCastExpressionNode* CX =
      dynamic_cast<const ASTCastExpressionNode*>(Expr);
    assert(CX && "Could not dynamic_cast to an ASTCastExpressionNode!");

    MPD = new ASTMPDecimalNode(Id, Bits, CX);
    assert(MPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeImplicitConversion: {
    const ASTImplicitConversionNode* ICX =
      dynamic_cast<const ASTImplicitConversionNode*>(Expr);
    assert(ICX && "Could not dynamic_cast to an ASTImplicitConversionNode!");
    MPD = new ASTMPDecimalNode(Id, Bits, ICX);
    assert(MPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypePopcountExpr: {
    const ASTPopcountNode* PN = dynamic_cast<const ASTPopcountNode*>(Expr);
    assert(PN && "Could not dynamic_cast to an ASTPopcountNode!");
    MPD = new ASTMPDecimalNode(Id, Bits, PN);
    assert(MPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeRotateExpr: {
    const ASTRotateNode* RN = dynamic_cast<const ASTRotateNode*>(Expr);
    assert(RN && "Could not dynamic_cast to an ASTRotateNode!");
    MPD = new ASTMPDecimalNode(Id, Bits, RN);
    assert(MPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  default:
    std::stringstream M;
    M << "Impossible ASTMPDecimalNode initialization from Expression.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    MPD = nullptr;
    break;
  }

  if (MPD) {
    Id->SetBits(Bits);
    STE->SetValue(new ASTValue<>(MPD, ASTTypeMPDecimal), ASTTypeMPDecimal);
  } else {
    STE->ResetValue();
  }

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPD->Mangle();
  MPD->MangleLiteral();
  return MPD;
}

ASTMPDecimalNode*
ASTBuilder::CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                   unsigned Bits,
                                   const char* String) {
  assert(Id && "Invalid ASTIdentifierRefNode!");
  assert(String && "Invalid Value String for ASTMPDecimalNode!");

  ASTSymbolTableEntry* STE = nullptr;

  Id->SetBits(Bits);
  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Bits,
                                                Id->GetSymbolType()))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeMPDecimal &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeMPDecimal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  ASTMPDecimalNode* MPD = new ASTMPDecimalNode(Id, Bits);
  assert(MPD && "Could not create an ASTMPDecimalNode!");

  MPD->SetValue(String);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPD, ASTTypeMPDecimal), ASTTypeMPDecimal);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTMPDecimalNode has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPD->Mangle();
  MPD->MangleLiteral();
  return MPD;
}

ASTMPDecimalNode*
ASTBuilder::CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                   unsigned Bits,
                                   const mpfr_t& MPFR) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  Id->SetBits(Bits);
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Bits,
                                                Id->GetSymbolType()))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeMPDecimal &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeMPDecimal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  ASTMPDecimalNode* MPD = new ASTMPDecimalNode(Id, Bits, MPFR);
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPD, ASTTypeMPDecimal), ASTTypeMPDecimal);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTMPDecimalNode has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPD->Mangle();
  MPD->MangleLiteral();
  return MPD;
}

ASTMPDecimalNode*
ASTBuilder::CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                   unsigned Bits, double DV) {
  assert(Id && "Invalid ASTIdentifierNode!");

  Id->SetBits(Bits);
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Bits,
                                                Id->GetSymbolType()))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeMPDecimal &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeMPDecimal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  ASTMPDecimalNode* MPD = new ASTMPDecimalNode(Id, Bits, DV);
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPD, ASTTypeMPDecimal), ASTTypeMPDecimal);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTMPDecimalNode has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPD->Mangle();
  MPD->MangleLiteral();
  return MPD;
}

ASTMPDecimalNode*
ASTBuilder::CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                   unsigned Bits, long double LDV) {
  assert(Id && "Invalid ASTIdentifierNode!");

  Id->SetBits(Bits);
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Bits,
                                                Id->GetSymbolType()))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeMPDecimal &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeMPDecimal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  ASTMPDecimalNode* MPD = new ASTMPDecimalNode(Id, Bits, LDV);
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPD, ASTTypeMPDecimal), ASTTypeMPDecimal);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTMPDecimalNode has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPD->Mangle();
  MPD->MangleLiteral();
  return MPD;
}

ASTMPDecimalNode*
ASTBuilder::CreateASTMPDecimalNode(const ASTIdentifierNode* Id,
                                   unsigned Bits, float FV) {
  assert(Id && "Invalid ASTIdentifierNode!");

  Id->SetBits(Bits);
  ASTSymbolTableEntry* STE = nullptr;
  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Bits,
                                                Id->GetSymbolType()))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeMPDecimal &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeMPDecimal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTMPDecimalNode* MPD = new ASTMPDecimalNode(Id, Bits, FV);
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPD, ASTTypeMPDecimal), ASTTypeMPDecimal);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTMPDecimalNode has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPD->Mangle();
  MPD->MangleLiteral();
  return MPD;
}

ASTMPDecimalNode*
ASTBuilder::CreateASTMPDecimalNodeFromExpression(const ASTIdentifierNode* Id,
                                                 unsigned Bits,
                                                 const ASTExpressionNode* EN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(EN && "Invalid ASTExpressionNode argument!");

  ASTSymbolTableEntry* STE = nullptr;
  ASTMPDecimalNode* DMPD = nullptr;

  Id->SetBits(Bits);
  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Bits,
                                                Id->GetSymbolType()))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  ASTType ETy = EN->GetASTType();

  switch (ETy) {
  case ASTTypeInt: {
    const ASTIntNode* I = dynamic_cast<const ASTIntNode*>(EN);
    assert(I && "Could not dynamic_cast to an ASTIntNode!");

    int32_t V = I->IsSigned() ? I->GetSignedValue() : 0;
    DMPD = ASTBuilder::Instance().CreateASTMPDecimalNode(Id, Bits, float(V));
    assert(DMPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeFloat: {
    const ASTFloatNode* F = dynamic_cast<const ASTFloatNode*>(EN);
    assert(F && "Could not dynamic_cast to an ASTFloatNode!");

    DMPD = ASTBuilder::Instance().CreateASTMPDecimalNode(Id, Bits,
                                                         F->GetValue());
    assert(DMPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeDouble: {
    const ASTDoubleNode* D = dynamic_cast<const ASTDoubleNode*>(EN);
    assert(D && "Could not dynamic_cast to an ASTDoubleNode!");

    DMPD = ASTBuilder::Instance().CreateASTMPDecimalNode(Id, Bits,
                                                         D->GetValue());
    assert(DMPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeMPInteger: {
    const ASTMPIntegerNode* MPI = dynamic_cast<const ASTMPIntegerNode*>(EN);
    assert(MPI && "Could not dynamic_cast to an ASTMPIntegerNode!");

    if ((Bits - Bits / 4) < MPI->GetBits()) {
      std::stringstream M;
      M << "Conversion from an ASTMPIntegerNode to an ASTMPDecimalNode "
        << "might result in truncation";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Warning);
    }

    std::string MPS = ASTMPIntegerNode::GetValue(MPI->GetMPValue(), 10);
    MPS += ".0";
    DMPD = ASTBuilder::Instance().CreateASTMPDecimalNode(Id, Bits,
                                                         MPS.c_str());
    assert(DMPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeMPDecimal: {
    const ASTMPDecimalNode* MPD = dynamic_cast<const ASTMPDecimalNode*>(EN);
    assert(MPD && "Could not dynamic_cast to an ASTMPDecimalNode!");

    if (Bits < static_cast<unsigned>(MPD->GetBits())) {
      std::stringstream M;
      M << "Assignment to an ASTMPDecimalNode might result in truncation "
        << "or loss of precision";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Warning);
    }

    DMPD = ASTBuilder::Instance().CreateASTMPDecimalNode(Id, Bits,
                                                         MPD->GetMPValue());
    assert(DMPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeBinaryOp: {
    const ASTBinaryOpNode* BOP = dynamic_cast<const ASTBinaryOpNode*>(EN);
    assert(BOP && "Could not dynamic_cast to an ASTBinaryOpNode!");
    DMPD = ASTBuilder::Instance().CreateASTMPDecimalNode(Id, Bits, BOP);
    assert(DMPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeUnaryOp: {
    const ASTUnaryOpNode* UOP = dynamic_cast<const ASTUnaryOpNode*>(EN);
    assert(UOP && "Could not dynamic_cast to an ASTUnaryOpNode!");
    DMPD = ASTBuilder::Instance().CreateASTMPDecimalNode(Id, Bits, UOP);
    assert(DMPD && "Could not create a valid ASTMPDecimalNode!");
  }
    break;
  case ASTTypeBitset: {
    const ASTCBitNode* CBN = dynamic_cast<const ASTCBitNode*>(EN);
    assert(CBN && "Could not dynamic_cast to an ASTCBitNode!");
    const std::string& CBS = CBN->AsString();
    DMPD = ASTBuilder::Instance().CreateASTMPDecimalNode(Id, Bits, "0");
    assert(DMPD && "Could not create a valid ASTMPDecimalNode!");
    DMPD->SetValue(CBS.c_str(), 2);
  }
    break;
  default:
    DMPD = ASTBuilder::Instance().CreateASTMPDecimalNode(Id, Bits, EN);
    assert(DMPD && "Could not create a valid ASTMPDecimalNode!");
    break;
  }

  if (!DMPD) {
    std::stringstream M;
    M << "Impossible initialization of an ASTMPDecimalNode from an "
      << PrintTypeEnum(EN->GetASTType()) << ".";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(EN), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(DMPD, ASTTypeMPDecimal), ASTTypeMPDecimal);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  DMPD->Mangle();
  DMPD->MangleLiteral();
  return DMPD;
}

ASTMPComplexNode*
ASTBuilder::CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                                   unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  Id->SetBits(NumBits);
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, NumBits, Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTMPComplexNode* MPC = new ASTMPComplexNode(Id, NumBits);
  assert(MPC && "Could not create a valid ASTMPComplexNode!");

  Id->SetBits(NumBits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPC, ASTTypeMPComplex), ASTTypeMPComplex);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPC->Mangle();
  MPC->MangleLiteral();
  return MPC;
}

ASTMPComplexNode*
ASTBuilder::CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                                   const ASTMPComplexNode* MPCC,
                                   unsigned Bits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(MPCC && "Invalid ASTMPComplexNode argument!");

  Id->SetBits(Bits);
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, MPCC->GetBits(),
                                      Id->GetSymbolType());
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                            Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  Id->SetBits(Bits);
  ASTMPComplexNode* MPC = new ASTMPComplexNode(Id, *MPCC, Bits);
  assert(MPC && "Could not create a valid ASTMPComplexNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPC, ASTTypeMPComplex), ASTTypeMPComplex);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPC->Mangle();
  MPC->MangleLiteral();
  return MPC;
}

ASTMPComplexNode*
ASTBuilder::CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                                   const ASTComplexExpressionNode* CE,
                                   unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(CE && "Invalid ASTComplexExpressionNode argument!");

  Id->SetBits(NumBits);

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, NumBits,
                                      Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTMPComplexNode* MPC = new ASTMPComplexNode(Id, CE, NumBits);
  assert(MPC && "Could not create a valid ASTMPComplexNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPC, ASTTypeMPComplex), ASTTypeMPComplex);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPC->Mangle();
  MPC->MangleLiteral();
  return MPC;
}

ASTMPComplexNode*
ASTBuilder::CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                                   const ASTMPDecimalNode* R,
                                   const ASTMPDecimalNode* I,
                                   ASTOpType OT,
                                   unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(R && "Invalid Real complex part argument!");
  assert(I && "Invalid Imaginary complex part argument!");

  Id->SetBits(NumBits);
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, NumBits, Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTMPComplexNode* MPC = new ASTMPComplexNode(Id, R, I, OT, NumBits);
  assert(MPC && "Could not create a valid ASTMPComplexNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPC, ASTTypeMPComplex), ASTTypeMPComplex);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPC->Mangle();
  MPC->MangleLiteral();
  return MPC;
}

ASTMPComplexNode*
ASTBuilder::CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                                   const ASTMPIntegerNode* R,
                                   const ASTMPIntegerNode* I,
                                   ASTOpType OT,
                                   unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(R && "Invalid Real complex part argument!");
  assert(I && "Invalid Imaginary complex part argument!");

  Id->SetBits(NumBits);
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, NumBits, Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTMPComplexNode* MPC = new ASTMPComplexNode(Id, R, I, OT, NumBits);
  assert(MPC && "Could not create a valid ASTMPComplexNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPC, ASTTypeMPComplex), ASTTypeMPComplex);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPC->Mangle();
  MPC->MangleLiteral();
  return MPC;
}

ASTMPComplexNode*
ASTBuilder::CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                                   const ASTMPDecimalNode* R,
                                   const ASTMPIntegerNode* I,
                                   ASTOpType OT,
                                   unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(R && "Invalid Real complex part argument!");
  assert(I && "Invalid Imaginary complex part argument!");

  Id->SetBits(NumBits);
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, NumBits, Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTMPComplexNode* MPC = new ASTMPComplexNode(Id, R, I, OT, NumBits);
  assert(MPC && "Could not create a valid ASTMPComplexNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPC, ASTTypeMPComplex), ASTTypeMPComplex);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPC->Mangle();
  MPC->MangleLiteral();
  return MPC;
}

ASTMPComplexNode*
ASTBuilder::CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                                   const ASTMPIntegerNode* R,
                                   const ASTMPDecimalNode* I,
                                   ASTOpType OT,
                                   unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(R && "Invalid Real complex part argument!");
  assert(I && "Invalid Imaginary complex part argument!");

  Id->SetBits(NumBits);
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, NumBits, Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTMPComplexNode* MPC = new ASTMPComplexNode(Id, R, I, OT, NumBits);
  assert(MPC && "Could not create a valid ASTMPComplexNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPC, ASTTypeMPComplex), ASTTypeMPComplex);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPC->Mangle();
  MPC->MangleLiteral();
  return MPC;
}

ASTMPComplexNode*
ASTBuilder::CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                                   const std::string& REP,
                                   unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  Id->SetBits(NumBits);
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, NumBits, Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTMPComplexNode* MPC = new ASTMPComplexNode(Id, REP, NumBits);
  assert(MPC && "Could not create a valid ASTMPComplexNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPC, ASTTypeMPComplex), ASTTypeMPComplex);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPC->Mangle();
  MPC->MangleLiteral();
  return MPC;
}


ASTMPComplexNode*
ASTBuilder::CreateASTMPComplexNode(const ASTIdentifierNode* Id,
                                   const ASTFunctionCallNode* FC,
                                   unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  Id->SetBits(NumBits);
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, NumBits, Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTMPComplexNode* MPC = new ASTMPComplexNode(Id, FC, NumBits);
  assert(MPC && "Could not create a valid ASTMPComplexNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MPC, ASTTypeMPComplex), ASTTypeMPComplex);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  MPC->Mangle();
  MPC->MangleLiteral();
  return MPC;
}

ASTComplexExpressionNode*
ASTBuilder::CreateASTComplexExpressionNode(const ASTIdentifierNode* Id,
                                           const ASTBinaryOpNode* BOp) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(BOp && "Invalid ASTBinaryOpNode argument!");

  if (Id->GetBits() == 0)
    Id->SetBits(ASTMPComplexNode::DefaultBits);

  Id->SetPolymorphicName("complexexpression");
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                      Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  if (!BOp->IsMangled())
    const_cast<ASTBinaryOpNode*>(BOp)->Mangle();

  ASTComplexExpressionNode* ACE = new ASTComplexExpressionNode(Id, BOp);
  assert(ACE && "Could not create a valid ASTComplexExpressionNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ACE, ASTTypeComplexExpression),
                ASTTypeComplexExpression);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  ACE->Mangle();
  return ACE;
}

ASTComplexExpressionNode*
ASTBuilder::CreateASTComplexExpressionNode(const ASTIdentifierNode* Id,
                                           const ASTUnaryOpNode* UOp) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(UOp && "Invalid ASTUnaryOpNode argument!");

  if (Id->GetBits() == 0)
    Id->SetBits(ASTMPComplexNode::DefaultBits);

  Id->SetPolymorphicName("complexexpression");
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                      Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  if (!UOp->IsMangled())
    const_cast<ASTUnaryOpNode*>(UOp)->Mangle();

  ASTComplexExpressionNode* ACE = new ASTComplexExpressionNode(Id, UOp);
  assert(ACE && "Could not create a valid ASTComplexExpressionNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ACE, ASTTypeComplexExpression),
                ASTTypeComplexExpression);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  ACE->Mangle();
  return ACE;
}

ASTBoolNode*
ASTBuilder::CreateASTBoolNode(bool Value) {
  return new ASTBoolNode(Value);
}

ASTBoolNode*
ASTBuilder::CreateASTBoolNode(const ASTIdentifierNode* Id,
                              bool Value) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTBoolNode::BoolBits,
                                      Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTBoolNode* BN = new ASTBoolNode(Id, Value);
  assert(BN && "Could not create a valid ASTBoolNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(BN, ASTTypeBool), ASTTypeBool);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return BN;
}

ASTBoolNode*
ASTBuilder::CreateASTBoolNode(const ASTIdentifierNode* Id,
                              const ASTBinaryOpNode* BOP) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(BOP && "Invalid ASTBinaryOpNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTBoolNode::BoolBits,
                                      Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTBoolNode* BN = new ASTBoolNode(Id, BOP);
  assert(BN && "Could not create a valid ASTBoolNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(BN, ASTTypeBool), ASTTypeBool);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return BN;
}

ASTBoolNode*
ASTBuilder::CreateASTBoolNode(const ASTIdentifierNode* Id,
                              const ASTUnaryOpNode* UOP) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(UOP && "Invalid ASTUnaryOpNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTBoolNode::BoolBits,
                                      Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTBoolNode* BN = new ASTBoolNode(Id, UOP);
  assert(BN && "Could not create a valid ASTBoolNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(BN, ASTTypeBool), ASTTypeBool);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return BN;
}

ASTBoolNode*
ASTBuilder::CreateASTBoolNode(const ASTIdentifierNode* Id,
                              const ASTExpressionNode* EN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(EN && "Invalid ASTExpressionNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTBoolNode::BoolBits,
                                      Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTBoolNode* BN = new ASTBoolNode(Id, EN);
  assert(BN && "Could not create a valid ASTBoolNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(BN, ASTTypeBool), ASTTypeBool);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return BN;
}

ASTIntNode*
ASTBuilder::CreateASTIntNode(int32_t Value, const ASTCVRQualifiers& CVR) {
  return new ASTIntNode(static_cast<int32_t>(Value), CVR);
}

ASTIntNode*
ASTBuilder::CreateASTIntNode(uint32_t Value, const ASTCVRQualifiers& CVR) {
  return new ASTIntNode(static_cast<uint32_t>(Value), CVR);
}

ASTIntNode*
ASTBuilder::CreateASTIntNode(const ASTIdentifierNode* Id,
                             int32_t Value, const ASTCVRQualifiers& CVR) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->IsIndexed()) {
    Id = ASTBuilder::Instance().FindUnindexedASTIdentifierNode(Id);
    assert(Id && "Invalid ASTIdentifierNode argument!");
  }

  ASTIntNode* IR = new ASTIntNode(Id, Value, CVR);
  assert(IR && "Could not create a valid ASTIntNode!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTIntNode::IntBits,
                                      Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(IR, ASTTypeInt), ASTTypeInt);
  assert(STE->HasValue() && "ASTIntNode SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  IR->Mangle();
  IR->MangleLiteral();
  return IR;
}

ASTIntNode*
ASTBuilder::CreateASTIntNode(const ASTIdentifierNode* Id,
                             uint32_t Value, const ASTCVRQualifiers& CVR) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->IsIndexed()) {
    Id = ASTBuilder::Instance().FindUnindexedASTIdentifierNode(Id);
    assert(Id && "Invalid ASTIdentifierNode argument!");
  }

  ASTIntNode* IR = new ASTIntNode(Id, Value, CVR);
  assert(IR && "Could not create a valid ASTIntNode!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTIntNode::IntBits,
                                      Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(IR, ASTTypeInt), ASTTypeInt);
  assert(STE->HasValue() && "ASTIntNode SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  IR->Mangle();
  IR->MangleLiteral();
  return IR;
}

ASTIntNode*
ASTBuilder::CreateASTIntNode(const ASTIdentifierNode* Id,
                             const ASTExpressionNode* E,
                             bool Unsigned,
                             const ASTCVRQualifiers& CVR) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(E && "Invalid ASTExpressionNode argument!");

  ASTIntNode* RI = nullptr;

  switch (E->GetASTType()) {
  case ASTTypeInt: {
    const ASTIntNode* EI = dynamic_cast<const ASTIntNode*>(E);
    assert(EI && "Could not dynamic_cast to an ASTIntNode!");

    if (Unsigned) {
      uint32_t RV = static_cast<uint32_t>(EI->IsSigned() ?
                                          EI->GetSignedValue() :
                                          EI->GetUnsignedValue());
      RI = new ASTIntNode(Id, RV, CVR);
    } else {
      int32_t RV = static_cast<int32_t>(EI->IsSigned() ?
                                        EI->GetSignedValue() :
                                        EI->GetUnsignedValue());
      RI = new ASTIntNode(Id, RV, CVR);
    }
  }
    break;
  case ASTTypeFloat: {
    const ASTFloatNode* EF = dynamic_cast<const ASTFloatNode*>(E);
    assert(EF && "Could not dynamic_cast to an ASTFloatNode!");
    if (Unsigned)
      RI = new ASTIntNode(Id, static_cast<uint32_t>(EF->GetValue()), CVR);
    else
      RI = new ASTIntNode(Id, static_cast<int32_t>(EF->GetValue()), CVR);
  }
    break;
  case ASTTypeDouble: {
    const ASTDoubleNode* ED = dynamic_cast<const ASTDoubleNode*>(E);
    assert(ED && "Could not dynamic_cast to an ASTDoubleNode!");
    if (Unsigned)
      RI = new ASTIntNode(Id, static_cast<uint32_t>(ED->GetValue()), CVR);
    else
      RI = new ASTIntNode(Id, static_cast<int32_t>(ED->GetValue()), CVR);
  }
    break;
  case ASTTypeMPInteger: {
    const ASTMPIntegerNode* MPI = dynamic_cast<const ASTMPIntegerNode*>(E);
    assert(MPI && "Could not dynamic_cast to an ASTMPIntegerNode!");
    if (MPI->IsExpression()) {
      RI = new ASTIntNode(Id, MPI->GetExpression(), ASTIntNode::IntBits,
                          Unsigned);
    } else {
      std::string MPV = MPI->GetValue();
      if (Unsigned) {
        uint32_t UV = static_cast<uint32_t>(std::stoi(MPV));
        RI = new ASTIntNode(Id, UV, CVR);
      } else {
        int32_t SV = static_cast<int32_t>(std::stoi(MPV));
        RI = new ASTIntNode(Id, SV, CVR);
      }
    }
  }
    break;
  case ASTTypeMPDecimal: {
    const ASTMPDecimalNode* MPD = dynamic_cast<const ASTMPDecimalNode*>(E);
    assert(MPD && "Could not dynamic_cast to an ASTMPDecimalNode!");
    if (MPD->IsExpression()) {
      RI = new ASTIntNode(Id, MPD->GetExpression(), ASTIntNode::IntBits,
                          Unsigned);
    } else {
      std::string MPV = MPD->GetValue();
      if (Unsigned) {
        uint32_t UV = static_cast<uint32_t>(std::stoi(MPV));
        RI = new ASTIntNode(Id, UV, CVR);
      } else {
        int32_t SV = static_cast<int32_t>(std::stoi(MPV));
        RI = new ASTIntNode(Id, SV, CVR);
      }
    }
  }
    break;
  case ASTTypeBinaryOp: {
    const ASTBinaryOpNode* BOP = dynamic_cast<const ASTBinaryOpNode*>(E);
    assert(BOP && "Could not dynamic_cast to an ASTBinaryOpNode!");
    RI = new ASTIntNode(Id, BOP, ASTIntNode::IntBits, Unsigned);
  }
    break;
  case ASTTypeUnaryOp: {
    const ASTUnaryOpNode* UOP = dynamic_cast<const ASTUnaryOpNode*>(E);
    assert(UOP && "Could not dynamic_cast to an ASTUnaryOpNode!");
    RI = new ASTIntNode(Id, UOP, ASTIntNode::IntBits, Unsigned);
  }
    break;
  case ASTTypeKernelCallExpression:
  case ASTTypeDefcalCallExpression:
  case ASTTypeFunctionCallExpression: {
    const ASTFunctionCallNode* FC = dynamic_cast<const ASTFunctionCallNode*>(E);
    assert(FC && "Could not dynamic_cast to an ASTFunctionCallNode!");
    if (Unsigned)
      RI = new ASTIntNode(Id, FC, ASTIntNode::IntBits, false);
    else
      RI = new ASTIntNode(Id, FC, ASTIntNode::IntBits, true);
  }
    break;
  case ASTTypeFunctionCall: {
    const ASTFunctionCallNode* FC = dynamic_cast<const ASTFunctionCallNode*>(E);
    assert(FC && "Could not dynamic_cast to an ASTFunctionCallNode!");
    if (Unsigned)
      RI = new ASTIntNode(Id, FC, ASTIntNode::IntBits, false);
    else
      RI = new ASTIntNode(Id, FC, ASTIntNode::IntBits, true);
  }
    break;
  case ASTTypeIdentifier: {
    const ASTIdentifierNode* EId = nullptr;
    if (E->IsIdentifier())
      EId = E->GetIdentifier();
    else
      EId = dynamic_cast<const ASTIdentifierNode*>(E);
    assert(EId && "Could not dynamic_cast to an ASTIdentifierNode!");

    ASTScopeController::Instance().CheckUndefined(EId);
    ASTScopeController::Instance().CheckOutOfScope(EId);

    if (EId->IsIndexed()) {
      EId = ASTBuilder::Instance().FindUnindexedASTIdentifierNode(EId);
      assert(EId &&
             "Could not retrieve the corresponding Unindexed "
             "ASTIdentifierNode!");
    }

    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(EId, EId->GetBits(),
                                        EId->GetSymbolType());
    assert(STE && "Invalid SymbolTable Entry for ASTIdentifierNode!");

    switch (STE->GetValueType()) {
    case ASTTypeInt: {
      ASTIntNode* Int = STE->GetValue()->GetValue<ASTIntNode*>();
      assert(Int &&
             "Could not retrieve an ASTIntNode from the SymbolTable Entry!");
      if (Int->IsSigned())
        RI = new ASTIntNode(Id, Int->GetSignedValue());
      else
        RI = new ASTIntNode(Id, Int->GetUnsignedValue());
    }
      break;
    case ASTTypeMPInteger: {
      ASTMPIntegerNode* MPI = STE->GetValue()->GetValue<ASTMPIntegerNode*>();
      assert(MPI &&
             "Could not retrieve an ASTMPIntegerNode from the SymbolTable "
             "Entry!");
      std::string MPS = MPI->GetValue();
      if (MPI->IsSigned())
        RI = new ASTIntNode(Id, static_cast<int32_t>(std::stoi(MPS)));
      else
        RI = new ASTIntNode(Id, static_cast<uint32_t>(std::stoi(MPS)));
    }
      break;
    case ASTTypeFunctionCallExpression: {
      ASTFunctionCallNode* FC = STE->GetValue()->GetValue<ASTFunctionCallNode*>();
      assert(FC && "Could not retrieve a valid ASTFunctionCallNode from the "
                    "SymbolTable Entry!");
      if (Unsigned)
        RI = new ASTIntNode(Id, FC, ASTIntNode::IntBits, false);
      else
        RI = new ASTIntNode(Id, FC, ASTIntNode::IntBits, true);
    }
      break;
    default: {
      std::stringstream M;
      M << "Impossible ASTIntNode initialization from "
        << PrintTypeEnum(STE->GetValueType()) << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      RI = nullptr;
    }
      break;
    }
  }
    break;
  case ASTTypeCast: {
    const ASTCastExpressionNode* CX = dynamic_cast<const ASTCastExpressionNode*>(E);
    assert(CX && "Could not dynamic_cast to an ASTCastExpressionNode!");
    RI = new ASTIntNode(Id, CX, ASTIntNode::IntBits, Unsigned ? true : false);
  }
    break;
  case ASTTypePopcountExpr: {
    const ASTPopcountNode* PN = dynamic_cast<const ASTPopcountNode*>(E);
    assert(PN && "Could not dynamic_cast to an ASTPopcountNode!");
    RI = new ASTIntNode(Id, PN, ASTIntNode::IntBits, Unsigned ? true : false);
  }
    break;
  case ASTTypeImplicitConversion: {
    const ASTImplicitConversionNode* ICX =
      dynamic_cast<const ASTImplicitConversionNode*>(E);
    assert(ICX && "Could not dynamic_cast to an ASTImplicitConversionNode!");
    RI = new ASTIntNode(Id, ICX, ASTIntNode::IntBits, Unsigned ? true : false);
  }
    break;
  case ASTTypeRotateExpr: {
    const ASTRotateNode* RN = dynamic_cast<const ASTRotateNode*>(E);
    assert(RN && "Could not dynamic_cast to an ASTRotateNode!");
    RI = new ASTIntNode(Id, RN, ASTIntNode::IntBits, Unsigned ? true : false);
  }
    break;
  default: {
    std::stringstream M;
    M << "Impossible ASTIntNode initialization from Expression.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    RI = nullptr;
  }
    break;
  }

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTIntNode::IntBits,
                                      Id->GetSymbolType());
  assert(STE && "Invalid SymbolTable Entry for ASTIdentifierNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(RI, ASTTypeInt), ASTTypeInt);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  RI->Mangle();
  RI->MangleLiteral();
  return RI;
}

ASTFloatNode*
ASTBuilder::CreateASTFloatNode(float Value, const ASTCVRQualifiers& CVR) {
  return new ASTFloatNode(Value, CVR);
}

ASTFloatNode*
ASTBuilder::CreateASTFloatNode(const ASTIdentifierNode* Id,
                               float Value,
                               const ASTCVRQualifiers& CVR) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTFloatNode::FloatBits,
                                      ASTTypeFloat);
  if (!STE) {
    STE = new ASTSymbolTableEntry(Id, ASTTypeFloat);
    assert(STE && "Could not create a valid ASTSymbolTable Entry!");
    if (!ASTSymbolTable::Instance().Insert(const_cast<ASTIdentifierNode*>(Id),
                                           STE)) {
      std::stringstream M;
      M << "Could not insert a valid SymbolTableEntry into the "
        << "SymbolTable!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
      return nullptr;
    }
  }

  ASTFloatNode* FN = new ASTFloatNode(Id, Value, CVR);
  assert(FN && "Could not create a valid ASTFloatNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(FN, ASTTypeFloat), ASTTypeFloat);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  FN->Mangle();
  FN->MangleLiteral();
  return FN;
}

ASTFloatNode*
ASTBuilder::CreateASTFloatNode(const ASTIdentifierNode* Id,
                               const ASTExpressionNode* E,
                               const ASTCVRQualifiers& CVR) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(E && "Invalid ASTExpressionNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTFloatNode::FloatBits,
                                      ASTTypeFloat);
  if (!STE) {
    STE = new ASTSymbolTableEntry(Id, ASTTypeFloat);
    assert(STE && "Could not create a valid ASTSymbolTable Entry!");
    if (!ASTSymbolTable::Instance().Insert(const_cast<ASTIdentifierNode*>(Id),
                                           STE)) {
      std::stringstream M;
      M << "Could not insert a valid SymbolTableEntry into the "
        << "SymbolTable!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
      return nullptr;
    }
  }

  ASTFloatNode* FN = new ASTFloatNode(Id, E, CVR);
  assert(FN && "Could not create a valid ASTFloatNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(FN, ASTTypeFloat), ASTTypeFloat);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  FN->Mangle();
  FN->MangleLiteral();
  return FN;
}

ASTDoubleNode*
ASTBuilder::CreateASTDoubleNode(double Value, const ASTCVRQualifiers& CVR) {
  return new ASTDoubleNode(Value, CVR);
}

ASTDoubleNode*
ASTBuilder::CreateASTDoubleNode(const ASTIdentifierNode* Id,
                                double Value,
                                const ASTCVRQualifiers& CVR) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTDoubleNode::DoubleBits,
                                      Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTDoubleNode* DN = new ASTDoubleNode(Value, CVR);
  assert(DN && "Could not create a valid ASTDoubleNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDouble), ASTTypeDouble);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  DN->Mangle();
  DN->MangleLiteral();
  return DN;
}

ASTDoubleNode*
ASTBuilder::CreateASTDoubleNode(const ASTIdentifierNode* Id,
                                const ASTExpressionNode* E,
                                const ASTCVRQualifiers& CVR) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(E && "Invalid ASTExpressionNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTDoubleNode::DoubleBits,
                                      Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTDoubleNode* DN = new ASTDoubleNode(Id, E, CVR);
  assert(DN && "Could not create a valid ASTDoubleNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDouble), ASTTypeDouble);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  DN->Mangle();
  DN->MangleLiteral();
  return DN;
}

ASTIdentifierNode*
ASTBuilder::CreateASTIdentifierNode(const std::string& Id) {
  if (Id.empty())
    return nullptr;

  ASTIdentifierNode* IDN = nullptr;
  if ((IDN = ASTBuilder::Instance().FindASTIdentifierNode(Id)) != nullptr)
    return IDN;

  if (ASTAngleContextControl::Instance().InOpenContext() &&
      ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id)) {
    IDN = new ASTIdentifierNode(Id, ASTAngleNode::AngleBits);
    assert(IDN && "Could not create a valid ASTIdentifierNode!");

    ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(IDN, ASTTypeAngle);
    assert(STE && "Could not create a valid SymbolTable Entry!");

    if (!ASTSymbolTable::Instance().Insert(IDN, STE))
      return nullptr;

    return IDN;
  } else {
    IDN = new ASTIdentifierNode(Id, unsigned(0U));
    assert(IDN && "Could not create a valid ASTIdentifierNode!");

    ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(IDN, ASTTypeUndefined);
    assert(STE && "Could not create a valid SymbolTable Entry!");

    if (!ASTSymbolTable::Instance().InsertUndefined(IDN, STE))
      return nullptr;

    return IDN;
  }

  return nullptr;
}

ASTIdentifierNode*
ASTBuilder::CreateASTIdentifierNode(const std::string& Id, unsigned Bits,
                                    ASTType Type,
                                    const ASTDeclarationContext* DC,
                                    bool TypeSystemBuilder) {
  if (Id.empty() || ASTIdentifierNode::InvalidBits(Bits))
    return nullptr;

  ASTType STy = Type;

  if (ASTAngleContextControl::Instance().InOpenContext() &&
      ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id)) {
    ASTIdentifierNode* IDN = nullptr;

    if (Type != ASTTypeAngle && Type != ASTTypeGateQubitParam)
      Type = ASTTypeAngle;

    // ASTTypeSystemBuilder will create the reserved pi, tau and euler
    // ASTMPDecimalNodes in the Global Context.
    if (TypeSystemBuilder)
      Type = STy;

    IDN = new ASTIdentifierNode(Id, Type,
                                Bits == 0 ? ASTAngleNode::AngleBits : Bits);
    assert(IDN && "Could not create a valid ASTIdentifierNode!");

    if (DC)
      IDN->SetDeclarationContext(DC);
    else
      IDN->SetDeclarationContext(
           ASTDeclarationContextTracker::Instance().GetCurrentContext());

    ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(IDN, IDN->GetSymbolType());
    assert(STE && "Could not create a valid ASTSymbolTable Entry!");

    if (DC)
      STE->SetContext(DC);
    else
      STE->SetContext(
           ASTDeclarationContextTracker::Instance().GetCurrentContext());

    if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DC)) {
      IDN->SetGlobalScope();
      STE->SetGlobalScope();
    } else {
      IDN->SetLocalScope();
      STE->SetLocalScope();
    }

    IDN->SetSymbolTableEntry(STE);

    if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DC)) {
      if (!ASTSymbolTable::Instance().InsertGlobal(IDN, STE))
        return nullptr;
      return IDN;
    } else {
      if (!ASTSymbolTable::Instance().Insert(IDN, STE))
        return nullptr;
      return IDN;
    }
  } else if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id)) {
    if (Type != ASTTypeAngle)
      Type = ASTTypeAngle;

    // ASTTypeSystemBuilder will create the reserved pi, tau and euler
    // ASTMPDecimalNodes in the Global Context.
    if (TypeSystemBuilder)
      Type = STy;

    ASTIdentifierNode* IDN = nullptr;
    ASTSymbolTableEntry* STE = nullptr;

    if (TypeSystemBuilder) {
      IDN = new ASTIdentifierNode(Id, Type, Bits);
      assert(IDN && "Could not create a valid ASTIdentifierNode!");

      IDN->SetDeclarationContext(DC);
      IDN->SetGlobalScope();

      STE = new ASTSymbolTableEntry(IDN, Type);
      assert(STE && "Could not create a valid ASTSymbolTable Entry!");

      STE->SetContext(DC);
      STE->SetGlobalScope();
    } else {
      IDN = new ASTIdentifierNode(Id, Type,
                                  Bits == 0 ? ASTAngleNode::AngleBits : Bits);
      assert(IDN && "Could not create a valid ASTIdentifierNode!");

      IDN->SetDeclarationContext(
              ASTDeclarationContextTracker::Instance().GetGlobalContext());
      STE = new ASTSymbolTableEntry(IDN, ASTTypeAngle);
      assert(STE && "Could not create a valid ASTSymbolTable Entry!");

      STE->SetContext(ASTDeclarationContextTracker::Instance().GetGlobalContext());
    }

    if (ASTCalContextBuilder::Instance().InOpenContext()) {
      if (!ASTSymbolTable::Instance().InsertToCalibrationTable(IDN, STE))
        return nullptr;
    } else {
      if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DC) &&
          TypeSystemBuilder) {
        if (!ASTSymbolTable::Instance().InsertGlobal(IDN, STE))
          return nullptr;
      } else {
        if (!ASTSymbolTable::Instance().Insert(IDN, STE))
          return nullptr;
      }
    }

    IDN->SetSymbolTableEntry(STE);
    IDN->SetGlobalScope();
    STE->SetGlobalScope();
    STE->SetDoNotDelete();
    return IDN;
  } else if (ASTTypeSystemBuilder::Instance().IsBuiltinFunction(Id)) {
    assert(Type == ASTTypeFunction &&
           "Builtin functions can only be of ASTTypeFunction!");
    assert(Bits == ASTFunctionDefinitionNode::FunctionBits &&
           "Function bits are immutable!");

    ASTIdentifierNode* IDN = new ASTIdentifierNode(Id, Type, Bits);
    assert(IDN && "Could not create a valid ASTIdentifierNode!");

    IDN->SetDeclarationContext(
         ASTDeclarationContextTracker::Instance().GetGlobalContext());
    ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(IDN, Type);
    assert(STE && "Could not create a valid ASTSymbolTable Entry!");

    STE->SetContext(ASTDeclarationContextTracker::Instance().GetGlobalContext());

    IDN->SetGlobalScope();
    STE->SetGlobalScope();
    STE->SetDoNotDelete();
    IDN->SetSymbolTableEntry(STE);

    if (!ASTSymbolTable::Instance().InsertFunction(IDN, STE))
      return nullptr;
    return IDN;
  } else if (ASTIdentifierTypeController::Instance().IsGateType(Type)) {
    assert(Bits == ASTGateNode::GateBits && "Gate bits are immutable!");

    ASTIdentifierNode* IDN = new ASTIdentifierNode(Id, Type, Bits);
    assert(IDN && "Could not create a valid ASTIdentifierNode!");

    IDN->SetDeclarationContext(
         ASTDeclarationContextTracker::Instance().GetGlobalContext());

    ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(IDN, Type);
    assert(STE && "Could not create a valid ASTSymbolTable Entry!");

    STE->SetContext(ASTDeclarationContextTracker::Instance().GetGlobalContext());

    IDN->SetGlobalScope();
    STE->SetGlobalScope();
    STE->SetDoNotDelete();
    IDN->SetSymbolTableEntry(STE);

    if (!ASTSymbolTable::Instance().InsertGate(IDN, STE))
      return nullptr;
    return IDN;
  } else if (Type == ASTTypeDefcal) {
    assert(Bits == ASTDefcalNode::DefcalBits && "Defcal bits are immutable!");

    ASTIdentifierNode* IDN = new ASTIdentifierNode(Id, Type, Bits);
    assert(IDN && "Could not create a valid ASTIdentifierNode!");

    ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(IDN, Type);
    assert(STE && "Could not create a valid ASTSymbolTable Entry!");

    IDN->SetDefcalGroupName(Id);
    IDN->SetSymbolTableEntry(STE);
    IDN->SetGlobalScope();
    STE->SetGlobalScope();
    STE->SetDoNotDelete();

    ASTMapSymbolTableEntry* MSTE =
      ASTSymbolTable::Instance().FindDefcalGroup(IDN->GetDefcalGroupName());
    if (!MSTE) {
      ASTIdentifierNode* MId = new ASTIdentifierNode(Id, ASTTypeDefcalGroup,
                                                     ASTDefcalNode::DefcalBits);
      assert(MId && "Could not create a valid ASTIdentifierNode!");

      MSTE = new ASTMapSymbolTableEntry(MId, ASTTypeDefcalGroup);
      assert(MSTE && "Could not create a valid ASTMapSymbolTable Entry!");

      MId->SetSymbolTableEntry(STE);
      MId->SetGlobalScope();
      MSTE->SetGlobalScope();
      MSTE->SetDoNotDelete();

      if (!ASTSymbolTable::Instance().InsertDefcalGroup(MId, MSTE))
        return nullptr;
    }

    if (!ASTSymbolTable::Instance().InsertDefcal(IDN, STE))
      return nullptr;
    return IDN;
  } else if (Type == ASTTypeDefcalGroup) {
    assert(Bits == ASTDefcalNode::DefcalBits && "Defcal bits are immutable!");

    ASTIdentifierNode* IDN = new ASTIdentifierNode(Id, Type, Bits);
    assert(IDN && "Could not create a valid ASTIdentifierNode!");

    ASTMapSymbolTableEntry* STE = new ASTMapSymbolTableEntry(IDN, Type);
    assert(STE && "Could not create a valid ASTSymbolTable Entry!");

    IDN->SetSymbolTableEntry(STE);
    IDN->SetGlobalScope();
    STE->SetGlobalScope();
    STE->SetDoNotDelete();

    if (!ASTSymbolTable::Instance().InsertDefcalGroup(IDN, STE))
      return nullptr;
    return IDN;
  } else if (Type == ASTTypeFunction) {
    assert(Bits == ASTFunctionDefinitionNode::FunctionBits &&
           "Function Definition bits are immutable!");

    ASTIdentifierNode* IDN = new ASTIdentifierNode(Id, Type, Bits);
    assert(IDN && "Could not create a valid ASTIdentifierNode!");

    ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(IDN, Type);
    assert(STE && "Could not create a valid ASTSymbolTable Entry!");

    IDN->SetGlobalScope();
    STE->SetGlobalScope();
    STE->SetDoNotDelete();
    IDN->SetSymbolTableEntry(STE);

    if (!ASTSymbolTable::Instance().InsertFunction(IDN, STE))
      return nullptr;
    return IDN;
  } else if (Type == ASTTypeFunctionDeclaration) {
    assert(Bits == ASTFunctionDeclarationNode::FunctionDeclBits &&
           "Function Declaration bits are immutable!");

    ASTIdentifierNode* IDN = new ASTIdentifierNode(Id, Type, Bits);
    assert(IDN && "Could not create a valid ASTIdentifierNode!");

    ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(IDN, Type);
    assert(STE && "Could not create a valid ASTSymbolTable Entry!");

    if (!ASTSymbolTable::Instance().InsertFunction(IDN, STE))
      return nullptr;

    IDN->SetGlobalScope();
    STE->SetGlobalScope();
    STE->SetDoNotDelete();
    IDN->SetSymbolTableEntry(STE);
    return IDN;
  } else if (Type == ASTTypeUndefined) {
    assert(Bits == 0 && "ASTTypeUndefined can only have 0 Bits!");

    const ASTDeclarationContext* CTX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();

    ASTIdentifierNode* IDN = new ASTIdentifierNode(Id, Type, Bits);
    assert(IDN && "Could not create a valid ASTIdentifierNode!");

    if (DC) {
      IDN->SetDeclarationContext(DC);
      if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DC))
        IDN->SetGlobalScope();
      else
        IDN->SetLocalScope();
    } else {
      IDN->SetDeclarationContext(CTX);
      if (ASTDeclarationContextTracker::Instance().IsGlobalContext(CTX))
        IDN->SetGlobalScope();
      else
        IDN->SetLocalScope();
    }

    ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(IDN, Type);
    assert(STE && "Could not create a valid ASTSymbolTable Entry!");

    if (DC) {
      STE->SetContext(DC);
      if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DC))
        STE->SetGlobalScope();
      else
        STE->SetLocalScope();
    } else {
      STE->SetContext(CTX);
      if (ASTDeclarationContextTracker::Instance().IsGlobalContext(CTX))
        STE->SetGlobalScope();
      else
        STE->SetLocalScope();
    }

    IDN->SetSymbolTableEntry(STE);

    if (ASTCalContextBuilder::Instance().InOpenContext()) {
      if (!ASTSymbolTable::Instance().InsertToCalibrationTable(IDN, STE))
        return nullptr;
    } else {
      if (!ASTSymbolTable::Instance().InsertUndefined(IDN, STE))
        return nullptr;
    }

    return IDN;
  } else {
    assert(!ASTIdentifierNode::InvalidBits(Bits) && "Invalid number of bits!");

    const ASTDeclarationContext* CTX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();

    ASTIdentifierNode* IDN = new ASTIdentifierNode(Id, Type, Bits);
    assert(IDN && "Could not create a valid ASTIdentifierNode!");

    if (DC) {
      IDN->SetDeclarationContext(DC);
      if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DC))
        IDN->SetGlobalScope();
      else
        IDN->SetLocalScope();
    } else {
      IDN->SetDeclarationContext(CTX);
      if (ASTDeclarationContextTracker::Instance().IsGlobalContext(CTX))
        IDN->SetGlobalScope();
      else
        IDN->SetLocalScope();
    }

    ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(IDN, Type);
    assert(STE && "Could not create a valid ASTSymbolTable Entry!");

    if (DC) {
      STE->SetContext(DC);
      if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DC))
        STE->SetGlobalScope();
      else
        STE->SetLocalScope();
    } else {
      STE->SetContext(CTX);
      if (ASTDeclarationContextTracker::Instance().IsGlobalContext(CTX))
        STE->SetGlobalScope();
      else
        STE->SetLocalScope();
    }

    IDN->SetSymbolTableEntry(STE);

    if (DC) {
      if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DC)) {
        if (!ASTSymbolTable::Instance().InsertGlobal(IDN, STE))
          return nullptr;
        return IDN;
      } else {
        if (!ASTSymbolTable::Instance().InsertLocal(IDN, STE))
          return nullptr;
        return IDN;
      }
    } else {
      if (ASTCalContextBuilder::Instance().InOpenContext()) {
        if (!ASTSymbolTable::Instance().InsertToCalibrationTable(IDN, STE))
          return nullptr;
        return IDN;
      } else if (ASTDeclarationContextTracker::Instance().IsGlobalContext(CTX)) {
        if (!ASTSymbolTable::Instance().InsertGlobal(IDN, STE))
          return nullptr;
        return IDN;
      } else {
        if (!ASTSymbolTable::Instance().InsertLocal(IDN, STE))
          return nullptr;
        return IDN;
      }
    }
  }

  return nullptr;
}

// WARNING: This method does not perform any Symbol Table type checks.
// It simply creates and inserts into the Local Symbol Table.
ASTIdentifierNode*
ASTBuilder::CreateLocalScopeASTIdentifierNode(const std::string& Id,
                                              unsigned Bits,
                                              ASTType Type,
                                              const ASTDeclarationContext* DCX,
                                              const ASTToken* TK) {
  assert(DCX && "Invalid ASTDeclarationContext argument!");
  assert(TK && "Invalid ASTToken argument!");

  if (ASTDeclarationContextTracker::Instance().IsGlobalContext(DCX)) {
    std::stringstream M;
    M << "Declaration Context is Global.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::Error);
    return nullptr;
  }

  const ASTSymbolTableEntry* LSTE =
    ASTSymbolTable::Instance().FindLocalSymbol(Id, Bits, Type);
  if (LSTE) {
    std::stringstream M;
    M << "Symbol " << Id << " of type " << PrintTypeEnum(Type)
      << " size " << Bits << " already exists at Local Scope.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::Warning);
    return const_cast<ASTIdentifierNode*>(LSTE->GetIdentifier());
  }

  ASTIdentifierNode* IDN = new ASTIdentifierNode(Id, Type, Bits);
  assert(IDN && "Could not create a valid ASTIdentifierNode!");

  ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(IDN, Type);
  assert(STE && "Could not create a valid ASTSymbolTableEntry!");

  IDN->SetDeclarationContext(DCX);
  IDN->SetLocation(TK->GetLocation());
  IDN->SetLocalScope();
  STE->SetContext(DCX);
  STE->SetLocalScope();
  IDN->SetSymbolTableEntry(STE);

  if (!ASTSymbolTable::Instance().InsertLocal(IDN, STE)) {
    std::stringstream M;
    M << "Failure inserting symbol " << Id << " into the Local Symbol "
      << "Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(TK), M.str(), DiagLevel::Error);
    return nullptr;
  }

  return IDN;
}

ASTIdentifierNode*
ASTBuilder::CreateASTPragmaIdentifierNode(const std::string& Id) {
  if (Id.empty())
    return nullptr;

  if (!ASTPragmaContextBuilder::Instance().InOpenContext()) {
    std::stringstream M;
    M << "Attempting to create a pragma identifier node outside a "
      << "pragma context.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ASTIdentifierNode* PId = new ASTIdentifierNode(Id, ASTTypePragma,
                                                 ASTPragmaNode::PragmaBits);
  assert(PId && "Could not create a valid pragma ASTIdentifierNode!");

  PId->SetLocalScope();
  return PId;
}

ASTIdentifierNode*
ASTBuilder::FindASTIdentifierNode(const std::string& Id) {
  if (Id.empty())
    return nullptr;

  if (ASTCalContextBuilder::Instance().InOpenContext() ||
      ASTDefcalContextBuilder::Instance().InOpenContext()) {
    if (ASTSymbolTableEntry* STE =
        ASTSymbolTable::Instance().FindOpenPulseSymbol(Id))
      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
  }

  if (ASTAngleContextControl::Instance().InOpenContext() &&
      ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id) &&
      !ASTIdentifierTypeController::Instance().SeenLBrace()) {
    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(Id, ASTAngleNode::AngleBits,
                                        ASTTypeAngle);
    if (STE)
      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());

    STE = ASTSymbolTable::Instance().Lookup(Id, 1U, ASTTypeQubitContainer);
    if (STE)
      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());

    STE = ASTSymbolTable::Instance().Lookup(Id, 1U, ASTTypeGateQubitParam);
    if (STE)
      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());

    return nullptr;
  }

  if (ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id) &&
      (ASTGateContextBuilder::Instance().InOpenContext() ||
       ASTDefcalContextBuilder::Instance().InOpenContext())) {
    if (ASTSymbolTableEntry* STE =
        ASTSymbolTable::Instance().Lookup(Id, ASTAngleNode::AngleBits,
                                          ASTTypeAngle)) {
      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
    }
  }

  if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id))
    return const_cast<ASTIdentifierNode*>(ASTSymbolTable::Instance().LookupAngle(Id));
  else if (ASTTypeSystemBuilder::Instance().IsBuiltinFunction(Id))
    return const_cast<ASTIdentifierNode*>(ASTSymbolTable::Instance().LookupFunction(Id));

  if (ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id))
    return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());

  ASTType CTy = ASTIdentifierTypeController::Instance().GetCurrentType();
  ASTSymbolTableEntry* STE = nullptr;

  switch (CTy) {
  case ASTTypeDefcal: {
    STE = ASTSymbolTable::Instance().Lookup(Id, ASTDefcalNode::DefcalBits, CTy);
    return STE ? const_cast<ASTIdentifierNode*>(STE->GetIdentifier()) : nullptr;
  }
    break;
  case ASTTypeGate: {
    STE = ASTSymbolTable::Instance().Lookup(Id, ASTGateNode::GateBits, CTy);
    return STE ? const_cast<ASTIdentifierNode*>(STE->GetIdentifier()) : nullptr;
  }
    break;
  case ASTTypeGateQubitParam: {
    if (ASTIdentifierTypeController::Instance().InQubitList()) {
      STE = ASTSymbolTable::Instance().Lookup(Id, 1U, CTy);
      return STE ? const_cast<ASTIdentifierNode*>(STE->GetIdentifier()) : nullptr;
    }
  }
    break;
  default:
    break;
  }

  std::multimap<unsigned, ASTIdentifierNode*> IM;

  ASTIdentifierNode* IDN =
    const_cast<ASTIdentifierNode*>(ASTSymbolTable::Instance().LookupLocal(Id));
  if (IDN) {
    IM.insert(std::make_pair(IDN->GetDeclarationContext()->GetIndex(), IDN));
    goto found;
  } else {
    IDN = const_cast<ASTIdentifierNode*>(ASTSymbolTable::Instance().LookupGate(Id));
  }

  if (IDN) {
    IM.insert(std::make_pair(IDN->GetDeclarationContext()->GetIndex(), IDN));
    goto found;
  } else {
    IDN = const_cast<ASTIdentifierNode*>(ASTSymbolTable::Instance().LookupDefcal(Id));
  }

  if (IDN) {
    IM.insert(std::make_pair(IDN->GetDeclarationContext()->GetIndex(), IDN));
    goto found;
  } else {
    IDN = const_cast<ASTIdentifierNode*>(ASTSymbolTable::Instance().LookupQubit(Id));
  }

  if (IDN) {
    IM.insert(std::make_pair(IDN->GetDeclarationContext()->GetIndex(), IDN));
    goto found;
  } else {
    IDN = const_cast<ASTIdentifierNode*>(ASTSymbolTable::Instance().LookupFunction(Id));
  }

  if (IDN) {
    IM.insert(std::make_pair(IDN->GetDeclarationContext()->GetIndex(), IDN));
    goto found;
  } else {
    IDN = const_cast<ASTIdentifierNode*>(ASTSymbolTable::Instance().LookupAngle(Id));
  }

  if (IDN) {
    IM.insert(std::make_pair(IDN->GetDeclarationContext()->GetIndex(), IDN));
    goto found;
  } else {
    IDN = const_cast<ASTIdentifierNode*>(ASTSymbolTable::Instance().LookupGlobal(Id));
  }

  if (IDN) {
    IM.insert(std::make_pair(IDN->GetDeclarationContext()->GetIndex(), IDN));
    goto found;
  } else {
    IDN = const_cast<ASTIdentifierNode*>(ASTSymbolTable::Instance().LookupUndefined(Id));
  }

  if (IDN) {
    IM.insert(std::make_pair(IDN->GetDeclarationContext()->GetIndex(), IDN));
    goto found;
  }

  if (ASTStringUtils::Instance().IsIndexed(Id)) {
    std::vector<const ASTIdentifierNode*> VID =
      ASTSymbolTable::Instance().LookupGeneric(Id);
    if (!VID.empty()) {
      for (std::vector<const ASTIdentifierNode*>::iterator I = VID.begin();
           I != VID.end(); ++I) {
        const ASTIdentifierRefNode* IDR = nullptr;

        if ((IDR = dynamic_cast<const ASTIdentifierRefNode*>(*I)) != nullptr) {
          IM.insert(std::make_pair(
                    IDR->GetIdentifier()->GetDeclarationContext()->GetIndex(),
                    const_cast<ASTIdentifierRefNode*>(IDR)));
        } else {
          IDN = const_cast<ASTIdentifierNode*>(*I);

          if (ASTStringUtils::Instance().IsIndexed(Id)) {
            unsigned IX = ASTStringUtils::Instance().GetIdentifierIndex(Id);
            IDR = IDN->GetReference(IX);
            IM.insert(std::make_pair(
                IDR->GetIdentifier()->GetDeclarationContext()->GetIndex(),
                const_cast<ASTIdentifierRefNode*>(IDR)));
          } else {
            IM.insert(std::make_pair(
                IDN->GetDeclarationContext()->GetIndex(), IDN));
          }
        }
      }
    }
  }

found:
  if (IM.size() == 1) {
    std::multimap<unsigned, ASTIdentifierNode*>::iterator I = IM.begin();
    return (*I).second;
  } else if (IM.size() > 1) {
    std::stringstream M;
    M << "Ambiguous Identifier '" << Id << "' as it resolves "
      << "to more than one SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);

    std::multimap<unsigned, ASTIdentifierNode*>::iterator I = IM.end();
    --I;
    return (*I).second;
  }

  std::vector<ASTSymbolTableEntry*> R = ASTSymbolTable::Instance().LookupRange(Id);

  if (R.empty())
    return nullptr;

  if (R.size() > 1) {
    std::stringstream M;
    M << "Ambiguous Identifier '" << Id << "' resolves "
      << "to more than one Symbol.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  const ASTDeclarationContext* GDC =
    ASTDeclarationContextTracker::Instance().GetGlobalContext();

  for (std::vector<ASTSymbolTableEntry*>::const_iterator I = R.begin();
       I != R.end(); ++I) {
    const ASTDeclarationContext* CDC =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
    if ((*I)->GetContext() == CDC)
      return const_cast<ASTIdentifierNode*>((*I)->GetIdentifier());

    CDC = CDC->GetParentContext();

    while (CDC) {
      if ((*I)->GetContext() == CDC)
        return const_cast<ASTIdentifierNode*>((*I)->GetIdentifier());

      CDC = CDC->GetParentContext();
    }

    if ((*I)->GetContext() == GDC ||
        (*I)->GetContext()->GetIndex() > GDC->GetIndex())
      return const_cast<ASTIdentifierNode*>((*I)->GetIdentifier());
  }

  return nullptr;
}

ASTIdentifierNode*
ASTBuilder::FindASTIdentifierNode(const ASTIdentifierNode* Id) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->GetBits() != 0 && Id->GetSymbolType() != ASTTypeUndefined) {
    if (ASTCalContextBuilder::Instance().InOpenContext() ||
        ASTDefcalContextBuilder::Instance().InOpenContext()) {
      if (ASTSymbolTableEntry* STE =
          ASTSymbolTable::Instance().FindOpenPulseSymbol(Id))
        return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
    }

    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), Id->GetSymbolType());
    if (STE)
      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
  }

  return FindASTIdentifierNode(Id->GetName());
}

ASTIdentifierNode*
ASTBuilder::FindASTIdentifierNode(const std::string& Id, ASTType Ty) {
  if (!Id.empty()) {
    if (ASTCalContextBuilder::Instance().InOpenContext() ||
        ASTDefcalContextBuilder::Instance().InOpenContext()) {
      if (ASTSymbolTableEntry* STE =
          ASTSymbolTable::Instance().FindOpenPulseSymbol(Id, Ty))
        return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
    }

    if (ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id, Ty)) {
      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
    }
  }

  return nullptr;
}

ASTIdentifierNode*
ASTBuilder::FindASTIdentifierNode(const std::string& Id, unsigned Bits,
                                  ASTType Type) {
  if (!Id.empty()) {
    if (ASTCalContextBuilder::Instance().InOpenContext() ||
        ASTDefcalContextBuilder::Instance().InOpenContext()) {
      if (ASTSymbolTableEntry* STE =
          ASTSymbolTable::Instance().FindOpenPulseSymbol(Id, Bits, Type))
        return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
    }

    if (ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id, Bits,
                                                                     Type))
      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
  }

  return nullptr;
}

unsigned
ASTBuilder::FindASTIdentifierNode(const std::string& Id,
                                  unsigned Bits,
                                  ASTType Type,
                                  std::vector<ASTSymbolTableEntry*>& STV) {
  if (ASTAngleContextControl::Instance().InOpenContext() &&
      ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id) &&
      Type == ASTTypeAngle) {
    if (ASTSymbolTableEntry* STE =
        ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeAngle))
      STV.push_back(STE);

    return static_cast<unsigned>(STV.size());
  }

  STV = ASTSymbolTable::Instance().LookupRange(Id);

  std::vector<std::vector<ASTSymbolTableEntry*>::iterator> EV;

  for (std::vector<ASTSymbolTableEntry*>::iterator I = STV.begin();
       I != STV.end(); ++I) {
    if ((*I)->GetIdentifier()->GetBits() != Bits &&
        (*I)->GetValueType() != Type)
      EV.push_back(I);
  }

  for (std::vector<std::vector<ASTSymbolTableEntry*>::iterator>::iterator I =
       EV.begin(); I != EV.end(); ++I)
    STV.erase(*I);

  return static_cast<unsigned>(STV.size());
}

ASTIdentifierNode*
ASTBuilder::FindOrCreateASTIdentifierNode(const std::string& Id) {
  if (Id.empty())
    return nullptr;

  if (ASTAngleContextControl::Instance().InOpenContext() &&
      ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id)) {
    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(Id, ASTAngleNode::AngleBits, ASTTypeAngle);

    if (STE) {
      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
    } else {
      ASTIdentifierNode* IDN = new ASTIdentifierNode(Id, ASTAngleNode::AngleBits);
      assert(IDN && "Could not create a valid ASTIdentifierNode!");

      if (!ASTSymbolTable::Instance().Insert(IDN, ASTAngleNode::AngleBits,
                                             ASTTypeAngle)) {
        std::stringstream M;
        M << "Failure inserting an ASTIdentifierNode into the "
          << "SymbolTable";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(IDN), M.str(),
                                                        DiagLevel::Error);
        return nullptr;
      }

      STE = new ASTSymbolTableEntry(IDN, ASTTypeAngle);
      assert(STE && "Could not create a valid ASTSymbolTableEntry!");

      STE->SetLocalScope();
      IDN->SetLocalScope();
      IDN->SetSymbolTableEntry(STE);
      IDN->SetHasSymbolTableEntry(true);
      if (!ASTSymbolTable::Instance().Insert(IDN, STE)) {
        std::stringstream M;
        M << "Failure inserting an ASTSymbolTableEntry into "
          << "the SymbolTable";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(IDN), M.str(),
                                                        DiagLevel::Error);
        return nullptr;
      }

      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
    }
  } else if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id)) {
    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(Id, ASTAngleNode::AngleBits, ASTTypeAngle);
    if (!STE) {
      ASTIdentifierNode* IDN = new ASTIdentifierNode(Id, ASTAngleNode::AngleBits);
      assert(IDN && "Could not create a valid ASTIdentifierNode!");

      if (!ASTSymbolTable::Instance().Insert(IDN, ASTAngleNode::AngleBits,
                                             ASTTypeAngle)) {
        std::stringstream M;
        M << "Failure inserting an ASTIdentifierNode into the "
          << "SymbolTable";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(IDN), M.str(),
                                                        DiagLevel::Error);
        return nullptr;
      }

      STE = new ASTSymbolTableEntry(IDN, ASTTypeAngle);
      assert(STE && "Could not create a valid ASTSymbolTableEntry!");

      STE->SetDoNotDelete();
      STE->SetGlobalScope();
      if (!ASTSymbolTable::Instance().Insert(IDN, STE)) {
        std::stringstream M;
        M << "Failure inserting an ASTSymbolTableEntry into "
          << "the SymbolTable";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(IDN), M.str(),
                                                        DiagLevel::Error);
        return nullptr;
      }

      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
    }

    return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
  } else if (ASTTypeSystemBuilder::Instance().IsBuiltinFunction(Id)) {
    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(Id,
                                        ASTFunctionDefinitionNode::FunctionBits,
                                        ASTTypeFunction);
    if (!STE) {
      ASTIdentifierNode* IDN = new ASTIdentifierNode(Id,
                                   ASTFunctionDefinitionNode::FunctionBits);
      assert(IDN && "Could not create a valid ASTIdentifierNode!");

      if (!ASTSymbolTable::Instance().Insert(IDN,
                                             ASTFunctionDefinitionNode::FunctionBits,
                                             ASTTypeFunction)) {
        std::stringstream M;
        M << "Failure inserting an ASTIdentifierNode into the "
          << "SymbolTable";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(IDN), M.str(),
                                                        DiagLevel::Error);
        return nullptr;
      }

      STE = new ASTSymbolTableEntry(IDN, ASTTypeFunction);
      assert(STE && "Could not create a valid ASTSymbolTableEntry!");

      STE->SetDoNotDelete();
      STE->SetGlobalScope();
      if (!ASTSymbolTable::Instance().Insert(IDN, STE)) {
        std::stringstream M;
        M << "Failure inserting an ASTSymbolTableEntry into "
          << "the SymbolTable";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(IDN), M.str(),
                                                        DiagLevel::Error);
        return nullptr;
      }

      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
    }
  } else {
    if (const ASTIdentifierNode* IDN =
        ASTBuilder::Instance().FindASTIdentifierNode(Id))
      return const_cast<ASTIdentifierNode*>(IDN);
  }

  std::vector<ASTSymbolTableEntry*> STEV =
    ASTSymbolTable::Instance().LookupRange(Id);

  if (!STEV.empty()) {
    for (std::vector<ASTSymbolTableEntry*>::iterator I = STEV.begin();
         I != STEV.end(); ++I) {
      if ((*I)->GetIdentifier()->GetBits() != static_cast<unsigned>(~0x0) &&
          (*I)->HasValue() && (*I)->GetValueType() != ASTTypeUndefined &&
          (*I)->GetScope() == ASTSymbolScope::Global) {
        return const_cast<ASTIdentifierNode*>((*I)->GetIdentifier());
      } else if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id)) {
        ASTIdentifierNode* IId =
          const_cast<ASTIdentifierNode*>((*I)->GetIdentifier());
        IId->SetBits(ASTAngleNode::AngleBits);
        IId->SetLocalScope();
        return const_cast<ASTIdentifierNode*>((*I)->GetIdentifier());
      }
    }
  }

  ASTIdentifierNode* IDN = new ASTIdentifierNode(Id);
  assert(IDN && "Failed to construct an ASTIdentifierNode!");

  if (!ASTSymbolTable::Instance().Insert(Id, IDN)) {
    return nullptr;
  }

  return IDN;
}

ASTIdentifierNode*
ASTBuilder::FindOrCreateASTIdentifierNode(const std::string& Id,
                                          unsigned Bits,
                                          ASTType Type) {
  if (ASTAngleContextControl::Instance().InOpenContext() &&
      ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id)) {
    if (Type != ASTTypeAngle) {
      std::stringstream M;
      M << "Invalid Type " << PrintTypeEnum(Type) << " for "
        << "implicit angle.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }

    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(Id, ASTAngleNode::AngleBits, Type);
    if (STE) {
      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
    } else {
      ASTIdentifierNode* IDN =
        new ASTIdentifierNode(Id, ASTAngleNode::AngleBits);
      assert(IDN && "Could not create a valid ASTIdentifierNode!");

      STE = new ASTSymbolTableEntry(IDN, Type);
      assert(STE && "Could not create a valid ASTSymbolTableEntry!");

      if (!ASTSymbolTable::Instance().Insert(IDN, STE))
        return nullptr;

      IDN->SetLocalScope();
      IDN->SetSymbolTableEntry(STE);
      IDN->SetHasSymbolTableEntry(true);
      STE->SetDoNotDelete();
      STE->SetLocalScope();
      return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
    }
  }

  ASTSymbolTableEntry* STE = nullptr;

  if ((STE = ASTSymbolTable::Instance().Lookup(Id, Bits, Type)) != nullptr) {
    if (STE->GetIdentifier()->GetBits() != Bits) {
      switch (STE->GetValueType()) {
      case ASTTypeUndefined:
      case ASTTypeIdentifier:
      case ASTTypeGate:
      case ASTTypeMeasure:
      case ASTTypeBarrier:
      case ASTTypeMPInteger:
      case ASTTypeMPDecimal:
      case ASTTypeInt:
      case ASTTypeFloat:
      case ASTTypeDouble:
      case ASTTypeBool:
        STE->GetIdentifier()->SetBits(Bits);
        STE->SetValueType(Type);
        if (STE->HasValue())
          STE->GetValue()->SetASTType(Type);
        break;
      case ASTTypeQubit:
        if (Bits == 0) {
          Bits = STE->GetIdentifier()->GetBits();
          Type = STE->GetValueType();
        } else {
          STE->SetValueType(Type);
          if (STE->HasValue())
            STE->GetValue()->SetASTType(Type);
        }
        break;
      case ASTTypeQReg:
        if (STE->GetIdentifier()->GetBits() != static_cast<unsigned>(~0x0) &&
            STE->GetIdentifier()->GetBits() > Bits) {
          ASTQRegNode* QRN = STE->GetValue()->GetValue<ASTQRegNode*>();
          assert(QRN && "Could not retrieve a valid SymbolTable ASTQRegNode!");

          ASTQubitNode* QBN = QRN->GetQubit(Bits);
          assert(QBN && "Could not obtain a valid QubitNode!");

          std::stringstream S;
          S << STE->GetIdentifier()->GetName() << '[' << Bits << ']';
          ASTIdentifierRefNode* IdR =
            new ASTIdentifierRefNode(S.str(), STE->GetIdentifier(), Bits);

          assert(IdR && "Could not create a valid RValue ASTIdentifierNode!");

          if (!ASTSymbolTable::Instance().Insert(IdR, STE)) {
            std::stringstream M;
            M << "Could not insert a SymbolTableEntry for the QReg "
              << "RValue ASTIdentifierRefNode!";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                         DiagLevel::ICE);
            return nullptr;
          }

          return IdR;
        }
        break;
      case ASTTypeAngle:
      case ASTTypeQubitContainer:
      case ASTTypeQubitContainerAlias:
        if (Bits == 0 &&
            !ASTIdentifierNode::InvalidBits(STE->GetIdentifier()->GetBits())) {
          Bits = STE->GetIdentifier()->GetBits();
          Type = STE->GetValueType();
        } else if (STE->GetIdentifier()->GetBits() == static_cast<unsigned>(~0x0) ||
            STE->GetIdentifier()->GetBits() != Bits) {
          if (STE->GetIdentifier()->GetBits() == static_cast<unsigned>(~0x0))
            STE->GetIdentifier()->SetBits(Bits);
          else if (Bits < STE->GetIdentifier()->GetBits()) {
            std::stringstream S;
            S << STE->GetIdentifier()->GetName() << '[' << Bits << ']';

            ASTIdentifierRefNode* IdR =
              new ASTIdentifierRefNode(S.str(), STE->GetIdentifier(), 1);
            assert(IdR && "Could not create a valid RValue ASTIdentifierNode!");

            if (!ASTSymbolTable::Instance().Insert(IdR, STE)) {
              std::stringstream M;
              M << "Could not insert a SymbolTableEntry for the Qubit "
                << "RValue ASTIdentifierRefNode!";
              QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                           DiagLevel::ICE);
              return nullptr;
            }

            return IdR;
          } else {
            std::stringstream M;
            M << "Qubit Index exceeds the available number of Qubits "
              << "for the " << PrintTypeEnum(STE->GetValueType()) << ".";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                         DiagLevel::Error);
            return nullptr;
          }
        }
        break;
      case ASTTypeBitset:
        if (STE->GetIdentifier()->GetBits() == static_cast<unsigned>(~0x0) ||
            STE->GetIdentifier()->GetBits() != Bits) {
          if (STE->GetIdentifier()->GetBits() == static_cast<unsigned>(~0x0))
            STE->GetIdentifier()->SetBits(1);
          else if (Bits < STE->GetIdentifier()->GetBits()) {
            std::stringstream S;
            S << STE->GetIdentifier()->GetName() << '[' << Bits << ']';

            ASTIdentifierRefNode* IdR =
              new ASTIdentifierRefNode(S.str(), STE->GetIdentifier(), Bits);
            assert(IdR && "Could not create a valid RValue ASTIdentifierNode!");

            if (!ASTSymbolTable::Instance().Insert(IdR, STE)) {
              std::stringstream M;
              M << "Could not insert a SymbolTableEntry for the "
                << "RValue ASTIdentifierRefNode!";
              QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                           DiagLevel::ICE);
              return nullptr;
            }

            return IdR;
          } else {
            std::stringstream M;
            M << "Bit Index exceeds the available number of Bits for "
              << "the " << PrintTypeEnum(STE->GetValueType()) << ".";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                         DiagLevel::Error);
            return nullptr;
          }
        }
        break;
      case ASTTypeArray:
      case ASTTypeAngleArray:
      case ASTTypeBoolArray:
      case ASTTypeCBitArray:
      case ASTTypeCBitNArray:
      case ASTTypeFloatArray:
      case ASTTypeIntArray:
      case ASTTypeLengthArray:
      case ASTTypeMPDecimalArray:
      case ASTTypeMPIntegerArray:
      case ASTTypeQubitArray:
      case ASTTypeQubitNArray:
        return CreateArrayASTIdentifierRefNode(STE->GetIdentifier(), STE->GetValueType(),
                                               STE, Bits);
        break;
      default:
        if (STE->GetValueType() == Type) {
          STE->GetIdentifier()->SetBits(Bits);
          if (STE->HasValue())
            STE->GetValue()->SetASTType(Type);
        }
        break;
      }
    }

    return const_cast<ASTIdentifierNode*>(STE->GetIdentifier());
  }

  ASTIdentifierNode* IDN = new ASTIdentifierNode(Id, Bits);
  assert(IDN && "Failed to construct an ASTIdentifierNode!");

  if (IDN->IsIndexed()) {
    ASTIdentifierNode* UId =
      ASTBuilder::Instance().FindUnindexedASTIdentifierNode(IDN);
    if (!UId) {
      std::stringstream M;
      M << "No Corresponding Unindexed ASTIdentifierNode for Identifier "
        << IDN->GetName();
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return nullptr;
    }

    ASTSymbolTableEntry* USTE = ASTSymbolTable::Instance().Lookup(UId);
    if (!USTE) {
      std::stringstream M;
      M << "Unindexed ASTIdentifierNode " << UId->GetName() << " has "
        << "no SymbolTable Entry!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return nullptr;
    }

    Type = USTE->GetValueType();
    return FindOrCreateASTIdentifierNode(UId->GetName(), IDN->GetBits(),
                                         Type);
  }

  if (!ASTSymbolTable::Instance().Insert(Id, IDN, Type))
    return nullptr;

  STE = ASTSymbolTable::Instance().Lookup(Id, Bits, Type);
  if (!STE)
    return nullptr;

  STE->GetIdentifier()->SetBits(Bits);
  STE->SetValueType(Type);
  IDN->SetSymbolTableEntry(STE);
  IDN->SetHasSymbolTableEntry(true);

  return IDN;
}

ASTIdentifierNode*
ASTBuilder::CreateASTIdentifierRValueReference(const ASTIdentifierNode* Id) {
  assert(Id && "Inalid ASTIdentifierNode argument!");

  if (!Id->IsIndexed()) {
    std::stringstream M;
    M << "ASTIdentifierNode argument is not Indexed!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return const_cast<ASTIdentifierNode*>(Id);
  }

  unsigned Index = static_cast<unsigned>(~0x0);

  ASTIdentifierNode* UId =
    ASTBuilder::Instance().FindUnindexedASTIdentifierNode(Id);
  if (!UId) {
    std::stringstream M;
    M << "No Corresponding Unindexed ASTIdentifierNode for Identifier "
      << Id->GetName();
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(UId);
  assert(STE && "Could not retrieve a valid Unindexed SymbolTable Entry!");
  assert(STE->HasValue() && "Unindexed SymbolTable Entry has no Value!");

  ASTType Ty = STE->GetValueType();

  switch (Ty) {
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias:
  case ASTTypeBitset:
  case ASTTypeArray:
  case ASTTypeAngleArray:
  case ASTTypeBoolArray:
  case ASTTypeCBitArray:
  case ASTTypeCBitNArray:
  case ASTTypeFloatArray:
  case ASTTypeIntArray:
  case ASTTypeLengthArray:
  case ASTTypeMPDecimalArray:
  case ASTTypeMPIntegerArray:
  case ASTTypeQubitArray:
  case ASTTypeQubitNArray:
    break;
  default: {
    std::stringstream M;
    M << "RValue Type " << PrintTypeEnum(Ty) << " is not Array-Indexable!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  if (!Id->IndexIsIdentifier()) {
    Index = Id->GetBits();
    if (Index == static_cast<unsigned>(~0x0)) {
      std::stringstream M;
      M << "Invalid number of Bits from ASTIdentifierNode!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return nullptr;
    }

    std::stringstream S;
    S << UId->GetName() << '[' << Index << ']';
    return ASTBuilder::Instance().FindOrCreateASTIdentifierNode(S.str(),
                                                                Index, Ty);
  }

  STE = ASTSymbolTable::Instance().Lookup(Id->GetIndexIdentifier());
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");
  assert(STE->HasValue() && "Identifier SymbolTable Entry has no Value!");

  if (STE->GetValueType() != ASTTypeInt &&
      STE->GetValueType() != ASTTypeMPInteger) {
    std::stringstream M;
    M << "Index Value is not an Integer Constant Expression!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  Index = ASTUtils::Instance().GetUnsignedValue(STE->GetIdentifier());
  if (Index == static_cast<unsigned>(~0x0)) {
    std::stringstream M;
    M << "Invalid number of Bits from Index ASTIdentifierNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  std::stringstream S;
  S << UId->GetName() << '[' << Index << ']';

  return ASTBuilder::Instance().FindOrCreateASTIdentifierNode(S.str(),
                                                              Index, Ty);
}

ASTIdentifierNode*
ASTBuilder::FindOrCreateASTIdentifierNode(const std::string& Id,
                                          const ASTBinaryOpNode* BOP,
                                          unsigned Bits,
                                          ASTType Type) {
  ASTIdentifierNode* RId =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(Id, Bits, Type);
  assert(RId && "Could not create a valid ASTIdentifierNode!");

  if (!BOP->IsIntegerConstantExpression()) {
    std::stringstream M;
    M << "BinaryOp argument to Indexed ASTIdentifierNode is not an "
      << "Integer Constant Expression.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Error);
    return nullptr;
  }

  RId->SetBinaryOp(BOP);
  return RId;
}

ASTIdentifierNode*
ASTBuilder::FindOrCreateASTIdentifierNode(const std::string& Id,
                                          const ASTUnaryOpNode* UOP,
                                          unsigned Bits,
                                          ASTType Type) {
  ASTIdentifierNode* RId =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(Id, Bits, Type);
  assert(RId && "Could not create a valid ASTIdentifierNode!");

  if (!UOP->IsIntegerConstantExpression()) {
    std::stringstream M;
    M << "UnaryOp argument to Indexed ASTIdentifierNode is not an "
      << "Integer Constant Expression.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Error);
    return nullptr;
  }

  RId->SetUnaryOp(UOP);
  return RId;
}

ASTIdentifierNode*
ASTBuilder::FindOrCreateIndexedASTIdentifierNode(const std::string& Id,
                                                 unsigned Bits,
                                                 ASTType Type) {
  ASTIdentifierNode* Ident =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(Id, Bits, Type);

  if (Ident) {
    ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
    if (!STE) {
      std::stringstream M;
      M << "No SymbolTableEntry for ASTIdentifierNode " << Id << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }

    // FIXME: Full Implementation.
    if (!STE->HasValue()) {
      switch (Type) {
      case ASTTypeInt:
        STE->SetValue(new ASTValue<>(static_cast<int>(Bits), Type), Type);
        return Ident;
        break;
      case ASTTypeUnsignedInt:
        STE->SetValue(new ASTValue<>(Bits, Type), Type);
        return Ident;
        break;
      case ASTTypeMPInteger: {
        ASTMPIntegerNode* MPI =
          ASTBuilder::Instance().CreateASTMPIntegerNode(Ident, Bits);
        STE->SetValue(new ASTValue<>(MPI, Type), Type);
        return Ident;
      }
        break;
      case ASTTypeMPDecimal: {
        ASTMPDecimalNode* MPD =
          ASTBuilder::Instance().CreateASTMPDecimalNode(Ident, Bits);
        STE->SetValue(new ASTValue<>(MPD, Type), Type);
        return Ident;
      }
        break;
      default:
        STE->SetValue(new ASTValue<>(Bits, Type), Type);
        return Ident;
        break;
      }
    }

    if ((STE->GetValueType() == Type) && (STE->HasValue()) &&
        (STE->GetValue()->GetASTType() == Type))
      return Ident;

    if ((STE->GetValueType() != Type) || (STE->GetValue()->GetASTType() != Type)) {
      std::stringstream M;
      M << "SymbolTableEntry Type Mismatch for ASTIdentifierNode!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      return nullptr;
    }
  }

  std::stringstream M;
  M << "Failed to create or locate an ASTIdentifierNode for " << Id << ".";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
    DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  return nullptr;
}

ASTIdentifierNode*
ASTBuilder::FindUnindexedASTIdentifierNode(const ASTIdentifierNode* Id) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (!Id->IsIndexed()) {
    std::stringstream M;
    M << "ASTIdentifierNode " << Id->GetName() << " is not indexed.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  const std::string& IDN = Id->GetName();
  std::string::size_type F = IDN.find('[');
  std::string UIDN = IDN.substr(0, F);
  ASTIdentifierNode* UId = ASTBuilder::Instance().FindASTIdentifierNode(UIDN);
  return UId;
}

ASTBarrierNode*
ASTBuilder::CreateASTBarrierNode(const ASTIdentifierNode* Id,
                                 const ASTIdentifierList& List) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  unsigned Bits = ASTTypeSystemBuilder::Instance().GetTypeBits(ASTTypeBarrier);
  assert(!ASTTypeSystemBuilder::Instance().InvalidBits(Bits) &&
         "Invalid bits for ASTTypeBarrier!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeBarrier);
  assert(STE && "ASTBarrier has no SymbolTable Entry!");

  ASTBarrierNode* BN = new ASTBarrierNode(Id, List);
  assert(BN && "Could not create a valid ASTBarrierNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(BN, ASTTypeBarrier), ASTTypeBarrier);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTBarrierNode has no Value!");

  return BN;
}

ASTResetNode*
ASTBuilder::CreateASTResetNode(const ASTIdentifierNode* Id,
                               const ASTIdentifierNode* Target) {
  assert(Id && "Invalid Reset ASTIdentifierNode argument!");
  assert(Target && "Invalid Reset Target ASTIdentifierNode argument!");

  unsigned Bits = ASTTypeSystemBuilder::Instance().GetTypeBits(ASTTypeReset);
  assert(!ASTTypeSystemBuilder::Instance().InvalidBits(Bits) &&
         "Invalid bits for ASTTypeReset!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeReset);
  assert(STE && "ASTReset has no SymbolTable Entry!");

  ASTResetNode* RN = new ASTResetNode(Id, Target);
  assert(RN && "Could not create a valid ASTResetNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(RN, ASTTypeReset), ASTTypeReset);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTResetNode has no Value!");

  return RN;
}

ASTMeasureNode*
ASTBuilder::CreateASTMeasureNode(const ASTIdentifierNode* Id,
                                 ASTQubitContainerNode* Target,
                                 ASTCBitNode* Result,
                                 unsigned RI) {
  assert(Id && "Invalid ASTIdentifierNode Measure argument!");
  assert(Target && "Invalid Target ASTQubitContainerNode Measure argument!");
  assert(Result && "Invalid Result ASTCBitNode Measure argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTMeasureNode::MeasureBits,
                                      ASTTypeMeasure);
  assert(STE && "Could not retrieve a valid Measure SymbolTable Entry!");

  ASTMeasureNode* MN = new ASTMeasureNode(Id, Target, Result, RI);
  assert(MN && "Could not create a valid ASTMeasureNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MN, ASTTypeMeasure), ASTTypeMeasure);
  assert(STE->HasValue() && "Measure SymbolTable Entry has no Value!");

  return MN;
}

ASTMeasureNode*
ASTBuilder::CreateASTMeasureNode(const ASTIdentifierNode* Id,
                                 ASTQubitContainerNode* Target,
                                 ASTAngleNode* Result,
                                 unsigned RI) {
  assert(Id && "Invalid ASTIdentifierNode Measure argument!");
  assert(Target && "Invalid Target ASTQubitContainerNode Measure argument!");
  assert(Result && "Invalid Result ASTAngleNode Measure argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTMeasureNode::MeasureBits,
                                      ASTTypeMeasure);
  assert(STE && "Could not retrieve a valid Measure SymbolTable Entry!");

  ASTMeasureNode* MN = new ASTMeasureNode(Id, Target, Result, RI);
  assert(MN && "Could not create a valid ASTMeasureNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MN, ASTTypeMeasure), ASTTypeMeasure);
  assert(STE->HasValue() && "Measure SymbolTable Entry has no Value!");

  return MN;
}

ASTMeasureNode*
ASTBuilder::CreateASTMeasureNode(const ASTIdentifierNode* Id,
                                 ASTQubitContainerNode* Target,
                                 ASTMPComplexNode* Result,
                                 unsigned RI) {
  assert(Id && "Invalid ASTIdentifierNode Measure argument!");
  assert(Target && "Invalid Target ASTQubitContainerNode Measure argument!");
  assert(Result && "Invalid Result ASTMPComplexNode Measure argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTMeasureNode::MeasureBits,
                                      ASTTypeMeasure);
  assert(STE && "Could not retrieve a valid Measure SymbolTable Entry!");

  ASTMeasureNode* MN = new ASTMeasureNode(Id, Target, Result, RI);
  assert(MN && "Could not create a valid ASTMeasureNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(MN, ASTTypeMeasure), ASTTypeMeasure);
  assert(STE->HasValue() && "Measure SymbolTable Entry has no Value!");

  return MN;
}

void
ASTBuilder::MaterializeASTAngleNode(ASTAngleNode* AN) {
  assert(AN && "Invalid ASTAngleNode argument!");

  const ASTIdentifierNode* AId = AN->GetIdentifier();
  assert(AId && "Invalid Angle ASTIdentifierNode!");
  assert(AId->GetSymbolType() == ASTTypeAngle &&
         "Invalid Symbol type for Angle ASTIdentifierNode!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(AId, AId->GetBits(), AId->GetSymbolType());
  assert(STE && "Invalid ASTAngleNode without a SymbolTable Entry!");

  std::stringstream AS;
  const std::string& ANS = AId->GetName();

  for (unsigned I = 0; I < 4; ++I) {
    AS.clear();
    AS.str("");
    AS << ANS << '[' << I << ']';

    ASTSymbolTableEntry* XSTE =
      ASTSymbolTable::Instance().Lookup(AS.str(), ASTIntNode::IntBits,
                                        ASTTypeAngle);
    if (!XSTE) {
      ASTIdentifierRefNode* IdR = new ASTIdentifierRefNode(AS.str(), AId,
                                                           ASTIntNode::IntBits);
      assert(IdR && "Could not create a valid ASTIdentifierNode!");

      XSTE = new ASTSymbolTableEntry(IdR, ASTTypeAngle);
      assert(XSTE && "Could not create a valid ASTSymbolTableEntry!");

      XSTE->ResetValue();
      XSTE->SetValue(new ASTValue<>(AN, ASTTypeAngle), ASTTypeAngle, true);
      IdR->SetSymbolTableEntry(XSTE);

      if (ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal() ||
          STE->IsGlobalScope()) {
        if (!ASTSymbolTable::Instance().Insert(IdR, XSTE)) {
          std::stringstream M;
          M << "Failure inserting angle element in the Symbol Table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(IdR), M.str(),
                                                          DiagLevel::ICE);
        }
      } else {
        if (!ASTSymbolTable::Instance().InsertLocalSymbol(IdR, XSTE)) {
          std::stringstream M;
          M << "Failure inserting angle element in the Symbol Table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(IdR), M.str(),
                                                          DiagLevel::ICE);
        }
      }

      if (ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal() ||
          ASTTypeSystemBuilder::Instance().IsReservedAngle(AId->GetName()) ||
          STE->IsGlobalScope())
        ASTSymbolTable::Instance().TransferAngle(IdR, ASTIntNode::IntBits,
                                                 ASTTypeAngle);
      AN->AddPointElement(IdR, I);
    } else {
      ASTIdentifierRefNode* IdR =
        dynamic_cast<ASTIdentifierRefNode*>(XSTE->GetIdentifier());
      assert(IdR && "ASTSymbolTableEntry without an ASTIdentifierNode!");

      if (IdR->GetSymbolType() != ASTTypeAngle ||
          IdR->GetBits() != ASTIntNode::IntBits) {
        std::stringstream M;
        M << "Angle Element " << IdR->GetName() << " already exists "
          << "in the symbol table with a different bit width and/or type.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(AId), M.str(),
                                                        DiagLevel::ICE);
      }
    }
  }
}

ASTAngleNode*
ASTBuilder::CreateASTAngleNode(const ASTIdentifierNode* Id,
                               ASTAngleType ATy,
                               unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode!");

  ASTAngleNode* AN = nullptr;

  if (NumBits == 0)
    NumBits = ASTAngleNode::AngleBits;

  if (Id->GetBits() == 0U)
    Id->SetBits(NumBits);

  ASTSymbolTableEntry* STE =
    const_cast<ASTSymbolTableEntry*>(Id->GetSymbolTableEntry());
  assert(STE && "Invalid ASTSymbolTableEntry!");

  if (!STE->HasValue()) {
    if (Id->GetName() == "pi" || Id->GetName() == "π") {
      assert(ATy == ASTAngleTypePi && "Invalid ASTAngleType!");

      if (NumBits <= ASTFloatNode::FloatBits)
        AN = new ASTAngleNode(Id, ASTFloatNode::Pi(), ATy, NumBits);
      else if (NumBits > ASTFloatNode::FloatBits &&
               NumBits <= ASTDoubleNode::DoubleBits)
        AN = new ASTAngleNode(Id, ASTDoubleNode::Pi(), ATy, NumBits);
      else
        AN = new ASTAngleNode(Id, ASTMPDecimalNode::Pi(NumBits), ATy,
                              NumBits);
      assert(AN && "Could not create a valid ASTAngleNode!");
    } else if (Id->GetName() == "tau" || Id->GetName() == "τ") {
      assert(ATy == ASTAngleTypeTau && "Invalid ASTAngleType!");

      if (NumBits <= ASTFloatNode::FloatBits)
        AN = new ASTAngleNode(Id, ASTFloatNode::Tau(), ATy, NumBits);
      else if (NumBits > ASTFloatNode::FloatBits &&
               NumBits <= ASTDoubleNode::DoubleBits)
        AN = new ASTAngleNode(Id, ASTDoubleNode::Tau(), ATy, NumBits);
      else
        AN = new ASTAngleNode(Id, ASTMPDecimalNode::Tau(NumBits), ATy,
                              NumBits);
      assert(AN && "Could not create a valid ASTAngleNode!");
    } else if (Id->GetName() == "euler_number" ||
               Id->GetName() == "euler_gamma"  ||
               Id->GetName() == "euler" ||
               Id->GetName() == "ℇ") {
      assert(ATy == ASTAngleTypeEuler && "Invalid ASTAngleType!");

      if (NumBits <= ASTFloatNode::FloatBits)
        AN = new ASTAngleNode(Id, ASTFloatNode::Euler(), ATy, NumBits);
      else if (NumBits > ASTFloatNode::FloatBits &&
               NumBits <= ASTDoubleNode::DoubleBits)
        AN = new ASTAngleNode(Id, ASTDoubleNode::Euler(), ATy, NumBits);
      else
        AN = new ASTAngleNode(Id, ASTMPDecimalNode::Euler(NumBits), ATy,
                              NumBits);
      assert(AN && "Could not create a valid ASTAngleNode!");
    } else {
      AN = new ASTAngleNode(Id, ATy, NumBits);
      assert(AN && "Could not create a valid ASTAngleNode!");
    }

    STE->ResetValue();
    STE->SetValue(new ASTValue<>(AN, ASTTypeAngle), ASTTypeAngle);
    assert(STE->HasValue() && "SymbolTableEntry for Angle has no Value!");
    assert(!STE->GetValue()->HasError() &&
           "Invalid SymbolTableEntry Value for Angle!");
    assert((STE->GetValueType() == ASTTypeAngle) &&
           "Invalid SymbolTableEntry for Angle Node!");

    if (Id->GetName() == "pi" || Id->GetName() == "π" ||
        Id->GetName() == "tau" || Id->GetName() == "τ" ||
        Id->GetName() == "euler" || Id->GetName() == "euler_number" ||
        Id->GetName() == "ε") {
      STE->SetGlobalScope();
      STE->SetDoNotDelete();
      const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
      const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
    }

    if (!ASTAngleNodeMap::Instance().Insert(Id->GetName(), AN)) {
      std::stringstream M;
      M << "Failed to insert ASTAngleNode " << Id->GetName()
        << " in the SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return nullptr;
    }

    if (ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal() ||
        STE->IsGlobalScope())
      ASTSymbolTable::Instance().TransferAngle(Id, ASTAngleNode::AngleBits,
                                               ASTTypeAngle);

    AN->Mangle();
    AN->MangleLiteral();
    MaterializeASTAngleNode(AN);
    return AN;
  }

  assert(STE->HasValue() && "SymbolTableEntry for Generic Angle has no Value!");
  assert(!STE->GetValue()->HasError() &&
         "Invalid SymbolTableEntry Value for Generic Angle!");
  assert((STE->GetValueType() == ASTTypeAngle) &&
         "Invalid SymbolTableEntry for Generic Angle Node!");

  try {
    AN = STE->GetValue()->GetValue<ASTAngleNode*>();
  } catch (const std::bad_any_cast& E) {
    AN = nullptr;
    std::stringstream M;
    M << "Caught " << E.what() << " for Generic ASTAngleNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
  } catch ( ... ) {
    AN = nullptr;
    std::stringstream M;
    M << "Caught an unknown exception for Generic ASTAngleNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
  }

  assert(AN && "Invalid Generic Angle SymbolTableEntry Value!");

  AN->SetBits(NumBits);
  if (!ASTAngleNodeMap::Instance().Insert(Id->GetName(), AN)) {
    std::stringstream M;
    M << "Failed to insert ASTAngleNode " << Id->GetName()
      << " in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
    return nullptr;
  }

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);

  if (ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal()) {
    STE->SetGlobalScope();
    STE->SetDoNotDelete();
    const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
    ASTSymbolTable::Instance().TransferAngle(Id, ASTAngleNode::AngleBits,
                                             ASTTypeAngle);
  }

  AN->Mangle();
  AN->MangleLiteral();
  MaterializeASTAngleNode(AN);
  return AN;
}

ASTAngleNode*
ASTBuilder::CreateASTAngleNode(const ASTIdentifierNode* Id,
                               ASTAngleType ATy,
                               const mpfr_t& MPV,
                               unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE = nullptr;

  if (ASTAngleContextControl::Instance().InOpenContext() &&
      ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id->GetName())) {
    NumBits = ASTAngleNode::AngleBits;
    STE = ASTSymbolTable::Instance().Lookup(Id, NumBits, ASTTypeAngle);
  } else if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
    STE = ASTSymbolTable::Instance().Lookup(Id, NumBits, ASTTypeAngle);
  } else {
    STE = ASTSymbolTable::Instance().Lookup(Id, NumBits, ASTTypeAngle);
  }

  if (STE && STE->HasValue()) {
    ASTAngleNode* SAN = STE->GetValue()->GetValue<ASTAngleNode*>();
    assert(SAN && "Could not obtain a valid ASTAngleNode from the SymbolTable!");
    return SAN->Clone(Id);
  }

  ASTAngleNode* AN = nullptr;

  if (!STE) {
    if (Id->GetName() == "pi" || Id->GetName() == "π") {
      assert(ATy == ASTAngleTypePi && "Invalid ASTAngleType!");

      AN = new ASTAngleNode(Id, NumBits, MPV, ATy);
      assert(AN && "Could not create a valid ASTAngleNode!");
    } else if (Id->GetName() == "tau" || Id->GetName() == "τ") {
      assert(ATy == ASTAngleTypeTau && "Invalid ASTAngleType!");

      AN = new ASTAngleNode(Id, NumBits, MPV, ATy);
      assert(AN && "Could not create a valid ASTAngleNode!");
    } else if (Id->GetName() == "euler_number" ||
               Id->GetName() == "euler" ||
               Id->GetName() == "ℇ") {
      assert(ATy == ASTAngleTypeEuler && "Invalid ASTAngleType!");

      AN = new ASTAngleNode(Id, NumBits, MPV, ATy);
      assert(AN && "Could not create a valid ASTAngleNode!");
    } else {
      AN = new ASTAngleNode(Id, NumBits, MPV, ATy);
      assert(AN && "Could not create a valid ASTAngleNode!");
    }

    AN->SetBits(NumBits);

    if (!ASTSymbolTable::Instance().Insert(Id->GetName(),
                                           const_cast<ASTIdentifierNode*>(Id),
                                           ASTTypeAngle)) {
      std::stringstream M;
      M << "Failed to insert a SymbolTableEntry for ASTAngleNode!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
      return nullptr;
    }

    if (!(STE = ASTSymbolTable::Instance().Lookup(Id, NumBits, ASTTypeAngle))) {
      assert(STE && "Failed to create a Generic Angle SymbolTableEntry!");
      return nullptr;
    }

    STE->ResetValue();
    STE->SetValue(new ASTValue<>(AN, ASTTypeAngle), ASTTypeAngle);
    assert(!STE->GetValue()->HasError() &&
           "Invalid SymbolTableEntry Value for Generic Angle!");
    assert((STE->GetValueType() == ASTTypeAngle) &&
           "Invalid SymbolTableEntry for Generic Angle Node!");

    const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
    const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(true);

    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
      STE->SetGlobalScope();
      STE->SetDoNotDelete();
      const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
    }

    if (!ASTAngleNodeMap::Instance().Insert(Id->GetName(), AN)) {
      std::stringstream M;
      M << "Could not insert AngleNode in AngleMap!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
      return nullptr;
    }

    if (ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal() ||
        STE->IsGlobalScope()) {
      // Do not transfer the arbitrary precision Angle if it's a reserved
      // name.
      if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()))
        ASTSymbolTable::Instance().TransferAngle(Id, ASTAngleNode::AngleBits,
                                                 ASTTypeAngle);
    }

    AN->Mangle();
    AN->MangleLiteral();
    MaterializeASTAngleNode(AN);
    return AN;
  }

  STE->ResetValue();
  AN = new ASTAngleNode(Id, NumBits, MPV, ATy);
  assert(AN && "Could not create a valid ASTAngleNode!");

  AN->SetBits(NumBits);
  STE->SetValue(new ASTValue<>(AN, ASTTypeAngle), ASTTypeAngle);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  if (!ASTAngleNodeMap::Instance().Insert(Id->GetName(), AN)) {
    std::stringstream M;
    M << "Failed to insert ASTAngleNode " << Id->GetName()
      << " in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
    return nullptr;
  }

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(true);

  if (ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal()) {
    STE->SetGlobalScope();
    STE->SetDoNotDelete();
    const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
    // Do not transfer the arbitrary precision Angle if it's a reserved
    // name.
    if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()))
      ASTSymbolTable::Instance().TransferAngle(Id, ASTAngleNode::AngleBits,
                                               ASTTypeAngle);
  }

  AN->Mangle();
  AN->MangleLiteral();
  MaterializeASTAngleNode(AN);
  return AN;
}

ASTAngleNode*
ASTBuilder::CreateASTAngleNode(const ASTIdentifierNode* Id,
                               uint64_t W, uint64_t X, uint64_t Y,
                               uint64_t Z, ASTAngleType ATy,
                               unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
    std::stringstream M;
    M << "A reserved angle cannot be created by this method.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                   DiagLevel::ICE);
    return nullptr;
  }

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, NumBits, Id->GetSymbolType());

  if (!STE) {
    STE = new ASTSymbolTableEntry(const_cast<ASTIdentifierNode*>(Id),
                                  ASTTypeAngle);
    assert(STE && "Could not create a valid ASTSymbolTableEntry!");

    if (!ASTSymbolTable::Instance().Insert(const_cast<ASTIdentifierNode*>(Id),
                                           STE)) {
      std::stringstream M;
      M << "Failure creating a SymbolTable Entry for angle "
        << Id->GetName() << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return nullptr;
    }
  }

  STE = ASTSymbolTable::Instance().Lookup(Id, NumBits, ASTTypeAngle);
  assert(STE && "Could not create a valid ASTSymbolTableEntry!");

  ATy = ASTAngleNode::DetermineAngleType(Id->GetName());
  ASTAngleNode* AN = new ASTAngleNode(Id, W, X, Y, Z, ATy, NumBits);
  assert(AN && "Could not create a valid ASTAngleNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(AN, ASTTypeAngle), ASTTypeAngle);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(STE);
  if (ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal() ||
      STE->IsGlobalScope()) {
    // Do not transfer the arbitrary precision Angle if it's a reserved
    // name.
    if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()))
      ASTSymbolTable::Instance().TransferAngle(Id, ASTAngleNode::AngleBits,
                                               ASTTypeAngle);
  }

  AN->Mangle();
  AN->MangleLiteral();
  MaterializeASTAngleNode(AN);
  return AN;
}

ASTAngleNode*
ASTBuilder::CreateASTAngleNodeFromExpression(const ASTIdentifierNode* Id,
                                             const ASTExpressionNode* E,
                                             ASTAngleType ATy,
                                             unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(E && "Invalid ASTExpressionNode argument!");

  ASTAngleNode* AN = nullptr;
  ASTType ETy = E->GetASTType();

  switch (ETy) {
  case ASTTypeInt:
    AN = new ASTAngleNode(Id, dynamic_cast<const ASTIntNode*>(E),
                          ATy, NumBits);
    assert(AN && "Could not create a valid ASTAngleNode!");
    break;
  case ASTTypeFloat:
    AN = new ASTAngleNode(Id, dynamic_cast<const ASTFloatNode*>(E), ATy,
                          NumBits);
    assert(AN && "Could not create a valid ASTAngleNode!");
    break;
  case ASTTypeDouble:
    AN = new ASTAngleNode(Id, dynamic_cast<const ASTDoubleNode*>(E), ATy,
                          NumBits);
    assert(AN && "Could not create a valid ASTAngleNode!");
    break;
  case ASTTypeLongDouble:
    AN = new ASTAngleNode(Id, dynamic_cast<const ASTLongDoubleNode*>(E), ATy,
                          NumBits);
    assert(AN && "Could not create a valid ASTAngleNode!");
    break;
  case ASTTypeMPInteger:
    AN = new ASTAngleNode(Id, dynamic_cast<const ASTMPIntegerNode*>(E), ATy,
                          NumBits);
    assert(AN && "Could not create a valid ASTAngleNode!");
    break;
  case ASTTypeMPDecimal:
    AN = new ASTAngleNode(Id, dynamic_cast<const ASTMPDecimalNode*>(E), ATy,
                          NumBits);
    assert(AN && "Could not create a valid ASTAngleNode!");
    break;
  case ASTTypeBinaryOp:
    AN = new ASTAngleNode(Id, dynamic_cast<const ASTBinaryOpNode*>(E), ATy,
                          NumBits);
    assert(AN && "Could not create a valid ASTAngleNode!");
    break;
  case ASTTypeUnaryOp:
    AN = new ASTAngleNode(Id, dynamic_cast<const ASTUnaryOpNode*>(E), ATy,
                          NumBits);
    assert(AN && "Could not create a valid ASTAngleNode!");
    break;
  case ASTTypeAngleArray: {
    ASTIdentifierNode* AId = CreateASTIdentifierRValueReference(Id);
    assert(AId &&
           "Could not obtain a valid ASTIdentifierRefNode RValue Reference");

    ASTIdentifierRefNode* IdR = dynamic_cast<ASTIdentifierRefNode*>(AId);
    assert(IdR && "Could not dynamic_cast to an ASTIdentifierRefNode!");

    ASTSymbolTableEntry* ASTE = ASTSymbolTable::Instance().Lookup(AId);
    assert(ASTE && "No SymbolTable Entry for the ASTIdentifierRefNode "
           "RValue Reference!");
    ASTAngleArrayNode* AAN = ASTE->GetValue()->GetValue<ASTAngleArrayNode*>();
    assert(AAN && "Could not retrieve a valid ASTAngleArray Node from "
           "the SymbolTable Entry!");
    AN = AAN->GetElement(IdR->GetIndex());
    assert(AN && "Could not obtain a valid ASTAngleNode from the "
           "ASTAngleArrayNode!");
  }
    break;
  case ASTTypeIdentifier: {
    if (const ASTIdentifierNode* EId = E->GetIdentifier()) {
      ASTScopeController::Instance().CheckUndefined(EId);
      ASTScopeController::Instance().CheckOutOfScope(EId);

      if (EId->IsReference()) {
        if (const ASTIdentifierRefNode* EIdR =
            dynamic_cast<const ASTIdentifierRefNode*>(EId)) {
          ASTSymbolTableEntry* ASTE = ASTSymbolTable::Instance().Lookup(EIdR);
          assert(ASTE && "No SymbolTable Entry for array ASTIdentifierRefNode!");
          ASTAngleNode* AAN = ASTE->GetValue()->GetValue<ASTAngleNode*>();
          assert(AAN && "Could not retrieve a valid ASTAngleNode from "
                        "the SymbolTable Entry!");
          AN = new ASTAngleNode(Id, ATy, NumBits);
          assert(AN && "Could not create a valid ASTAngleNode!");
          AN->SetValue(AAN->GetMPValue());
        }
      } else {
        AN = new ASTAngleNode(Id, ATy, NumBits);
        assert(AN && "Could not create a valid ASTAngleNode!");
        AN->SetExpression(EId);
      }
    }
  }
    break;
  default:
    AN = new ASTAngleNode(Id, E, ATy, NumBits);
    assert(AN && "Could not create a valid ASTAngleNode!");
    break;
  }

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), ASTTypeAngle);
  if (!STE) {
    STE = new ASTSymbolTableEntry(Id, ASTTypeAngle);
    assert(STE && "Could not create a valid SymbolTable Entry!");
  }

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(AN, ASTTypeAngle), ASTTypeAngle);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(STE);

  if (ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal())
    ASTSymbolTable::Instance().TransferAngle(Id, ASTAngleNode::AngleBits,
                                             ASTTypeAngle);
  AN->Mangle();
  AN->MangleLiteral();
  MaterializeASTAngleNode(AN);
  return AN;
}


ASTAngleNode*
ASTBuilder::CreateASTAngleNode(const ASTIdentifierNode* Id,
                               const ASTExpressionNode* E,
                               ASTAngleType ATy,
                               unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode!");
  assert(E && "Invalid ASTExpressionNode!");

  ASTAngleNode* AN = nullptr;
  ASTSymbolTableEntry* STE = nullptr;

  if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName()))
    STE = ASTSymbolTable::Instance().Lookup(Id, ASTAngleNode::AngleBits,
                                            ASTTypeAngle);
  else
    STE = ASTSymbolTable::Instance().Lookup(Id, NumBits, ASTTypeAngle);

  if (!STE) {
    AN = ASTBuilder::Instance().CreateASTAngleNodeFromExpression(Id, E, ATy,
                                                                 NumBits);
    assert(AN && "Could not create a valid ASTAngleNode!");

    if (!ASTSymbolTable::Instance().Insert(Id->GetName(),
                                           const_cast<ASTIdentifierNode*>(Id),
                                           ASTTypeAngle)) {
      assert(0 && "Failed to insert a SymbolTableEntry for ASTAngleNode!");
      std::stringstream M;
      M << "Failed to insert a SymbolTableEntry for ASTAngleNode!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
      return nullptr;
    }

    if (!(STE = ASTSymbolTable::Instance().Lookup(Id))) {
      assert(STE && "Failed to create a Generic Angle SymbolTableEntry!");
      return nullptr;
    }

    STE->ResetValue();
    STE->SetValue(new ASTValue<>(AN, ASTTypeAngle), ASTTypeAngle);
    assert(!STE->GetValue()->HasError() &&
           "Invalid SymbolTableEntry Value for Generic Angle!");
    assert((STE->GetValueType() == ASTTypeAngle) &&
           "Invalid SymbolTableEntry for Generic Angle Node!");

    if (!ASTAngleNodeMap::Instance().Insert(Id->GetName(), AN)) {
      assert(0 && "Could not insert AngleNode in AngleMap!");
      std::stringstream M;
      M << "Could not insert AngleNode in AngleMap!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
      return nullptr;
    }

    const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(STE);

    if (ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal())
      ASTSymbolTable::Instance().TransferAngle(Id, ASTAngleNode::AngleBits,
                                               ASTTypeAngle);
    AN->Mangle();
    AN->MangleLiteral();
    MaterializeASTAngleNode(AN);
    return AN;
  }

  if (!STE->HasValue()) {
    AN = ASTBuilder::Instance().CreateASTAngleNodeFromExpression(Id, E, ATy,
                                                                 NumBits);
    assert(AN && "Could not create a valid ASTAngleNode!");

    STE->ResetValue();
    STE->SetValue(new ASTValue<>(AN, ASTTypeAngle), ASTTypeAngle);
    assert(STE->HasValue() && "SymbolTableEntry for Generic Angle has no Value!");
    assert(!STE->GetValue()->HasError() &&
           "Invalid SymbolTableEntry Value for Generic Angle!");
    assert((STE->GetValueType() == ASTTypeAngle) &&
           "Invalid SymbolTableEntry for Generic Angle Node!");

    if (!ASTAngleNodeMap::Instance().Insert(Id->GetName(), AN)) {
      assert(0 && "Could not insert AngleNode in AngleMap!");
      std::stringstream M;
      M << "Could not insert AngleNode in AngleMap!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
      return nullptr;
    }

    const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(STE);

    if (ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal())
      ASTSymbolTable::Instance().TransferAngle(Id, ASTAngleNode::AngleBits,
                                               ASTTypeAngle);
    AN->Mangle();
    AN->MangleLiteral();
    MaterializeASTAngleNode(AN);
    return AN;
  }

  assert(STE->HasValue() &&
         "SymbolTableEntry for Generic Angle has no Value!");
  assert(!STE->GetValue()->HasError() &&
         "Invalid SymbolTableEntry Value for Generic Angle!");

  if (STE->GetValueType() != ASTTypeAngle) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable "
      << "with a different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeAngle) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  try {
    AN = STE->GetValue()->GetValue<ASTAngleNode*>();
  } catch (const std::bad_any_cast& EXC) {
    AN = nullptr;
    std::stringstream M;
    M << "Caught " << EXC.what() << " when casting to an ASTAngleNode!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
  }

  assert(AN && "Invalid ASTAngleNode SymbolTableEntry Value!");

  if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
    assert(AN->GetBits() >= NumBits &&
           "Invalid bit width for ASTAngleNode!");

    if (!ASTAngleNodeMap::Instance().Insert(Id->GetName(), AN)) {
      std::stringstream M;
      M << "Could not insert AngleNode in AngleMap!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return nullptr;
    }
  }

  const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(STE);

  if (ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal())
    ASTSymbolTable::Instance().TransferAngle(Id, ASTAngleNode::AngleBits,
                                             ASTTypeAngle);
  AN->Mangle();
  AN->MangleLiteral();
  MaterializeASTAngleNode(AN);
  return AN;
}

ASTAngleNode*
ASTBuilder::CreateASTAngleNode(const std::string& Id, ASTAngleType ATy,
                               unsigned NumBits) {
  ASTIdentifierNode* AId =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(Id, NumBits,
                                                         ASTTypeAngle);
  assert(AId && "Invalid Angle ASTIdentifierNode!");

  return ASTBuilder::Instance().CreateASTAngleNode(AId, ATy, NumBits);
}

ASTAngleNode*
ASTBuilder::CloneASTAngleNode(const ASTIdentifierNode* Id,
                              const ASTAngleNode* AN) {
  assert(Id && "Invalid ASTIdentifierNode Argument!");
  assert(AN && "Invalid ASTAngleNode Argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), ASTTypeAngle);
  if (!STE) {
    std::stringstream M;
    M << "ASTAngleNode Identifier " << Id->GetName() << " has no "
      << "SymbolTable Entry!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ASTAngleNode* CAN = AN->Clone(Id);
  assert(CAN && "Could not clone to a valid ASTAngleNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(CAN, ASTTypeAngle), ASTTypeAngle);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  CAN->Mangle();
  CAN->MangleLiteral();
  MaterializeASTAngleNode(CAN);
  return CAN;
}

ASTAngleNode*
ASTBuilder::CloneASTAngleNode(const ASTIdentifierNode* Id,
                              const ASTAngleNode* AN,
                              unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode Argument!");
  assert(AN && "Invalid ASTAngleNode Argument!");
  assert(NumBits != static_cast<unsigned>(~0x0) &&
         "Invalid number of bits for ASTAngleNode Clone!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), ASTTypeAngle);
  if (!STE) {
    std::stringstream M;
    M << "ASTAngleNode Identifier " << Id->GetName() << " has no "
      << "SymbolTable Entry!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  if (NumBits < AN->GetBits()) {
    std::stringstream M;
    M << "Cloning ASTAngleNode to " << NumBits << " bits will result "
      << "in truncation";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  ASTAngleNode* CAN = AN->Clone(Id, NumBits);
  assert(CAN && "Could not clone to a valid ASTAngleNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(CAN, ASTTypeAngle), ASTTypeAngle);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  CAN->Mangle();
  CAN->MangleLiteral();
  MaterializeASTAngleNode(CAN);
  return CAN;
}

ASTCBitNode*
ASTBuilder::CreateASTCBitNode(const ASTIdentifierNode* Id,
                              std::size_t Size,
                              std::size_t Bitmask) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                                ASTTypeBitset))) {
    STE = new ASTSymbolTableEntry(Id, ASTTypeBitset);
    assert(STE && "Could not create a valid ASTSymbolTableEntry!");

    if (!ASTSymbolTable::Instance().Insert(const_cast<ASTIdentifierNode*>(Id),
                                           STE)) {
      std::stringstream M;
      M << "Failure inserting the SymbolTable Entry into the "
        << "SymbolTable!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return nullptr;
    }
  }

  ASTCBitNode* CBN = new ASTCBitNode(Id, Size, Bitmask);
  assert(CBN && "Unable to create an ASTCBitNode!");

  CBN->SetAllTo(Bitmask);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(CBN, ASTTypeBitset), ASTTypeBitset);
  assert(STE->HasValue() && "CBit SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetExpression(CBN);
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);

  return CBN;
}

ASTCBitNode*
ASTBuilder::CreateASTCBitNode(const ASTIdentifierNode* Id,
                              std::size_t Size,
                              const std::string& Bitmask) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                                ASTTypeBitset))) {
    STE = new ASTSymbolTableEntry(Id, ASTTypeBitset);
    assert(STE && "Could not create a valid ASTSymbolTableEntry!");

    if (!ASTSymbolTable::Instance().Insert(const_cast<ASTIdentifierNode*>(Id),
                                           STE)) {
      std::stringstream M;
      M << "Failure inserting the SymbolTable Entry into the "
        << "SymbolTable!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return nullptr;
    }
  }

  std::string BM = ASTStringUtils::Instance().Sanitize(Bitmask);
  ASTCBitNode* CBN = new ASTCBitNode(Id, Size, BM);
  assert(CBN && "Unable to create an ASTCBitNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(CBN, ASTTypeBitset), ASTTypeBitset);
  assert(STE->HasValue() && "CBit SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetExpression(CBN);
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);

  return CBN;
}

ASTCBitNode*
ASTBuilder::CreateASTCBitNode(const ASTIdentifierNode* Id, std::size_t Size,
                              const ASTCastExpressionNode* C) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(C && "Invalid ASTCastExpressionNode argument!");

  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                                ASTTypeBitset))) {
    STE = new ASTSymbolTableEntry(Id, ASTTypeBitset);
    assert(STE && "Could not create a valid ASTSymbolTableEntry!");

    if (!ASTSymbolTable::Instance().Insert(const_cast<ASTIdentifierNode*>(Id),
                                           STE)) {
      std::stringstream M;
      M << "Failure inserting the SymbolTable Entry into the "
        << "SymbolTable!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return nullptr;
    }
  }

  ASTCBitNode* CBN = new ASTCBitNode(Id, Size, C);
  assert(CBN && "Unable to create an ASTCBitNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(CBN, ASTTypeBitset), ASTTypeBitset);
  assert(STE->HasValue() && "CBit SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetExpression(CBN);
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);

  return CBN;
}

ASTCBitNode*
ASTBuilder::CreateASTCBitNode(const ASTIdentifierNode* Id, std::size_t Size,
                              const ASTImplicitConversionNode* IC) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IC && "Invalid ASTImplicitConversionNode argument!");

  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                                ASTTypeBitset))) {
    STE = new ASTSymbolTableEntry(Id, ASTTypeBitset);
    assert(STE && "Could not create a valid ASTSymbolTableEntry!");

    if (!ASTSymbolTable::Instance().Insert(const_cast<ASTIdentifierNode*>(Id),
                                           STE)) {
      std::stringstream M;
      M << "Failure inserting the SymbolTable Entry into the "
        << "SymbolTable!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return nullptr;
    }
  }

  ASTCBitNode* CBN = new ASTCBitNode(Id, Size, IC);
  assert(CBN && "Unable to create an ASTCBitNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(CBN, ASTTypeBitset), ASTTypeBitset);
  assert(STE->HasValue() && "CBit SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetExpression(CBN);
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);

  return CBN;
}

ASTCBitNode*
ASTBuilder::CreateASTCBitNode(const ASTIdentifierNode* Id, std::size_t Size,
                              const ASTBinaryOpNode* BOP) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(BOP && "Invalid ASTBinaryOpNode argument!");

  ASTCastExpressionNode* CX = new ASTCastExpressionNode(BOP, ASTTypeBitset,
                                                        Size);
  assert(CX && "Could not create a valid ASTCastExpressionNode!");
  return CreateASTCBitNode(Id, Size, CX);
}

ASTCBitNode*
ASTBuilder::CreateASTCBitNode(const ASTIdentifierNode* Id, std::size_t Size,
                              const ASTUnaryOpNode* UOP) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(UOP && "Invalid ASTUnaryOpNode argument!");

  ASTCastExpressionNode* CX = new ASTCastExpressionNode(UOP, ASTTypeBitset,
                                                        Size);
  assert(CX && "Could not create a valid ASTCastExpressionNode!");
  return CreateASTCBitNode(Id, Size, CX);
}

ASTQubitContainerNode*
ASTBuilder::CreateASTQubitContainerNode(const ASTIdentifierNode* Id,
                                        unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  const std::string& IDS = Id->GetName();
  ASTType ITy = Id->GetSymbolTableEntry()->GetValueType();

  if (Id->GetSymbolType() == ASTTypeGateQubitParam)
    Id->SetBits(1U);

  ASTSymbolTableEntry* STE =
    const_cast<ASTSymbolTableEntry*>(Id->GetSymbolTableEntry());
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(IDS, NumBits, Id->GetSymbolType());
  if (!STE && Id->GetBits() == 0) {
    STE = ASTSymbolTable::Instance().Lookup(IDS, Id->GetBits(), ITy);
    Id->SetBits(NumBits);
  }

  if (!STE) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetIdentifier() != Id) {
    std::stringstream M;
    M << "Inconsistent SymbolTable ASTIdentifierNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
    return nullptr;
  }

  // If the Identifier has 0 bits, force re-creation of the
  // Qubit register vector.
  if (Id->GetBits() == 0U && (ITy == ASTTypeQubitContainer ||
                              ITy == ASTTypeQubitContainerAlias))
    STE->ResetValue();

  // Now set the correct number of Qubit register bits.
  if (Id->GetBits() != NumBits)
    Id->SetBits(NumBits);

  ASTSymbolTable::Instance().TransferQubit(Id, NumBits, ITy);

  switch (STE->GetValueType()) {
  case ASTTypeInt:
  case ASTTypeFloat:
  case ASTTypeDouble:
  case ASTTypeQubit:
  case ASTTypeBitset:
  case ASTTypeUndefined:
  case ASTTypeIdentifier:
    STE->ResetValue();
    STE->SetValueType(ASTTypeQubitContainer);
    break;
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias:
  case ASTTypeGateQubitParam:
    break;
  case ASTTypeQubitArray:
    if (Id->IsIndexed() || Id->IsReference()) {
      STE->ResetValue();
      STE->SetValueType(ASTTypeQubitContainer);
    } else {
      std::stringstream M;
      M << "ASTIdentifier " << IDS << " exists in the SymbolTable "
        << "with a different Type (" << PrintTypeEnum(STE->GetValueType())
        << " vs. " << PrintTypeEnum(ASTTypeQubitContainer) << ").";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }
    break;
  default: {
    std::stringstream M;
    M << "ASTIdentifier " << IDS << " exists in the SymbolTable "
      << "with a different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeQubitContainer) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  ASTSymbolTableEntry* QSTE =
    const_cast<ASTSymbolTableEntry*>(Id->GetSymbolTableEntry());

  if (QSTE) {
    if (QSTE->HasValue()) {
      ASTQubitContainerNode* QCN =
        QSTE->GetValue()->GetValue<ASTQubitContainerNode*>();
      assert(QCN &&
             "Invalid ASTQubitContainerNode obtained from the SymbolTable!");

      if (QCN->Size() != Id->GetBits()) {
        std::stringstream M;
        M << "Existing ASTQubitContainerNode does not have the width "
          << "requested (" << QCN->Size() << " vs. " << Id->GetBits()
          << ").";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                     DiagLevel::Warning);
      }

      const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(QSTE);
      return QCN;
    }
  } else {
    QSTE = new ASTSymbolTableEntry(Id, ASTTypeQubitContainer);
    assert(QSTE && "Could not create a valid ASTSymbolTable Entry!");
  }

  ASTQubitContainerNode* QCN = new ASTQubitContainerNode(Id, NumBits);
  assert(QCN && "Unable to create an ASTQubitContainerNode!");

  QSTE->ResetValue();
  QSTE->SetValue(new ASTValue<>(QCN, ASTTypeQubitContainer),
                 ASTTypeQubitContainer);
  assert(QSTE->HasValue() &&
         "ASTQubitContainerNode SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(QSTE);

  std::stringstream QS;
  std::stringstream QSS;

  for (unsigned I = 0; I < NumBits; ++I) {
    QS.clear();
    QS.str("");

    if (IDS[0] == '%' || IDS[0] == '$')
      QS << IDS << ':' << I;
    else
      QS << '%' << IDS << ':' << I;

    ASTIdentifierNode* QId =
      ASTBuilder::Instance().CreateASTIdentifierNode(QS.str(), 1U,
                                                     ASTTypeQubit);
    assert(QId && "Could not create a valid ASTIdentifierNode!");

    ASTSymbolTableEntry* QQSTE =
      ASTSymbolTable::Instance().Lookup(QId, 1U, ASTTypeQubit);
    if (!QQSTE) {
      ASTSymbolTable::Instance().Insert(QId, 1U, ASTTypeQubit);
      QQSTE = ASTSymbolTable::Instance().Lookup(QId, 1U, ASTTypeQubit);
    }

    assert(QQSTE && "Could not obtain or create a valid ASTQubitNode "
                    "SymbolTable Entry!");

    if (QQSTE->HasValue()) {
      ASTQubitNode* QBN = QQSTE->GetValue()->GetValue<ASTQubitNode*>();
      assert(QBN && "Invalid SymbolTable Entry ASTQubitNode Value!");

      QBN->Mangle();
      QCN->AddQubit(QBN);
    } else {
      ASTQubitNode* QBN = ASTBuilder::CreateASTQubitNode(QId, I);
      assert(QBN && "Could not create a valid ASTQubitNode!");

      QQSTE->ResetValue();
      QQSTE->SetValue(new ASTValue<>(QBN, ASTTypeQubit), ASTTypeQubit);
      assert(QQSTE->HasValue() && "ASTQubit SymbolTable Entry has no Value!");

      QBN->Mangle();
      QCN->AddQubit(QBN);
    }

    QSS.str("");
    QSS.clear();
    if (IDS[0] == '%' || IDS[0] == '$')
      QSS << IDS.substr(1);
    else
      QSS << IDS;
    QSS << '[' << I << ']';

    ASTIdentifierRefNode* IdR = new ASTIdentifierRefNode(QSS.str(), QId, 1U);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");

    ASTSymbolTableEntry* XSTE = new ASTSymbolTableEntry(IdR, ASTTypeQubit);
    assert(XSTE && "Could not create a valid ASTSymbolTableEntry!");

    if (!ASTSymbolTable::Instance().Insert(IdR, XSTE)) {
      std::stringstream M;
      M << "Failure inserting qubit " << QSS.str() << " into the SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(QId), M.str(), DiagLevel::Error);
    }
  }

  QCN->Mangle();
  return QCN;
}

ASTQubitContainerNode*
ASTBuilder::CreateASTQubitContainerNode(const ASTIdentifierNode* Id,
                                        const std::vector<ASTQubitNode*>& QV) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, QV.size(), ASTTypeQubitContainer);

  if (!STE && Id->GetBits() == 0) {
    STE = ASTSymbolTable::Instance().Lookup(Id, 0U, ASTTypeQubitContainer);
    Id->SetBits(QV.size());
  }

  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTSymbolTable::Instance().TransferQubit(Id, QV.size(),
                                           ASTTypeQubitContainer);

  ASTQubitContainerNode* QCN = new ASTQubitContainerNode(Id, QV);
  assert(QCN && "Could not create a valid ASTQubitContainerNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QCN, ASTTypeQubitContainer), ASTTypeQubitContainer);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTQubitContainerNode has no Value!");
  assert(STE->GetValueType() == ASTTypeQubitContainer &&
         "Invalid ASTType for ASTQubitContainerNode SymbolTable Entry!");

  std::stringstream QSS;
  unsigned IX = 0;

  for (std::vector<ASTQubitNode*>::const_iterator QI = QV.begin();
       QI != QV.end(); ++QI) {
    QSS.str("");
    QSS.clear();
    QSS << Id->GetName() << '[' << IX++ << ']';
    ASTIdentifierRefNode* IdR = new ASTIdentifierRefNode(QSS.str(), Id, 1U);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");

    ASTSymbolTableEntry* XSTE = new ASTSymbolTableEntry(IdR, ASTTypeQubit);
    assert(XSTE && "Could not create a valid ASTSymbolTableEntry!");

    (*QI)->Mangle();
    XSTE->ResetValue();
    XSTE->SetValue(new ASTValue<>((*QI), ASTTypeQubit), ASTTypeQubit, true);

    if (!ASTSymbolTable::Instance().Insert(IdR, XSTE)) {
      std::stringstream M;
      M << "Failure inserting qubit " << QSS.str() << " into the SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    }
  }

  QCN->Mangle();
  return QCN;
}

ASTQubitContainerNode*
ASTBuilder::CreateASTQubitContainerNode(const ASTIdentifierNode* Id,
                                        unsigned NumBits,
                                        const std::vector<ASTQubitNode*>& QV) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, QV.size(), ASTTypeQubitContainer);

  if (!STE && Id->GetBits() == 0) {
    STE = ASTSymbolTable::Instance().Lookup(Id, 0U, ASTTypeQubitContainer);
    Id->SetBits(NumBits);
  }

  assert(STE && "Could not retrieve a valid ASTSymbolTable Entry!");

  ASTSymbolTable::Instance().TransferQubit(Id, QV.size(),
                                           ASTTypeQubitContainer);

  ASTQubitContainerNode* QCN = new ASTQubitContainerNode(Id, NumBits, QV);
  assert(QCN && "Could not create a valid ASTQubitContainerNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QCN, ASTTypeQubitContainer), ASTTypeQubitContainer);
  assert(STE->HasValue() &&
         "SymbolTable Entry for ASTQubitContainerNode has no Value!");
  assert(STE->GetValueType() == ASTTypeQubitContainer &&
         "Invalid ASTType for ASTQubitContainerNode SymbolTable Entry!");

  std::stringstream QSS;
  unsigned IX = 0;

  for (std::vector<ASTQubitNode*>::const_iterator QI = QV.begin();
       QI != QV.end(); ++QI) {
    QSS.str("");
    QSS.clear();
    QSS << Id->GetName() << '[' << IX++ << ']';
    ASTIdentifierRefNode* IdR = new ASTIdentifierRefNode(QSS.str(), Id, 1U);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");

    ASTSymbolTableEntry* XSTE = new ASTSymbolTableEntry(IdR, ASTTypeQubit);
    assert(XSTE && "Could not create a valid ASTSymbolTableEntry!");

    (*QI)->Mangle();
    XSTE->ResetValue();
    XSTE->SetValue(new ASTValue<>((*QI), ASTTypeQubit), ASTTypeQubit, true);

    if (!ASTSymbolTable::Instance().Insert(IdR, XSTE)) {
      std::stringstream M;
      M << "Failure inserting qubit " << QSS.str() << " into the SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    }
  }

  QCN->Mangle();
  return QCN;
}

ASTQubitContainerAliasNode*
ASTBuilder::CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                             int32_t Index) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTQubitContainerAliasNode* QCAN = new ASTQubitContainerAliasNode(Id, Index);
  assert(QCAN && "Could not create a valid ASTQubitContainerAliasNode!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  switch (STE->GetValueType()) {
  case ASTTypeUndefined:
  case ASTTypeIdentifier:
  case ASTTypeQubitContainerAlias:
    break;
  default: {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeQubitContainerAlias) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QCAN, ASTTypeQubitContainerAlias),
                ASTTypeQubitContainerAlias);
  assert(STE->HasValue() &&
         "ASTQubitContainerAlias has no SymbolTable Value!");

  QCAN->Mangle();
  return QCAN;
}

ASTQubitContainerAliasNode*
ASTBuilder::CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                             const ASTQubitContainerNode* QCN,
                                             int32_t Index) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(QCN && "Invalid ASTQubitContainerNode argument!");

  ASTQubitContainerAliasNode* QCAN = QCN->CreateAlias(Id, Index);
  assert(QCAN && "Could not create a valid ASTQubitContainerAliasNode!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  switch (STE->GetValueType()) {
  case ASTTypeUndefined:
  case ASTTypeIdentifier:
  case ASTTypeQubitContainerAlias:
    break;
  default: {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeQubitContainerAlias) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QCAN, ASTTypeQubitContainerAlias),
                ASTTypeQubitContainerAlias);
  assert(STE->HasValue() &&
         "ASTQubitContainerAlias has no SymbolTable Value!");

  QCAN->Mangle();
  return QCAN;
}

ASTQubitContainerAliasNode*
ASTBuilder::CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                             const ASTQubitContainerAliasNode* QCA,
                                             int32_t Index) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(QCA && "Invalid ASTQubitContainerAliasNode argument!");

  ASTQubitContainerAliasNode* QCAN = QCA->CreateAlias(Id, Index);
  assert(QCAN && "Could not create a valid ASTQubitContainerAliasNode!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  switch (STE->GetValueType()) {
  case ASTTypeUndefined:
  case ASTTypeIdentifier:
  case ASTTypeQubitContainerAlias:
    break;
  default: {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeQubitContainerAlias) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QCAN, ASTTypeQubitContainerAlias),
                ASTTypeQubitContainerAlias);
  assert(STE->HasValue() &&
         "ASTQubitContainerAlias has no SymbolTable Value!");

  QCAN->Mangle();
  return QCAN;
}

ASTQubitContainerAliasNode*
ASTBuilder::CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                             const ASTQubitContainerNode* QCN,
                                             const ASTIntegerList& IL,
                                             char Method) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(QCN && "Invalid ASTQubitContainerNode argument!");

  ASTQubitContainerAliasNode* QCAN = QCN->CreateAlias(Id, IL, Method);
  assert(QCAN && "Could not create a valid ASTQubitContainerAliasNode!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  switch (STE->GetValueType()) {
  case ASTTypeUndefined:
  case ASTTypeIdentifier:
  case ASTTypeQubitContainerAlias:
    break;
  default: {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeQubitContainerAlias) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QCAN, ASTTypeQubitContainerAlias),
                ASTTypeQubitContainerAlias);
  assert(STE->HasValue() &&
         "ASTQubitContainerAlias has no SymbolTable Value!");

  QCAN->Mangle();
  return QCAN;
}

ASTQubitContainerAliasNode*
ASTBuilder::CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                             const ASTQubitContainerAliasNode* QCA,
                                             const ASTIntegerList& IL,
                                             char Method) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(QCA && "Invalid ASTQubitContainerAliasNode argument!");

  ASTQubitContainerAliasNode* QCAN = QCA->CreateAlias(Id, IL, Method);
  assert(QCAN && "Could not create a valid ASTQubitContainerAliasNode!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  switch (STE->GetValueType()) {
  case ASTTypeUndefined:
  case ASTTypeIdentifier:
  case ASTTypeQubitContainerAlias:
    break;
  default: {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeQubitContainerAlias) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QCAN, ASTTypeQubitContainerAlias),
                ASTTypeQubitContainerAlias);
  assert(STE->HasValue() &&
         "ASTQubitContainerAlias has no SymbolTable Value!");

  QCAN->Mangle();
  return QCAN;
}

ASTQubitContainerAliasNode*
ASTBuilder::CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                             const ASTQubitContainerNode* QCN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(QCN && "Invalid ASTQubitContainerNode argument!");

  ASTQubitContainerAliasNode* QCAN = QCN->CreateAlias(Id);
  assert(QCAN && "Could not create a valid ASTQubitContainerAliasNode!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  switch (STE->GetValueType()) {
  case ASTTypeUndefined:
  case ASTTypeIdentifier:
  case ASTTypeQubitContainerAlias:
    break;
  default: {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeQubitContainerAlias) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QCAN, ASTTypeQubitContainerAlias),
                ASTTypeQubitContainerAlias);
  assert(STE->HasValue() &&
         "ASTQubitContainerAlias has no SymbolTable Value!");

  QCAN->Mangle();
  return QCAN;
}

ASTQubitContainerAliasNode*
ASTBuilder::CreateASTQubitContainerAliasNode(const ASTIdentifierNode* Id,
                                             const ASTQubitContainerAliasNode* QCA) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(QCA && "Invalid ASTQubitContainerAliasNode argument!");

  ASTQubitContainerAliasNode* QCAN = QCA->CreateAlias(Id);
  assert(QCAN && "Could not create a valid ASTQubitContainerAliasNode!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  switch (STE->GetValueType()) {
  case ASTTypeUndefined:
  case ASTTypeIdentifier:
  case ASTTypeQubitContainerAlias:
    break;
  default: {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeQubitContainerAlias) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QCAN, ASTTypeQubitContainerAlias),
                ASTTypeQubitContainerAlias);
  assert(STE->HasValue() &&
         "ASTQubitContainerAlias has no SymbolTable Value!");

  QCAN->Mangle();
  return QCAN;
}

ASTQRegNode*
ASTBuilder::CreateASTQRegNode(const ASTIdentifierNode* Id,
                              unsigned NumQubits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE = nullptr;
  const std::string& IDS = Id->GetName();

  if (!(STE = ASTSymbolTable::Instance().Lookup(IDS))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeQReg &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeQReg) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTQRegNode* QRN = new ASTQRegNode(Id, NumQubits);
  assert(QRN && "Unable to create an ASTQRegNode!");

  std::stringstream QS;
  for (unsigned I = 0; I < NumQubits; ++I) {
    const std::string& QN = Id->GetName();
    QS.str("");

    if (QN[0] == '%' || QN[0] == '$')
      QS << QN.c_str() << ':' << I;
    else
      QS << '%' << Id->GetName() << ':' << I;

    ASTIdentifierNode* QId =
      ASTBuilder::Instance().FindOrCreateASTIdentifierNode(QS.str(), 1,
                                                           ASTTypeQubit);
    assert(QId && "Could not create a valid Qubit ASTIdentifierNode!");

    ASTSymbolTableEntry* QSTE = ASTSymbolTable::Instance().Lookup(QId);
    assert(QSTE && "ASTQubitNode Identifier has no SymbolTableEntry!");

    ASTQubitNode* QBN = ASTBuilder::Instance().CreateASTQubitNode(QId, I);
    assert(QBN && "Could not create a valid ASTQubitNode!");

    ASTQubitNodeBuilder::Instance().Append(QBN);
    QRN->AddQubit(QBN);
  }

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QRN, ASTTypeQReg), ASTTypeQReg);
  assert(STE->HasValue() && "SymbolTable Entry for ASTQRegNode has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return QRN;
}

ASTCRegNode*
ASTBuilder::CreateASTCRegNode(const ASTIdentifierNode* Id,
                              unsigned NumBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE = nullptr;
  const std::string& IDS = Id->GetName();

  if (!(STE = ASTSymbolTable::Instance().Lookup(IDS))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeCReg &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeCReg) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTCRegNode* CR = new ASTCRegNode(Id, NumBits);
  assert(CR && "Could not create a valid ASTCRegNode!");

  Id->SetBits(NumBits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(CR, ASTTypeCReg), ASTTypeCReg);
  assert(STE->HasValue() && "ASTCReg SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return CR;
}

ASTCRegNode*
ASTBuilder::CreateASTCRegNode(const ASTIdentifierNode* Id,
                              const std::vector<unsigned>& BV) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE = nullptr;
  const std::string& IDS = Id->GetName();

  if (!(STE = ASTSymbolTable::Instance().Lookup(IDS))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeCReg &&
      STE->GetValueType() != ASTTypeUndefined) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeCReg) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTCRegNode* CR = new ASTCRegNode(Id, BV);
  assert(CR && "Could not create a valid ASTCRegNode!");

  Id->SetBits(BV.size());
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(CR, ASTTypeCReg), ASTTypeCReg);
  assert(STE->HasValue() && "ASTCReg SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return CR;
}

ASTQubitNode*
ASTBuilder::CreateASTQubitNode(const ASTIdentifierNode* Id, unsigned Index) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE = nullptr;
  const std::string& IDS = Id->GetName();

  if (!(STE = ASTSymbolTable::Instance().Lookup(IDS))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeQubit &&
      STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeIdentifier) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeQubit) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTable::Instance().TransferQubit(Id, 1, ASTTypeQubit);

  ASTQubitNode* QN = new ASTQubitNode(Id, Index);
  assert(QN && "Unable to create an ASTQubitNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QN, ASTTypeQubit), ASTTypeQubit);
  assert(STE->HasValue() && "ASTQubit SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return QN;
}

ASTResultNode*
ASTBuilder::CreateASTResultNode(const ASTIdentifierNode* Id, ASTType RTy,
                                unsigned NumBits,
                                ASTSignbit SB) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), Id->GetSymbolType());
  assert(STE && "Could not obtain a valid SymbolTable Entry!");

  ASTResultNode* RN = nullptr;

  switch (RTy) {
  case ASTTypeAngle: {
    ASTAngleNode* AN = new ASTAngleNode(&ASTIdentifierNode::Angle,
                                        ASTAngleTypeGeneric,
                                        NumBits == 0 ?
                                        ASTAngleNode::AngleBits : NumBits);
    assert(AN && "Could not create a valid ASTAngleNode!");
    RN = new ASTResultNode(Id, AN);
    assert(RN && "Could not create a alid ASTResultNode!");
  }
    break;
  case ASTTypeBool: {
    ASTBoolNode* BN = new ASTBoolNode(&ASTIdentifierNode::Bool, false);
    assert(BN && "Could not create a valid ASTBoolNode!");
    RN = new ASTResultNode(Id, BN);
    assert(RN && "Could not create a valid ASTResultNode!");
  }
    break;
  case ASTTypeBitset: {
    ASTCBitNode* CN = new ASTCBitNode(&ASTIdentifierNode::Bitset, NumBits);
    assert(CN && "Could not create a valid ASTCBitNode!");
    RN = new ASTResultNode(Id, CN);
    assert(RN && "Could not create a valid ASTResultNode!");
  }
    break;
  case ASTTypeDouble: {
    ASTDoubleNode* DN = new ASTDoubleNode(&ASTIdentifierNode::Double, 0.0);
    assert(DN && "Could not create a valid ASTDoubleNode!");
    RN = new ASTResultNode(Id, DN);
    assert(RN && "Could not create a valid ASTResultNode!");
  }
    break;
  case ASTTypeFloat: {
    ASTFloatNode* FN = new ASTFloatNode(&ASTIdentifierNode::Float, 0.0f);
    assert(FN && "Could not create a valid ASTFloatNode!");
    RN = new ASTResultNode(Id, FN);
    assert(RN && "Could not create a valid ASTResultNode!");
  }
    break;
  case ASTTypeInt: {
    ASTIntNode* IN =
      new ASTIntNode(&ASTIdentifierNode::Int, SB == ASTSignbit::Signed ?
                                              int32_t(0) :
                                              uint32_t(0U));
    assert(IN && "Could not create a valid ASTIntNode!");
    RN = new ASTResultNode(Id, IN);
    assert(RN && "Could not create a valid ASTResultNode!");
  }
    break;
  case ASTTypeVoid: {
    ASTVoidNode* VN = new ASTVoidNode(nullptr);
    assert(VN && "Could not create a valid ASTVoidNode!");
    RN = new ASTResultNode(Id, VN);
    assert(RN && "Could not create a valid ASTResultNode!");
  }
    break;
  case ASTTypeMPInteger: {
    ASTMPIntegerNode* MPI =
      new ASTMPIntegerNode(&ASTIdentifierNode::MPInt, SB,
                           NumBits == 0 ?
                           ASTMPIntegerNode::DefaultBits :
                           NumBits);
    assert(MPI && "Could not create a valid ASTMPIntegerNode!");
    RN = new ASTResultNode(Id, MPI);
    assert(RN && "Could not create a valid ASTResultNode!");
  }
    break;
  case ASTTypeMPDecimal: {
    ASTMPDecimalNode* MPD =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec,
                           NumBits == 0 ?
                           ASTMPDecimalNode::DefaultBits :
                           NumBits);
    assert(MPD && "Could not create a valid ASTMPDecimalNode!");
    RN = new ASTResultNode(Id, MPD);
    assert(RN && "Could not create a valid ASTResultNode!");
  }
    break;
  case ASTTypeMPComplex: {
    ASTMPComplexNode* MPC =
      new ASTMPComplexNode(&ASTIdentifierNode::MPComplex,
                           NumBits == 0 ?
                           ASTMPComplexNode::DefaultBits :
                           NumBits);
    assert(MPC && "Could not create a valid ASTMPComplexNode!");
    RN = new ASTResultNode(Id, MPC);
    assert(RN && "Could not create a valid ASTResultNode!");
  }
    break;
  case ASTTypeComplexList: {
    ASTMPComplexList* MPCL = new ASTMPComplexList();
    assert(MPCL && "Could not create a valid ASTMPComplexList!");
    RN = new ASTResultNode(Id, MPCL);
    assert(RN && "Could not create a valid ASTResultNode!");
  }
    break;
  case ASTTypeOpenPulseWaveform: {
    ASTMPComplexList MPCL;
    OpenPulse::ASTOpenPulseWaveformNode* WFN =
      new OpenPulse::ASTOpenPulseWaveformNode(&ASTIdentifierNode::Waveform, MPCL);
    assert(WFN && "Could not create a valid OpenPulse ASTWaveformNode!");
    RN = new ASTResultNode(Id, WFN);
    assert(RN && "Could not create a valid ASTResultNode!");
  }
    break;
  case ASTTypeOpenPulseFrame: {
    // FIXME: IMPLEMENT.
  }
    break;
  default: {
    std::stringstream M;
    M << "Impossible type " << PrintTypeEnum(RTy) << " for function result.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                   DiagLevel::Error);
  }
    break;
  }

  if (RN) {
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(RN, ASTTypeResult), ASTTypeResult);
  }

  return RN;
}

ASTFunctionDefinitionNode*
ASTBuilder::CreateASTFunctionDefinition(const ASTIdentifierNode* Id,
                                        const ASTDeclarationList& PDL,
                                        const ASTStatementList& SL,
                                        ASTResultNode *RES,
                                        bool IsDefinition) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), Id->GetSymbolType());
  if (!STE) {
    std::stringstream M;
    M << "Failed to locate '" << Id->GetName() << "' ASTIdentifierNode "
      << "in the SymbolTable!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeFunction) {
    std::stringstream M;
    M << "Symbol " << Id->GetName() << " already exists in the "
      << "SymbolTable with a different Type ("
      << QASM::PrintTypeEnum(STE->GetValueType()) << " vs. "
      << PrintTypeEnum(ASTTypeFunction) << ")!" << std::endl;
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return nullptr;
  }

  STE->ResetValue();
  ASTFunctionDefinitionNode* FDN =
    new ASTFunctionDefinitionNode(Id, PDL, SL, RES, IsDefinition);
  assert(FDN && "Could not create an ASTFunctionDeclarationNode!");

  if (ASTTypeSystemBuilder::Instance().IsBuiltinFunction(Id->GetName())) {
    const_cast<ASTDeclarationList&>(PDL).CreateBuiltinParameterSymbols(Id);
  }

  STE->SetValue(new ASTValue<>(FDN, ASTTypeFunction), ASTTypeFunction);
  assert(STE->HasValue() && "Invalid FunctionDefinition Value for the "
         "ASTSymbolTable Entry!");

  STE->SetValueType(ASTTypeFunction);
  STE->SetGlobalScope();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(true);

  if (!ASTFunctionDefinitionBuilder::Instance().Insert(Id->GetName(), FDN)) {
    STE->ResetValue();
    std::stringstream M;
    M << "Symbol " << Id->GetName() << " already exists in "
      << "the SymbolTable!" << std::endl;
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  Id->SetBits(ASTFunctionDefinitionNode::FunctionBits);
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  return FDN;
}

ASTFunctionDeclarationNode*
ASTBuilder::CreateASTFunctionDeclaration(const ASTIdentifierNode* Id,
                                         const ASTFunctionDefinitionNode* FN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(FN && "Invalid FunctionDefinitionNode argument!");

  ASTFunctionDeclarationNode* DN = new ASTFunctionDeclarationNode(Id, FN);
  assert(DN && "Could not create a valid ASTFunctionDeclarationNode!");

  ASTDeclarationBuilder::Instance().Append(DN);
  ASTFunctionDeclarationMap::Instance().Insert(FN->GetName(), DN);
  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  return DN;
}

ASTKernelNode*
ASTBuilder::CreateASTKernelNode(const ASTIdentifierNode* Id,
                                const ASTDeclarationList& DL,
                                const ASTStatementList& SL,
                                ASTResultNode *RES) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(IDS))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeKernel) {
    std::stringstream M;
    M << "Symbol " << IDS << " already exists in the SymbolTable "
      << "with a different Type (" << QASM::PrintTypeEnum(STE->GetValueType())
      << " vs. " << QASM::PrintTypeEnum(ASTTypeKernel) << ")!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  Id->SetBits(ASTKernelNode::KernelBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeKernel);
  STE->SetGlobalScope();

  ASTKernelNode* KN = new ASTKernelNode(Id, DL, SL, RES);
  assert(KN && "Could not create a valid ASTKernelNode!");

  STE->SetValue(new ASTValue<>(KN, ASTTypeKernel), ASTTypeKernel);
  assert(STE->HasValue() && "Invalid KernelNode Value for the "
         "ASTSymbolTable Entry!");

  if (!ASTKernelBuilder::Instance().Insert(IDS, KN)) {
    STE->ResetValue();
    std::stringstream M;
    M << "Kernel Symbol " << IDS << " already exists in the SymbolTable!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return KN;
}

ASTKernelDeclarationNode*
ASTBuilder::CreateASTKernelDeclaration(const ASTIdentifierNode* Id,
                                       const ASTKernelNode* KN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(KN && "Invalid ASTKernelNode argument!");

  ASTKernelDeclarationNode* KDN = new ASTKernelDeclarationNode(Id, KN);
  assert(KDN && "Could not create a valid ASTKernelDeclarationNode!");

  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  ASTDeclarationBuilder::Instance().Append(KDN);
  return KDN;
}

ASTBinaryOpNode*
ASTBuilder::CreateASTArithNegateBinaryOpNode(const ASTIdentifierNode* Id,
                                             const std::string& BON) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
    ASTAngleNode* AN = ASTAngleNodeMap::Instance().Find(Id->GetName());
    if (!AN) {
      std::stringstream M;
      M << "Could not obtain a valid reserved angle.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    }

    ASTIntNode* INT = new ASTIntNode(ASTIdentifierNode::Int.Clone(),
                                     int32_t(-1), ASTTypeInt);
    assert(INT && "Failed to create an ASTIntNode!");

    INT->Mangle();
    INT->MangleLiteral();
    ASTBinaryOpNode* RBO = CreateASTBinaryOpNode(BON, AN, INT,
                                                 ASTOpTypeMul, true);
    assert(RBO && "Could not create an ASTBinaryOpNode!");

    ASTScopeController::Instance().SetLocalScope(RBO->GetIdentifier());
    return RBO;
  } else if (ASTAngleContextControl::Instance().InOpenContext() &&
             ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id->GetName())) {
    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(Id, ASTAngleNode::AngleBits,
                                        ASTTypeAngle);
    if (STE) {
      ASTIntNode* INT =
        new ASTIntNode(ASTIdentifierNode::Int.Clone(), int32_t(-1), ASTTypeInt);
      assert(INT && "Failed to create an ASTIntNode!");

      INT->Mangle();
      INT->MangleLiteral();

      if (STE->HasValue()) {
        if (ASTAngleNode* AN = STE->GetValue()->GetValue<ASTAngleNode*>()) {
          ASTBinaryOpNode* RBO = CreateASTBinaryOpNode(BON, AN, INT,
                                                       ASTOpTypeMul, true);
          assert(RBO && "Could not create an ASTBinaryOpNode!");

          RBO->AddParens();
          RBO->Mangle();
          ASTScopeController::Instance().SetLocalScope(RBO->GetIdentifier());
          return RBO;
        }
      }
    }
  }

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                      Id->GetSymbolTableEntry()->GetValueType());
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTOperatorNode* OPX = Id->HasExpression() ?
    ASTBuilder::Instance().CreateASTOperatorNode(Id->GetExpression(), ASTOpTypeNegative) :
    ASTBuilder::Instance().CreateASTOperatorNode(Id, ASTOpTypeNegative);
  assert(OPX && "Failed to create a valid ASTOperatorNode!");

  ASTBinaryOpNode* RBO = nullptr;

  switch (STE->GetValueType()) {
  case ASTTypeInt: {
    ASTIntNode* INT = new ASTIntNode(ASTIdentifierNode::Int.Clone(),
                                     int32_t(-1), ASTTypeInt);
    assert(INT && "Failed to create an ASTIntNode!");

    INT->Mangle();
    INT->MangleLiteral();
    RBO = CreateASTBinaryOpNode(BON, OPX, INT, ASTOpTypeMul, true);
    assert(RBO && "Could not create an ASTBinaryOpNode!");
  }
    break;
  case ASTTypeFloat: {
    ASTFloatNode* FLT = new ASTFloatNode(ASTIdentifierNode::Float.Clone(),
                                         -1.0f);
    assert(FLT && "Failed to create an ASTFloatNode!");

    FLT->Mangle();
    FLT->MangleLiteral();

    RBO = CreateASTBinaryOpNode(BON, OPX, FLT, ASTOpTypeMul, true);
    assert(RBO && "Could not create an ASTBinaryOpNode!");
  }
    break;
  case ASTTypeDouble: {
    ASTDoubleNode* DBL = new ASTDoubleNode(ASTIdentifierNode::Double.Clone(),
                                           -1.0);
    assert(DBL && "Failed to create an ASTDoubleNode!");

    DBL->Mangle();
    DBL->MangleLiteral();

    RBO = CreateASTBinaryOpNode(BON, OPX, DBL, ASTOpTypeMul, true);
    assert(RBO && "Could not create an ASTBinaryOpNode!");
  }
    break;
  case ASTTypePiAngle:
  case ASTTypeTauAngle:
  case ASTTypeEulerAngle:
  case ASTTypeLambdaAngle:
  case ASTTypePhiAngle:
  case ASTTypeThetaAngle:
  case ASTTypeAngle: {
    ASTMPDecimalNode* MPD =
      new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                           Id->GetBits(), "-1.0");
    assert(MPD && "Failed to create an ASTMPDecimalNode!");

    MPD->Mangle();
    MPD->MangleLiteral();

    RBO = CreateASTBinaryOpNode(BON, OPX, MPD, ASTOpTypeMul, true);
    assert(RBO && "Could not create an ASTBinaryOpNode!");
  }
    break;
  case ASTTypeMPInteger: {
    ASTMPIntegerNode* MPI =
      new ASTMPIntegerNode(ASTIdentifierNode::MPInt.Clone(),
                           Signed, Id->GetBits(), "-1.0");
    assert(MPI && "Failed to create an ASTMPIntegerNode!");

    MPI->Mangle();
    MPI->MangleLiteral();

    RBO = CreateASTBinaryOpNode(BON, OPX, MPI, ASTOpTypeMul, true);
    assert(RBO && "Could not create an ASTBinaryOpNode!");
  }
    break;
  case ASTTypeMPDecimal: {
    ASTMPDecimalNode* MPD =
      new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                           Id->GetBits(), "-1.0");
    assert(MPD && "Failed to create an ASTMPDecimalNode!");

    MPD->Mangle();
    MPD->MangleLiteral();

    RBO = CreateASTBinaryOpNode(BON, OPX, MPD, ASTOpTypeMul, true);
    assert(RBO && "Could not create an ASTBinaryOpNode!");
  }
    break;
  default: {
    ASTIntNode* INT = new ASTIntNode(ASTIdentifierNode::Int.Clone(),
                                     int32_t(-1), ASTTypeInt);
    assert(INT && "Failed to create an ASTIntNode!");

    INT->Mangle();
    INT->MangleLiteral();

    RBO = CreateASTBinaryOpNode(BON, OPX, INT, ASTOpTypeMul, true);
    assert(RBO && "Could not create an ASTBinaryOpNode!");
  }
    break;
  }

  if (!RBO) {
    std::stringstream M;
    M << "Could not determine the RHS ASTType for creating a "
      << "Negate ASTBinaryOpNode Multiplication!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  return RBO;
}

ASTReturnStatementNode*
ASTBuilder::CreateASTReturnStatementNode(const ASTIdentifierNode* Id,
                                         ASTIdentifierNode* RId) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(RId && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    QASM::ASTSymbolTable::Instance().Lookup(Id->GetName(), Id->GetBits(),
                                            Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid Return SymbolTable Entry!");
  assert(STE->GetValueType() == ASTTypeReturn &&
         "Invalid ASTType for Return SymbolTable Entry!");

  ASTReturnStatementNode* RN = new ASTReturnStatementNode(Id, RId);
  assert(RN && "Could not create a valid ASTReturnStatementNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(RN, ASTTypeReturn), ASTTypeReturn);
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return RN;
}

ASTReturnStatementNode*
ASTBuilder::CreateASTReturnStatementNode(const ASTIdentifierNode* Id,
                                         ASTExpressionNode* EN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(EN && "Invalid ASTExpressionNode argument!");

  ASTSymbolTableEntry* STE =
    QASM::ASTSymbolTable::Instance().Lookup(Id->GetName(), Id->GetBits(),
                                            Id->GetSymbolType());
  assert(STE && "Could not retrieve a valid Return SymbolTable Entry!");
  assert(STE->GetValueType() == ASTTypeReturn &&
         "Invalid ASTType for Return SymbolTable Entry!");

  ASTType ETy = EN->GetASTType();
  ASTReturnStatementNode* RN = nullptr;

  switch (ETy) {
  case ASTTypeBool:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTBoolNode*>(EN));
    break;
  case ASTTypeInt:
  case ASTTypeUInt:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTIntNode*>(EN));
    break;
  case ASTTypeFloat:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTFloatNode*>(EN));
    break;
  case ASTTypeDouble:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTDoubleNode*>(EN));
    break;
  case ASTTypeLongDouble:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTLongDoubleNode*>(EN));
    break;
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTMPIntegerNode*>(EN));
    break;
  case ASTTypeMPComplex:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTMPComplexNode*>(EN));
    break;
  case ASTTypeMPDecimal:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTMPDecimalNode*>(EN));
    break;
  case ASTTypeBinaryOp:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTBinaryOpNode*>(EN));
    break;
  case ASTTypeUnaryOp:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTUnaryOpNode*>(EN));
    break;
  case ASTTypeAngle:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTAngleNode*>(EN));
    break;
  case ASTTypeBitset:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTCBitNode*>(EN));
    break;
  case ASTTypeMeasure:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTMeasureNode*>(EN));
    break;
  case ASTTypeQubit:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTQubitNode*>(EN));
    break;
  case ASTTypeQubitContainer:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTQubitContainerNode*>(EN));
    break;
  case ASTTypeVoid:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTVoidNode*>(EN));
    break;
  case ASTTypeCast:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTCastExpressionNode*>(EN));
    break;
  default:
    RN = new ASTReturnStatementNode(Id, EN);
    break;
  }

  assert(RN && "Could not create a valid ASTReturnStatementNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(RN, ASTTypeReturn), ASTTypeReturn);
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return RN;
}

ASTReturnStatementNode*
ASTBuilder::CreateASTReturnStatementNode(const ASTIdentifierNode* Id,
                                         ASTFunctionCallStatementNode* FSN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(FSN && "Invalid ASTFunctionCallStatementNode argument!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id->GetName());
  assert(STE && "Could not retrieve a valid Return SymbolTable Entry!");
  assert(STE->GetValueType() == ASTTypeReturn &&
         "Invalid ASTType for Return SymbolTable Entry!");

  ASTReturnStatementNode* RN = new ASTReturnStatementNode(Id, FSN);
  assert(RN && "Could not create a valid ASTReturnStatementNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(RN, ASTTypeReturn), ASTTypeReturn);
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return RN;
}

ASTReturnStatementNode*
ASTBuilder::CreateASTReturnStatementNode(const ASTIdentifierNode* Id,
                                         ASTMeasureNode* MN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(MN && "Invalid ASTMeasureNode argument!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id->GetName());
  assert(STE && "Could not retrieve a valid Return SymbolTable Entry!");
  assert(STE->GetValueType() == ASTTypeReturn &&
         "Invalid ASTType for Return SymbolTable Entry!");

  ASTReturnStatementNode* RN = new ASTReturnStatementNode(Id, MN);
  assert(RN && "Could not create a valid ASTReturnStatementNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(RN, ASTTypeReturn), ASTTypeReturn);
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return RN;
}

ASTReturnStatementNode*
ASTBuilder::CreateASTReturnStatementNode(const ASTIdentifierNode* Id,
                                         ASTStatementNode* SN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(SN && "Invalid ASTStatementNode argument!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id->GetName());
  assert(STE && "Could not retrieve a valid Return SymbolTable Entry!");
  assert(STE->GetValueType() == ASTTypeReturn &&
         "Invalid ASTType for Return SymbolTable Entry!");

  ASTExpressionNode* EN = nullptr;
  ASTType STy = SN->GetASTType();
  ASTReturnStatementNode* RN = nullptr;

  switch (STy) {
  case ASTTypeInt:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTIntNode*>(SN));
    break;
  case ASTTypeFloat:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTFloatNode*>(SN));
    break;
  case ASTTypeDouble:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTDoubleNode*>(SN));
    break;
  case ASTTypeLongDouble:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTLongDoubleNode*>(SN));
    break;
  case ASTTypeMPInteger:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTMPIntegerNode*>(SN));
    break;
  case ASTTypeMPDecimal:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTMPDecimalNode*>(SN));
    break;
  case ASTTypeBinaryOp:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTBinaryOpNode*>(SN));
    break;
  case ASTTypeUnaryOp:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTUnaryOpNode*>(SN));
    break;
  case ASTTypeAngle:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTAngleNode*>(SN));
    break;
  case ASTTypeBitset:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTCBitNode*>(SN));
    break;
  case ASTTypeMeasure:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTMeasureNode*>(SN));
    break;
  case ASTTypeQubit:
    RN = new ASTReturnStatementNode(Id, dynamic_cast<ASTQubitNode*>(SN));
    break;
  default: {
    if (STy == ASTTypeStatement) {
      if (SN->IsExpression()) {
        EN = SN->GetExpression();
        assert(EN && "Invalid ASTExpressionNode contained in the "
               "ASTStatementNode!");

        // Do NOT use the ASTExpressionNode Identifier here!
        RN = ASTBuilder::CreateASTReturnStatementNode(Id, EN);
      }
    } else {
      RN = new ASTReturnStatementNode(Id, SN);
    }
  }
    break;
  }

  assert(RN && "Could not create a valid ASTReturnStatementNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(RN, ASTTypeReturn), ASTTypeReturn);
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return RN;
}

ASTReturnStatementNode*
ASTBuilder::CreateASTReturnStatementNode(const ASTIdentifierNode* Id) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    QASM::ASTSymbolTable::Instance().Lookup(Id->GetName());
  assert(STE && "Could not retrieve a valid Return SymbolTable Entry!");
  assert(STE->GetValueType() == ASTTypeReturn &&
         "Invalid ASTType for Return SymbolTable Entry!");

  ASTReturnStatementNode* RN = new ASTReturnStatementNode(Id);
  assert(RN && "Could not create a valid ASTReturnStatementNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(RN, ASTTypeReturn), ASTTypeReturn);
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return RN;
}

ASTPopcountNode*
ASTBuilder::CreateASTPopcountNode(const ASTIdentifierNode* Id,
                                  const ASTIntNode* I) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(I && "Invalid ASTIntNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                      ASTTypePopcountExpr);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");

  ASTPopcountNode* PN = new ASTPopcountNode(Id, I);
  assert(PN && "Could not create a valid ASTPopcountNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(PN, ASTTypePopcountExpr), ASTTypePopcountExpr);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  PN->Mangle();
  return PN;
}

ASTPopcountNode*
ASTBuilder::CreateASTPopcountNode(const ASTIdentifierNode* Id,
                                  const ASTMPIntegerNode* MPI) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(MPI && "Invalid ASTMPIntegerNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                      ASTTypePopcountExpr);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");

  ASTPopcountNode* PN = new ASTPopcountNode(Id, MPI);
  assert(PN && "Could not create a valid ASTPopcountNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(PN, ASTTypePopcountExpr), ASTTypePopcountExpr);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  PN->Mangle();
  return PN;
}

ASTPopcountNode*
ASTBuilder::CreateASTPopcountNode(const ASTIdentifierNode* Id,
                                  const ASTCBitNode* CBI) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(CBI && "Invalid ASTCBitNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                      ASTTypePopcountExpr);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");

  ASTPopcountNode* PN = new ASTPopcountNode(Id, CBI);
  assert(PN && "Could not create a valid ASTPopcountNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(PN, ASTTypePopcountExpr), ASTTypePopcountExpr);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  PN->Mangle();
  return PN;
}

ASTRotateNode*
ASTBuilder::CreateASTRotateNode(const ASTIdentifierNode* Id,
                                const ASTIntNode* I,
                                const ASTIntNode* S,
                                ASTRotationType OpTy) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(I && "Invalid ASTIntNode argument!");
  assert(S && "Invalid ASTIntNode argument!");

  if (I->IsSigned()) {
    std::stringstream M;
    M << "Signed integer types cannot be rotated.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(I), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                      ASTTypeRotateExpr);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");

  ASTRotateNode* RN = new ASTRotateNode(Id, I, S, OpTy);
  assert(RN && "Could not create a valid ASTRotateNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(RN, ASTTypeRotateExpr), ASTTypeRotateExpr);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  RN->Mangle();
  return RN;
}

ASTRotateNode*
ASTBuilder::CreateASTRotateNode(const ASTIdentifierNode* Id,
                                const ASTMPIntegerNode* MPI,
                                const ASTIntNode* S,
                                ASTRotationType OpTy) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(MPI && "Invalid ASTMPIntegerNode argument!");
  assert(S && "Invalid ASTIntNode argument!");

  if (MPI->IsSigned()) {
    std::stringstream M;
    M << "Signed integer types cannot be rotated.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(MPI), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                      ASTTypeRotateExpr);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");

  ASTRotateNode* RN = new ASTRotateNode(Id, MPI, S, OpTy);
  assert(RN && "Could not create a valid ASTRotateNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(RN, ASTTypeRotateExpr), ASTTypeRotateExpr);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  RN->Mangle();
  return RN;
}

ASTRotateNode*
ASTBuilder::CreateASTRotateNode(const ASTIdentifierNode* Id,
                                const ASTCBitNode* CBI,
                                const ASTIntNode* S,
                                ASTRotationType OpTy) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(CBI && "Invalid ASTCBitNode argument!");
  assert(S && "Invalid ASTIntNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                      ASTTypeRotateExpr);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");

  ASTRotateNode* RN = new ASTRotateNode(Id, CBI, S, OpTy);
  assert(RN && "Could not create a valid ASTRotateNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(RN, ASTTypeRotateExpr), ASTTypeRotateExpr);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  RN->Mangle();
  return RN;
}

ASTDefcalNode*
ASTBuilder::CreateASTDefcalNode(const ASTIdentifierNode* Id,
                                const ASTExpressionNodeList& EL,
                                const ASTStatementList& SL,
                                const ASTBoundQubitList* BQL) {
  assert(Id && "Invalid Defcal ASTIdentifierNode argument!");
  assert(BQL && "Invalid Defcal BoundQubitList argument!");

  const std::string& IDS = Id->GetName();
  ASTMapSymbolTableEntry* MSTE =
    ASTSymbolTable::Instance().FindDefcalGroup(IDS);

  if (!MSTE) {
    std::stringstream M;
    M << "Failed to locate defcal group " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  Id->SetDefcalGroupName(IDS);
  ASTSymbolTableEntry* STE =
    const_cast<ASTIdentifierNode*>(Id)->GetSymbolTableEntry();
  if (!STE) {
    std::stringstream M;
    M << "Defcal Identifier has no SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  assert(STE != MSTE && "Defcal SymbolTableEntry points to DefcalGroup!");

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDefcal) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeDefcal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTDefcalGroupNode* DGN =
    ASTBuilder::Instance().ASTBuilder::CreateASTDefcalGroupNode(Id);
  assert(DGN && "Could not obtain a valid ASTDefcalGroupNode!");

  Id->SetBits(ASTDefcalNode::DefcalBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeDefcal);

  ASTDefcalNode* DN = new ASTDefcalNode(Id, EL, SL, BQL);
  assert(DN && "Could not create a valid ASTDefcalNode!");

  DN->Mangle();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDefcal), ASTTypeDefcal);
  assert(STE->HasValue() && "Invalid ASTDefcalNode Value for the "
                            "ASTSymbolTable Entry");

  STE->SetGlobalScope();
  STE->SetDoNotDelete();
  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(true);

  if (!ASTDefcalBuilder::Instance().Insert(Id->GetMangledName(), DN)) {
    std::stringstream M;
    M << "ASTDefcalBuilder insertion failure for defcal "
      << Id->GetName() << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Warning);
  }

  if (!ASTSymbolTable::Instance().InsertDefcal(Id, STE)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
  }

  DGN->AddDefcal(DN);
  return DN;
}

ASTDefcalNode*
ASTBuilder::CreateASTDefcalNode(const ASTIdentifierNode* Id,
                                const std::string& GM,
                                const ASTExpressionNodeList& EL,
                                const ASTStatementList& SL,
                                const ASTBoundQubitList* BQL) {
  assert(Id && "Invalid Defcal ASTIdentifierNode argument!");
  assert(BQL && "Invalid Defcal BoundQubitList argument!");

  const std::string& IDS = Id->GetName();
  ASTMapSymbolTableEntry* MSTE =
    ASTSymbolTable::Instance().FindDefcalGroup(IDS);

  if (!MSTE) {
    std::stringstream M;
    M << "Failed to locate defcal group " << IDS
      << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTableEntry* STE =
    const_cast<ASTIdentifierNode*>(Id)->GetSymbolTableEntry();
  if (!STE) {
    std::stringstream M;
    M << "Defcal Identifier has no SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  assert(STE != MSTE && "Defcal SymbolTableEntry points to DefcalGroup!");

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDefcal) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeDefcal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTDefcalGroupNode* DGN =
    ASTBuilder::Instance().ASTBuilder::CreateASTDefcalGroupNode(Id);
  assert(DGN && "Could not obtain a valid ASTDefcalGroupNode!");

  Id->SetBits(ASTDefcalNode::DefcalBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeDefcal);

  ASTDefcalNode* DN = new ASTDefcalNode(Id, GM, EL, SL, BQL);
  assert(DN && "Could not create a valid ASTDefcalNode!");

  DN->Mangle();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDefcal), ASTTypeDefcal);
  assert(STE->HasValue() && "Invalid ASTDefcalNode Value for the "
                            "ASTSymbolTable Entry");

  STE->SetGlobalScope();
  STE->SetDoNotDelete();
  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(true);

  if (!ASTDefcalBuilder::Instance().Insert(Id->GetMangledName(), DN)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Warning);
  }

  if (!ASTSymbolTable::Instance().InsertDefcal(Id, STE)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
  }

  DGN->AddDefcal(DN);
  return DN;
}

ASTDefcalNode*
ASTBuilder::CreateASTDefcalNode(const ASTIdentifierNode* Id,
                                const ASTStatementList& SL,
                                const ASTMeasureNode* MN) {
  assert(Id && "Invalid Defcal ASTIdentifierNode argument!");
  assert(MN && "Invalid Defcal Target ASTMeasureNode argument!");

  const std::string& IDS = Id->GetName();
  ASTMapSymbolTableEntry* MSTE =
    ASTSymbolTable::Instance().FindDefcalGroup(IDS);

  if (!MSTE) {
    std::stringstream M;
    M << "Failed to locate defcal group " << IDS
      << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTableEntry* STE =
    const_cast<ASTIdentifierNode*>(Id)->GetSymbolTableEntry();
  if (!STE) {
    std::stringstream M;
    M << "Defcal Identifier has no SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  assert(STE != MSTE && "Defcal SymbolTableEntry points to DefcalGroup!");

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDefcal) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeDefcal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTDefcalGroupNode* DGN =
    ASTBuilder::Instance().ASTBuilder::CreateASTDefcalGroupNode(Id);
  assert(DGN && "Could not obtain a valid ASTDefcalGroupNode!");

  Id->SetBits(ASTDefcalNode::DefcalBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeDefcal);

  ASTDefcalNode* DN = new ASTDefcalNode(Id, SL, MN);
  assert(DN && "Could not create a valid ASTDefcalNode!");

  DN->Mangle();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDefcal), ASTTypeDefcal);
  assert(STE->HasValue() && "Invalid ASTDefcalNode Value for the "
                            "ASTSymbolTable Entry");

  STE->SetGlobalScope();
  STE->SetDoNotDelete();
  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(true);

  DN->ToDefcalParamSymbolTable(MN->GetName(),
                               MN->GetIdentifier()->GetSymbolTableEntry());
  ASTSymbolTable::Instance().EraseLocalSymbol(MN->GetIdentifier(),
                                              MN->GetIdentifier()->GetBits(),
                                              MN->GetIdentifier()->GetSymbolType());

  switch (MN->GetResultType()) {
  case ASTTypeBitset: {
    const ASTCBitNode* CBN = MN->GetResult();
    assert(CBN && "Invalid ASTCBitNode defcal measure result!");

    DN->ToDefcalParamSymbolTable(CBN->GetName(),
                                 CBN->GetIdentifier()->GetSymbolTableEntry());
    ASTSymbolTable::Instance().EraseLocalSymbol(CBN->GetIdentifier(),
                                                CBN->GetIdentifier()->GetBits(),
                                                CBN->GetIdentifier()->GetSymbolType());
  }
    break;
  case ASTTypeAngle: {
    const ASTAngleNode* AN = MN->GetAngleResult();
    assert(AN && "Invalid ASTAngleNode defcal measure result!");

    DN->ToDefcalParamSymbolTable(AN->GetName(),
                                 AN->GetIdentifier()->GetSymbolTableEntry());
    ASTSymbolTable::Instance().EraseLocalSymbol(AN->GetIdentifier(),
                                                AN->GetIdentifier()->GetBits(),
                                                AN->GetIdentifier()->GetSymbolType());
  }
    break;
  case ASTTypeMPComplex: {
    const ASTMPComplexNode* MPC = MN->GetComplexResult();
    assert(MPC && "Invalid ASTMPComplexNode defcal measure result!");

    DN->ToDefcalParamSymbolTable(MPC->GetName(),
                                 MPC->GetIdentifier()->GetSymbolTableEntry());
    ASTSymbolTable::Instance().EraseLocalSymbol(MPC->GetIdentifier(),
                                                MPC->GetIdentifier()->GetBits(),
                                                MPC->GetIdentifier()->GetSymbolType());
  }
    break;
  default:
    break;
  }

  if (!ASTDefcalBuilder::Instance().Insert(Id->GetMangledName(), DN)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Warning);
  }

  if (!ASTSymbolTable::Instance().InsertDefcal(Id, STE)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
  }

  DGN->AddDefcal(DN);
  return DN;
}

ASTDefcalNode*
ASTBuilder::CreateASTDefcalNode(const ASTIdentifierNode* Id,
                                const std::string& GM,
                                const ASTStatementList& SL,
                                const ASTMeasureNode* MN) {
  assert(Id && "Invalid Defcal ASTIdentifierNode argument!");
  assert(MN && "Invalid Defcal Target ASTMeasureNode argument!");

  const std::string& IDS = Id->GetName();
  ASTMapSymbolTableEntry* MSTE =
    ASTSymbolTable::Instance().FindDefcalGroup(IDS);

  if (!MSTE) {
    std::stringstream M;
    M << "Failed to locate defcal group " << IDS
      << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTableEntry* STE =
    const_cast<ASTIdentifierNode*>(Id)->GetSymbolTableEntry();
  if (!STE) {
    std::stringstream M;
    M << "Defcal Identifier has no SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  assert(STE != MSTE && "Defcal SymbolTableEntry points to DefcalGroup!");

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDefcal) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeDefcal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTDefcalGroupNode* DGN =
    ASTBuilder::Instance().ASTBuilder::CreateASTDefcalGroupNode(Id);
  assert(DGN && "Could not obtain a valid ASTDefcalGroupNode!");

  Id->SetBits(ASTDefcalNode::DefcalBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeDefcal);

  ASTDefcalNode* DN = new ASTDefcalNode(Id, GM, SL, MN);
  assert(DN && "Could not create a valid ASTDefcalNode!");

  DN->Mangle();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDefcal), ASTTypeDefcal);
  assert(STE->HasValue() && "Invalid ASTDefcalNode Value for the "
                            "ASTSymbolTable Entry");

  STE->SetGlobalScope();
  STE->SetDoNotDelete();
  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(true);

  DN->ToDefcalParamSymbolTable("measure",
                               MN->GetIdentifier()->GetSymbolTableEntry());
  ASTSymbolTable::Instance().EraseLocalSymbol(MN->GetIdentifier(),
                                              MN->GetIdentifier()->GetBits(),
                                              MN->GetIdentifier()->GetSymbolType());

  switch (MN->GetResultType()) {
  case ASTTypeBitset: {
    const ASTCBitNode* CBN = MN->GetResult();
    DN->ToDefcalParamSymbolTable("result",
                                 CBN->GetIdentifier()->GetSymbolTableEntry());
    ASTSymbolTable::Instance().EraseLocalSymbol(CBN->GetIdentifier(),
                                                CBN->GetIdentifier()->GetBits(),
                                                CBN->GetIdentifier()->GetSymbolType());
  }
    break;
  case ASTTypeAngle: {
    const ASTAngleNode* AN = MN->GetAngleResult();
    DN->ToDefcalParamSymbolTable("result",
                                 AN->GetIdentifier()->GetSymbolTableEntry());
    ASTSymbolTable::Instance().EraseLocalSymbol(AN->GetIdentifier(),
                                                AN->GetIdentifier()->GetBits(),
                                                AN->GetIdentifier()->GetSymbolType());
  }
    break;
  case ASTTypeMPComplex: {
    const ASTMPComplexNode* MPC = MN->GetComplexResult();
    DN->ToDefcalParamSymbolTable("result",
                                 MPC->GetIdentifier()->GetSymbolTableEntry());
    ASTSymbolTable::Instance().EraseLocalSymbol(MPC->GetIdentifier(),
                                                MPC->GetIdentifier()->GetBits(),
                                                MPC->GetIdentifier()->GetSymbolType());
  }
    break;
  default:
    break;
  }

  if (!ASTDefcalBuilder::Instance().Insert(Id->GetMangledName(), DN)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Warning);
  }

  if (!ASTSymbolTable::Instance().InsertDefcal(Id, STE)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
  }

  DGN->AddDefcal(DN);
  return DN;
}

ASTDefcalNode*
ASTBuilder::CreateASTDefcalNode(const ASTIdentifierNode* Id,
                                const ASTStatementList& SL,
                                const ASTResetNode* RN) {
  assert(Id && "Invalid Defcal ASTIdentifierNode argument!");
  assert(RN && "Invalid Defcal Target ASTResetNode argument!");

  const std::string& IDS = Id->GetName();
  ASTMapSymbolTableEntry* MSTE =
    ASTSymbolTable::Instance().FindDefcalGroup(IDS);

  if (!MSTE) {
    std::stringstream M;
    M << "Failed to locate defcal group " << IDS
      << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTableEntry* STE =
    const_cast<ASTIdentifierNode*>(Id)->GetSymbolTableEntry();
  if (!STE) {
    std::stringstream M;
    M << "Defcal Identifier has no SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  assert(STE != MSTE && "Defcal SymbolTableEntry points to DefcalGroup!");

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDefcal) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeDefcal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTDefcalGroupNode* DGN =
    ASTBuilder::Instance().ASTBuilder::CreateASTDefcalGroupNode(Id);
  assert(DGN && "Could not obtain a valid ASTDefcalGroupNode!");

  Id->SetBits(ASTDefcalNode::DefcalBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeDefcal);

  ASTDefcalNode* DN = new ASTDefcalNode(Id, SL, RN);
  assert(DN && "Could not create a valid ASTDefcalNode!");

  DN->Mangle();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDefcal), ASTTypeDefcal);
  assert(STE->HasValue() && "Invalid ASTDefcalNode Value for the "
                            "ASTSymbolTable Entry");

  STE->SetGlobalScope();
  STE->SetDoNotDelete();
  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(true);

  DN->ToDefcalParamSymbolTable("reset",
                               RN->GetIdentifier()->GetSymbolTableEntry());
  ASTSymbolTable::Instance().EraseLocalSymbol(RN->GetIdentifier(),
                                              RN->GetIdentifier()->GetBits(),
                                              RN->GetIdentifier()->GetSymbolType());

  if (!ASTDefcalBuilder::Instance().Insert(Id->GetMangledName(), DN)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Warning);
  }

  if (!ASTSymbolTable::Instance().InsertDefcal(Id, STE)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
  }

  DGN->AddDefcal(DN);
  return DN;
}

ASTDefcalNode*
ASTBuilder::CreateASTDefcalNode(const ASTIdentifierNode* Id,
                                const std::string& GM,
                                const ASTStatementList& SL,
                                const ASTResetNode* RN) {
  assert(Id && "Invalid Defcal ASTIdentifierNode argument!");
  assert(RN && "Invalid Defcal Target ASTResetNode argument!");

  const std::string& IDS = Id->GetName();
  ASTMapSymbolTableEntry* MSTE =
    ASTSymbolTable::Instance().FindDefcalGroup(IDS);

  if (!MSTE) {
    std::stringstream M;
    M << "Failed to locate defcal group " << IDS
      << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTableEntry* STE =
    const_cast<ASTIdentifierNode*>(Id)->GetSymbolTableEntry();
  if (!STE) {
    std::stringstream M;
    M << "Defcal Identifier has no SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  assert(STE != MSTE && "Defcal SymbolTableEntry points to DefcalGroup!");

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDefcal) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeDefcal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTDefcalGroupNode* DGN =
    ASTBuilder::Instance().ASTBuilder::CreateASTDefcalGroupNode(Id);
  assert(DGN && "Could not obtain a valid ASTDefcalGroupNode!");

  Id->SetBits(ASTDefcalNode::DefcalBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeDefcal);

  ASTDefcalNode* DN = new ASTDefcalNode(Id, GM, SL, RN);
  assert(DN && "Could not create a valid ASTDefcalNode!");

  DN->Mangle();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDefcal), ASTTypeDefcal);
  assert(STE->HasValue() && "Invalid ASTDefcalNode Value for the "
                            "ASTSymbolTable Entry");

  STE->SetGlobalScope();
  STE->SetDoNotDelete();
  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(true);

  DN->ToDefcalParamSymbolTable("reset",
                               RN->GetIdentifier()->GetSymbolTableEntry());
  ASTSymbolTable::Instance().EraseLocalSymbol(RN->GetIdentifier(),
                                              RN->GetIdentifier()->GetBits(),
                                              RN->GetIdentifier()->GetSymbolType());

  if (!ASTDefcalBuilder::Instance().Insert(Id->GetMangledName(), DN)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Warning);
  }

  if (!ASTSymbolTable::Instance().InsertDefcal(Id, STE)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
  }

  DGN->AddDefcal(DN);
  return DN;
}

ASTDefcalNode*
ASTBuilder::CreateASTDefcalNode(const ASTIdentifierNode* Id,
                                const ASTDelayNode* DN,
                                const ASTStatementList& SL,
                                const ASTBoundQubitList* BQL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DN && "Invalid ASTDelayNode argument!");
  assert(BQL && "Invalid ASTBoundQubitList argument!");

  const std::string& IDS = Id->GetName();
  ASTMapSymbolTableEntry* MSTE = ASTSymbolTable::Instance().FindDefcalGroup(IDS);

  if (!MSTE) {
    std::stringstream M;
    M << "Failed to locate defcal group " << IDS << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTableEntry* STE =
    const_cast<ASTIdentifierNode*>(Id)->GetSymbolTableEntry();
  if (!STE) {
    std::stringstream M;
    M << "Defcal Identifier has no SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return nullptr;
  }

  if (STE == MSTE) {
    std::stringstream M;
    M << "Defcal SymbolTable Entry points to DefcalGroup SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDefcal) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeDefcal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ASTDefcalGroupNode* DGN =
    ASTBuilder::Instance().ASTBuilder::CreateASTDefcalGroupNode(Id);
  assert(DGN && "Could not obtain a valid ASTDefcalGroupNode!");

  ASTDefcalNode* DCN = new ASTDefcalNode(Id, DN, SL, BQL);
  assert(DCN && "Could not create a valid ASTDefcalNode!");

  DCN->ToDefcalParamSymbolTable("delay",
                                DN->GetIdentifier()->GetSymbolTableEntry());
  ASTSymbolTable::Instance().EraseLocalSymbol(DCN->GetIdentifier(),
                                              DCN->GetIdentifier()->GetBits(),
                                              DCN->GetIdentifier()->GetSymbolType());
  if (!ASTDefcalBuilder::Instance().Insert(Id->GetMangledName(), DCN)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Warning);
  }

  if (!ASTSymbolTable::Instance().InsertDefcal(Id, STE)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
  }

  DGN->AddDefcal(DCN);
  return DCN;
}

ASTDefcalNode*
ASTBuilder::CreateASTDefcalNode(const ASTIdentifierNode* Id,
                                const std::string& GS,
                                const ASTDelayNode* DN,
                                const ASTStatementList& SL,
                                const ASTBoundQubitList* BQL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DN && "Invalid ASTDelayNode argument!");
  assert(BQL && "Invalid ASTBoundQubitList argument!");

  const std::string& IDS = Id->GetName();
  ASTMapSymbolTableEntry* MSTE = ASTSymbolTable::Instance().FindDefcalGroup(IDS);

  if (!MSTE) {
    std::stringstream M;
    M << "Failed to locate defcal group " << IDS << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTableEntry* STE =
    const_cast<ASTIdentifierNode*>(Id)->GetSymbolTableEntry();
  if (!STE) {
    std::stringstream M;
    M << "Defcal Identifier has no SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return nullptr;
  }

  if (STE == MSTE) {
    std::stringstream M;
    M << "Defcal SymbolTable Entry points to DefcalGroup SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDefcal) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeDefcal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ASTDefcalGroupNode* DGN =
    ASTBuilder::Instance().ASTBuilder::CreateASTDefcalGroupNode(Id);
  assert(DGN && "Could not obtain a valid ASTDefcalGroupNode!");

  ASTDefcalNode* DCN = new ASTDefcalNode(Id, GS, DN, SL, BQL);
  assert(DCN && "Could not create a valid ASTDefcalNode!");

  DCN->ToDefcalParamSymbolTable("delay",
                                DCN->GetIdentifier()->GetSymbolTableEntry());
  ASTSymbolTable::Instance().EraseLocalSymbol(DCN->GetIdentifier(),
                                              DCN->GetIdentifier()->GetBits(),
                                              DCN->GetIdentifier()->GetSymbolType());
  if (!ASTDefcalBuilder::Instance().Insert(Id->GetMangledName(), DCN)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Warning);
  }

  if (!ASTSymbolTable::Instance().InsertDefcal(Id, STE)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
  }

  DGN->AddDefcal(DCN);
  return DCN;
}

ASTDefcalNode*
ASTBuilder::CreateASTDefcalNode(const ASTIdentifierNode* Id,
                                const ASTDurationOfNode* DN,
                                const ASTStatementList& SL,
                                const ASTBoundQubitList* BQL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DN && "Invalid ASTDurationOfNode argument!");
  assert(BQL && "Invalid ASTBoundQubitList argument!");

  const std::string& IDS = Id->GetName();
  ASTMapSymbolTableEntry* MSTE = ASTSymbolTable::Instance().FindDefcalGroup(IDS);

  if (!MSTE) {
    std::stringstream M;
    M << "Failed to locate defcal group " << IDS << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTableEntry* STE =
    const_cast<ASTIdentifierNode*>(Id)->GetSymbolTableEntry();
  if (!STE) {
    std::stringstream M;
    M << "Defcal Identifier has no SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return nullptr;
  }

  if (STE == MSTE) {
    std::stringstream M;
    M << "Defcal SymbolTable Entry points to DefcalGroup SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDefcal) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeDefcal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ASTDefcalGroupNode* DGN =
    ASTBuilder::Instance().ASTBuilder::CreateASTDefcalGroupNode(Id);
  assert(DGN && "Could not obtain a valid ASTDefcalGroupNode!");

  ASTDefcalNode* DCN = new ASTDefcalNode(Id, DN, SL, BQL);
  assert(DCN && "Could not create a valid ASTDefcalNode!");

  DCN->ToDefcalParamSymbolTable("durationof",
                                DN->GetIdentifier()->GetSymbolTableEntry());
  ASTSymbolTable::Instance().EraseLocalSymbol(DCN->GetIdentifier(),
                                              DCN->GetIdentifier()->GetBits(),
                                              DCN->GetIdentifier()->GetSymbolType());
  if (!ASTDefcalBuilder::Instance().Insert(Id->GetMangledName(), DCN)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Warning);
  }

  if (!ASTSymbolTable::Instance().InsertDefcal(Id, STE)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
  }

  DGN->AddDefcal(DCN);
  return DCN;
}

ASTDefcalNode*
ASTBuilder::CreateASTDefcalNode(const ASTIdentifierNode* Id,
                                const std::string& GS,
                                const ASTDurationOfNode* DN,
                                const ASTStatementList& SL,
                                const ASTBoundQubitList* BQL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DN && "Invalid ASTDurationOfNode argument!");
  assert(BQL && "Invalid ASTBoundQubitList argument!");

  const std::string& IDS = Id->GetName();
  ASTMapSymbolTableEntry* MSTE = ASTSymbolTable::Instance().FindDefcalGroup(IDS);

  if (!MSTE) {
    std::stringstream M;
    M << "Failed to locate defcal group " << IDS << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTableEntry* STE =
    const_cast<ASTIdentifierNode*>(Id)->GetSymbolTableEntry();
  if (!STE) {
    std::stringstream M;
    M << "Defcal Identifier has no SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return nullptr;
  }

  if (STE == MSTE) {
    std::stringstream M;
    M << "Defcal SymbolTable Entry points to DefcalGroup SymbolTable Entry.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDefcal) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeDefcal) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return nullptr;
  }

  ASTDefcalGroupNode* DGN =
    ASTBuilder::Instance().ASTBuilder::CreateASTDefcalGroupNode(Id);
  assert(DGN && "Could not obtain a valid ASTDefcalGroupNode!");

  ASTDefcalNode* DCN = new ASTDefcalNode(Id, GS, DN, SL, BQL);
  assert(DCN && "Could not create a valid ASTDefcalNode!");

  DCN->ToDefcalParamSymbolTable("durationof",
                                DCN->GetIdentifier()->GetSymbolTableEntry());
  ASTSymbolTable::Instance().EraseLocalSymbol(DCN->GetIdentifier(),
                                              DCN->GetIdentifier()->GetBits(),
                                              DCN->GetIdentifier()->GetSymbolType());
  if (!ASTDefcalBuilder::Instance().Insert(Id->GetMangledName(), DCN)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Warning);
  }

  if (!ASTSymbolTable::Instance().InsertDefcal(Id, STE)) {
    std::stringstream M;
    M << "Defcal Symbol " << IDS << " already exists in the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
  }

  DGN->AddDefcal(DCN);
  return DCN;
}

ASTDefcalDeclarationNode*
ASTBuilder::CreateASTDefcalDeclaration(const ASTIdentifierNode* Id,
                                       const ASTDefcalNode* DN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DN && "Invalid ASTDefcalNode argument!");

  ASTDefcalDeclarationNode* DDN = new ASTDefcalDeclarationNode(Id, DN);
  assert(DDN && "Could not create a valid ASTDefcalDeclarationNode!");

  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  ASTDeclarationBuilder::Instance().Append(DDN);
  return DDN;
}

ASTDefcalGroupNode*
ASTBuilder::CreateASTDefcalGroupNode(const ASTIdentifierNode* Id) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTDefcalGroupNode* DGN = nullptr;
  ASTMapSymbolTableEntry* MSTE =
    ASTSymbolTable::Instance().FindDefcalGroup(Id->GetName());

  if (!MSTE) {
    MSTE = new ASTMapSymbolTableEntry(Id, Id->GetSymbolType());
    assert(MSTE && "Could not create a valid ASTMapSymbolTableEntry!");

    DGN = new ASTDefcalGroupNode(Id);
    assert(DGN && "Could not create a valid ASTDefcalGroupNode!");

    MSTE->ResetValue();
    MSTE->SetValue(new ASTValue<>(DGN, ASTTypeDefcalGroup),
                   ASTTypeDefcalGroup, true);
    assert(MSTE->HasValue() && "Defcal group SymbolTable Entry has no Value!");

    if (!ASTSymbolTable::Instance().InsertDefcalGroup(Id, MSTE)) {
      std::stringstream M;
      M << "Failure inserting new Defcal group into the Symbol Table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
      return nullptr;
    }
  } else if (MSTE && !MSTE->HasValue()) {
    DGN = new ASTDefcalGroupNode(Id);
    assert(DGN && "Could not create a valid ASTDefcalGroupNode!");
    assert(DGN->GetASTType() == ASTTypeDefcalGroup &&
           "Invalid ASTType for ASTDefcalGroupNode!");

    MSTE->SetValue(new ASTValue<>(DGN, ASTTypeDefcalGroup),
                   ASTTypeDefcalGroup, true);
    assert(MSTE->HasValue() && "Defcal group SymbolTable Entry has no Value!");
    assert(MSTE->GetValueType() == ASTTypeDefcalGroup &&
           "Invalid ASTType for defcal group SymbolTable Entry!");
  } else {
    DGN = MSTE->GetValue()->GetValue<ASTDefcalGroupNode*>();
    assert(DGN && "Could not obtain a valid ASTDefcalGroupNode!");
  }

  return DGN;
}

ASTDurationNode*
ASTBuilder::CreateASTDurationNode(const ASTIdentifierNode* Id,
                                  const std::string& Units) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(!Units.empty() && "Invalid Length Units argument!");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, ASTDurationNode::DurationBits,
                                                ASTTypeDuration))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDuration) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeLength) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  Id->SetBits(ASTDurationNode::DurationBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeDuration);

  ASTDurationNode* DN = new ASTDurationNode(Id, Units);
  assert(DN && "Could not create a valid ASTDurationNode!");

  STE->SetValue(new ASTValue<>(DN, ASTTypeDuration), ASTTypeDuration);
  assert(STE->HasValue() && "Invalid ASTDurationNode Value for the "
                            "ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return DN;
}

ASTDurationNode*
ASTBuilder::CreateASTDurationNode(const ASTIdentifierNode* Id,
                                  const ASTDurationOfNode* DON) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DON && "Invalid ASTDurationOfNode argument!");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, ASTDurationNode::DurationBits,
                                                ASTTypeDuration))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDuration) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeLength) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  Id->SetBits(ASTDurationNode::DurationBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeDuration);

  ASTDurationNode* DN = new ASTDurationNode(Id, DON);
  assert(DN && "Could not create a valid ASTDurationNode!");

  STE->SetValue(new ASTValue<>(DN, ASTTypeDuration), ASTTypeDuration);
  assert(STE->HasValue() && "Invalid ASTDurationNode Value for the "
                            "ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return DN;
}

ASTDurationNode*
ASTBuilder::CreateASTDurationNode(const ASTIdentifierNode* Id,
                                  const ASTBinaryOpNode* BOP) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(BOP && "Invalid ASTBinaryOpNode argument!");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, ASTDurationNode::DurationBits,
                                                ASTTypeDuration))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDuration) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeLength) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  Id->SetBits(ASTDurationNode::DurationBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeDuration);

  ASTDurationNode* DN = new ASTDurationNode(Id, BOP);
  assert(DN && "Could not create a valid ASTDurationNode!");

  STE->SetValue(new ASTValue<>(DN, ASTTypeDuration), ASTTypeDuration);
  assert(STE->HasValue() && "Invalid ASTDurationNode Value for the "
                            "ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return DN;
}

ASTDurationNode*
ASTBuilder::CreateASTDurationNode(const ASTIdentifierNode* Id,
                                  const ASTExpressionNode* EXP) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(EXP && "Invalid ASTExpressionNode argument!");

  const ASTFunctionCallNode* FC = dynamic_cast<const ASTFunctionCallNode*>(EXP);
  if (!FC) {
    std::stringstream M;
    M << "Expression is not a function call.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTSymbolTableEntry* STE = nullptr;
  if (!(STE = ASTSymbolTable::Instance().Lookup(Id, ASTDurationNode::DurationBits,
                                                ASTTypeDuration))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << Id->GetName()
      << " in the Symbol Table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDuration) {
    std::stringstream M;
    M << "Identifier " << Id->GetName() << " exists in the SymbolTable with a "
      << "different Type (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeLength) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  Id->SetBits(ASTDurationNode::DurationBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeDuration);

  ASTDurationNode* DN = new ASTDurationNode(Id, FC);
  assert(DN && "Could not create a valid ASTDurationNode!");

  STE->SetValue(new ASTValue<>(DN, ASTTypeDuration), ASTTypeDuration);
  assert(STE->HasValue() && "Invalid ASTDurationNode Value for the "
                            "ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return DN;
}

ASTDurationOfNode*
ASTBuilder::CreateASTDurationOfNode(const ASTIdentifierNode* Id,
                                    const ASTIdentifierNode* TId) {
  assert(Id && "Invalid DurationOf ASTIdentifierNode argument!");
  assert(TId && "Invalid DurationOf Target ASTIdentifierNode argument!");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), Id->GetSymbolType());

  if (!STE) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDurationOf) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeLengthOf) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  STE->ResetValue();
  STE->SetValueType(ASTTypeDurationOf);

  ASTDurationOfNode* DON = new ASTDurationOfNode(Id, TId);
  assert(DON && "Could not create a valid ASTDurationOfNode!");

  STE->SetValue(new ASTValue<>(DON, ASTTypeDurationOf), ASTTypeDurationOf);
  assert(STE->HasValue() && "Invalid ASTDurationOfNode Value for the "
                            "ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  DON->Mangle();
  return DON;
}

ASTDurationOfNode*
ASTBuilder::CreateASTDurationOfNode(const ASTIdentifierNode* Id,
                                    const ASTIdentifierNode* TId,
                                    const ASTIdentifierList* QIL) {
  assert(Id && "Invalid DurationOf ASTIdentifierNode argument!");
  assert(TId && "Invalid DurationOf Target ASTIdentifierNode argument!");
  assert(QIL && "Invalid DurationOf ASTIdentifierList operands argument!");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), Id->GetSymbolType());

  if (!STE) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDurationOf) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeLengthOf) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  STE->ResetValue();
  STE->SetValueType(ASTTypeDurationOf);

  ASTDurationOfNode* DON = new ASTDurationOfNode(Id, TId, *QIL);
  assert(DON && "Could not create a valid ASTDurationOfNode!");

  STE->SetValue(new ASTValue<>(DON, ASTTypeDurationOf), ASTTypeDurationOf);
  assert(STE->HasValue() && "Invalid ASTDurationOfNode Value for the "
                            "ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  DON->Mangle();
  return DON;
}


ASTDurationOfNode*
ASTBuilder::CreateASTDurationOfNode(const ASTIdentifierNode* Id,
                                    const ASTIdentifierNode* TId,
                                    const ASTExpressionList* AIL,
                                    const ASTIdentifierList* QIL) {
  assert(Id && "Invalid DurationOf ASTIdentifierNode argument!");
  assert(TId && "Invalid DurationOf Target ASTIdentifierNode argument!");
  assert(AIL && "Invalid DurationOf ASTExpressionList operands argument!");
  assert(QIL && "Invalid DurationOf ASTIdentifierList operands argument!");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), Id->GetSymbolType());

  if (!STE) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDurationOf) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeLengthOf) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  STE->ResetValue();
  STE->SetValueType(ASTTypeDurationOf);

  ASTDurationOfNode* DON = new ASTDurationOfNode(Id, TId, *AIL, *QIL);
  assert(DON && "Could not create a valid ASTDurationOfNode!");

  STE->SetValue(new ASTValue<>(DON, ASTTypeDurationOf), ASTTypeDurationOf);
  assert(STE->HasValue() && "Invalid ASTDurationOfNode Value for the "
                            "ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  DON->Mangle();
  return DON;
}

ASTDurationOfNode*
ASTBuilder::CreateASTDurationOfNode(const ASTIdentifierNode* Id,
                                    const ASTGateQOpNode* QOP) {
  assert(Id && "Invalid DurationOf ASTIdentifierNode argument!");
  assert(QOP && "Invalid ASTGateQOpNode argument!");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), Id->GetSymbolType());

  if (!STE) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeDurationOf) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeLengthOf) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  STE->ResetValue();
  STE->SetValueType(ASTTypeDurationOf);

  ASTDurationOfNode* DON = new ASTDurationOfNode(Id, QOP);
  assert(DON && "Could not create a valid ASTDurationOfNode!");

  STE->SetValue(new ASTValue<>(DON, ASTTypeDurationOf), ASTTypeDurationOf);
  assert(STE->HasValue() && "Invalid ASTDurationOfNode Value for the "
                            "ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  DON->Mangle();
  return DON;
}

ASTLengthNode*
ASTBuilder::CreateASTLengthNode(const ASTIdentifierNode* Id,
                                const std::string& Units) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(!Units.empty() && "Invalid Length Units argument!");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(IDS))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeLength) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeLength) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  Id->SetBits(32);
  STE->ResetValue();
  STE->SetValueType(ASTTypeLength);

  ASTLengthNode* LN = new ASTLengthNode(Id, Units);
  assert(LN && "Could not create a valid ASTLengthNode!");

  STE->SetValue(new ASTValue<>(LN, ASTTypeLength), ASTTypeLength);
  assert(STE->HasValue() && "Invalid ASTLengthNode Value for the "
                            "ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return LN;
}

ASTLengthNode*
ASTBuilder::CreateASTLengthNode(const ASTIdentifierNode* Id,
                                const ASTLengthOfNode* LON) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(LON && "Invalid ASTLengthOfNode argument!");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(IDS))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeLength) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeLength) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  Id->SetBits(ASTLengthNode::LengthBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeLength);

  ASTLengthNode* LN = new ASTLengthNode(Id, LON);
  assert(LN && "Could not create a valid ASTLengthNode!");

  STE->SetValue(new ASTValue<>(LN, ASTTypeLength), ASTTypeLength);
  assert(STE->HasValue() && "Invalid ASTLengthNode Value for the "
                            "ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return LN;
}

ASTLengthNode*
ASTBuilder::CreateASTLengthNode(const ASTIdentifierNode* Id,
                                const ASTBinaryOpNode* BOP) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(BOP && "Invalid ASTBinaryOpNode argument!");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(IDS))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeLength) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeLength) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  Id->SetBits(ASTLengthNode::LengthBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeLength);

  ASTLengthNode* LN = new ASTLengthNode(Id, BOP);
  assert(LN && "Could not create a valid ASTLengthNode!");

  STE->SetValue(new ASTValue<>(LN, ASTTypeLength), ASTTypeLength);
  assert(STE->HasValue() && "Invalid ASTLengthNode Value for the "
                            "ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return LN;
}

ASTLengthOfNode*
ASTBuilder::CreateASTLengthOfNode(const ASTIdentifierNode* Id,
                                  const ASTIdentifierNode* TId) {
  assert(Id && "Invalid LengthOf ASTIdentifierNode argument!");
  assert(TId && "Invalid LengthOf Target ASTIdentifierNode argument!");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(IDS))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeLengthOf) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeLengthOf) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  Id->SetBits(ASTLengthOfNode::LengthOfBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeLengthOf);

  ASTLengthOfNode* LON = new ASTLengthOfNode(Id, TId);
  assert(LON && "Could not create a valid ASTLengthOfNode!");

  STE->SetValue(new ASTValue<>(LON, ASTTypeLengthOf), ASTTypeLengthOf);
  assert(STE->HasValue() && "Invalid ASTLengthOfNode Value for the "
                            "ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return LON;
}

ASTCallExpressionNode*
ASTBuilder::CreateASTCallExpressionNode(const ASTIdentifierNode* Id,
                                        const ASTIdentifierNode* CId,
                                        const ASTParameterList& PL,
                                        const ASTIdentifierList& IL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(CId && "Invalid Callee ASTIdentifierNode Argument!");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(IDS))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeFunctionCall) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeFunctionCall) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  Id->SetBits(ASTCallExpressionNode::CallExpressionBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeFunctionCall);

  ASTCallExpressionNode* CEN = new ASTCallExpressionNode(Id, CId, PL, IL);
  assert(CEN && "Could not create a valid ASTCallExpressionNode!");

  STE->SetValue(new ASTValue<>(CEN, ASTTypeFunctionCall),
                ASTTypeFunctionCall);
  assert(STE->HasValue() && "Invalid ASTCallExpressionNode Value for"
                            "the ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return CEN;
}

ASTControlExpressionNode*
ASTBuilder::CreateASTControlExpressionNode(const ASTIdentifierNode* Id,
                                           const ASTCtrlAssocList* CL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(CL && "Invalid ASTCtrlAssocList argument");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(IDS))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeControlExpression) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeControlExpression) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  Id->SetBits(ASTControlExpressionNode::ControlExpressionBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeControlExpression);

  ASTControlExpressionNode* CEN = new ASTControlExpressionNode(Id, CL);
  assert(CEN && "Could not create a valid ASTControlExpressionNode!");

  STE->SetValue(new ASTValue<>(CEN, ASTTypeControlExpression),
                ASTTypeControlExpression);
  assert(STE->HasValue() && "Invalid ASTControlExpressionNode Value for"
                            "the ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return CEN;
}

ASTInverseExpressionNode*
ASTBuilder::CreateASTInverseExpressionNode(const ASTIdentifierNode* Id,
                                           const ASTInverseAssocList* IL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IL && "Invalid ASTInverseAssocList argument");

  const std::string& IDS = Id->GetName();
  ASTSymbolTableEntry* STE = nullptr;

  if (!(STE = ASTSymbolTable::Instance().Lookup(IDS))) {
    std::stringstream M;
    M << "Failed to locate Identifier " << IDS << " in the Symbol Table!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (STE->GetValueType() != ASTTypeUndefined &&
      STE->GetValueType() != ASTTypeInverseExpression) {
    std::stringstream M;
    M << "Identifier " << IDS << " exists in the SymbolTable with a "
      << "different Type! (" << PrintTypeEnum(STE->GetValueType())
      << " vs. " << PrintTypeEnum(ASTTypeInverseExpression) << ").";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  Id->SetBits(ASTInverseExpressionNode::InverseExpressionBits);
  STE->ResetValue();
  STE->SetValueType(ASTTypeInverseExpression);

  ASTInverseExpressionNode* IEN = new ASTInverseExpressionNode(Id, IL);
  assert(IEN && "Could not create a valid ASTControlExpressionNode!");

  STE->SetValue(new ASTValue<>(IEN, ASTTypeInverseExpression),
                ASTTypeInverseExpression);
  assert(STE->HasValue() && "Invalid ASTControlExpressionNode Value for"
                            "the ASTSymbolTable Entry!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return IEN;
}

ASTGateNode*
ASTBuilder::CreateASTGateNode(const ASTIdentifierNode* Id,
                              GateKind GK,
                              const ASTParameterList& PL,
                              const ASTIdentifierList& IL,
                              const ASTGateQOpList& OL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTGateNode* GN = nullptr;
  ASTSymbolTableEntry* STE = nullptr;

  Id->SetBits(ASTGateNode::GateBits);
  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();

  switch (GK) {
    case ASTGateKindGeneric:
      STE = new ASTSymbolTableEntry(Id, ASTTypeGate);
      assert(STE && "Could not create a valid ASTGateNode SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
    case ASTGateKindCX:
      STE = new ASTSymbolTableEntry(Id, ASTTypeCXGate);
      assert(STE && "Could not create a valid ASTCXGateNode SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
    case ASTGateKindCCX:
      STE = new ASTSymbolTableEntry(Id, ASTTypeCCXGate);
      assert(STE && "Could not create a valid ASTCCXGateNode "
                    "SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
    case ASTGateKindCNOT:
      STE = new ASTSymbolTableEntry(Id, ASTTypeCNotGate);
      assert(STE && "Could not create a valid ASTCNotGateNode "
                    "SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
    case ASTGateKindH:
      STE = new ASTSymbolTableEntry(Id, ASTTypeHadamardGate);
      assert(STE && "Could not create a valid ASTHadamardGateNode "
                    "SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
    case ASTGateKindU:
      STE = new ASTSymbolTableEntry(Id, ASTTypeUGate);
      assert(STE && "Could not create a valid ASTUGateNode SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
    default:
      STE = new ASTSymbolTableEntry(Id, ASTTypeGate);
      assert(STE && "Could not create a valid ASTGateNode SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
  }

  switch (GK) {
  case ASTGateKindGeneric:
    GN = new ASTGateNode(Id, PL, IL, false, OL);
    assert(GN && "Could not create a valid Generic ASTGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeGate), ASTTypeGate);
    assert(STE->HasValue() && "Generic ASTGateNode has no Value!");
    break;
  case ASTGateKindCX:
    GN = new ASTCXGateNode(Id, PL, IL, false, OL);
    assert(GN && "Could not create a valid ASTCXGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeCXGate), ASTTypeCXGate);
    assert(STE->HasValue() && "CX Gate ASTGateNode has no Value!");
    break;
  case ASTGateKindCCX:
    GN = new ASTCCXGateNode(Id, PL, IL, false, OL);
    assert(GN && "Could not create a valid ASTCCXGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeCCXGate), ASTTypeCCXGate);
    assert(STE->HasValue() && "CCX Gate ASTGateNode has no Value!");
    break;
  case ASTGateKindCNOT:
    GN = new ASTCNotGateNode(Id, PL, IL, false, OL);
    assert(GN && "Could not create a valid ASTCNotGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeCNotGate), ASTTypeCNotGate);
    assert(STE->HasValue() && "CNot Gate ASTGateNode has no Value!");
    break;
  case ASTGateKindH:
    GN = new ASTHadamardGateNode(Id, PL, IL, false, OL);
    assert(GN && "Could not create a valid ASTHadamardGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeHadamardGate),
                  ASTTypeHadamardGate);
    assert(STE->HasValue() && "HadamardGate ASTGateNode has no Value!");
    break;
  case ASTGateKindU:
    GN = new ASTUGateNode(Id, PL, IL, false, OL);
    assert(GN && "Could not create a valid ASTUGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeUGate), ASTTypeUGate);
    assert(STE->HasValue() && "UGate ASTGateNode has no Value!");
    break;
  default:
    GN = new ASTGateNode(Id, PL, IL, false, OL);
    assert(GN && "Could not create a valid Generic ASTGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeGate), ASTTypeGate);
    assert(STE->HasValue() && "Generic ASTGateNode has no Value!");
    break;
  }

  STE->SetGlobalScope();
  STE->SetDoNotDelete();
  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(true);

  if (ASTSymbolTable::Instance().InsertGate(const_cast<ASTIdentifierNode*>(Id),
                                            STE)) {
    ASTGateNodeBuilder::Instance().Append(GN);
    return GN;
  }

  std::stringstream M;
  M << "Failure inserting Gate '" << Id->GetName() << "' into the "
    << "Gate Symbol Table.";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
    DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
  return nullptr;
}

ASTGateNode*
ASTBuilder::CreateASTGateNode(const ASTIdentifierNode* Id,
                              GateKind GK,
                              const ASTArgumentNodeList& AL,
                              const ASTAnyTypeList& QL,
                              const ASTGateQOpList& OL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTGateNode* GN = nullptr;
  ASTSymbolTableEntry* STE = nullptr;

  Id->SetBits(ASTGateNode::GateBits);
  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();

  switch (GK) {
    case ASTGateKindGeneric:
      STE = new ASTSymbolTableEntry(Id, ASTTypeGate);
      assert(STE && "Could not create a valid ASTGateNode SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
    case ASTGateKindCX:
      STE = new ASTSymbolTableEntry(Id, ASTTypeCXGate);
      assert(STE && "Could not create a valid ASTCXGateNode SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
    case ASTGateKindCCX:
      STE = new ASTSymbolTableEntry(Id, ASTTypeCCXGate);
      assert(STE && "Could not create a valid ASTCCXGateNode "
                    "SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
    case ASTGateKindCNOT:
      STE = new ASTSymbolTableEntry(Id, ASTTypeCNotGate);
      assert(STE && "Could not create a valid ASTCNotGateNode "
                    "SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
    case ASTGateKindH:
      STE = new ASTSymbolTableEntry(Id, ASTTypeHadamardGate);
      assert(STE && "Could not create a valid ASTHadamardGateNode "
                    "SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
    case ASTGateKindU:
      STE = new ASTSymbolTableEntry(Id, ASTTypeUGate);
      assert(STE && "Could not create a valid ASTUGateNode SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
    default:
      STE = new ASTSymbolTableEntry(Id, ASTTypeGate);
      assert(STE && "Could not create a valid ASTGateNode SymbolTableEntry!");
      STE->SetGlobalScope();
      break;
  }

  switch (GK) {
  case ASTGateKindGeneric:
    GN = new ASTGateNode(Id, AL, QL, false, OL);
    assert(GN && "Could not create a valid Generic ASTGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeGate), ASTTypeGate);
    assert(STE->HasValue() && "Generic ASTGateNode has no Value!");
    break;
  case ASTGateKindCX:
    GN = new ASTCXGateNode(Id, AL, QL, false, OL);
    assert(GN && "Could not create a valid ASTCXGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeCXGate), ASTTypeCXGate);
    assert(STE->HasValue() && "CX Gate ASTGateNode has no Value!");
    break;
  case ASTGateKindCCX:
    GN = new ASTCCXGateNode(Id, AL, QL, false, OL);
    assert(GN && "Could not create a valid ASTCCXGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeCCXGate), ASTTypeCCXGate);
    assert(STE->HasValue() && "CCX Gate ASTGateNode has no Value!");
    break;
  case ASTGateKindCNOT:
    GN = new ASTCNotGateNode(Id, AL, QL, false, OL);
    assert(GN && "Could not create a valid ASTCNotGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeCNotGate), ASTTypeCNotGate);
    assert(STE->HasValue() && "CNot Gate ASTGateNode has no Value!");
    break;
  case ASTGateKindH:
    GN = new ASTHadamardGateNode(Id, AL, QL, false, OL);
    assert(GN && "Could not create a valid ASTHadamardGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeHadamardGate),
                  ASTTypeHadamardGate);
    assert(STE->HasValue() && "HadamardGate ASTGateNode has no Value!");
    break;
  case ASTGateKindU:
    GN = new ASTUGateNode(Id, AL, QL, false, OL);
    assert(GN && "Could not create a valid ASTUGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeUGate), ASTTypeUGate);
    assert(STE->HasValue() && "UGate ASTGateNode has no Value!");
    break;
  default:
    GN = new ASTGateNode(Id, AL, QL, false, OL);
    assert(GN && "Could not create a valid Generic ASTGateNode!");
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(GN, ASTTypeGate), ASTTypeGate);
    assert(STE->HasValue() && "Generic ASTGateNode has no Value!");
    break;
  }

  STE->SetGlobalScope();
  STE->SetDoNotDelete();
  const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  const_cast<ASTIdentifierNode*>(Id)->SetHasSymbolTableEntry(true);

  if (ASTSymbolTable::Instance().InsertGate(const_cast<ASTIdentifierNode*>(Id),
                                            STE)) {
    ASTGateNodeBuilder::Instance().Append(GN);
    return GN;
  }

  std::stringstream M;
  M << "Failure inserting Gate '" << Id->GetName() << "' into the "
    << "Gate Symbol Table.";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
    DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
  return nullptr;
}

ASTCastExpressionNode*
ASTBuilder::CreateASTCastExpressionNode(const ASTExpressionNode* From,
                                        ASTType To, unsigned Bits) {
  assert(From && "Invalid ASTExpressionNode* argument!");
  switch (To) {
  case ASTTypeIdentifier:
  case ASTTypeBool:
  case ASTTypeInt:
  case ASTTypeFloat:
  case ASTTypeDouble:
  case ASTTypeMPInteger:
  case ASTTypeMPDecimal:
  case ASTTypeBitset:
  case ASTTypeAngle:
    break;
  default: {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(From->GetASTType())
      << " to " << PrintTypeEnum(To) << ".";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  switch (From->GetASTType()) {
  case ASTTypeIdentifier: {
    const ASTIdentifierNode* Id = nullptr;

    if (From->IsIdentifier())
      Id = From->GetIdentifier();
    else
      Id = dynamic_cast<const ASTIdentifierNode*>(From);

    assert(Id && "Could not dynamic_cast to an ASTIdentifierNode!");

    ASTScopeController::Instance().CheckUndefined(Id);
    ASTScopeController::Instance().CheckOutOfScope(Id);

    switch (To) {
    case ASTTypeBool:
      return new ASTCastExpressionNode(Id, ASTTypeBool, Bits);
      break;
    case ASTTypeInt:
      return new ASTCastExpressionNode(Id, ASTTypeInt, Bits);
      break;
    case ASTTypeFloat:
      return new ASTCastExpressionNode(Id, ASTTypeFloat, Bits);
      break;
    case ASTTypeDouble:
      return new ASTCastExpressionNode(Id, ASTTypeDouble, Bits);
      break;
    case ASTTypeMPInteger:
      return new ASTCastExpressionNode(Id, ASTTypeMPInteger, Bits);
      break;
    case ASTTypeMPDecimal:
      return new ASTCastExpressionNode(Id, ASTTypeMPDecimal, Bits);
      break;
    case ASTTypeBitset:
      return new ASTCastExpressionNode(Id, ASTTypeBitset, Bits);
      break;
    case ASTTypeAngle:
      return new ASTCastExpressionNode(Id, ASTTypeAngle, Bits);
      break;
    default: {
      std::stringstream M;
      M << "Impossible cast from " << PrintTypeEnum(From->GetASTType())
        << " to " << PrintTypeEnum(To) << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }
      break;
    }
  }
    break;
  case ASTTypeBool: {
    const ASTBoolNode* B = dynamic_cast<const ASTBoolNode*>(From);
    assert(B && "Could not dynamic_cast to an ASTBoolNode!");
    switch (To) {
    case ASTTypeBool:
      return new ASTCastExpressionNode(B, ASTTypeBool, Bits);
      break;
    case ASTTypeInt:
      return new ASTCastExpressionNode(B, ASTTypeInt, Bits);
      break;
    case ASTTypeFloat:
      return new ASTCastExpressionNode(B, ASTTypeFloat, Bits);
      break;
    case ASTTypeDouble:
      return new ASTCastExpressionNode(B, ASTTypeDouble, Bits);
      break;
    case ASTTypeMPInteger:
      return new ASTCastExpressionNode(B, ASTTypeMPInteger, Bits);
      break;
    case ASTTypeMPDecimal:
      return new ASTCastExpressionNode(B, ASTTypeMPDecimal, Bits);
      break;
    case ASTTypeBitset:
      return new ASTCastExpressionNode(B, ASTTypeBitset, Bits);
      break;
    case ASTTypeAngle:
      return new ASTCastExpressionNode(B, ASTTypeAngle, Bits);
      break;
    default: {
      std::stringstream M;
      M << "Impossible cast from " << PrintTypeEnum(From->GetASTType())
        << " to " << PrintTypeEnum(To) << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }
      break;
    }
  }
    break;
  case ASTTypeInt: {
    const ASTIntNode* I = dynamic_cast<const ASTIntNode*>(From);
    assert(I && "Could not dynamic_cast to an ASTIntNode!");
    switch (To) {
    case ASTTypeBool:
      return new ASTCastExpressionNode(I, ASTTypeBool, Bits);
      break;
    case ASTTypeInt:
      return new ASTCastExpressionNode(I, ASTTypeInt, Bits);
      break;
    case ASTTypeFloat:
      return new ASTCastExpressionNode(I, ASTTypeFloat, Bits);
      break;
    case ASTTypeDouble:
      return new ASTCastExpressionNode(I, ASTTypeDouble, Bits);
      break;
    case ASTTypeMPInteger:
      return new ASTCastExpressionNode(I, ASTTypeMPInteger, Bits);
      break;
    case ASTTypeMPDecimal:
      return new ASTCastExpressionNode(I, ASTTypeMPDecimal, Bits);
      break;
    case ASTTypeBitset:
      return new ASTCastExpressionNode(I, ASTTypeBitset, Bits);
      break;
    case ASTTypeAngle:
      return new ASTCastExpressionNode(I, ASTTypeAngle, Bits);
      break;
    default: {
      std::stringstream M;
      M << "Impossible cast from " << PrintTypeEnum(From->GetASTType())
        << " to " << PrintTypeEnum(To) << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }
      break;
    }
  }
    break;
  case ASTTypeFloat: {
    const ASTFloatNode* F = dynamic_cast<const ASTFloatNode*>(From);
    assert(F && "Could not dynamic_cast to an ASTFloatNode!");
    switch (To) {
    case ASTTypeBool:
      return new ASTCastExpressionNode(F, ASTTypeBool, Bits);
      break;
    case ASTTypeInt:
      return new ASTCastExpressionNode(F, ASTTypeInt, Bits);
      break;
    case ASTTypeFloat:
      return new ASTCastExpressionNode(F, ASTTypeFloat, Bits);
      break;
    case ASTTypeDouble:
      return new ASTCastExpressionNode(F, ASTTypeDouble, Bits);
      break;
    case ASTTypeMPInteger:
      return new ASTCastExpressionNode(F, ASTTypeMPInteger, Bits);
      break;
    case ASTTypeMPDecimal:
      return new ASTCastExpressionNode(F, ASTTypeMPDecimal, Bits);
      break;
    case ASTTypeBitset:
      return new ASTCastExpressionNode(F, ASTTypeBitset, Bits);
      break;
    case ASTTypeAngle:
      return new ASTCastExpressionNode(F, ASTTypeAngle, Bits);
      break;
    default: {
      std::stringstream M;
      M << "Impossible cast from " << PrintTypeEnum(From->GetASTType())
        << " to " << PrintTypeEnum(To) << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }
      break;
    }
  }
    break;
  case ASTTypeDouble: {
    const ASTDoubleNode* D = dynamic_cast<const ASTDoubleNode*>(From);
    assert(D && "Could not dynamic_cast to an ASTDoubleNode!");
    switch (To) {
    case ASTTypeBool:
      return new ASTCastExpressionNode(D, ASTTypeBool, Bits);
      break;
    case ASTTypeInt:
      return new ASTCastExpressionNode(D, ASTTypeInt, Bits);
      break;
    case ASTTypeFloat:
      return new ASTCastExpressionNode(D, ASTTypeFloat, Bits);
      break;
    case ASTTypeDouble:
      return new ASTCastExpressionNode(D, ASTTypeDouble, Bits);
      break;
    case ASTTypeMPInteger:
      return new ASTCastExpressionNode(D, ASTTypeMPInteger, Bits);
      break;
    case ASTTypeMPDecimal:
      return new ASTCastExpressionNode(D, ASTTypeMPDecimal, Bits);
      break;
    case ASTTypeBitset:
      return new ASTCastExpressionNode(D, ASTTypeBitset, Bits);
      break;
    case ASTTypeAngle:
      return new ASTCastExpressionNode(D, ASTTypeAngle, Bits);
      break;
    default: {
      std::stringstream M;
      M << "Impossible cast from " << PrintTypeEnum(From->GetASTType())
        << " to " << PrintTypeEnum(To) << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }
      break;
    }
  }
    break;
  case ASTTypeMPInteger: {
    const ASTMPIntegerNode* MPI = dynamic_cast<const ASTMPIntegerNode*>(From);
    assert(MPI && "Could not dynamic_cast to an ASTMPIntegerNode!");
    switch (To) {
    case ASTTypeBool:
      return new ASTCastExpressionNode(MPI, ASTTypeBool, Bits);
      break;
    case ASTTypeInt:
      return new ASTCastExpressionNode(MPI, ASTTypeInt, Bits);
      break;
    case ASTTypeFloat:
      return new ASTCastExpressionNode(MPI, ASTTypeFloat, Bits);
      break;
    case ASTTypeDouble:
      return new ASTCastExpressionNode(MPI, ASTTypeDouble, Bits);
      break;
    case ASTTypeMPInteger:
      return new ASTCastExpressionNode(MPI, ASTTypeMPInteger, Bits);
      break;
    case ASTTypeMPDecimal:
      return new ASTCastExpressionNode(MPI, ASTTypeMPDecimal, Bits);
      break;
    case ASTTypeBitset:
      return new ASTCastExpressionNode(MPI, ASTTypeBitset, Bits);
      break;
    case ASTTypeAngle:
      return new ASTCastExpressionNode(MPI, ASTTypeAngle, Bits);
      break;
    default: {
      std::stringstream M;
      M << "Impossible cast from " << PrintTypeEnum(From->GetASTType())
        << " to " << PrintTypeEnum(To) << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }
      break;
    }
  }
    break;
  case ASTTypeMPDecimal: {
    const ASTMPDecimalNode* MPD = dynamic_cast<const ASTMPDecimalNode*>(From);
    assert(MPD && "Could not dynamic_cast to an ASTMPDecimalNode!");
    switch (To) {
    case ASTTypeBool:
      return new ASTCastExpressionNode(MPD, ASTTypeBool, Bits);
      break;
    case ASTTypeInt:
      return new ASTCastExpressionNode(MPD, ASTTypeInt, Bits);
      break;
    case ASTTypeFloat:
      return new ASTCastExpressionNode(MPD, ASTTypeFloat, Bits);
      break;
    case ASTTypeDouble:
      return new ASTCastExpressionNode(MPD, ASTTypeDouble, Bits);
      break;
    case ASTTypeMPInteger:
      return new ASTCastExpressionNode(MPD, ASTTypeMPInteger, Bits);
      break;
    case ASTTypeMPDecimal:
      return new ASTCastExpressionNode(MPD, ASTTypeMPDecimal, Bits);
      break;
    case ASTTypeBitset:
      return new ASTCastExpressionNode(MPD, ASTTypeBitset, Bits);
      break;
    case ASTTypeAngle:
      return new ASTCastExpressionNode(MPD, ASTTypeAngle, Bits);
      break;
    default: {
      std::stringstream M;
      M << "Impossible cast from " << PrintTypeEnum(From->GetASTType())
        << " to " << PrintTypeEnum(To) << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }
      break;
    }
  }
    break;
  case ASTTypeBitset: {
    const ASTCBitNode* CB = dynamic_cast<const ASTCBitNode*>(From);
    assert(CB && "Could not dynamic_cast to an ASTCBitNode!");
    switch (To) {
    case ASTTypeBool:
      return new ASTCastExpressionNode(CB, ASTTypeBool, Bits);
      break;
    case ASTTypeInt:
      return new ASTCastExpressionNode(CB, ASTTypeInt, Bits);
      break;
    case ASTTypeFloat:
      return new ASTCastExpressionNode(CB, ASTTypeFloat, Bits);
      break;
    case ASTTypeDouble:
      return new ASTCastExpressionNode(CB, ASTTypeDouble, Bits);
      break;
    case ASTTypeMPInteger:
      return new ASTCastExpressionNode(CB, ASTTypeMPInteger, Bits);
      break;
    case ASTTypeMPDecimal:
      return new ASTCastExpressionNode(CB, ASTTypeMPDecimal, Bits);
      break;
    case ASTTypeBitset:
      return new ASTCastExpressionNode(CB, ASTTypeBitset, Bits);
      break;
    case ASTTypeAngle:
      return new ASTCastExpressionNode(CB, ASTTypeAngle, Bits);
      break;
    default: {
      std::stringstream M;
      M << "Impossible cast from " << PrintTypeEnum(From->GetASTType())
        << " to " << PrintTypeEnum(To) << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }
      break;
    }
  }
    break;
  case ASTTypeBinaryOp: {
    const ASTBinaryOpNode* BOP = dynamic_cast<const ASTBinaryOpNode*>(From);
    assert(BOP && "Could not dynamic_cast to an ASTBinaryOpNode!");
    switch (To) {
    case ASTTypeBool:
      return new ASTCastExpressionNode(BOP, ASTTypeBool, Bits);
      break;
    case ASTTypeInt:
      return new ASTCastExpressionNode(BOP, ASTTypeInt, Bits);
      break;
    case ASTTypeFloat:
      return new ASTCastExpressionNode(BOP, ASTTypeFloat, Bits);
      break;
    case ASTTypeDouble:
      return new ASTCastExpressionNode(BOP, ASTTypeDouble, Bits);
      break;
    case ASTTypeMPInteger:
      return new ASTCastExpressionNode(BOP, ASTTypeMPInteger, Bits);
      break;
    case ASTTypeMPDecimal:
      return new ASTCastExpressionNode(BOP, ASTTypeMPDecimal, Bits);
      break;
    case ASTTypeBitset:
      return new ASTCastExpressionNode(BOP, ASTTypeBitset, Bits);
      break;
    case ASTTypeAngle:
      return new ASTCastExpressionNode(BOP, ASTTypeAngle, Bits);
      break;
    default: {
      std::stringstream M;
      M << "Impossible cast from " << PrintTypeEnum(From->GetASTType())
        << " to " << PrintTypeEnum(To) << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }
      break;
    }
  }
    break;
  case ASTTypeUnaryOp: {
    const ASTUnaryOpNode* UOP = dynamic_cast<const ASTUnaryOpNode*>(From);
    assert(UOP && "Could not dynamic_cast to an ASTUnaryOpNode!");
    switch (To) {
    case ASTTypeBool:
      return new ASTCastExpressionNode(UOP, ASTTypeBool, Bits);
      break;
    case ASTTypeInt:
      return new ASTCastExpressionNode(UOP, ASTTypeInt, Bits);
      break;
    case ASTTypeFloat:
      return new ASTCastExpressionNode(UOP, ASTTypeFloat, Bits);
      break;
    case ASTTypeDouble:
      return new ASTCastExpressionNode(UOP, ASTTypeDouble, Bits);
      break;
    case ASTTypeMPInteger:
      return new ASTCastExpressionNode(UOP, ASTTypeMPInteger, Bits);
      break;
    case ASTTypeMPDecimal:
      return new ASTCastExpressionNode(UOP, ASTTypeMPDecimal, Bits);
      break;
    case ASTTypeBitset:
      return new ASTCastExpressionNode(UOP, ASTTypeBitset, Bits);
      break;
    case ASTTypeAngle:
      return new ASTCastExpressionNode(UOP, ASTTypeAngle, Bits);
      break;
    default: {
      std::stringstream M;
      M << "Impossible cast from " << PrintTypeEnum(From->GetASTType())
        << " to " << PrintTypeEnum(To) << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }
      break;
    }
  }
    break;
  case ASTTypeAngle: {
    const ASTAngleNode* ANG = dynamic_cast<const ASTAngleNode*>(From);
    assert(ANG && "Could not dynamic_cast to an ASTAngleNode!");
    switch (To) {
    case ASTTypeBool:
      return new ASTCastExpressionNode(ANG, ASTTypeBool, Bits);
      break;
    case ASTTypeInt:
      return new ASTCastExpressionNode(ANG, ASTTypeInt, Bits);
      break;
    case ASTTypeFloat:
      return new ASTCastExpressionNode(ANG, ASTTypeFloat, Bits);
      break;
    case ASTTypeDouble:
      return new ASTCastExpressionNode(ANG, ASTTypeDouble, Bits);
      break;
    case ASTTypeMPInteger:
      return new ASTCastExpressionNode(ANG, ASTTypeMPInteger, Bits);
      break;
    case ASTTypeMPDecimal:
      return new ASTCastExpressionNode(ANG, ASTTypeMPDecimal, Bits);
      break;
    case ASTTypeBitset:
      return new ASTCastExpressionNode(ANG, ASTTypeBitset, Bits);
      break;
    case ASTTypeAngle:
      return new ASTCastExpressionNode(ANG, ASTTypeAngle, Bits);
      break;
    default: {
      std::stringstream M;
      M << "Impossible cast from " << PrintTypeEnum(From->GetASTType())
        << " to " << PrintTypeEnum(To) << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return nullptr;
    }
      break;
    }
  }
    break;
  default: {
    std::stringstream M;
    M << "Impossible cast from " << PrintTypeEnum(From->GetASTType())
      << " to " << PrintTypeEnum(To) << ".";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Error);
    return nullptr;
  }
    break;
  }

  return nullptr;
}

ASTDelayNode*
ASTBuilder::CreateASTDelayNode(const ASTIdentifierNode* Id,
                               const ASTIdentifierNode* TId,
                               const std::string& Time) {
  assert(Id && "Invalid ASTIdentifierNode Delay argument!");
  assert(TId && "Inalid ASTIdentifierNode Delay Target argument!");

  Id->SetBits(ASTDelayNode::DelayBits);

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Delay SymbolTable Entry!");

  ASTDelayNode* DN = new ASTDelayNode(Id, TId, Time);
  assert(DN && "Could not create a valid ASTDelayNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDelay), ASTTypeDelay);
  assert(STE->HasValue() && "Delay SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return DN;
}

ASTDelayNode*
ASTBuilder::CreateASTDelayNode(const ASTIdentifierNode* Id,
                               const ASTBinaryOpNode* BOP) {
  assert(Id && "Invalid ASTIdentifierNode Delay argument!");
  assert(BOP && "Invalid ASTBinaryOpNode Delay argument!");

  Id->SetBits(ASTDelayNode::DelayBits);

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Delay SymbolTable Entry!");

  ASTDelayNode* DN = new ASTDelayNode(Id, BOP);
  assert(DN && "Could not create a valid ASTDelayNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDelay), ASTTypeDelay);
  assert(STE->HasValue() && "Delay SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return DN;
}

ASTDelayNode*
ASTBuilder::CreateASTDelayNode(const ASTIdentifierNode* Id,
                               const ASTUnaryOpNode* UOP) {
  assert(Id && "Invalid ASTIdentifierNode Delay argument!");
  assert(UOP && "Invalid ASTUnaryOpNode Delay argument!");

  Id->SetBits(ASTDelayNode::DelayBits);

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Delay SymbolTable Entry!");

  ASTDelayNode* DN = new ASTDelayNode(Id, UOP);
  assert(DN && "Could not create a valid ASTDelayNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDelay), ASTTypeDelay);
  assert(STE->HasValue() && "Delay SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return DN;
}

ASTDelayNode*
ASTBuilder::CreateASTDelayNode(const ASTIdentifierNode* Id,
                               const ASTIdentifierNode* TId,
                               const ASTIdentifierNode* QId,
                               const std::string& Time) {
  assert(Id && "Invalid ASTIdentifierNode Delay argument!");
  assert(TId && "Inalid ASTIdentifierNode Delay Target argument!");
  assert(QId && "Inalid ASTIdentifierNode Delay Qubit argument!");

  Id->SetBits(ASTDelayNode::DelayBits);

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Delay SymbolTable Entry!");

  ASTDelayNode* DN = new ASTDelayNode(Id, TId, QId, Time);
  assert(DN && "Could not create a valid ASTDelayNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDelay), ASTTypeDelay);
  assert(STE->HasValue() && "Delay SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return DN;
}

ASTDelayNode*
ASTBuilder::CreateASTDelayNode(const ASTIdentifierNode* Id,
                               const ASTIdentifierNode* TId,
                               const ASTIdentifierList& IL,
                               const std::string& Time) {
  assert(Id && "Invalid ASTIdentifierNode Delay argument!");
  assert(TId && "Inalid ASTIdentifierNode Delay Target argument!");

  Id->SetBits(ASTDelayNode::DelayBits);

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Delay SymbolTable Entry!");

  ASTDelayNode* DN = new ASTDelayNode(Id, TId, IL, Time);
  assert(DN && "Could not create a valid ASTDelayNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDelay), ASTTypeDelay);
  assert(STE->HasValue() && "Delay SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return DN;
}

ASTDelayNode*
ASTBuilder::CreateASTDelayNode(const ASTIdentifierNode* Id,
                               const ASTIdentifierList& IL,
                               const std::string& Time) {
  assert(Id && "Invalid ASTIdentifierNode Delay argument!");

  Id->SetBits(ASTDelayNode::DelayBits);

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Delay SymbolTable Entry!");

  ASTDelayNode* DN = new ASTDelayNode(Id, IL, Time);
  assert(DN && "Could not create a valid ASTDelayNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDelay), ASTTypeDelay);
  assert(STE->HasValue() && "Delay SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return DN;
}

ASTDelayNode*
ASTBuilder::CreateASTDelayNode(const ASTIdentifierNode* Id,
                               const ASTDurationOfNode* DON,
                               const ASTIdentifierList& IL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DON && "Invalid ASTDurationOfNode argument!");

  Id->SetBits(ASTDelayNode::DelayBits);

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Delay SymbolTable Entry!");

  ASTDelayNode* DN = new ASTDelayNode(Id, DON, IL);
  assert(DN && "Could not create a valid ASTDelayNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(DN, ASTTypeDelay), ASTTypeDelay);
  assert(STE->HasValue() && "Delay SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return DN;
}

ASTDelayNode*
ASTBuilder::CreateASTDelayNode(const ASTIdentifierNode* Id,
                               const ASTDurationNode* DN,
                               const ASTIdentifierList& IL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DN && "Invalid ASTDurationNode argument!");

  Id->SetBits(ASTDelayNode::DelayBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Delay SymbolTable Entry!");

  ASTDelayNode* DDN = new ASTDelayNode(Id, DN, IL);
  assert(DDN && "Could not create a valid ASTDelayNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(DDN, ASTTypeDelay), ASTTypeDelay);
  assert(STE->HasValue() && "Delay SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return DDN;
}

ASTStretchNode*
ASTBuilder::CreateASTStretchNode(const ASTIdentifierNode* Id) {
  assert(Id && "Invalid ASTIdentifierNode Stretch argument!");

  Id->SetBits(ASTStretchNode::StretchBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Stretch SymbolTable Entry!");

  ASTStretchNode* SN = new ASTStretchNode(Id, 0UL, DT);
  assert(SN && "Could not create a valid ASTStretchNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(SN, ASTTypeStretch), ASTTypeStretch);
  assert(STE->HasValue() && "Stretch SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return SN;
}

ASTStretchNode*
ASTBuilder::CreateASTStretchNode(const ASTIdentifierNode* Id,
                                 const std::string& LU) {
  assert(Id && "Invalid ASTIdentifierNode Stretch argument!");

  Id->SetBits(ASTDelayNode::DelayBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Stretch SymbolTable Entry!");

  ASTStretchNode* SN = new ASTStretchNode(Id, LU);
  assert(SN && "Could not create a valid ASTStretchNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(SN, ASTTypeStretch), ASTTypeStretch);
  assert(STE->HasValue() && "Stretch SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return SN;
}

ASTStretchNode*
ASTBuilder::CreateASTStretchNode(const ASTIdentifierNode* Id,
                                 uint64_t Duration,
                                 LengthUnit LU) {
  assert(Id && "Invalid ASTIdentifierNode Stretch argument!");

  Id->SetBits(ASTDelayNode::DelayBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Stretch SymbolTable Entry!");

  ASTStretchNode* SN = new ASTStretchNode(Id, Duration, LU);
  assert(SN && "Could not create a valid ASTStretchNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(SN, ASTTypeStretch), ASTTypeStretch);
  assert(STE->HasValue() && "Stretch SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  SN->Mangle();
  return SN;
}

ASTStretchNode*
ASTBuilder::CreateASTStretchNode(const ASTIdentifierNode* Id,
                                 const ASTIntNode* I) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(I && "Invalid ASTIntNode argument!");

  Id->SetBits(ASTStretchNode::StretchBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Stretch SymbolTable Entry!");

  ASTStretchNode* SN = new ASTStretchNode(Id, I);
  assert(SN && "Could not create a valid ASTStretchNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(SN, ASTTypeStretch), ASTTypeStretch);
  assert(STE->HasValue() && "Stretch SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  SN->Mangle();
  return SN;
}

ASTStretchNode*
ASTBuilder::CreateASTStretchNode(const ASTIdentifierNode* Id,
                                 const ASTFloatNode* F) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(F && "Invalid ASTFloatNode argument!");

  Id->SetBits(ASTStretchNode::StretchBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Stretch SymbolTable Entry!");

  ASTStretchNode* SN = new ASTStretchNode(Id, F);
  assert(SN && "Could not create a valid ASTStretchNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(SN, ASTTypeStretch), ASTTypeStretch);
  assert(STE->HasValue() && "Stretch SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  SN->Mangle();
  return SN;
}

ASTStretchNode*
ASTBuilder::CreateASTStretchNode(const ASTIdentifierNode* Id,
                                 const ASTDoubleNode* D) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(D && "Invalid ASTDoubleNode argument!");

  Id->SetBits(ASTStretchNode::StretchBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Stretch SymbolTable Entry!");

  ASTStretchNode* SN = new ASTStretchNode(Id, D);
  assert(SN && "Could not create a valid ASTStretchNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(SN, ASTTypeStretch), ASTTypeStretch);
  assert(STE->HasValue() && "Stretch SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  SN->Mangle();
  return SN;
}

ASTStretchNode*
ASTBuilder::CreateASTStretchNode(const ASTIdentifierNode* Id,
                                 const ASTMPIntegerNode* MPI) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(MPI && "Invalid ASTIdentifierNode argument!");

  Id->SetBits(ASTStretchNode::StretchBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Stretch SymbolTable Entry!");

  ASTStretchNode* SN = new ASTStretchNode(Id, MPI);
  assert(SN && "Could not create a valid ASTStretchNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(SN, ASTTypeStretch), ASTTypeStretch);
  assert(STE->HasValue() && "Stretch SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  SN->Mangle();
  return SN;
}

ASTStretchNode*
ASTBuilder::CreateASTStretchNode(const ASTIdentifierNode* Id,
                                 const ASTMPDecimalNode* MPD) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(MPD && "Invalid ASTIdentifierNode argument!");

  Id->SetBits(ASTStretchNode::StretchBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Stretch SymbolTable Entry!");

  ASTStretchNode* SN = new ASTStretchNode(Id, MPD);
  assert(SN && "Could not create a valid ASTStretchNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(SN, ASTTypeStretch), ASTTypeStretch);
  assert(STE->HasValue() && "Stretch SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  SN->Mangle();
  return SN;
}

ASTStretchNode*
ASTBuilder::CreateASTStretchNode(const ASTIdentifierNode* Id,
                                 const ASTBinaryOpNode* BOP) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(BOP && "Invalid ASTBinaryOpNode argument!");

  Id->SetBits(ASTStretchNode::StretchBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Stretch SymbolTable Entry!");

  ASTStretchNode* SN = new ASTStretchNode(Id, BOP);
  assert(SN && "Could not create a valid ASTStretchNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(SN, ASTTypeStretch), ASTTypeStretch);
  assert(STE->HasValue() && "Stretch SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  SN->Mangle();
  return SN;
}

ASTStretchNode*
ASTBuilder::CreateASTStretchNode(const ASTIdentifierNode* Id,
                                 const ASTUnaryOpNode* UOP) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(UOP && "Invalid ASTUnaryOpNode argument!");

  Id->SetBits(ASTStretchNode::StretchBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Stretch SymbolTable Entry!");

  ASTStretchNode* SN = new ASTStretchNode(Id, UOP);
  assert(SN && "Could not create a valid ASTStretchNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(SN, ASTTypeStretch), ASTTypeStretch);
  assert(STE->HasValue() && "Stretch SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  SN->Mangle();
  return SN;
}

ASTBoxExpressionNode*
ASTBuilder::CreateASTBoxExpressionNode(const ASTIdentifierNode* Id,
                                       const ASTStatementList& SL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  Id->SetBits(ASTDelayNode::DelayBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid Box SymbolTable Entry!");

  ASTBoxExpressionNode* BXN = new ASTBoxExpressionNode(Id, SL);
  assert(BXN && "Could not create a valid ASTBoxExpressionNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(BXN, ASTTypeBox), ASTTypeBox);
  assert(STE->HasValue() && "Box SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  BXN->Mangle();
  return BXN;
}

ASTBoxAsExpressionNode*
ASTBuilder::CreateASTBoxAsExpressionNode(const ASTIdentifierNode* Id,
                                         const ASTStatementList& SL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  Id->SetPolymorphicName("boxas");
  Id->SetBits(ASTBoxAsExpressionNode::BoxAsBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid BoxAs SymbolTable Entry!");

  ASTBoxAsExpressionNode* BXAN = new ASTBoxAsExpressionNode(Id, SL);
  assert(BXAN && "Could not create a valid ASTBoxAsExpressionNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(BXAN, ASTTypeBoxAs), ASTTypeBoxAs);
  assert(STE->HasValue() && "BoxAs SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  BXAN->Mangle();
  return BXAN;
}

ASTBoxToExpressionNode*
ASTBuilder::CreateASTBoxToExpressionNode(const ASTIdentifierNode* Id,
                                         const std::string& Time,
                                         const ASTStatementList& SL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  Id->SetPolymorphicName("boxto");
  Id->SetBits(ASTBoxToExpressionNode::BoxToBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id);
  assert(STE && "Could not retrieve a valid BoxTo SymbolTable Entry!");

  ASTBoxToExpressionNode* BXTN = new ASTBoxToExpressionNode(Id, Time, SL);
  assert(BXTN && "Could not create a valid ASTBoxToExpressionNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(BXTN, ASTTypeBoxTo), ASTTypeBoxTo);
  assert(STE->HasValue() && "BoxTo SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  BXTN->Mangle();
  return BXTN;
}

ASTGPhaseExpressionNode*
ASTBuilder::CreateASTGPhaseExpressionNode(const ASTIdentifierNode* GId,
                                          const ASTIdentifierNode* TId) {
  assert(GId && "Invalid GPhase ASTIdentifierNode argument!");
  assert(TId && "Invalid GPhase Target ASTIdentifierNode argument!");

  GId->SetPolymorphicName("gphase");
  GId->SetBits(ASTGPhaseExpressionNode::GPhaseBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(GId);
  assert(STE && "Could not retrieve a valid GPhase SymbolTable Entry!");

  ASTGPhaseExpressionNode* GPE = new ASTGPhaseExpressionNode(GId, TId);
  assert(GPE && "Could not create a valid ASTGPhaseExpressionNode!");

  GPE->GetIdentifier()->SetPolymorphicName("gphase");
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(GPE, ASTTypeGPhaseExpression),
                ASTTypeGPhaseExpression);
  assert(STE->HasValue() && "GPhase SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(GId)->SetSymbolTableEntry(STE);
  GPE->Mangle();
  return GPE;
}

ASTGPhaseExpressionNode*
ASTBuilder::CreateASTGPhaseExpressionNode(const ASTIdentifierNode* GId,
                                          const ASTBinaryOpNode* BOp) {
  assert(GId && "Invalid GPhase ASTIdentifierNode argument!");
  assert(BOp && "Invalid GPhase BinaryOpNode argument!");

  GId->SetPolymorphicName("gphase");
  GId->SetBits(ASTGPhaseExpressionNode::GPhaseBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(GId);
  assert(STE && "Could not retrieve a valid GPhase SymbolTable Entry!");

  ASTGPhaseExpressionNode* GPE = new ASTGPhaseExpressionNode(GId, BOp);
  assert(GPE && "Could not create a valid ASTGPhaseExpressionNode!");

  GPE->GetIdentifier()->SetPolymorphicName("gphase");
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(GPE, ASTTypeGPhaseExpression),
                ASTTypeGPhaseExpression);
  assert(STE->HasValue() && "GPhase SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(GId)->SetSymbolTableEntry(STE);
  GPE->Mangle();
  return GPE;
}

ASTGPhaseExpressionNode*
ASTBuilder::CreateASTGPhaseExpressionNode(const ASTIdentifierNode* GId,
                                          const ASTUnaryOpNode* UOp) {
  assert(GId && "Invalid GPhase ASTIdentifierNode argument!");
  assert(UOp && "Invalid GPhase UnaryOpNode argument!");

  GId->SetPolymorphicName("gphase");
  GId->SetBits(ASTGPhaseExpressionNode::GPhaseBits);
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(GId);
  assert(STE && "Could not retrieve a valid GPhase SymbolTable Entry!");

  ASTGPhaseExpressionNode* GPE = new ASTGPhaseExpressionNode(GId, UOp);
  assert(GPE && "Could not create a valid ASTGPhaseExpressionNode!");

  GPE->GetIdentifier()->SetPolymorphicName("gphase");
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(GPE, ASTTypeGPhaseExpression),
                ASTTypeGPhaseExpression);
  assert(STE->HasValue() && "GPhase SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(GId)->SetSymbolTableEntry(STE);
  GPE->Mangle();
  return GPE;
}

ASTGateQOpNode*
ASTBuilder::CreateASTGateControlStatement(const ASTIdentifierNode* Id,
                                          const ASTGateControlNode* GCN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(GCN && "Invalid ASTGateControlNode argument!");

  Id->SetPolymorphicName("ctrl");
  Id->SetBits(ASTGateOpNode::GateOpBits);
  ASTGateQOpNode* QOP = new ASTGateQOpNode(Id, GCN);
  assert(QOP && "Could not create a valid ASTGateQOpNode!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id,
                             ASTGateControlStmtNode::GateControlStmtBits,
                             ASTTypeGateControlStatement);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QOP, ASTTypeGateQOpNode), ASTTypeGateQOpNode);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  QOP->Mangle();
  return QOP;
}

ASTGateQOpNode*
ASTBuilder::CreateASTGateNegControlStatement(const ASTIdentifierNode* Id,
                                             const ASTGateNegControlNode* GNCN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(GNCN && "Invalid ASTGateNegControlNode argument!");

  Id->SetPolymorphicName("negctrl");
  Id->SetBits(ASTGateOpNode::GateOpBits);
  ASTGateQOpNode* QOP = new ASTGateQOpNode(Id, GNCN);
  assert(QOP && "Could not create a valid ASTGateQOpNode!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id,
                             ASTGateNegControlStmtNode::GateNegControlStmtBits,
                             ASTTypeGateNegControlStatement);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QOP, ASTTypeGateQOpNode), ASTTypeGateQOpNode);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  QOP->Mangle();
  return QOP;
}

ASTGateQOpNode*
ASTBuilder::CreateASTGateInverseStatement(const ASTIdentifierNode* Id,
                                          const ASTGateInverseNode* GIN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(GIN && "Invalid ASTGateInverseNode argument!");

  Id->SetBits(ASTGateOpNode::GateOpBits);
  ASTGateQOpNode* QOP = new ASTGateQOpNode(Id, GIN);
  assert(QOP && "Could not create a valid ASTGateQOpNode!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id,
                                      ASTGateInverseStmtNode::GateInverseStmtBits,
                                      ASTTypeGateInverseStatement);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QOP, ASTTypeGateQOpNode), ASTTypeGateQOpNode);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  QOP->Mangle();
  return QOP;
}

ASTGateQOpNode*
ASTBuilder::CreateASTGatePowerStatement(const ASTIdentifierNode* Id,
                                        const ASTGatePowerNode* GPN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(GPN && "Invalid ASTGatePowerNode argument!");

  Id->SetBits(ASTGateOpNode::GateOpBits);
  ASTGateQOpNode* QOP = new ASTGateQOpNode(Id, GPN);
  assert(QOP && "Could not create a valid ASTGateQOpNode!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id,
                                      ASTGatePowerStmtNode::GatePowerStmtBits,
                                      ASTTypeGatePowerStatement);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QOP, ASTTypeGateQOpNode), ASTTypeGateQOpNode);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  QOP->Mangle();
  return QOP;
}

ASTGateQOpNode*
ASTBuilder::CreateASTGateGPhaseStatement(const ASTIdentifierNode* Id,
                                         const ASTGateGPhaseExpressionNode* GEN) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(GEN && "Invalid ASTGateGPhaseExpressionNode argument!");

  Id->SetBits(ASTGateOpNode::GateOpBits);
  ASTGateQOpNode* QOP = new ASTGateQOpNode(Id, GEN);
  assert(QOP && "Could not create a valid ASTGateQOpNode!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id,
                                      ASTGPhaseStatementNode::GPhaseStmtBits,
                                      ASTTypeGateGPhaseStatement);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(QOP, ASTTypeGateQOpNode), ASTTypeGateQOpNode);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  QOP->Mangle();
  return QOP;
}

ASTFunctionCallNode*
ASTBuilder::CreateASTFunctionCallNode(const ASTIdentifierNode* Id,
                                      const ASTArgumentNodeList* ANL,
                                      const ASTAnyTypeList* ATL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(ANL && "Invalid ASTArgumentNodeList argument!");
  assert(ATL && "Invalid ASTAnyTypeList argument!");

  Id->SetBits(ASTFunctionCallNode::FunctionCallBits);
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, ASTFunctionDefinitionNode::FunctionBits,
                                      ASTTypeFunction);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");

  ASTFunctionDefinitionNode* FDN =
    STE->GetValue()->GetValue<ASTFunctionDefinitionNode*>();
  assert(FDN && "Invalid FunctionDefinitionNode obtained "
                "from the SymbolTable!");
  if (!FDN) {
    std::stringstream M;
    M << "Invalid FunctionDefinitionNode " << Id->GetName()
      << " obtained from the SymbolTable!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  }

  ASTExpressionList CEL;

  if (!ANL->Empty()) {
    for (ASTArgumentNodeList::const_iterator I = ANL->begin();
         I != ANL->end(); ++I) {
      if ((*I)->IsIdentifier()) {
        ASTIdentifierNode* AId =
          const_cast<ASTIdentifierNode*>((*I)->GetIdentifier());
        CEL.Append(AId);
      } else if ((*I)->IsIdentifierRef()) {
        ASTIdentifierRefNode* AIdR =
          const_cast<ASTIdentifierRefNode*>((*I)->GetIdentifierRef());
        CEL.Append(AIdR);
      } else if ((*I)->IsConstExpr()) {
        if (ASTExpressionNode* EN =
            const_cast<ASTExpressionNode*>((*I)->GetExpression())) {
          EN->Mangle();
          CEL.Append(EN);
        }
      } else if ((*I)->IsExpression()) {
        if (ASTExpressionNode* EN = (*I)->GetExpression()) {
          EN->Mangle();
          CEL.Append(EN);
        }
      }
    }
  }

  ASTIdentifierList QIL;

  if (!ATL->Empty()) {
    for (unsigned X = 0; X < ATL->Size(); ++X) {
      if (ATL->IsIdentifier(X)) {
        QIL.Append(const_cast<ASTIdentifierNode*>(ATL->GetIdentifier(X)));
      } else if (ATL->IsIdentifierRef(X)) {
        QIL.Append(const_cast<ASTIdentifierRefNode*>(ATL->GetIdentifierRef(X)));
      } else {
        std::stringstream M;
        M << "Only Identifier Literals are allowed for Quantum Arguments.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
        break;
      }
    }
  }

  std::stringstream CS;
  CS << "ast-function-call-expression-" << Id->GetName() << '-'
    << DIAGLineCounter::Instance().GetIdentifierLocation();
  ASTFunctionCallNode* FCN =
    ASTBuilder::Instance().CreateASTFunctionCallNode(CS.str(), FDN,
                                                     &CEL, &QIL);
  assert(FCN && "Could not create a valid FunctionCallNode!");

  FCN->Mangle();
  return FCN;
}

ASTFunctionCallNode*
ASTBuilder::CreateASTFunctionCallNode(const std::string& EId,
                                      const ASTFunctionDefinitionNode* FDN,
                                      const ASTExpressionList* EL) {
  assert(!EId.empty() && "Invalid Function Call Identifier argument!");
  assert(FDN && "Invalid ASTFunctionDefinition argument!");
  assert(EL && "Invalid ASTExpressionList argument!");

  ASTIdentifierNode* Id =
    ASTBuilder::Instance().CreateASTIdentifierNode(EId,
                                    ASTFunctionCallNode::FunctionCallBits,
                                    ASTTypeFunctionCallExpression);
  assert(Id && "Could not create a valid ASTFunctionCall ASTIdentifierNode!");

  Id->SetPolymorphicName(FDN->GetIdentifier()->GetName());
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id->GetName(),
                                      ASTFunctionCallNode::FunctionCallBits,
                                      ASTTypeFunctionCallExpression);
  assert(STE && "Could not retrieve a valid ASTFunctionCall SymbolTable Entry!");

  STE->ResetValue();
  ASTFunctionCallNode* FCN = new ASTFunctionCallNode(Id, FDN, EL);
  assert(FCN && "Could not create a valid Function ASTFunctionCallNode!");

  STE->SetValue(new ASTValue<>(FCN, ASTTypeFunctionCallExpression),
                ASTTypeFunctionCallExpression);
  assert(STE->HasValue() && "ASTFunctionCall SymbolTable Entry has no Value!");

  unsigned XI = 0U;
  for (ASTExpressionList::const_iterator I = EL->begin(); I != EL->end(); ++I) {
    const ASTExpressionNode* EXN = dynamic_cast<const ASTExpressionNode*>(*I);
    if (EXN) {
      const ASTIdentifierNode* XId = EXN->GetIdentifier();
      ASTSymbolTableEntry* XSTE = ASTSymbolTable::Instance().Lookup(XId);
      assert(XSTE && "Invalid SymbolTableEntry for function call argument!");

      ASTFunctionCallValidator::Instance().ValidateArgument(FDN, XSTE, XI,
                                                            XId->GetName(),
                                                            Id->GetPolymorphicName());
      const_cast<ASTExpressionNode*>(EXN)->Mangle();
      FCN->AddSymbolTableEntry(XSTE);
    } else {
      FCN->AddSymbolTableEntry(nullptr);
    }

    ++XI;
  }

  FCN->Mangle();
  Id->SetSymbolTableEntry(STE);
  return FCN;
}

ASTFunctionCallNode*
ASTBuilder::CreateASTFunctionCallNode(const std::string& EId,
                                      const ASTFunctionDefinitionNode* FDN,
                                      const ASTExpressionList* EL,
                                      const ASTIdentifierList* IL) {
  assert(!EId.empty() && "Invalid Function Call Identifier argument!");
  assert(FDN && "Invalid ASTFunctionDefinition argument!");
  assert(EL && "Invalid ASTExpressionList argument!");
  assert(IL && "Invalid ASTIdentifierList argument!");

  ASTIdentifierNode* Id =
    ASTBuilder::Instance().CreateASTIdentifierNode(EId,
                                    ASTFunctionCallNode::FunctionCallBits,
                                    ASTTypeFunctionCallExpression);
  assert(Id && "Could not create a valid ASTFunctionCall ASTIdentifierNode!");

  Id->SetPolymorphicName(FDN->GetIdentifier()->GetName());
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id->GetName(),
                                      ASTFunctionCallNode::FunctionCallBits,
                                      ASTTypeFunctionCallExpression);
  assert(STE &&
         "Could not retrieve a valid ASTFunctionCall SymbolTable Entry!");

  STE->ResetValue();
  ASTFunctionCallNode* FCN = new ASTFunctionCallNode(Id, FDN, EL, IL);
  assert(FCN && "Could not create a valid Function ASTFunctionCallNode!");

  STE->SetValue(new ASTValue<>(FCN, ASTTypeFunctionCallExpression),
                ASTTypeFunctionCallExpression);
  assert(STE->HasValue() && "ASTFunctionCall SymbolTable Entry has no Value!");

  unsigned XI = 0U;
  for (ASTExpressionList::const_iterator I = EL->begin(); I != EL->end(); ++I) {
    const ASTExpressionNode* EXN = dynamic_cast<const ASTExpressionNode*>(*I);
    if (EXN) {
      const ASTIdentifierNode* XId = EXN->GetIdentifier();
      ASTSymbolTableEntry* XSTE = ASTSymbolTable::Instance().Lookup(XId);
      assert(XSTE && "Invalid SymbolTableEntry for function call argument!");

      ASTFunctionCallValidator::Instance().ValidateArgument(FDN, XSTE, XI,
                                                            XId->GetName(),
                                                            Id->GetPolymorphicName());
      const_cast<ASTExpressionNode*>(EXN)->Mangle();
      FCN->AddSymbolTableEntry(XSTE);
    } else {
      FCN->AddSymbolTableEntry(nullptr);
    }

    ++XI;
  }

  for (ASTIdentifierList::const_iterator I = IL->begin(); I != IL->end(); ++I) {
    const ASTIdentifierNode* IId = *I;
    ASTSymbolTableEntry* XSTE = ASTSymbolTable::Instance().Lookup(IId);
    assert(XSTE && "Invalid SymbolTableEntry for function call argument!");

    ASTFunctionCallValidator::Instance().ValidateArgument(FDN, XSTE, XI,
                                                          IId->GetName(),
                                                          Id->GetPolymorphicName());
    FCN->AddSymbolTableEntry(XSTE);
    ++XI;
  }

  FCN->Mangle();
  Id->SetSymbolTableEntry(STE);
  return FCN;
}

ASTFunctionCallNode*
ASTBuilder::CreateASTFunctionCallNode(const std::string& EId,
                                      const ASTKernelNode* KN,
                                      const ASTExpressionList* EL) {
  assert(!EId.empty() && "Invalid Kernel Call Identifier argument!");
  assert(KN && "Invalid ASTKernelNode argument!");
  assert(EL && "Invalid ASTExpressionList argument!");

  ASTIdentifierNode* Id =
    ASTBuilder::Instance().CreateASTIdentifierNode(EId,
                                    ASTFunctionCallNode::FunctionCallBits,
                                    ASTTypeKernelCallExpression);
  assert(Id && "Could not create a valid ASTKernelCall ASTIdentifierNode!");

  Id->SetPolymorphicName(KN->GetIdentifier()->GetName());
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id->GetName(),
                                      ASTFunctionCallNode::FunctionCallBits,
                                      ASTTypeKernelCallExpression);
  assert(STE &&
         "Could not retrieve a valid ASTKernelCall SymbolTable Entry!");

  STE->ResetValue();
  ASTFunctionCallNode *FCN = new ASTFunctionCallNode(Id, KN, EL);
  assert(FCN && "Could not create a valid Kernel ASTFunctionCallNode!");

  STE->SetValue(new ASTValue<>(FCN, ASTTypeKernelCallExpression),
                ASTTypeKernelCallExpression);
  assert(STE->HasValue() && "ASTKernelCall SymbolTable Entry has no Value!");

  unsigned XI = 0U;
  for (ASTExpressionList::const_iterator I = EL->begin(); I != EL->end(); ++I) {
    const ASTExpressionNode* EXN = dynamic_cast<const ASTExpressionNode*>(*I);
    if (EXN) {
      const ASTIdentifierNode* XId = EXN->GetIdentifier();
      assert(XId && "Invalid ASTIdentifierNode for ASTExpressionNode argument!");

      const ASTSymbolTableEntry* XSTE = XId->GetSymbolTableEntry();
      if (!XSTE)
        XSTE = ASTSymbolTable::Instance().LookupLocal(XId);
      assert(XSTE && "Invalid SymbolTableEntry for function call argument!");

      ASTFunctionCallValidator::Instance().ValidateArgument(XSTE, XI,
                                                            XId->GetName(),
                                                            Id->GetPolymorphicName());
      const_cast<ASTExpressionNode*>(EXN)->Mangle();
      FCN->AddSymbolTableEntry(const_cast<ASTSymbolTableEntry*>(XSTE));
    } else {
      FCN->AddSymbolTableEntry(nullptr);
    }
    ++XI;
  }

  FCN->Mangle();
  Id->SetSymbolTableEntry(STE);
  return FCN;
}

ASTFunctionCallNode*
ASTBuilder::CreateASTFunctionCallNode(const std::string& EId,
                                      const ASTDefcalNode* DN,
                                      const ASTExpressionList* EL) {
  assert(!EId.empty() && "Invalid Defcal Call Identifier argument!");
  assert(DN && "Invalid ASTDefcalNode argument!");
  assert(EL && "Invalid ASTExpressionList argument!");

  ASTIdentifierNode* Id =
    ASTBuilder::Instance().CreateASTIdentifierNode(EId,
                                    ASTFunctionCallNode::FunctionCallBits,
                                    ASTTypeDefcalCallExpression);
  assert(Id && "Could not create a valid ASTDefcalCall ASTIdentifierNode!");

  Id->SetPolymorphicName(DN->GetIdentifier()->GetName());
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id->GetName(),
                                      ASTFunctionCallNode::FunctionCallBits,
                                      ASTTypeDefcalCallExpression);
  assert(STE &&
         "Could not retrieve a valid ASTDefcalCall SymbolTable Entry!");

  STE->ResetValue();
  ASTFunctionCallNode* FCN = new ASTFunctionCallNode(Id, DN, EL);
  assert(FCN && "Could not create a valid Defcal ASTFunctionCallNode!");

  STE->SetValue(new ASTValue<>(FCN, ASTTypeDefcalCallExpression),
                ASTTypeDefcalCallExpression);
  assert(STE->HasValue() && "ASTKernelCall SymbolTable Entry has no Value!");

  unsigned XI = 0U;
  for (ASTExpressionList::const_iterator I = EL->begin(); I != EL->end(); ++I) {
    const ASTExpressionNode* EXN = dynamic_cast<const ASTExpressionNode*>(*I);
    if (EXN) {
      const ASTIdentifierNode* XId = EXN->GetIdentifier();
      ASTSymbolTableEntry* XSTE = ASTSymbolTable::Instance().Lookup(XId);
      assert(XSTE && "Invalid SymbolTableEntry for function call argument!");

      ASTFunctionCallValidator::Instance().ValidateArgument(XSTE, XI,
                                                            XId->GetName(),
                                                            Id->GetPolymorphicName());
      const_cast<ASTExpressionNode*>(EXN)->Mangle();
      FCN->AddSymbolTableEntry(XSTE);
    } else {
      FCN->AddSymbolTableEntry(nullptr);
    }

    ++XI;
  }

  FCN->Mangle();
  Id->SetSymbolTableEntry(STE);
  return FCN;
}

ASTArrayNode*
ASTBuilder::CreateASTCBitArrayNode(const ASTIdentifierNode* Id,
                                   const ASTIntNode* IX) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IX && "Invalid ASTIntNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), ASTTypeCBitArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeCBitArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  unsigned Bits = ASTUtils::Instance().GetUnsignedValue(IX);
  assert(!ASTIdentifierNode::InvalidBits(Bits) &&
         "Invalid number of bits for ASTArray!");

  ASTArrayNode* ARN = new ASTCBitArrayNode(Id, Bits);
  assert(ARN && "Could not create a valid ASTCBitArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeCBitArray), ASTTypeCBitArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTCBitArrayNode(const ASTIdentifierNode* Id,
                                   unsigned Bits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(!ASTIdentifierNode::InvalidBits(Bits) &&
         "Invalid number of bits for ASTArray!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeCBitArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeCBitArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTCBitArrayNode(Id, Bits);
  assert(ARN && "Could not create a valid ASTCBitArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeCBitArray), ASTTypeCBitArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTCBitArrayNode(const ASTIdentifierNode* Id,
                                   const ASTIntNode* IX,
                                   const ASTIntNode* CX) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IX && "Invalid ASTIntNode Index argument!");
  assert(CX && "Invalid ASTIntNode Size argument!");

  unsigned Bits = ASTUtils::Instance().GetUnsignedValue(IX);
  unsigned CBits = ASTUtils::Instance().GetUnsignedValue(CX);

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeCBitArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeCBitArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTCBitArrayNode(Id, Bits, CBits);
  assert(ARN && "Could not create a valid ASTCBitArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeCBitArray), ASTTypeCBitArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTCBitArrayNode(const ASTIdentifierNode* Id,
                                   unsigned Bits, unsigned CBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeCBitArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeCBitArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTCBitArrayNode(Id, Bits, CBits);
  assert(ARN && "Could not create a valid ASTCBitArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeCBitArray), ASTTypeCBitArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTQubitArrayNode(const ASTIdentifierNode* Id,
                                    const ASTIntNode* IX) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IX && "Invalid ASTIntNode argument!");

  unsigned Size = ASTUtils::Instance().GetUnsignedValue(IX);
  assert(!ASTIdentifierNode::InvalidBits(Size) &&
         "Invalid Qubit Array size!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Size, ASTTypeQubitArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeQubitArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTQubitArrayNode(Id, Size);
  assert(ARN && "Could not create a valid ASTQubitArrayNode!");

  Id->SetBits(Size);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeQubitArray), ASTTypeQubitArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTQubitArrayNode(const ASTIdentifierNode* Id,
                                    unsigned Bits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeQubitArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeQubitArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTQubitArrayNode(Id, Bits);
  assert(ARN && "Could not create a valid ASTQubitArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeQubitArray), ASTTypeQubitArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTQubitArrayNode(const ASTIdentifierNode* Id,
                                    const ASTIntNode* IX,
                                    const ASTIntNode* QX) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IX && "Invalid ASTIntNode Index argument!");
  assert(QX && "Invalid ASTIntNode Size argument!");

  unsigned Size = ASTUtils::Instance().GetUnsignedValue(IX);
  unsigned QSize = ASTUtils::Instance().GetUnsignedValue(QX);

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Size, ASTTypeQubitArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeQubitArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTQubitArrayNode(Id, Size, QSize);
  assert(ARN && "Could not create a valid ASTQubitArrayNode!");

  Id->SetBits(Size);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeQubitArray), ASTTypeQubitArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTQubitArrayNode(const ASTIdentifierNode* Id,
                                    unsigned Bits, unsigned QBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeQubitArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeQubitArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTQubitArrayNode(Id, Bits, QBits);
  assert(ARN && "Could not create a valid ASTQubitArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeQubitArray), ASTTypeQubitArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTAngleArrayNode(const ASTIdentifierNode* Id,
                                    const ASTIntNode* IX) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IX && "Invalid ASTIntNode argument!");

  unsigned Bits = ASTUtils::Instance().GetUnsignedValue(IX);
  assert(!ASTIdentifierNode::InvalidBits(Bits) &&
         "Invalid Angle Array size!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeAngleArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeAngleArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTAngleArrayNode(Id, Bits);
  assert(ARN && "Could not create a valid ASTAngleArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeAngleArray), ASTTypeAngleArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTAngleArrayNode(const ASTIdentifierNode* Id,
                                    unsigned Bits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeAngleArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeAngleArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTAngleArrayNode(Id, Bits);
  assert(ARN && "Could not create a valid ASTAngleArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeAngleArray), ASTTypeAngleArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTAngleArrayNode(const ASTIdentifierNode* Id,
                                    const ASTIntNode* IX,
                                    const ASTIntNode* AX) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IX && "Invalid ASTIntNode Index argument!");
  assert(AX && "Invalid ASTIntNode Angle Size argument!");

  unsigned Bits = ASTUtils::Instance().GetUnsignedValue(IX);
  assert(!ASTIdentifierNode::InvalidBits(Bits) &&
         "Invalid Angle Array size!");
  unsigned ABits = ASTUtils::Instance().GetUnsignedValue(AX);
  assert(!ASTIdentifierNode::InvalidBits(ABits) && "Invalid Angle size!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeAngleArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeAngleArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTAngleArrayNode(Id, Bits, ABits);
  assert(ARN && "Could not create a valid ASTAngleArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeAngleArray), ASTTypeAngleArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTAngleArrayNode(const ASTIdentifierNode* Id,
                                    unsigned Bits, unsigned ABits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeAngleArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeAngleArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTAngleArrayNode(Id, Bits, ABits);
  assert(ARN && "Could not create a valid ASTAngleArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeAngleArray), ASTTypeAngleArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTBoolArrayNode(const ASTIdentifierNode* Id,
                                   const ASTIntNode* IX) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IX && "Invalid ASTIntNode argument!");

  unsigned Bits = ASTUtils::Instance().GetUnsignedValue(IX);
  assert(!ASTIdentifierNode::InvalidBits(Bits) &&
         "Invalid size for Bool array!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeBoolArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeBoolArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTBoolArrayNode(Id, Bits);
  assert(ARN && "Could not create a valid ASTBoolArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeBoolArray), ASTTypeBoolArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTBoolArrayNode(const ASTIdentifierNode* Id,
                                   unsigned Bits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeBoolArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeBoolArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTBoolArrayNode(Id, Bits);
  assert(ARN && "Could not create a valid ASTBoolArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeBoolArray), ASTTypeBoolArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTIntArrayNode(const ASTIdentifierNode* Id,
                                  const ASTIntNode* IX,
                                  bool Unsigned) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IX && "Invalid ASTIntNode argument!");

  unsigned Bits = ASTUtils::Instance().GetUnsignedValue(IX);
  assert(!ASTIdentifierNode::InvalidBits(Bits) &&
         "Invalid size for int array!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeIntArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeIntArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTIntArrayNode(Id, Bits, Unsigned);
  assert(ARN && "Could not create a valid ASTIntArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeIntArray), ASTTypeIntArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTIntArrayNode(const ASTIdentifierNode* Id,
                                  unsigned Bits,
                                  bool Unsigned) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeIntArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeIntArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTIntArrayNode(Id, Bits, Unsigned);
  assert(ARN && "Could not create a valid ASTIntArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeIntArray), ASTTypeIntArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTMPIntegerArrayNode(const ASTIdentifierNode* Id,
                                        const ASTIntNode* IX,
                                        const ASTIntNode* IIX,
                                        bool Unsigned) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IX && "Invalid ASTIntNode Index argument!");
  assert(IIX && "Invalid ASTIntNode MPInteger Size argument!");

  unsigned Bits = ASTUtils::Instance().GetUnsignedValue(IX);
  assert(!ASTIdentifierNode::InvalidBits(Bits) &&
         "Invalid size for mpint array!");

  unsigned IBits = ASTUtils::Instance().GetUnsignedValue(IIX);
  assert(!ASTIdentifierNode::InvalidBits(IBits) &&
         "Invalid bits for mpint!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeMPIntegerArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeMPIntegerArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTMPIntegerArrayNode(Id, Bits, IBits, Unsigned);
  assert(ARN && "Could not create a valid ASTMPIntegerArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeMPIntegerArray),
                ASTTypeMPIntegerArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTMPIntegerArrayNode(const ASTIdentifierNode* Id,
                                        unsigned Bits, unsigned MPBits,
                                        bool Unsigned) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeMPIntegerArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeMPIntegerArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTMPIntegerArrayNode(Id, Bits, MPBits, Unsigned);
  assert(ARN && "Could not create a valid ASTMPIntegerArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeMPIntegerArray),
                ASTTypeMPIntegerArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTFloatArrayNode(const ASTIdentifierNode* Id,
                                    const ASTIntNode* IX) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IX && "Invalid ASTIntNode argument!");

  unsigned Bits = ASTUtils::Instance().GetUnsignedValue(IX);
  assert(!ASTIdentifierNode::InvalidBits(Bits) &&
         "Invalid size for float array!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeFloatArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeFloatArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTFloatArrayNode(Id, Bits);
  assert(ARN && "Could not create a valid ASTFloatArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeFloatArray), ASTTypeFloatArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTFloatArrayNode(const ASTIdentifierNode* Id,
                                    unsigned Bits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeFloatArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeFloatArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTFloatArrayNode(Id, Bits);
  assert(ARN && "Could not create a valid ASTFloatArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeFloatArray), ASTTypeFloatArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTMPDecimalArrayNode(const ASTIdentifierNode* Id,
                                        const ASTIntNode* IX,
                                        const ASTIntNode* FX) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IX && "Invalid ASTIntNode Index argument!");
  assert(FX && "Invalid ASTIntNode MPDecimal Size argument!");

  unsigned Bits = ASTUtils::Instance().GetUnsignedValue(IX);
  assert(!ASTIdentifierNode::InvalidBits(Bits) &&
         "Invalid mpdecimal array size!");
  unsigned FBits = ASTUtils::Instance().GetUnsignedValue(FX);
  assert(!ASTIdentifierNode::InvalidBits(FBits) &&
         "Invalid mpdecimal bits!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeMPDecimalArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeMPDecimalArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTMPDecimalArrayNode(Id, Bits, FBits);
  assert(ARN && "Could not create a valid ASTMPDecimalArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeMPDecimalArray),
                ASTTypeMPDecimalArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTMPDecimalArrayNode(const ASTIdentifierNode* Id,
                                        unsigned Bits,
                                        unsigned MPBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeMPDecimalArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeMPDecimalArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTMPDecimalArrayNode(Id, Bits, MPBits);
  assert(ARN && "Could not create a valid ASTMPDecimalArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeMPDecimalArray),
                ASTTypeMPDecimalArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTMPComplexArrayNode(const ASTIdentifierNode* Id,
                                        const ASTIntNode* IX,
                                        const ASTIntNode* CX) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IX && "Invalid ASTIntNode Index argument!");
  assert(CX && "Invalid ASTIntNode MPComplex Size argument!");

  unsigned Bits = ASTUtils::Instance().GetUnsignedValue(IX);
  assert(!ASTIdentifierNode::InvalidBits(Bits) &&
         "Invalid mpcomplex array size!");
  unsigned CBits = ASTUtils::Instance().GetUnsignedValue(CX);
  assert(!ASTIdentifierNode::InvalidBits(Bits) &&
         "Invalid mpcomplex bits size!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeMPComplexArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeMPComplexArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTMPComplexArrayNode(Id, Bits, CBits);
  assert(ARN && "Could not create a valid ASTMPComplexArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeMPComplexArray),
                ASTTypeMPComplexArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTMPComplexArrayNode(const ASTIdentifierNode* Id,
                                        unsigned Bits,
                                        unsigned MPBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeMPComplexArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeMPComplexArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTMPComplexArrayNode(Id, Bits, MPBits);
  assert(ARN && "Could not create a valid ASTMPComplexArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeMPComplexArray),
                ASTTypeMPComplexArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTMPComplexArrayNode(const ASTIdentifierNode* Id,
                                        unsigned Size,
                                        unsigned Bits,
                                        const ASTComplexExpressionNode* CE) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(CE && "Invalid ASTComplexExpressionNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Size, ASTTypeMPComplexArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeMPComplexArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTMPComplexArrayNode(Id, Size, Bits, CE);
  assert(ARN && "Could not create a valid ASTMPComplexArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeMPComplexArray),
                ASTTypeMPComplexArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTDurationArrayNode(const ASTIdentifierNode* Id,
                                       const ASTIntNode* IX,
                                       const std::string& Length) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(IX && "Invalid ASTIntNode argument!");
  assert(!Length.empty() && "Invalid - empty - LengthUnit!");

  unsigned Bits = ASTUtils::Instance().GetUnsignedValue(IX);
  assert(!ASTIdentifierNode::InvalidBits(Bits) &&
         "Invalid duration array size!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeDurationArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeDurationArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTDurationArrayNode(Id, Bits, Length);
  assert(ARN && "Could not create a valid ASTDurationArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeDurationArray),
                ASTTypeDurationArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTDurationArrayNode(const ASTIdentifierNode* Id,
                                       unsigned Bits,
                                       const std::string& Length) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeDurationArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeDurationArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTDurationArrayNode(Id, Bits, Length);
  assert(ARN && "Could not create a valid ASTDurationArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeDurationArray),
                ASTTypeDurationArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTDurationArrayNode(const ASTIdentifierNode* Id,
                                       unsigned Bits,
                                       const ASTDurationOfNode* DON) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(DON && "Invalid ASTDurationOfNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeDurationArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeDurationArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTDurationArrayNode(Id, Bits, DON);
  assert(ARN && "Could not create a valid ASTDurationArrayNode!");

  Id->SetBits(Bits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeDurationArray),
                ASTTypeDurationArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTOpenPulseFrameArrayNode(const ASTIdentifierNode* Id,
                                             unsigned Size) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Size, ASTTypeOpenPulseFrameArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeOpenPulseFrameArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTOpenPulseFrameArrayNode(Id, Size);
  assert(ARN && "Could not create a valid ASTOpenPulseFrameArrayNode!");

  Id->SetBits(Size);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeOpenPulseFrameArray),
                ASTTypeOpenPulseFrameArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTOpenPulsePortArrayNode(const ASTIdentifierNode* Id,
                                            unsigned Size) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Size, ASTTypeOpenPulsePortArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeOpenPulsePortArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTOpenPulsePortArrayNode(Id, Size);
  assert(ARN && "Could not create a valid ASTOpenPulseFrameArrayNode!");

  Id->SetBits(Size);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeOpenPulsePortArray),
                ASTTypeOpenPulsePortArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTArrayNode*
ASTBuilder::CreateASTOpenPulseWaveformArrayNode(const ASTIdentifierNode* Id,
                                                unsigned Size) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Size, ASTTypeOpenPulseWaveformArray);
  if (!STE)
    STE = ASTSymbolTable::Instance().Lookup(Id, 0, ASTTypeOpenPulseWaveformArray);
  assert(STE && "Could not retrieve a valid SymbolTable Entry!");

  ASTArrayNode* ARN = new ASTOpenPulseWaveformArrayNode(Id, Size);
  assert(ARN && "Could not create a valid ASTOpenPulseFrameArrayNode!");

  Id->SetBits(Size);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(ARN, ASTTypeOpenPulseWaveformArray),
                ASTTypeOpenPulseWaveformArray);
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return ARN;
}

ASTIdentifierRefNode*
ASTBuilder::CreateArrayASTIdentifierRefNode(const ASTIdentifierNode* Id,
                                            ASTType AType,
                                            ASTSymbolTableEntry* STE,
                                            unsigned Index) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(STE && "Invalid SymbolTable Entry argument!");
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  if (Index >= Id->GetBits()) {
    std::stringstream M;
    M << "Array Index attempts to read past the end of the Array!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  std::stringstream S;
  S << Id->GetName() << '[' << Index << ']';

  ASTArrayNode* ARN = STE->GetValue()->GetValue<ASTArrayNode*>();
  assert(ARN && "Could not retrieve a valid ASTArrayNode from the "
                "SymbolTable Entry!");

  union {
    ASTArrayNode* AN;
    ASTCBitArrayNode* CB;
    ASTCBitNArrayNode* CBN;
    ASTQubitArrayNode* QB;
    ASTQubitNArrayNode* QBN;
    ASTAngleArrayNode* ANG;
    ASTBoolArrayNode* BOOL;
    ASTIntArrayNode* INT;
    ASTFloatArrayNode* FLT;
    ASTMPIntegerArrayNode* MPI;
    ASTMPDecimalArrayNode* MPD;
    ASTMPComplexArrayNode* MPC;
    ASTDurationArrayNode* DUR;
  } U;

  ASTIdentifierRefNode* IdR = nullptr;
  U.AN = nullptr;

  switch (AType) {
  case ASTTypeAngleArray:
    U.ANG = dynamic_cast<ASTAngleArrayNode*>(ARN);
    assert(U.ANG && "Could not dynamic_cast to an ASTAngleArrayNode!");
    assert(Index < U.ANG->Size() && "Index is past ASTAngleArrayNode end!");

    IdR = new ASTIdentifierRefNode(S.str(), Id, Index);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
    break;
  case ASTTypeBoolArray:
    U.BOOL = dynamic_cast<ASTBoolArrayNode*>(ARN);
    assert(U.BOOL && "Could not dynamic_cast to an ASTBoolArrayNode!");
    assert(Index < U.BOOL->Size() && "Index is past ASTBoolArrayNode end!");

    IdR = new ASTIdentifierRefNode(S.str(), Id, Index);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
    break;
  case ASTTypeCBitArray:
    U.CB = dynamic_cast<ASTCBitArrayNode*>(ARN);
    assert(U.CB && "Could not dynamic_cast to an ASTCBitArrayNode!");
    assert(Index < U.CB->Size() && "Index is past ASTCBitArrayNode end!");

    IdR = new ASTIdentifierRefNode(S.str(), Id, Index);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
    break;
  case ASTTypeCBitNArray:
    U.CBN = dynamic_cast<ASTCBitNArrayNode*>(ARN);
    assert(U.CBN && "Could not dynamic_cast to an ASTCBitNArrayNode!");
    assert(Index < U.CBN->Size() && "Index is past ASTCBitNArrayNode end!");

    IdR = new ASTIdentifierRefNode(S.str(), Id, Index);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
    break;
  case ASTTypeFloatArray:
    U.FLT = dynamic_cast<ASTFloatArrayNode*>(ARN);
    assert(U.FLT && "Could not dynamic_cast to an ASTFloatArrayNode!");
    assert(Index < U.FLT->Size() && "Index is past ASTFloatArrayNode end!");

    IdR = new ASTIdentifierRefNode(S.str(), Id, Index);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
    break;
  case ASTTypeIntArray:
    U.INT = dynamic_cast<ASTIntArrayNode*>(ARN);
    assert(U.INT && "Could not dynamic_cast to an ASTIntArrayNode!");
    assert(Index < U.INT->Size() && "Index is past ASTIntArrayNode end!");

    IdR = new ASTIdentifierRefNode(S.str(), Id, Index);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
    break;
  case ASTTypeDurationArray:
    U.DUR = dynamic_cast<ASTDurationArrayNode*>(ARN);
    assert(U.DUR && "Could not dynamic_cast to an ASTDurationArrayNode!");
    assert(Index < U.DUR->Size() && "Index is past ASTDurationArrayNode end!");

    IdR = new ASTIdentifierRefNode(S.str(), Id, Index);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
    break;
  case ASTTypeMPDecimalArray:
    U.MPD = dynamic_cast<ASTMPDecimalArrayNode*>(ARN);
    assert(U.MPD && "Could not dynamic_cast to an ASTMPDecimalArrayNode!");
    assert(Index < U.MPD->Size() && "Index is past ASTMPDecimalArrayNode end!");

    IdR = new ASTIdentifierRefNode(S.str(), Id, Index);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
    break;
  case ASTTypeMPComplexArray:
    U.MPC = dynamic_cast<ASTMPComplexArrayNode*>(ARN);
    assert(U.MPC && "Could not dynamic_cast to an ASTMPComplexArrayNode!");
    assert(Index < U.MPC->Size() && "Index is past ASTMPComplexArrayNode end!");

    IdR = new ASTIdentifierRefNode(S.str(), Id, Index);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
    break;
  case ASTTypeMPIntegerArray:
    U.MPI = dynamic_cast<ASTMPIntegerArrayNode*>(ARN);
    assert(U.MPI && "Could not dynamic_cast to an ASTMPIntegerArrayNode!");
    assert(Index < U.MPI->Size() && "Index is past ASTMPIntegerArrayNode end!");

    IdR = new ASTIdentifierRefNode(S.str(), Id, Index);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
    break;
  case ASTTypeQubitArray:
    U.QB = dynamic_cast<ASTQubitArrayNode*>(ARN);
    assert(U.QB && "Could not dynamic_cast to an ASTQubitArrayNode!");
    assert(Index < U.QB->Size() && "Index is past ASTQubitArrayNode end!");

    IdR = new ASTIdentifierRefNode(S.str(), Id, Index);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
    break;
  case ASTTypeQubitNArray:
    U.QBN = dynamic_cast<ASTQubitNArrayNode*>(ARN);
    assert(U.QBN && "Could not dynamic_cast to an ASTQubitNArrayNode!");
    assert(Index < U.QBN->Size() && "Index is past ASTQubitNArrayNode end!");

    IdR = new ASTIdentifierRefNode(S.str(), Id, Index);
    assert(IdR && "Could not create a valid ASTIdentifierRefNode!");
    break;
  default:
    U.AN = nullptr;
    IdR = nullptr;
    break;
  }

  if (!U.AN || !IdR) {
    std::stringstream M;
    M << "Impossible Cast to an " << PrintTypeEnum(AType) << "!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  if (!ASTSymbolTable::Instance().Insert(IdR, STE)) {
    std::stringstream M;
    M << "Could not insert a SymbolTableEntry for the RValue "
      << "ASTIdentifierRefNode!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return nullptr;
  }

  return IdR;
}

ASTEllipsisNode*
ASTBuilder::CreateASTEllipsisNode(const ASTIdentifierNode* Id) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), Id->GetSymbolType());
  if (!STE) {
    STE = new ASTSymbolTableEntry(Id, ASTTypeEllipsis);
    assert(STE && "Could not create a valid SymbolTable Entry!");
  }

  ASTEllipsisNode* EN = new ASTEllipsisNode(Id);
  assert(EN && "Could not create a valid ASTEllipsisNode!");

  STE->ResetValue();
  STE->SetValue(new ASTValue<>(EN, ASTTypeEllipsis), ASTTypeEllipsis);
  assert(STE->HasValue() && "Ellipsis SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return EN;
}

OpenPulse::ASTOpenPulseFrameNode*
ASTBuilder::CreateASTOpenPulseFrameNode(const ASTIdentifierNode* Id,
                                        const ASTExpressionList* EL,
                                        bool Extern) {
  assert(Id && "Invalid ASTIdentifierNode argument!");
  assert(EL && "Invalid ASTExpressionList argument!");

  ASTType ITy = Id->GetSymbolTableEntry()->GetValueType();
  unsigned FBits = OpenPulse::ASTOpenPulseFrameNode::FrameBits;

  if (EL->Empty()) {
    ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id, FBits, ITy);
    assert(STE && "OpenPulse Frame has no SymbolTable Value!");

    OpenPulse::ASTOpenPulseFrameNode* FN =
      new OpenPulse::ASTOpenPulseFrameNode(Id, nullptr, nullptr,
                                           nullptr, Extern);
    assert(FN && "Could not create a valid ASTOpenPulseFrameNode!");

    Id->SetBits(OpenPulse::ASTOpenPulseFrameNode::FrameBits);
    STE->ResetValue();
    STE->SetValue(new ASTValue<>(FN, ASTTypeOpenPulseFrame),
                  ASTTypeOpenPulseFrame);
    assert(STE->HasValue() && "FrameNode SymbolTable Entry has no Value!");

    FN->Mangle();
    const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);

    if (ASTDeclarationContextTracker::Instance().InCalibrationContext()) {
      if (!ASTSymbolTable::Instance().InsertToCalibrationTable(Id, STE)) {
        std::stringstream M;
        M << "Failure inserting " << Id->GetName() << " into the "
          << "calibration table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
        return nullptr;
      }
    }

    return FN;
  }

  if (EL->Size() < 3 || EL->Size() > 5) {
    std::stringstream M;
    M << "Invalid number of arguments for OpenPulse Frame creation.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTExpressionList::const_iterator I = EL->begin();

  const ASTExpressionNode* IEN = dynamic_cast<const ASTExpressionNode*>(*I);
  assert(IEN && "Could not dynamic_cast to a valid ASTExpressionNode!");

  const ASTIdentifierNode* PId = IEN->GetIdentifier();
  if (!PId) {
    std::stringstream M;
    M << "Invalid first element for the frame initialization "
      << "ExpressionList.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(*I), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  if (!PId) {
    std::stringstream M;
    M << "Invalid first element for the frame initialization "
      << "ExpressionList.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(*I), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTScopeController::Instance().CheckOutOfScope(PId);
  const unsigned PBits = OpenPulse::ASTOpenPulsePortNode::PortBits;
  ASTSymbolTableEntry* STE =
    ASTSymbolTable::Instance().Lookup(PId, PBits, ASTTypeOpenPulsePort);
  assert(STE && "Could not obtain a valid OpenPulse Port SymbolTable Entry!");
  assert(STE->HasValue() && "OpenPulse Port SymbolTable Entry has no Value!");

  OpenPulse::ASTOpenPulsePortNode* PN =
    STE->GetValue()->GetValue<OpenPulse::ASTOpenPulsePortNode*>();
  if (!PN) {
    std::stringstream M;
    M << "Unable to obtain a valid ASTOpenPulsePortNode from the SymbolTable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(*I), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ++I;
  ASTMPDecimalNode* MPD = nullptr;

  if ((*I)->GetASTType() == ASTTypeIdentifier) {
    const ASTIdentifierNode* DId = (*I)->GetIdentifier();
    assert(DId && "Could not dynamic_cast to a valid ASTIdentifierNode!");

    MPD = OpenPulse::ASTOpenPulseFrameNodeResolver::Instance().ResolveFrequency(DId);
    if (!MPD) {
      std::stringstream M;
      M << "Unable to resolve a valid frame ASTMPDecimalNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(*I), M.str(),
                                                     DiagLevel::Error);
      return nullptr;
    }
  } else if ((*I)->GetASTType() == ASTTypeIdentifierRef) {
    ASTIdentifierRefNode* DRId = dynamic_cast<ASTIdentifierRefNode*>((*I));
    assert(DRId && "Could not dynamic_cast to a valid ASTIdentifierRefNode!");

    MPD = OpenPulse::ASTOpenPulseFrameNodeResolver::Instance().ResolveFrequency(DRId);
    if (!MPD) {
      std::stringstream M;
      M << "Unable to resolve a valid frame ASTMPDecimalNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(*I), M.str(), DiagLevel::Error);
      return nullptr;
    }
  } else {
    ASTExpressionNode* EN = dynamic_cast<ASTExpressionNode*>(*I);
    assert(EN && "Could not dynamic_cast to a valid ASTExpressionNode!");

    MPD = OpenPulse::ASTOpenPulseFrameNodeResolver::Instance().ResolveFrequency(EN);
    if (!MPD) {
      std::stringstream M;
      M << "Unable to resolve a valid frame ASTMPDecimalNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(*I), M.str(), DiagLevel::Error);
      return nullptr;
    }
  }

  ASTSymbolTableEntry* FSTE =
    ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(), Id->GetSymbolType());
  assert(FSTE && "Could not obtain valid frame ASTSymbolTableEntry!");

  ++I;
  ASTExpressionNode* EN = dynamic_cast<ASTExpressionNode*>(*I);
  assert(EN && "Could not dynamic_cast to a valid ASTExpressionNode!");

  ASTAngleNode* AN =
    OpenPulse::ASTOpenPulseFrameNodeResolver::Instance().ResolveAngle(EN);
  if (!AN) {
    std::stringstream M;
    M << "Unable to resolve a valid frame ASTAngleNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(*I), M.str(),
                                                   DiagLevel::Error);
    return nullptr;
  }

  ASTDurationNode* DN = nullptr;
  if (EL->Size() == 4) {
    ++I;
    EN = dynamic_cast<ASTExpressionNode*>(*I);
    assert(EN && "Could not dynamic_cast to a valid ASTExpressionNode!");

    DN = OpenPulse::ASTOpenPulseFrameNodeResolver::Instance().ResolveDuration(EN);
    if (!DN) {
      std::stringstream M;
      M << "Unable to resolve a valid frame ASTDurationNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
      return nullptr;
    }
  } else if (EL->Size() == 5) {
    ++I;
    ASTExpressionNode* E0 = dynamic_cast<ASTExpressionNode*>(*I);
    assert(E0 && "Could not dynamic_cast to a valid ASTExpressionNode!");

    ++I;
    ASTExpressionNode* E1 = dynamic_cast<ASTExpressionNode*>(*I);
    assert(E1 && "Could not dynamic_cast to a valid ASTExpressionNode!");

    DN =
      OpenPulse::ASTOpenPulseFrameNodeResolver::Instance().ResolveDuration(E0, E1);
    if (!DN) {
      std::stringstream M;
      M << "Unable to resolve a valid frame ASTDurationNode.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
      return nullptr;
    }
  }

  OpenPulse::ASTOpenPulseFrameNode* FN = nullptr;

  if (EL->Size() == 3)
    FN = new OpenPulse::ASTOpenPulseFrameNode(Id, PN, MPD, AN, Extern);
  else if (EL->Size() == 4 || EL->Size() == 5)
    FN = new OpenPulse::ASTOpenPulseFrameNode(Id, PN, MPD, AN, DN, Extern);

  assert(FN && "Could not create a valid ASTOpenPulseFrameNode!");

  FSTE = ASTSymbolTable::Instance().Lookup(Id, Id->GetBits(),
                                           Id->GetSymbolType());
  assert(FSTE && "OpenPulse Frame has no SymbolTable Value!");

  Id->SetBits(OpenPulse::ASTOpenPulseFrameNode::FrameBits);
  FSTE->ResetValue();
  FSTE->SetValue(new ASTValue<>(FN, ASTTypeOpenPulseFrame),
                 ASTTypeOpenPulseFrame);
  assert(FSTE->HasValue() && "FrameNode SymbolTable Entry has no Value!");

  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(FSTE);
  FN->Mangle();

  if (ASTDeclarationContextTracker::Instance().InCalibrationContext()) {
    if (!ASTSymbolTable::Instance().InsertToCalibrationTable(Id, FSTE)) {
      std::stringstream M;
      M << "Failure inserting into the calibration symbol table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return nullptr;
    }
  }

  return FN;
}

OpenPulse::ASTOpenPulsePortNode*
ASTBuilder::CreateASTOpenPulsePortNode(const ASTIdentifierNode* Id,
                                       bool Extern) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTType ITy = Id->GetSymbolTableEntry()->GetValueType();
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id,
                             OpenPulse::ASTOpenPulsePortNode::PortBits, ITy);
  assert(STE && "Could not obtain a valid SymbolTable Entry for OpenPulse port!");

  OpenPulse::ASTOpenPulsePortNode* PN =
    new OpenPulse::ASTOpenPulsePortNode(Id, Extern);
  assert(PN && "Could not create a valid ASTOpenPulsePortNode!");

  Id->SetBits(OpenPulse::ASTOpenPulsePortNode::PortBits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(PN, ASTTypeOpenPulsePort), ASTTypeOpenPulsePort);
  assert(STE->HasValue() && "SymbolTable Entry for OpenPulse port has no Value!");

  PN->Mangle();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);

  if (ASTDeclarationContextTracker::Instance().InCalibrationContext()) {
    if (!ASTSymbolTable::Instance().InsertToCalibrationTable(Id, STE)) {
      std::stringstream M;
      M << "Failure inserting " << Id->GetName() << " into the "
        << "calibration table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return nullptr;
    }
  }

  return PN;
}

OpenPulse::ASTOpenPulsePortNode*
ASTBuilder::CreateASTOpenPulsePortNode(const ASTIdentifierNode* Id,
                                       uint64_t PId,
                                       bool Extern) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTType ITy = Id->GetSymbolTableEntry()->GetValueType();
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id,
                             OpenPulse::ASTOpenPulsePortNode::PortBits, ITy);
  assert(STE && "Could not obtain a valid SymbolTable Entry for OpenPulse port!");

  OpenPulse::ASTOpenPulsePortNode* PN =
    new OpenPulse::ASTOpenPulsePortNode(Id, PId, Extern);
  assert(PN && "Could not create a valid ASTOpenPulsePortNode!");

  Id->SetBits(OpenPulse::ASTOpenPulsePortNode::PortBits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(PN, ASTTypeOpenPulsePort), ASTTypeOpenPulsePort);
  assert(STE->HasValue() && "SymbolTable Entry for OpenPulse port has no Value!");

  PN->Mangle();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);

  if (ASTDeclarationContextTracker::Instance().InCalibrationContext()) {
    if (!ASTSymbolTable::Instance().InsertToCalibrationTable(Id, STE)) {
      std::stringstream M;
      M << "Failure inserting " << Id->GetName() << " into the "
        << "calibration table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return nullptr;
    }
  }

  return PN;
}

OpenPulse::ASTOpenPulseWaveformNode*
ASTBuilder::CreateASTOpenPulseWaveformNode(const ASTIdentifierNode* Id,
                                           const ASTMPComplexList& CXL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTType ITy = Id->GetSymbolTableEntry()->GetValueType();
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id,
                             OpenPulse::ASTOpenPulseWaveformNode::WaveformBits,
                             ITy);
  if (!STE) {
    STE = new ASTSymbolTableEntry(Id, ASTTypeOpenPulseWaveform);
    assert(STE && "Could not create a valid ASTSymbolTable Entry!");
  }

  OpenPulse::ASTOpenPulseWaveformNode* WFN =
    new OpenPulse::ASTOpenPulseWaveformNode(Id, CXL);
  assert(WFN && "Could not create a valid OpenPulse WaveformNode!");

  Id->SetBits(OpenPulse::ASTOpenPulseWaveformNode::WaveformBits);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(WFN, ASTTypeOpenPulseWaveform),
                ASTTypeOpenPulseWaveform);
  assert(STE->HasValue() && "OpenPulseWaveformNode SymbolTable Entry has "
                            "no Value!");

  WFN->Mangle();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);

  if (ASTDeclarationContextTracker::Instance().InCalibrationContext()) {
    if (!ASTSymbolTable::Instance().InsertToCalibrationTable(Id, STE)) {
      std::stringstream M;
      M << "Failure inserting " << Id->GetName() << " into the "
        << "calibration table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return nullptr;
    }
  }

  return WFN;
}

OpenPulse::ASTOpenPulsePlayNode*
ASTBuilder::CreateASTOpenPulsePlayNode(const ASTIdentifierNode* Id,
                              const OpenPulse::ASTOpenPulseWaveformNode* W,
                              const OpenPulse::ASTOpenPulseFrameNode* F) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTType ITy = Id->GetSymbolTableEntry()->GetValueType();
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id,
                             OpenPulse::ASTOpenPulsePlayNode::PlayBits, ITy);
  assert(STE && "PlayNode Identifier has no SymbolTable Entry!");

  OpenPulse::ASTOpenPulsePlayNode* PN =
    new OpenPulse::ASTOpenPulsePlayNode(Id, W, F);
  assert(PN && "Could not create a valid ASTOpenPulseFrameNode!");

  Id->SetBits(OpenPulse::ASTOpenPulsePlayNode::PlayBits);
  const_cast<ASTIdentifierNode*>(Id)->SetLocalScope();
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(PN, ASTTypeOpenPulsePlay), ASTTypeOpenPulsePlay);
  assert(STE->HasValue() && "PlayNode SymbolTable Entry has no Value!");

  STE->SetLocalScope();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return PN;
}

OpenPulse::ASTOpenPulsePlayNode*
ASTBuilder::CreateASTOpenPulsePlayNode(const ASTIdentifierNode* Id,
                                       const ASTFunctionCallNode* C,
                                       const OpenPulse::ASTOpenPulseFrameNode* F) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTType ITy = Id->GetSymbolTableEntry()->GetValueType();
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id,
                             OpenPulse::ASTOpenPulsePlayNode::PlayBits, ITy);
  assert(STE && "PlayNode Identifier has no SymbolTable Entry!");

  OpenPulse::ASTOpenPulsePlayNode* PN =
    new OpenPulse::ASTOpenPulsePlayNode(Id, C, F);
  assert(PN && "Could not create a valid ASTOpenPulseFrameNode!");

  Id->SetBits(OpenPulse::ASTOpenPulsePlayNode::PlayBits);
  const_cast<ASTIdentifierNode*>(Id)->SetLocalScope();
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(PN, ASTTypeOpenPulsePlay), ASTTypeOpenPulsePlay);
  assert(STE->HasValue() && "PlayNode SymbolTable Entry has no Value!");

  STE->SetLocalScope();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return PN;
}


OpenPulse::ASTOpenPulsePlayNode*
ASTBuilder::CreateASTOpenPulsePlayNode(const ASTIdentifierNode* Id,
                                       const ASTMPComplexList& CXL,
                                       const OpenPulse::ASTOpenPulseFrameNode* F) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTType ITy = Id->GetSymbolTableEntry()->GetValueType();
  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id,
                             OpenPulse::ASTOpenPulsePlayNode::PlayBits, ITy);
  assert(STE && "PlayNode Identifier has no SymbolTable Entry!");

  OpenPulse::ASTOpenPulsePlayNode* PN =
    new OpenPulse::ASTOpenPulsePlayNode(Id, CXL, F);
  assert(PN && "Could not create a valid ASTOpenPulseFrameNode!");

  Id->SetBits(OpenPulse::ASTOpenPulsePlayNode::PlayBits);
  const_cast<ASTIdentifierNode*>(Id)->SetLocalScope();
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(PN, ASTTypeOpenPulsePlay), ASTTypeOpenPulsePlay);
  assert(STE->HasValue() && "PlayNode SymbolTable Entry has no Value!");

  STE->SetLocalScope();
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  return PN;
}

OpenPulse::ASTOpenPulseCalibration*
ASTBuilder::CreateASTOpenPulseCalibration(const ASTIdentifierNode* Id,
                                          const ASTStatementList& OSL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTSymbolTableEntry* STE = ASTSymbolTable::Instance().Lookup(Id,
                             OpenPulse::ASTOpenPulseCalibration::CalibrationBits,
                             ASTTypeOpenPulseCalibration);
  assert(STE && "CalibrationNode Identifier has no SymbolTable Entry!");

  OpenPulse::ASTOpenPulseCalibration* CAL =
    new OpenPulse::ASTOpenPulseCalibration(Id, OSL);
  assert(CAL && "Could not create a valid ASTOpenPulseCalibration!");

  const ASTDeclarationContext* DCX =
    ASTDeclarationContextTracker::Instance().GetDefaultCalibrationContext();
  assert(DCX && "Could not obtain a valid default calibration context!");

  const_cast<ASTIdentifierNode*>(Id)->SetDeclarationContext(DCX);
  STE->ResetValue();
  STE->SetValue(new ASTValue<>(CAL, ASTTypeOpenPulseCalibration),
                ASTTypeOpenPulseCalibration);
  assert(STE->HasValue() && "CalibrationNode SymbolTable Entry has no Value!");

  STE->SetContext(DCX);
  const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(STE);
  CAL->SetCalibrationContext();
  return CAL;
}

} // namespace QASM

