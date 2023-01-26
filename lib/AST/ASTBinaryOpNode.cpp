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

#include <qasm/AST/ASTTypes.h>

namespace QASM {

void ASTBinaryOpNode::print() const {
  assert(Left && "Invalid Left Operand to ASTBinaryOpNode!");
  assert(Right && "Invalid Right Operand to ASTBinaryOpNode!");

  std::cout << "<BinaryOpNode>" << std::endl;
  if (Parens)
    std::cout << "<LParen>" << "(" << "</LParen>" << std::endl;

  std::cout << "<Left>" << std::endl;

  if (Left)
    Left->print();

  std::cout << "</Left>" << std::endl;
  std::cout << "<Op>";

  switch (OpType) {
  case ASTOpTypeUndefined:
    std::cout << "Undefined";
    break;
  case ASTOpTypeAdd:
    std::cout << "+";
    break;
  case ASTOpTypeAddAssign:
    std::cout << "+=";
    break;
  case ASTOpTypeSub:
    std::cout << "-";
    break;
  case ASTOpTypeSubAssign:
    std::cout << "-=";
    break;
  case ASTOpTypeMul:
    std::cout << "*";
    break;
  case ASTOpTypeMulAssign:
    std::cout << "*=";
    break;
  case ASTOpTypeDiv:
    std::cout << "/";
    break;
  case ASTOpTypeDivAssign:
    std::cout << "/=";
    break;
  case ASTOpTypeMod:
    std::cout << "%";
    break;
  case ASTOpTypeModAssign:
    std::cout << "%=";
    break;
  case ASTOpTypeLogicalAnd:
    std::cout << "&&";
    break;
  case ASTOpTypeLogicalOr:
    std::cout << "||";
    break;
  case ASTOpTypeBitAnd:
    std::cout << "&";
    break;
  case ASTOpTypeBitAndAssign:
    std::cout << "&=";
    break;
  case ASTOpTypeBitOr:
    std::cout << "|";
    break;
  case ASTOpTypeBitOrAssign:
    std::cout << "|=";
    break;
  case ASTOpTypeXor:
    std::cout << "^";
    break;
  case ASTOpTypeXorAssign:
    std::cout << "^=";
    break;
  case ASTOpTypeAssign:
    std::cout << "=";
    break;
  case ASTOpTypeLT:
    std::cout << "<";
    break;
  case ASTOpTypeGT:
    std::cout << ">";
    break;
  case ASTOpTypeLE:
    std::cout << "<=";
    break;
  case ASTOpTypeGE:
    std::cout << ">=";
    break;
  case ASTOpTypeNegate:
    std::cout << "-";
    break;
  case ASTOpTypeBitNot:
    std::cout << "~";
    break;
  case ASTOpTypeCompEq:
    std::cout << "==";
    break;
  case ASTOpTypeCompNeq:
    std::cout << "!=";
    break;
  case ASTOpTypeLogicalNot:
    std::cout << "!";
    break;
  case ASTOpTypePreInc:
    std::cout << "++[*]";
    break;
  case ASTOpTypePreDec:
    std::cout << "--[*]";
    break;
  case ASTOpTypePostInc:
    std::cout << "[*]++";
    break;
  case ASTOpTypePostDec:
    std::cout << "[*]--";
    break;
  case ASTOpTypePositive:
    std::cout << "+";
    break;
  case ASTOpTypeNegative:
    std::cout << "-";
    break;
  case ASTOpTypePow:
    std::cout << "**";
    break;
  case ASTOpTypeLeftShift:
    std::cout << "<<";
    break;
  case ASTOpTypeRightShift:
    std::cout << ">>";
    break;
  case ASTOpTypeLeftShiftAssign:
    std::cout << "<<=";
    break;
  case ASTOpTypeRightShiftAssign:
    std::cout << ">>=";
    break;
  case ASTOpTypeSin:
    std::cout << "sin";
    break;
  case ASTOpTypeCos:
    std::cout << "cos";
    break;
  case ASTOpTypeTan:
    std::cout << "tan";
    break;
  case ASTOpTypeExp:
    std::cout << "exp";
    break;
  case ASTOpTypeLn:
    std::cout << "ln";
    break;
  case ASTOpTypeSqrt:
    std::cout << "sqrt";
    break;
  default:
    std::cout << "unknown";
    break;
  }

  std::cout << "</Op>" << std::endl;

  std::cout << "<EvalMethod>" << PrintEvalMethod(EM)
    << "</EvalMethod>" << std::endl;

  std::cout << "<Right>" << std::endl;

  if (Right)
    Right->print();

  if (IM)
    IM->print();

  std::cout << "</Right>" << std::endl;

  if (Parens)
    std::cout << "<RParen>" << ")" << "</RParen>" << std::endl;

  std::cout << "</BinaryOpNode>" << std::endl;
}

} // namespace QASM

