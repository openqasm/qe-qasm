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

#ifndef __QASM_AST_OPENPULSE_FRAME_H
#define __QASM_AST_OPENPULSE_FRAME_H

#include <qasm/AST/ASTDuration.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/OpenPulse/ASTOpenPulsePort.h>

#include <cassert>
#include <iostream>

namespace QASM {
namespace OpenPulse {

class ASTOpenPulseFrameNode : public ASTExpressionNode {
private:
  const ASTOpenPulsePortNode *PRT;
  const ASTMPDecimalNode *FRQ;
  const ASTAngleNode *PHS;
  mutable const ASTDurationNode *DUR;
  bool EXT;

private:
  ASTOpenPulseFrameNode() = delete;

protected:
  ASTOpenPulseFrameNode(const ASTIdentifierNode *Id, const std::string &EM)
      : ASTExpressionNode(Id, new ASTStringNode(EM), ASTTypeExpressionError),
        PRT(nullptr), FRQ(nullptr), PHS(nullptr), DUR(nullptr), EXT(false) {}

public:
  static const unsigned FrameBits = 64U;

public:
  ASTOpenPulseFrameNode(const ASTIdentifierNode *Id)
      : ASTExpressionNode(Id, ASTTypeOpenPulseFrame), PRT(nullptr),
        FRQ(nullptr), PHS(nullptr), DUR(nullptr), EXT(false) {}

  ASTOpenPulseFrameNode(const ASTIdentifierNode *Id,
                        const ASTOpenPulsePortNode *PN,
                        const ASTMPDecimalNode *FR, const ASTAngleNode *PH,
                        bool Extern = false)
      : ASTExpressionNode(Id, ASTTypeOpenPulseFrame), PRT(PN), FRQ(FR), PHS(PH),
        DUR(nullptr), EXT(Extern) {}

  ASTOpenPulseFrameNode(const ASTIdentifierNode *Id,
                        const ASTOpenPulsePortNode *PN,
                        const ASTMPDecimalNode *FR, const ASTAngleNode *PH,
                        const ASTDurationNode *DN, bool Extern = false)
      : ASTExpressionNode(Id, ASTTypeOpenPulseFrame), PRT(PN), FRQ(FR), PHS(PH),
        DUR(DN), EXT(Extern) {}

  virtual ~ASTOpenPulseFrameNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeOpenPulseFrame; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual bool IsAggregate() const override { return true; }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::GetIdentifier()->GetName();
  }

  const ASTOpenPulsePortNode *GetPort() const { return PRT; }

  const ASTMPDecimalNode *GetFrequency() const { return FRQ; }

  const ASTAngleNode *GetPhase() const { return PHS; }

  const ASTDurationNode *GetDuration() const { return DUR; }

  ASTDurationNode *GetDuration() { return const_cast<ASTDurationNode *>(DUR); }

  void SetPhase(const ASTAngleNode *PH) { PHS = PH; }

  void SetFrequency(const ASTMPDecimalNode *MPD) { FRQ = MPD; }

  void SetDuration(const ASTDurationNode *DN) { DUR = DN; }

  void SetExtern(bool EV) { EXT = EV; }

  bool IsExtern() const { return EXT; }

  virtual unsigned GetBits() const { return FrameBits; }

  static ASTOpenPulseFrameNode *ExpressionError(const ASTIdentifierNode *Id,
                                                const std::string &ERM) {
    return new ASTOpenPulseFrameNode(Id, ERM);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  virtual void print() const override {
    std::cout << "<OpenPulseFrame>" << std::endl;
    ASTExpressionNode::print();

    if (PRT) {
      std::cout << "<Port>" << std::endl;
      PRT->print();
      std::cout << "</Port>" << std::endl;
    }

    if (FRQ) {
      std::cout << "<Frequency>" << std::endl;
      FRQ->print();
      std::cout << "</Frequency>" << std::endl;
    }

    if (PHS) {
      std::cout << "<Phase>" << std::endl;
      PHS->print();
      std::cout << "</Phase>" << std::endl;
    }

    if (DUR) {
      std::cout << "<Duration>" << std::endl;
      DUR->print();
      std::cout << "</Duration>" << std::endl;
    }

    std::cout << "<IsExtern>" << std::boolalpha << EXT << "</IsExtern>"
              << std::endl;
    std::cout << "</OpenPulseFrame>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTOpenPulseFrameNodeResolver {
private:
  static ASTOpenPulseFrameNodeResolver FRN;

protected:
  ASTOpenPulseFrameNodeResolver() = default;

public:
  static ASTOpenPulseFrameNodeResolver &Instance() { return FRN; }

  ~ASTOpenPulseFrameNodeResolver() = default;

  ASTAngleNode *ResolveAngle(ASTExpressionNode *E);

  ASTMPDecimalNode *ResolveFrequency(const ASTIdentifierNode *Id);

  ASTMPDecimalNode *ResolveFrequency(const ASTIdentifierRefNode *IdR);

  ASTMPDecimalNode *ResolveFrequency(ASTExpressionNode *E);

  ASTDurationNode *ResolveDuration(ASTExpressionNode *E);

  ASTDurationNode *ResolveDuration(ASTExpressionNode *E0,
                                   ASTExpressionNode *E1);
};

} // namespace OpenPulse
} // namespace QASM

#endif // __QASM_AST_OPENPULSE_FRAME_H
