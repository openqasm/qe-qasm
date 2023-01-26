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

#include <qasm/AST/ASTBuiltinFunctionsBuilder.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseWaveform.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

namespace QASM {

ASTBuiltinFunctionsBuilder ASTBuiltinFunctionsBuilder::FB;
bool ASTBuiltinFunctionsBuilder::IsInit = false;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTFunctionDefinitionNode*
ASTBuiltinFunctionsBuilder::CreateBuiltinFunction(const std::string& Name) const {
  if (Name.empty())
    return nullptr;

  const ASTDeclarationContext* GCX =
    ASTDeclarationContextTracker::Instance().GetGlobalContext();
  assert(GCX && "Could not obtain a valid ASTDeclarationContext!");

  ASTLocation Loc;

  if (Name == "mix") {
    ASTIdentifierNode* Id = ASTBuilder::Instance().CreateASTIdentifierNode(Name,
                                        ASTFunctionDefinitionNode::FunctionBits,
                                        ASTTypeFunction);
    assert(Id && "Could not create a valid builtin function ASTIdentifierNode!");

    Id->SetDeclarationContext(GCX);
    Id->SetGlobalScope();
    Id->SetLocation(Loc);

    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(Id,
                                        ASTFunctionDefinitionNode::FunctionBits,
                                        ASTTypeFunction);
    assert(STE && "Unable to locate a valid SymbolTable Entry for "
                  "builtin function!");

    STE->SetContext(GCX);
    STE->SetGlobalScope();

    const ASTDeclarationContext* FCX =
      ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeFunction);
    assert(FCX && "Could not create a valid ASTDeclarationContext!");

    ASTDeclarationList PDL;
    ASTMPComplexList CXL;
    ASTStatementList SL;

    PDL.SetLocation(Loc);
    SL.SetLocation(Loc);

    ASTIdentifierNode* WId0 =
      ASTBuilder::Instance().CreateASTIdentifierNode("wf1",
                             OpenPulse::ASTOpenPulseWaveformNode::WaveformBits,
                             ASTTypeOpenPulseWaveform, FCX);
    assert(WId0 && "Could not create a valid Waveform ASTIdentifierNode!");

    WId0->SetDeclarationContext(FCX);
    WId0->SetLocalScope();
    WId0->SetLocation(Loc);

    OpenPulse::ASTOpenPulseWaveformNode* WFN0 =
      new OpenPulse::ASTOpenPulseWaveformNode(WId0, CXL);
    assert(WFN0 && "Could not create a valid OpenPulse Waveform!");

    WFN0->SetDeclarationContext(FCX);
    WFN0->SetLocation(Loc);
    WFN0->Mangle();

    ASTSymbolTableEntry* WSTE0 = WId0->GetSymbolTableEntry();
    assert(WSTE0 && "Could not obtain a valid ASTSymbolTableEntry!");

    WSTE0->SetContext(FCX);
    WSTE0->SetLocalScope();
    WSTE0->ResetValue();
    WSTE0->SetValue(new ASTValue<>(WFN0, WFN0->GetASTType()), WFN0->GetASTType());

    ASTDeclarationNode* WFD0 =
      new ASTDeclarationNode(WId0, WFN0, ASTTypeOpenPulseWaveform);
    assert(WFD0 && "Could not create a valid ASTDeclarationNode!");

    WFD0->SetDeclarationContext(FCX);
    WFD0->SetLocation(Loc);
    PDL.Append(WFD0);

    ASTIdentifierNode* WId1 =
      ASTBuilder::Instance().CreateASTIdentifierNode("wf2",
                             OpenPulse::ASTOpenPulseWaveformNode::WaveformBits,
                             ASTTypeOpenPulseWaveform, FCX);
    assert(WId1 && "Could not create a valid Waveform ASTIdentifierNode!");

    WId1->SetDeclarationContext(FCX);
    WId1->SetLocalScope();
    WId1->SetLocation(Loc);

    OpenPulse::ASTOpenPulseWaveformNode* WFN1 =
      new OpenPulse::ASTOpenPulseWaveformNode(WId1, CXL);
    assert(WFN1 && "Could not create a valid OpenPulse Waveform!");

    WFN1->SetDeclarationContext(FCX);
    WFN1->SetLocation(Loc);
    WFN1->Mangle();

    ASTSymbolTableEntry* WSTE1 = WId1->GetSymbolTableEntry();
    assert(WSTE1 && "Could not obtain a valid ASTSymbolTableEntry!");

    WSTE1->SetContext(FCX);
    WSTE1->SetLocalScope();
    WSTE1->ResetValue();
    WSTE1->SetValue(new ASTValue<>(WFN1, WFN1->GetASTType()), WFN1->GetASTType());

    ASTDeclarationNode* WFD1 =
      new ASTDeclarationNode(WId1, WFN1, ASTTypeOpenPulseWaveform);
    assert(WFD1 && "Could not create a valid ASTDeclarationNode!");

    WFD1->SetDeclarationContext(FCX);
    WFD1->SetLocation(Loc);
    PDL.Append(WFD1);

    ASTBinaryOpNode* BOP =
      new ASTBinaryOpNode(ASTIdentifierNode::BinaryOp.Clone(), WFN0, WFN1,
                          ASTOpTypeMul);
    assert(BOP && "Could not create a valid ASTBinaryOpNode!");

    BOP->SetDeclarationContext(FCX);
    BOP->SetLocation(Loc);
    BOP->Mangle();

    ASTStatementNode* SN = new ASTStatementNode(BOP->GetIdentifier(), BOP);
    assert(SN && "Could not create a valid ASTStatementNode!");

    SN->SetDeclarationContext(FCX);
    SL.Append(SN);

    OpenPulse::ASTOpenPulseWaveformNode* WFNR =
      new OpenPulse::ASTOpenPulseWaveformNode(ASTIdentifierNode::Waveform.Clone(),
                                              CXL);
    assert(WFNR && "Could not create a valid Result WaveformNode!");

    WFNR->SetDeclarationContext(FCX);
    WFNR->Mangle();

    ASTIdentifierNode* RId =
      ASTBuilder::Instance().CreateASTIdentifierNode("ast-result-builtin-mix",
                                      ASTResultNode::ResultBits,
                                      ASTTypeResult, FCX);
    assert(RId && "Could not create a valid ASTIdentifierNode!");

    ASTResultNode* RN = new ASTResultNode(RId, WFNR);
    assert(RN && "Could not create a valid ASTResultNode!");

    RN->SetDeclarationContext(FCX);
    RN->SetLocation(Loc);

    ASTSymbolTableEntry* RSTE = RId->GetSymbolTableEntry();
    assert(RSTE && "Could not obtain a valid ASTSymbolTableEntry!");

    RSTE->ResetValue();
    RSTE->SetValue(new ASTValue<>(RN, RN->GetASTType()), RN->GetASTType());

    PDL.SetDeclarationContext(FCX);
    SL.SetDeclarationContext(FCX);

    ASTFunctionDefinitionNode* FDN =
      ASTBuilder::Instance().CreateASTFunctionDefinition(Id, PDL, SL, RN);
    assert(FDN && "Could not create a valid ASTFunctionDefinitionNode!");

    RN->SetFunction(Id, FDN);
    FDN->Mangle();
    RN->Mangle();
    ASTDeclarationContextTracker::Instance().PopCurrentContext();
    ASTTypeSystemBuilder::Instance().RegisterFunction(Id->GetName());
    return FDN;
  } else if (Name == "sum") {
    ASTIdentifierNode* Id =
      ASTBuilder::Instance().CreateASTIdentifierNode(Name,
                             ASTFunctionDefinitionNode::FunctionBits,
                             ASTTypeFunction);
    assert(Id && "Could not create a valid builtin function ASTIdentifierNode!");

    Id->SetDeclarationContext(GCX);
    Id->SetGlobalScope();
    Id->SetLocation(Loc);

    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(Id,
                                 ASTFunctionDefinitionNode::FunctionBits,
                                 ASTTypeFunction);
    assert(STE && "Unable to locate a valid SymbolTable Entry for "
                  "builtin function!");

    STE->SetContext(GCX);
    STE->SetGlobalScope();

    const ASTDeclarationContext* FCX =
      ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeFunction);
    assert(FCX && "Could not create a valid ASTDeclarationContext!");

