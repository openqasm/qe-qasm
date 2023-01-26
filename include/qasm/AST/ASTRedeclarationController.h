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

#ifndef __QASM_AST_REDECLARATION_CONTROLLER_H
#define __QASM_AST_REDECLARATION_CONTROLLER_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTSymbolTable.h>

#include <cassert>

namespace QASM {

class ASTRedeclarationController {
private:
  // In ASTDeclarationBuilder.cpp.
  static ASTRedeclarationController RDC;
  static bool ARD;

protected:
  ASTRedeclarationController() = default;

public:
  static ASTRedeclarationController& Instance() {
    return RDC;
  }

  ~ASTRedeclarationController() = default;

  bool TypeAllowsRedeclaration(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeBool:
    case ASTTypeBitset:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeFloat:
    case ASTTypeDouble:
    case ASTTypeLongDouble:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
    case ASTTypeMPDecimal:
    case ASTTypeMPComplex:
    case ASTTypeAngle:
      return true;
      break;
    default:
      return false;
      break;
    }

    return false;
  }

  bool AllowsRedeclaration() const {
    return ARD;
  }

  void AllowRedeclarations(bool V) {
    ARD = V;
  }

  bool TypeAllowsRedeclaration(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return TypeAllowsRedeclaration(Id->GetSymbolType());
  }
};

} // namespace QASM

#endif // __QASM_AST_REDECLARATION_CONTROLLER_H

