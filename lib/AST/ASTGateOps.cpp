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

#include <qasm/AST/ASTArgument.h>
#include <qasm/AST/ASTGates.h>
#include <qasm/AST/ASTReset.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTMeasure.h>
#include <qasm/AST/ASTBarrier.h>
#include <qasm/AST/ASTGPhase.h>
#include <qasm/AST/ASTDefcal.h>
#include <qasm/AST/ASTDefcalBuilder.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTStringUtils.h>
#include <qasm/AST/ASTMathUtils.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <iomanip>
#include <sstream>
#include <cassert>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTGateOpNode::ASTGateOpNode(const ASTIdentifierNode* Id,
                             const ASTGateNode* GN)
  : ASTStatementNode(Id, GN), IsDefcal(false),
  MTy(ASTTypeUndefined), OTy(GN ? GN->GetASTType() : ASTTypeGate) { }

ASTGateOpNode::ASTGateOpNode(const ASTIdentifierNode* Id,
                             const ASTDefcalNode* DN)
  : ASTStatementNode(Id, DN->AsExpression()), IsDefcal(true),
  MTy(ASTTypeUndefined), OTy(DN ? DN->GetASTType() : ASTTypeDefcal) { }

ASTGateOpNode::ASTGateOpNode(const ASTIdentifierNode* Id,
                             const ASTDefcalGroupNode* DGN)
  : ASTStatementNode(Id, DGN), IsDefcal(true),
  MTy(ASTTypeUndefined), OTy(DGN ? DGN->GetASTType() : ASTTypeDefcalGroup) { }

ASTGateOpNode::ASTGateOpNode(const ASTIdentifierNode* Id,
                             const ASTGPhaseExpressionNode* GPE)
  : ASTStatementNode(Id, GPE), IsDefcal(false),
  MTy(ASTTypeUndefined),
  OTy(GPE ? GPE->GetASTType() : ASTTypeGPhaseExpression) { }

ASTGateOpNode::ASTGateOpNode(const ASTIdentifierNode* Id,
                             const ASTGateControlNode* GCN)
  : ASTStatementNode(Id, GCN), IsDefcal(false),
  MTy(GCN->GetASTType()),
  OTy(GCN ? GCN->GetASTType() : ASTTypeGateControl) { }

ASTGateOpNode::ASTGateOpNode(const ASTIdentifierNode* Id,
                             const ASTGateNegControlNode* GNCN)
  : ASTStatementNode(Id, GNCN), IsDefcal(false),
  MTy(GNCN->GetASTType()),
  OTy(GNCN ? GNCN->GetASTType() : ASTTypeGateNegControl) { }

ASTGateOpNode::ASTGateOpNode(const ASTIdentifierNode* Id,
                             const ASTGatePowerNode* GPN)
  : ASTStatementNode(Id, GPN), IsDefcal(false),
  MTy(GPN->GetASTType()),
  OTy(GPN ? GPN->GetASTType() : ASTTypeGatePower) { }

ASTGateOpNode::ASTGateOpNode(const ASTIdentifierNode* Id,
                             const ASTGateInverseNode* GIN)
  : ASTStatementNode(Id, GIN), IsDefcal(false),
  MTy(GIN->GetASTType()),
  OTy(GIN ? GIN->GetASTType() : ASTTypeGateInverse) { }

void ASTGateOpNode::SetModifier(const ASTGateControlNode* N) {
  if (const ASTGateNode* GN =
      dynamic_cast<const ASTGateNode*>(ASTStatementNode::GetExpression())) {
    GN->SetControl(N);
    MTy = N->GetASTType();
  }
}

void ASTGateOpNode::SetModifier(const ASTGateNegControlNode* N) {
  if (const ASTGateNode* GN =
      dynamic_cast<const ASTGateNode*>(ASTStatementNode::GetExpression())) {
    GN->SetNegControl(N);
    OTy = MTy = N->GetASTType();
  }
}

