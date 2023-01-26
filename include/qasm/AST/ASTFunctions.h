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

#ifndef __QASM_AST_FUNCTIONS_H
#define __QASM_AST_FUNCTIONS_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTPrimitives.h>
#include <qasm/AST/ASTDeclaration.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTParameterList.h>
#include <qasm/AST/ASTArgument.h>
#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTResult.h>

#include <iostream>
#include <vector>
#include <map>
#include <cassert>

namespace QASM {

class ASTSymbolTableEntry;
class ASTIfStatementNode;
class ASTElseIfStatementNode;
class ASTElseStatementNode;
class ASTForStatementNode;
class ASTWhileStatementNode;
class ASTDoWhileStatementNode;
class ASTSwitchStatementNode;
class ASTCaseStatementNode;
class ASTDefaultStatementNode;
class ASTReturnStatementNode;

class ASTFunctionDefinitionNode : public ASTExpressionNode {
  friend class ASTFunctionDefinitionBuilder;

public:
  class QCS {
  private:
    ASTType Ty;
    union {
      ASTQubitContainerNode* QCN;
      ASTQubitContainerAliasNode* QCA;
    };

  public:
    QCS() : Ty(ASTTypeUndefined), QCN(nullptr) { }

    QCS(ASTQubitContainerNode* QN)
    : Ty(ASTTypeQubitContainer), QCN(QN) { }

    QCS(ASTQubitContainerAliasNode* QA)
    : Ty(ASTTypeQubitContainerAlias), QCA(QA) { }

    ~QCS() = default;

    ASTType GetType() const {
      return Ty;
    }

    ASTQubitContainerNode* GetQubitContainer() const {
      return Ty == ASTTypeQubitContainer ? QCN : nullptr;
    }

    ASTQubitContainerAliasNode* GetQubitContainerAlias() const {
      return Ty == ASTTypeQubitContainerAlias ? QCA : nullptr;
    }
  };

protected:
  std::map<unsigned, ASTDeclarationNode*> Params;
  std::map<std::string, const ASTSymbolTableEntry*> STM;
  ASTStatementList Statements;
  ASTResultNode* Result;
  bool IsDeclADefinition;
  bool Ellipsis;
  bool Builtin;
  bool Extern;

protected:
  virtual bool CheckReturnType(const ASTReturnStatementNode* RSN,
                               std::pair<ASTType, ASTType>& OP) const;
  virtual bool CheckMeasureReturnType(const ASTReturnStatementNode* RSN,
                                      std::pair<ASTType, ASTType>& OP) const;
  virtual bool CheckFunctionReturnType(const ASTReturnStatementNode* RSN,
                                       std::pair<ASTType, ASTType>& OP) const;

  virtual const ASTReturnStatementNode*
  CheckReturnStatement(const ASTIfStatementNode* IFS,
                       std::pair<ASTType, ASTType>& OP) const;

  virtual const ASTReturnStatementNode*
  CheckReturnStatement(const ASTElseIfStatementNode* EIS,
                       std::pair<ASTType, ASTType>& OP) const;

  virtual const ASTReturnStatementNode*
  CheckReturnStatement(const ASTElseStatementNode* ES,
                       std::pair<ASTType, ASTType>& OP) const;

  virtual const ASTReturnStatementNode*
  CheckReturnStatement(const ASTForStatementNode* FS,
                       std::pair<ASTType, ASTType>& OP) const;

  virtual const ASTReturnStatementNode*
  CheckReturnStatement(const ASTWhileStatementNode* WS,
                       std::pair<ASTType, ASTType>& OP) const;

  virtual const ASTReturnStatementNode*
  CheckReturnStatement(const ASTDoWhileStatementNode* DWS,
                       std::pair<ASTType, ASTType>& OP) const;

  virtual const ASTReturnStatementNode*
  CheckReturnStatement(const ASTSwitchStatementNode* SW,
                       std::pair<ASTType, ASTType>& OP) const;

  virtual const ASTReturnStatementNode*
  CheckReturnStatement(const ASTCaseStatementNode* CSN,
                       std::pair<ASTType, ASTType>& OP) const;

