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

#include <qasm/Frontend/QasmScanner.h>

namespace QASM {

std::vector<std::string *> ASTScanner::SV;

using token = QASM::Parser::token;
int ASTScanner::start_openqasm = token::TOK_START_OPENQASM;
int ASTScanner::start_openpulse = token::TOK_START_OPENPULSE;

} // namespace QASM
