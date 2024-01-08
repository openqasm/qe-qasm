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

#ifndef __QASM_AST_POPCOUNT_H
#define __QASM_AST_POPCOUNT_H

#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTTypes.h>

#include <iostream>

namespace QASM {

class ASTPopcountNode : public ASTExpressionNode {
private:
  union {
    const ASTIntNode *I;
    const ASTMPIntegerNode *MPI;
    const ASTCBitNode *CBI;
  };

  ASTType IType;

private:
  ASTPopcountNode() = delete;

protected:
  ASTPopcountNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        I(nullptr), IType(ASTTypeExpressionError) {}

public:
  ASTPopcountNode(const ASTIdentifierNode *Id, const ASTIntNode *Target)
      : ASTExpressionNode(Id, ASTTypePopcountExpr), I(Target),
        IType(ASTTypeInt) {}

  ASTPopcountNode(const ASTIdentifierNode *Id, const ASTMPIntegerNode *Target)
      : ASTExpressionNode(Id, ASTTypePopcountExpr), MPI(Target),
        IType(ASTTypeMPInteger) {}

  ASTPopcountNode(const ASTIdentifierNode *Id, const ASTCBitNode *Target)
      : ASTExpressionNode(Id, ASTTypePopcountExpr), CBI(Target),
        IType(ASTTypeBitset) {}

  virtual ~ASTPopcountNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypePopcountExpr; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  // Implemented in ASTTypes.cpp.
  virtual void Mangle() override;

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual ASTType GetTargetType() const { return IType; }

  virtual const ASTIntNode *GetIntegerTarget() const {
    return IType == ASTTypeInt ? I : nullptr;
  }

  virtual const ASTMPIntegerNode *GetMPIntegerTarget() const {
    return IType == ASTTypeMPInteger ? MPI : nullptr;
  }

  virtual const ASTCBitNode *GetBitsetTarget() const {
    return IType == ASTTypeBitset ? CBI : nullptr;
  }

  unsigned GetPopcount() const {
    switch (IType) {
    case ASTTypeInt:
      return I->Popcount();
      break;
    case ASTTypeMPInteger:
      return MPI->Popcount();
      break;
    case ASTTypeBitset:
      return CBI->Popcount();
      break;
    default:
      break;
    }

    return static_cast<unsigned>(~0x0);
  }

  virtual bool IsError() const override {
    return IType == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTPopcountNode *ExpressionError(const std::string &ERM) {
    return new ASTPopcountNode(ASTIdentifierNode::Popcount.Clone(), ERM);
  }

  static ASTPopcountNode *ExpressionError(const ASTIdentifierNode *Id,
                                          const std::string &ERM) {
    return new ASTPopcountNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<PopcountNode>" << std::endl;
    std::cout << "<Target>" << std::endl;
    if (IType == ASTTypeInt)
      I->print();
    else if (IType == ASTTypeMPInteger)
      MPI->print();
    else if (IType == ASTTypeBitset)
      CBI->print();
    std::cout << "</Target>" << std::endl;
    std::cout << "<PopcountValue>";
    if (IType == ASTTypeInt)
      std::cout << I->Popcount();
    else if (IType == ASTTypeMPInteger)
      std::cout << MPI->Popcount();
    else if (IType == ASTTypeBitset)
      std::cout << CBI->Popcount();
    std::cout << "</PopcountValue>" << std::endl;
    std::cout << "</PopcountNode>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTPopcountStatementNode : public ASTStatementNode {
private:
  ASTPopcountStatementNode() = delete;

protected:
  ASTPopcountStatementNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)) {}

public:
  ASTPopcountStatementNode(const ASTPopcountNode *PN)
      : ASTStatementNode(PN->GetIdentifier(), PN) {}

  virtual ~ASTPopcountStatementNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypePopcountStmt; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual const ASTPopcountNode *GetPopcountNode() const {
    return dynamic_cast<const ASTPopcountNode *>(
        ASTStatementNode::GetExpression());
  }

  virtual bool IsError() const override { return ASTStatementNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTPopcountStatementNode *StatementError(const ASTIdentifierNode *Id,
                                                  const std::string &ERM) {
    return new ASTPopcountStatementNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<PopcountStatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</PopcountStatement>" << std::endl;
  }

  virtual void push(ASTBase * /* unused*/) override {}
};

} // namespace QASM

#endif // __QASM_AST_POPCOUNT_H
