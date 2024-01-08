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

#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTTypes.h>

namespace QASM {

bool ASTBinaryOpNode::IsIntegerConstantExpression() const {
  bool LI = Left->GetASTType() == ASTTypeInt ||
            Left->GetASTType() == ASTTypeMPInteger;
  bool RI = Right->GetASTType() == ASTTypeInt ||
            Right->GetASTType() == ASTTypeMPInteger;

  if (LI && RI)
    return true;

  if (Left->IsIdentifier()) {
    const ASTIdentifierNode *Id = Left->GetIdentifier();
    assert(Id && "LHS did not provide a valid ASTIdentifierNode!");

    LI = Left->GetASTType() == ASTTypeInt ||
         Left->GetASTType() == ASTTypeMPInteger;

    if (!LI) {
      ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().Lookup(Id);
      assert(STE && "Invalid SymbolTable Entry for LHS ASTIdentifierNode!");

      if (STE)
        LI = STE->GetValueType() == ASTTypeInt ||
             STE->GetValueType() == ASTTypeMPInteger;
    }
  } else {
    switch (Left->GetASTType()) {
    case ASTTypeInt:
    case ASTTypeMPInteger:
      LI = true;
    default:
      break;
    }
  }

  if (Right->IsIdentifier()) {
    const ASTIdentifierNode *Id = Right->GetIdentifier();
    assert(Id && "RHS did not provide a valid ASTIdentifierNode!");

    RI = Right->GetASTType() == ASTTypeInt ||
         Right->GetASTType() == ASTTypeMPInteger;

    if (!RI) {
      ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().Lookup(Id);
      assert(STE && "Invalid SymbolTable Entry for RHS ASTIdentifierNode!");

      if (STE)
        RI = STE->GetValueType() == ASTTypeInt ||
             STE->GetValueType() == ASTTypeMPInteger;
    }
  } else {
    switch (Right->GetASTType()) {
    case ASTTypeInt:
    case ASTTypeMPInteger:
      RI = true;
    default:
      break;
    }
  }

  return LI && RI;
}

bool ASTUnaryOpNode::IsIntegerConstantExpression() const {
  bool RI = Right->GetASTType() == ASTTypeInt ||
            Right->GetASTType() == ASTTypeMPInteger;

  if (RI)
    return true;

  if (Right->IsIdentifier()) {
    const ASTIdentifierNode *Id = Right->GetIdentifier();
    assert(Id && "RHS did not provide a valid ASTIdentifierNode!");

    RI = Right->GetASTType() == ASTTypeInt ||
         Right->GetASTType() == ASTTypeMPInteger;

    if (!RI) {
      ASTSymbolTableEntry *STE = ASTSymbolTable::Instance().Lookup(Id);
      assert(STE && "Invalid SymbolTable Entry for RHS ASTIdentifierNode!");

      if (STE)
        RI = STE->GetValueType() == ASTTypeInt ||
             STE->GetValueType() == ASTTypeMPInteger;
    }
  } else {
    switch (Right->GetASTType()) {
    case ASTTypeInt:
    case ASTTypeMPInteger:
      RI = true;
    default:
      break;
    }
  }

  return RI;
}

} // namespace QASM
