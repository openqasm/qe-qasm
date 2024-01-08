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

#include <qasm/AST/ASTArray.h>
#include <qasm/AST/ASTBarrier.h>
#include <qasm/AST/ASTBox.h>
#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTDefcal.h>
#include <qasm/AST/ASTDelay.h>
#include <qasm/AST/ASTDuration.h>
#include <qasm/AST/ASTFunction.h>
#include <qasm/AST/ASTGPhase.h>
#include <qasm/AST/ASTGateControl.h>
#include <qasm/AST/ASTGates.h>
#include <qasm/AST/ASTKernel.h>
#include <qasm/AST/ASTLength.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTMeasure.h>
#include <qasm/AST/ASTPragma.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTResult.h>
#include <qasm/AST/ASTStretch.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseCalibration.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseFrame.h>
#include <qasm/AST/OpenPulse/ASTOpenPulsePlay.h>
#include <qasm/AST/OpenPulse/ASTOpenPulsePort.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseWaveform.h>

#include <qasm/AST/ASTAngleNodeBuilder.h>
#include <qasm/AST/ASTAnyTypeBuilder.h>
#include <qasm/AST/ASTArgumentNodeBuilder.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTBuiltinFunctionsBuilder.h>
#include <qasm/AST/ASTDefcalGrammarBuilder.h>
#include <qasm/AST/ASTDoWhileStatementBuilder.h>
#include <qasm/AST/ASTExpressionBuilder.h>
#include <qasm/AST/ASTExpressionEvaluator.h>
#include <qasm/AST/ASTExpressionNodeBuilder.h>
#include <qasm/AST/ASTForStatementBuilder.h>
#include <qasm/AST/ASTGateOpBuilder.h>
#include <qasm/AST/ASTGateQubitParamBuilder.h>
#include <qasm/AST/ASTGateQubitTracker.h>
#include <qasm/AST/ASTIdentifierBuilder.h>
#include <qasm/AST/ASTInitializerListBuilder.h>
#include <qasm/AST/ASTIntegerListBuilder.h>
#include <qasm/AST/ASTOpenQASMVersionTracker.h>
#include <qasm/AST/ASTParameterBuilder.h>
#include <qasm/AST/ASTQubitConcatBuilder.h>
#include <qasm/AST/ASTStringList.h>
#include <qasm/AST/ASTSwitchStatementBuilder.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTTypeSystemBuilder.h>
#include <qasm/AST/ASTWhileStatementBuilder.h>

#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <map>
#include <set>
#include <string>

