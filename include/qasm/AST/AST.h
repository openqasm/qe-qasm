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
#ifndef __QASM_AST_H
#define __QASM_AST_H

#include <qasm/AST/ASTArgument.h>
#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTBlock.h>
#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTDeclaration.h>
#include <qasm/AST/ASTDefcal.h>
#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTFunction.h>
#include <qasm/AST/ASTFunctions.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTKernel.h>
#include <qasm/AST/ASTLength.h>
#include <qasm/AST/ASTPrimitives.h>
#include <qasm/AST/ASTProgramBlock.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTRegisters.h>
#include <qasm/AST/ASTReset.h>
#include <qasm/AST/ASTReturn.h>
#include <qasm/AST/ASTRoot.h>
#include <qasm/AST/ASTStatement.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTTypeSpecifier.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTValue.h>

namespace QASM {

ASTRoot *ParseAST();

} // namespace QASM

#endif // __QASM_AST_H
