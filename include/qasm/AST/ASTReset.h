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

#ifndef __QASM_AST_RESET_NODE_H
#define __QASM_AST_RESET_NODE_H

#include <qasm/AST/ASTArgument.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTTypes.h>

namespace QASM {

class ASTResetNode : public ASTGateQOpNode {
private:
  const ASTIdentifierNode *Target;

protected:
  ASTResetNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTGateQOpNode(Id, ERM), Target(nullptr) {}

private:
  ASTResetNode() = delete;

public:
  static const unsigned ResetBits = 64U;

public:
  ASTResetNode(const ASTIdentifierNode *Id, const ASTIdentifierNode *T)
      : ASTGateQOpNode(Id, static_cast<ASTGateNode *>(nullptr)), Target(T) {}

  virtual ~ASTResetNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeReset; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual void Mangle() override;

  const ASTIdentifierNode *GetTarget() const { return Target; }

  ASTIdentifierNode *GetTarget() {
    return const_cast<ASTIdentifierNode *>(Target);
  }

  virtual bool IsError() const override {
    return ASTGateOpNode::MTy == ASTTypeExpressionError ||
           ASTGateOpNode::OTy == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTResetNode *StatementError(const ASTIdentifierNode *Id,
                                      const std::string &ERM) {
    return new ASTResetNode(Id, ERM);
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_RESET_NODE_H