namespace QASM {

std::map<std::string, ASTSymbolScope> ASTTypeSystemBuilder::LM;
std::map<std::string, ASTSymbolScope> ASTTypeSystemBuilder::GM;
std::map<std::string, ASTSymbolScope> ASTTypeSystemBuilder::FM;
std::map<ASTType, unsigned> ASTTypeSystemBuilder::TBM;
std::set<std::string> ASTTypeSystemBuilder::BFM;
std::set<std::string> ASTTypeSystemBuilder::BGM;
std::set<std::string> ASTTypeSystemBuilder::OQ2RG;
std::set<std::string> ASTTypeSystemBuilder::RS;
std::set<std::string> ASTTypeSystemBuilder::FR;

ASTTypeSystemBuilder ASTTypeSystemBuilder::TSB;

void ASTTypeSystemBuilder::Init() {
  ASTStringListBuilder::Instance().Init();
  ASTExpressionNodeBuilder::Instance().Init();
  ASTInitializerListBuilder::Instance().Init();
  ASTDeclarationContextTracker::Instance().Init();
  ASTExpressionEvaluator::Instance().Init();
  ASTMangler::Init();
  ASTDemangler::Init();
  ASTIdentifierBuilder::Instance().Init();
  ASTArgumentNodeBuilder::Instance().Init();
  ASTAnyTypeBuilder::Instance().Init();
  ASTIntegerListBuilder::Instance().Init();
  ASTForStatementBuilder::Instance().Init();
  ASTSwitchStatementBuilder::Instance().Init();
  ASTSwitchScopedStatementBuilder::Instance().Init();
  ASTSwitchUnscopedStatementBuilder::Instance().Init();
  ASTDoWhileStatementBuilder::Instance().Init();
  ASTWhileStatementBuilder::Instance().Init();
  ASTGateQubitParamBuilder::Instance().Init();
  ASTParameterBuilder::Instance().Init();
  ASTGateOpBuilder::Instance().Init();
  ASTQubitConcatListBuilder::Instance().Init();
  ASTExpressionBuilder::Instance().Init();

  unsigned Bits = ASTAngleNode::AngleBits;

  if (LM.empty()) {
    LM = {
        {u8"pi", ASTSymbolScope::Global},
        {u8"tau", ASTSymbolScope::Global},
        {u8"euler", ASTSymbolScope::Global},
    };
  }

  if (GM.empty()) {
    GM = {
        {std::string(u8"π"), ASTSymbolScope::Global},
        {std::string(u8"τ"), ASTSymbolScope::Global},
        {std::string(u8"ε"), ASTSymbolScope::Global},
    };
  }

  if (FM.empty()) {
    FM = {
        {std::string(u8"mix"), ASTSymbolScope::Global},
        {std::string(u8"sum"), ASTSymbolScope::Global},
        {std::string(u8"phase_shift"), ASTSymbolScope::Global},
        {std::string(u8"scale"), ASTSymbolScope::Global},
        {std::string(u8"set_phase"), ASTSymbolScope::Global},
        {std::string(u8"get_phase"), ASTSymbolScope::Global},
        {std::string(u8"shift_phase"), ASTSymbolScope::Global},
        {std::string(u8"set_frequency"), ASTSymbolScope::Global},
        {std::string(u8"get_frequency"), ASTSymbolScope::Global},
        {std::string(u8"shift_frequency"), ASTSymbolScope::Global},
    };
  }

  if (TBM.empty()) {
    TBM = {
        {
            ASTTypeAngle,
            ASTAngleNode::AngleBits,
        },
        {
            ASTTypeArray,
            ASTArrayNode::ArrayBits,
        },
        {
            ASTTypeBarrier,
            ASTBarrierNode::BarrierBits,
        },
        {
            ASTTypeBinaryOp,
            ASTBinaryOpNode::BinaryOpBits,
        },
        {
            ASTTypeBool,
            ASTBoolNode::BoolBits,
        },
        {
            ASTTypeBoxAs,
            ASTBoxAsExpressionNode::BoxAsBits,
        },
        {
            ASTTypeBox,
            ASTBoxExpressionNode::BoxBits,
        },
        {
            ASTTypeBoxTo,
            ASTBoxToExpressionNode::BoxToBits,
        },
        {
            ASTTypeBitset,
            ASTCBitNode::CBitBits,
        },
        {
            ASTTypeChar,
            ASTCharNode::CharBits,
        },
        {
            ASTTypeUTF8,
            ASTCharNode::CharBits,
        },
        {
            ASTTypeMPComplex,
            ASTMPComplexNode::DefaultBits,
        },
        {
            ASTTypeMPDecimal,
            ASTMPDecimalNode::DefaultBits,
        },
        {
            ASTTypeDefcal,
            ASTDefcalNode::DefcalBits,
        },
        {
            ASTTypeDefcalMeasure,
            ASTDefcalNode::DefcalBits,
        },
        {
            ASTTypeDefcalReset,
            ASTDefcalNode::DefcalBits,
        },
        {
            ASTTypeDefcalGrammar,
            ASTDefcalGrammarNode::GrammarBits,
        },
        {
            ASTTypeDelay,
            ASTDelayNode::DelayBits,
        },
        {
            ASTTypeDouble,
            ASTDoubleNode::DoubleBits,
        },
        {
            ASTTypeDuration,
            ASTDurationNode::DurationBits,
        },
        {
            ASTTypeDurationOf,
            ASTDurationOfNode::DurationOfBits,
        },
        {
            ASTTypeEllipsis,
            ASTEllipsisNode::EllipsisBits,
        },
        {
            ASTTypeFloat,
            ASTFloatNode::FloatBits,
        },
        {
            ASTTypeFunction,
            ASTFunctionDefinitionNode::FunctionBits,
        },
        {
            ASTTypeFunctionDeclaration,
            ASTFunctionDeclarationNode::FunctionDeclBits,
        },
        {
            ASTTypeFunctionCallExpression,
            ASTFunctionCallNode::FunctionCallBits,
        },
        {
            ASTTypeGate,
            ASTGateNode::GateBits,
        },
        {
            ASTTypeGPhaseExpression,
            ASTGPhaseExpressionNode::GPhaseBits,
        },
        {
            ASTTypeGateGPhaseExpression,
            ASTGateGPhaseExpressionNode::GateGPhaseBits,
        },
        {
            ASTTypeGPhaseStatement,
            ASTGPhaseStatementNode::GPhaseStmtBits,
        },
        {ASTTypeGateQubitParam, 1U},
        {
            ASTTypeIdentifier,
            ASTIdentifierNode::IdentifierBits,
        },
        {
            ASTTypeIdentifierRef,
            ASTIdentifierNode::IdentifierBits,
        },
        {
            ASTTypeInt,
            ASTIntNode::IntBits,
        },
        {
            ASTTypeUInt,
            ASTIntNode::IntBits,
        },
        {
            ASTTypeLong,
            64U,
        },
        {ASTTypeULong, 64U},
        {ASTTypeHash, 64U},
        {
            ASTTypeMPInteger,
            ASTMPIntegerNode::DefaultBits,
        },
        {
            ASTTypeMPUInteger,
            ASTMPIntegerNode::DefaultBits,
        },
        {
            ASTTypeKernel,
            ASTKernelNode::KernelBits,
        },
        {
            ASTTypeLength,
            ASTLengthNode::LengthBits,
        },
        {
            ASTTypeLengthOf,
            ASTLengthOfNode::LengthOfBits,
        },
        {
            ASTTypeLongDouble,
            ASTLongDoubleNode::LongDoubleBits,
        },
        {
            ASTTypeMeasure,
            ASTMeasureNode::MeasureBits,
        },
        {
            ASTTypePragma,
            ASTPragmaNode::PragmaBits,
        },
        {
            ASTTypeQubit,
            ASTQubitNode::QubitBits,
        },
        {
            ASTTypeQubitContainer,
            ASTQubitContainerNode::QubitContainerBits,
        },
        {
            ASTTypeQubitContainerAlias,
            ASTQubitContainerAliasNode::QubitContainerAliasBits,
        },
        {
            ASTTypeResult,
            ASTResultNode::ResultBits,
        },
        {
            ASTTypeReset,
            ASTResetNode::ResetBits,
        },
        {
            ASTTypeStretch,
            ASTStretchNode::StretchBits,
        },
        {
            ASTTypeUnaryOp,
            ASTUnaryOpNode::UnaryOpBits,
        },
        {
            ASTTypeControlExpression,
            ASTControlExpressionNode::ControlExpressionBits,
        },
        {
            ASTTypeGateNegControl,
            ASTGateNegControlNode::GateNegControlBits,
        },
        {
            ASTTypeGateControl,
            ASTGateControlNode::GateControlBits,
        },
        {
            ASTTypeGateControlStatement,
            ASTGateControlStmtNode::GateControlStmtBits,
        },
        {
            ASTTypeGateNegControlStatement,
            ASTGateNegControlStmtNode::GateNegControlStmtBits,
        },
        {
            ASTTypeGateInverseStatement,
            ASTGateInverseStmtNode::GateInverseStmtBits,
        },
        {
            ASTTypeGatePowerStatement,
            ASTGatePowerStmtNode::GatePowerStmtBits,
        },
        {
            ASTTypeGateNegControl,
            ASTGateNegControlNode::GateNegControlBits,
        },
        {ASTTypeFunctionCall, ASTCallExpressionNode::CallExpressionBits},
        {
            ASTTypeInverseExpression,
            ASTInverseExpressionNode::InverseExpressionBits,
        },
        {
            ASTTypeGateOpNode,
            ASTGateOpNode::GateOpBits,
        },
        {
            ASTTypeGateQOpNode,
            ASTGateOpNode::GateOpBits,
        },
        {
            ASTTypeGateUOpNode,
            ASTGateOpNode::GateOpBits,
        },
        {
            ASTTypeGateGenericOpNode,
            ASTGateOpNode::GateOpBits,
        },
        {
            ASTTypeGateHOpNode,
            ASTGateOpNode::GateOpBits,
        },
        {
            ASTTypeCXGateOpNode,
            ASTGateOpNode::GateOpBits,
        },
        {
            ASTTypeCCXGateOpNode,
            ASTGateOpNode::GateOpBits,
        },
        {ASTTypeCNotGateOpNode, ASTGateOpNode::GateOpBits},
        {
            ASTTypeGateControlStatement,
            ASTGateControlStmtNode::GateControlStmtBits,
        },
        {
            ASTTypeOpenPulseCalibration,
            OpenPulse::ASTOpenPulseCalibration::CalibrationBits,
        },
        {
            ASTTypeOpenPulseFrame,
            OpenPulse::ASTOpenPulseFrameNode::FrameBits,
        },
        {
            ASTTypeOpenPulseWaveform,
            OpenPulse::ASTOpenPulseWaveformNode::WaveformBits,
        },
        {
            ASTTypeOpenPulsePort,
            OpenPulse::ASTOpenPulsePortNode::PortBits,
        },
        {
            ASTTypeOpenPulsePlay,
            OpenPulse::ASTOpenPulsePlayNode::PlayBits,
        },
    };
  }

  if (RS.empty()) {
    RS = {
        "alpha",   "beta", "gamma", "delta",  "epsilon", "zeta",    "eta",
        "theta",   "iota", "kappa", "lambda", "mu",      "nu",      "xi",
        "omicron", "pi",   "rho",   "sigma",  "tau",     "upsilon", "phi",
        "chi",     "psi",  "omega", u8"α",    u8"β",     u8"γ",     u8"δ",
        u8"ε",     u8"ζ",  u8"η",   u8"θ",    u8"ι",     u8"κ",     u8"λ",
        u8"μ",     u8"ν",  u8"ξ",   u8"ο",    u8"π",     u8"ρ",     u8"σ",
        u8"τ",     u8"υ",  u8"φ",   u8"χ",    u8"ψ",     u8"ω",     u8"Α",
        u8"Β",     u8"Γ",  u8"Δ",   u8"Ε",    u8"Ζ",     u8"Η",     u8"Θ",
        u8"Ι",     u8"Κ",  u8"Λ",   u8"Μ",    u8"Ν",     u8"Ξ",     u8"Ο",
        u8"Π",     u8"Ρ",  u8"Σ",   u8"Τ",    u8"Υ",     u8"Φ",     u8"Χ",
        u8"Ψ",     u8"Ω",
    };
  }

  if (BFM.empty()) {
    BFM = {
        "mix",
        "sum",
        "phase_shift",
        "scale",
    };
  }

  if (BGM.empty()) {
    BGM = {
        "U",
    };
  }

  if (OQ2RG.empty()) {
    OQ2RG = {
        "h", "hadamard", "cx", "ccx", "cnot", "u",
    };
  }

  ASTIdentifierNode *Id = nullptr;
  ASTSymbolTableEntry *STE = nullptr;

  ASTGateContextBuilder::Instance().OpenContext();

  for (std::map<std::string, ASTSymbolScope>::const_iterator I = LM.begin();
       I != LM.end(); ++I) {
    Id = ASTBuilder::Instance().CreateASTIdentifierNode((*I).first, Bits,
                                                        ASTTypeAngle);
    assert(Id && "Could not create an ASTIdentifierNode!");

    Id->SetPolymorphicName((*I).first);
    Id->SetGlobalScope();

    ASTAngleNode *AN = nullptr;
    ASTAngleType ATy = ASTAngleNode::DetermineAngleType((*I).first);
    STE = ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeAngle);

    if (!STE) {
      AN = ASTBuilder::Instance().CreateASTAngleNode(Id, ATy, Bits);
      assert(AN && "Could not create an ASTAngleNode!");

      STE = ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeAngle);
      assert(STE && "ASTAngleNode has no SymbolTable Entry!");
    }

