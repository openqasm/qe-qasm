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

#ifndef __QASM_AST_OPENPULSE_WAVEFORM_H
#define __QASM_AST_OPENPULSE_WAVEFORM_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTDuration.h>
#include <qasm/AST/ASTMPComplexList.h>

#include <iostream>
#include <vector>
#include <cassert>

namespace QASM {

class ASTResultNode;
class ASTFunctionCallNode;

namespace OpenPulse {

class ASTOpenPulseWaveformNode : public ASTExpressionNode {
private:
  const ASTMPComplexNode* AMP;
  const ASTDurationNode* D;
  const ASTDurationNode* SQW;
  const ASTDurationNode* SIG;
  const ASTMPDecimalNode* BTA;
  const ASTMPDecimalNode* FRQ;
  const ASTAngleNode* PHS;
  const ASTResultNode* FR;
  const ASTFunctionCallNode* FC;
  ASTMPComplexList CXV;
  unsigned TE;

private:
  ASTOpenPulseWaveformNode() = delete;

protected:
  ASTOpenPulseWaveformNode(const ASTIdentifierNode* Id, const std::string& EM)
  : ASTExpressionNode(Id, new ASTStringNode(EM), ASTTypeExpressionError),
  AMP(nullptr), D(nullptr), SQW(nullptr), SIG(nullptr),
  BTA(nullptr), FRQ(nullptr), PHS(nullptr), FR(nullptr),
  FC(nullptr), CXV(), TE(static_cast<unsigned>(~0x0)) { }

public:
  static const unsigned WaveformBits = 64U;

public:
  ASTOpenPulseWaveformNode(const ASTIdentifierNode* Id,
                           const ASTMPComplexList& CV)
  : ASTExpressionNode(Id, ASTTypeOpenPulseWaveform),
  AMP(nullptr), D(nullptr), SQW(nullptr), SIG(nullptr),
  BTA(nullptr), FRQ(nullptr), PHS(nullptr), FR(nullptr),
  FC(nullptr), CXV(CV), TE(0U) { }

  ASTOpenPulseWaveformNode(const ASTIdentifierNode* Id,
                           const ASTMPComplexNode* AMPC,
                           const ASTDurationNode* DR,
                           const ASTDurationNode* SGM)
  : ASTExpressionNode(Id, ASTTypeOpenPulseWaveform),
  AMP(AMPC), D(DR), SQW(nullptr), SIG(SGM), BTA(nullptr),
  FRQ(nullptr), PHS(nullptr), FR(nullptr), FC(nullptr),
  CXV(), TE(1U) { }

  ASTOpenPulseWaveformNode(const ASTIdentifierNode* Id,
                           const ASTMPComplexNode* AMPC,
                           const ASTDurationNode* DR,
                           const ASTDurationNode* SW,
                           const ASTDurationNode* SGM)
  : ASTExpressionNode(Id, ASTTypeOpenPulseWaveform),
  AMP(AMPC), D(DR), SQW(SW), SIG(SGM), BTA(nullptr),
  FRQ(nullptr), PHS(nullptr), FR(nullptr), FC(nullptr),
  CXV(), TE(2U) { }

  ASTOpenPulseWaveformNode(const ASTIdentifierNode* Id,
                           const ASTMPComplexNode* AMPC,
                           const ASTDurationNode* DR,
                           const ASTMPDecimalNode* BT)
  : ASTExpressionNode(Id, ASTTypeOpenPulseWaveform),
  AMP(AMPC), D(DR), SQW(nullptr), SIG(nullptr), BTA(BT),
  FRQ(nullptr), PHS(nullptr), FR(nullptr), FC(nullptr),
  CXV(), TE(3U) { }

  ASTOpenPulseWaveformNode(const ASTIdentifierNode* Id,
                           const ASTMPComplexNode* AMPC,
                           const ASTDurationNode* DR)
  : ASTExpressionNode(Id, ASTTypeOpenPulseWaveform),
  AMP(AMPC), D(DR), SQW(nullptr), SIG(nullptr), BTA(nullptr),
  FRQ(nullptr), PHS(nullptr), FR(nullptr), FC(nullptr),
  CXV(), TE(4U) { }

  ASTOpenPulseWaveformNode(const ASTIdentifierNode* Id,
                           const ASTMPComplexNode* AMPC,
                           const ASTDurationNode* DR,
                           const ASTMPDecimalNode* FQ,
                           const ASTAngleNode* PH)
  : ASTExpressionNode(Id, ASTTypeOpenPulseWaveform),
  AMP(AMPC), D(DR), SQW(nullptr), SIG(nullptr), BTA(nullptr),
  FRQ(FQ), PHS(PH), FR(nullptr), FC(nullptr), CXV(), TE(5U) { }

  virtual ~ASTOpenPulseWaveformNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeOpenPulseWaveform;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual const ASTIdentifierNode* GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string& GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual void SetAmplitude(const ASTMPComplexNode* V) {
    AMP = V;
  }

  virtual void SetDuration(const ASTDurationNode* V) {
    D = V;
  }

  virtual void SetSquareWidth(const ASTDurationNode* V) {
    SQW = V;
  }

  virtual void SetSigma(const ASTDurationNode* V) {
    SIG = V;
  }

  virtual void SetBeta(const ASTMPDecimalNode* V) {
    BTA = V;
  }

  virtual void SetFrequency(const ASTMPDecimalNode* V) {
    FRQ = V;
  }

  virtual void SetPhase(const ASTAngleNode* V) {
    PHS = V;
  }

  virtual void SetSamples(const ASTMPComplexList& CV) {
    CXV = CV;
  }

  virtual void SetFunctionResult(const ASTResultNode* RN) {
    FR = RN;
  }

  virtual void SetFunctionCall(const ASTFunctionCallNode* F) {
    FC = F;
  }

  virtual const ASTMPComplexNode* GetAmplitude() const {
    return AMP;
  }

  virtual const ASTDurationNode* GetDuration() const {
    return D;
  }

  virtual const ASTDurationNode* GetSquareWidth() const {
    return SQW;
  }

  virtual const ASTDurationNode* GetSigma() const {
    return SIG;
  }

  virtual const ASTMPDecimalNode* GetBeta() const {
    return BTA;
  }

  virtual const ASTMPDecimalNode* GetFrequency() const {
    return FRQ;
  }

  virtual const ASTAngleNode* GetPhase() const {
    return PHS;
  }

  virtual const ASTMPComplexList& GetSamples() const {
    return CXV;
  }

  virtual const ASTMPComplexNode* GetComplex(unsigned Index) const {
    return CXV.GetComplex(Index);
  }

  virtual const ASTResultNode* GetFunctionResult() const {
    return FR;
  }

  virtual const ASTFunctionCallNode* GetFunctionCall() const {
    return FC;
  }

  virtual bool IsFunctionCallResult() const {
    return FR != nullptr;
  }

  virtual bool IsFunctionCall() {
    return FC != nullptr;
  }

  static ASTOpenPulseWaveformNode* ExpressionError(const ASTIdentifierNode* Id,
                                                   const std::string& ERM) {
    return new ASTOpenPulseWaveformNode(Id, ERM);
  }

  virtual bool IsError() const override {
    return ASTExpressionNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTExpressionNode::GetError();
  }

  virtual void print() const override;
};

} // namespace OpenPulse
} // namespace QASM

#endif // __QASM_AST_OPENPULSE_WAVEFORM_H

