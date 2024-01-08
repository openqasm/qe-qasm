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

#ifndef __QASM_AST_DRIVER_H
#define __QASM_AST_DRIVER_H

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#include "QasmParser.tab.h"

#include <qasm/Frontend/QasmParser.h>

namespace QASM {

class ASTDriver {
public:
  ASTDriver()
      : P(nullptr), S(nullptr), Chars(0), Words(0), Lines(0), Upper(0),
        Lower(0) {}

  virtual ~ASTDriver() = default;

  ASTDriver(const ASTDriver &RHS) = delete;
  ASTDriver &operator=(const ASTDriver &RHS) = delete;

  int Parse(std::istream &In);
  int Parse(const char *File);

  void IncrementChars() { ++Chars; }
  void IncrementWords() { ++Words; }
  void IncrementWords(const std::string &Word);
  void IncrementLines() { ++Lines; }
  void IncrementUpper() { ++Upper; }
  void IncrementLower() { ++Lower; }

private:
  int ExecParse(std::istream &In);

private:
  std::unique_ptr<Parser> P;
  std::unique_ptr<ASTScanner> S;

  // Statistics
  uint64_t Chars;
  uint64_t Words;
  uint64_t Lines;
  uint64_t Upper;
  uint64_t Lower;
};

} // namespace QASM

#endif // __QASM_AST_DRIVER_H