    if (!STE->HasValue()) {
      AN = ASTBuilder::Instance().CreateASTAngleNode(Id, ATy, Bits);
      assert(AN && "Could not create an ASTAngleNode!");

      AN->Mangle();
      AN->MangleLiteral();
      STE->ResetValue();
      STE->SetValue(new ASTValue<>(AN, ASTTypeAngle), ASTTypeAngle);
    }

    STE->SetGlobalScope();
    STE->SetDoNotDelete();
    ASTAngleNodeBuilder::Instance().Append(AN);
    ASTAngleNodeBuilder::Instance().Insert(AN);
  }

  for (std::map<std::string, ASTSymbolScope>::const_iterator I = GM.begin();
       I != GM.end(); ++I) {
    Id = ASTBuilder::Instance().CreateASTIdentifierNode((*I).first, Bits,
                                                        ASTTypeAngle);
    assert(Id && "Could not create an ASTIdentifierNode!");

    Id->SetPolymorphicName((*I).first);
    Id->SetGlobalScope();

    ASTAngleNode *AN = nullptr;
    ASTAngleType ATy = ASTAngleNode::DetermineAngleType((*I).first);
    STE = ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeAngle);

    if (!STE) {
      AN = ASTBuilder::Instance().CreateASTAngleNode(Id, ATy, Bits);
      assert(AN && "Could not create an ASTAngleNode!");

      AN->Mangle();
      AN->MangleLiteral();
      STE = ASTSymbolTable::Instance().Lookup(Id, Bits, ASTTypeAngle);
      assert(STE && "ASTAngleNode has no SymbolTable Entry!");
    }

    if (!STE->HasValue()) {
      AN = ASTBuilder::Instance().CreateASTAngleNode(Id, ATy, Bits);
      assert(AN && "Could not create an ASTAngleNode!");

      STE->ResetValue();
      STE->SetValue(new ASTValue<>(AN, ASTTypeAngle), ASTTypeAngle);
    }

    STE->SetGlobalScope();
    STE->SetDoNotDelete();
    ASTAngleNodeBuilder::Instance().Append(AN);
    ASTAngleNodeBuilder::Instance().Insert(AN);
  }

  ASTTypeSystemBuilder::Instance().CreateASTReservedAngles();
  ASTTypeSystemBuilder::Instance().CreateASTReservedMPDecimalValues();
  ASTTypeSystemBuilder::Instance().CreateASTBuiltinUGate();
  ASTTypeSystemBuilder::Instance().CreateASTBuiltinCXGate();

  ASTDefcalGrammarBuilder::Instance().SetCurrent("openpulse");
  ASTBuiltinFunctionsBuilder::Instance().Init();
}

