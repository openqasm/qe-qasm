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

#ifndef __QASM_AST_VALUE_H
#define __QASM_AST_VALUE_H

#include <qasm/AST/ASTAnyType.h>

#include <functional>
#include <type_traits>
#include <typeindex>
#include <typeinfo>

namespace QASM {

template <typename __Type = std::any>
class ASTValue : public ASTBase {
  friend class ASTSymbolTableEntry;

private:
  QASM::ASTAnyType Value;
  mutable bool Error;

private:
  ASTValue() = delete;

public:
  explicit ASTValue(const __Type &V, ASTType VT)
      : ASTBase(), Value(V, VT), Error(false) {}

  explicit ASTValue(const ASTValue &RHS)
      : ASTBase(RHS), Value(RHS.Value), Error(RHS.Error) {}

  virtual ~ASTValue() = default;

  ASTValue &operator=(const ASTValue &RHS) {
    if (this != &RHS) {
      ASTBase::operator=(RHS);
      try {
        Value.first.emplace<__Type>(RHS.Value.first);
        Value.second = RHS.Value.second;
        Error = RHS.Error;
      } catch (const std::bad_any_cast &E) {
        Value.first.reset();
        Value.second = ASTTypeUndefined;
        Error = true;
      }
    }

    return *this;
  }

  virtual bool HasError() const { return Error; }

  virtual inline operator bool() const { return !HasError(); }

  virtual ASTType GetASTType() const override { return Value.second; }

  virtual void SetASTType(ASTType Ty) { Value.second = Ty; }

  virtual std::any &AsAny() { return Value.first; }

  virtual const std::any &AsAny() const { return Value.first; }

  template <typename __ConvertTo>
  __ConvertTo GetValue() const {
    try {
      return std::any_cast<__ConvertTo>(Value.first);
    } catch (const std::bad_any_cast &E) {
      // FIXME: Use Diagnostic subsystem.
      std::cerr << "ASTValue caught " << E.what() << std::endl;
      Error = true;
    }

    return __ConvertTo();
  }

  virtual void SetValue(const __Type &V, ASTType Ty) {
    Value.first = std::any(V);
    Value.second = Ty;
    Error = false;
  }

  template <typename __Other>
  bool IsConvertible(const ASTValue<__Other> &) const {
    return std::is_convertible<__Type, __Other>::value;
  }

  virtual void push(ASTBase * /* unused */) override {}

  virtual void print() const override {}
};

} // namespace QASM

#endif // __QASM_AST_VALUE_H
