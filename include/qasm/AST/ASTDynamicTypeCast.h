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

#ifndef __QASM_AST_DYNAMIC_TYPE_CAST_H
#define __QASM_AST_DYNAMIC_TYPE_CAST_H

#include "qasm/AST/ASTBase.h"
#include "qasm/AST/ASTAnyType.h"

#include <functional>

namespace QASM {

class ASTDynamicTypeCast {
private:
  static ASTDynamicTypeCast DTC;

private:
  ASTDynamicTypeCast() = default;

public:
  static ASTDynamicTypeCast& Instance() {
    return DTC;
  }

  template<typename __To>
  __To* DynCast(ASTBase* From);

  template<typename __To>
  const __To*
  DynCast(const ASTBase* From);

  template<typename __To>
  __To* DynCast(ASTAnyType& From);

  template<typename __To>
  const __To* DynCast(const ASTAnyType& From);
};

} // namespace QASM

#define __QASM_AST_DYNAMIC_TYPE_CAST_CPP
#include "qasm/AST/ASTDynamicTypeCast.inc"
#undef __QASM_AST_DYNAMIC_TYPE_CAST_CPP

#endif // __QASM_AST_DYNAMIC_TYPE_CAST_H

