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

#ifndef __QASM_DIAG_LINE_BUFFER_H
#define __QASM_DIAG_LINE_BUFFER_H

#include <qasm/AST/ASTStringUtils.h>

#include <sstream>
#include <string>

namespace QASM {

class DIAGLineBuffer {
private:
  static DIAGLineBuffer DLB;
  std::stringstream S;
  unsigned SZ;
  unsigned CTL;
  unsigned CCP;

protected:
  DIAGLineBuffer() : S(), SZ(0U), CTL(0U), CCP(0U) {}

public:
  static DIAGLineBuffer &Instance() { return DLB; }

  void Append(const char *B) {
    if (B) {
      S << B;
      CTL = ASTStringUtils::Instance().UTF8Len(B);
      SZ += CTL;
      CCP = SZ ? SZ - CTL : 1U;
    } else {
      CTL = 0U;
    }
  }

  void Clear() {
    S.str("");
    S.clear();
    SZ = 0U;
    CTL = 0U;
    CCP = 1U;
  }

  std::string GetBuffer() const { return S.str(); }

  unsigned Size() const { return SZ; }

  unsigned GetTokenLength() const { return CTL; }

  unsigned GetCaretPosition() const { return CCP; }

  std::string GetCaretString() const {
    std::string R(CCP, ' ');
    R += '^';
    return R;
  }
};

} // namespace QASM

#endif // __QASM_DIAG_LINE_BUFFER_H