    ASTDeclarationList PDL;
    ASTMPComplexList CXL;
    ASTStatementList SL;

    PDL.SetLocation(Loc);
    SL.SetLocation(Loc);

    ASTIdentifierNode* WId0 =
      ASTBuilder::Instance().CreateASTIdentifierNode("wf1",
                             OpenPulse::ASTOpenPulseWaveformNode::WaveformBits,
                             ASTTypeOpenPulseWaveform, FCX);
    assert(WId0 && "Could not create a valid Waveform ASTIdentifierNode!");

    WId0->SetDeclarationContext(FCX);
    WId0->SetLocalScope();
    WId0->SetLocation(Loc);

    OpenPulse::ASTOpenPulseWaveformNode* WFN0 =
      new OpenPulse::ASTOpenPulseWaveformNode(WId0, CXL);
    assert(WFN0 && "Could not create a valid OpenPulse Waveform!");

    WFN0->SetDeclarationContext(FCX);
    WFN0->SetLocation(Loc);
    WFN0->Mangle();

    ASTSymbolTableEntry* WSTE0 = WId0->GetSymbolTableEntry();
    assert(WSTE0 && "Could not obtain a valid ASTSymbolTableEntry!");

    WSTE0->SetContext(FCX);
    WSTE0->SetLocalScope();
    WSTE0->ResetValue();
    WSTE0->SetValue(new ASTValue<>(WFN0, WFN0->GetASTType()), WFN0->GetASTType());