void ASTTypeSystemBuilder::CreateASTBuiltinUGate() const {
  ASTParameterList GPL;
  ASTIdentifierList QIL;

  // Ensure that the gate angle parameters and the gate qubit
  // parameter are created in a Gate Declaration Context.
  ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeGate);

  ASTIdentifierNode *ThetaId = ASTBuilder::Instance().CreateASTIdentifierNode(
      u8"θ", ASTAngleNode::AngleBits, ASTTypeAngle);
  assert(ThetaId && "Could not create a valid Theta ASTIdentifierNode!");

  ThetaId->SetPolymorphicName(u8"θ");
  ThetaId->SetGateLocal(true);
  ThetaId->SetLocalScope();

  ASTAngleNode *Theta = ASTBuilder::Instance().CreateASTAngleNode(
      ThetaId, ASTAngleNode::DetermineAngleType(u8"θ"),
      ASTAngleNode::AngleBits);
  assert(Theta && "Could not createa a valid Theta ASTAngleNode!");

  ASTIdentifierNode *PhiId = ASTBuilder::Instance().CreateASTIdentifierNode(
      u8"φ", ASTAngleNode::AngleBits, ASTTypeAngle);
  assert(PhiId && "Could not create a valid Phi ASTIdentifierNode!");

  PhiId->SetPolymorphicName(u8"φ");
  PhiId->SetGateLocal(true);
  PhiId->SetLocalScope();

  ASTAngleNode *Phi = ASTBuilder::Instance().CreateASTAngleNode(
      PhiId, ASTAngleNode::DetermineAngleType(u8"φ"), ASTAngleNode::AngleBits);
  assert(Phi && "Could not create a valid Phi ASTAngleNode!");

  ASTIdentifierNode *LambdaId = ASTBuilder::Instance().CreateASTIdentifierNode(
      u8"λ", ASTAngleNode::AngleBits, ASTTypeAngle);
  assert(LambdaId && "Could not create a valid Lambda ASTIdentifierNode!");

  LambdaId->SetPolymorphicName(u8"λ");
  LambdaId->SetGateLocal(true);
  LambdaId->SetLocalScope();

  ASTAngleNode *Lambda = ASTBuilder::Instance().CreateASTAngleNode(
      LambdaId, ASTAngleNode::DetermineAngleType(u8"λ"),
      ASTAngleNode::AngleBits);
  assert(Lambda && "Could not create a valid Lambda ASTAngleNode!");

  GPL.Append(Lambda);
  GPL.Append(Theta);
  GPL.Append(Phi);

  ASTIdentifierNode *QId = ASTBuilder::Instance().CreateASTIdentifierNode(
      "q", 1U, ASTTypeGateQubitParam);
  assert(QId && "Could not create a valid Qubit ASTIdentifierNode!");

  ASTIdentifierTypeController::Instance().CheckGateQubitParamType(QId);
  QId->SetGateLocal(true);
  QId->SetLocalScope();
  QIL.Append(QId);

  ASTIdentifierNode *UId = ASTBuilder::Instance().CreateASTIdentifierNode(
      u8"U", ASTGateNode::GateBits, ASTTypeUGate);
  assert(UId && "Could not create a valid UGate ASTIdentifierNode!");

  UId->SetPolymorphicName("U");
  ASTGateNode *UGN =
      ASTBuilder::Instance().CreateASTGateNode(UId, ASTGateKindU, GPL, QIL);
  assert(UGN && "Could not create a valid UGate ASTGateNode!");

  UGN->Mangle();
  ASTGateQubitTracker::Instance().Erase();
  QIL.DeleteSymbols();
  GPL.DeleteSymbols();
  ASTSymbolTable::Instance().EraseLocalAngle(LambdaId);
  ASTSymbolTable::Instance().EraseLocalAngle(ThetaId);
  ASTSymbolTable::Instance().EraseLocalAngle(PhiId);
  ASTGateQubitParamBuilder::Instance().ReleaseQubits();
  ASTGateContextBuilder::Instance().CloseContext();
  ASTDeclarationContextTracker::Instance().PopCurrentContext();
}

