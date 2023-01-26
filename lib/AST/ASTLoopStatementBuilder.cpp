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

#include <qasm/AST/ASTForStatementBuilder.h>
#include <qasm/AST/ASTWhileStatementBuilder.h>
#include <qasm/AST/ASTDoWhileStatementBuilder.h>

namespace QASM {

// ASTForStatementBuilder:
ASTForStatementBuilder ASTForStatementBuilder::FSB;
ASTStatementList ASTForStatementBuilder::SL;
ASTStatementList* ASTForStatementBuilder::SLP;
std::vector<ASTStatementList*> ASTForStatementBuilder::SLV;

// ASTWhileStatementBuilder:
ASTWhileStatementBuilder ASTWhileStatementBuilder::WSB;
ASTStatementList ASTWhileStatementBuilder::SL;
ASTStatementList* ASTWhileStatementBuilder::SLP;
std::vector<ASTStatementList*> ASTWhileStatementBuilder::SLV;

// ASTDoWhileStatementBuilder:
ASTDoWhileStatementBuilder ASTDoWhileStatementBuilder::DWSB;
ASTStatementList ASTDoWhileStatementBuilder::SL;
ASTStatementList* ASTDoWhileStatementBuilder::SLP;
std::vector<ASTStatementList*> ASTDoWhileStatementBuilder::SLV;

} // namespace QASM

