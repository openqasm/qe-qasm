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

#ifndef __QASM_AST_GATE_CONTEXT_BUILDER_H
#define __QASM_AST_GATE_CONTEXT_BUILDER_H

#include <iostream>

namespace QASM {

class ASTToken;

// Implemented in ASTGates.cpp
class ASTGateContextBuilder {
private:
  static ASTGateContextBuilder GCB;
  static bool GCS;
  static unsigned CMDepth;
  static const ASTToken *CMTok;

protected:
  ASTGateContextBuilder() = default;

public:
  static ASTGateContextBuilder &Instance() { return GCB; }

  void OpenContext() { GCS = true; }

  void CloseContext() { GCS = false; }

  bool InOpenContext() const { return GCS; }

  void EnterControlModifier(const ASTToken *TK) {
    ++CMDepth;
    CMTok = TK;
  }

  void ExitControlModifier() {
    if (CMDepth)
      --CMDepth;
    if (!CMDepth)
      CMTok = nullptr;
  }

  bool InControlModifier() const { return CMDepth > 0; }

  const ASTToken *GetControlModifierToken() const { return CMTok; }
};

} // namespace QASM

#endif // __QASM_AST_GATE_CONTEXT_BUILDER_H