void ASTGateOpNode::SetModifier(const ASTGatePowerNode* N) {
  if (const ASTGateNode* GN =
      dynamic_cast<const ASTGateNode*>(ASTStatementNode::GetExpression())) {
    GN->SetPower(N);
    OTy = MTy = N->GetASTType();
  }
}

void ASTGateOpNode::SetModifier(const ASTGateInverseNode* N) {
  if (const ASTGateNode* GN =
      dynamic_cast<const ASTGateNode*>(ASTStatementNode::GetExpression())) {
    GN->SetInverse(N);
    OTy = MTy = N->GetASTType();
  }
}

const ASTGateControlNode*
ASTGateOpNode::GetGateControlModifier() const {
  if (const ASTGateNode* GN =
      dynamic_cast<const ASTGateNode*>(ASTStatementNode::GetExpression())) {
    return GN->GetControl();
  }

  return nullptr;
}

const ASTGateNegControlNode*
ASTGateOpNode::GetGateNegControlModifier() const {
  if (const ASTGateNode* GN =
      dynamic_cast<const ASTGateNode*>(ASTStatementNode::GetExpression())) {
    return GN->GetNegControl();
  }

  return nullptr;
}

const ASTGatePowerNode*
ASTGateOpNode::GetPowerModifier() const {
  if (const ASTGateNode* GN =
      dynamic_cast<const ASTGateNode*>(ASTStatementNode::GetExpression())) {
    return GN->GetPower();
  }

  return nullptr;
}

const ASTGateInverseNode*
ASTGateOpNode::GetInverseModifier() const {
  if (const ASTGateNode* GN =
      dynamic_cast<const ASTGateNode*>(ASTStatementNode::GetExpression())) {
    return GN->GetInverse();
  }

  return nullptr;
}

bool ASTGateOpNode::HasModifier() const {
  if (const ASTGateNode* GN =
      dynamic_cast<const ASTGateNode*>(ASTStatementNode::GetExpression())) {
    return (GN->HasControl() || GN->HasNegControl() ||
            GN->HasInverse() || GN->HasPower()) &&
      MTy != ASTTypeUndefined;
  }

  return false;
}

ASTGateOpNode::ASTGateOpNode(const ASTIdentifierNode* Id,
                             const ASTGateGPhaseExpressionNode* GGEN)
  : ASTStatementNode(Id, GGEN), IsDefcal(false),
  MTy(ASTTypeUndefined) { }

ASTGateQOpNode::ASTGateQOpNode(const ASTGPhaseExpressionNode* GPE)
  : ASTGateOpNode(GPE->GetIdentifier(), GPE) { }

ASTGateQOpNode::ASTGateQOpNode(const ASTGateControlNode* GCN)
  : ASTGateOpNode(GCN->GetIdentifier(), GCN) { }

ASTGateQOpNode::ASTGateQOpNode(const ASTGateNegControlNode* GNCN)
  : ASTGateOpNode(GNCN->GetIdentifier(), GNCN) { }

ASTGateQOpNode::ASTGateQOpNode(const ASTGatePowerNode* GPN)
  : ASTGateOpNode(GPN->GetIdentifier(), GPN) { }

ASTGateQOpNode::ASTGateQOpNode(const ASTGateInverseNode* GIN)
  : ASTGateOpNode(GIN->GetIdentifier(), GIN) { }

ASTGateQOpNode::ASTGateQOpNode(const ASTGateGPhaseExpressionNode* GGEN)
  : ASTGateOpNode(GGEN->GetIdentifier(), GGEN) { }

const ASTGateNode*
ASTGateQOpNode::GetGateNode() const {
  return dynamic_cast<const ASTGateNode*>(ASTStatementNode::Expr);
}

ASTGateNode*
ASTGateQOpNode::GetGateNode() {
  return const_cast<ASTGateNode*>(
         dynamic_cast<const ASTGateNode*>(ASTStatementNode::Expr));
}

const ASTDefcalNode*
ASTGateQOpNode::GetDefcalNode() const {
  if (ASTStatementNode::Expr->IsStatement())
    return dynamic_cast<const ASTDefcalNode*>(
                        ASTStatementNode::Expr->GetStatement());
  return nullptr;
}

