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

#ifndef __QASM_AST_TYPE_CAST_CONTROLLER_H
#define __QASM_AST_TYPE_CAST_CONTROLLER_H

#include <qasm/AST/ASTTypes.h>

namespace QASM {

class ASTCastExpressionNode;
class ASTIdentifierNode;
class ASTIdentifierNodeRef;

class ASTTypeCastController {
private:
  static ASTTypeCastController TCC;

protected:
  ASTTypeCastController() = default;

public:
  static ASTTypeCastController &Instance() { return TCC; }

  ~ASTTypeCastController() = default;

  bool CanCast(ASTType From, ASTType To) const;

  bool CanCast(const ASTBinaryOpNode *From, ASTType To) const;

  bool CanCast(const ASTUnaryOpNode *From, ASTType To) const;

  bool CanCast(const ASTIdentifierNode *From, ASTType To) const;

  bool CanCast(const ASTExpressionNode *From, ASTType To) const;

  bool CanCast(const ASTCastExpressionNode *XC) const;

  bool CanImplicitConvert(ASTType From, ASTType To) const;

  bool CanImplicitConvert(const ASTBinaryOpNode *From, ASTType To) const;

  bool CanImplicitConvert(const ASTUnaryOpNode *From, ASTType To) const;

  bool CanImplicitConvert(const ASTIdentifierNode *From, ASTType To) const;

  bool CanImplicitConvert(const ASTExpressionNode *From, ASTType To) const;

  bool CanImplicitConvert(const ASTCastExpressionNode *XC, ASTType To) const;

  ASTTypeConversionMethod ResolveConversionMethod(ASTType From,
                                                  ASTType To) const;

  ASTTypeConversionMethod ResolveConversionMethod(const ASTIdentifierNode *Id,
                                                  ASTType To) const;

  ASTTypeConversionMethod ResolveConversionMethod(const ASTBinaryOpNode *BOP,
                                                  ASTType To) const;

  ASTTypeConversionMethod ResolveConversionMethod(const ASTUnaryOpNode *BOP,
                                                  ASTType To) const;

  ASTTypeConversionMethod ResolveConversionMethod(const ASTExpressionNode *EX,
                                                  ASTType To) const;
};

} // namespace QASM

#endif // __QASM_AST_TYPE_CAST_CONTROLLER_H
