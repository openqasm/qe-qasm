/* -*- coding: utf-8 -*-
 *
 * Copyright 2021 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_BUILTIN_FUNCTIONS_BUILDER_H
#define __QASM_AST_BUILTIN_FUNCTIONS_BUILDER_H

#include <qasm/AST/ASTFunctions.h>

#include <iostream>
#include <string>
#include <map>
#include <cassert>

namespace QASM {
class ASTBuiltinFunctionsBuilder {
private:
  static ASTBuiltinFunctionsBuilder FB;
  static bool IsInit;
  std::map<std::string, ASTFunctionDefinitionNode*> FDEM;
  std::map<std::string, ASTFunctionDeclarationNode*> FDCM;

protected:
  ASTBuiltinFunctionsBuilder() = default;

protected:
  ASTFunctionDefinitionNode*
  CreateBuiltinFunction(const std::string& Name) const;

public:
  static ASTBuiltinFunctionsBuilder& Instance() {
    return FB;
  }

  virtual ~ASTBuiltinFunctionsBuilder() = default;

  void Init();

  bool AddBuiltinFunction(ASTFunctionDefinitionNode* F);

  bool IsBuiltinFunction(const std::string& S) {
    return FDEM.find(S) != FDEM.end();
  }

  ASTFunctionDefinitionNode* GetBuiltinFunction(const std::string& S) {
    std::map<std::string, ASTFunctionDefinitionNode*>::iterator I =
      FDEM.find(S);
    return I == FDEM.end() ? nullptr : (*I).second;
  }

  ASTFunctionDeclarationNode*
  GetBuiltinFunctionDeclaration(const std::string& S) {
    std::map<std::string, ASTFunctionDeclarationNode*>::iterator I =
      FDCM.find(S);
    return I == FDCM.end() ? nullptr : (*I).second;
  }

  void print() const {
    std::cout << "<BuiltinFunctionsBuilder>" << std::endl;
    for (std::map<std::string,
                  ASTFunctionDefinitionNode*>::const_iterator I = FDEM.begin();
         I != FDEM.end(); ++I)
      (*I).second->print();
    std::cout << "</BuiltinFunctionsBuilder>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_BUILTIN_FUNCTIONS_BUILDER_H

