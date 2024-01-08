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

#ifndef __QASM_AST_ANNOTATION_H
#define __QASM_AST_ANNOTATION_H

#include <qasm/AST/ASTExpressionNodeList.h>
#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <iostream>
#include <string>

namespace QASM {

class ASTAnnotationNode : public ASTExpressionNode {
private:
  std::string AN;
  ASTExpressionNodeList EL;

private:
  ASTAnnotationNode() = delete;

protected:
  ASTAnnotationNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        AN(ERM), EL() {}

public:
  static const unsigned AnnotationBits = 64U;

public:
  ASTAnnotationNode(const ASTIdentifierNode *Id, const std::string &N,
                    const ASTExpressionNodeList &EXL)
      : ASTExpressionNode(Id, ASTTypeAnnotation), AN(N), EL(EXL) {}

  virtual ~ASTAnnotationNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeAnnotation; }

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

  virtual const std::string &GetAnnotationName() const { return AN; }

  virtual const ASTExpressionNodeList &GetExpressionList() const { return EL; }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTAnnotationNode *ExpressionError(const std::string &ERM) {
    return new ASTAnnotationNode(ASTIdentifierNode::Annotation.Clone(), ERM);
  }

  static ASTAnnotationNode *ExpressionError(const ASTIdentifierNode *Id,
                                            const std::string &ERM) {
    return new ASTAnnotationNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<Annotation>" << std::endl;
    std::cout << "<AnnotationDirective>" << AN << "</AnnotationDirective>"
              << std::endl;
    EL.print();
    std::cout << "</Annotation>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTAnnotationStatementNode : public ASTStatementNode {
private:
  ASTAnnotationStatementNode() = delete;

protected:
  ASTAnnotationStatementNode(const ASTIdentifierNode *Id,
                             const ASTExpressionNode *EN)
      : ASTStatementNode(Id, EN) {}

public:
  ASTAnnotationStatementNode(const ASTAnnotationNode *AN)
      : ASTStatementNode(AN->GetIdentifier(), AN) {}

  virtual ~ASTAnnotationStatementNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeAnnotationStmt; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual const ASTAnnotationNode *GetAnnotation() const {
    return dynamic_cast<const ASTAnnotationNode *>(
        ASTStatementNode::GetExpression());
  }

  static ASTAnnotationStatementNode *StatementError(const std::string &ERM) {
    const ASTIdentifierNode *Id = ASTIdentifierNode::Annotation.Clone();
    assert(Id && "Could not clone a valid ASTIdentifierNode!");

    ASTAnnotationStatementNode *SR = new ASTAnnotationStatementNode(
        Id, ASTAnnotationNode::ExpressionError(Id, ERM));
    assert(SR && "Could not create a valid ASTStatementNode!");
    return SR;
  }

  virtual bool IsError() const override { return ASTStatementNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  virtual void print() const override {
    std::cout << "<AnnotationStatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</AnnotationStatement>" << std::endl;
  }

  virtual void push(ASTBase * /* unused*/) override {}
};

} // namespace QASM

#endif // __QASM_AST_ANNOTATION_H
