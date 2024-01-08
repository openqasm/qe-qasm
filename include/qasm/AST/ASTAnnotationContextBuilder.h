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

#ifndef __QASM_AST_ANNOTATION_CONTEXT_BUILDER_H
#define __QASM_AST_ANNOTATION_CONTEXT_BUILDER_H

#include <iostream>
#include <string>
#include <vector>

namespace QASM {

// Implemented in ASTAnnotation.cpp
class ASTAnnotationContextBuilder {
private:
  static ASTAnnotationContextBuilder ACB;
  static bool ACS;
  static std::vector<std::string> AXV;

protected:
  ASTAnnotationContextBuilder() = default;

public:
  static ASTAnnotationContextBuilder &Instance() { return ACB; }

  void OpenContext() { ACS = true; }

  void CloseContext() {
    ACS = false;
    AXV.clear();
  }

  void AddDirective(const char *SB) {
    if (SB && *SB && (*SB != u8' ' && *SB != u8'\n' && *SB != u8'\t')) {
      std::stringstream SS;
      SS << SB;
      AXV.push_back(SS.str());
    }
  }

  void AddDirective(const std::string &S) { AXV.push_back(S); }

  void Clear() { AXV.clear(); }

  const std::vector<std::string> &GetDirectiveVector() const { return AXV; }

  bool InOpenContext() const { return ACS; }

  void print_vector() const {
    std::cout << "<AnnotationDirectives>" << std::endl;

    for (std::vector<std::string>::const_iterator I = AXV.begin();
         I != AXV.end(); ++I) {
      std::cout << "<Directive>" << (*I) << "</Directive>" << std::endl;
    }

    std::cout << "</AnnotationDirectives>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_ANNOTATION_CONTEXT_BUILDER_H