void ASTTypeSystemBuilder::CreateASTBuiltinCXGate() const {
  if (ASTOpenQASMVersionTracker::Instance().GetVersion() >= 3.0)
    return;

  ASTParameterList GPL;
  ASTIdentifierList QIL;

  ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeGate);

  ASTIdentifierNode *CId = ASTBuilder::Instance().CreateASTIdentifierNode(
      "c", 1U, ASTTypeGateQubitParam);
  assert(CId && "Could not create a valid Qubit ASTIdentifierNode!");

  ASTIdentifierTypeController::Instance().CheckGateQubitParamType(CId);
  CId->SetGateLocal(true);
  CId->SetLocalScope();
  QIL.Append(CId);

  ASTIdentifierNode *TId = ASTBuilder::Instance().CreateASTIdentifierNode(
      "t", 1U, ASTTypeGateQubitParam);
  assert(TId && "Could not create a valid Qubit ASTIdentifierNode!");

  ASTIdentifierTypeController::Instance().CheckGateQubitParamType(TId);
  TId->SetGateLocal(true);
  TId->SetLocalScope();
  QIL.Append(TId);

  ASTIdentifierNode *GId = ASTBuilder::Instance().CreateASTIdentifierNode(
      u8"CX", ASTGateNode::GateBits, ASTTypeCXGate);
  assert(GId && "Could not create a valid UGate ASTIdentifierNode!");

  GId->SetPolymorphicName("CX");
  ASTGateNode *CGN =
      ASTBuilder::Instance().CreateASTGateNode(GId, ASTGateKindCX, GPL, QIL);
  assert(CGN && "Could not create a valid UGate ASTGateNode!");

  CGN->Mangle();
  ASTGateQubitTracker::Instance().Erase();
  QIL.DeleteSymbols();
  GPL.DeleteSymbols();
  ASTGateQubitParamBuilder::Instance().ReleaseQubits();
  ASTGateContextBuilder::Instance().CloseContext();
  ASTDeclarationContextTracker::Instance().PopCurrentContext();
}

