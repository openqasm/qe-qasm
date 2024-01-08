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
#include <qasm/AST/ASTFunctionDefinitionBuilder.h>
#include <qasm/AST/ASTFunctionStatementBuilder.h>
#include <qasm/AST/ASTIntegerListBuilder.h>
#include <qasm/AST/ASTKernelBuilder.h>
#include <qasm/AST/ASTWhileStatementBuilder.h>

namespace QASM {

std::map<std::string, ASTFunctionDefinitionNode *>
    ASTFunctionDefinitionBuilder::FM;

ASTFunctionDefinitionBuilder ASTFunctionDefinitionBuilder::FDB;

ASTFunctionStatementBuilder ASTFunctionStatementBuilder::FSB;

ASTIntegerListBuilder ASTIntegerListBuilder::ILB;
ASTIntegerList ASTIntegerListBuilder::IL;
ASTIntegerList *ASTIntegerListBuilder::ILP;
std::vector<ASTIntegerList *> ASTIntegerListBuilder::ILV;

} // namespace QASM
