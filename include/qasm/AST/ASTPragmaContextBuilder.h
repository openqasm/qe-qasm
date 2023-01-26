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

#ifndef __QASM_AST_PRAGMA_CONTEXT_BUILDER_H
#define __QASM_AST_PRAGMA_CONTEXT_BUILDER_H

#include <iostream>
#include <string>
#include <vector>

namespace QASM {

// Implemented in ASTPragma.cpp
class ASTPragmaContextBuilder {
private:
  static ASTPragmaContextBuilder PCB;
  static bool PCS;
  static std::vector<std::string> PXV;

protected:
  ASTPragmaContextBuilder() = default;

public:
  static ASTPragmaContextBuilder& Instance() {
    return PCB;
  }

  void OpenContext() {
    PCS = true;
  }

  void CloseContext() {
    PCS = false;
    PXV.clear();
  }

  void AddDirective(const char* SB) {
    if (SB && *SB && (*SB != u8' ' && *SB != u8'\n' && *SB != u8'\t')) {
      std::stringstream SS;
      SS << SB;
      PXV.push_back(SS.str());
    }
  }

  const std::vector<std::string>& GetDirectiveVector() const {
    return PXV;
  }

  bool InOpenContext() const {
    return PCS;
  }

  void print_vector() const {
    std::cout << "<PragmaDirectives>" << std::endl;

    for (std::vector<std::string>::const_iterator I = PXV.begin();
         I != PXV.end(); ++I) {
      std::cout << "<Directive>" << (*I) << "</Directive>" << std::endl;
    }

    std::cout << "</PragmaDirectives>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_PRAGMA_CONTEXT_BUILDER_H

