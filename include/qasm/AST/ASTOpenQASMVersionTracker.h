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

#ifndef __QASM_AST_OPENQASM_VERSION_TRACKER_H
#define __QASM_AST_OPENQASM_VERSION_TRACKER_H

#include <string>

namespace QASM {

class ASTOpenQASMVersionTracker {
private:
  double Version;
  double DefaultVersion;

private:
  static ASTOpenQASMVersionTracker OQT;

private:
  ASTOpenQASMVersionTracker()
  : Version(0.0), DefaultVersion(3.0) { }

public:
  ~ASTOpenQASMVersionTracker() = default;

  static ASTOpenQASMVersionTracker& Instance() {
    return OQT;
  }

  bool ParseOpenQASMLine(std::string& L, std::string& OQ,
                         std::string& OV, bool& SOQ, bool OQR = false);

  double GetVersion() const {
    return Version;
  }

  double GetDefaultVersion() const {
    return DefaultVersion;
  }

  void SetVersion(double D = 3.0) {
    Version = D;
  }
};

} // namespace QASM

#endif // __QASM_AST_OPENQASM_VERSION_TRACKER_H


