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

#ifndef __QASM_AST_GATE_CONTROL_H
#define __QASM_AST_GATE_CONTROL_H

#include <qasm/AST/ASTTypes.h>

namespace QASM {

class ASTGateControlStmtNode : public ASTStatementNode {
private:
  ASTGateControlStmtNode() = delete;

public:
  static const unsigned GateControlStmtBits = 64U;

public:
  ASTGateControlStmtNode(const ASTGateControlNode* GCN)
  : ASTStatementNode(GCN->GetIdentifier(), GCN) { }

  virtual ~ASTGateControlStmtNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeGateControlStatement;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual void Mangle() override;

  virtual const ASTGateControlNode* GetControlNode() const {
    return dynamic_cast<const ASTGateControlNode*>(
                              ASTStatementNode::GetExpression());
  }

  virtual void print() const override {
    std::cout << "<GateControlStatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</GateControlStatement>" << std::endl;
  }

  virtual void push(ASTBase* /* unused*/) override { }
};

class ASTGateNegControlStmtNode : public ASTStatementNode {
private:
  ASTGateNegControlStmtNode() = delete;

public:
  static const unsigned GateNegControlStmtBits = 64U;

public:
  ASTGateNegControlStmtNode(const ASTGateNegControlNode* GNCN)
  : ASTStatementNode(GNCN->GetIdentifier(), GNCN) { }

  virtual ~ASTGateNegControlStmtNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeGateNegControlStatement;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual void Mangle() override;

  virtual const ASTGateNegControlNode* GetNegControlNode() const {
    return dynamic_cast<const ASTGateNegControlNode*>(
                              ASTStatementNode::GetExpression());
  }

  virtual void print() const override {
    std::cout << "<GateNegControlStatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</GateNegControlStatement>" << std::endl;
  }

  virtual void push(ASTBase* /* unused*/) override { }
};

class ASTGateInverseStmtNode : public ASTStatementNode {
private:
  ASTGateInverseStmtNode() = delete;

public:
  static const unsigned GateInverseStmtBits = 64U;

public:
  ASTGateInverseStmtNode(const ASTGateInverseNode* GIN)
  : ASTStatementNode(GIN->GetIdentifier(), GIN) { }

  virtual ~ASTGateInverseStmtNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeGateInverseStatement;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual void Mangle() override;

  virtual const ASTGateInverseNode* GetInverseNode() const {
    return dynamic_cast<const ASTGateInverseNode*>(
                              ASTStatementNode::GetExpression());
  }

  virtual void print() const override {
    std::cout << "<GateInverseStatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</GateInverseStatement>" << std::endl;
  }

  virtual void push(ASTBase* /* unused*/) override { }
};

class ASTGatePowerStmtNode : public ASTStatementNode {
private:
  ASTGatePowerStmtNode() = delete;

public:
  static const unsigned GatePowerStmtBits = 64U;

public:
  ASTGatePowerStmtNode(const ASTGatePowerNode* GPN)
  : ASTStatementNode(GPN->GetIdentifier(), GPN) { }

  virtual ~ASTGatePowerStmtNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeGatePowerStatement;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual void Mangle() override;

  virtual const ASTGatePowerNode* GetPowerNode() const {
    return dynamic_cast<const ASTGatePowerNode*>(
                              ASTStatementNode::GetExpression());
  }

  virtual void print() const override {
    std::cout << "<GatePowertatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</GatePowerStatement>" << std::endl;
  }

  virtual void push(ASTBase* /* unused*/) override { }
};

} // namespace QASM

#endif // __QASM_AST_GATE_CONTROL_H

