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

#include <qasm/AST/ASTAngleNodeBuilder.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTFunctionCallArgument.h>
#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTGPhase.h>
#include <qasm/AST/ASTGateContextBuilder.h>
#include <qasm/AST/ASTGateFockControl.h>
#include <qasm/AST/ASTGateNodeBuilder.h>
#include <qasm/AST/ASTGateOpList.h>
#include <qasm/AST/ASTGateQubitTracker.h>
#include <qasm/AST/ASTGateTemplateParamBuilder.h>
#include <qasm/AST/ASTGates.h>
#include <qasm/AST/ASTIdentifierBuilder.h>
#include <qasm/AST/ASTImplicitConversionExpr.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTUtils.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <set>
#include <sstream>
#include <string>

namespace QASM {

ASTGateQOpList ASTGateQOpList::EmptyDefault;

ASTGateContextBuilder ASTGateContextBuilder::GCB;
bool ASTGateContextBuilder::GCS = false;
unsigned ASTGateContextBuilder::CMDepth = 0U;
const ASTToken *ASTGateContextBuilder::CMTok = nullptr;

ASTGateTemplateParamBuilder ASTGateTemplateParamBuilder::TPB;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

namespace {

bool IsDispGateIdentifier(const ASTIdentifierNode *Id) {
  return Id &&
         (Id->GetName() == u8"disp" || Id->GetSymbolType() == ASTTypeDispGate);
}

bool ExpressionInvolvesComplex(const ASTExpressionNode *EN) {
  if (!EN)
    return false;

  switch (EN->GetASTType()) {
  case ASTTypeMPComplex:
  case ASTTypeComplexExpression:
    return true;
  case ASTTypeIdentifier:
  case ASTTypeIdentifierRef: {
    const ASTIdentifierNode *XId = EN->GetIdentifier();
    if (!XId)
      return false;
    if (XId->GetSymbolType() == ASTTypeMPComplex ||
        XId->GetSymbolType() == ASTTypeMPComplexArray)
      return true;
    if (const ASTSymbolTableEntry *STE = XId->GetSymbolTableEntry())
      return STE->GetValueType() == ASTTypeMPComplex ||
             STE->GetValueType() == ASTTypeMPComplexArray;
    return false;
  }
  case ASTTypeBinaryOp: {
    const ASTBinaryOpNode *B = dynamic_cast<const ASTBinaryOpNode *>(EN);
    return B && (ExpressionInvolvesComplex(B->GetLeft()) ||
                 ExpressionInvolvesComplex(B->GetRight()));
  }
  case ASTTypeUnaryOp: {
    const ASTUnaryOpNode *U = dynamic_cast<const ASTUnaryOpNode *>(EN);
    return U && ExpressionInvolvesComplex(U->GetExpression());
  }
  case ASTTypeOpndTy: {
    const ASTOperandNode *Op = dynamic_cast<const ASTOperandNode *>(EN);
    if (!Op)
      return false;
    if (Op->IsIdentifier()) {
      const ASTIdentifierNode *TId = Op->GetTargetIdentifier();
      if (!TId)
        return false;
      if (TId->GetSymbolType() == ASTTypeMPComplex ||
          TId->GetSymbolType() == ASTTypeMPComplexArray)
        return true;
      if (const ASTSymbolTableEntry *STE = TId->GetSymbolTableEntry())
        return STE->GetValueType() == ASTTypeMPComplex ||
               STE->GetValueType() == ASTTypeMPComplexArray;
      return false;
    }
    return ExpressionInvolvesComplex(Op->GetExpression());
  }
  default:
    return false;
  }
}

bool PreferComplexGateParam(const ASTIdentifierNode *GateId,
                            const ASTExpressionNode *EN) {
  return IsDispGateIdentifier(GateId) || ExpressionInvolvesComplex(EN);
}

} // namespace

ASTMPComplexNode *
ASTGateNode::MaterializeComplexParamExpr(unsigned Index,
                                         const ASTExpressionNode *EN) {
  assert(EN && "Invalid expression for complex gate Param!");

  std::stringstream SCN;
  SCN << ASTDemangler::TypeName(ASTTypeMPComplex) << Index;
  ASTIdentifierNode *CId = ASTBuilder::Instance().CreateASTIdentifierNode(
      SCN.str(), ASTMPComplexNode::DefaultBits, ASTTypeMPComplex);
  assert(CId && "Could not create a Complex ASTIdentifierNode!");

  std::stringstream CES;
  CES << "complexexpression" << Index;
  ASTIdentifierNode *CEId = ASTBuilder::Instance().CreateASTIdentifierNode(
      CES.str(), ASTMPComplexNode::DefaultBits, ASTTypeComplexExpression);
  assert(CEId && "Could not create a ComplexExpression ASTIdentifierNode!");

  ASTComplexExpressionNode *CE = nullptr;
  if (const ASTBinaryOpNode *BOP = dynamic_cast<const ASTBinaryOpNode *>(EN))
    CE = ASTBuilder::Instance().CreateASTComplexExpressionNode(CEId, BOP);
  else if (const ASTUnaryOpNode *UOP = dynamic_cast<const ASTUnaryOpNode *>(EN))
    CE = ASTBuilder::Instance().CreateASTComplexExpressionNode(CEId, UOP);
  assert(CE && "Could not create a valid ASTComplexExpressionNode!");

  // Keep Expr without Evaluate: alpha/2 is not a real+imag literal pair.
  ASTMPComplexNode *MPC = ASTBuilder::Instance().CreateASTMPComplexNode(
      CId, ASTMPComplexNode::DefaultBits);
  assert(MPC && "Could not create a valid ASTMPComplexNode!");
  MPC->AttachExpression(CE);

  CId->SetPolymorphicName(SCN.str());
  MPC->Mangle();
  MPC->MangleLiteral();
  ToGateParamSymbolTable(CId, CId->GetSymbolTableEntry());
  return MPC;
}

bool ASTGateNode::FormalWantsComplex(unsigned Index) const {
  if (Index < FormalParamTypes.size() &&
      FormalParamTypes[Index] == ASTTypeMPComplex)
    return true;
  // Builtin disp always expects a complex scalar at parameter 0.
  if (GateCall && IsDispGateIdentifier(GetIdentifier()) && Index == 0U)
    return true;
  return false;
}

bool ASTGateNode::FormalWantsFloatScalar(unsigned Index) const {
  if (Index >= FormalParamTypes.size())
    return false;
  ASTType T = FormalParamTypes[Index];
  return T == ASTTypeFloat || T == ASTTypeDouble || T == ASTTypeMPDecimal;
}

bool ASTGateNode::FormalWantsFloatArray(unsigned Index) const {
  if (Index >= FormalParamTypes.size())
    return false;
  ASTType T = FormalParamTypes[Index];
  return T == ASTTypeFloatArray || T == ASTTypeMPDecimalArray;
}

ASTMPDecimalArrayNode *
ASTGateNode::MaterializeMPDecimalArrayFromAngles(unsigned Index,
                                                 const ASTAngleArrayNode *AAN) {
  assert(AAN && "Invalid AngleArray for float-array materialization!");

  const unsigned Bits = ASTMPDecimalNode::DefaultBits;
  std::vector<ASTMPDecimalNode *> Decimals;
  Decimals.reserve(AAN->Size());

  for (unsigned I = 0; I < AAN->Size(); ++I) {
    const ASTAngleNode *AN = AAN->GetElement(I);
    assert(AN && "AngleArray element is null!");

    std::stringstream SN;
    SN << "ast-gate-array-mpdecimal-" << Index << '-' << I;
    ASTIdentifierNode *EId = ASTBuilder::Instance().CreateASTIdentifierNode(
        SN.str(), Bits, ASTTypeMPDecimal);
    assert(EId && "Could not create an MPDecimal ASTIdentifierNode!");

    ASTMPDecimalNode *MPD = nullptr;
    if (AN->IsExpression()) {
      const ASTExpressionNode *E = AN->GetExpression();
      assert(E && "Angle expression is null!");
      MPD = ASTBuilder::Instance().CreateASTMPDecimalNodeFromExpression(
          EId, Bits, E);
    } else {
      MPD = ASTBuilder::Instance().CreateASTMPDecimalNode(EId, Bits,
                                                          AN->GetMPValue());
    }
    assert(MPD && "Could not materialize MPDecimal from AngleArray element!");

    MPD->Mangle();
    MPD->MangleLiteral();
    Decimals.push_back(MPD);
  }

  std::stringstream ArrName;
  ArrName << "ast-gate-mpdecimal-array-lit-" << Decimals.size();
  ASTIdentifierNode *ArrId =
      new ASTIdentifierNode(ArrName.str(), ASTTypeMPDecimalArray,
                            static_cast<unsigned>(Decimals.size()));
  assert(ArrId && "Could not create an MPDecimal Array ASTIdentifierNode!");
  ArrId->SetPolymorphicName("gatearraympdecimal");

  ASTMPDecimalArrayNode *MDAN =
      new ASTMPDecimalArrayNode(ArrId, Decimals, Bits);
  assert(MDAN && "Could not create a valid ASTMPDecimalArrayNode!");
  MDAN->Mangle();
  (void)Index;
  return MDAN;
}

ASTMPComplexNode *
ASTGateNode::MaterializeComplexFromReal(unsigned Index,
                                        const ASTMPDecimalNode *R) {
  assert(R && "Invalid real part for complex gate Param!");

  std::stringstream SCN;
  SCN << ASTDemangler::TypeName(ASTTypeMPComplex) << Index;
  ASTIdentifierNode *CId = ASTBuilder::Instance().CreateASTIdentifierNode(
      SCN.str(), ASTMPComplexNode::DefaultBits, ASTTypeMPComplex);
  assert(CId && "Could not create a Complex ASTIdentifierNode!");

  std::stringstream ISN;
  ISN << "complex-im-" << Index;
  ASTIdentifierNode *IId = ASTBuilder::Instance().CreateASTIdentifierNode(
      ISN.str(), ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal);
  assert(IId && "Could not create an Imaginary ASTIdentifierNode!");

  ASTMPDecimalNode *Imag = ASTBuilder::Instance().CreateASTMPDecimalNode(
      IId, ASTMPDecimalNode::DefaultBits, 0.0);
  assert(Imag && "Could not create a zero imaginary ASTMPDecimalNode!");

  ASTMPComplexNode *MPC = ASTBuilder::Instance().CreateASTMPComplexNode(
      CId, R, Imag, ASTOpTypeAdd, ASTMPComplexNode::DefaultBits);
  assert(MPC && "Could not create a valid ASTMPComplexNode!");

  CId->SetPolymorphicName(SCN.str());
  MPC->Mangle();
  MPC->MangleLiteral();
  ToGateParamSymbolTable(CId, CId->GetSymbolTableEntry());
  return MPC;
}

void ASTGateNode::ToGateParamSymbolTable(const ASTIdentifierNode *Id,
                                         const ASTSymbolTableEntry *STE) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  std::map<std::string, const ASTSymbolTableEntry *>::iterator I =
      GSTM.find(Id->GetName());
  if (I != GSTM.end())
    return;

  const ASTDeclarationContext *DCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
  assert(DCX && "Could not obtain a valid ASTDeclarationContext!");

  if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
    const_cast<ASTIdentifierNode *>(Id)->SetDeclarationContext(DCX);
    const_cast<ASTSymbolTableEntry *>(STE)->SetContext(DCX);
    const_cast<ASTIdentifierNode *>(Id)->SetLocalScope();
    const_cast<ASTSymbolTableEntry *>(STE)->SetLocalScope();
  }

  if (!GSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
    std::stringstream M;
    M << "Insertion failure to the local symbol table.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
  }

  if (Id->GetSymbolType() == ASTTypeAngle) {
    std::stringstream S;
    for (unsigned J = 0; J < 3U; ++J) {
      S.str("");
      S.clear();
      S << Id->GetName() << '[' << J << ']';

      const ASTSymbolTableEntry *ASTE =
          ASTSymbolTable::Instance().FindLocal(S.str());
      if (!ASTE)
        ASTE = ASTSymbolTable::Instance().FindAngle(S.str());

      if (ASTE) {
        if (!GSTM.insert(std::make_pair(S.str(), ASTE)).second) {
          std::stringstream M;
          M << "Insertion failure to the local symbol table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
              DiagLevel::ICE);
        }

        const ASTIdentifierNode *AId = ASTE->GetIdentifier();
        assert(AId && "Invalid ASTIdentifierNode for ASTSymbolTableEntry!");

        if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
          const_cast<ASTIdentifierNode *>(AId)->SetDeclarationContext(DCX);
          const_cast<ASTSymbolTableEntry *>(ASTE)->SetContext(DCX);
          const_cast<ASTIdentifierNode *>(AId)->SetLocalScope();
          const_cast<ASTSymbolTableEntry *>(ASTE)->SetLocalScope();
        }

        ASTSymbolTable::Instance().EraseLocalSymbol(S.str(), AId->GetBits(),
                                                    AId->GetSymbolType());
        ASTSymbolTable::Instance().EraseLocalAngle(S.str(), AId->GetBits(),
                                                   AId->GetSymbolType());
      }
    }

    ASTSymbolTable::Instance().EraseLocalSymbol(Id->GetName(), Id->GetBits(),
                                                Id->GetSymbolType());
  }
}

ASTSymbolTableEntry *
ASTGateNode::MangleGateOperandParam(ASTIdentifierNode *Id,
                                    ASTSymbolTableEntry *&STE, unsigned IX,
                                    unsigned Bits, unsigned QBits) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (!STE) {
    STE = new ASTSymbolTableEntry(Id, Id->GetSymbolType());
    assert(STE && "Could not create a valid ASTSymbolTableEntry!");
  }

  if (Bits == 0U && QBits != 0U) {
    std::stringstream M;
    M << "A Qubit Parameter cannot have a Qubit size.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
  }

  std::stringstream QNS;
  QNS << (Bits == 0U ? "qubitparam" : "qubitarg") << IX;
  ASTType QTy = Id->GetSymbolType();

  switch (QTy) {
  case ASTTypeQubit: {
    ASTQubitNode *QN = nullptr;
    if (!STE->HasValue()) {
      QN = new ASTQubitNode(Id, IX, QNS.str());
      assert(QN && "Could not create a valid ASTQubitNode!");
      STE->ResetValue();
      STE->SetValue(new ASTValue<>(QN, QTy), QTy);
      assert(STE->HasValue() && "Qubit SymbolTable Entry has no Value!");
    } else {
      QN = STE->GetValue()->GetValue<ASTQubitNode *>();
      assert(QN && "Could not create a valid ASTQubitNode!");
    }

    QN->Mangle();
  } break;
  case ASTTypeGateOperandParam: {
    ASTGateOperandParamNode *QPN = nullptr;
    if (!STE->HasValue()) {
      QPN = new ASTGateOperandParamNode(Id, IX, Bits, QNS.str());
      assert(QPN && "Could not create a valid ASTGateOperandParamNode!");
      STE->ResetValue();
      STE->SetValue(new ASTValue<>(QPN, QTy), QTy);
      assert(STE->HasValue() && "Qubit SymbolTable Entry has no Value!");
    } else {
      QPN = STE->GetValue()->GetValue<ASTGateOperandParamNode *>();
      if (!QPN) {
        std::map<std::string, const ASTSymbolTableEntry *>::iterator QI =
            GSTM.find(Id->GetName());
        if (QI == GSTM.end()) {
          QPN = new ASTGateOperandParamNode(Id, IX, Bits, QNS.str());
          assert(QPN && "Could not create a valid ASTGateOperandParamNode!");
          STE->ResetValue();
          STE->SetValue(new ASTValue<>(QPN, QTy), QTy);
          assert(STE->HasValue() && "Qubit SymbolTable Entry has no Value!");
        } else if ((*QI).second->GetValueType() == ASTTypeGateOperandParam) {
          QPN = (*QI).second->GetValue()->GetValue<ASTGateOperandParamNode *>();
          assert(QPN && "Could not create a valid ASTGateOperandParamNode!");
        } else {
          std::stringstream M;
          M << "Identifier '" << Id->GetName() << "' already exists "
            << "in the Gate's SymbolTable but with a different Type ("
            << PrintTypeEnum((*QI).second->GetValueType()) << " vs. "
            << PrintTypeEnum(ASTTypeGateOperandParam) << ").";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
              DiagLevel::ICE);
        }
      }
    }

    QPN->Mangle();
  } break;
  case ASTTypeQubitContainer: {
    ASTQubitContainerNode *QCN = nullptr;
    if (!STE->HasValue()) {
      QCN = new ASTQubitContainerNode(Id, Bits, QBits, QNS.str());
      assert(QCN && "Could not create a valid ASTQubitContainerNode!");
      STE->ResetValue();
      STE->SetValue(new ASTValue<>(QCN, QTy), QTy);
      assert(STE->HasValue() && "Qubit SymbolTable Entry has no Value!");
    } else {
      QCN = STE->GetValue()->GetValue<ASTQubitContainerNode *>();
      assert(QCN && "Could not obtain a valid ASTQubitContainerNode!");
    }

    QCN->Mangle();
  } break;
  case ASTTypeQubitContainerAlias: {
    if (!STE->HasValue()) {
      std::stringstream M;
      M << "Impossible initialization of gate qubit parameter from "
        << "type " << PrintTypeEnum(QTy) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return nullptr;
    }

    ASTQubitContainerAliasNode *QCAN =
        STE->GetValue()->GetValue<ASTQubitContainerAliasNode *>();
    assert(QCAN && "Could not obtain a valid ASTQubitContainerAliasNode!");
    QCAN->Mangle();
  } break;
  case ASTTypeQumode: {
    ASTQumodeNode *QN = nullptr;
    if (!STE->HasValue()) {
      QN = new ASTQumodeNode(Id, IX, QNS.str());
      assert(QN && "Could not create a valid ASTQumodeNode!");
      STE->ResetValue();
      STE->SetValue(new ASTValue<>(QN, QTy), QTy);
      assert(STE->HasValue() && "Qumode SymbolTable Entry has no Value!");
    } else {
      QN = STE->GetValue()->GetValue<ASTQumodeNode *>();
      assert(QN && "Could not obtain a valid ASTQumodeNode!");
    }

    QN->Mangle();
  } break;
  case ASTTypeQumodeContainer: {
    ASTQumodeContainerNode *QCN = nullptr;
    if (!STE->HasValue()) {
      QCN = new ASTQumodeContainerNode(Id, Bits == 0U ? 1U : Bits);
      assert(QCN && "Could not create a valid ASTQumodeContainerNode!");
      STE->ResetValue();
      STE->SetValue(new ASTValue<>(QCN, QTy), QTy);
      assert(STE->HasValue() && "Qumode SymbolTable Entry has no Value!");
    } else {
      QCN = STE->GetValue()->GetValue<ASTQumodeContainerNode *>();
      assert(QCN && "Could not obtain a valid ASTQumodeContainerNode!");
    }

    QCN->Mangle();
  } break;
  default: {
    std::stringstream M;
    M << "Impossible initialization of gate qubit parameter from "
      << "type " << PrintTypeEnum(QTy) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
    return nullptr;
  } break;
  }

  return STE;
}