    ASTDeclarationNode* WFD0 =
      new ASTDeclarationNode(WId0, WFN0, ASTTypeOpenPulseWaveform);
    assert(WFD0 && "Could not create a valid ASTDeclarationNode!");

    WFD0->SetDeclarationContext(FCX);
    WFD0->SetLocation(Loc);
    PDL.Append(WFD0);

    ASTIdentifierNode* WId1 =
      ASTBuilder::Instance().CreateASTIdentifierNode("wf2",
                             OpenPulse::ASTOpenPulseWaveformNode::WaveformBits,
                             ASTTypeOpenPulseWaveform, FCX);
    assert(WId1 && "Could not create a valid Waveform ASTIdentifierNode!");

    WId1->SetDeclarationContext(FCX);
    WId1->SetLocalScope();
    WId1->SetLocation(Loc);

    OpenPulse::ASTOpenPulseWaveformNode* WFN1 =
      new OpenPulse::ASTOpenPulseWaveformNode(WId1, CXL);
    assert(WFN1 && "Could not create a valid OpenPulse Waveform!");

    WFN1->SetDeclarationContext(FCX);
    WFN1->SetLocation(Loc);
    WFN1->Mangle();

    ASTSymbolTableEntry* WSTE1 = WId1->GetSymbolTableEntry();
    assert(WSTE1 && "Could not obtain a valid ASTSymbolTableEntry!");

    WSTE1->SetContext(FCX);
    WSTE1->SetLocalScope();
    WSTE1->ResetValue();
    WSTE1->SetValue(new ASTValue<>(WFN1, WFN1->GetASTType()), WFN1->GetASTType());

    ASTDeclarationNode* WFD1 =
      new ASTDeclarationNode(WId1, WFN1, ASTTypeOpenPulseWaveform);
    assert(WFD1 && "Could not create a valid ASTDeclarationNode!");

    WFD1->SetDeclarationContext(FCX);
    WFD1->SetLocation(Loc);
    PDL.Append(WFD1);

    ASTBinaryOpNode* BOP =
      new ASTBinaryOpNode(ASTIdentifierNode::BinaryOp.Clone(),
                          WFN0, WFN1, ASTOpTypeAdd);
    assert(BOP && "Could not create a valid ASTBinaryOpNode!");

    BOP->SetDeclarationContext(FCX);
    BOP->SetLocation(Loc);
    BOP->Mangle();

    ASTStatementNode* SN = new ASTStatementNode(BOP->GetIdentifier(), BOP);
    assert(SN && "Could not create a valid ASTStatementNode!");

    SN->SetDeclarationContext(FCX);
    SL.Append(SN);

    OpenPulse::ASTOpenPulseWaveformNode* WFNR =
      new OpenPulse::ASTOpenPulseWaveformNode(ASTIdentifierNode::Waveform.Clone(),
                                              CXL);
    assert(WFNR && "Could not create a valid Result WaveformNode!");

