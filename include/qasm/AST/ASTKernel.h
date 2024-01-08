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

#ifndef __QASM_AST_KERNEL_H
#define __QASM_AST_KERNEL_H

#include <qasm/AST/ASTDeclarationList.h>
#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTResult.h>
#include <qasm/AST/ASTStatement.h>
#include <qasm/AST/ASTTypes.h>

#include <map>
#include <string>

namespace QASM {

class ASTSymbolTableEntry;

class ASTKernelNode : public ASTStatementNode {
private:
  std::map<unsigned, ASTDeclarationNode *> Params;
  ASTStatementList Statements;
  const ASTResultNode *Result;
  std::map<std::string, const ASTSymbolTableEntry *> STM;
  bool Extern;
  bool Ellipsis;

private:
  ASTKernelNode() = delete;

public:
  static const unsigned KernelBits = 64U;

public:
  ASTKernelNode(const ASTIdentifierNode *Id, const ASTDeclarationList &DL,
                const ASTStatementList &SL, ASTResultNode *RES);

  virtual ~ASTKernelNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeKernel; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual void Mangle() override;

  virtual const std::string &GetName() const override {
    return ASTStatementNode::Ident->GetName();
  }

  virtual const std::string &GetMangledName() const override {
    return ASTStatementNode::Ident->GetMangledName();
  }

  virtual void AttachStatements(const ASTStatementList &SL) { Statements = SL; }

  virtual bool IsExtern() const { return Extern; }

  virtual bool HasResult() const { return Result; }

  virtual const ASTStatementList &GetStatements() const { return Statements; }

  virtual const std::map<unsigned, ASTDeclarationNode *> &
  GetParameters() const {
    return Params;
  }

  virtual void SetExtern(bool E) { Extern = E; }

  virtual void SetResult(ASTResultNode *R) { Result = R; }

  virtual const ASTResultNode *GetResult() const { return Result; }

  virtual ASTResultNode *GetResult() {
    return const_cast<ASTResultNode *>(Result);
  }

  virtual ASTType GetResultType() const {
    if (HasResult())
      return GetResult()->GetResultType();

    return ASTTypeUndefined;
  }

  virtual std::map<std::string, const ASTSymbolTableEntry *> &GetSymbolTable() {
    return STM;
  }

  virtual const std::map<std::string, const ASTSymbolTableEntry *> &
  GetSymbolTable() const {
    return STM;
  }

  virtual const ASTSymbolTableEntry *GetSymbol(const std::string &SN) const {
    std::map<std::string, const ASTSymbolTableEntry *>::const_iterator I =
        STM.find(SN);
    return I == STM.end() ? nullptr : (*I).second;
  }

  virtual bool HasParameters() const { return Params.size() != 0; }

  virtual bool HasEllipsis() const { return Ellipsis; }

  virtual void TransferLocalSymbolTable();

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTKernelDeclarationNode : public ASTDeclarationNode {
private:
  const ASTKernelNode *KN;

protected:
  ASTKernelDeclarationNode(const ASTIdentifierNode *Id, const std::string &EM)
      : ASTDeclarationNode(Id, new ASTStringNode(EM), ASTTypeDeclarationError),
        KN(nullptr) {}

public:
  ASTKernelDeclarationNode(const ASTIdentifierNode *Id,
                           const ASTKernelNode *KRN)
      : ASTDeclarationNode(Id, ASTTypeKernel), KN(KRN) {}

  virtual ~ASTKernelDeclarationNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeKernelDeclaration;
  }

  virtual const ASTKernelNode *GetKernel() const { return KN; }

  static ASTKernelDeclarationNode *DeclarationError(const ASTIdentifierNode *Id,
                                                    const std::string &EM) {
    return new ASTKernelDeclarationNode(Id, EM);
  }

  virtual void print() const override {
    std::cout << "<KernelDeclaration>" << std::endl;
    KN->print();
    std::cout << "</KernelDeclaration>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_KERNEL_H