ASTAngleNode *
ASTGateNode::CreateAngleConversion(const ASTSymbolTableEntry *XSTE) const {
  assert(XSTE && "Invalid ASTSymbolTableEntry argument!");

  ASTAngleNode *XAN = nullptr;
  ASTImplicitConversionNode *ICE = nullptr;
  const ASTLocation &LC = XSTE->GetIdentifier()->GetLocation();

  switch (XSTE->GetValueType()) {
  case ASTTypeFloat: {
    const ASTFloatNode *FN = XSTE->GetValue()->GetValue<ASTFloatNode *>();
    assert(FN && "Could not obtain a valid ASTFloatNode!");

    unsigned ConvertBits = FN->GetBits();

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), FN,
                           ASTAngleTypeGeneric, ConvertBits);
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(FN, ASTTypeAngle, ConvertBits);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  } break;
  case ASTTypeDouble: {
    const ASTDoubleNode *DN = XSTE->GetValue()->GetValue<ASTDoubleNode *>();
    assert(DN && "Could not obtain a valid ASTDoubleNode!");

    unsigned ConvertBits = DN->GetBits();

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), DN,
                           ASTAngleTypeGeneric, ConvertBits);
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(DN, ASTTypeAngle, ConvertBits);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  } break;
  case ASTTypeInt:
  case ASTTypeUInt: {
    const ASTIntNode *IN = XSTE->GetValue()->GetValue<ASTIntNode *>();
    assert(IN && "Could not obtain a valid ASTIntNode!");

    unsigned ConvertBits = IN->GetBits();

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), IN,
                           ASTAngleTypeGeneric, ConvertBits);
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(IN, ASTTypeAngle, ConvertBits);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  } break;
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger: {
    const ASTMPIntegerNode *MPI =
        XSTE->GetValue()->GetValue<ASTMPIntegerNode *>();
    assert(MPI && "Could not obtain a valid ASTMPIntegerNode!");

    unsigned ConvertBits = MPI->GetBits();

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), MPI,
                           ASTAngleTypeGeneric, ConvertBits);
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(MPI, ASTTypeAngle, ConvertBits);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  } break;
  case ASTTypeMPDecimal: {
    const ASTMPDecimalNode *MPD =
        XSTE->GetValue()->GetValue<ASTMPDecimalNode *>();
    assert(MPD && "Could not obtain a valid ASTMPDecimalNode!");

    unsigned ConvertBits = MPD->GetBits();

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), MPD,
                           ASTAngleTypeGeneric, ConvertBits);
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(MPD, ASTTypeAngle, ConvertBits);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  } break;
  case ASTTypeBitset: {
    const ASTCBitNode *CBN = XSTE->GetValue()->GetValue<ASTCBitNode *>();
    assert(CBN && "Could not obtain a valid ASTCBitNode!");

    unsigned ConvertBits = CBN->GetBits();

    if (CBN->Size() > ConvertBits) {
      std::stringstream M;
      M << "Conversion from " << PrintTypeEnum(XSTE->GetValueType())
        << " to Angle Type will result in truncation.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(XSTE->GetIdentifier()),
          M.str(), DiagLevel::Warning);
    }

    unsigned SZ = std::min(static_cast<unsigned>(CBN->Size()), ConvertBits);

    if (SZ >= 4U)
      SZ = SZ % 4;

    double D = 0.0;

    for (unsigned I = 0; I < SZ; ++I)
      if ((*CBN)[I])
        D += static_cast<double>(M_PI / 2);

    XAN = new ASTAngleNode(ASTIdentifierNode::Angle.Clone(LC), D,
                           ASTAngleTypeGeneric, ConvertBits);
    assert(XAN && "Could not create a valid ASTAngleNode!");
    ICE = new ASTImplicitConversionNode(CBN, ASTTypeAngle, SZ);
    assert(ICE && "Could not create a valid ASTImplicitConversionNode!");
    XAN->SetImplicitConversion(ICE);
  } break;
  default: {
    std::stringstream M;
    M << "Impossible implicit conversion to Angle Type from "
      << PrintTypeEnum(XSTE->GetValueType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(XSTE->GetIdentifier()), M.str(),
        DiagLevel::Error);
    return nullptr;
  } break;
  }

  if (ICE && !ICE->IsValidConversion()) {
    std::stringstream M;
    M << "Impossible implicit conversion to Angle Type from "
      << PrintTypeEnum(XSTE->GetValueType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(XSTE->GetIdentifier()), M.str(),
        DiagLevel::Error);
    return nullptr;
  }

  ASTObjectTracker::Instance().Unregister(XAN);

  XAN->SetLocation(LC);
  XAN->Mangle();
  return XAN;
}

ASTAngleNode *
ASTGateNode::CreateAngleTemporary(const ASTSymbolTableEntry *XSTE) const {
  assert(XSTE && "Invalid ASTSymbolTableEntry argument!");

  ASTAngleNode *XAN =
      new ASTAngleNode(ASTIdentifierNode::Angle.Clone(), ASTAngleTypeGeneric,
                       XSTE->GetIdentifier()->GetBits());
  assert(XAN && "Could not create a valid ASTAngleNode!");

  ASTObjectTracker::Instance().Unregister(XAN);
  return XAN;
}

ASTAngleNode *
ASTGateNode::CreateAngleSymbolTableEntry(ASTSymbolTableEntry *XSTE) const {
  assert(XSTE && "Invalid ASTSymbolTableEntry argument!");

  ASTAngleNode *XAN =
      new ASTAngleNode(XSTE->GetIdentifier(), ASTAngleTypeGeneric,
                       XSTE->GetIdentifier()->GetBits());
  assert(XAN && "Could not create a valid ASTAngleNode!");

  XSTE->ResetValue();
  XSTE->SetValue(new ASTValue<>(XAN, ASTTypeAngle), ASTTypeAngle);
  assert(XSTE->HasValue() && "ASTSymbolTable Entry has no Value!");

  return XAN;
}

void ASTGateNode::ClearLocalGateSymbols() const {
  std::map<std::string, const ASTSymbolTableEntry *>::const_iterator MI;
  for (MI = GSTM.begin(); MI != GSTM.end(); ++MI) {
    if ((*MI).second) {
      if ((*MI).second->GetValueType() == ASTTypeGateOperandParam)
        ASTSymbolTable::Instance().EraseGateOperandParam(
            (*MI).second->GetIdentifier());

      ASTSymbolTable::Instance().EraseLocalSymbol(
          (*MI).second->GetIdentifier(),
          (*MI).second->GetIdentifier()->GetBits(),
          (*MI).second->GetValueType());
    } else {
      ASTSymbolTable::Instance().EraseGateOperandParam((*MI).first);
      ASTSymbolTable::Instance().EraseLocalSymbol((*MI).first);
    }
  }
}

ASTType ASTGateNode::ResolveOperandQuantumType(const ASTIdentifierNode *QId,
                                               unsigned Index) const {
  if (Index < FormalQuantumTypes.size()) {
    const ASTType FTy = FormalQuantumTypes[Index];
    if (FTy == ASTTypeQubit || FTy == ASTTypeQumode)
      return FTy;
  }

  if (QId) {
    const ASTType PTy = QId->GetPolymorphicType();
    if (PTy == ASTTypeQubit || PTy == ASTTypeQumode)
      return PTy;

    const ASTType STy = QId->GetSymbolType();
    if (STy == ASTTypeQumode || STy == ASTTypeQumodeContainer)
      return ASTTypeQumode;
    if (STy == ASTTypeQubit || STy == ASTTypeQubitContainer ||
        STy == ASTTypeQubitContainerAlias)
      return ASTTypeQubit;
  }

  return ASTTypeQubit;
}

void ASTGateNode::ClearGateOperands() const {
  for (std::vector<ASTQubitNode *>::const_iterator QI = Operands.begin();
       QI != Operands.end(); ++QI) {
    switch ((*QI)->GetASTType()) {
    case ASTTypeQubit:
    case ASTTypeQubitContainer:
    case ASTTypeQubitContainerAlias:
    case ASTTypeQumode:
    case ASTTypeQumodeContainer:
    case ASTTypeGateOperandParam:
      ASTSymbolTable::Instance().EraseGateLocalQubit((*QI)->GetName());
      break;
    default:
      break;
    }
  }
}

void ASTGateNode::MaterializeBuiltinUGate(const ASTIdentifierNode *GId,
                                          const ASTParameterList &PL,
                                          const ASTIdentifierList &IL) {
  assert(GId && "Invalid ASTIdentifierNode argument!");

  if (PL.Size() != 3) {
    std::stringstream M;
    M << "Invalid number of angle parameters for builtin UGate.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(GId), M.str(), DiagLevel::ICE);
  }

  if (IL.Size() != 1) {
    std::stringstream M;
    M << "Invalid number of qubit parameters for builtin UGate.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(GId), M.str(), DiagLevel::ICE);
  }

  const ASTDeclarationContext *DCX =
      ASTDeclarationContextTracker::Instance().GetGlobalContext();
  assert(DCX && "Could not obtain a valid ASTDeclarationContext!");

  for (ASTParameterList::const_iterator I = PL.begin(); I != PL.end(); ++I) {
    ASTParameter *AP = dynamic_cast<ASTParameter *>(*I);
    assert(AP && "Could not dynamic_cast to an ASTParameter!");

    ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().Lookup(
        AP->GetName(), ASTAngleNode::AngleBits, ASTTypeAngle);
    assert(STE && "UGate Angle Parameter has no SymbolTable Entry!");
    assert(STE->HasValue() &&
           "UGate Angle Parameter SymbolTable Entry has no Value!");

    ASTAngleNode *AN = STE->GetValue()->GetValue<ASTAngleNode *>();
    assert(AN && "UGate Angle Parameter is invalid!");

    ASTIdentifierNode *AId =
        const_cast<ASTIdentifierNode *>(AN->GetIdentifier());
    assert(AId && "UGate Angle Parameter has an invalid ASTIdentifierNode!");

    AId->SetDeclarationContext(DCX);
    AId->SetLocalScope();
    STE->SetContext(DCX);
    STE->SetLocalScope();
    AId->SetSymbolTableEntry(STE);

    ToGateParamSymbolTable(AId, STE);
    AddParam(ASTTypeAngle, AN);
    ASTSymbolTable::Instance().EraseLocalAngle(AId);
  }

  for (ASTIdentifierList::const_iterator I = IL.begin(); I != IL.end(); ++I) {
    ASTIdentifierNode *QId = const_cast<ASTIdentifierNode *>(*I);
    assert(QId && "Invalid Qubit ASTIdentifierNode!");

    ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().Lookup(
        QId->GetName(), QId->GetBits(), ASTTypeGateOperandParam);
    assert(STE && "UGate Qubit Parameter has no SymbolTable Entry!");

    QId->SetDeclarationContext(DCX);
    QId->SetLocalScope();
    STE->SetContext(DCX);
    STE->SetLocalScope();
    QId->SetSymbolTableEntry(STE);

    MaterializeGateOperandParam(QId);
    OperandParams.push_back(STE);
    STE = MangleGateOperandParam(QId, STE, 0, QId->GetBits(), QId->GetBits());
    ToGateParamSymbolTable(QId, STE);
    ASTSymbolTable::Instance().EraseLocalQubitParam(QId);
  }
}

