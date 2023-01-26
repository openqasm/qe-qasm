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

#ifndef __QASM_INCLUDE_PATHS_RESOLVER_H
#define __QASM_INCLUDE_PATHS_RESOLVER_H

#include <iostream>
#include <vector>
#include <string>

namespace QASM {

class QasmPathsResolver {
private:
  std::vector<std::string> IncludePaths;
  std::string TU;
  std::istream* IS;

public:
  QasmPathsResolver() : IncludePaths(), TU(""), IS(nullptr) { }
  QasmPathsResolver(const QasmPathsResolver& RHS)
  : IncludePaths(RHS.IncludePaths), TU(RHS.TU) { }
  virtual ~QasmPathsResolver() = default;
  QasmPathsResolver& operator=(const QasmPathsResolver& RHS) {
    if (this != &RHS) {
      IncludePaths = RHS.IncludePaths;
      TU = RHS.TU;
    }

    return *this;
  }

  void ParseCommandLineArguments(int argc, char* const argv[]);

  void AddIncludePath(const std::string& Path) {
    IncludePaths.push_back(Path);
  }

  void ClearTU() {
    TU = "";
  }

  std::string ResolvePath(const std::string& File) const;

  const std::string& GetTU() const { return TU; }

  std::string GetNormalizedTU() const;

  const std::vector<std::string>& GetIncludePaths() const {
    return IncludePaths;
  }

  void SetTranslationUnit(const std::string& FilePath) {
    TU = FilePath;
  }

  void SetTranslationUnit(std::istream* In) {
    IS = In;
  }

  std::istream* GetIStream() const {
    return IS;
  }

  bool IsTU() const {
    return !TU.empty();
  }

  bool IsIStream() const {
    return IS;
  }

  void PrintIncludePaths() const {
    for (std::vector<std::string>::const_iterator I = IncludePaths.begin();
         I != IncludePaths.end(); ++I)
      std::cout << (*I) << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_INCLUDE_PATHS_RESOLVER_H

