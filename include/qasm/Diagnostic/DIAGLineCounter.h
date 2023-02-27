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

#ifndef __QASM_DIAG_LINE_COUNTER_H
#define __QASM_DIAG_LINE_COUNTER_H

#include <string>
#include <sstream>
#include <vector>
#include <map>

namespace QASM {

class ASTBase;
class ASTLocation;
class ASTToken;

class DIAGLineCounter {
private:
  static DIAGLineCounter DLC;
  static uint64_t ILC;
  static ASTLocation NoLocation;

  std::string File;
  uint32_t LineNo;
  uint32_t ColNo;

  struct LineLoc {
    const char* Filename;
    uint32_t Line;
  };

  std::map<uint64_t, LineLoc> OMAP;

protected:
  DIAGLineCounter() : File(""), LineNo(1U), ColNo(1U), OMAP() { }

public:
  static DIAGLineCounter& Instance() {
    return DLC;
  }

  void SetLineCount(uint32_t V) {
    LineNo = V;
  }

  void SetColCount(uint32_t V) {
    ColNo = V;
  }

  void IncrementLine() {
    ++LineNo;
  }

  void IncrementCol() {
    ++ColNo;
  }

  void SetLocation(uint32_t L, uint32_t C) {
    LineNo = L;
    ColNo = C;
  }

  void SetTranslationUnit(const std::string& TU) {
    File = TU;
  }

  void SetTranslationUnit(const char* TU) {
    File = TU;
  }

  void RecordLineOffset(const char* FN, uint64_t OFF, uint32_t LN) {
    LineLoc LL = { FN, LN };
    OMAP.insert(std::make_pair(OFF, LL));
  }

  void ResetCol() {
    ColNo = 1U;
  }

  void ResetLine() {
    LineNo = 1U;
  }

  uint32_t GetLineCount() const {
    return LineNo;
  }

  uint32_t GetColCount() const {
    return ColNo;
  }

  const std::string& GetTranslationUnit() const {
    return File;
  }

  const ASTLocation&  GetLocation() const;

  const ASTLocation& GetLocation(const ASTBase* LB) const;

  const ASTLocation& GetLocation(const ASTBase& LB) const;

  const ASTLocation& GetLocation(const ASTToken* TK) const;

  const ASTLocation& GetLocation(const ASTToken& TK) const;

  bool isNoLocation(const ASTLocation& Loc) const;

  std::string GetIdentifierLocation() const {
    std::stringstream S;
    S << LineNo << '-' << ColNo << '-' << ++ILC;
    return S.str();
  }

  std::string GetIdentifierLocation(const ASTBase* LB) const;

  std::string GetIdentifierLocation(const ASTToken* TK) const;
};

} // namespace QASM

#endif // __QASM_DIAG_LINE_COUNTER_H