void ASTTypeSystemBuilder::CreateASTReservedAngles() const {
  ASTIdentifierNode *Id;
  const ASTSymbolTableEntry *STE;
  ASTAngleNode *ANG;

  Id = ASTBuilder::Instance().CreateASTIdentifierNode(
      u8"pi", ASTAngleNode::AngleBits, ASTTypeAngle);
  assert(Id && "Could not create a valid ASTIdentifierNode!");

  ASTMPDecimalNode *Pi = ASTMPDecimalNode::Pi(ASTAngleNode::AngleBits);
  assert(Pi && "Could not obtain a valid Pi constant!");

  ANG = ASTBuilder::Instance().CreateASTAngleNode(
      Id, ASTAngleTypePi, Pi->GetMPValue(), ASTAngleNode::AngleBits);
  assert(ANG && "Could not create a valid ASTAngleNode!");

  STE = ASTSymbolTable::Instance().FindAngle(Id);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  Id = ASTBuilder::Instance().CreateASTIdentifierNode(
      u8"π", ASTAngleNode::AngleBits, ASTTypeAngle);
  assert(Id && "Could not create a valid ASTIdentifierNode!");

  ANG = ASTBuilder::Instance().CreateASTAngleNode(
      Id, ASTAngleTypePi, Pi->GetMPValue(), ASTAngleNode::AngleBits);
  assert(ANG && "Could not create a valid ASTAngleNode!");

  STE = ASTSymbolTable::Instance().FindAngle(Id);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  Id = ASTBuilder::Instance().CreateASTIdentifierNode(
      u8"tau", ASTAngleNode::AngleBits, ASTTypeAngle);
  assert(Id && "Could not create a valid ASTIdentifierNode!");

  ASTMPDecimalNode *Tau = ASTMPDecimalNode::Tau(ASTAngleNode::AngleBits);
  assert(Tau && "Could not obtain a valid Tau constant!");

  ANG = ASTBuilder::Instance().CreateASTAngleNode(
      Id, ASTAngleTypeTau, Tau->GetMPValue(), ASTAngleNode::AngleBits);
  assert(ANG && "Could not create a valid ASTAngleNode!");

  STE = ASTSymbolTable::Instance().FindAngle(Id);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  Id = ASTBuilder::Instance().CreateASTIdentifierNode(
      u8"τ", ASTAngleNode::AngleBits, ASTTypeAngle);
  assert(Id && "Could not create a valid ASTIdentifierNode!");

  ANG = ASTBuilder::Instance().CreateASTAngleNode(
      Id, ASTAngleTypeTau, Tau->GetMPValue(), ASTAngleNode::AngleBits);
  assert(ANG && "Could not create a valid ASTAngleNode!");

  STE = ASTSymbolTable::Instance().FindAngle(Id);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  Id = ASTBuilder::Instance().CreateASTIdentifierNode(
      u8"euler", ASTAngleNode::AngleBits, ASTTypeAngle);
  assert(Id && "Could not create a valid ASTIdentifierNode!");

  ASTMPDecimalNode *Euler = ASTMPDecimalNode::Euler(ASTAngleNode::AngleBits);
  assert(Euler && "Could not obtain a valid Euler constant!");

  ANG = ASTBuilder::Instance().CreateASTAngleNode(
      Id, ASTAngleTypeTau, Euler->GetMPValue(), ASTAngleNode::AngleBits);
  assert(ANG && "Could not create a valid ASTAngleNode!");

  STE = ASTSymbolTable::Instance().FindAngle(Id);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");

  Id = ASTBuilder::Instance().CreateASTIdentifierNode(
      u8"ε", ASTAngleNode::AngleBits, ASTTypeAngle);
  assert(Id && "Could not create a valid ASTIdentifierNode!");

  ANG = ASTBuilder::Instance().CreateASTAngleNode(
      Id, ASTAngleTypeEuler, Euler->GetMPValue(), ASTAngleNode::AngleBits);
  assert(ANG && "Could not create a valid ASTAngleNode!");

  STE = ASTSymbolTable::Instance().FindAngle(Id);
  assert(STE && "Could not obtain a valid SymbolTable Entry!");
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");
}