ASTDefcalNode*
ASTGateQOpNode::GetDefcalNode() {
  if (ASTStatementNode::Expr->IsStatement())
    return dynamic_cast<ASTDefcalNode*>(
      const_cast<ASTStatementNode*>(ASTStatementNode::Expr->GetStatement()));

  return nullptr;
}

const ASTDefcalGroupNode*
ASTGateQOpNode::GetDefcalGroupNode() const {
  if (GetExpression() &&
      GetExpression()->GetASTType() == ASTTypeDefcalGroup)
    return dynamic_cast<const ASTDefcalGroupNode*>(GetExpression());

  return nullptr;
}

ASTDefcalGroupNode*
ASTGateQOpNode::GetDefcalGroupNode() {
  if (GetExpression() &&
      GetExpression()->GetASTType() == ASTTypeDefcalGroup)
    return dynamic_cast<ASTDefcalGroupNode*>(GetExpression());

  return nullptr;
}

const ASTGPhaseExpressionNode*
ASTGateQOpNode::GetGPhaseNode() const {
  return dynamic_cast<const ASTGPhaseExpressionNode*>(ASTStatementNode::Expr);
}

ASTGPhaseExpressionNode*
ASTGateQOpNode::GetGPhaseNode() {
  return const_cast<ASTGPhaseExpressionNode*>(
         dynamic_cast<const ASTGPhaseExpressionNode*>(ASTStatementNode::Expr));
}

const ASTGateControlNode*
ASTGateQOpNode::GetGateControlNode() const {
  return dynamic_cast<const ASTGateControlNode*>(ASTStatementNode::Expr);
}

ASTGateControlNode*
ASTGateQOpNode::GetGateControlNode() {
  return const_cast<ASTGateControlNode*>(
         dynamic_cast<const ASTGateControlNode*>(ASTStatementNode::Expr));
}

const ASTGateNegControlNode*
ASTGateQOpNode::GetGateNegControlNode() const {
  return dynamic_cast<const ASTGateNegControlNode*>(ASTStatementNode::Expr);
}

ASTGateNegControlNode*
ASTGateQOpNode::GetGateNegControlNode() {
  return const_cast<ASTGateNegControlNode*>(
         dynamic_cast<const ASTGateNegControlNode*>(ASTStatementNode::Expr));
}

const ASTGatePowerNode*
ASTGateQOpNode::GetGatePowerNode() const {
  return dynamic_cast<const ASTGatePowerNode*>(ASTStatementNode::Expr);
}

ASTGatePowerNode*
ASTGateQOpNode::GetGatePowerNode() {
  return const_cast<ASTGatePowerNode*>(
         dynamic_cast<const ASTGatePowerNode*>(ASTStatementNode::Expr));
}

const ASTGateInverseNode*
ASTGateQOpNode::GetGateInverseNode() const {
  return dynamic_cast<const ASTGateInverseNode*>(ASTStatementNode::Expr);
}

ASTGateInverseNode*
ASTGateQOpNode::GetGateInverseNode() {
  return const_cast<ASTGateInverseNode*>(
         dynamic_cast<const ASTGateInverseNode*>(ASTStatementNode::Expr));
}

const ASTGateGPhaseExpressionNode*
ASTGateQOpNode::GetGateGPhaseNode() const {
  return dynamic_cast<const ASTGateGPhaseExpressionNode*>(ASTStatementNode::Expr);
}

ASTGateGPhaseExpressionNode*
ASTGateQOpNode::GetGateGPhaseNode() {
  return const_cast<ASTGateGPhaseExpressionNode*>(
         dynamic_cast<const ASTGateGPhaseExpressionNode*>(ASTStatementNode::Expr));
}