    WFNR->SetDeclarationContext(FCX);
    WFNR->Mangle();

    ASTIdentifierNode* RId =
      ASTBuilder::Instance().CreateASTIdentifierNode("ast-result-builtin-sum",
                                                     ASTResultNode::ResultBits,
                                                     ASTTypeResult, FCX);
    assert(RId && "Could not create a valid ASTIdentifierNode!");

    RId->SetDeclarationContext(FCX);
    RId->SetLocalScope();
    RId->SetLocation(Loc);

    ASTResultNode* RN = new ASTResultNode(RId, WFNR);
    assert(RN && "Could not create a valid ASTResultNode!");

    RN->SetDeclarationContext(FCX);
    RN->SetLocation(Loc);

    ASTSymbolTableEntry* RSTE = RId->GetSymbolTableEntry();
    assert(RSTE && "Could not obtain a valid ASTSymbolTableEntry!");

    RSTE->ResetValue();
    RSTE->SetValue(new ASTValue<>(RN, RN->GetASTType()), RN->GetASTType());

    PDL.SetDeclarationContext(FCX);
    SL.SetDeclarationContext(FCX);

    ASTFunctionDefinitionNode* FDN =
      ASTBuilder::Instance().CreateASTFunctionDefinition(Id, PDL, SL, RN);
    assert(FDN && "Could not create a valid ASTFunctionDefinitionNode!");