ASTGateNode::ASTGateNode(const ASTIdentifierNode *Id,
                         const ASTArgumentNodeList &AL,
                         const ASTAnyTypeList &QL, bool IsGateCall,
                         const ASTGateQOpList &OL,
                         const std::vector<ASTType> *CallFormalParamTypes)
    : ASTExpressionNode(Id, ASTTypeGate), Params(), Operands(), OperandParams(),
      OpList(OL), Ctrl(nullptr), GDId(IsGateCall ? nullptr : Id), GSTM(),
      ControlType(ASTTypeUndefined), Opaque(false), GateCall(IsGateCall),
      FullyTyped(false), FormalParamTypes(), FormalParamArraySizes(),
      FormalQuantumTypes(), TemplateParams(),
      FormalParamArraySizeTemplateIndices() {
  if (CallFormalParamTypes)
    FormalParamTypes = *CallFormalParamTypes;

  unsigned C = 0;
  std::set<std::string> PNS;
  std::vector<const ASTIdentifierNode *> NQV;
  ASTType Ty = ASTTypeUndefined;

  for (ASTArgumentNodeList::const_iterator I = AL.begin(); I != AL.end(); ++I) {
    Ty = (*I)->GetValueType();
    ASTSymbolTableEntry *ASTE = nullptr;

    switch (Ty) {
    case ASTTypeExpression: {
      try {
        const ASTExpression *EN =
            std::any_cast<const ASTExpression *>((*I)->GetValue());
        (void)EN; // Quiet.
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeIdentifier: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTIdentifierNode *ID = EN->DynCast<const ASTIdentifierNode>();
        assert(ID && "Failed to dynamic_cast to an IdentifierNode!");

        const ASTSymbolTableEntry *XSTE =
            ASTSymbolTable::Instance().FindAngle(ID->GetName());
        if (!XSTE)
          XSTE = ASTSymbolTable::Instance().FindLocal(ID->GetName());
        if (!XSTE)
          XSTE = ASTSymbolTable::Instance().FindGlobal(ID->GetName());

        if (!XSTE) {
          std::stringstream M;
          M << "Non-existent angle " << ID->GetName() << " passed as "
            << "angle argument to Gate Call.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(ID), M.str(),
              DiagLevel::Error);
        }

        // Complex variables are stored as-is; do not coerce to Angle.
        if (XSTE && XSTE->HasValue() &&
            XSTE->GetValueType() == ASTTypeMPComplex) {
          ASTMPComplexNode *XMPC =
              XSTE->GetValue()->GetValue<ASTMPComplexNode *>();
          assert(XMPC && "Could not obtain a valid ASTMPComplexNode!");

          std::stringstream SCN;
          SCN << ASTDemangler::TypeName(ASTTypeMPComplex) << C;
          ASTIdentifierNode *CId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SCN.str(), ASTMPComplexNode::DefaultBits, ASTTypeMPComplex);
          assert(CId && "Could not create a Complex ASTIdentifierNode!");

          ASTMPComplexNode *MPC = ASTBuilder::Instance().CreateASTMPComplexNode(
              CId, XMPC, ASTMPComplexNode::DefaultBits);
          assert(MPC && "Could not create a valid ASTMPComplexNode!");

          CId->SetPolymorphicName(ID->GetName());
          MPC->Mangle();
          MPC->MangleLiteral();
          ToGateParamSymbolTable(CId, CId->GetSymbolTableEntry());
          AddParam(ASTTypeMPComplex, MPC);
          PNS.insert(MPC->GetName());
          break;
        }

        // Typed complex formal: promote real scalar identifiers to real+0i.
        if (IsGateCall && FormalWantsComplex(C) && XSTE && XSTE->HasValue()) {
          const ASTMPDecimalNode *R = nullptr;
          ASTMPDecimalNode *OwnedR = nullptr;
          if (XSTE->GetValueType() == ASTTypeMPDecimal) {
            R = XSTE->GetValue()->GetValue<ASTMPDecimalNode *>();
          } else if (XSTE->GetValueType() == ASTTypeFloat) {
            const ASTFloatNode *FLT =
                XSTE->GetValue()->GetValue<ASTFloatNode *>();
            if (FLT) {
              std::stringstream RSN;
              RSN << "complex-re-" << C;
              ASTIdentifierNode *RId =
                  ASTBuilder::Instance().CreateASTIdentifierNode(
                      RSN.str(), ASTMPDecimalNode::DefaultBits,
                      ASTTypeMPDecimal);
              OwnedR = ASTBuilder::Instance().CreateASTMPDecimalNode(
                  RId, ASTMPDecimalNode::DefaultBits,
                  static_cast<double>(FLT->GetValue()));
              R = OwnedR;
            }
          } else if (XSTE->GetValueType() == ASTTypeDouble) {
            const ASTDoubleNode *DBL =
                XSTE->GetValue()->GetValue<ASTDoubleNode *>();
            if (DBL) {
              std::stringstream RSN;
              RSN << "complex-re-" << C;
              ASTIdentifierNode *RId =
                  ASTBuilder::Instance().CreateASTIdentifierNode(
                      RSN.str(), ASTMPDecimalNode::DefaultBits,
                      ASTTypeMPDecimal);
              OwnedR = ASTBuilder::Instance().CreateASTMPDecimalNode(
                  RId, ASTMPDecimalNode::DefaultBits, DBL->GetValue());
              R = OwnedR;
            }
          }
          if (R) {
            ASTMPComplexNode *MPC = MaterializeComplexFromReal(C, R);
            assert(MPC && "Could not materialize complex Param from id!");
            AddParam(ASTTypeMPComplex, MPC);
            PNS.insert(MPC->GetName());
            break;
          }
          (void)OwnedR;
        }

        // Named array / float-scalar args: keep type-faithful Params.
        if (XSTE && XSTE->HasValue()) {
          const ASTType VT = XSTE->GetValueType();
          if (VT == ASTTypeMPDecimalArray || VT == ASTTypeFloatArray ||
              VT == ASTTypeAngleArray || VT == ASTTypeMPComplexArray) {
            ASTExpressionNode *Arr = nullptr;
            if (VT == ASTTypeMPDecimalArray)
              Arr = XSTE->GetValue()->GetValue<ASTMPDecimalArrayNode *>();
            else if (VT == ASTTypeFloatArray)
              Arr = XSTE->GetValue()->GetValue<ASTFloatArrayNode *>();
            else if (VT == ASTTypeAngleArray)
              Arr = XSTE->GetValue()->GetValue<ASTAngleArrayNode *>();
            else
              Arr = XSTE->GetValue()->GetValue<ASTMPComplexArrayNode *>();
            if (!Arr) {
              if (ASTArrayNode *ARN =
                      XSTE->GetValue()->GetValue<ASTArrayNode *>())
                Arr = ARN;
            }
            assert(Arr && "Named array gate arg has no array node!");
            if (IsGateCall && FormalWantsFloatArray(C) &&
                VT == ASTTypeAngleArray) {
              ASTMPDecimalArrayNode *MDAN = MaterializeMPDecimalArrayFromAngles(
                  C, dynamic_cast<ASTAngleArrayNode *>(Arr));
              assert(MDAN && "Could not convert named AngleArray arg!");
              AddParam(ASTTypeMPDecimalArray, MDAN);
              PNS.insert(MDAN->GetName());
            } else {
              AddParam(VT, Arr);
              PNS.insert(Arr->GetName());
            }
            PNS.insert(ID->GetName());
            ID->SetNoQubit(true);
            break;
          }

          if (IsGateCall && FormalWantsFloatScalar(C) &&
              (VT == ASTTypeFloat || VT == ASTTypeDouble ||
               VT == ASTTypeMPDecimal)) {
            if (VT == ASTTypeMPDecimal) {
              ASTMPDecimalNode *R =
                  XSTE->GetValue()->GetValue<ASTMPDecimalNode *>();
              assert(R && "Named MPDecimal gate arg is null!");
              AddParam(ASTTypeMPDecimal, R);
              PNS.insert(R->GetName());
            } else if (VT == ASTTypeFloat) {
              ASTFloatNode *FLT = XSTE->GetValue()->GetValue<ASTFloatNode *>();
              assert(FLT && "Named Float gate arg is null!");
              if (FormalParamTypes[C] == ASTTypeFloat) {
                AddParam(ASTTypeFloat, FLT);
                PNS.insert(FLT->GetName());
              } else {
                std::stringstream RSN;
                RSN << "ast-gate-mpdecimal-" << C;
                ASTIdentifierNode *RId =
                    ASTBuilder::Instance().CreateASTIdentifierNode(
                        RSN.str(), ASTMPDecimalNode::DefaultBits,
                        ASTTypeMPDecimal);
                ASTMPDecimalNode *R =
                    ASTBuilder::Instance().CreateASTMPDecimalNode(
                        RId, ASTMPDecimalNode::DefaultBits,
                        static_cast<double>(FLT->GetValue()));
                assert(R && "Could not promote Float id to MPDecimal!");
                R->Mangle();
                R->MangleLiteral();
                ToGateParamSymbolTable(RId, RId->GetSymbolTableEntry());
                AddParam(ASTTypeMPDecimal, R);
                PNS.insert(R->GetName());
              }
            } else {
              ASTDoubleNode *DBL =
                  XSTE->GetValue()->GetValue<ASTDoubleNode *>();
              assert(DBL && "Named Double gate arg is null!");
              if (FormalParamTypes[C] == ASTTypeDouble) {
                AddParam(ASTTypeDouble, DBL);
                PNS.insert(DBL->GetName());
              } else {
                std::stringstream RSN;
                RSN << "ast-gate-mpdecimal-" << C;
                ASTIdentifierNode *RId =
                    ASTBuilder::Instance().CreateASTIdentifierNode(
                        RSN.str(), ASTMPDecimalNode::DefaultBits,
                        ASTTypeMPDecimal);
                ASTMPDecimalNode *R =
                    ASTBuilder::Instance().CreateASTMPDecimalNode(
                        RId, ASTMPDecimalNode::DefaultBits, DBL->GetValue());
                assert(R && "Could not promote Double id to MPDecimal!");
                R->Mangle();
                R->MangleLiteral();
                ToGateParamSymbolTable(RId, RId->GetSymbolTableEntry());
                AddParam(ASTTypeMPDecimal, R);
                PNS.insert(R->GetName());
              }
            }
            PNS.insert(ID->GetName());
            ID->SetNoQubit(true);
            break;
          }
        }

        ASTAngleNode *XAN = nullptr;
        bool IMPC = false;

        if (XSTE->HasValue()) {
          if (ASTUtils::Instance().IsAngleType(XSTE->GetValueType()) ||
              ASTUtils::Instance().IsReservedAngleType(XSTE->GetValueType())) {
            XAN = XSTE->GetValue()->GetValue<ASTAngleNode *>();
          } else {
            XAN = CreateAngleConversion(XSTE);
            IMPC = true;
          }
        } else {
          XAN = CreateAngleTemporary(XSTE);
          IMPC = true;
        }

        assert(XAN && "Could not obtain a valid ASTAngleNode!");

        ASTAngleNode *AN = nullptr;
        ASTIdentifierNode *AId = nullptr;

        std::stringstream SAN;
        SAN << ASTDemangler::TypeName(ASTTypeAngle) << C;
        AId = ASTBuilder::Instance().CreateASTIdentifierNode(
            SAN.str(), ASTAngleNode::AngleBits, ASTTypeAngle);
        assert(AId && "Could not create an Angle ASTIdentifierNode!");

        ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
        AN = ASTBuilder::Instance().CreateASTAngleNode(AId, ATy,
                                                       ASTAngleNode::AngleBits);
        assert(AN && "Could not create a valid ASTAngleNode!");

        XAN->Clone(AN);
        AN->SetGateParamName(ID->GetName());
        AN->SetExpression(ID);
        AId->SetPolymorphicName(AId->GetName());
        ASTE = ASTSymbolTable::Instance().Lookup(AId, ASTAngleNode::AngleBits,
                                                 ASTTypeAngle);
        assert(ASTE &&
               "ASTAngleNode ASTIdentifierNode has no SymbolTable Entry!");

        AId->SetLocalScope();
        ASTE->SetLocalScope();
        AId->SetSymbolTableEntry(ASTE);
        AN->Mangle();
        AN->MangleLiteral();
        ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                       AId->GetSymbolType());
        ToGateParamSymbolTable(AId, ASTE);
        ASTSymbolTable::Instance().EraseLocalAngle(AId);
        ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                              AId->GetSymbolType());

        ASTAngleNodeBuilder::Instance().Insert(AN);
        ASTAngleNodeBuilder::Instance().Append(AN);

        AddParam(ASTTypeAngle, AN);

        if (XAN && IMPC)
          delete XAN;

        PNS.insert(AN->GetName());
        PNS.insert(ID->GetName());
        ID->SetNoQubit(true);
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeBinaryOp: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTBinaryOpNode *BOP = EN->DynCast<const ASTBinaryOpNode>();
        assert(BOP && "Failed to dynamic_cast to a BinaryOpNode!");

        if (IsGateCall &&
            (PreferComplexGateParam(Id, BOP) || FormalWantsComplex(C))) {
          ASTMPComplexNode *MPC = MaterializeComplexParamExpr(C, BOP);
          assert(MPC && "Could not materialize complex gate Param!");
          AddParam(ASTTypeMPComplex, MPC);
          PNS.insert(MPC->GetName());
          break;
        }

        if (IsGateCall && FormalWantsFloatScalar(C)) {
          std::stringstream RSN;
          RSN << "ast-gate-mpdecimal-" << C;
          ASTIdentifierNode *RId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  RSN.str(), ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal);
          assert(RId && "Could not create an MPDecimal ASTIdentifierNode!");
          ASTMPDecimalNode *R =
              ASTBuilder::Instance().CreateASTMPDecimalNodeFromExpression(
                  RId, ASTMPDecimalNode::DefaultBits, BOP);
          assert(R && "Could not materialize MPDecimal Param from BinaryOp!");
          ToGateParamSymbolTable(RId, RId->GetSymbolTableEntry());
          AddParam(ASTTypeMPDecimal, R);
          PNS.insert(R->GetName());
          break;
        }

        const ASTIdentifierNode *BId = BOP->GetIdentifier();
        assert(BId && "Invalid ASTIdentifierNode for ASTBinaryOpNode!");

        ASTAngleNode *AN = ASTAngleNodeMap::Instance().Find(BId->GetName());

        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(BId->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), BId->GetBits(), ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, BOP, ATy,
                                                         AId->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(AId->GetName());
          AId->SetPolymorphicName(AId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          AId->GetSymbolTableEntry()->ResetValue();
          AId->GetSymbolTableEntry()->SetValue(new ASTValue<>(AN, ASTTypeAngle),
                                               ASTTypeAngle);
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        AddParam(ASTTypeAngle, AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeUnaryOp: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTUnaryOpNode *UOP = EN->DynCast<const ASTUnaryOpNode>();
        assert(UOP && "Failed to dynamic_cast to an UnaryOpNode!");

        if (IsGateCall &&
            (PreferComplexGateParam(Id, UOP) || FormalWantsComplex(C))) {
          ASTMPComplexNode *MPC = MaterializeComplexParamExpr(C, UOP);
          assert(MPC && "Could not materialize complex gate Param!");
          AddParam(ASTTypeMPComplex, MPC);
          PNS.insert(MPC->GetName());
          break;
        }

        if (IsGateCall && FormalWantsFloatScalar(C)) {
          std::stringstream RSN;
          RSN << "ast-gate-mpdecimal-" << C;
          ASTIdentifierNode *RId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  RSN.str(), ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal);
          assert(RId && "Could not create an MPDecimal ASTIdentifierNode!");
          ASTMPDecimalNode *R =
              ASTBuilder::Instance().CreateASTMPDecimalNodeFromExpression(
                  RId, ASTMPDecimalNode::DefaultBits, UOP);
          assert(R && "Could not materialize MPDecimal Param from UnaryOp!");
          ToGateParamSymbolTable(RId, RId->GetSymbolTableEntry());
          AddParam(ASTTypeMPDecimal, R);
          PNS.insert(R->GetName());
          break;
        }

        const ASTIdentifierNode *UId = UOP->GetIdentifier();
        assert(UId && "Invalid ASTIdentifierNode for ASTUnaryOpNode!");

        ASTAngleNode *AN = ASTAngleNodeMap::Instance().Find(UId->GetName());

        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(UId->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), UId->GetBits(), ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, UOP, ATy,
                                                         AId->GetBits());
          assert(AN && "Could not create an ASTAngleNode!!");

          AN->SetGateParamName(UOP->GetName());
          AId->SetPolymorphicName(UId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        AddParam(ASTTypeAngle, AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeInt: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTIntNode *INT = dynamic_cast<const ASTIntNode *>(EN);
        assert(INT && "Failed to dynamic_cast to a IntNode!");

        const ASTIdentifierNode *ID = INT->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTIntNode!");

        if (IsGateCall && FormalWantsComplex(C)) {
          std::stringstream RSN;
          RSN << "complex-re-" << C;
          ASTIdentifierNode *RId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  RSN.str(), ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal);
          assert(RId && "Could not create a Real ASTIdentifierNode!");
          ASTMPDecimalNode *R = ASTBuilder::Instance().CreateASTMPDecimalNode(
              RId, ASTMPDecimalNode::DefaultBits,
              static_cast<double>(INT->IsSigned() ? INT->GetSignedValue()
                                                  : INT->GetUnsignedValue()));
          assert(R && "Could not create real ASTMPDecimalNode!");
          ASTMPComplexNode *MPC = MaterializeComplexFromReal(C, R);
          assert(MPC && "Could not materialize complex Param from int!");
          AddParam(ASTTypeMPComplex, MPC);
          PNS.insert(MPC->GetName());
          break;
        }

        if (INT->GetSignedValue() > static_cast<double>(M_PI * 2)) {
          std::stringstream M;
          M << "Angle value exceeds 2pi.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(ID), M.str(),
              DiagLevel::Warning);
        }

        ASTAngleNode *AN = ASTAngleNodeMap::Instance().Find(ID->GetName());
        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), ASTAngleNode::AngleBits, ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(
              AId, INT, ATy, ASTAngleNode::AngleBits);
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(AId->GetName());
          AId->SetPolymorphicName(AId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        AddParam(ASTTypeAngle, AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeFloat: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTFloatNode *FLT = dynamic_cast<const ASTFloatNode *>(EN);
        assert(FLT && "Failed to dynamic_cast to a FloatNode!");

        const ASTIdentifierNode *ID = FLT->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTFloatNode!");

        if (IsGateCall && FormalWantsComplex(C)) {
          std::stringstream RSN;
          RSN << "complex-re-" << C;
          ASTIdentifierNode *RId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  RSN.str(), ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal);
          assert(RId && "Could not create a Real ASTIdentifierNode!");
          ASTMPDecimalNode *R = ASTBuilder::Instance().CreateASTMPDecimalNode(
              RId, ASTMPDecimalNode::DefaultBits,
              static_cast<double>(FLT->GetValue()));
          assert(R && "Could not create real ASTMPDecimalNode!");
          ASTMPComplexNode *MPC = MaterializeComplexFromReal(C, R);
          assert(MPC && "Could not materialize complex Param from float!");
          AddParam(ASTTypeMPComplex, MPC);
          PNS.insert(MPC->GetName());
          break;
        }

        if (IsGateCall && FormalWantsFloatScalar(C)) {
          ASTType FTy = FormalParamTypes[C];
          if (FTy == ASTTypeFloat) {
            AddParam(ASTTypeFloat, const_cast<ASTFloatNode *>(FLT));
            PNS.insert(FLT->GetName());
            break;
          }
          std::stringstream RSN;
          RSN << "ast-gate-mpdecimal-" << C;
          ASTIdentifierNode *RId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  RSN.str(), ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal);
          assert(RId && "Could not create an MPDecimal ASTIdentifierNode!");
          ASTMPDecimalNode *R = ASTBuilder::Instance().CreateASTMPDecimalNode(
              RId, ASTMPDecimalNode::DefaultBits,
              static_cast<double>(FLT->GetValue()));
          assert(R && "Could not create MPDecimal Param from float!");
          R->Mangle();
          R->MangleLiteral();
          ToGateParamSymbolTable(RId, RId->GetSymbolTableEntry());
          AddParam(ASTTypeMPDecimal, R);
          PNS.insert(R->GetName());
          break;
        }

        if (FLT->GetValue() > static_cast<float>(M_PI * 2)) {
          std::stringstream M;
          M << "Angle value exceeds 2pi.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(ID), M.str(),
              DiagLevel::Warning);
        }

        ASTAngleNode *AN = ASTAngleNodeMap::Instance().Find(ID->GetName());
        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), ASTAngleNode::AngleBits, ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, FLT, ATy,
                                                         AId->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(AId->GetName());
          AId->SetPolymorphicName(AId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        AddParam(ASTTypeAngle, AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeDouble: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTDoubleNode *DBL = dynamic_cast<const ASTDoubleNode *>(EN);
        assert(DBL && "Failed to dynamic_cast to a DoubleNode!");

        const ASTIdentifierNode *ID = DBL->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTDoubleNode!");

        if (IsGateCall && FormalWantsComplex(C)) {
          std::stringstream RSN;
          RSN << "complex-re-" << C;
          ASTIdentifierNode *RId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  RSN.str(), ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal);
          assert(RId && "Could not create a Real ASTIdentifierNode!");
          ASTMPDecimalNode *R = ASTBuilder::Instance().CreateASTMPDecimalNode(
              RId, ASTMPDecimalNode::DefaultBits, DBL->GetValue());
          assert(R && "Could not create real ASTMPDecimalNode!");
          ASTMPComplexNode *MPC = MaterializeComplexFromReal(C, R);
          assert(MPC && "Could not materialize complex Param from double!");
          AddParam(ASTTypeMPComplex, MPC);
          PNS.insert(MPC->GetName());
          break;
        }

        if (IsGateCall && FormalWantsFloatScalar(C)) {
          ASTType FTy = FormalParamTypes[C];
          if (FTy == ASTTypeDouble) {
            AddParam(ASTTypeDouble, const_cast<ASTDoubleNode *>(DBL));
            PNS.insert(DBL->GetName());
            break;
          }
          std::stringstream RSN;
          RSN << "ast-gate-mpdecimal-" << C;
          ASTIdentifierNode *RId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  RSN.str(), ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal);
          assert(RId && "Could not create an MPDecimal ASTIdentifierNode!");
          ASTMPDecimalNode *R = ASTBuilder::Instance().CreateASTMPDecimalNode(
              RId, ASTMPDecimalNode::DefaultBits, DBL->GetValue());
          assert(R && "Could not create MPDecimal Param from double!");
          R->Mangle();
          R->MangleLiteral();
          ToGateParamSymbolTable(RId, RId->GetSymbolTableEntry());
          AddParam(ASTTypeMPDecimal, R);
          PNS.insert(R->GetName());
          break;
        }

        if (DBL->GetValue() > static_cast<double>(M_PI * 2)) {
          std::stringstream M;
          M << "Angle value exceeds 2pi.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(ID), M.str(),
              DiagLevel::Warning);
        }

        ASTAngleNode *AN =
            ASTAngleNodeMap::Instance().Find(ID->GetName(), ID->GetBits());
        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), ASTAngleNode::AngleBits, ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, DBL, ATy,
                                                         AId->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(AId->GetName());
          AId->SetPolymorphicName(AId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        AddParam(ASTTypeAngle, AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeMPInteger: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTMPIntegerNode *MPI =
            dynamic_cast<const ASTMPIntegerNode *>(EN);
        assert(MPI && "Failed to dynamic_cast to a MPIntegerNode!");

        const ASTIdentifierNode *ID = MPI->GetIdentifier()->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTMPIntegerNode!");

        mpfr_t MPV;
        mpfr_init2(MPV, MPI->GetBits());
        (void)mpfr_set_z(MPV, MPI->GetMPValue(), MPFR_RNDN);
        mpfr_t MP2PI;
        mpfr_init2(MP2PI, MPI->GetBits());
        (void)mpfr_const_pi(MP2PI, MPFR_RNDN);
        (void)mpfr_mul_d(MP2PI, MP2PI, 2.0, MPFR_RNDN);
        if (mpfr_greater_p(MPV, MP2PI) != 0) {
          std::stringstream M;
          M << "Angle value exceeds 2pi.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(ID), M.str(),
              DiagLevel::Warning);
        }

        ASTAngleNode *AN =
            ASTAngleNodeMap::Instance().Find(ID->GetName(), ID->GetBits());
        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), MPI->GetBits(), ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, MPI, ATy,
                                                         MPI->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(AId->GetName());
          AId->SetPolymorphicName(AId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        AddParam(ASTTypeAngle, AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeMPDecimal: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTMPDecimalNode *MPD =
            dynamic_cast<const ASTMPDecimalNode *>(EN);
        assert(MPD && "Failed to dynamic_cast to a MPDecimalNode!");

        const ASTIdentifierNode *ID = MPD->GetIdentifier()->GetIdentifier();
        assert(ID && "Invalid ASTIdentifierNode for ASTMPDecimalNode!");

        if (IsGateCall && FormalWantsComplex(C)) {
          ASTMPComplexNode *MPC = MaterializeComplexFromReal(C, MPD);
          assert(MPC && "Could not materialize complex Param from MPDecimal!");
          AddParam(ASTTypeMPComplex, MPC);
          PNS.insert(MPC->GetName());
          break;
        }

        if (IsGateCall && FormalWantsFloatScalar(C)) {
          AddParam(ASTTypeMPDecimal, const_cast<ASTMPDecimalNode *>(MPD));
          PNS.insert(MPD->GetName());
          break;
        }

        mpfr_t MP2PI;
        mpfr_init2(MP2PI, MPD->GetBits());
        (void)mpfr_const_pi(MP2PI, MPFR_RNDN);
        (void)mpfr_mul_d(MP2PI, MP2PI, 2.0, MPFR_RNDN);
        if (mpfr_greater_p(MPD->GetMPValue(), MP2PI) != 0) {
          std::stringstream M;
          M << "Angle value exceeds 2pi.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(ID), M.str(),
              DiagLevel::Warning);
        }

        ASTAngleNode *AN =
            ASTAngleNodeMap::Instance().Find(ID->GetName(), ID->GetBits());
        if (!AN) {
          std::stringstream SAN;
          SAN << ASTDemangler::TypeName(ID->GetSymbolType()) << C;
          ASTIdentifierNode *AId =
              ASTBuilder::Instance().CreateASTIdentifierNode(
                  SAN.str(), MPD->GetBits(), ASTTypeAngle);
          assert(AId && "Could not create an Angle ASTIdentifierNode!");

          ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
          AN = ASTBuilder::Instance().CreateASTAngleNode(AId, MPD, ATy,
                                                         MPD->GetBits());
          assert(AN && "Could not create an ASTAngleNode!");

          AN->SetGateParamName(AId->GetName());
          AId->SetPolymorphicName(AId->GetName());
          AN->Mangle();
          AN->MangleLiteral();
          ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                         AId->GetSymbolType());
          ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
          ASTSymbolTable::Instance().EraseLocalAngle(AId);
          ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                                AId->GetSymbolType());
        }

        AddParam(ASTTypeAngle, AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeFunctionCallArgument: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to dynamic_cast to an ExpressionNode!");

        const ASTFunctionCallArgumentNode *FCA =
            dynamic_cast<const ASTFunctionCallArgumentNode *>(EN);
        assert(FCA &&
               "Failed to dynamic_cast to an ASTFunctionCallArgumentNode!");

        const ASTResultNode *RN = FCA->GetResult();
        assert(RN && "Could not obtain a valid function call ASTResultNode!");

        unsigned RBits = RN->GetResultBits() < ASTAngleNode::AngleBits
                             ? ASTAngleNode::AngleBits
                             : RN->GetResultBits();
        std::stringstream SAN;
        SAN << ASTDemangler::TypeName(RN->GetResultType()) << C;
        ASTIdentifierNode *AId = ASTBuilder::Instance().CreateASTIdentifierNode(
            SAN.str(), RBits, ASTTypeAngle);
        assert(AId && "Could not create an Angle ASTIdentifierNode!");

        ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
        ASTAngleNode *AN =
            ASTBuilder::Instance().CreateASTAngleNodeFromExpression(AId, FCA,
                                                                    ATy, RBits);
        assert(AN && "Could not create an ASTAngleNode!");

        AN->SetGateParamName(AId->GetName());
        AId->SetPolymorphicName(AId->GetName());
        AN->Mangle();
        AN->MangleLiteral();
        ToGateParamSymbolTable(AId, AId->GetSymbolTableEntry());
        ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                       AId->GetSymbolType());
        ASTSymbolTable::Instance().EraseLocalAngle(AId);
        AddParam(ASTTypeAngle, AN);
        PNS.insert(AN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeAngleArray: {
      try {
        ASTExpressionNode *EN = nullptr;
        try {
          EN = const_cast<ASTExpressionNode *>(
              std::any_cast<const ASTExpressionNode *>((*I)->GetValue()));
        } catch (const std::bad_any_cast &) {
          EN = std::any_cast<ASTExpressionNode *>((*I)->GetValue());
        }
        assert(EN && "Failed to any_cast to an ExpressionNode!");

        ASTAngleArrayNode *AAN = dynamic_cast<ASTAngleArrayNode *>(EN);
        assert(AAN && "Failed to dynamic_cast to an ASTAngleArrayNode!");

        if (IsGateCall && FormalWantsFloatArray(C)) {
          ASTMPDecimalArrayNode *MDAN =
              MaterializeMPDecimalArrayFromAngles(C, AAN);
          assert(MDAN &&
                 "Could not materialize MPDecimalArray from AngleArray!");
          // float[64] formals use MPDecimalArray; Params match
          // FormalParamTypes.
          AddParam(ASTTypeMPDecimalArray, MDAN);
          PNS.insert(MDAN->GetName());
        } else {
          AddParam(ASTTypeAngleArray, AAN);
          PNS.insert(AAN->GetName());
        }
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeFloatArray: {
      try {
        ASTExpressionNode *EN = nullptr;
        try {
          EN = const_cast<ASTExpressionNode *>(
              std::any_cast<const ASTExpressionNode *>((*I)->GetValue()));
        } catch (const std::bad_any_cast &) {
          EN = std::any_cast<ASTExpressionNode *>((*I)->GetValue());
        }
        assert(EN && "Failed to any_cast to an ExpressionNode!");

        ASTFloatArrayNode *FAN = dynamic_cast<ASTFloatArrayNode *>(EN);
        assert(FAN && "Failed to dynamic_cast to an ASTFloatArrayNode!");

        AddParam(ASTTypeFloatArray, FAN);
        PNS.insert(FAN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeMPDecimalArray: {
      try {
        ASTExpressionNode *EN = nullptr;
        try {
          EN = const_cast<ASTExpressionNode *>(
              std::any_cast<const ASTExpressionNode *>((*I)->GetValue()));
        } catch (const std::bad_any_cast &) {
          EN = std::any_cast<ASTExpressionNode *>((*I)->GetValue());
        }
        assert(EN && "Failed to any_cast to an ExpressionNode!");

        ASTMPDecimalArrayNode *MDAN = dynamic_cast<ASTMPDecimalArrayNode *>(EN);
        assert(MDAN && "Failed to dynamic_cast to an ASTMPDecimalArrayNode!");

        AddParam(ASTTypeMPDecimalArray, MDAN);
        PNS.insert(MDAN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeMPComplexArray: {
      try {
        ASTExpressionNode *EN = nullptr;
        try {
          EN = const_cast<ASTExpressionNode *>(
              std::any_cast<const ASTExpressionNode *>((*I)->GetValue()));
        } catch (const std::bad_any_cast &) {
          EN = std::any_cast<ASTExpressionNode *>((*I)->GetValue());
        }
        assert(EN && "Failed to any_cast to an ExpressionNode!");

        ASTMPComplexArrayNode *CAN = dynamic_cast<ASTMPComplexArrayNode *>(EN);
        assert(CAN && "Failed to dynamic_cast to an ASTMPComplexArrayNode!");

        AddParam(ASTTypeMPComplexArray, CAN);
        PNS.insert(CAN->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeComplexExpression: {
      try {
        const ASTExpressionNode *EN =
            std::any_cast<const ASTExpressionNode *>((*I)->GetValue());
        assert(EN && "Failed to any_cast to an ExpressionNode!");

        const ASTComplexExpressionNode *CEN =
            dynamic_cast<const ASTComplexExpressionNode *>(EN);
        assert(CEN && "Failed to dynamic_cast to an ASTComplexExpressionNode!");

        std::stringstream SCN;
        SCN << ASTDemangler::TypeName(ASTTypeMPComplex) << C;
        ASTIdentifierNode *CId = ASTBuilder::Instance().CreateASTIdentifierNode(
            SCN.str(), ASTMPComplexNode::DefaultBits, ASTTypeMPComplex);
        assert(CId && "Could not create a Complex ASTIdentifierNode!");

        ASTMPComplexNode *MPC = ASTBuilder::Instance().CreateASTMPComplexNode(
            CId, CEN, ASTMPComplexNode::DefaultBits);
        assert(MPC && "Could not create a valid ASTMPComplexNode!");

        CId->SetPolymorphicName(SCN.str());
        MPC->Mangle();
        MPC->MangleLiteral();
        ToGateParamSymbolTable(CId, CId->GetSymbolTableEntry());
        AddParam(ASTTypeMPComplex, MPC);
        PNS.insert(MPC->GetName());
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    default:
      break;
    }

    ++C;
  }

  const bool IsDispCall =
      IsGateCall &&
      (Id->GetName() == u8"disp" || Id->GetSymbolType() == ASTTypeDispGate);
  if (IsDispCall) {
    for (std::size_t PI = 0; PI < Params.size(); ++PI) {
      if (Params[PI].Ty == ASTTypeAngleArray ||
          Params[PI].Ty == ASTTypeMPComplexArray) {
        std::stringstream M;
        M << "The disp gate expects a complex parameter, not an array.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
            DiagLevel::Error);
        return;
      }
    }

    // Promote leftover scalar/angle params to complex.
    // Expression-backed angles (BinaryOp/UnaryOp) keep their Expr tree;
    // only constant/numeric angles become real+0i.
    std::vector<ASTGateParam> Promoted;
    Promoted.reserve(Params.size());
    unsigned PromoIX = 0;
    for (std::size_t PI = 0; PI < Params.size(); ++PI) {
      if (Params[PI].Ty == ASTTypeMPComplex) {
        Promoted.push_back(Params[PI]);
        continue;
      }

      ASTAngleNode *AN = dynamic_cast<ASTAngleNode *>(Params[PI].Expr);
      assert(AN && "Invalid ASTAngleNode in disp gate Params!");

      const ASTExpressionNode *AEx = AN->GetExpression();
      if (AEx && (AEx->GetASTType() == ASTTypeBinaryOp ||
                  AEx->GetASTType() == ASTTypeUnaryOp)) {
        ASTMPComplexNode *MPC = MaterializeComplexParamExpr(PromoIX, AEx);
        assert(MPC && "Could not materialize complex Param from angle Expr!");
        Promoted.emplace_back(ASTTypeMPComplex, MPC);
        ++PromoIX;
        continue;
      }

      std::stringstream SCN;
      SCN << ASTDemangler::TypeName(ASTTypeMPComplex) << PromoIX;
      ASTIdentifierNode *CId = ASTBuilder::Instance().CreateASTIdentifierNode(
          SCN.str(), ASTMPComplexNode::DefaultBits, ASTTypeMPComplex);
      assert(CId && "Could not create a Complex ASTIdentifierNode!");

      std::stringstream ISN;
      ISN << "disp-im-" << PromoIX;
      ASTIdentifierNode *IId = ASTBuilder::Instance().CreateASTIdentifierNode(
          ISN.str(), ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal);
      assert(IId && "Could not create an Imaginary ASTIdentifierNode!");

      ASTMPDecimalNode *R = AN->AsMPDecimal();
      assert(R && "Could not obtain real part from ASTAngleNode!");
      ASTMPDecimalNode *Imag = ASTBuilder::Instance().CreateASTMPDecimalNode(
          IId, ASTMPDecimalNode::DefaultBits, 0.0);
      assert(Imag && "Could not create a zero imaginary ASTMPDecimalNode!");

      ASTMPComplexNode *MPC = ASTBuilder::Instance().CreateASTMPComplexNode(
          CId, R, Imag, ASTOpTypeAdd, ASTMPComplexNode::DefaultBits);
      assert(MPC && "Could not create a valid ASTMPComplexNode!");

      CId->SetPolymorphicName(SCN.str());
      MPC->Mangle();
      MPC->MangleLiteral();
      ToGateParamSymbolTable(CId, CId->GetSymbolTableEntry());
      Promoted.emplace_back(ASTTypeMPComplex, MPC);
      ++PromoIX;
    }
    Params.swap(Promoted);

    if (C != Params.size() || GetNumComplexParams() != C) {
      std::stringstream M;
      M << "The disp gate expects exactly one complex parameter.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
      return;
    }
  } else if (C != Params.size()) {
    std::stringstream M;
    M << C
      << " inconsistent parameters in the gate call for the "
         "corresponding gate definition";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return;
  }

  C = 0;
  Ty = ASTTypeUndefined;

  for (ASTAnyTypeList::const_iterator I = QL.begin(); I != QL.end(); ++I) {
    Ty = (*I).second;
    switch (Ty) {
    case ASTTypeExpression: {
      const ASTExpression *EN = nullptr;

      try {
        EN = std::any_cast<const ASTExpression *>((*I).first);
        (void)EN; // Quiet.
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeIdentifier: {
      try {
        ASTIdentifierNode *QId = std::any_cast<ASTIdentifierNode *>((*I).first);
        assert(QId && "Invalid ASTIdentifierNode!");

        std::stringstream M;
        ASTSymbolTableEntry *QSTE = QId->GetSymbolTableEntry();
        assert(QSTE && "Gate Qubit Argument has no SymbolTable Entry!");

        if (!QSTE->HasValue()) {
          ASTGateOperandParamNode *QPN = new ASTGateOperandParamNode(
              QId, C, QId->GetBits(), QId->GetName());
          assert(QPN && "Could not create a valid ASTGateOperandParamNode!");
          QPN->Mangle();
          QSTE->ResetValue();
          QSTE->SetValue(new ASTValue<>(QPN, ASTTypeGateOperandParam),
                         ASTTypeGateOperandParam);
        }

        assert(QSTE->HasValue() &&
               "Gate Qubit Parameter SymbolTable Entry has no Value!");

        OperandParams.push_back(QSTE);
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    case ASTTypeIdentifierRef: {
      try {
        ASTIdentifierRefNode *IdR =
            std::any_cast<ASTIdentifierRefNode *>((*I).first);
        assert(IdR && "Invalid ASTIdentifierNodeRef!");

        ASTIdentifierNode *QId =
            const_cast<ASTIdentifierNode *>(IdR->GetIdentifier());
        assert(QId && "Invalid ASTIdentifierNode!");

        ASTSymbolTableEntry *QSTE = QId->GetSymbolTableEntry();
        assert(QSTE && "Gate Qubit Argument has no SymbolTable Entry!");
        assert(QSTE->HasValue() &&
               "Gate Qubit Argument SymbolTable Entry has no Value!");

        OperandParams.push_back(QSTE);
        if (IdR->IsInductionVariable())
          OperandParamIds.insert(
              std::make_pair(C, IdR->GetInductionVariable()));
        else if (IdR->IsIndexedIdentifier())
          OperandParamIds.insert(
              std::make_pair(C, IdR->GetIndexedIdentifier()));
      } catch (const std::bad_any_cast &E) {
        std::stringstream M;
        M << "std::bad_any_cast caught at index " << C << ": " << E.what();
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      } catch (...) {
        std::stringstream M;
        M << "Unknown exception caught at index " << C << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
      }
    } break;
    default: {
      std::stringstream M;
      M << "Only Qubit Identifiers are allowed as Gate Qubit arguments.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
    } break;
    }

    ++C;
  }

  assert(C == OperandParams.size() && "Inconsistent number of OperandParams!");
  Mangle();
}

ASTGateNode::ASTGateNode(const ASTIdentifierNode *Id,
                         const ASTParameterList &PL,
                         const ASTIdentifierList &IL, bool IsGateCall,
                         const ASTGateQOpList &OL)
    : ASTExpressionNode(Id, ASTTypeGate), Params(), Operands(), OperandParams(),
      OpList(OL), Ctrl(nullptr), GDId(IsGateCall ? nullptr : Id), GSTM(),
      ControlType(ASTTypeUndefined), Opaque(false), GateCall(IsGateCall),
      FullyTyped(false), FormalParamTypes(), FormalParamArraySizes(),
      FormalQuantumTypes(), TemplateParams(),
      FormalParamArraySizeTemplateIndices() {
  unsigned C = 0;
  std::set<std::string> PNS;
  std::vector<const ASTIdentifierNode *> NQV;

  if (!IsGateCall && Id->GetName() == u8"U") {
    MaterializeBuiltinUGate(Id, PL, IL);
    return;
  }

  for (ASTParameterList::const_iterator I = PL.begin(); I != PL.end(); ++I) {
    ASTParameter *AP = dynamic_cast<ASTParameter *>(*I);
    assert(AP && "Could not dynamic_cast to an ASTParameter!");

    // Explicitly typed classical formals (fully-typed gate decls).
    if (const ASTDeclarationNode *PDN = AP->GetDeclaration()) {
      ASTType PTy = PDN->GetASTType();
      // Prefer the Identifier's bound symbol type when the Decl wrapper
      // reports a generic/expression type for array formals.
      if (const ASTIdentifierNode *PId = AP->GetIdentifier()) {
        ASTType STy = PId->GetSymbolType();
        if (STy == ASTTypeMPComplexArray || STy == ASTTypeAngleArray ||
            STy == ASTTypeFloatArray || STy == ASTTypeMPDecimalArray)
          PTy = STy;
      }
      if (PTy == ASTTypeMPComplex) {
        const ASTIdentifierNode *CId = AP->GetIdentifier();
        assert(CId && "Typed complex gate param has no Identifier!");

        ASTSymbolTableEntry *CSTE =
            const_cast<ASTSymbolTableEntry *>(CId->GetSymbolTableEntry());
        if (!CSTE)
          CSTE = ASTSymbolTable::Instance().Lookup(
              CId->GetName(), CId->GetBits(), ASTTypeMPComplex);
        assert(CSTE && CSTE->HasValue() &&
               "Typed complex gate param has no SymbolTable Entry!");

        ASTMPComplexNode *MPC =
            CSTE->GetValue()->GetValue<ASTMPComplexNode *>();
        assert(MPC && "Could not obtain a valid ASTMPComplexNode!");

        ToGateParamSymbolTable(CId, CSTE);
        if (CId)
          const_cast<ASTIdentifierNode *>(CId)->SetNoQubit(true);
        AddParam(ASTTypeMPComplex, MPC);
        PNS.insert(AP->GetName());
        PNS.insert(CId->GetName());
        ++C;
        continue;
      }

      if (PTy == ASTTypeAngleArray || PTy == ASTTypeFloatArray ||
          PTy == ASTTypeMPDecimalArray || PTy == ASTTypeMPComplexArray) {
        const ASTIdentifierNode *AId = AP->GetIdentifier();
        assert(AId && "Typed array gate param has no Identifier!");

        ASTSymbolTableEntry *ASTE =
            const_cast<ASTSymbolTableEntry *>(AId->GetSymbolTableEntry());
        if (!ASTE)
          ASTE = ASTSymbolTable::Instance().Lookup(AId->GetName(),
                                                   AId->GetBits(), PTy);
        assert(ASTE && ASTE->HasValue() &&
               "Typed array gate param has no SymbolTable Entry!");

        // Erase the formal from LSTM before publishing to GSTM so later
        // TransferLocalContextSymbols / InsertLocal cannot collide.
        ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(), PTy);
        ASTSymbolTable::Instance().EraseLocalSymbol(AId, AId->GetBits(), PTy);

        // Keep declared STE identity for body lookup (thetas[i] / alphas[i]).
        ToGateParamSymbolTable(AId, ASTE);
        if (AId)
          const_cast<ASTIdentifierNode *>(AId)->SetNoQubit(true);

        if (PTy == ASTTypeMPComplexArray) {
          ASTMPComplexArrayNode *CAN =
              ASTE->GetValue()->GetValue<ASTMPComplexArrayNode *>();
          if (!CAN) {
            ASTArrayNode *ARN = ASTE->GetValue()->GetValue<ASTArrayNode *>();
            CAN = dynamic_cast<ASTMPComplexArrayNode *>(ARN);
          }
          if (!CAN && PDN->GetExpression())
            CAN = dynamic_cast<ASTMPComplexArrayNode *>(
                const_cast<ASTExpressionNode *>(PDN->GetExpression()));
          assert(CAN && "Typed complex-array gate param is not an "
                        "ASTMPComplexArrayNode!");
          AddParam(ASTTypeMPComplexArray, CAN);
        } else if (PTy == ASTTypeAngleArray) {
          ASTAngleArrayNode *AAN =
              ASTE->GetValue()->GetValue<ASTAngleArrayNode *>();
          if (!AAN) {
            ASTArrayNode *ARN = ASTE->GetValue()->GetValue<ASTArrayNode *>();
            AAN = dynamic_cast<ASTAngleArrayNode *>(ARN);
          }
          if (!AAN && PDN->GetExpression())
            AAN = dynamic_cast<ASTAngleArrayNode *>(
                const_cast<ASTExpressionNode *>(PDN->GetExpression()));
          assert(AAN && "Could not obtain a valid ASTAngleArrayNode!");
          AddParam(ASTTypeAngleArray, AAN);
        } else {
          // FloatArray / MPDecimalArray: store the declared array in Params
          // (type-faithful). Do not synthesize a .gatearray AngleArray view.
          ASTArrayNode *ARN = ASTE->GetValue()->GetValue<ASTArrayNode *>();
          if (!ARN) {
            if (PTy == ASTTypeMPDecimalArray)
              ARN = ASTE->GetValue()->GetValue<ASTMPDecimalArrayNode *>();
            else if (PTy == ASTTypeFloatArray)
              ARN = ASTE->GetValue()->GetValue<ASTFloatArrayNode *>();
          }
          if (!ARN && PDN->GetExpression())
            ARN = dynamic_cast<ASTArrayNode *>(
                const_cast<ASTExpressionNode *>(PDN->GetExpression()));
          assert(ARN && "Typed array gate param is not an ASTArrayNode!");
          AddParam(PTy, ARN);
        }

        PNS.insert(AP->GetName());
        PNS.insert(AId->GetName());
        ++C;
        continue;
      }

      if (PTy == ASTTypeFloat || PTy == ASTTypeDouble ||
          PTy == ASTTypeMPDecimal) {
        // Type-faithful Params: keep the declared float/decimal binding.
        const ASTIdentifierNode *PId = AP->GetIdentifier();
        assert(PId && "Typed classical gate param has no Identifier!");

        ASTSymbolTableEntry *PSTE =
            const_cast<ASTSymbolTableEntry *>(PId->GetSymbolTableEntry());
        if (!PSTE)
          PSTE = ASTSymbolTable::Instance().Lookup(PId->GetName(),
                                                   PId->GetBits(), PTy);
        assert(PSTE && PSTE->HasValue() &&
               "Typed float/decimal gate param has no SymbolTable Entry!");

        ToGateParamSymbolTable(PId, PSTE);
        if (PId)
          const_cast<ASTIdentifierNode *>(PId)->SetNoQubit(true);

        ASTExpressionNode *EN = nullptr;
        if (PTy == ASTTypeMPDecimal)
          EN = PSTE->GetValue()->GetValue<ASTMPDecimalNode *>();
        else if (PTy == ASTTypeFloat)
          EN = PSTE->GetValue()->GetValue<ASTFloatNode *>();
        else
          EN = PSTE->GetValue()->GetValue<ASTDoubleNode *>();
        if (!EN && PDN->GetExpression())
          EN = const_cast<ASTExpressionNode *>(PDN->GetExpression());
        assert(EN && "Typed float/decimal gate param has no value node!");

        AddParam(PTy, EN);
        PNS.insert(AP->GetName());
        PNS.insert(PId->GetName());
        ++C;
        continue;
      }

      if (PTy == ASTTypeInt || PTy == ASTTypeUInt || PTy == ASTTypeMPInteger ||
          PTy == ASTTypeMPUInteger || PTy == ASTTypeBool ||
          PTy == ASTTypeBitset || PTy == ASTTypeDuration) {
        // Non-float classical scalars still use an angle carrier so existing
        // gate-body angle machinery keeps working (Phase 1 scope).
        const ASTIdentifierNode *PId = AP->GetIdentifier();
        assert(PId && "Typed classical gate param has no Identifier!");

        ASTIdentifierNode *AId = ASTBuilder::Instance().CreateASTIdentifierNode(
            AP->GetName() + ".gateparam", ASTAngleNode::AngleBits,
            ASTTypeAngle);
        assert(AId && "Could not create an Angle ASTIdentifierNode!");
        AId->SetPolymorphicName(AP->GetName());

        ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AP->GetName());
        ASTAngleNode *AN = ASTBuilder::Instance().CreateASTAngleNode(
            AId, ATy, ASTAngleNode::AngleBits);
        assert(AN && "Could not create a valid ASTAngleNode!");

        AN->SetGateParamName(AP->GetName());
        ASTSymbolTableEntry *ASTE = ASTSymbolTable::Instance().Lookup(
            AId, AId->GetBits(), AId->GetSymbolType());
        assert(ASTE && "Angle ASTIdentifierNode has no SymbolTable Entry!");
        AId->SetLocalScope();
        ASTE->SetLocalScope();
        AId->SetSymbolTableEntry(ASTE);
        AN->Mangle();
        AN->MangleLiteral();
        ToGateParamSymbolTable(AId, ASTE);

        // Also publish under the formal name used in the gate body.
        ToGateParamSymbolTable(PId, PId->GetSymbolTableEntry());

        AddParam(ASTTypeAngle, AN);
        PNS.insert(AP->GetName());
        PNS.insert(AN->GetName());
        ++C;
        continue;
      }

      // ASTTypeAngle and untyped (implicit-angle) NamedTypeDecls fall through
      // to the historical angle materialization path below.
    }

    ASTSymbolTableEntry *ASTE = nullptr;
    ASTAngleNode *AN = nullptr;

    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(AP->GetName())) {
      const ASTSymbolTableEntry *STE =
          ASTSymbolTable::Instance().FindAngle(AP->GetName());
      assert(STE &&
             "Could not obtain a valid reserved Angle SymbolTable Entry!");
      AN = STE->GetValue()->GetValue<ASTAngleNode *>();
    } else {
      const ASTSymbolTableEntry *STE =
          ASTSymbolTable::Instance().Lookup(AP->GetName(), ASTTypeAngle);
      if (STE) {
        if (!STE->HasValue()) {
          ASTAngleNode *XAN = CreateAngleSymbolTableEntry(
              const_cast<ASTSymbolTableEntry *>(STE));
          assert(XAN && "Could not create a valid ASTAngleNode!");
        }

        AN = STE->GetValue()->GetValue<ASTAngleNode *>();
      }
    }

    if (!AN) {
      ASTIdentifierNode *AId = ASTBuilder::Instance().CreateASTIdentifierNode(
          AP->GetName(), ASTAngleNode::AngleBits, ASTTypeAngle);
      assert(AId && "Could not create an Angle ASTIdentifierNode!");

      ASTAngleType ATy = ASTAngleNode::DetermineAngleType(AId->GetName());
      AN = ASTBuilder::Instance().CreateASTAngleNode(AId, ATy,
                                                     ASTAngleNode::AngleBits);
      assert(AN && "Could not create a valid ASTAngleNode!");

      AN->SetGateParamName(AId->GetName());
      AId->SetPolymorphicName(AP->GetName());
      ASTE = ASTSymbolTable::Instance().Lookup(AId, AId->GetBits(),
                                               AId->GetSymbolType());
      assert(ASTE &&
             "ASTAngleNode ASTIdentifierNode has no SymbolTable Entry!");

      AId->SetLocalScope();
      ASTE->SetLocalScope();
      AId->SetSymbolTableEntry(ASTE);
      AN->Mangle();
      AN->MangleLiteral();
      ASTSymbolTable::Instance().TransferAngleToLSTM(AId, AId->GetBits(),
                                                     AId->GetSymbolType());
      ToGateParamSymbolTable(AId, ASTE);
      ASTSymbolTable::Instance().EraseLocalSymbol(AId, AId->GetBits(),
                                                  AId->GetSymbolType());
      ASTSymbolTable::Instance().EraseLocalAngle(AId);
      ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                            AId->GetSymbolType());
    } else {
      const ASTIdentifierNode *AId = AN->GetIdentifier();
      assert(AId && "ASTAngleNode has an invalid ASTIdentifierNode!");

      ASTE = ASTSymbolTable::Instance().Lookup(AId->GetName(), AId->GetBits(),
                                               ASTTypeAngle);
      assert(ASTE && "ASTIdentifierNode did not create a SymbolTable Entry!");

      if (const ASTIdentifierNode *PId = AP->GetIdentifier())
        if (AId->GetBits() == PId->GetBits())
          AN = AN->Clone(AId);
        else
          AN = AN->Clone(AId, PId->GetBits());
      else
        AN = AN->Clone(AId);

      assert(AN && "Could not clone a valid ASTAngleNode!");

      AN->SetGateParamName(AId->GetName());
      AId->SetPolymorphicName(AId->GetName());
      ASTE->ResetValue();
      ASTE->SetValue(new ASTValue<>(AN, ASTTypeAngle), ASTTypeAngle);
      assert(ASTE->HasValue() &&
             "ASTAngleNode SymbolTable Entry has no Value!");

      ASTE->SetLocalScope();
      AN->Mangle();
      AN->MangleLiteral();
      ToGateParamSymbolTable(AId, ASTE);
      ASTSymbolTable::Instance().EraseLocalSymbol(AId, AId->GetBits(),
                                                  AId->GetSymbolType());
      ASTSymbolTable::Instance().EraseLocalAngle(AId);
      ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                            AId->GetSymbolType());
      const_cast<ASTIdentifierNode *>(AId)->SetLocalScope();
      const_cast<ASTIdentifierNode *>(AId)->SetSymbolTableEntry(ASTE);
      const_cast<ASTIdentifierNode *>(AId)->SetHasSymbolTableEntry(true);
    }

    if (const ASTIdentifierNode *PId =
            ASTBuilder::Instance().FindASTIdentifierNode(AP->GetName())) {
      PId->SetNoQubit(true);
    }

    AddParam(ASTTypeAngle, AN);
    PNS.insert(AP->GetName());
    PNS.insert(AN->GetName());
    ++C;
  }

  if (C != Params.size()) {
    std::stringstream M;
    M << C
      << " inconsistent parameters in the gate call for the "
         "corresponding gate definition";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return;
  }

  for (std::size_t I = 0; I < Params.size(); ++I) {
    if (Params[I].Ty != ASTTypeAngle)
      continue;
    ASTAngleNode *AN = dynamic_cast<ASTAngleNode *>(Params[I].Expr);
    if (!AN)
      continue;
    const ASTIdentifierNode *AId = AN->GetIdentifier();
    assert(AId && "Invalid ASTIdentifierNode!");

    ASTSymbolTable::Instance().EraseLocalAngle(AId->GetName(), AId->GetBits(),
                                               ASTTypeAngle);
    ASTSymbolTable::Instance().EraseLocal(AId, AId->GetBits(),
                                          AId->GetSymbolType());
  }

  C = 0;

  for (ASTIdentifierList::const_iterator I = IL.begin(); I != IL.end(); ++I) {
    ASTIdentifierNode *QId = const_cast<ASTIdentifierNode *>(*I);
    assert(QId && "Invalid Qubit ASTIdentifierNode!");

    ASTIdentifierRefNode *IdR = nullptr;

    if (QId->IsReference() &&
        (IdR = dynamic_cast<ASTIdentifierRefNode *>(QId))) {
      std::stringstream M;
      M << "Indexed Qubit register references cannot be used as Gate "
        << "Qubit Parameters.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::Error);
    }

    if ((PNS.find(QId->GetName()) != PNS.end()) || QId->IsNoQubit()) {
      NQV.push_back(QId);
      continue;
    }

    MaterializeGateOperandParam(QId);

    ASTSymbolTableEntry *STE = nullptr;
    std::map<std::string, const ASTSymbolTableEntry *>::iterator QI =
        GSTM.find(QId->GetName());

    if (QI != GSTM.end())
      STE = const_cast<ASTSymbolTableEntry *>((*QI).second);

    if (!STE) {
      STE = new ASTSymbolTableEntry(QId, QId->GetSymbolType());
      assert(STE && "Could not create a valid SymbolTable Entry!");
    }

    if (STE && STE->GetValueType() == ASTTypeUndefined) {
      if (GateCall) {
        std::stringstream M;
        M << "Undefined Types (" << QId->GetName() << ") are not "
          << "permitted in a Gate Call.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(QId), M.str(),
            DiagLevel::ICE);
      }

      QId->SetGateLocal();
      QId->SetLocalScope();
      QId->SetBits(1);
      STE->SetLocalScope();
      STE->SetValueType(ASTTypeGateOperandParam);
    }

    QId->SetLocalScope();

    if (!STE->HasValue() &&
        (QId->GetSymbolType() == ASTTypeQubitContainer ||
         QId->GetSymbolType() == ASTTypeQubitContainerAlias)) {
      std::stringstream M;
      M << "An uninitialized qubit container cannot be used as "
           "gate call argument.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(QId), M.str(),
          DiagLevel::Error);
    }

    unsigned Bits = 0U;
    unsigned QBits = 0U;

    switch (QId->GetSymbolType()) {
    case ASTTypeQubitContainer: {
      if (ASTQubitContainerNode *QCN =
              STE->GetValue()->GetValue<ASTQubitContainerNode *>()) {
        Bits = QCN->Size();
        QBits = 1U;
      }
    } break;
    case ASTTypeQubitContainerAlias: {
      if (ASTQubitContainerAliasNode *QCAN =
              STE->GetValue()->GetValue<ASTQubitContainerAliasNode *>()) {
        Bits = QCAN->Size();
        QBits = 1U;
      }
    } break;
    case ASTTypeQumodeContainer: {
      if (ASTQumodeContainerNode *QCN =
              STE->GetValue()->GetValue<ASTQumodeContainerNode *>()) {
        Bits = QCN->Size();
        QBits = 1U;
      }
    } break;
    case ASTTypeQubit:
    case ASTTypeQumode:
    case ASTTypeGateOperandParam:
      Bits = QBits = 1U;
      break;
    default:
      break;
    }

    if (STE) {
      STE->SetLocalScope();
      OperandParams.push_back(STE);
    }

    STE = MangleGateOperandParam(QId, STE, C, Bits, QBits);
    ToGateParamSymbolTable(QId, STE);

    if (IdR)
      ToGateParamSymbolTable(IdR, STE);

    if (!QId->IsGateLocal()) {
      switch (STE->GetValueType()) {
      case ASTTypeQubit:
      case ASTTypeQubitContainer:
      case ASTTypeQubitContainerAlias:
      case ASTTypeQumode:
      case ASTTypeQumodeContainer:
      case ASTTypeGateOperandParam:
        break;
      default:
        continue;
        break;
      }
    }

    ASTGateQubitTracker::Instance().Insert(QId);

    // Do not insert the gate identifier into the qubit param's
    // ast identifier. It will cause scope resolution conflicts.
    std::stringstream QS;
    QS << "ast-gate-operand-param-" << QId->GetName() << '-' << C;

    ASTIdentifierNode *QBId =
        new ASTIdentifierNode(QS.str(), ASTTypeGateOperandParam, 0U);
    assert(QBId && "Could not create a valid Qubit ASTIdentifierNode!");

    QBId->SetPolymorphicName(QId->GetName());

    const ASTType OpQTy = ResolveOperandQuantumType(QId, C);
    ASTQubitNode *OpN = nullptr;
    if (OpQTy == ASTTypeQumode)
      OpN = new ASTQumodeNode(QBId, C, QId->GetName());
    else
      OpN = new ASTQubitNode(QBId, C, QId->GetName());
    assert(OpN && "Could not create a valid gate Operand node!");

    Operands.push_back(OpN);
    Operands.back()->Mangle();
    ASTQubitNodeBuilder::Instance().Append(Operands.back());
    ASTSymbolTable::Instance().EraseGateLocalQubit(QId->GetName());
    ++C;
  }

  assert(C == Operands.size() && "Inconsistent number of Operands!");

  OL.TransferToSymbolTable(GSTM);
  Mangle();
}

void ASTGateNode::print() const {
  std::cout << "<Gate>" << std::endl;
  std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
  std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
            << std::endl;
  std::cout << "<Opaque>" << std::boolalpha << Opaque << "</Opaque>"
            << std::endl;
  std::cout << "<GateCall>" << std::boolalpha << GateCall << "</GateCall>"
            << std::endl;

  if (GateCall) {
    std::cout << "<GateDefinitionName>" << GDId->GetName()
              << "</GateDefinitionName>" << std::endl;
  }

  std::cout << "<FullyTyped>" << std::boolalpha << FullyTyped << "</FullyTyped>"
            << std::endl;

  // Source order: gate name[TemplateParams](Params) quantum-operands
  if (!TemplateParams.empty()) {
    std::cout << "<TemplateParams>" << std::endl;
    for (std::size_t I = 0; I < TemplateParams.size(); ++I) {
      std::cout << "<TemplateParam>" << std::endl;
      std::cout << "<Type>" << PrintTypeEnum(TemplateParams[I].Ty) << "</Type>"
                << std::endl;
      std::cout << "<Name>" << TemplateParams[I].Name << "</Name>" << std::endl;
      // Decl: NaN placeholder (like angle Params). Call: bound value.
      // Body identifiers named Name are not overwritten.
      if (TemplateParams[I].HasBound())
        std::cout << "<Value>" << std::dec << *TemplateParams[I].Bound
                  << "</Value>" << std::endl;
      else
        std::cout << "<Value>NaN</Value>" << std::endl;
      std::cout << "</TemplateParam>" << std::endl;
    }
    std::cout << "</TemplateParams>" << std::endl;
  }

  if (!FormalParamTypes.empty()) {
    std::cout << "<FormalParamTypes>" << std::endl;
    for (std::size_t I = 0; I < FormalParamTypes.size(); ++I) {
      std::cout << "<FormalParamType>" << std::endl;
      std::cout << "<Type>" << PrintTypeEnum(FormalParamTypes[I]) << "</Type>"
                << std::endl;
      if (I < FormalParamArraySizes.size() && FormalParamArraySizes[I] != 0U)
        std::cout << "<ArraySize>" << std::dec << FormalParamArraySizes[I]
                  << "</ArraySize>" << std::endl;
      if (I < FormalParamArraySizeTemplateIndices.size() &&
          FormalParamArraySizeTemplateIndices[I] !=
              static_cast<unsigned>(~0U) &&
          FormalParamArraySizeTemplateIndices[I] < TemplateParams.size())
        std::cout << "<ArraySizeTemplate>"
                  << TemplateParams[FormalParamArraySizeTemplateIndices[I]].Name
                  << "</ArraySizeTemplate>" << std::endl;
      std::cout << "</FormalParamType>" << std::endl;
    }
    std::cout << "</FormalParamTypes>" << std::endl;
  }

  if (!FormalQuantumTypes.empty()) {
    std::cout << "<FormalQuantumTypes>" << std::endl;
    for (std::size_t I = 0; I < FormalQuantumTypes.size(); ++I)
      std::cout << "<Type>" << PrintTypeEnum(FormalQuantumTypes[I]) << "</Type>"
                << std::endl;
    std::cout << "</FormalQuantumTypes>" << std::endl;
  }

  if (!Params.empty()) {
    std::cout << "<Params>" << std::endl;
    for (std::size_t I = 0; I < Params.size(); ++I) {
      std::cout << "<Param>" << std::endl;
      std::cout << "<Type>" << PrintTypeEnum(Params[I].Ty) << "</Type>"
                << std::endl;
      if (Params[I].Expr)
        Params[I].Expr->print();
      std::cout << "</Param>" << std::endl;
    }
    std::cout << "</Params>" << std::endl;
  }

  if (!Operands.empty()) {
    std::cout << "<Operands>" << std::endl;
    for (std::vector<ASTQubitNode *>::const_iterator I = Operands.begin();
         I != Operands.end(); ++I)
      (*I)->print();
    std::cout << "</Operands>" << std::endl;
  }

  // Always print OperandParams when present (decls and calls). Call consumers
  // use these names with FormalQuantumTypes; Operands may be empty.
  if (!OperandParams.empty()) {
    std::cout << "<OperandParams>" << std::endl;
    unsigned XC = 0;
    std::map<unsigned, const ASTIdentifierNode *>::const_iterator MI;

    for (std::vector<const ASTSymbolTableEntry *>::const_iterator I =
             OperandParams.begin();
         I != OperandParams.end(); ++I) {
      const ASTIdentifierNode *QId = (*I)->GetIdentifier();
      assert(QId &&
             "Invalid ASTIdentifierNode obtained from the SymbolTable Entry!");
      std::cout << "<OperandParam>" << std::endl;
      const std::string &QN = QId->GetName();
      MI = OperandParamIds.find(XC);
      if (MI != OperandParamIds.end() &&
          ASTStringUtils::Instance().IsIndexed(QN)) {
        std::string BN = ASTStringUtils::Instance().GetIdentifierBase(QN);
        std::cout << "<Name>" << BN << '[' << (*MI).second->GetName() << ']'
                  << "</Name>" << std::endl;
      } else {
        std::cout << "<Name>" << QId->GetName() << "</Name>" << std::endl;
      }
      std::cout << "</OperandParam>" << std::endl;
      ++XC;
    }

    std::cout << "</OperandParams>" << std::endl;
  }

  if (!OpList.Empty())
    OpList.print();

  switch (ControlType) {
  case ASTTypeGateControl:
  case ASTTypeGateNegControl:
  case ASTTypeGateInverse:
  case ASTTypeGatePower:
    std::cout << "<ControlType>";
    std::cout << PrintTypeEnum(ControlType);
    std::cout << "</ControlType>" << std::endl;
    break;
  default:
    break;
  }

  std::cout << "</Gate>" << std::endl;
}

ASTGateControlNode::ASTGateControlNode(const ASTGateNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GN(N),
      TType(ASTTypeGate), MV(nullptr), MType(ASTTypeUndefined), CBits(1U) {
  GN->SetControl(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GN(N),
      TType(ASTTypeGate), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  GN->SetControl(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateQOpNode *QN)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GQN(QN),
      TType(ASTTypeGateQOpNode), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {}

ASTGateControlNode::ASTGateControlNode(const ASTGateQOpNode *QN,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GQN(QN),
      TType(ASTTypeGateQOpNode), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {}

ASTGateControlNode::ASTGateControlNode(const ASTGateControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), CN(N),
      TType(ASTTypeGateControl), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateControlNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), CN(N),
      TType(ASTTypeGateControl), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateNegControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), NCN(N),
      TType(ASTTypeGateNegControl), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateNegControlNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), NCN(N),
      TType(ASTTypeGateNegControl), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGatePowerNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), PN(N),
      TType(ASTTypeGatePower), MV(nullptr), MType(ASTTypeUndefined), CBits(1U) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGatePowerNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), PN(N),
      TType(ASTTypeGatePower), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateInverseNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), IN(N),
      TType(ASTTypeGateInverse), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGateInverseNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), IN(N),
      TType(ASTTypeGateInverse), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateControlNode::ASTGateControlNode(const ASTGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GPN(N),
      TType(ASTTypeGPhaseExpression), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {}

ASTGateControlNode::ASTGateControlNode(const ASTGPhaseExpressionNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GPN(N),
      TType(ASTTypeGPhaseExpression), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {}

ASTGateControlNode::ASTGateControlNode(const ASTGateGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GGEN(N),
      TType(ASTTypeGateGPhaseExpression), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {}

ASTGateControlNode::ASTGateControlNode(const ASTGateGPhaseExpressionNode *N,
                                       const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::Ctrl, ASTTypeGateControl), GGEN(N),
      TType(ASTTypeGateGPhaseExpression), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GN(N), TType(ASTTypeGate), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  GN->SetNegControl(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateNode *N,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GN(N), TType(ASTTypeGate), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  GN->SetNegControl(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateQOpNode *QN)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GQN(QN), TType(ASTTypeGateQOpNode), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateQOpNode *QN,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GQN(QN), TType(ASTTypeGateQOpNode), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      CN(N), TType(ASTTypeGateControl), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateControlNode *N,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      CN(N), TType(ASTTypeGateControl), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateNegControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      NCN(N), TType(ASTTypeGateNegControl), MV(nullptr),
      MType(ASTTypeUndefined), CBits(1U) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateNegControlNode *N,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      NCN(N), TType(ASTTypeGateNegControl), MV(nullptr),
      MType(ASTTypeUndefined), CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGatePowerNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      PN(N), TType(ASTTypeGatePower), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGatePowerNode *N,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      PN(N), TType(ASTTypeGatePower), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateInverseNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      IN(N), TType(ASTTypeGateInverse), MV(nullptr), MType(ASTTypeUndefined),
      CBits(1U) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGateInverseNode *N,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      IN(N), TType(ASTTypeGateInverse), MV(nullptr), MType(ASTTypeUndefined),
      CBits(CB->GetUnsignedValue()) {
  N->SetModifier(this);
}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GPN(N), TType(ASTTypeGPhaseExpression), MV(nullptr),
      MType(ASTTypeUndefined), CBits(1U) {}

ASTGateNegControlNode::ASTGateNegControlNode(const ASTGPhaseExpressionNode *N,
                                             const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GPN(N), TType(ASTTypeGPhaseExpression), MV(nullptr),
      MType(ASTTypeUndefined), CBits(CB->GetUnsignedValue()) {}

ASTGateNegControlNode::ASTGateNegControlNode(
    const ASTGateGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GGEN(N), TType(ASTTypeGateGPhaseExpression), MV(nullptr),
      MType(ASTTypeUndefined), CBits(1U) {}

ASTGateNegControlNode::ASTGateNegControlNode(
    const ASTGateGPhaseExpressionNode *N, const ASTIntNode *CB)
    : ASTExpressionNode(&ASTIdentifierNode::NegCtrl, ASTTypeGateNegControl),
      GGEN(N), TType(ASTTypeGateGPhaseExpression), MV(nullptr),
      MType(ASTTypeUndefined), CBits(CB->GetUnsignedValue()) {}

ASTGateInverseNode::ASTGateInverseNode(const ASTGateNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), GN(N),
      TType(ASTTypeGate), MV(nullptr), MType(ASTTypeUndefined) {
  GN->SetInverse(this);
}

ASTGateInverseNode::ASTGateInverseNode(const ASTGateQOpNode *QN)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), GQN(QN),
      TType(ASTTypeGateQOpNode), MV(nullptr), MType(ASTTypeUndefined) {}

ASTGateInverseNode::ASTGateInverseNode(const ASTGateControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), CN(N),
      TType(ASTTypeGateControl), MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGateInverseNode::ASTGateInverseNode(const ASTGateNegControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), NCN(N),
      TType(ASTTypeGateNegControl), MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGateInverseNode::ASTGateInverseNode(const ASTGatePowerNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), PN(N),
      TType(ASTTypeGatePower), MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGateInverseNode::ASTGateInverseNode(const ASTGateInverseNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), IN(N),
      TType(ASTTypeGateInverse), MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGateInverseNode::ASTGateInverseNode(const ASTGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), GPN(N),
      TType(ASTTypeGPhaseExpression), MV(nullptr), MType(ASTTypeUndefined) {}

ASTGateInverseNode::ASTGateInverseNode(const ASTGateGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Inv, ASTTypeGateInverse), GGEN(N),
      TType(ASTTypeGateGPhaseExpression), MV(nullptr), MType(ASTTypeUndefined) {
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNode *N, const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      GN(N), EType(Exp->GetASTType()), TType(ASTTypeGate), MV(nullptr),
      MType(ASTTypeUndefined) {
  GN->SetPower(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNode *N,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      GN(N), EType(Exp->GetASTType()), TType(ASTTypeGate), MV(nullptr),
      MType(ASTTypeUndefined) {
  GN->SetPower(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNode *N,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      GN(N), EType(Exp->GetASTType()), TType(ASTTypeGate), MV(nullptr),
      MType(ASTTypeUndefined) {
  GN->SetPower(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNode *N,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      GN(N), EType(Exp->GetASTType()), TType(ASTTypeGate), MV(nullptr),
      MType(ASTTypeUndefined) {
  GN->SetPower(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateQOpNode *QN)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), GQN(QN),
      EType(ASTTypeUndefined), TType(ASTTypeGateQOpNode), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateQOpNode *QN,
                                   const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      GQN(QN), EType(Exp->GetASTType()), TType(ASTTypeGateQOpNode), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateQOpNode *QN,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      GQN(QN), EType(Exp->GetASTType()), TType(ASTTypeGateQOpNode), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateQOpNode *QN,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      GQN(QN), EType(Exp->GetASTType()), TType(ASTTypeGateQOpNode), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateQOpNode *QN,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      GQN(QN), EType(Exp->GetASTType()), TType(ASTTypeGateQOpNode), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), CN(N),
      EType(ASTTypeUndefined), TType(ASTTypeGateControl), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateControlNode *N,
                                   const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      CN(N), EType(Exp->GetASTType()), TType(ASTTypeGateControl), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateControlNode *N,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      CN(N), EType(Exp->GetASTType()), TType(ASTTypeGateControl), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateControlNode *N,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      CN(N), EType(Exp->GetASTType()), TType(ASTTypeGateControl), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateControlNode *N,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      CN(N), EType(Exp->GetASTType()), TType(ASTTypeGateControl), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNegControlNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), NCN(N),
      EType(ASTTypeUndefined), TType(ASTTypeGateNegControl), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNegControlNode *N,
                                   const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      NCN(N), EType(Exp->GetASTType()), TType(ASTTypeGateNegControl),
      MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNegControlNode *N,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      NCN(N), EType(Exp->GetASTType()), TType(ASTTypeGateNegControl),
      MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNegControlNode *N,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      NCN(N), EType(Exp->GetASTType()), TType(ASTTypeGateNegControl),
      MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateNegControlNode *N,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      NCN(N), EType(Exp->GetASTType()), TType(ASTTypeGateNegControl),
      MV(nullptr), MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGatePowerNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), PN(N),
      EType(ASTTypeUndefined), TType(ASTTypeGatePower), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGatePowerNode *N,
                                   const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      PN(N), EType(Exp->GetASTType()), TType(ASTTypeGatePower), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGatePowerNode *N,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      PN(N), EType(Exp->GetASTType()), TType(ASTTypeGatePower), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGatePowerNode *N,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      PN(N), EType(Exp->GetASTType()), TType(ASTTypeGatePower), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGatePowerNode *N,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      PN(N), EType(Exp->GetASTType()), TType(ASTTypeGatePower), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateInverseNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), IN(N),
      EType(ASTTypeUndefined), TType(ASTTypeGateInverse), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateInverseNode *N,
                                   const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      IN(N), EType(Exp->GetASTType()), TType(ASTTypeGateInverse), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateInverseNode *N,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      IN(N), EType(Exp->GetASTType()), TType(ASTTypeGateInverse), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateInverseNode *N,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      IN(N), EType(Exp->GetASTType()), TType(ASTTypeGateInverse), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateInverseNode *N,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      IN(N), EType(Exp->GetASTType()), TType(ASTTypeGateInverse), MV(nullptr),
      MType(ASTTypeUndefined) {
  N->SetModifier(this);
}

ASTGatePowerNode::ASTGatePowerNode(const ASTGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), GPN(N),
      EType(ASTTypeUndefined), TType(ASTTypeGPhaseExpression), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), GGEN(N),
      EType(ASTTypeUndefined), TType(ASTTypeGateGPhaseExpression), MV(nullptr),
      MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N,
                                   const ASTIntNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), I(Exp),
      GGEN(N), EType(Exp->GetASTType()), TType(ASTTypeGateGPhaseExpression),
      MV(nullptr), MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N,
                                   const ASTIdentifierNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), ID(Exp),
      GGEN(N), EType(Exp->GetASTType()), TType(ASTTypeGateGPhaseExpression),
      MV(nullptr), MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N,
                                   const ASTBinaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), BOP(Exp),
      GGEN(N), EType(Exp->GetASTType()), TType(ASTTypeGateGPhaseExpression),
      MV(nullptr), MType(ASTTypeUndefined) {}

ASTGatePowerNode::ASTGatePowerNode(const ASTGateGPhaseExpressionNode *N,
                                   const ASTUnaryOpNode *Exp)
    : ASTExpressionNode(&ASTIdentifierNode::Pow, ASTTypeGatePower), UOP(Exp),
      GGEN(N), EType(Exp->GetASTType()), TType(ASTTypeGateGPhaseExpression),
      MV(nullptr), MType(ASTTypeUndefined) {}

void ASTGateControlNode::print() const {
  std::cout << "<GateControlNode>" << std::endl;
  std::cout << "<Target>" << std::endl;

  switch (TType) {
  case ASTTypeGate:
    GN->print();
    break;
  case ASTTypeGateQOpNode:
    GQN->print();
    break;
  case ASTTypeGPhaseExpression:
    GPN->print();
    break;
  case ASTTypeGateGPhaseExpression:
    GGEN->print();
    break;
  case ASTTypeGateControl:
    CN->print();
    break;
  case ASTTypeGateNegControl:
    NCN->print();
    break;
  case ASTTypeGatePower:
    PN->print();
    break;
  case ASTTypeGateInverse:
    IN->print();
    break;
  default:
    break;
  }

  std::cout << "</Target>" << std::endl;

  std::cout << "<HasModifier>" << std::boolalpha << HasModifier()
            << "</HasModifier>" << std::endl;
  std::cout << "<ControlBits>" << CBits << "</ControlBits>" << std::endl;

  std::cout << "</GateControlNode>" << std::endl;
}

void ASTGateNegControlNode::print() const {
  std::cout << "<GateNegControlNode>" << std::endl;
  std::cout << "<Target>" << std::endl;

  switch (TType) {
  case ASTTypeGate:
    GN->print();
    break;
  case ASTTypeGateQOpNode:
    GQN->print();
    break;
  case ASTTypeGPhaseExpression:
    GPN->print();
    break;
  case ASTTypeGateGPhaseExpression:
    GGEN->print();
    break;
  case ASTTypeGateControl:
    CN->print();
    break;
  case ASTTypeGateNegControl:
    NCN->print();
    break;
  case ASTTypeGatePower:
    PN->print();
    break;
  case ASTTypeGateInverse:
    IN->print();
    break;
  default:
    break;
  }

  std::cout << "</Target>" << std::endl;

  std::cout << "<HasModifier>" << std::boolalpha << HasModifier()
            << "</HasModifier>" << std::endl;
  std::cout << "<ControlBits>" << CBits << "</ControlBits>" << std::endl;

  std::cout << "</GateNegControlNode>" << std::endl;
}

void ASTGateInverseNode::print() const {
  std::cout << "<GateInverseNode>" << std::endl;
  std::cout << "<Target>" << std::endl;

  switch (TType) {
  case ASTTypeGate:
    GN->print();
    break;
  case ASTTypeGateQOpNode:
    GQN->print();
    break;
  case ASTTypeGPhaseExpression:
    GPN->print();
    break;
  case ASTTypeGateGPhaseExpression:
    GGEN->print();
    break;
  case ASTTypeGateControl:
    CN->print();
    break;
  case ASTTypeGateNegControl:
    NCN->print();
    break;
  case ASTTypeGatePower:
    PN->print();
    break;
  case ASTTypeGateInverse:
    IN->print();
    break;
  default:
    break;
  }

  std::cout << "</Target>" << std::endl;

  std::cout << "<HasModifier>" << std::boolalpha << HasModifier()
            << "</HasModifier>" << std::endl;

  std::cout << "</GateInverseNode>" << std::endl;
}

void ASTGatePowerNode::print() const {
  std::cout << "<GatePowerNode>" << std::endl;

  std::cout << "<Exponent>" << std::endl;
  if (EType == ASTTypeInt)
    I->print();
  else if (EType == ASTTypeIdentifier)
    ID->print();
  else if (EType == ASTTypeBinaryOp)
    BOP->print();
  else if (EType == ASTTypeUnaryOp)
    UOP->print();
  std::cout << "</Exponent>" << std::endl;

  std::cout << "<Target>" << std::endl;

  switch (TType) {
  case ASTTypeGate:
    std::cout << "<GateTarget>" << std::endl;
    GN->print();
    std::cout << "</GateTarget>" << std::endl;
    break;
  case ASTTypeGateQOpNode:
    GQN->print();
    break;
  case ASTTypeGPhaseExpression:
    GPN->print();
    break;
  case ASTTypeGateGPhaseExpression:
    GGEN->print();
    break;
  case ASTTypeGateControl:
    CN->print();
    break;
  case ASTTypeGateNegControl:
    NCN->print();
    break;
  case ASTTypeGatePower:
    PN->print();
    break;
  case ASTTypeGateInverse:
    IN->print();
    break;
  default:
    break;
  }

  std::cout << "</Target>" << std::endl;

  std::cout << "<HasModifier>" << std::boolalpha << HasModifier()
            << "</HasModifier>" << std::endl;

  std::cout << "</GatePowerNode>" << std::endl;
}

GateKind ASTGateNode::DetermineGateKind(const std::string &GN) {
  if (GN == u8"h")
    return ASTGateKindH;
  else if (GN == u8"U")
    return ASTGateKindU;
  else if (GN == u8"disp")
    return ASTGateKindDisp;

  std::string N = ASTStringUtils::Instance().ToLower(GN);

  if (N == u8"cx")
    return ASTGateKindCX;
  else if (N == u8"ccx")
    return ASTGateKindCCX;
  else if (N == u8"cnot")
    return ASTGateKindCNOT;

  return ASTGateKindGeneric;
}

ASTType ASTGateNode::DetermineGateType(const std::string &GN) {
  std::string N = ASTStringUtils::Instance().ToLower(GN);
  return ASTTypeGate;
}

void ASTGateNode::MaterializeGateOperandParam(ASTIdentifierNode *Id) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  const ASTDeclarationContext *CTX = Id->GetDeclarationContext();
  const ASTDeclarationContext *CCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
  ASTSymbolTableEntry *STE = Id->GetSymbolTableEntry();

  if (GateCall) {
    if (Id->GetSymbolType() == ASTTypeUndefined) {
      std::stringstream M;
      M << "Undefined Types (" << Id->GetName() << ") are not "
        << "permitted in a Gate Call.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    }

    if (CTX != CCX) {
      std::stringstream M;
      M << "Gate declaration context mismatch.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    }
  }

  if (Id->GetSymbolType() == ASTTypeUndefined) {
    Id->SetSymbolType(ASTTypeGateOperandParam);
    Id->SetBits(1U);
    Id->SetLocalScope();
    STE->SetValueType(ASTTypeGateOperandParam);
    STE->SetLocalScope();
  }

  if (Id->GetSymbolType() == ASTTypeGateOperandParam) {
    unsigned Bits = 1U;
    if (ASTStringUtils::Instance().IsIndexed(Id->GetName()))
      Bits = ASTStringUtils::Instance().GetIdentifierIndex(Id->GetName());
    assert(!ASTIdentifierNode::InvalidBits(Bits) &&
           "Invalid number of bits for ASTGateOperandParam!");

    if (!GSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
      std::stringstream M;
      M << "Failure inserting Qubit Param into the Gate Symbol Table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    }

    ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(),
                                                Id->GetSymbolType());
  } else if (ASTUtils::Instance().IsQuantumRegisterType(Id->GetSymbolType())) {
    // Fully-typed gate operands may RestoreType() back to qubit/qumode.
    if (!GSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
      std::stringstream M;
      M << "Failure inserting quantum Param into the Gate Symbol Table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    }

    ASTSymbolTable::Instance().EraseLocalSymbol(Id, Id->GetBits(),
                                                Id->GetSymbolType());
  } else {
    if (!GateCall) {
      std::stringstream M;
      M << "Only Qubit Parameters are permitted in a Gate declaration.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    }

    if (!GSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
      std::stringstream M;
      M << "Failure inserting Qubit Argument into the Gate Symbol Table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
    }
  }
}

ASTIdentifierNode *ASTGateNode::GateCallIdentifier(const std::string &Name,
                                                   ASTType GTy,
                                                   unsigned Bits) const {
  assert(!Name.empty() && "Invalid Gate Name!");

  ASTIdentifierNode *GId = new ASTIdentifierNode(Name, GTy, Bits);
  assert(GId && "Could not create a valid GateCall ASTIdentifierNode!");

  return GId;
}

ASTGateNode *ASTGateNode::CloneCall(const ASTIdentifierNode *Id,
                                    const ASTArgumentNodeList &AL,
                                    const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTGateNode *RG = new ASTGateNode(
      GId, AL, QL, true, ASTGateQOpList::EmptyDefault, &FormalParamTypes);
  assert(RG && "Could not create a valid ASTGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->FullyTyped = FullyTyped;
  RG->FormalParamArraySizes = FormalParamArraySizes;
  RG->FormalQuantumTypes = FormalQuantumTypes;
  RG->TemplateParams = TemplateParams;
  RG->FormalParamArraySizeTemplateIndices = FormalParamArraySizeTemplateIndices;
  RG->Mangle();
  return RG;
}

ASTUGateNode *ASTUGateNode::CloneCall(const ASTIdentifierNode *Id,
                                      const ASTArgumentNodeList &AL,
                                      const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTUGateNode *RG = new ASTUGateNode(GId, AL, QL, true);
  assert(RG && "Could not create a valid ASTUGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTDispGateNode *ASTDispGateNode::CloneCall(const ASTIdentifierNode *Id,
                                            const ASTArgumentNodeList &AL,
                                            const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTDispGateNode *RG = new ASTDispGateNode(GId, AL, QL, true);
  assert(RG && "Could not create a valid ASTDispGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTCXGateNode *ASTCXGateNode::CloneCall(const ASTIdentifierNode *Id,
                                        const ASTArgumentNodeList &AL,
                                        const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTCXGateNode *RG = new ASTCXGateNode(GId, AL, QL, true);
  assert(RG && "Could not create a valid ASTCXGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTCXGateNode *ASTCXGateNode::CloneCall(const ASTIdentifierNode *Id,
                                        const ASTParameterList &PL,
                                        const ASTIdentifierList &IL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTCXGateNode *RG = new ASTCXGateNode(GId, PL, IL, true);
  assert(RG && "Could not create a valid ASTCXGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTCCXGateNode *ASTCCXGateNode::CloneCall(const ASTIdentifierNode *Id,
                                          const ASTArgumentNodeList &AL,
                                          const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTCCXGateNode *RG = new ASTCCXGateNode(GId, AL, QL, true);
  assert(RG && "Could not create a valid ASTCCXGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTCCXGateNode *ASTCCXGateNode::CloneCall(const ASTIdentifierNode *Id,
                                          const ASTParameterList &PL,
                                          const ASTIdentifierList &IL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTCCXGateNode *RG = new ASTCCXGateNode(GId, PL, IL, true);
  assert(RG && "Could not create a valid ASTCXGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTHadamardGateNode *
ASTHadamardGateNode::CloneCall(const ASTIdentifierNode *Id,
                               const ASTArgumentNodeList &AL,
                               const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTHadamardGateNode *RG = new ASTHadamardGateNode(GId, AL, QL, true);
  assert(RG && "Could not create a valid ASTHadamardGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

ASTCNotGateNode *ASTCNotGateNode::CloneCall(const ASTIdentifierNode *Id,
                                            const ASTArgumentNodeList &AL,
                                            const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *GId =
      GateCallIdentifier(Id->GetName(), Id->GetSymbolType(), Id->GetBits());
  assert(GId && "Could not create a valid Gate ASTIdentifierNode!");

  GId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));
  ASTCNotGateNode *RG = new ASTCNotGateNode(GId, AL, QL, true);
  assert(RG && "Could not create a valid ASTCNotGateNode!");

  RG->OpList = OpList;
  RG->GDId = Id;
  RG->Void = Void;
  RG->ControlType = ControlType;
  RG->Opaque = Opaque;
  RG->GateCall = true;
  RG->Mangle();
  return RG;
}

void ASTGateNode::Mangle() {
  ASTMangler M;
  M.Start();

  if (GateCall)
    M.TypeIdentifier(ASTTypeGateCall, GetName());
  else
    M.TypeIdentifier(GetASTType(), GetName());

  if (GateCall) {
    unsigned X = 0;
    for (unsigned I = 0; I < Params.size(); ++I) {
      ASTExpressionNode *EN = Params[I].Expr;
      assert(EN && "Invalid classical gate call parameter!");

      if (ASTAngleNode *AN = dynamic_cast<ASTAngleNode *>(EN)) {
        if (AN->IsExpression()) {
          if (const ASTExpressionNode *EXN = AN->GetExpression()) {
            switch (EXN->GetASTType()) {
            case ASTTypeBinaryOp: {
              const ASTBinaryOpNode *BOP =
                  dynamic_cast<const ASTBinaryOpNode *>(EXN);
              assert(BOP && "Invalid dynamic_cast to an ASTBinaryOpNode!");
              M.GateArg(X, ASTStringUtils::Instance().SanitizeMangled(
                               BOP->GetMangledName()));
            } break;
            case ASTTypeUnaryOp: {
              const ASTUnaryOpNode *UOP =
                  dynamic_cast<const ASTUnaryOpNode *>(EXN);
              assert(UOP && "Invalid dynamic_cast to an ASTUnaryOpNode!");
              M.GateArg(X, ASTStringUtils::Instance().SanitizeMangled(
                               UOP->GetMangledName()));
            } break;
            default:
              M.GateArg(X, ASTStringUtils::Instance().SanitizeMangled(
                               AN->GetMangledName()));
              break;
            }
          } else {
            M.GateArg(X, ASTStringUtils::Instance().SanitizeMangled(
                             AN->GetMangledName()));
          }
        } else {
          M.GateArg(X, ASTStringUtils::Instance().SanitizeMangled(
                           AN->GetMangledName()));
        }
      } else {
        if (EN->GetMangledName().empty())
          EN->Mangle();
        M.GateArg(X, ASTStringUtils::Instance().SanitizeMangled(
                         EN->GetMangledName()));
      }
      ++X;
    }

    if (!OperandParams.empty()) {
      for (unsigned I = 0; I < OperandParams.size(); ++I) {
        M.GateArg(X + I,
                  ASTStringUtils::Instance().SanitizeMangled(
                      OperandParams[I]->GetIdentifier()->GetMangledName()));
      }
    }
  } else {
    // Definition: encode classical formals then quantum formals.
    unsigned X = 0;

    auto EmitSanitizedParam = [&M, &X](ASTExpressionNode *Node) {
      assert(Node && "Invalid gate formal for mangling!");
      if (Node->GetMangledName().empty())
        Node->Mangle();
      M.GateParam(X, ASTStringUtils::Instance().SanitizeMangled(
                         Node->GetMangledName()));
      ++X;
    };

    for (std::size_t I = 0; I < Params.size(); ++I)
      EmitSanitizedParam(Params[I].Expr);

    for (unsigned I = 0; I < Operands.size(); ++I) {
      const ASTIdentifierNode *QId = nullptr;
      if (I < OperandParams.size() && OperandParams[I])
        QId = OperandParams[I]->GetIdentifier();
      const ASTType QTy = ResolveOperandQuantumType(QId, I);

      M.GateParam(X + I, QTy, 1U,
                  Operands[I]->GetIdentifier()->GetGateParamName());
    }
  }

  if (GateCall)
    M.CallEnd();

  M.End();

  const_cast<ASTIdentifierNode *>(GetIdentifier())
      ->SetMangledName(M.AsString(), true);
}

ASTGateNode *ASTGateControlNode::Resolve() {
  // FIXME: IMPLEMENT.
  return GN->GetControlType() == ASTTypeUndefined
             ? const_cast<ASTGateNode *>(GN)
             : nullptr;
}

ASTGateNode *ASTGateNegControlNode::Resolve() {
  // FIXME: IMPLEMENT.
  return GN->GetControlType() == ASTTypeUndefined
             ? const_cast<ASTGateNode *>(GN)
             : nullptr;
}

ASTGateNode *ASTGateInverseNode::Resolve() {
  // FIXME: IMPLEMENT.
  return GN->GetControlType() == ASTTypeUndefined
             ? const_cast<ASTGateNode *>(GN)
             : nullptr;
}

std::vector<ASTGateNode *> ASTGatePowerNode::Resolve() {
  std::vector<ASTGateNode *> V;

  // FIXME: IMPLEMENT
  if (GN->GetControlType() == ASTTypeUndefined)
    V.push_back(const_cast<ASTGateNode *>(GN));

  return V;
}

bool ASTGateQOpList::TransferToSymbolTable(
    std::map<std::string, const ASTSymbolTableEntry *> &MM) const {
  if (List.empty())
    return true;

  for (std::vector<ASTGateQOpNode *>::const_iterator I = List.begin();
       I != List.end(); ++I) {
    const ASTIdentifierNode *Id = (*I)->GetIdentifier();
    assert(Id && "Could not obtain a valid ASTIdentifierNode!");

    if (ASTTypeSystemBuilder::Instance().IsBuiltinGate(Id->GetName()))
      continue;

    const ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().FindLocal(Id);
    if (STE) {
      if (!MM.insert(std::make_pair(Id->GetName(), STE)).second) {
        std::stringstream M;
        M << "Failed to insert ASTGateQOp " << Id->GetName() << " into the "
          << "Gate SymbolTable.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
            DiagLevel::ICE);
        return false;
      }

      ASTSymbolTable::Instance().EraseLocal(Id, Id->GetBits(),
                                            Id->GetSymbolType());
    }
  }

  return true;
}

void ASTGateFockControlNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  switch (LevelType) {
  case ASTTypeInt:
    if (I) {
      const std::string &MN = I->GetMangledName();
      M.Identifier(MN.empty() ? I->GetName() : MN);
    }
    break;
  case ASTTypeIdentifier:
    if (ID) {
      const std::string &MN = ID->GetMangledName();
      M.Identifier(MN.empty() ? ID->GetName() : MN);
    }
    break;
  case ASTTypeBinaryOp:
    if (BOP) {
      const std::string &MN = BOP->GetMangledName();
      M.Identifier(MN.empty() ? BOP->GetName() : MN);
    }
    break;
  case ASTTypeUnaryOp:
    if (UOP) {
      const std::string &MN = UOP->GetMangledName();
      M.Identifier(MN.empty() ? UOP->GetName() : MN);
    }
    break;
  default:
    break;
  }
  switch (TType) {
  case ASTTypeGate:
    if (GN)
      M.Identifier(GN->GetMangledName());
    break;
  case ASTTypeGateQOpNode:
    if (GQN)
      M.Identifier(GQN->GetMangledName());
    break;
  case ASTTypeGateGPhaseExpression:
    if (GGEN)
      M.Identifier(GGEN->GetMangledName());
    break;
  default:
    break;
  }
  M.End();
  const_cast<ASTIdentifierNode *>(GetIdentifier())
      ->SetMangledName(M.AsString());
}

void ASTGateFockNegControlNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  switch (LevelType) {
  case ASTTypeInt:
    if (I) {
      const std::string &MN = I->GetMangledName();
      M.Identifier(MN.empty() ? I->GetName() : MN);
    }
    break;
  case ASTTypeIdentifier:
    if (ID) {
      const std::string &MN = ID->GetMangledName();
      M.Identifier(MN.empty() ? ID->GetName() : MN);
    }
    break;
  case ASTTypeBinaryOp:
    if (BOP) {
      const std::string &MN = BOP->GetMangledName();
      M.Identifier(MN.empty() ? BOP->GetName() : MN);
    }
    break;
  case ASTTypeUnaryOp:
    if (UOP) {
      const std::string &MN = UOP->GetMangledName();
      M.Identifier(MN.empty() ? UOP->GetName() : MN);
    }
    break;
  default:
    break;
  }
  switch (TType) {
  case ASTTypeGate:
    if (GN)
      M.Identifier(GN->GetMangledName());
    break;
  case ASTTypeGateQOpNode:
    if (GQN)
      M.Identifier(GQN->GetMangledName());
    break;
  case ASTTypeGateGPhaseExpression:
    if (GGEN)
      M.Identifier(GGEN->GetMangledName());
    break;
  default:
    break;
  }
  M.End();
  const_cast<ASTIdentifierNode *>(GetIdentifier())
      ->SetMangledName(M.AsString());
}

void ASTGateFockControlNode::print() const {
  std::cout << "<GateFockControlNode>" << std::endl;
  std::cout << "<FockLevel>" << std::endl;
  switch (LevelType) {
  case ASTTypeInt:
    if (I)
      I->print();
    break;
  case ASTTypeIdentifier:
    if (ID)
      ID->print();
    break;
  case ASTTypeBinaryOp:
    if (BOP)
      BOP->print();
    break;
  case ASTTypeUnaryOp:
    if (UOP)
      UOP->print();
    break;
  default:
    break;
  }
  std::cout << "</FockLevel>" << std::endl;
  std::cout << "<Target>" << std::endl;
  switch (TType) {
  case ASTTypeGate:
    if (GN)
      GN->print();
    break;
  case ASTTypeGateQOpNode:
    if (GQN)
      GQN->print();
    break;
  case ASTTypeGateGPhaseExpression:
    if (GGEN)
      GGEN->print();
    break;
  default:
    break;
  }
  std::cout << "</Target>" << std::endl;
  std::cout << "</GateFockControlNode>" << std::endl;
}

void ASTGateFockNegControlNode::print() const {
  std::cout << "<GateFockNegControlNode>" << std::endl;
  std::cout << "<FockLevel>" << std::endl;
  switch (LevelType) {
  case ASTTypeInt:
    if (I)
      I->print();
    break;
  case ASTTypeIdentifier:
    if (ID)
      ID->print();
    break;
  case ASTTypeBinaryOp:
    if (BOP)
      BOP->print();
    break;
  case ASTTypeUnaryOp:
    if (UOP)
      UOP->print();
    break;
  default:
    break;
  }
  std::cout << "</FockLevel>" << std::endl;
  std::cout << "<Target>" << std::endl;
  switch (TType) {
  case ASTTypeGate:
    if (GN)
      GN->print();
    break;
  case ASTTypeGateQOpNode:
    if (GQN)
      GQN->print();
    break;
  case ASTTypeGateGPhaseExpression:
    if (GGEN)
      GGEN->print();
    break;
  default:
    break;
  }
  std::cout << "</Target>" << std::endl;
  std::cout << "</GateFockNegControlNode>" << std::endl;
}

} // namespace QASM
