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

#include <qasm/AST/ASTBase.h>
#include <qasm/QPP/QasmPathsResolver.h>
#include <qasm/QPP/QasmPPFileCleaner.h>
#include <qasm/AST/ASTObjectTracker.h>

#include <iostream>
#include <string>
#include <filesystem>
#include <cstring>

namespace QASM {

void
QasmPathsResolver::ParseCommandLineArguments(int argc,
                                             char* const argv[]) {
  bool push = false;

  for (int I = 1; I < argc; ++I) {
    if ((argv[I][0] == '-') && (argv[I][1] == 'I')) {
      if (argv[I][2] == '.' || argv[I][2] == '/') {
        IncludePaths.push_back(&argv[I][2]);
        push = false;
      } else {
        push = true;
      }
    } else {
      if (push) {
        IncludePaths.push_back(argv[I]);
        push = false;
      } else {
        if (std::strcmp(argv[I], "-keep-temps") == 0)
          QasmPPFileCleaner::Instance().SetKeepTemps(true);
        else if (std::strcmp(argv[I], "-enable-free") == 0)
          ASTObjectTracker::Instance().Enable();
        else
          TU = argv[I] ? argv[I] : "";
      }
    }
  }
}

std::string
QasmPathsResolver::ResolvePath(const std::string& File) const {
  if (File.empty())
    return std::string();

  std::error_code EC;
  unsigned IC = 0;

  for (unsigned I = 0; I < IncludePaths.size(); ++I) {
    std::filesystem::path Path = IncludePaths[I] + "/" + File;
    try {
      EC.clear();
      Path = std::filesystem::canonical(Path, EC);
      if (EC)
        ++IC;
    } catch (const std::filesystem::filesystem_error& E) {
      ++IC;
    } catch ( ... ) {
      ++IC;
    }

    if (std::filesystem::exists(Path))
      return Path;
  }

  if (IC == IncludePaths.size())
    std::cerr << "Error: File '" << File << "' : " << EC.message()
      << "." << std::endl;
  else
    std::cerr << "Error: File '" << File << "' not found." << std::endl;
  return std::string();
}

std::string
QasmPathsResolver::GetNormalizedTU() const {
  if (TU.empty())
    return TU;

  std::filesystem::path Path = TU;
  return Path.lexically_normal();
}

} // namespace QASM