    RN->SetFunction(Id, FDN);
    FDN->Mangle();
    RN->Mangle();
    ASTDeclarationContextTracker::Instance().PopCurrentContext();
    ASTTypeSystemBuilder::Instance().RegisterFunction(Id->GetName());
    return FDN;
  } else if (Name == "phase_shift") {
    ASTIdentifierNode* Id =
      ASTBuilder::Instance().CreateASTIdentifierNode(Name,
                             ASTFunctionDefinitionNode::FunctionBits,
                             ASTTypeFunction);
    assert(Id && "Could not create a valid builtin function ASTIdentifierNode!");

    Id->SetDeclarationContext(GCX);
    Id->SetGlobalScope();
    Id->SetLocation(Loc);

    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(Id,
                                 ASTFunctionDefinitionNode::FunctionBits,
                                 ASTTypeFunction);
    assert(STE && "Unable to locate a valid SymbolTable Entry for "
                  "builtin function!");

    STE->SetContext(GCX);
    STE->SetGlobalScope();

    const ASTDeclarationContext* FCX =
      ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeFunction);
    assert(FCX && "Could not create a valid ASTDeclarationContext!");

    ASTDeclarationList PDL;
    ASTMPComplexList CXL;
    ASTStatementList SL;

    PDL.SetLocation(Loc);
    SL.SetLocation(Loc);

    ASTIdentifierNode* WId =
      ASTBuilder::Instance().CreateASTIdentifierNode("wf",
                             OpenPulse::ASTOpenPulseWaveformNode::WaveformBits,
                             ASTTypeOpenPulseWaveform, FCX);
    assert(WId && "Could not create a valid Waveform ASTIdentifierNode!");

    WId->SetDeclarationContext(FCX);
    WId->SetLocalScope();
    WId->SetLocation(Loc);

    OpenPulse::ASTOpenPulseWaveformNode* WFN =
      new OpenPulse::ASTOpenPulseWaveformNode(WId, CXL);
    assert(WFN && "Could not create a valid OpenPulse Waveform!");

    WFN->SetDeclarationContext(FCX);
    WFN->SetLocation(Loc);
    WFN->Mangle();

    ASTSymbolTableEntry* WSTE = WId->GetSymbolTableEntry();
    assert(WSTE && "Could not obtain a valid ASTSymbolTableEntry!");

    WSTE->SetContext(FCX);
    WSTE->SetLocalScope();
    WSTE->ResetValue();
    WSTE->SetValue(new ASTValue<>(WFN, WFN->GetASTType()), WFN->GetASTType());

    ASTDeclarationNode* WFD =
      new ASTDeclarationNode(WId, WFN, ASTTypeOpenPulseWaveform);
    assert(WFD && "Could not create a valid ASTDeclarationNode!");

    WFD->SetDeclarationContext(FCX);
    WFD->SetLocation(Loc);

    PDL.Append(WFD);

    ASTIdentifierNode* AId =
      ASTBuilder::Instance().CreateASTIdentifierNode("ang",
                             ASTAngleNode::AngleBits, ASTTypeAngle, FCX);
    assert(AId && "Could not create a valid Angle ASTIdentifierNode!");

    AId->SetLocation(Loc);
    AId->SetDeclarationContext(FCX);
    AId->SetLocalScope();

    ASTAngleType ATy = ASTAngleNode::DetermineAngleType("ang");
    ASTAngleNode* AN = new ASTAngleNode(AId, ATy, ASTAngleNode::AngleBits);
    assert(AN && "Could not create a valid ASTAngleNode!");

    AN->SetLocation(Loc);
    AN->SetDeclarationContext(FCX);
    AN->Mangle();

    ASTSymbolTableEntry* ASTE = AId->GetSymbolTableEntry();
    assert(ASTE && "Could not obtain a valid ASTSymbolTableEntry!");

    ASTE->SetContext(FCX);
    ASTE->SetLocalScope();
    ASTE->ResetValue();
    ASTE->SetValue(new ASTValue<>(AN, ASTTypeAngle), ASTTypeAngle);

    ASTDeclarationNode* ADN = new ASTDeclarationNode(AId, AN, ASTTypeAngle);
    assert(ADN && "Could not create a valid ASTDeclarationNode!");

    ADN->SetDeclarationContext(FCX);
    ADN->SetLocation(Loc);

    PDL.Append(ADN);

    // FIXME:
    // This needs to be clarified. The OpenPulse Spec is 100% vague.
    ASTBinaryOpNode* BOP =
      new ASTBinaryOpNode(ASTIdentifierNode::BinaryOp.Clone(), WFN, AN,
                          ASTOpTypeMul);
    assert(BOP && "Could not create a valid ASTBinaryOpNode!");

    BOP->SetDeclarationContext(FCX);
    BOP->SetLocation(Loc);

    ASTStatementNode* SN = new ASTStatementNode(BOP->GetIdentifier(), BOP);
    assert(SN && "Could not create a valid ASTStatementNode!");

    SN->SetDeclarationContext(FCX);
    SN->SetLocation(Loc);
    SL.Append(SN);

    OpenPulse::ASTOpenPulseWaveformNode* WFNR =
      new OpenPulse::ASTOpenPulseWaveformNode(ASTIdentifierNode::Waveform.Clone(),
                                              CXL);
    assert(WFNR && "Could not create a valid OpenPulse Result Waveform!");

    WFNR->SetDeclarationContext(FCX);
    WFNR->SetLocation(Loc);
    WFNR->Mangle();

    ASTIdentifierNode* RId =
      ASTBuilder::Instance().CreateASTIdentifierNode("ast-result-builtin-phase_shift",
                                      ASTResultNode::ResultBits,
                                      ASTTypeResult, FCX);
    assert(RId && "Could not create a valid ASTIdentifierNode!");

    RId->SetDeclarationContext(FCX);
    RId->SetLocalScope();
    RId->SetLocation(Loc);

    ASTResultNode* RN = new ASTResultNode(RId, WFNR);
    assert(RN && "Could not create a valid ASTResultNode!");

    RN->SetDeclarationContext(FCX);
    RN->SetLocation(Loc);

    ASTSymbolTableEntry* RSTE = RId->GetSymbolTableEntry();
    assert(RSTE && "Could not obtain a valid ASTSymbolTableEntry!");

    RSTE->SetContext(FCX);
    RSTE->ResetValue();
    RSTE->SetValue(new ASTValue<>(RN, RN->GetASTType()), RN->GetASTType());

    ASTFunctionDefinitionNode* FDN =
      ASTBuilder::Instance().CreateASTFunctionDefinition(Id, PDL, SL, RN);
    assert(FDN && "Could not create a valid ASTFunctionDefinitionNode!");

    RN->SetFunction(Id, FDN);
    FDN->Mangle();
    RN->Mangle();
    ASTDeclarationContextTracker::Instance().PopCurrentContext();
    ASTTypeSystemBuilder::Instance().RegisterFunction(Id->GetName());
    return FDN;
  } else if (Name == "scale") {
    ASTIdentifierNode* Id =
      ASTBuilder::Instance().CreateASTIdentifierNode(Name,
                             ASTFunctionDefinitionNode::FunctionBits,
                             ASTTypeFunction);
    assert(Id && "Could not create a valid builtin function ASTIdentifierNode!");

    Id->SetDeclarationContext(GCX);
    Id->SetGlobalScope();
    Id->SetLocation(Loc);

    ASTSymbolTableEntry* STE =
      ASTSymbolTable::Instance().Lookup(Id, ASTFunctionDefinitionNode::FunctionBits,
                                        ASTTypeFunction);
    assert(STE && "Unable to locate a valid SymbolTable Entry for "
                  "builtin function!");

    STE->SetContext(GCX);
    STE->SetGlobalScope();

    const ASTDeclarationContext* FCX =
      ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeFunction);
    assert(FCX && "Could not create a valid ASTDeclarationContext!");

    ASTDeclarationList PDL;
    ASTMPComplexList CXL;
    ASTStatementList SL;

    PDL.SetLocation(Loc);
    SL.SetLocation(Loc);

    ASTIdentifierNode* WId =
      ASTBuilder::Instance().CreateASTIdentifierNode("wf",
                             OpenPulse::ASTOpenPulseWaveformNode::WaveformBits,
                             ASTTypeOpenPulseWaveform, FCX);
    assert(WId && "Could not create a valid Waveform ASTIdentifierNode!");

    WId->SetDeclarationContext(FCX);
    WId->SetLocalScope();
    WId->SetLocation(Loc);

    OpenPulse::ASTOpenPulseWaveformNode* WFN =
      new OpenPulse::ASTOpenPulseWaveformNode(WId, CXL);
    assert(WFN && "Could not create a valid OpenPulse Waveform!");

    WFN->SetDeclarationContext(FCX);
    WFN->SetLocation(Loc);
    WFN->Mangle();

    ASTSymbolTableEntry* WSTE = WId->GetSymbolTableEntry();
    assert(WSTE && "Could not obtain a valid ASTSymbolTableEntry!");

    WSTE->SetContext(FCX);
    WSTE->SetLocalScope();
    WSTE->ResetValue();
    WSTE->SetValue(new ASTValue<>(WFN, WFN->GetASTType()), WFN->GetASTType());

    ASTDeclarationNode* WFD =
      new ASTDeclarationNode(WId, WFN, ASTTypeOpenPulseWaveform);
    assert(WFD && "Could not create a valid ASTDeclarationNode!");

    WFD->SetDeclarationContext(FCX);
    WFD->SetLocation(Loc);

    PDL.Append(WFD);

    ASTIdentifierNode* DId =
      ASTBuilder::Instance().CreateASTIdentifierNode("factor",
                             ASTDoubleNode::DoubleBits, ASTTypeMPDecimal, FCX);
    assert(DId && "Could not create a valid MPDecimal ASTIdentifierNode!");

    DId->SetLocation(Loc);
    DId->SetDeclarationContext(FCX);
    DId->SetLocalScope();

    ASTMPDecimalNode* MPD = new ASTMPDecimalNode(DId, ASTDoubleNode::DoubleBits,
                                                 static_cast<double>(1.0));
    assert(MPD && "Could not create a valid ASTMPDecimalNode!");

    MPD->SetLocation(Loc);
    MPD->SetDeclarationContext(FCX);
    MPD->Mangle();

    ASTSymbolTableEntry* DSTE = DId->GetSymbolTableEntry();
    assert(DSTE && "Could not obtain a valid ASTSymbolTableEntry!");

    DSTE->SetContext(FCX);
    DSTE->SetLocalScope();
    DSTE->ResetValue();
    DSTE->SetValue(new ASTValue<>(MPD, ASTTypeMPDecimal), ASTTypeMPDecimal);

    ASTDeclarationNode* MPDN = new ASTDeclarationNode(DId, MPD, ASTTypeMPDecimal);
    assert(MPDN && "Could not create a valid ASTDeclarationNode!");

    MPDN->SetDeclarationContext(FCX);
    MPDN->SetLocation(Loc);

    PDL.Append(MPDN);

    // FIXME:
    // This needs to be clarified. We assume multiplication.
    ASTBinaryOpNode* BOP =
      new ASTBinaryOpNode(ASTIdentifierNode::BinaryOp.Clone(), WFN, MPD,
                          ASTOpTypeMul);
    assert(BOP && "Could not create a valid ASTBinaryOpNode!");

    BOP->SetDeclarationContext(FCX);
    BOP->SetLocation(Loc);

    ASTStatementNode* SN = new ASTStatementNode(&ASTIdentifierNode::BinaryOp,
                                                BOP);
    assert(SN && "Could not create a valid ASTStatementNode!");

    SN->SetDeclarationContext(FCX);
    SN->SetLocation(Loc);
    SL.Append(SN);

    OpenPulse::ASTOpenPulseWaveformNode* WFNR =
      new OpenPulse::ASTOpenPulseWaveformNode(ASTIdentifierNode::Waveform.Clone(),
                                              CXL);
    assert(WFNR && "Could not create a valid OpenPulse Result Waveform!");

    WFNR->SetDeclarationContext(FCX);
    WFNR->SetLocation(Loc);
    WFNR->Mangle();

    ASTIdentifierNode* RId =
      ASTBuilder::Instance().CreateASTIdentifierNode("ast-result-builtin-scale",
                                                     ASTResultNode::ResultBits,
                                                     ASTTypeResult, FCX);
    assert(RId && "Could not create a valid ASTIdentifierNode!");

    RId->SetDeclarationContext(FCX);
    RId->SetLocalScope();
    RId->SetLocation(Loc);

    ASTResultNode* RN = new ASTResultNode(RId, WFNR);
    assert(RN && "Could not create a valid ASTResultNode!");

    RN->SetDeclarationContext(FCX);
    RN->SetLocation(Loc);

    ASTSymbolTableEntry* RSTE = RId->GetSymbolTableEntry();
    assert(RSTE && "Could not obtain a valid ASTSymbolTableEntry!");

    RSTE->SetContext(FCX);
    RSTE->ResetValue();
    RSTE->SetValue(new ASTValue<>(RN, RN->GetASTType()), RN->GetASTType());

    ASTFunctionDefinitionNode* FDN =
      ASTBuilder::Instance().CreateASTFunctionDefinition(Id, PDL, SL, RN);
    assert(FDN && "Could not create a valid ASTFunctionDefinitionNode!");

    RN->SetFunction(Id, FDN);
    FDN->Mangle();
    RN->Mangle();
    ASTDeclarationContextTracker::Instance().PopCurrentContext();
    ASTTypeSystemBuilder::Instance().RegisterFunction(Id->GetName());
    return FDN;
  }

  return nullptr;
}

