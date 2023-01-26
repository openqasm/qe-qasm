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

#ifndef __QASM_AST_CALL_EXPR_H
#define __QASM_AST_CALL_EXPR_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTGates.h>
#include <qasm/AST/ASTSymbolTable.h>

#include <vector>

namespace QASM {

class ASTCallExpressionNode : public ASTExpressionNode {
protected:
  const ASTIdentifierNode* CId;
  ASTParameterList PL;
  ASTIdentifierList IL;
  ASTType CallType;
  std::vector<const ASTSymbolTableEntry*> PST;
  std::vector<const ASTSymbolTableEntry*> AST;

private:
  ASTCallExpressionNode() = delete;

public:
  static const unsigned CallExpressionBits = 64U;

public:
  ASTCallExpressionNode(const ASTIdentifierNode* Id,
                        const ASTIdentifierNode* Callee,
                        const ASTParameterList& Params,
                        const ASTIdentifierList& Args);

  virtual ~ASTCallExpressionNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeFunctionCall;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetCallType() const {
    return CallType;
  }

  virtual const ASTIdentifierNode* GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const ASTParameterList& GetParameterList() const {
    return PL;
  }

  virtual const ASTIdentifierList& GetArgumentList() const {
    return IL;
  }

  virtual void ResolveSymbolTable();

  virtual const std::vector<const ASTSymbolTableEntry*>&
  GetParamSymbolTable() const {
    return PST;
  }

  virtual const std::vector<const ASTSymbolTableEntry*>&
  GetArgumentSymbolTable() const {
    return AST;
  }

  virtual void print() const override {
    std::cout << "<CallExpression>" << std::endl;
    std::cout << "<CallType>" << PrintTypeEnum(CallType)
      << "</CallType>" << std::endl;
    std::cout << "<Identifier>" << GetName() << "</Identifier>" << std::endl;
    std::cout << "<Callee>" << CId->GetName() << "</Callee>" << std::endl;
    std::cout << "<Parameters>" << std::endl;
    PL.print();
    std::cout << "</Parameters>" << std::endl;
    std::cout << "<Arguments>" << std::endl;
    IL.print();
    std::cout << "</Arguments>" << std::endl;
    std::cout << "</CallExpression>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM

#endif // __QASM_AST_CALL_EXPR_H

