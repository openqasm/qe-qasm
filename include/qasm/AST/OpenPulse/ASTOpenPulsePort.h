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

#ifndef __QASM_AST_OPENPULSE_PORT_H
#define __QASM_AST_OPENPULSE_PORT_H

#include <qasm/AST/ASTTypes.h>

#include <iostream>
#include <cassert>

namespace QASM {
namespace OpenPulse {

class ASTOpenPulsePortNode : public ASTExpressionNode {
private:
  bool EXT;
  uint64_t PID;

private:
  ASTOpenPulsePortNode() = delete;

protected:
  ASTOpenPulsePortNode(const ASTIdentifierNode* Id, const std::string& EM)
  : ASTExpressionNode(Id, new ASTStringNode(EM), ASTTypeExpressionError),
  EXT(false), PID(static_cast<uint64_t>(~0x0)) { }

public:
  static const unsigned PortBits = 64U;

public:
  ASTOpenPulsePortNode(const ASTIdentifierNode* Id, bool Extern = false)
  : ASTExpressionNode(Id, ASTTypeOpenPulsePort), EXT(Extern),
  PID(0UL) { }

  ASTOpenPulsePortNode(const ASTIdentifierNode* Id, uint64_t PId,
                       bool Extern = false)
  : ASTExpressionNode(Id, ASTTypeOpenPulsePort), EXT(Extern),
  PID(PId) { }

  virtual ~ASTOpenPulsePortNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeOpenPulsePort;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  // Implemented in ASTOpenPulseFrame.cpp.
  virtual void Mangle() override;

  virtual const std::string& GetName() const override {
    return ASTExpressionNode::GetIdentifier()->GetName();
  }

  virtual uint64_t GetPortId() const {
    return PID;
  }

  virtual bool IsExtern() const {
    return EXT;
  }

  virtual void SetPortId(uint64_t V) {
    PID = V;
  }

  virtual unsigned GetBits() const {
    return PortBits;
  }

  static ASTOpenPulsePortNode* ExpressionError(const ASTIdentifierNode* Id,
                                               const std::string& ERM) {
    return new ASTOpenPulsePortNode(Id, ERM);
  }

  virtual bool IsError() const override {
    return ASTExpressionNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTExpressionNode::GetError();
  }

  virtual void print() const override {
    std::cout << "<OpenPulsePort>" << std::endl;
    ASTExpressionNode::print();
    std::cout << "<IsExtern>" << std::boolalpha << EXT
      << "</IsExtern>" << std::endl;
    std::cout << "<PortID>" << PID << "</PortID>" << std::endl;
    std::cout << "</OpenPulsePort>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace OpenPulse
} // namespace QASM

#endif // __QASM_AST_OPENPULSE_PORT_H

