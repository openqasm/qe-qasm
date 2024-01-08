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

#ifndef __QASM_AST_OPENPULSE_PLAY_H
#define __QASM_AST_OPENPULSE_PLAY_H

#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseFrame.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseWaveform.h>

#include <cassert>
#include <iostream>

namespace QASM {
namespace OpenPulse {

class ASTOpenPulsePlayNode : public ASTExpressionNode {
private:
  const ASTOpenPulseWaveformNode *WN;
  const ASTFunctionCallNode *FC;
  const ASTOpenPulseFrameNode *FN;

private:
  ASTOpenPulsePlayNode() = delete;

protected:
  ASTOpenPulsePlayNode(const ASTIdentifierNode *Id, const std::string &EM)
      : ASTExpressionNode(Id, new ASTStringNode(EM), ASTTypeExpressionError),
        WN(nullptr), FC(nullptr), FN(nullptr) {}

public:
  static const unsigned PlayBits = 64U;

public:
  ASTOpenPulsePlayNode(const ASTIdentifierNode *Id,
                       const ASTOpenPulseWaveformNode *W,
                       const ASTOpenPulseFrameNode *F)
      : ASTExpressionNode(Id, ASTTypeOpenPulsePlay), WN(W), FC(nullptr), FN(F) {
    assert(F && "Invalid ASTOpenPulseFrameNode argument!");
  }

  ASTOpenPulsePlayNode(const ASTIdentifierNode *Id,
                       const ASTFunctionCallNode *C,
                       const ASTOpenPulseFrameNode *F)
      : ASTExpressionNode(Id, ASTTypeOpenPulsePlay), WN(nullptr), FC(C), FN(F) {
    assert(F && "Invalid ASTOpenPulseFrameNode argument!");
  }

  ASTOpenPulsePlayNode(const ASTIdentifierNode *Id, const ASTMPComplexList &CXL,
                       const ASTOpenPulseFrameNode *F)
      : ASTExpressionNode(Id, ASTTypeOpenPulsePlay), WN(nullptr), FC(nullptr),
        FN(F) {
    assert(F && "Invalid ASTOpenPulseFrameNode argument!");
    WN = new ASTOpenPulseWaveformNode(&ASTIdentifierNode::Waveform, CXL);
    assert(WN && "Could not create a valid OpenPulse WaveformNode!");
  }

  virtual ~ASTOpenPulsePlayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeOpenPulsePlay; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual bool HasFunctionCall() const { return FC != nullptr; }

  virtual bool HasWaveform() const { return WN != nullptr; }

  const ASTOpenPulseWaveformNode *GetWaveform() { return WN; }

  const ASTOpenPulseWaveformNode *GetWaveform() const { return WN; }

  const ASTFunctionCallNode *GetFunctionCall() { return FC; }

  const ASTFunctionCallNode *GetFunctionCall() const { return FC; }

  const ASTOpenPulseFrameNode *GetFrame() { return FN; }

  const ASTOpenPulseFrameNode *GetFrame() const { return FN; }

  static ASTOpenPulsePlayNode *ExpressionError(const ASTIdentifierNode *Id,
                                               const std::string &ERM) {
    return new ASTOpenPulsePlayNode(Id, ERM);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  virtual void print() const override {
    std::cout << "<OpenPulsePlay>" << std::endl;
    if (WN)
      WN->print();
    if (FC)
      FC->print();
    if (FN)
      FN->print();
    std::cout << "</OpenPulsePlay>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTOpenPulsePlayStatementNode : public ASTStatementNode {
private:
  ASTOpenPulsePlayStatementNode() = delete;

protected:
  ASTOpenPulsePlayStatementNode(const ASTIdentifierNode *Id,
                                const std::string &ERM)
      : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)) {}

public:
  ASTOpenPulsePlayStatementNode(const ASTOpenPulsePlayNode *PN)
      : ASTStatementNode(PN->GetIdentifier(), PN) {}

  virtual ~ASTOpenPulsePlayStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeOpenPulsePlayStatement;
  }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual const ASTOpenPulsePlayNode *GetPlayNode() const {
    return dynamic_cast<const ASTOpenPulsePlayNode *>(
        ASTStatementNode::GetExpression());
  }

  virtual bool IsError() const override {
    if (const ASTExpressionNode *EN =
            dynamic_cast<const ASTExpressionNode *>(Expr)) {
      if (dynamic_cast<const ASTStringNode *>(EN))
        return true;
    }

    return false;
  }

  virtual const std::string &GetError() const override {
    if (const ASTExpressionNode *EN =
            dynamic_cast<const ASTExpressionNode *>(Expr)) {
      if (dynamic_cast<const ASTStringNode *>(EN))
        return ASTStatementNode::GetError();
    }

    return ASTStringUtils::Instance().EmptyString();
  }

  static ASTOpenPulsePlayStatementNode *
  StatementError(const ASTIdentifierNode *Id, const std::string &ERM) {
    return new ASTOpenPulsePlayStatementNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<OpenPulsePlayStatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</OpenPulsePlayStatement>" << std::endl;
  }

  virtual void push(ASTBase * /* unused*/) override {}
};

} // namespace OpenPulse
} // namespace QASM

#endif // __QASM_AST_OPENPULSE_PLAY_H
