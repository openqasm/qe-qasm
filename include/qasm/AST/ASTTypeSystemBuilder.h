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

#ifndef __QASM_AST_TYPE_SYSTEM_BUILDER_H
#define __QASM_AST_TYPE_SYSTEM_BUILDER_H

#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTAngleContextControl.h>
#include <qasm/AST/ASTGateContextBuilder.h>

#include <map>
#include <set>
#include <string>

namespace QASM {

class ASTTypeSystemBuilder {
private:
  static ASTTypeSystemBuilder TSB;
  static std::map<std::string, ASTSymbolScope> LM;
  static std::map<std::string, ASTSymbolScope> GM;
  static std::map<std::string, ASTSymbolScope> FM;
  static std::map<ASTType, unsigned> TBM;
  static std::set<std::string> BFM;
  static std::set<std::string> BGM;
  static std::set<std::string> OQ2RG;
  static std::set<std::string> RS;
  static std::set<std::string> FR;

protected:
  ASTTypeSystemBuilder() = default;
  bool LocateImplicitSymbol(const std::string& S) const;

protected:
  void CreateASTBuiltinCXGate() const;
  void CreateASTBuiltinUGate() const;
  void CreateASTReservedAngles() const;
  void CreateASTReservedMPDecimalValues() const;

public:
  static ASTTypeSystemBuilder& Instance() {
    return TSB;
  }

  static void Init();

  bool IsReservedAngle(const std::string& S) const {
    if (LM.find(S) == LM.end())
      return GM.find(S) != GM.end();

    return true;
  }

  bool IsReservedConstant(const std::string& S) const {
    if (LM.find(S) == LM.end())
      return GM.find(S) != GM.end();

    return true;
  }

  bool IsImplicitAngle(const std::string& S) const {
    if (ASTAngleContextControl::Instance().InOpenContext() ||
        ASTGateContextBuilder::Instance().InOpenContext()) {
      return RS.find(S) != RS.end();
    }

    return IsReservedAngle(S);
  }

  bool IsBuiltinGate(const std::string& S) const {
    return BGM.find(S) != BGM.end();
  }

  bool IsReservedGateV2(const std::string& S) const {
    std::string LS = ASTStringUtils::Instance().ToLower(S);
    return OQ2RG.find(LS) != OQ2RG.end();
  }

  bool IsBuiltinFunction(const std::string& S) const {
    return BFM.find(S) != BFM.end();
  }

  bool IsReservedFunction(const std::string& S) const {
    return FM.find(S) != FM.end();
  }

  bool InvalidBits(unsigned B) const {
    return B == static_cast<unsigned>(~0x0);
  }

  void RegisterFunction(const std::string& FN) {
    FR.insert(FN);
  }

  bool IsRegistered(const std::string& FN) {
    return FR.find(FN) != FR.end();
  }

  unsigned GetTypeBits(ASTType Ty) const;
};

} // namespace QASM

#endif // __QASM_AST_TYPE_SYSTEM_BUILDER_H

