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

#ifndef __QASM_FILE_CLEANER_H
#define __QASM_FILE_CLEANER_H

#include <string>
#include <vector>

namespace QASM {

class QasmPPFileCleaner {
private:
  static QasmPPFileCleaner QFC;

  std::vector<std::string> Files;
  bool KeepTemps;

private:
  QasmPPFileCleaner() : Files(), KeepTemps(false) {}

public:
  static QasmPPFileCleaner &Instance() { return QFC; }

  ~QasmPPFileCleaner();

  void RegisterFile(const std::string &Path) { Files.push_back(Path); }

  void SetKeepTemps(bool V) { KeepTemps = V; }
};

} // namespace QASM

#endif // __QASM_FILE_CLEANER_H
