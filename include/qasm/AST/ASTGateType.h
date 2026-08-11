/* -*- coding: utf-8 -*-
 *
 * Copyright 2023 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_GATE_TYPE_H
#define __QASM_AST_GATE_TYPE_H

#include <qasm/AST/ASTArgument.h>
#include <qasm/AST/ASTTypeEnums.h>

#include <cstddef>
#include <optional>

namespace QASM {

/// Thin classification of classical gate formals / call args for fully-typed
/// gates. FormalParamTypes remains the oracle; this only packages ASTType plus
/// an optional array length for compatibility checks.
class ASTGateType {
private:
  ASTType Ty;
  /// Array length when known; empty means not an array or size unknown.
  std::optional<unsigned> ArraySize;

  ASTGateType(ASTType T, std::optional<unsigned> SZ) : Ty(T), ArraySize(SZ) {}

public:
  ASTGateType() : Ty(ASTTypeUndefined), ArraySize() {}

  ASTType GetType() const { return Ty; }

  bool HasArraySize() const { return ArraySize.has_value(); }

  unsigned GetArraySize() const { return ArraySize.value_or(0U); }

  bool IsRealArrayFamily() const;
  bool IsComplexArray() const;
  bool IsArray() const;
  bool IsComplexScalar() const;
  bool IsRealScalarFamily() const;

  static ASTGateType
  ClassifyFormal(ASTType Ty, std::optional<unsigned> Size = std::nullopt);

  static ASTGateType ClassifyArg(const ASTArgumentNode *Arg);

  /// Named array argument with a declared type but no initializer list.
  static bool ArgIsUninitializedArray(const ASTArgumentNode *Arg);

  /// Identifier for a named call argument, or nullptr.
  static const class ASTIdentifierNode *
  ArgIdentifier(const ASTArgumentNode *Arg);

  /// True when call arg A may bind to formal F.
  /// Real↔angle array family; real→complex scalar and real-array→complex-array
  /// promotion; reject complex→real (scalar or array).
  static bool Compatible(const ASTGateType &F, const ASTGateType &A);

  /// True if any element of a literal AngleArray arg has a complex expression.
  static bool ArgHasComplexElements(const ASTArgumentNode *Arg);

  /// True if any element of a gate array literal list is complex-typed.
  static bool ExpressionListHasComplex(const class ASTExpressionList *EL);
};

} // namespace QASM

#endif // __QASM_AST_GATE_TYPE_H