  virtual const ASTReturnStatementNode*
  CheckReturnStatement(const ASTDefaultStatementNode* DSN,
                       std::pair<ASTType, ASTType>& OP) const;

  virtual const ASTReturnStatementNode*
  CheckReturnStatements(const ASTStatementList& SL,
                        std::pair<ASTType, ASTType>& OP,
                        ASTType STy) const;


private:
  ASTFunctionDefinitionNode() = delete;

public:
  static const unsigned FunctionBits = 64U;

public:
  ASTFunctionDefinitionNode(const ASTIdentifierNode* Id,
                            const ASTDeclarationList& PDL,
                            const ASTStatementList& SL,
                            ASTResultNode *RES,
                            bool IsDefinition = true);

  virtual ~ASTFunctionDefinitionNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeFunction;
  }

  ASTSemaType GetSemaType() const override {
    return SemaTypeFunction;
  }

  virtual void Mangle() override;

  virtual bool CheckReturnType(std::pair<ASTType, ASTType>& OP) const;

  virtual const ASTReturnStatementNode*
  CheckReturnStatements(std::pair<ASTType, ASTType>& OP) const;

  virtual void SetIsDefinition(bool B) { IsDeclADefinition = B; }

  virtual void SetIsBuiltin(bool B = true) { Builtin = B; }

  virtual bool IsDefinition() const { return IsDeclADefinition; }

  virtual bool IsBuiltin() const { return Builtin; }

  virtual bool IsExtern() const { return Extern; }

  virtual void AttachStatements(const ASTStatementList& SL) {
    Statements = SL;
  }

  virtual const ASTStatementList& GetStatements() const {
    return Statements;
  }

  virtual const std::map<unsigned, ASTDeclarationNode*>& GetParameters() const {
    return Params;
  }

  virtual std::map<std::string, const ASTSymbolTableEntry*>& GetSymbolTable() {
    return STM;
  }

  virtual const std::map<std::string, const ASTSymbolTableEntry*>&
  GetSymbolTable() const {
    return STM;
  }

  virtual const ASTSymbolTableEntry* GetSymbol(const std::string& SN) const {
    std::map<std::string, const ASTSymbolTableEntry*>::const_iterator I =
      STM.find(SN);
    return I == STM.end() ? nullptr : (*I).second;
  }

  virtual bool HasParameters() const {
    return Params.size() != 0;
  }

  virtual bool HasEllipsis() const {
    return Ellipsis;
  }

  virtual bool HasResult() const {
    return Result;
  }

  virtual void SetExtern(bool E) {
    Extern = E;
  }

  virtual void SetResult(ASTResultNode* R) {
    Result = R;
  }

  virtual const ASTResultNode* GetResult() const {
    return Result;
  }

  virtual ASTResultNode* GetResult() {
    return Result;
  }

  virtual ASTType GetResultType() const {
    if (HasResult())
      return GetResult()->GetResultType();

    return ASTTypeUndefined;
  }

  const ASTDeclarationNode* GetParameter(unsigned IX) const {
    std::map<unsigned, ASTDeclarationNode*>::const_iterator I = Params.find(IX);
    return I == Params.end() ? nullptr : (*I).second;
  }

  const ASTSymbolTableEntry* GetParameterSymbol(const std::string& SYN) const {
    std::map<std::string, const ASTSymbolTableEntry*>::const_iterator I =
      STM.find(SYN);
    return I == STM.end() ? nullptr : (*I).second;
  }

  virtual unsigned GetNumParameters() const {
    return static_cast<unsigned>(Params.size());
  }

  virtual const ASTDeclarationContext* GetDeclarationContext() const override {
    return GetIdentifier()->GetDeclarationContext();
  }

