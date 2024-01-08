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

#ifndef __QASM_AST_FUNCTION_CALL_EXPR_H
#define __QASM_AST_FUNCTION_CALL_EXPR_H

#include <qasm/AST/ASTDefcal.h>
#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTFunctions.h>
#include <qasm/AST/ASTKernel.h>
#include <qasm/AST/ASTTypes.h>

#include <iostream>
#include <map>

namespace QASM {

class ASTSymbolTableEntry;

class ASTFunctionCallNode : public ASTExpressionNode {
private:
  // Identifier of object (function, defcal, extern) being called.
  // Function call Identifier is in the ASTIdentifierNode constructor
  // argument.
  const ASTIdentifierNode *CId;

  union {
    const ASTFunctionDefinitionNode *FDN;
    const ASTKernelNode *KDN;
    const ASTDefcalNode *DDN;
  };

  ASTExpressionList CEL;
  ASTIdentifierList QIL;
  ASTType CallType;
  std::map<unsigned, ASTSymbolTableEntry *> STEMap;

private:
  ASTFunctionCallNode() = delete;

protected:
  ASTFunctionCallNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        CId(Id), FDN(nullptr), CEL(), QIL(), CallType(ASTTypeExpressionError),
        STEMap() {}

public:
  static const unsigned FunctionCallBits = 64U;

public:
  using list_type = ASTExpressionList;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTFunctionCallNode(const ASTIdentifierNode *Id,
                      const ASTFunctionDefinitionNode *FD,
                      const ASTExpressionList *EL)
      : ASTExpressionNode(Id, ASTTypeFunctionCallExpression),
        CId(FD->GetIdentifier()), FDN(FD), CEL(*EL), QIL(),
        CallType(ASTTypeFunctionCallExpression), STEMap() {}

  ASTFunctionCallNode(const ASTIdentifierNode *Id,
                      const ASTFunctionDefinitionNode *FD,
                      const ASTExpressionList *EL, const ASTIdentifierList *QL)
      : ASTExpressionNode(Id, ASTTypeFunctionCallExpression),
        CId(FD->GetIdentifier()), FDN(FD), CEL(*EL), QIL(*QL),
        CallType(ASTTypeFunctionCallExpression), STEMap() {}

  ASTFunctionCallNode(const ASTIdentifierNode *Id, const ASTDefcalNode *DD,
                      const ASTExpressionList *EL)
      : ASTExpressionNode(Id, ASTTypeDefcalCallExpression),
        CId(DD->GetIdentifier()), DDN(DD), CEL(*EL), QIL(),
        CallType(ASTTypeDefcalCallExpression), STEMap() {}

  ASTFunctionCallNode(const ASTIdentifierNode *Id, const ASTKernelNode *KD,
                      const ASTExpressionList *EL)
      : ASTExpressionNode(Id, ASTTypeKernelCallExpression),
        CId(KD->GetIdentifier()), KDN(KD), CEL(*EL), QIL(),
        CallType(ASTTypeKernelCallExpression), STEMap() {}

  virtual ~ASTFunctionCallNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeFunctionCall; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetFunctionCallType() const { return CallType; }

  virtual bool ReturnsResult() const {
    switch (CallType) {
    case ASTTypeDefcalCallExpression:
      return DDN->GetResultType() != ASTTypeVoid &&
             DDN->GetResultType() != ASTTypeUndefined;
      break;
    case ASTTypeFunctionCallExpression:
      return FDN->GetResultType() != ASTTypeVoid &&
             FDN->GetResultType() != ASTTypeUndefined;
      break;
    case ASTTypeKernelCallExpression:
      return KDN->GetResultType() != ASTTypeVoid &&
             KDN->GetResultType() != ASTTypeUndefined;
      break;
    default:
      break;
    }

    return false;
  }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const ASTIdentifierNode *GetCallIdentifier() const { return CId; }

  virtual const std::string &GetCallName() const { return CId->GetName(); }

  virtual unsigned GetNumArguments() const { return QIL.Size() + CEL.Size(); }

  virtual const ASTFunctionDefinitionNode *GetFunctionDefinition() const {
    return CallType == ASTTypeFunctionCallExpression ? FDN : nullptr;
  }

  virtual const ASTDefcalNode *GetDefcalDefinition() const {
    return CallType == ASTTypeDefcalCallExpression ? DDN : nullptr;
  }

  virtual const ASTKernelNode *GetKernelDefinition() const {
    return CallType == ASTTypeKernelCallExpression ? KDN : nullptr;
  }

  const ASTResultNode *GetResult() const {
    switch (CallType) {
    case ASTTypeFunctionCallExpression:
      return FDN->GetResult();
      break;
    case ASTTypeKernelCallExpression:
      return KDN->GetResult();
      break;
    default:
      break;
    }

    return nullptr;
  }

  ASTType GetResultType() const {
    switch (CallType) {
    case ASTTypeFunctionCallExpression:
      return FDN ? FDN->GetResultType() : ASTTypeUndefined;
      break;
    case ASTTypeKernelCallExpression:
      return KDN ? KDN->GetResultType() : ASTTypeUndefined;
      break;
    case ASTTypeDefcalCallExpression:
      return DDN ? DDN->GetResultType() : ASTTypeUndefined;
      break;
    default:
      return ASTTypeUndefined;
      break;
    }
  }

