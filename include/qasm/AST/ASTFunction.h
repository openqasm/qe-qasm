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

#ifndef __QASM_AST_FUNCTION_H
#define __QASM_AST_FUNCTION_H

#include <qasm/AST/ASTBase.h>

#include <iostream>
#include <string>

namespace QASM {

class ASTFunction : public ASTBase {
private:
  std::string ID;
  const ASTBase* Param;
  const ASTBase* CompoundStatement;

public:
  ASTFunction(const std::string& Id, const ASTBase* P, const ASTBase* CS)
  : ASTBase(), ID(Id), Param(P), CompoundStatement(CS) { }

  virtual void print() const override {
    std::cout << "<Function id=\"" << ID << "\">" << std::endl;
    Param->print();
    CompoundStatement->print();
    std::cout << "</Function>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTParamList {
private:
  std::list<ASTBase*> Graph;

public:
  ASTParamList() : Graph() { }

  ASTParamList(ASTBase* Node) {
    Graph.push_back(Node);
  }

  virtual void print() const {
    for (std::list<ASTBase*>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I)
      (*I)->print();
  }

  virtual void push(ASTBase* Node) {
    Graph.push_back(Node);
  }
};

} // namespace QASM

#endif // __QASM_AST_FUNCTION_H