  virtual void print() const override {
    std::cout << "<FunctionDefinition>" << std::endl;
    std::cout << "<FunctionName>" << GetName() << "</FunctionName>"
      << std::endl;
    std::cout << "<MangledName>" << GetMangledName()
      << "</MangledName>" << std::endl;
    std::cout << "<Extern>" << std::boolalpha << Extern
      << "</Extern>" << std::endl;
    GetDeclarationContext()->print();

    if (!Params.empty()) {
      std::cout << "<FunctionParameters>" << std::endl;
      for (std::map<unsigned, ASTDeclarationNode*>::const_iterator I = Params.begin();
           I != Params.end(); ++I) {
        std::cout << "<FunctionParameter>" << std::endl;
        std::cout << "<ParameterIndex>" << (*I).first << "</ParameterIndex>"
          << std::endl;
        (*I).second->print();
        std::cout << "</FunctionParameter>" << std::endl;
      }
      std::cout << "</FunctionParameters>" << std::endl;
    }

    std::cout << "<HasEllipsis>" << std::boolalpha << Ellipsis
      << "</HasEllipsis>" << std::endl;
    std::cout << "<IsBuiltin>" << std::boolalpha << Builtin
      << "</IsBuiltin>" << std::endl;

    if (!Statements.Empty()) {
      std::cout << "<FunctionStatements>" << std::endl;
      Statements.print();
      std::cout << "</FunctionStatements>" << std::endl;
    }

    if (HasResult()) {
      Result->print();
    } else {
      std::cout << "<Result>" << std::endl;
      std::cout << "</Result>" << std::endl;
    }

    std::cout << "</FunctionDefinition>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTFunctionDeclarationNode : public ASTDeclarationNode {
private:
  const ASTFunctionDefinitionNode* FN;

protected:
  ASTFunctionDeclarationNode(const ASTIdentifierNode* Id,
                             const std::string& ERM)
  : ASTDeclarationNode(Id, new ASTStringNode(ERM), ASTTypeDeclarationError)
  { }

public:
  static const unsigned FunctionDeclBits = 64U;

public:
  ASTFunctionDeclarationNode(const ASTIdentifierNode* Id,
                             const ASTFunctionDefinitionNode* F)
  : ASTDeclarationNode(Id, ASTTypeFunction), FN(F) { }

  virtual ~ASTFunctionDeclarationNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeFunctionDeclaration;
  }

  static ASTFunctionDeclarationNode* DeclarationError(const ASTIdentifierNode* Id,
                                                      const std::string& ERM) {
    return new ASTFunctionDeclarationNode(Id, ERM);
  }

  virtual bool IsError() const override {
    return ASTDeclarationNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTDeclarationNode::GetError();
  }

  virtual void Mangle() override;

  virtual const ASTFunctionDefinitionNode* GetDefinition() const {
    return FN;
  }

  virtual void print() const override {
    std::cout << "<FunctionDeclaration>" << std::endl;
    FN->print();
    std::cout << "</FunctionDeclaration>" << std::endl;
  }
};

class ASTFunctionDeclarationMap {
private:
  static ASTFunctionDeclarationMap FDM;
  std::map<std::string, ASTFunctionDeclarationNode*> M;

protected:
  ASTFunctionDeclarationMap() = default;

public:
  static ASTFunctionDeclarationMap& Instance() {
    return FDM;
  }

  void Insert(const std::string& FDN, ASTFunctionDeclarationNode* FN) {
    assert(FN && "Invalid ASTFunctionDeclarationNode argument!");
    M.insert(std::make_pair(FDN, FN));
  }

  unsigned Size() const {
    return M.size();
  }

  void Erase(ASTFunctionDeclarationNode* FN) {
    assert(FN && "Invalid ASTFunctionDeclarationNode argument!");

    std::map<std::string, ASTFunctionDeclarationNode*>::iterator I =
      M.find(FN->GetName());
    if (I != M.end())
      M.erase(I);
  }

  const ASTFunctionDeclarationNode* Find(const std::string& S) const {
    std::map<std::string, ASTFunctionDeclarationNode*>::const_iterator I =
      M.find(S);

    return I == M.end() ? nullptr : (*I).second;
  }
};

} // namespace QASM

#endif // __QASM_AST_FUNCTIONS_H