  virtual const ASTExpressionList &GetExpressionList() const { return CEL; }

  virtual const ASTIdentifierList &GetQuantumIdentifierList() const {
    return QIL;
  }

  virtual void AddSymbolTableEntry(ASTSymbolTableEntry *STE) {
    STEMap.insert(std::make_pair(static_cast<unsigned>(STEMap.size()), STE));
  }

  virtual const std::map<unsigned, ASTSymbolTableEntry *> &
  GetSymbolMap() const {
    return STEMap;
  }

  virtual unsigned GetSymbolMapSize() const {
    return static_cast<unsigned>(STEMap.size());
  }

  ASTSymbolTableEntry *GetSymbolMapEntry(unsigned IX) const {
    std::map<unsigned, ASTSymbolTableEntry *>::const_iterator I =
        STEMap.find(IX);

    return I == STEMap.end() ? nullptr : (*I).second;
  }

  iterator begin() { return CEL.begin(); }

  const_iterator begin() const { return CEL.begin(); }

  iterator end() { return CEL.end(); }

  const_iterator end() const { return CEL.end(); }

  virtual bool IsError() const override {
    return CallType == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTFunctionCallNode *ExpressionError(const ASTIdentifierNode *Id,
                                              const std::string &ERM) {
    return new ASTFunctionCallNode(Id, ERM);
  }

  virtual void print() const override {
    if (CallType == ASTTypeFunctionCallExpression) {
      std::cout << "<FunctionCallExpression>" << std::endl;
      std::cout << "<CallType>" << PrintTypeEnum(CallType) << "</CallType>"
                << std::endl;
      std::cout << "<FunctionName>" << GetName() << "</FunctionName>"
                << std::endl;
      std::cout << "<FunctionDefinitionName>" << FDN->GetName()
                << "</FunctionDefinitionName>" << std::endl;
      std::cout << "<FunctionArguments>" << std::endl;
      CEL.print();
      std::cout << "</FunctionArguments>" << std::endl;
      std::cout << "<QuantumArguments>" << std::endl;
      QIL.print();
      std::cout << "</QuantumArguments>" << std::endl;
      FDN->print();
      std::cout << "</FunctionCallExpression>" << std::endl;
    } else if (CallType == ASTTypeDefcalCallExpression) {
      std::cout << "<DefcalCallExpression>" << std::endl;
      std::cout << "<CallType>" << PrintTypeEnum(CallType) << "</CallType>"
                << std::endl;
      std::cout << "<DefcalName>" << GetName() << "</DefcalName>" << std::endl;
      std::cout << "<DefcalDefinitionName>" << DDN->GetName()
                << "</DefcalDefinitionName>" << std::endl;
      std::cout << "<DefcalArguments>" << std::endl;
      CEL.print();
      std::cout << "</DefcalArguments>" << std::endl;
      DDN->print();
      std::cout << "</DefcalCallExpression>" << std::endl;
    } else if (CallType == ASTTypeKernelCallExpression) {
      std::cout << "<KernelCallExpression>" << std::endl;
      std::cout << "<CallType>" << PrintTypeEnum(CallType) << "</CallType>"
                << std::endl;
      std::cout << "<KernelName>" << GetName() << "</KernelName>" << std::endl;
      std::cout << "<KernelDefinitionName>" << KDN->GetName()
                << "</KernelDefinitionName>" << std::endl;
      std::cout << "<KernelArguments>" << std::endl;
      CEL.print();
      std::cout << "</KernelArguments>" << std::endl;
      KDN->print();
      std::cout << "</KernelCallExpression>" << std::endl;
    }
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTFunctionCallStatementNode : public ASTStatementNode {
private:
  ASTFunctionCallStatementNode() = delete;

protected:
  ASTFunctionCallStatementNode(const ASTIdentifierNode *Id,
                               const std::string &ERM)
      : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)) {}

public:
  ASTFunctionCallStatementNode(const ASTFunctionCallNode *FC)
      : ASTStatementNode(FC->GetIdentifier(), FC) {}

  virtual ~ASTFunctionCallStatementNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeFunctionCallStatement;
  }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual const ASTFunctionCallNode *GetFunctionCall() const {
    return dynamic_cast<const ASTFunctionCallNode *>(
        ASTStatementNode::GetExpression());
  }

  virtual bool IsError() const override { return ASTStatementNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTFunctionCallStatementNode *
  StatementError(const ASTIdentifierNode *Id, const std::string &ERM) {
    return new ASTFunctionCallStatementNode(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<FunctionCallStatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</FunctionCallStatement>" << std::endl;
  }

  virtual void push(ASTBase * /* unused*/) override {}
};

class ASTFunctionCallValidator {
private:
  static ASTFunctionCallValidator FCV;

protected:
  ASTFunctionCallValidator() = default;

public:
  static ASTFunctionCallValidator &Instance() { return FCV; }

  ~ASTFunctionCallValidator() = default;

  void ValidateArgument(const ASTFunctionDefinitionNode *FDN,
                        const ASTSymbolTableEntry *XSTE, unsigned XI,
                        const std::string &AN, const std::string &FN);

  void ValidateArgument(const ASTSymbolTableEntry *XSTE, unsigned XI,
                        const std::string &AN, const std::string &FN);
};

} // namespace QASM

#endif // __QASM_AST_FUNCTION_CALL_EXPR_H
