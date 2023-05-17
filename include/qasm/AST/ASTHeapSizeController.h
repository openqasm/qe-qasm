/* -*- coding: utf-8 -*-
 *
 * Copyright 2022, 2023 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_HEAP_SIZE_CONTROLLER_H
#define __QASM_AST_HEAP_SIZE_CONTROLLER_H

#include <cstdint>
#include <string>

namespace QASM {

class ASTHeapSizeController {
private:
  static ASTHeapSizeController HSC;
  uint64_t MaxHeapSize; // Expressed in bytes.

  enum HeapSizeUnit : unsigned {
    Indeterminate = 0,
    Kilobytes,
    Megabytes,
    Gigabytes,
    Bytes
  };

protected:
  ASTHeapSizeController() : MaxHeapSize(17179869184UL) { }

public:
  static ASTHeapSizeController& Instance() {
    return HSC;
  }

  ~ASTHeapSizeController() = default;

  // Expressed in bytes.
  void SetMaxHeapSize(uint64_t Size) {
    MaxHeapSize = Size;
  }

  void SetMaxHeapSize(const std::string& Size);

  uint64_t GetMaxHeapSize() const {
    return MaxHeapSize;
  }
};

} // namespace QASM

#endif // __QASM_AST_HEAP_SIZE_CONTROLLER_H

