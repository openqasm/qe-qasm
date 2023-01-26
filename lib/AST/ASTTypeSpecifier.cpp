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

#include "qasm/AST/ASTTypeEnums.h"
#include "qasm/AST/ASTTypeSpecifier.h"
#include "QasmParser.tab.h"

namespace QASM {

ASTType ASTTypeSpecifierNode::ResolveASTType(int Token) const {
  using token = Parser::token;

  switch (Token) {
  case token::TOK_VOID:
    return ASTTypeVoid;
    break;
  case token::TOK_CHAR:
    return Unsigned ? ASTTypeUnsignedChar : ASTTypeChar;
    break;
  case token::TOK_SHORT:
    return Unsigned ? ASTTypeUnsignedShort : ASTTypeShort;
    break;
  case token::TOK_INT:
    return Unsigned ? ASTTypeUnsignedInt : ASTTypeInt;
    break;
  case token::TOK_LONG:
    return Unsigned ? ASTTypeUnsignedLong : ASTTypeLong;
    break;
  case token::TOK_FLOAT:
    return ASTTypeFloat;
    break;
  case token::TOK_DOUBLE:
    return ASTTypeDouble;
    break;
  case token::TOK_LONG_DOUBLE:
    return ASTTypeLongDouble;
    break;
  case token::TOK_SIGNED:
    return ASTTypeSigned;
    break;
  case token::TOK_UNSIGNED:
    return ASTTypeUnsigned;
    break;
  case token::TOK_BOOL:
    return ASTTypeBool;
    break;
  case token::TOK_COMPLEX:
    return ASTTypeMPComplex;
    break;
  case token::TOK_IMAGINARY:
    return ASTTypeImaginary;
    break;
  case token::TOK_DEFCAL:
    return ASTTypeDefcal;
    break;
  case token::TOK_CREG:
    return ASTTypeCReg;
    break;
  case token::TOK_QREG:
    return ASTTypeQReg;
    break;
  case token::TOK_CNOT:
    return ASTTypeCNotGate;
    break;
  case token::TOK_HADAMARD:
    return ASTTypeHadamardGate;
    break;
  case token::TOK_CCX:
    return ASTTypeCCXGate;
    break;
  case token::TOK_CX:
    return ASTTypeCXGate;
    break;
  case token::TOK_QUBIT:
    return ASTTypeQubit;
    break;
  case token::TOK_QUBITS:
    return ASTTypeQubits;
    break;
  case token::TOK_U:
    return ASTTypeUGate;
    break;
  case token::TOK_GATE:
    return ASTTypeGate;
    break;
  case token::TOK_BARRIER:
    return ASTTypeBarrier;
    break;
  case token::TOK_MEASURE:
    return ASTTypeMeasure;
    break;
  case token::TOK_KERNEL:
    return ASTTypeKernel;
    break;
  case token::TOK_LAMBDA:
    return ASTTypeLambdaAngle;
    break;
  case token::TOK_THETA:
    return ASTTypeThetaAngle;
    break;
  case token::TOK_PHI:
    return ASTTypePhiAngle;
    break;
  case token::TOK_PI:
    return ASTTypePi;
    break;
  case token::TOK_SIN:
    return ASTTypeSin;
    break;
  case token::TOK_COS:
    return ASTTypeCos;
    break;
  case token::TOK_TAN:
    return ASTTypeTan;
    break;
  case token::TOK_EXP:
    return ASTTypeExp;
    break;
  case token::TOK_LN:
    return ASTTypeLn;
    break;
  case token::TOK_SQRT:
    return ASTTypeSqrt;
    break;
  default:
    return ASTTypeUndefined;
    break;
  }

  return ASTTypeUndefined;
}

const char* ASTTypeSpecifierNode::ResolveASTTypeName() const {
  // Workaround CNot, CX and CCX having the same Enum value as CNot.
  if (Type == ASTTypeCCXGate)
    return "ccx";
  else if (Type == ASTTypeCXGate)
    return "cx";

  switch (Type) {
  case ASTTypeVoid:
    return "void";
    break;
  case ASTTypeChar:
    return Unsigned ? "unsigned char" : "char";
    break;
  case ASTTypeShort:
    return Unsigned ? "unsigned short" : "short";
    break;
  case ASTTypeInt:
    return Unsigned ? "unsigned int" : "int";
    break;
  case ASTTypeLong:
    return Unsigned ? "unsigned long" : "long";
    break;
  case ASTTypeFloat:
    return "float";
    break;
  case ASTTypeDouble:
    return "double";
    break;
  case ASTTypeLongDouble:
    return "long double";
    break;
  case ASTTypeSigned:
    return "signed";
    break;
  case ASTTypeUnsigned:
    return "unsigned";
    break;
  case ASTTypeBool:
    return "bool";
    break;
  case ASTTypeMPComplex:
    return "complex";
    break;
  case ASTTypeImaginary:
    return "imaginary";
    break;
  case ASTTypeDefcal:
    return "defcal";
    break;
  case ASTTypeCReg:
    return "creg";
    break;
  case ASTTypeQReg:
    return "qreg";
    break;
  case ASTTypeCNotGate:
    return "cnot";
    break;
  case ASTTypeHadamardGate:
    return "hadamard";
    break;
  case ASTTypeQubit:
    return "qubit";
    break;
  case ASTTypeQubits:
    return "qubits";
    break;
  case ASTTypeUGate:
    return "U";
    break;
  case ASTTypeGate:
    return "gate";
    break;
  case ASTTypeBarrier:
    return "barrier";
    break;
  case ASTTypeMeasure:
    return "measure";
    break;
  case ASTTypeKernel:
    return "kernel";
    break;
  case ASTTypeLambdaAngle:
    return "lambda";
    break;
  case ASTTypeThetaAngle:
    return "theta";
    break;
  case ASTTypePhiAngle:
    return "phi";
    break;
  case ASTTypePi:
    return "pi";
    break;
  case ASTTypeSin:
    return "sin";
    break;
  case ASTTypeCos:
    return "cos";
    break;
  case ASTTypeTan:
    return "tan";
    break;
  case ASTTypeExp:
    return "exp";
    break;
  case ASTTypeLn:
    return "ln";
    break;
  case ASTTypeSqrt:
    return "sqrt";
    break;
  case ASTTypeUndefined:
    return "undefined";
    break;
  default:
    return "undefined";
    break;
  }

  return "undefined";
}

} // namespace QASM

