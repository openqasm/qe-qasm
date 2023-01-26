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

#ifndef __QASM_AST_DEFCAL_CONTEXT_BUILDER_H
#define __QASM_AST_DEFCAL_CONTEXT_BUILDER_H

#include <iostream>

namespace QASM {

// Implemented in ASTDefcal.cpp
class ASTDefcalContextBuilder {
private:
  static ASTDefcalContextBuilder DCB;
  static bool DCS;

protected:
  ASTDefcalContextBuilder() = default;

public:
  static ASTDefcalContextBuilder& Instance() {
    return DCB;
  }

  void OpenContext() {
    DCS = true;
  }

  void CloseContext() {
    DCS = false;
  }

  bool InOpenContext() const {
    return DCS;
  }
};

} // namespace QASM

#endif // __QASM_AST_DEFCAL_CONTEXT_BUILDER_H

