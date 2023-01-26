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

#ifndef __QASM_AST_INTEGER_SEQUENCE_H
#define __QASM_AST_INTEGER_SEQUENCE_H

#include <qasm/AST/ASTTypes.h>

#include <deque>

namespace QASM {

class ASTIntegerSequence : public ASTBase {
private:
  std::deque<int32_t> Sequence;

public:
  using sequence_type = std::deque<int32_t>;
  using iterator = typename sequence_type::iterator;
  using const_iterator = typename sequence_type::const_iterator;

public:
  ASTIntegerSequence() : Sequence() { }

  virtual ~ASTIntegerSequence() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeIntegerSequence;
  }

  iterator begin() {
    return Sequence.begin();
  }

  const_iterator begin() const {
    return Sequence.begin();
  }

  iterator end() {
    return Sequence.end();
  }

  const_iterator end() const {
    return Sequence.end();
  }

  virtual size_t Size() const {
    return Sequence.size();
  }

  virtual void Append(int32_t Val) {
    Sequence.push_back(Val);
  }

  virtual void Prepend(int32_t Val) {
    Sequence.push_front(Val);
  }

  int32_t operator[](unsigned Index) const {
    assert(Index < Sequence.size() && "Index is out-of-range!");
    return Sequence[Index];
  }

  virtual void print() const override {
    std::cout << "<IntegerSequence>" << std::endl;
    for (const_iterator I = Sequence.begin(); I != Sequence.end(); ++I)
      std::cout << "<Integer>" << (*I) << "</Integer>" << std::endl;
    std::cout << "</IntegerSequence>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM


#endif // __QASM_AST_INTEGER_SEQUENCE_NODE_H

