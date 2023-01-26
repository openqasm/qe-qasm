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
#include <qasm/AST/ASTIdentifierTypeController.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTScopeController.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <string_view>
#include <random>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

uint64_t ASTIdentifierNode::SI = 0UL;

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Char("char", ASTTypeChar, 8U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Short("short", ASTTypeShort, 16U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Int("int", ASTTypeInt, 32U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::UInt("unsigned int", ASTTypeInt, 32U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Long("long", ASTTypeLong, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::ULong("unsigned long", ASTTypeUnsignedLong, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Float("float", ASTTypeFloat, 32U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Double("double", ASTTypeDouble, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::LongDouble("long double", ASTTypeLongDouble, 128U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Void("void", ASTTypeVoid, ASTVoidNode::VoidBits);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::String("std::string", ASTTypeStringLiteral,
                                            (unsigned) ~0x0);
ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Bool("bool", ASTTypeBool, 8U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Pointer("pointer", ASTTypePointer, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Ellipsis("ellipsis", ASTTypeEllipsis, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::MPInt("mpinteger", ASTTypeMPInteger, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::MPDec("mpdecimal", ASTTypeMPDecimal, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::MPComplex("mpcomplex", ASTTypeMPComplex, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Imag("imag", ASTTypeImaginary, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Lambda("lambda", ASTTypeAngle, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Phi("phi", ASTTypeAngle, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Theta("theta", ASTTypeAngle, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Pi("pi", ASTTypeAngle, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Tau("tau", ASTTypeAngle, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::EulerNumber("euler_number", ASTTypeAngle, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Euler("euler", ASTTypeAngle, 64);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Gate("gate", ASTTypeGate, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::GateQOp("gateqop", ASTTypeGateQOpNode, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Defcal("defcal", ASTTypeDefcal, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Duration("duration", ASTTypeDuration, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::DurationOf("durationof", ASTTypeDurationOf, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Stretch("stretch", ASTTypeStretch, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Length("length", ASTTypeLength, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Measure("measure", ASTTypeMeasure, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Box("box", ASTTypeBox, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::BoxAs("boxas", ASTTypeBoxAs, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::BoxTo("boxto", ASTTypeBoxTo, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Qubit("qubit", ASTTypeQubit, 1U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::QubitParam("qubitparam", ASTTypeGateQubitParam, 1U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::QCAlias("qubitcontaineralias", ASTTypeQubitContainerAlias, 1U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::QC("qubitcontainer", ASTTypeQubitContainer, 1U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Bitset("bitset", ASTTypeBitset, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Angle("angle", ASTTypeAngle, ASTAngleNode::AngleBits);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Null("null", 8);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Operator("operator", ASTTypeOpTy, ASTOperatorNode::OperatorBits);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Operand("operand", ASTTypeOpndTy, ASTOperandNode::OperandBits);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::BinaryOp("binaryop", ASTTypeBinaryOp, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::UnaryOp("unaryop", ASTTypeUnaryOp, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Inv("inv", ASTTypeInverseExpression, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Pow("pow", ASTTypePow, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Cast("cast", ASTTypeCast, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::BadCast("badcast", ASTTypeBadCast, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::ImplConv("implicitconversion", ASTTypeImplicitConversion,
                            (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::BadImplConv("badimplicitconversion",
                               ASTTypeBadImplicitConversion, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Ctrl("ctrl", ASTTypeControlExpression, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::NegCtrl("negctrl", ASTTypeGateNegControl, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::BadCtrl("badctrl", ASTTypeControlExpression, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Expression("Expression", ASTTypeExpression, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Statement("Statement", ASTTypeStatement, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Reset("reset", ASTTypeReset, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Result("result", ASTTypeResult, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Return("return", ASTTypeReturn, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::GPhase("gphase", ASTTypeGPhaseExpression, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::BadGPhase("badgphase", ASTTypeGPhaseExpression, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Delay("delay", ASTTypeDelay, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Input("input", ASTTypeInputModifier, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Output("output", ASTTypeOutputModifier, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::IfExpression("IfStatement", ASTTypeIfStatement,
                                (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::ElseIfExpression("ElseIfStatement", ASTTypeElseIfStatement,
                                    (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::ElseExpression("ElseStatement", ASTTypeElseStatement,
                                  (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::For("for", ASTTypeForStatement, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::ForLoopRange("forlooprange", ASTTypeForLoopRange,
                                (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::While("while", ASTTypeWhileStatement, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::DoWhile("do-while", ASTTypeDoWhileStatement, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Switch("switch", ASTTypeSwitchStatement, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Case("case", ASTTypeCaseStatement, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Default("default", ASTTypeDefaultStatement, (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::QPPDirective("QPPDirective", ASTTypeDirectiveStatement,
                                (unsigned) ~0x0);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Pragma("pragma", ASTTypePragma, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Annotation("annotation", ASTTypeAnnotation, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Popcount("popcount", ASTTypePopcount, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Rotl("rotl", ASTTypeRotl, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Rotr("rotr", ASTTypeRotr, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Rotate("rotate", ASTTypeRotateExpr, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::ArraySubscript("ArraySubscript", ASTTypeArraySubscript, 64);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::CBitArray("cbitarray", ASTTypeCBitArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::QubitArray("qubitarray", ASTTypeQubitArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::BoolArray("boolarray", ASTTypeBoolArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::IntArray("intarray", ASTTypeIntArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::FloatArray("floatarray", ASTTypeFloatArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::MPIntArray("mpintarray", ASTTypeMPIntegerArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::MPDecArray("mpdecimalarray", ASTTypeMPDecimalArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::MPComplexArray("mpcomplexarray", ASTTypeMPComplexArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::AngleArray("anglearray", ASTTypeAngleArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::DurationArray("durationarray", ASTTypeDurationArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::FrameArray("framearray", ASTTypeOpenPulseFrameArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::PortArray("portarray", ASTTypeOpenPulsePortArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::WaveformArray("waveformarray", ASTTypeOpenPulseWaveformArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::InvalidArray("invalidarray", ASTTypeInvalidArray, 0U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::InitializerList("initializerlist", ASTTypeInitializerList,
                                   static_cast<unsigned>(~0x0));

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::SyntaxError("SyntaxError", ASTTypeSyntaxError, 64U);

// OpenPulse
ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Port("port", ASTTypeOpenPulsePort, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Play("play", ASTTypeOpenPulsePlay, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Frame("frame", ASTTypeOpenPulseFrame, 64U);

ASTIdentifierNode
__attribute__((init_priority(201)))
ASTIdentifierNode::Waveform("waveform", ASTTypeOpenPulseWaveform, 64U);

// ASTIdentifierTypeController
ASTIdentifierTypeController ASTIdentifierTypeController::ITC;
std::vector<ASTType> ASTIdentifierTypeController::TV;
ASTType ASTIdentifierTypeController::CT = ASTTypeUndefined;
ASTType ASTIdentifierTypeController::PT = ASTTypeUndefined;
ASTType ASTIdentifierTypeController::NT = ASTTypeUndefined;
bool ASTIdentifierTypeController::IA;
bool ASTIdentifierTypeController::SCR;
bool ASTIdentifierTypeController::PSC;

ASTIdentifierNode::ASTIdentifierNode(const std::string& Id,
                                     const ASTBinaryOpNode* BOp,
                                     unsigned B)
  : ASTExpression(), Name(Id), MangledName(), PolymorphicName(Id),
  MangledLiteralName(), IndexIdentifier(), Hash(0UL), MHash(0UL),
  MLHash(0UL), References(), Bits(B), NumericIndex(static_cast<unsigned>(~0x0)),
  Indexed(true), NoQubit(false), GateLocal(false), ComplexPart(false),
  HasSTE(false), RV(nullptr), BOP(BOp), EXP(nullptr), STE(nullptr),
  CTX(ASTDeclarationContextTracker::Instance().GetCurrentContext()),
  EvalType(ASTTypeBinaryOp), SType(BOp->GetASTType()), PType(ASTTypeUndefined),
  OpType(BOp->GetOpType()),
  SymScope(ASTDeclarationContextTracker::Instance().GetCurrentScope()),
  RD(false), PRD(nullptr) {
    CTX->RegisterSymbol(this, GetASTType());
    std::string::size_type LB = Name.find_last_of('[');
    std::string::size_type RB = Name.find_last_of(']');
    if (LB != std::string::npos && RB != std::string::npos) {
      SetIndexed(true);
      SetIndexIdentifier(LB, RB);
      SetNumericIndex(LB, RB);
    }
}

ASTIdentifierNode::ASTIdentifierNode(const std::string& Id,
                                     const ASTUnaryOpNode* UOp,
                                     unsigned B)
  : ASTExpression(), Name(Id), MangledName(), PolymorphicName(Id),
  MangledLiteralName(), IndexIdentifier(), Hash(0UL), MHash(0UL),
  MLHash(0UL), References(), Bits(B), NumericIndex(static_cast<unsigned>(~0x0)),
  Indexed(true), NoQubit(false), GateLocal(false), ComplexPart(false),
  HasSTE(false), RV(nullptr), UOP(UOp), EXP(nullptr), STE(nullptr),
  CTX(ASTDeclarationContextTracker::Instance().GetCurrentContext()),
  EvalType(ASTTypeUnaryOp), SType(UOp->GetASTType()), PType(ASTTypeUndefined),
  OpType(UOp->GetOpType()),
  SymScope(ASTDeclarationContextTracker::Instance().GetCurrentScope()),
  RD(false), PRD(nullptr) {
    CTX->RegisterSymbol(this, GetASTType());
    std::string::size_type LB = Name.find_last_of('[');
    std::string::size_type RB = Name.find_last_of(']');
    if (LB != std::string::npos && RB != std::string::npos) {
      SetIndexed(true);
      SetIndexIdentifier(LB, RB);
      SetNumericIndex(LB, RB);
    }
}

const ASTIdentifierNode* ASTExpression::GetIdentifier() const {
  return &ASTIdentifierNode::Expression;
}

void ASTIdentifierNode::SetPredecessor(const ASTIdentifierNode* PId) {
  assert(PId && "Invalid ASTIdentifierNode argument!");

  if (this == PId) {
    std::stringstream M;
    M << "An Identifier cannot be its own predecessor.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(),
                                                     DiagLevel::Error);
  }

  PRD = PId;
}

void ASTIdentifierRefNode::SetPredecessor(const ASTIdentifierNode* PId) {
  (void) PId; // Quiet compiler warning.

  std::stringstream M;
  M << "An Identifier Reference cannot have a predecessor.";
  QasmDiagnosticEmitter::Instance().EmitDiagnostic(
    DIAGLineCounter::Instance().GetLocation(this), M.str(),
                                                   DiagLevel::Error);
}

ASTIdentifierNode* ASTIdentifierNode::Clone() {
  if (!ASTObjectTracker::Instance().IsStatic(this)) {
    std::stringstream M;
    M << "Only data segment allocated ASTIdentifierNodes can be cloned.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  ASTIdentifierNode* RI = new ASTIdentifierNode(Name, SType, Bits);
  assert(RI && "Could not clone a valid ASTIdentifierNode!");
  return RI;
}

ASTIdentifierNode* ASTIdentifierNode::Clone(const ASTLocation& LC) {
  if (ASTIdentifierNode* RI = Clone()) {
    RI->SetLocation(LC);
    return RI;
  }

  return nullptr;
}

ASTIdentifierNode* ASTIdentifierNode::Clone(unsigned NBits) {
  if (ASTIdentifierNode::InvalidBits(NBits)) {
    std::stringstream M;
    M << "Invalid number of bits for ASTIdentifierNode.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(),
                                                     DiagLevel::Error);
    return nullptr;
  }

  ASTIdentifierNode* RI = Clone();
  assert(RI && "Could not clone a valid ASTIdentifierNode!");
  RI->SetBits(NBits);
  return RI;
}

ASTType ASTIdentifierNode::GetValueType() const {
  return STE ? STE->GetValueType() : ASTTypeUndefined;
}

void ASTIdentifierNode::SetSymbolTableEntry(ASTSymbolTableEntry* ST) {
  assert(ST && "Invalid SymbolTable Entry argument!");

  STE = ST;
  HasSTE = true;
  SType = STE->GetValueType();
}

void ASTIdentifierNode::SetSymbolType(ASTType STy) {
  SType = STy;
  STE->SetValueType(STy);
}

void ASTIdentifierNode::SetPolymorphicType(ASTType PTy) {
  PType = PTy;
}

void ASTIdentifierNode::RestoreType() {
  if (PType != ASTTypeUndefined) {
    SType = PType;
    STE->SetValueType(PType);
  }
}

void ASTIdentifierNode::AddReference(const ASTIdentifierRefNode* IdR) const {
  assert(IdR && "Invalid ASTIdentifierRefNode argument!");
  References.insert(std::make_pair(IdR->GetIndex(), IdR));
}

bool ASTIdentifierNode::IsReference(const ASTIdentifierRefNode* IdR) const {
  assert(IdR && "Invalid ASTIdentifierRefNode argument!");
  std::map<unsigned, const ASTIdentifierRefNode*>::const_iterator I =
    References.find(IdR->GetIndex());
  return I != References.end();
}

const ASTIdentifierRefNode* ASTIdentifierNode::GetReference(unsigned IX) const {
  std::map<unsigned, const ASTIdentifierRefNode*>::const_iterator I =
    References.find(IX);
  return I == References.end() ? nullptr: (*I).second;
}

void ASTIdentifierNode::SetBinaryOp(const ASTBinaryOpNode* BOp) {
  BOP = BOp;
  EvalType = BOp->GetASTType();
  OpType = BOp->GetOpType();
  SetIndexed(true);
}

void ASTIdentifierNode::SetUnaryOp(const ASTUnaryOpNode* UOp) {
  UOP = UOp;
  EvalType = UOp->GetASTType();
  OpType = UOp->GetOpType();
  SetIndexed(true);
}

void ASTIdentifierNode::print() const {
  std::cout << "<Identifier>" << std::endl;
  if (!IndexIdentifier.empty())
    std::cout << "<IndexIdentifier>" << IndexIdentifier.c_str()
      << "</IndexIdentifier>" << std::endl;
  std::cout << "<Name>" << Name.c_str() << "</Name>" << std::endl;
  std::cout << "<MangledName>" << MangledName.c_str() << "</MangledName>"
    << std::endl;

  if (!MangledLiteralName.empty())
    std::cout << "<MangledLiteralName>" << MangledLiteralName.c_str()
      << "</MangledLiteralName>" << std::endl;

  if (!PolymorphicName.empty())
    std::cout << "<PolymorphicName>" << PolymorphicName.c_str()
      << "</PolymorphicName>" << std::endl;

  std::cout << "<Bits>" << std::dec << Bits << "</Bits>" << std::endl;
  std::cout << "<Type>" << PrintTypeEnum(SType) << "</Type>" << std::endl;
  std::cout << "<DeclarationContext>" << CTX->GetIndex()
    << "</DeclarationContext>" << std::endl;
  std::cout << "<IsRedeclaration>" << std::boolalpha << RD
    << "</IsRedeclaration>" << std::endl;

  if (RD && PRD) {
    std::cout << "<Predecessor>" << std::endl;
    std::cout << "<Name>" << PRD->GetName() << "</Name>" << std::endl;
    std::cout << "<Bits>" << PRD->GetBits() << "</Bits>" << std::endl;
    std::cout << "<Type>" << PrintTypeEnum(PRD->GetSymbolType())
      << "</Type>" << std::endl;
    std::cout << "<DeclarationContext>"
      << PRD->GetDeclarationContext()->GetIndex() << "</DeclarationContext>"
      << std::endl;
    std::cout << "</Predecessor>" << std::endl;
  }

  std::cout << "<Indexed>" << std::boolalpha << IsIndexed() << "</Indexed>"
    << std::endl;
  std::cout << "<NoQubit>" << std::boolalpha << IsNoQubit()
    << "</NoQubit>" << std::endl;
  std::cout << "<GateLocal>" << std::boolalpha << IsGateLocal()
    << "</GateLocal>" << std::endl;
  std::cout << "<ComplexPart>" << std::boolalpha << IsComplexPart()
    << "</ComplexPart>" << std::endl;
  std::cout << "<SymbolType>" << PrintTypeEnum(SType)
    << "</SymbolType>" << std::endl;
  std::cout << "<HasSymbolTableEntry>" << std::boolalpha << HasSTE
    << "</HasSymbolTableEntry>" << std::endl;

  if (STE) {
    std::cout << "<SymbolTableEntry>";
    std::cout << static_cast<void*>(STE);
    std::cout << "</SymbolTableEntry>" << std::endl;
    std::cout << "<SymbolTableValueType>"
      << PrintTypeEnum(STE->GetValueType())
      << "</SymbolTableValueType>" << std::endl;
  } else {
    std::cout << "<SymbolTableEntry>";
    std::cout << "0x0";
    std::cout << "</SymbolTableEntry>" << std::endl;
  }

  CTX->print();

  std::cout << "<RValue>" << std::boolalpha << IsRValue() << "</RValue>"
    << std::endl;
  if (EvalType == ASTTypeBinaryOp) {
    std::cout << "<BinaryOp>" << PrintOpTypeEnum(OpType)
      << "</BinaryOp>" << std::endl;
    std::cout << "<BinaryOpType>" << PrintOpTypeEnum(BOP->GetOpType())
      << "</BinaryOpType>" << std::endl;
    BOP->print();
  } else if (EvalType == ASTTypeUnaryOp) {
    std::cout << "<UnaryOp>" << PrintOpTypeEnum(OpType)
      << "</UnaryOp>" << std::endl;
    std::cout << "<UnaryOpType>" << PrintOpTypeEnum(UOP->GetOpType())
      << "</UnaryOpType>" << std::endl;
    UOP->print();
  }

  std::cout << "<SymbolScope>" << PrintSymbolScope(SymScope)
    << "</SymbolScope>" << std::endl;
  std::cout << "</Identifier>" << std::endl;
}

std::string ASTIdentifierNode::GenRandomString(unsigned Len) {
  static const char AlNum[] = "0123456789"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "abcdefghijklmnopqrstuvwxyz"
                              "-_+#";

  static thread_local std::default_random_engine
    RandomEngine(std::random_device{}());
  static thread_local std::uniform_int_distribution<int>
    RandomDistribution(0, sizeof(AlNum) - 2);

  std::string Tmp(Len < 19 ? 19 : Len, '\0');

  for (std::string::value_type& C : Tmp)
    C = AlNum[RandomDistribution(RandomEngine)];

  return Tmp;
}

unsigned
ASTIdentifierRefNode::GetIndexChain(const ASTIdentifierRefNode* IdR,
                                    std::vector<unsigned>& IXC) {
  assert(IdR && "Invalid ASTIdentifierRefNode argument!");

  std::string::size_type SP = 0;
  std::string::size_type EP = 0;
  std::string::size_type XP = 0;
  const std::string& IdS = IdR->GetName();
  std::string::size_type L = IdS.length();

  bool MM = false;
  std::string IDX = IdS;
  IXC.clear();

  while (L && SP < L && EP < L) {
    SP = IDX.find_first_of('[');
    EP = IDX.find_first_of(']');

    if (SP == std::string::npos && EP == std::string::npos) {
      break;
    } else if (SP == std::string::npos || EP == std::string::npos) {
      MM = true;
      break;
    }

    IDX = IDX.substr(SP + 1, EP - (SP + 1));

    if (!IDX.empty()) {
      IXC.push_back(static_cast<unsigned>(std::stoul(IDX)));
      ++SP;
      XP += ++EP;

      IDX = IdS.substr(XP, std::string::npos);
    }
  }

  if (MM) {
    std::stringstream M;
    M << "Mismatched subscript operator brackets at index "
      << (IXC.size() + 1) << '.';
    IXC.clear();
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(IdR), M.str(),
                                                    DiagLevel::Error);
    return 0;
  }

  return static_cast<unsigned>(IXC.size());
}

void ASTIdentifierRefNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetSymbolType(), GetBits(), GetName());
  M.EndExpression();
  M.End();
  ASTIdentifierNode::MangledName = M.AsString();
}

void ASTIdentifierRefNode::print() const {
  std::cout << "<IdentifierReference>" << std::endl;
  std::cout << "<Name>" << Name.c_str() << "</Name>" << std::endl;
  std::cout << "<MangledName>" << ASTIdentifierNode::MangledName
    << "</MangledName>" << std::endl;
  std::cout << "<Bits>" << Bits << "</Bits>" << std::endl;
  std::cout << "<Index>" << Index << "</Index>" << std::endl;
  std::cout << "<Indexed>" << std::boolalpha << IsIndexed() << "</Indexed>"
    << std::endl;
  std::cout << "<NoQubit>" << std::boolalpha << IsNoQubit()
    << "</NoQubit>" << std::endl;
  std::cout << "<RValue>" << std::boolalpha << IsRValue() << "</RValue>"
    << std::endl;
  if (EvalType == ASTTypeBinaryOp) {
    std::cout << "<BinaryOp>" << PrintOpTypeEnum(OpType)
      << "</BinaryOp>" << std::endl;
    std::cout << "<BinaryOpType>" << PrintOpTypeEnum(BOP->GetOpType())
      << "</BinaryOpType>" << std::endl;
    BOP->print();
  } else if (EvalType == ASTTypeUnaryOp) {
    std::cout << "<UnaryOp>" << PrintOpTypeEnum(OpType)
      << "</UnaryOp>" << std::endl;
    std::cout << "<UnaryOpType>" << PrintOpTypeEnum(UOP->GetOpType())
      << "</UnaryOpType>" << std::endl;
    UOP->print();
  }

  Id->print();
  std::cout << "</IdentifierReference>" << std::endl;
}

void ASTIdentifierRefNode::SetBinaryOp(const ASTBinaryOpNode* BOp) {
  BOP = BOp;
  EvalType = BOp->GetASTType();
  OpType = BOp->GetOpType();
  SetIndexed(true);
}

void ASTIdentifierRefNode::SetUnaryOp(const ASTUnaryOpNode* UOp) {
  UOP = UOp;
  EvalType = UOp->GetASTType();
  OpType = UOp->GetOpType();
  SetIndexed(true);
}

ASTIdentifierRefNode*
ASTIdentifierRefNode::MPInteger(const std::string& Id,
                                unsigned NumBits) {
  ASTIdentifierNode* IDN =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(Id, NumBits,
                                                         ASTTypeMPInteger);
  assert(IDN && "Could not create a valid ASTIdentifierNode!");

  return new ASTIdentifierRefNode(IDN, NumBits);
}

ASTIdentifierRefNode*
ASTIdentifierRefNode::MPDecimal(const std::string& Id,
                                unsigned NumBits) {
  ASTIdentifierNode* IDN =
    ASTBuilder::Instance().FindOrCreateASTIdentifierNode(Id, NumBits,
                                                         ASTTypeMPDecimal);
  assert(IDN && "Could not create a valid ASTIdentifierNode!");

  return new ASTIdentifierRefNode(IDN, NumBits);
}

ASTType
ASTIdentifierRefNode::ResolveReferenceType(ASTType ITy) const {
  switch (ITy) {
  case ASTTypeAngleArray:
    return ASTTypeAngle;
    break;
  case ASTTypeBoolArray:
    return ASTTypeBool;
    break;
  case ASTTypeCBitArray:
  case ASTTypeCBitNArray:
    return ASTTypeBitset;
    break;
  case ASTTypeFloatArray:
    return ASTTypeFloat;
    break;
  case ASTTypeIntArray:
    return ASTTypeInt;
    break;
  case ASTTypeLengthArray:
    return ASTTypeLength;
    break;
  case ASTTypeDurationArray:
    return ASTTypeDuration;
    break;
  case ASTTypeMPDecimalArray:
    return ASTTypeMPDecimal;
    break;
  case ASTTypeMPIntegerArray:
    return ASTTypeMPInteger;
    break;
  case ASTTypeMPComplexArray:
    return ASTTypeMPComplex;
  case ASTTypeQubitArray:
  case ASTTypeQubitNArray:
    return ASTTypeQubitContainer;
    break;
  case ASTTypeBitset:
    return ASTTypeBitset;
    break;
  case ASTTypeQubit:
    return ASTTypeQubit;
    break;
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias:
    return ASTTypeQubitContainer;
    break;
  case ASTTypeAngle:
    return ASTTypeAngle;
    break;
  case ASTTypeOpenPulseFrameArray:
    return ASTTypeOpenPulseFrame;
    break;
  case ASTTypeOpenPulsePortArray:
    return ASTTypeOpenPulsePort;
    break;
  default:
    break;
  }

  return ASTTypeUndefined;
}

void ASTTimedIdentifierNode::print() const {
  std::cout << "<TimedIdentifier>" << std::endl;
  ASTIdentifierNode::print();
  std::cout << "<Duration>" << Duration << "</Duration>" << std::endl;
  std::cout << "<Units>" << PrintLengthUnit(Units) << "</Units>"
    << std::endl;
  std::cout << "<TimedIdentifier>" << std::endl;
}

void ASTTimedIdentifierNode::ParseDuration(const std::string& DU) {
  if (DU.empty() || DU == "dt" || DU == "DT") {
    Duration = 0UL;
    Units = DT;
    return;
  }

  const char* C = DU.c_str();

  if (!std::isdigit(*C)) {
    std::stringstream M;
    M << "Duration must be expressed in numeric units.";
    Duration = static_cast<uint64_t>(~0x0);
    Units = LengthUnspecified;
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(),
                                                     DiagLevel::Error);
  }

  Duration = std::stoul(DU);

  while (*C && std::isdigit(*C++));

  std::string_view U = --C;

  if (U == "ns")
    Units = Nanoseconds;
  else if (U == "us" || U == u8"μs")
    Units = Microseconds;
  else if (U == "ms")
    Units = Milliseconds;
  else if (U == "s")
    Units = Seconds;
  else if (U == "dt")
    Units = DT;
  else {
    std::stringstream M;
    M << "Parse error on Duration units!";
    Units = LengthUnspecified;
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  }
}

void ASTIdentifierList::CheckOutOfScope() const {
  if (!Graph.empty()) {
    for (std::vector<ASTIdentifierNode*>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I) {
      ASTScopeController::Instance().CheckOutOfScope(*I);
    }
  }
}

bool ASTIdentifierList::CheckOnlyQubits() const {
  if (!Graph.empty()) {
    for (std::vector<ASTIdentifierNode*>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I) {
      switch ((*I)->GetSymbolType()) {
      case ASTTypeQubit:
      case ASTTypeQubitContainer:
      case ASTTypeQubitContainerAlias:
        continue;
        break;
      default:
        return false;
        break;
      }
    }
  }

  return true;
}

void ASTIdentifierList::DeleteSymbols() const {
  if (!Graph.empty()) {
    for (ASTIdentifierList::const_iterator I = Graph.begin();
         I != Graph.end(); ++I) {
      if (ASTDeclarationContextTracker::Instance().IsGlobalContext(
                                                      (*I)->GetDeclarationContext())) {
        continue;
      }

      const ASTSymbolTableEntry* STE = (*I)->GetSymbolTableEntry();
      if (STE) {
        ASTType STy = STE->GetValueType();

        if (!ASTDeclarationContextTracker::Instance().IsGlobalContext(
                                                  (*I)->GetDeclarationContext())) {
          ASTSymbolTable::Instance().EraseLocalSymbol((*I), (*I)->GetBits(), STy);
          ASTSymbolTable::Instance().EraseLocal((*I), (*I)->GetBits(), STy);
          ASTSymbolTable::Instance().EraseGateLocalQubit((*I));
        }
      }
    }
  }
}

}