void ASTGateOpNode::print() const {
  std::cout << "<GateOpNode>" << std::endl;

  if (ASTStatementNode::IsExpression()) {
    ASTStatementNode::GetExpression()->print();
  }

  std::cout << "<IsDefcalCall>" << std::boolalpha
    << IsDefcal << "</IsDefcalCall>" << std::endl;

  if (MTy != ASTTypeUndefined) {
    std::cout << "<ModifierType>" << PrintTypeEnum(MTy)
      << "</ModifierType>" << std::endl;
  }

  if (OTy != ASTTypeUndefined) {
    std::cout << "<OperationType>" << PrintTypeEnum(OTy)
      << "</OperationType>" << std::endl;
  }

  std::cout << "</GateOpNode>" << std::endl;
}

void ASTGateQOpNode::print() const {
  std::cout << "<GateQOpNode>" << std::endl;
  ASTGateOpNode::print();
  std::cout << "</GateQOpNode>" << std::endl;
}

void ASTGenericGateOpNode::print() const {
  std::cout << "<GenericGateOpNode>" << std::endl;
  ASTGateOpNode::print();
  std::cout << "</GenericGateOpNode>" << std::endl;
}

void ASTHGateOpNode::print() const {
  std::cout << "<HGateOpNode>" << std::endl;
  ASTGateQOpNode::print();
  std::cout << "</HGateOpNode>" << std::endl;
}

void ASTUGateOpNode::print() const {
  std::cout << "<UGateOpNode>" << std::endl;
  ASTGateQOpNode::print();
  std::cout << "</UGateOpNode>" << std::endl;
}

void ASTCXGateOpNode::print() const {
  std::cout << "<CXGateOpNode>" << std::endl;
  ASTGateQOpNode::print();
  std::cout << "</CXGateOpNode>" << std::endl;
}

void ASTCCXGateOpNode::print() const {
  std::cout << "<CCXGateOpNode>" << std::endl;
  ASTGateQOpNode::print();
  std::cout << "</CCXGateOpNode>" << std::endl;
}

void ASTCNotGateOpNode::print() const {
  std::cout << "<CNotGateOpNode>" << std::endl;
  ASTGateQOpNode::print();
  std::cout << "</CNotGateOpNode>" << std::endl;
}

void ASTMeasureNode::print() const {
  std::cout << "<MeasureNode>" << std::endl;
  std::cout << "<Identifier>" << GetName() << "</Identifier>" << std::endl;
  std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
    << std::endl;
  ASTGateQOpNode::print();
  std::cout << "<Target>" << std::endl;

  if (TIV.size()) {
    for (unsigned I = 0; I < TIV.size(); ++I)
      std::cout << "<TargetIndex>" << TIV[I] << "</TargetIndex>"
        << std::endl;
  }

  Target->print();
  std::cout << "</Target>" << std::endl;

  if (RTy == ASTTypeBitset) {
    std::cout << "<Result>" << std::endl;

    if (RIV.size()) {
      for (unsigned I = 0; I < RIV.size(); ++I)
        std::cout << "<ResultIndex>" << RIV[I] << "</ResultIndex>"
          << std::endl;
    }

    Result->print();
    std::cout << "</Result>" << std::endl;
  } else if (RTy == ASTTypeAngle) {
    std::cout << "<Angle>" << std::endl;
    Angle->print();
    std::cout << "</Angle>" << std::endl;
  } else if (RTy == ASTTypeMPComplex) {
    std::cout << "<Complex>" << std::endl;
    Complex->print();
    std::cout << "</Complex>" << std::endl;
  }

  std::cout << "</MeasureNode>" << std::endl;
}

void ASTResetNode::print() const {
  std::cout << "<ResetNode>" << std::endl;
  ASTGateQOpNode::print();
  std::cout << "<MangledName>" << GetIdentifier()->GetMangledName()
    << "</MangledName>" << std::endl;
  std::cout << "<TargetName>" << Target->GetName() << "</TargetName>"
    << std::endl;
  std::cout << "<TargetMangledName>" << Target->GetMangledName()
    << "</TargetMangledName>" << std::endl;
  std::cout << "</ResetNode>" << std::endl;
}

void ASTBarrierNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTMeasureNode::Mangle() {
  Target->Mangle();

  switch (RTy) {
  case ASTTypeBitset:
    Result->Mangle();
    break;
  case ASTTypeAngle:
    Angle->Mangle();
    break;
  case ASTTypeMPComplex:
    Complex->Mangle();
    break;
  default:
    break;
  }

  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetIdentifier()->GetGateParamName());
  M.TypeIdentifier(GetTarget()->GetASTType(), GetTarget()->Size(),
                   GetTarget()->GetName());
  M.TypeValue(ASTTypeHash,
              ASTMathUtils::vector_hash<unsigned>()(GetTargetVector()));
  if (unsigned Z = GetTargetVectorSize()) {
    for (unsigned I = 0; I < Z; ++I)
      M.TypeValue(ASTTypeUInt, TIV[I]);
  } else {
    M.TypeValue(ASTTypeUInt, static_cast<unsigned>(~0x0));
  }

  M.EndExpression();

  M.TypeValue(ASTTypeHash,
              ASTMathUtils::vector_hash<unsigned>()(GetResultVector()));
  if (unsigned Z = GetResultVectorSize()) {
    for (unsigned I = 0; I < Z; ++I)
      M.TypeValue(ASTTypeUInt, RIV[I]);
  } else {
    M.TypeValue(ASTTypeUInt, static_cast<unsigned>(~0x0));
  }

  M.EndExpression();

  if (RTy == ASTTypeAngle && HasAngleResult())
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                  GetAngleResult()->GetMangledName()));
  else if (RTy == ASTTypeBitset && HasResult())
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                  GetResult()->GetMangledName()));
  else if (RTy == ASTTypeMPComplex && HasComplexResult())
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(
                                  GetComplexResult()->GetMangledName()));

  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTGateQOpNode::Mangle(ASTGateQOpNode* G) {
  assert(G && "Invalid ASTGateQOpNode argument!");

  ASTMangler M;
  M.Start();
  M.CallStart();
  if (IsDefcalOp())
    M.Type(ASTTypeDefcalCall);
  else
    M.Type(ASTTypeGateCall);

  M.TypeIdentifier(G->GetASTType(), G->GetName());

  if (IsDefcalOp()) {
    if (const ASTDefcalNode* DN = G->GetDefcalNode()) {
      M.SetCall(DN->GetMangledName());
      M.Underscore();
    } else if (const ASTDefcalGroupNode* DGN = G->GetDefcalGroupNode()) {
      M.SetCall(DGN->GetMangledName());
      M.Underscore();
    } else {
      std::stringstream MM;
      MM << "Invalid ASTDefcalNode obtained for defcal call.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::ICE);
    }
  } else {
    if (const ASTGateNode* GN = G->GetGateNode()) {
      if (G->HasModifier()) {
        switch (G->GetModifierType()) {
        case ASTTypeGateControl: {
          if (const ASTGateControlNode* GCN = G->GetGateControlNode()) {
            M.SetModifierCall(GCN->GetMangledName(), GN->GetMangledName());
          }
        }
          break;
        case ASTTypeGateNegControl: {
          if (const ASTGateNegControlNode* GCN = G->GetGateNegControlNode()) {
            M.SetModifierCall(GCN->GetMangledName(), GN->GetMangledName());
          }
        }
          break;
        case ASTTypeGateInverse: {
          if (const ASTGateInverseNode* GIN = G->GetGateInverseNode()) {
            M.SetModifierCall(GIN->GetMangledName(), GN->GetMangledName());
          }
        }
          break;
        case ASTTypeGatePower: {
          if (const ASTGatePowerNode* GPN = G->GetGatePowerNode()) {
            M.SetModifierCall(GPN->GetMangledName(), GN->GetMangledName());
          }
        }
          break;
        default: {
          std::stringstream MM;
          MM << "Invalid modifier type " << PrintTypeEnum(G->GetModifierType())
            << " for control modifier gate call.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::ICE);
        }
          break;
        }
      } else {
        M.SetCall(GN->GetMangledName());
        M.Underscore();
      }
    } else if (const ASTGPhaseExpressionNode* GPN = G->GetGPhaseNode()) {
      const_cast<ASTGPhaseExpressionNode*>(GPN)->Mangle();
      M.SetCall(GPN->GetMangledName());
      M.Underscore();
    } else if (const ASTGateGPhaseExpressionNode* GGPN = G->GetGateGPhaseNode()) {
      const_cast<ASTGateGPhaseExpressionNode*>(GGPN)->Mangle();
      M.SetCall(GGPN->GetMangledName());
      M.Underscore();
    } else if (const ASTGatePowerNode* GPON = G->GetGatePowerNode()) {
      const_cast<ASTGatePowerNode*>(GPON)->Mangle();
      M.SetCall(GPON->GetMangledName());
      M.Underscore();
    } else if (const ASTGateInverseNode* GIN = G->GetGateInverseNode()) {
      const_cast<ASTGateInverseNode*>(GIN)->Mangle();
      M.SetCall(GIN->GetMangledName());
      M.Underscore();
    } else if (const ASTGateControlNode* GCN = G->GetGateControlNode()) {
      const_cast<ASTGateControlNode*>(GCN)->Mangle();
      M.SetCall(GCN->GetMangledName());
      M.Underscore();
    } else if (const ASTGateNegControlNode* GNCN = G->GetGateNegControlNode()) {
      const_cast<ASTGateNegControlNode*>(GNCN)->Mangle();
      M.SetCall(GNCN->GetMangledName());
      M.Underscore();
    } else {
      std::stringstream MM;
      MM << "Invalid callable type " << PrintTypeEnum(G->GetASTType())
        << " obtained for gate call.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::ICE);
    }
  }

  M.CallEnd();
  G->GetIdentifier()->SetMangledName(M.AsString());
}

