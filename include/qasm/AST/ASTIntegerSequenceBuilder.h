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

#ifndef __QASM_AST_INTEGER_SEQUENCE_BUILDER_H
#define __QASM_AST_INTEGER_SEQUENCE_BUILDER_H

#include <qasm/AST/ASTIntegerSequence.h>

namespace QASM {

class ASTIntegerSequenceBuilder {
private:
  static ASTIntegerSequenceBuilder ISB;
  ASTIntegerSequence *CSEQ;

private:
  ASTIntegerSequenceBuilder() : CSEQ(nullptr) {}

public:
  static ASTIntegerSequenceBuilder &Instance() { return ISB; }

  ASTIntegerSequence *NewSequence() {
    CSEQ = new ASTIntegerSequence();
    return CSEQ;
  }

  ASTIntegerSequence *Sequence() const { return CSEQ; }

  void Clear() { CSEQ = nullptr; }

  void Append(int32_t Val) {
    assert(CSEQ && "Invalid Current ASTIntegerSequence!");
    CSEQ->Append(Val);
  }

  void Prepend(int32_t Val) {
    assert(CSEQ && "Invalid Current ASTIntegerSequence!");
    CSEQ->Prepend(Val);
  }
};

} // namespace QASM

#endif // __QASM_AST_INTEGER_SEQUENCE_BUILDER_H
