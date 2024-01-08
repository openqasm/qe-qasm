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

#ifndef __QASM_AST_PRAGMA_H
#define __QASM_AST_PRAGMA_H

#include <qasm/AST/ASTExpressionNodeList.h>
#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <iostream>
#include <string>

namespace QASM {

class ASTPragmaNode : public ASTExpressionNode {
private:
  std::string PN;
  ASTExpressionNodeList EL;

private:
  ASTPragmaNode() = delete;

protected:
  ASTPragmaNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        PN(ERM), EL() {}

public:
  static const unsigned PragmaBits = 64U;

public:
  ASTPragmaNode(const ASTIdentifierNode *Id, const std::string &N,
                const ASTExpressionNodeList &EXL)
      : ASTExpressionNode(Id, ASTTypePragma), PN(N), EL(EXL) {}

  virtual ~ASTPragmaNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypePragma; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::GetIdentifier()->GetName();
  }

  virtual const std::string &GetPragmaName() const { return PN; }

  virtual const ASTExpressionNodeList &GetExpressionList() const { return EL; }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTPragmaNode *ExpressionError(const std::string &ERM) {
    return new ASTPragmaNode(ASTIdentifierNode::Pragma.Clone(), ERM);
  }

  static ASTPragmaNode *ExpressionError(const ASTIdentifierNode *Id,
                                        const std::string &ERM) {
    return new ASTPragmaNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<Pragma>" << std::endl;
    std::cout << "<PragmaDirective>" << PN << "</PragmaDirective>" << std::endl;
    EL.print();
    std::cout << "</Pragma>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTPragmaStatementNode : public ASTStatementNode {
private:
  ASTPragmaStatementNode() = delete;

protected:
  ASTPragmaStatementNode(const ASTIdentifierNode *Id,
                         const ASTExpressionNode *EN)
      : ASTStatementNode(Id, EN) {}

public:
  ASTPragmaStatementNode(const ASTPragmaNode *PN)
      : ASTStatementNode(PN->GetIdentifier(), PN) {}

  virtual ~ASTPragmaStatementNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypePragmaStmt; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual const ASTPragmaNode *GetPragma() const {
    return dynamic_cast<const ASTPragmaNode *>(
        ASTStatementNode::GetExpression());
  }

  static ASTPragmaStatementNode *StatementError(const std::string &ERM) {
    const ASTIdentifierNode *Id = ASTIdentifierNode::Pragma.Clone();
    assert(Id && "Could not clone a valid ASTIdentifierNode!");

    ASTPragmaStatementNode *SR =
        new ASTPragmaStatementNode(Id, ASTPragmaNode::ExpressionError(Id, ERM));
    assert(SR && "Could not create a valid ASTStatementNode!");
    return SR;
  }

  virtual bool IsError() const override { return ASTStatementNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  virtual void print() const override {
    std::cout << "<PragmaStatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</PragmaStatement>" << std::endl;
  }

  virtual void push(ASTBase * /* unused*/) override {}
};

} // namespace QASM

#endif // __QASM_AST_PRAGMA_H
