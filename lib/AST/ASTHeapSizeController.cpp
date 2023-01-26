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

#include <qasm/AST/ASTHeapSizeController.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

namespace QASM {

ASTHeapSizeController ASTHeapSizeController::HSC;

void
ASTHeapSizeController::SetMaxHeapSize(const std::string& S) {
  HeapSizeUnit HSU = ASTHeapSizeController::Indeterminate;
  std::string::size_type IDX;
  std::string Units;

  if ((IDX = S.find("KB")) != std::string::npos) {
    HSU = Kilobytes;
    Units = S.substr(0, IDX);
  } else if ((IDX = S.find("kb")) != std::string::npos) {
    HSU = Kilobytes;
    Units = S.substr(0, IDX);
  } else if ((IDX = S.find("MB")) != std::string::npos) {
    HSU = Megabytes;
    Units = S.substr(0, IDX);
  } else if ((IDX = S.find("mb")) != std::string::npos) {
    HSU = Megabytes;
    Units = S.substr(0, IDX);
  } else if ((IDX = S.find("GB")) != std::string::npos) {
    HSU = Gigabytes;
    Units = S.substr(0, IDX);
  } else if ((IDX = S.find("gb")) != std::string::npos) {
    HSU = Gigabytes;
    Units = S.substr(0, IDX);
  } else {
    HSU = Bytes;
  }

  switch (HSU) {
  case Bytes:
    MaxHeapSize = std::stoul(S);
    break;
  case Kilobytes:
    MaxHeapSize = std::stoul(Units);
    MaxHeapSize *= 1024;
    break;
  case Megabytes:
    MaxHeapSize = std::stoul(Units);
    MaxHeapSize *= 1024 * 1024;
    break;
  case Gigabytes:
    MaxHeapSize = std::stoul(Units);
    MaxHeapSize *= 1024 * 1024 * 1024;
    break;
  default:
    MaxHeapSize = std::stoul(S);
    break;
  }
}

} // namespace QASM


