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

#include <qasm/Frontend/QasmDriver.h>
#include <qasm/Frontend/QasmScanner.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <fstream>
#include <sstream>
#include <cassert>
#include <cstddef>

namespace QASM {

int ASTDriver::ExecParse(std::istream& In) {
  if (!In.good()) {
    std::cerr << "Error: Bad Translation Unit!" << std::endl;
    return 1;
  }

  if (In.eof()) {
    std::cerr << "Error: Translation Unit is already at EOF!" << std::endl;
    return 1;
  }

  In.rdbuf()->pubseekpos(0, In.in);

  S.reset();
  S = std::make_unique<ASTScanner>(&In);

  P.reset();
  P = std::make_unique<Parser>(*S, *this);

  int R = P->parse();

  if (R != 0 || QasmDiagnosticEmitter::Instance().HasErrors()) {
    std::stringstream M;
    M << "Compilation terminated with "
      << QasmDiagnosticEmitter::Instance().GetNumErrors() << " errors.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      QASM::DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Error);
  } else {
    std::cout << "OK." << std::endl;
  }

  return R;
}

int ASTDriver::Parse(std::istream& In) {
  if (!In.good()) {
    std::cerr << "Error: Bad file!" << std::endl;
    return 1;
  }

  if (In.eof()) {
    std::cerr << "Error: Already at EOF?" << std::endl;
    return 1;
  }

  return ExecParse(In);
}

int ASTDriver::Parse(const char* InFile) {
  if (!InFile)
    return 1;

  if (!*InFile)
    return 1;

  std::ifstream In(InFile);
  return Parse(In);
}

void ASTDriver::IncrementWords(const std::string& Word) {
  Chars += Word.size();
  IncrementWords();

  const char* W = Word.c_str();
  if (std::isupper(W[0]))
    ++Upper;
  else
    ++Lower;
}

} // namespace QASM