void ASTTypeSystemBuilder::CreateASTReservedMPDecimalValues() const {
  ASTIdentifierNode *Id;
  ASTMPDecimalNode *MPD;
  const ASTSymbolTableEntry *STE;

  const ASTDeclarationContext *GCX =
      ASTDeclarationContextTracker::Instance().GetGlobalContext();
  assert(GCX && "Could not obtain a valid Global DeclarationContext!");

  Id = ASTBuilder::Instance().CreateASTIdentifierNode(
      "pi", ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal, GCX, true);
  assert(Id && "Could not create a valid ASTIdentifierNode!");

  ASTMPDecimalNode *Pi = ASTMPDecimalNode::Pi(ASTMPDecimalNode::DefaultBits);
  assert(Pi && "Could not obtain a valid Pi constant!");

  MPD = ASTBuilder::Instance().CreateASTMPDecimalNode(
      Id, ASTMPDecimalNode::DefaultBits, Pi->GetMPValue());
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  STE = ASTSymbolTable::Instance().FindGlobal(Id);
  assert(STE && "Could not obtain a valid ASTSymbolTable Entry!");
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");
  const_cast<ASTSymbolTableEntry *>(STE)->SetDoNotDelete();

  Id = ASTBuilder::Instance().CreateASTIdentifierNode(
      "π", ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal, GCX, true);
  assert(Id && "Could not create a valid ASTIdentifierNode!");

  MPD = ASTBuilder::Instance().CreateASTMPDecimalNode(
      Id, ASTMPDecimalNode::DefaultBits, Pi->GetMPValue());
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  STE = ASTSymbolTable::Instance().FindGlobal(Id);
  assert(STE && "Could not obtain a valid ASTSymbolTable Entry!");
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");
  const_cast<ASTSymbolTableEntry *>(STE)->SetDoNotDelete();

  ASTMPDecimalNode *Tau = ASTMPDecimalNode::Tau(ASTMPDecimalNode::DefaultBits);
  assert(Tau && "Could not obtain a valid Tau constant!");

  Id = ASTBuilder::Instance().CreateASTIdentifierNode(
      "tau", ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal, GCX, true);
  assert(Id && "Could not create a valid ASTIdentifierNode!");

  MPD = ASTBuilder::Instance().CreateASTMPDecimalNode(
      Id, ASTMPDecimalNode::DefaultBits, Tau->GetMPValue());
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  STE = ASTSymbolTable::Instance().FindGlobal(Id);
  assert(STE && "Could not obtain a valid ASTSymbolTable Entry!");
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");
  const_cast<ASTSymbolTableEntry *>(STE)->SetDoNotDelete();

  Id = ASTBuilder::Instance().CreateASTIdentifierNode(
      "τ", ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal, GCX, true);
  assert(Id && "Could not create a valid ASTIdentifierNode!");

  MPD = ASTBuilder::Instance().CreateASTMPDecimalNode(
      Id, ASTMPDecimalNode::DefaultBits, Tau->GetMPValue());
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  STE = ASTSymbolTable::Instance().FindGlobal(Id);
  assert(STE && "Could not obtain a valid ASTSymbolTable Entry!");
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");
  const_cast<ASTSymbolTableEntry *>(STE)->SetDoNotDelete();

  Id = ASTBuilder::Instance().CreateASTIdentifierNode(
      "euler", ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal, GCX, true);
  assert(Id && "Could not create a valid ASTIdentifierNode!");

  ASTMPDecimalNode *Euler =
      ASTMPDecimalNode::Euler(ASTMPDecimalNode::DefaultBits);
  assert(Euler && "Could not obtain a valid Euler constant!");

  MPD = ASTBuilder::Instance().CreateASTMPDecimalNode(
      Id, ASTMPDecimalNode::DefaultBits, Euler->GetMPValue());
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  STE = ASTSymbolTable::Instance().FindGlobal(Id);
  assert(STE && "Could not obtain a valid ASTSymbolTable Entry!");
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");
  const_cast<ASTSymbolTableEntry *>(STE)->SetDoNotDelete();

  Id = ASTBuilder::Instance().CreateASTIdentifierNode(
      "ε", ASTMPDecimalNode::DefaultBits, ASTTypeMPDecimal, GCX, true);
  assert(Id && "Could not create a valid ASTIdentifierNode!");

  MPD = ASTBuilder::Instance().CreateASTMPDecimalNode(
      Id, ASTMPDecimalNode::DefaultBits, Euler->GetMPValue());
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  STE = ASTSymbolTable::Instance().FindGlobal(Id);
  assert(STE && "Could not obtain a valid ASTSymbolTable Entry!");
  assert(STE->HasValue() && "SymbolTable Entry has no Value!");
  const_cast<ASTSymbolTableEntry *>(STE)->SetDoNotDelete();
}

unsigned ASTTypeSystemBuilder::GetTypeBits(ASTType Ty) const {
  std::map<ASTType, unsigned>::const_iterator I = TBM.find(Ty);
  return I == TBM.end() ? static_cast<unsigned>(~0x0) : (*I).second;
}

bool ASTTypeSystemBuilder::LocateImplicitSymbol(const std::string &S) const {
  assert(!S.empty() && "Invalid symbol name argument!");

  if (ASTTypeSystemBuilder::Instance().IsReservedAngle(S))
    return true;

  if (ASTSymbolTableEntry *STE =
          ASTSymbolTable::Instance().Lookup(S, ASTTypeAngle))
    return STE->GetValueType() == ASTTypeAngle;

  if (ASTAngleContextControl::Instance().InOpenContext() ||
      ASTGateContextBuilder::Instance().InOpenContext() ||
      ASTDefcalContextBuilder::Instance().InOpenContext())
    return RS.find(S) != RS.end();

  return false;
}

} // namespace QASM
