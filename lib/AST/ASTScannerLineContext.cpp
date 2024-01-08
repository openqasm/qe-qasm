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

#include <qasm/AST/ASTScannerLineContext.h>

#include <iostream>
#include <sstream>

namespace QASM {

ASTScannerLineContext ASTScannerLineContext::LC;

std::vector<std::string> ASTScannerLineContext::CreateParameterList() {
  std::string::size_type SC = Line.find_last_of(';');
  if (SC != std::string::npos) {
    std::string SL = Line.substr(SC + 1);
    Line = SL;
  }

  std::vector<std::string> PL;
  std::string::size_type LP = Line.find_first_of('(');

  if (LP == std::string::npos)
    return PL;

  std::string::size_type RP = Line.find_last_of(')');

  if (RP == std::string::npos)
    return PL;

  if (RP > LP) {
    ++LP;
    --RP;
    std::string S = Line.substr(LP, RP - LP + 1);

    std::istringstream IS(S);
    std::string Tmp;

    while (std::getline(IS, Tmp, ','))
      PL.push_back(Tmp);

    return PL;
  }

  std::cerr << "Error: Unbalanced paren block!" << std::endl;
  return PL;
}

} // namespace QASM
