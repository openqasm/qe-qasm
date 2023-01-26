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

#ifndef __QASM_AST_ROOT_H
#define __QASM_AST_ROOT_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTStatementBuilder.h>

#include <vector>

namespace QASM {

class ASTRoot {
private:
  std::vector<ASTBase*> Graph;

public:
  ASTRoot() : Graph() { }
  virtual ~ASTRoot() = default;

  virtual void print() const {
    QASM::ASTStatementBuilder::Instance().List()->print();
  }

  virtual void push(ASTBase* /* unused */) { }
};

} // namespace QASM

#endif // __QASM_AST_ROOT_H

