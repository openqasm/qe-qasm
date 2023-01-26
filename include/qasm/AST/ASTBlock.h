/* -*- coding: utf-8 -*-
 *
 * Copyright 2020 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_BLOCK_H
#define __QASM_AST_BLOCK_H

#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTTypes.h>

#include <iostream>
#include <vector>

namespace QASM {

class ASTBlock : public ASTExpression {
private:
  std::list<ASTBlockNode*> Graph;

public:
  ASTBlock() : ASTExpression() { }
  virtual ~ASTBlock() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeBlock;
  }

  ASTSemaType GetSemaType() const {
    return SemaTypeExpression;
  }

  virtual void print() const override {
    for (std::list<ASTBlockNode*>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I)
      (*I)->print();
  }

  virtual void push(ASTBase* Node) override {
    Graph.push_back(dynamic_cast<ASTBlockNode*>(Node));
  }
};

typedef std::vector<ASTBlock*> ASTBlockList;

} // namespace QASM

#endif // __QASM_AST_BLOCK_H

