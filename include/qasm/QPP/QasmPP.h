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

#ifndef __QASM_PP_H
#define __QASM_PP_H

#include <qasm/QPP/QasmPathsResolver.h>

#include <iostream>
#include <string>
#include <vector>

namespace QASM {

class QasmPreprocessor {
private:
  std::vector<std::string> IncludePaths;
  QasmPathsResolver QPR;
  static QasmPreprocessor QPP;
  static std::string CurrentFilePath;

private:
  QasmPreprocessor();

public:
  static QasmPreprocessor &Instance() { return QPP; }

  virtual ~QasmPreprocessor() = default;

  static const std::string &GetCurrentFilePath() { return CurrentFilePath; }

  void AddIncludePath(const std::string &Path) {
    IncludePaths.push_back(Path);
    QPR.AddIncludePath(Path);
  }

  void SetFilePath(std::string &FilePath);

  void SetTranslationUnit(const std::string &FilePath) {
    QPR.SetTranslationUnit(FilePath);
  }

  void SetTranslationUnit(std::istream *In) { QPR.SetTranslationUnit(In); }

  void ClearTU() { QPR.ClearTU(); }

  std::istream *GetIStream() const { return QPR.GetIStream(); }

  bool IsTU() const { return QPR.IsTU(); }

  bool IsIStream() const { return QPR.IsIStream(); }

  void ParseCommandLineArguments(int argc, char *const argv[]) {
    QPR.ParseCommandLineArguments(argc, argv);
  }

  void PrintIncludePaths() const {
    for (std::vector<std::string>::const_iterator I = IncludePaths.begin();
         I != IncludePaths.end(); ++I)
      std::cout << (*I) << std::endl;
  }

  const QasmPathsResolver &Resolver() const { return QPR; }

  QasmPathsResolver &Resolver() { return QPR; }

  bool Preprocess(std::ifstream &InFile);

  bool Preprocess(std::istream *InStream);

  std::string GenTempFilename(unsigned Length, const char *Suffix = ".qasm");
};

} // namespace QASM

#endif // __QASM_PP_H