bool ASTBuiltinFunctionsBuilder::AddBuiltinFunction(ASTFunctionDefinitionNode* F) {
  assert(F && "Invalid ASTFunctionDefinitionNode argument!");

  F->SetIsBuiltin(true);

  if (FDEM.insert(std::make_pair(F->GetName(), F)).second) {
    ASTFunctionDeclarationNode* FDC =
      ASTBuilder::Instance().CreateASTFunctionDeclaration(F->GetIdentifier(), F);
    assert(FDC && "Could not create a valid ASTFunctionDeclarationNode!");

    return FDCM.insert(std::make_pair(F->GetName(), FDC)).second;
  }

  return false;
}

void ASTBuiltinFunctionsBuilder::Init() {
  if (!IsInit) {
    std::stringstream M;

    if (ASTFunctionDefinitionNode* FDN = CreateBuiltinFunction("mix")) {
      AddBuiltinFunction(FDN);
    } else {
      M << "Failure creating OpenPulse builtin 'mix'.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
    }

    if (ASTFunctionDefinitionNode* FDN = CreateBuiltinFunction("sum")) {
      AddBuiltinFunction(FDN);
    } else {
      M << "Failure creating OpenPulse builtin 'sum'.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
    }

    if (ASTFunctionDefinitionNode* FDN = CreateBuiltinFunction("phase_shift")) {
      AddBuiltinFunction(FDN);
    } else {
      M << "Failure creating OpenPulse builtin 'phase_shift'.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
    }

    if (ASTFunctionDefinitionNode* FDN = CreateBuiltinFunction("scale")) {
      AddBuiltinFunction(FDN);
    } else {
      M << "Failure creating OpenPulse builtin 'scale'.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
    }

    IsInit = true;
  }
}

} // namespace QASM