void ASTGateQOpNode::Mangle() {
  ASTGateQOpNode::Mangle(this);
}

void ASTUGateOpNode::Mangle() {
  ASTGateQOpNode::Mangle(this);
}

void ASTGenericGateOpNode::Mangle() {
  ASTGateQOpNode::Mangle(this);
}

void ASTHGateOpNode::Mangle() {
  ASTGateQOpNode::Mangle(this);
}

void ASTCXGateOpNode::Mangle() {
  ASTGateQOpNode::Mangle(this);
}

void ASTCCXGateOpNode::Mangle() {
  ASTGateQOpNode::Mangle(this);
}

void ASTCNotGateOpNode::Mangle() {
  ASTGateQOpNode::Mangle(this);
}

void ASTGateOpNode::Mangle() {
  switch (GetASTType()) {
  case ASTTypeGateQOpNode: {
    if (ASTGateQOpNode* GQO = dynamic_cast<ASTGateQOpNode*>(this)) {
      GQO->Mangle();
    }
  }
    break;
  case ASTTypeGateUOpNode: {
    if (ASTUGateOpNode* UQO = dynamic_cast<ASTUGateOpNode*>(this)) {
      UQO->Mangle();
    }
  }
    break;
  case ASTTypeGateGenericOpNode: {
    if (ASTGenericGateOpNode* GGO = dynamic_cast<ASTGenericGateOpNode*>(this)) {
      GGO->Mangle();
    }
  }
    break;
  case ASTTypeGateHOpNode: {
    if (ASTHGateOpNode* GHO = dynamic_cast<ASTHGateOpNode*>(this)) {
      GHO->Mangle();
    }
  }
    break;
  case ASTTypeCXGateOpNode: {
    if (ASTCXGateOpNode* GCXO = dynamic_cast<ASTCXGateOpNode*>(this)) {
      GCXO->Mangle();
    }
  }
    break;
  case ASTTypeCCXGateOpNode: {
    if (ASTCCXGateOpNode* GCCXO = dynamic_cast<ASTCCXGateOpNode*>(this)) {
      GCCXO->Mangle();
    }
  }
    break;
  case ASTTypeCNotGateOpNode: {
    if (ASTCNotGateOpNode* GCNO = dynamic_cast<ASTCNotGateOpNode*>(this)) {
      GCNO->Mangle();
    }
  }
    break;
  default: {
    std::stringstream M;
    M << "Impossible ASTGateOpNode dynamic_cast to "
      << PrintTypeEnum(GetASTType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
  }
    break;
  }
}

void ASTGPhaseExpressionNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (!GetIdentifier()->GetPolymorphicName().empty())
    M.TypeIdentifier(GetASTType(), GetIdentifier()->GetPolymorphicName());
  else
    M.TypeIdentifier(GetASTType(), GetName());
  M.Underscore();

  switch (GetTargetType()) {
  case ASTTypeAngle:
    M.TypeIdentifier(Id->GetSymbolType(), Id->GetBits(), Id->GetName());
                     // ASTStringUtils::Instance().SanitizeMangled(Id->GetMangledName()));
    break;
  case ASTTypeBinaryOp:
    M.TypeIdentifier(BOP->GetASTType(),
                     ASTStringUtils::Instance().SanitizeMangled(BOP->GetMangledName()));
    break;
  case ASTTypeUnaryOp:
    M.TypeIdentifier(UOP->GetASTType(),
                     ASTStringUtils::Instance().SanitizeMangled(UOP->GetMangledName()));
    break;
  default:
    M.TypeIdentifier(Id->GetSymbolType(),
                     ASTStringUtils::Instance().SanitizeMangled(Id->GetMangledName()));
    break;
  }

  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTGateGPhaseExpressionNode::Mangle() {
  if (!IL.Empty()) {
    for (ASTIdentifierList::iterator I = IL.begin(); I != IL.end(); ++I) {
      ASTIdentifierNode* QId = *I;
      assert(QId && "Invalid ASTIdentifierNode in ASTIdentifierList!");

      ASTSymbolTableEntry* QSTE = QId->GetSymbolTableEntry();
      if (!QSTE) {
        QSTE = ASTSymbolTable::Instance().Lookup(QId, QId->GetBits(),
                                                 QId->GetSymbolType());
      }
      assert(QSTE && "Could not obtain a valid SymbolTable Entry!");

      ASTGateQubitParamNode* GQP = QSTE->GetValue()->GetValue<ASTGateQubitParamNode*>();
      assert(GQP && "Could not obtain a valid ASTGateQubitParamNode!");

      GQP->Mangle();
    }
  }

  ASTMangler M;
  M.Start();
  if (!GetIdentifier()->GetPolymorphicName().empty())
    M.TypeIdentifier(GetASTType(), GetIdentifier()->GetPolymorphicName());
  else
    M.TypeIdentifier(GetASTType(), GetName());
  M.Underscore();

  switch (GetTargetType()) {
  case ASTTypeGPhaseExpression:
    M.TypeIdentifier(GPE->GetASTType(), GPE->GetMangledName());
    break;
  case ASTTypeGateControl:
    M.TypeIdentifier(CN->GetASTType(), CN->GetMangledName());
    break;
  case ASTTypeGateNegControl:
    M.TypeIdentifier(NCN->GetASTType(), NCN->GetMangledName());
    break;
  case ASTTypeGateInverse:
    M.TypeIdentifier(IN->GetASTType(), IN->GetMangledName());
    break;
  case ASTTypeGatePower:
    M.TypeIdentifier(PN->GetASTType(), PN->GetMangledName());
    break;
  case ASTTypeGateQOpNode:
  case ASTTypeGateUOpNode:
  case ASTTypeGateGenericOpNode:
  case ASTTypeGateHOpNode:
  case ASTTypeCXGateOpNode:
  case ASTTypeCCXGateOpNode:
  case ASTTypeCNotGateOpNode:
    M.TypeIdentifier(GQN->GetASTType(), GQN->GetMangledName());
    break;
  default: {
    std::stringstream MM;
    MM << "Impossible ASTGateGPhaseExpressionNode from "
      << PrintTypeEnum(GetTargetType()) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), MM.str(), DiagLevel::ICE);
  }
    break;
  }

  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace QASM

