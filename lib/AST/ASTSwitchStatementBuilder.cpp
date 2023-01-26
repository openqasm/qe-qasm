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

#include <qasm/AST/ASTSwitchStatementBuilder.h>

namespace QASM {

ASTSwitchStatementBuilder ASTSwitchStatementBuilder::SSB;
ASTStatementList ASTSwitchStatementBuilder::SL;
ASTStatementList* ASTSwitchStatementBuilder::SLP;
std::vector<ASTStatementList*> ASTSwitchStatementBuilder::SLV;

ASTSwitchScopedStatementBuilder ASTSwitchScopedStatementBuilder::SSSB;
ASTStatementList ASTSwitchScopedStatementBuilder::SL;
ASTStatementList* ASTSwitchScopedStatementBuilder::SLP;
std::vector<ASTStatementList*> ASTSwitchScopedStatementBuilder::SLV;

ASTSwitchUnscopedStatementBuilder ASTSwitchUnscopedStatementBuilder::SUSB;
ASTStatementList ASTSwitchUnscopedStatementBuilder::SL;
ASTStatementList* ASTSwitchUnscopedStatementBuilder::SLP;
std::vector<ASTStatementList*> ASTSwitchUnscopedStatementBuilder::SLV;

} // namespace QASM

