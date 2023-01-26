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
#ifndef __QASM_AST_FUNCTION_CALL_ARGUMENT_H
#define __QASM_AST_FUNCTION_CALL_ARGUMENT_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTFunctions.h>
#include <qasm/AST/ASTFunctionCallExpr.h>

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

namespace QASM {

class ASTFunctionCallArgumentList : public ASTBase {
private:
  std::vector<const ASTExpressionNode*> FAL;

public:
  ASTFunctionCallArgumentList() = default;

  ASTFunctionCallArgumentList(const ASTFunctionCallArgumentList& RHS)
  : ASTBase(), FAL(RHS.FAL) { }

  virtual ~ASTFunctionCallArgumentList() = default;

  ASTFunctionCallArgumentList& operator=(const ASTFunctionCallArgumentList& RHS) {
    if (this != &RHS) {
      ASTBase::operator=(RHS);
      FAL = RHS.FAL;
    }

    return *this;
  }

  virtual ASTType GetASTType() const override {
    return ASTTypeFunctionCallArgumentList;
  }

  virtual unsigned Size() const {
    return static_cast<unsigned>(FAL.size());
  }

  virtual void Clear() {
    FAL.clear();
  }

  virtual void Append(const ASTExpressionNode* EN) {
    if (EN)
      FAL.push_back(EN);
  }

  virtual const ASTExpressionNode* GetArgument(unsigned Idx) {
    assert(Idx < FAL.size() && "Index is out-of-range!");

    try {
      return FAL.at(Idx);
    } catch (const std::out_of_range& E) {
      (void) E;
    } catch ( ... ) {
    }

    return nullptr;
  }

  virtual void print() const override {
    std::cout << "<FunctionCallArgumentList>" << std::endl;
    for (std::vector<const ASTExpressionNode*>::const_iterator I = FAL.begin();
         I != FAL.end(); ++I)
      (*I)->print();
    std::cout << "</FunctionCallArgumentList>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTFunctionCallArgumentNode : public ASTExpressionNode {
private:
  const ASTIdentifierNode* FId;
  const ASTFunctionDeclarationNode* FDN;
  const ASTFunctionCallArgumentList* FAL;
  const ASTResultNode* Result;

  union {
    const ASTBoolNode* B;
    const ASTCBitNode* C;
    const ASTIntNode* I;
    const ASTFloatNode* F;
    const ASTDoubleNode* D;
    const ASTMPIntegerNode* MPI;
    const ASTMPDecimalNode* MPD;
    const ASTAngleNode* A;
    void* UB;
  };

  ASTType EType;

public:
  ASTFunctionCallArgumentNode(const ASTIdentifierNode* FN,
                              const ASTFunctionCallArgumentList* AL)
  : ASTExpressionNode(FN, ASTTypeFunctionCallArgument),
  FId(FN), FDN(nullptr), FAL(AL), Result(nullptr),
  UB(nullptr), EType(ASTTypeUndefined) { }

  virtual ~ASTFunctionCallArgumentNode() = default;

  virtual bool ResolveFunctionCall();

  virtual ASTType GetASTType() const override {
    return ASTTypeFunctionCallArgument;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual bool IsExpression() const override {
    return true;
  }

  virtual const ASTIdentifierNode* GetIdentifier() const override {
    return Ident;
  }

  virtual const std::string& GetName() const override {
    return Ident->GetName();
  }

  virtual const std::string& GetFunctionName() const {
    return FId->GetName();
  }

  virtual const ASTResultNode* GetResult() const {
    return Result;
  }

  virtual ASTType GetResultType() const {
    return EType;
  }

  virtual void print() const override {
    std::cout << "<FunctionCallArgumentNode>" << std::endl;
    FId->print();
    FDN->print();
    FAL->print();
    Result->print();
    std::cout << "</FunctionCallArgumentNode>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM

#endif // __QASM_AST_FUNCTION_CALL_ARGUMENT_H

