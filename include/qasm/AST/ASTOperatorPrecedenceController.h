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

#ifndef __QASM_AST_OPERATOR_PRECEDENCE_CONTROLLER_H
#define __QASM_AST_OPERATOR_PRECEDENCE_CONTROLLER_H

#include <qasm/AST/ASTTypes.h>

#include <map>
#include <vector>

namespace QASM {

class ASTOperatorPrecedenceController {
private:
  static ASTOperatorPrecedenceController OPC;
  static const std::map<ASTOpType, uint32_t> OPM;

  // Rank Bias.
  static const uint32_t RB = 1000U;

protected:
  ASTOperatorPrecedenceController() = default;

public:
  static ASTOperatorPrecedenceController& Instance() {
    return OPC;
  }

  ~ASTOperatorPrecedenceController() = default;

  uint32_t GetOperatorPrecedence(ASTOpType OTy) const {
    std::map<ASTOpType, uint32_t>::const_iterator MI = OPM.find(OTy);
    return MI == OPM.end() ? static_cast<uint32_t>(~0x0) : (*MI).second;
  }

  void Expand(const ASTBinaryOpNode* BOP,
              std::multimap<uint32_t, ASTVariantOpNode>& MOP) const;

  void Expand(const ASTUnaryOpNode* UOP,
              std::multimap<uint32_t, ASTVariantOpNode>& MOP) const;

  void Expand(const ASTComplexExpressionNode* CEX,
              std::multimap<uint32_t, ASTVariantOpNode>& MOP) const;

  void print(const std::multimap<uint32_t, ASTVariantOpNode>& MOP) const;
};

} // namespace QASM

#endif // __QASM_AST_OPERATOR_PRECEDENCE_CONTROLLER_H

