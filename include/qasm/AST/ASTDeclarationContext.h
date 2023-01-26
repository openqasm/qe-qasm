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

#ifndef __QASM_AST_DECLARATION_CONTEXT_H
#define __QASM_AST_DECLARATION_CONTEXT_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTStringUtils.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <vector>
#include <cstdio>
#include <cassert>

namespace QASM {

class ASTDeclarationContext : public ASTBase {
private:
  std::string DCS;
  size_t Hash;
  unsigned IX;
  mutable ASTType CTy;
  mutable ASTScopeState SCS;
  const ASTDeclarationContext* PCX;
  mutable std::map<const ASTBase*, ASTType> STM;

private:
  ASTDeclarationContext() = delete;

public:
  using map_type = std::map<const ASTBase*, ASTType>;
  using iterator = typename map_type::iterator;
  using const_iterator = typename map_type::const_iterator;

public:
  ASTDeclarationContext(const std::string& S, unsigned Idx,
                        const ASTDeclarationContext* Parent = nullptr)
  : ASTBase(), DCS(S), Hash(std::hash<std::string>{}(S)), IX(Idx),
  CTy(ASTTypeUndefined), SCS(Alive), PCX(Parent), STM() { }

  ASTDeclarationContext(const std::string& S, unsigned Idx, ASTType CXTy,
                        const ASTDeclarationContext* Parent = nullptr)
  : ASTBase(), DCS(S), Hash(std::hash<std::string>{}(S)), IX(Idx),
  CTy(CXTy), SCS(Alive), PCX(Parent), STM() { }

  virtual ~ASTDeclarationContext() = default;

  ASTType GetASTType() const override {
    return ASTTypeDeclarationContext;
  }

  const std::string& GetString() const {
    return DCS;
  }

  const std::string& GetName() const {
    return DCS;
  }

  size_t GetHash() const {
    return Hash;
  }

  unsigned GetIndex() const {
    return IX;
  }

  const ASTDeclarationContext* GetParentContext() const {
    return PCX;
  }

  bool RegisterSymbol(const ASTBase* X, ASTType Ty) {
    assert(X && "Invalid ASTBase argument!");
    return STM.insert(std::make_pair(X, Ty)).second;
  }

  bool RegisterSymbol(const ASTBase* X, ASTType Ty) const {
    assert(X && "Invalid ASTBase argument!");
    return STM.insert(std::make_pair(X, Ty)).second;
  }

  void UnregisterSymbol(const ASTBase* X) {
    STM.erase(X);
  }

  void UnregisterSymbol(const ASTBase* X) const {
    STM.erase(X);
  }

  bool HasSymbol(const ASTBase* XS) const {
    std::map<const ASTBase*, ASTType>::const_iterator I = STM.find(XS);
    return I != STM.end();
  }

  const std::map<const ASTBase*, ASTType>& GetSymbolTable() const {
    return STM;
  }

  void SetContextType(ASTType Ty) const {
    assert(IX && "Cannot change the Context Type of the Global Context!");
    CTy = Ty;
  }

  ASTType GetContextType() const {
    return CTy;
  }

  void SetDead() const {
    if (IX > 0 && IX != static_cast<unsigned>(~0x0))
      SCS = Dead;
  }

  void SetAlive() const {
    if (IX > 0)
      SCS = Alive;
  }

  ASTScopeState GetScopeState() const {
    return SCS;
  }

  bool IsDead() const {
    return SCS == Dead;
  }

  bool IsAlive() const {
    return SCS == Alive;
  }

  iterator begin() {
    return STM.begin();
  }

  const_iterator begin() const {
    return STM.begin();
  }

  iterator end() {
    return STM.end();
  }

  const_iterator end() const {
    return STM.end();
  }

  virtual void print() const override {
    std::cout << "<DeclarationContext>" << std::endl;
    std::cout << "<String>" << DCS << "</String>" << std::endl;
    std::cout << "<Hash>" << std::hex << std::showbase << Hash
      << std::internal << "</Hash>" << std::endl;
    std::cout << std::dec << std::internal;
    std::cout << "<Index>" << IX << "</Index>" << std::endl;
    std::cout << "<ScopeState>" << PrintScopeState(SCS)
      << "</ScopeState>" << std::endl;
    std::cout << "<DeclarationContextType>" << PrintTypeEnum(CTy)
      << "</DeclarationContextType>" << std::endl;
    if (PCX) {
      std::cout << "<ParentContext>" << PCX->GetString()
        << "</ParentContext>" << std::endl;
      std::cout << "<ParentContextType>" << PrintTypeEnum(PCX->GetContextType())
        << "</ParentContextType>" << std::endl;
      std::cout << "<ParentContextIndex>" << PCX->GetIndex()
        << "</ParentContextIndex>" << std::endl;
    } else {
      std::cout << "<ParentContext>0x0" << "</ParentContext>" << std::endl;
    }
    std::cout << "</DeclarationContext>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTDeclarationContextTracker {
  friend class ASTTypeSystemBuilder;

private:
  // Do NOT change the ordering of these class data members.
  static bool II;
  static unsigned CIX;
  static unsigned RSL;
  static ASTDeclarationContextTracker DCT;
  static const ASTDeclarationContext GCX;
  static const ASTDeclarationContext CCX;
  static std::map<unsigned, const ASTDeclarationContext*> M;
  static std::vector<const ASTDeclarationContext*> CCV;

protected:
  ASTDeclarationContextTracker() = default;

  static void Init();

public:
  static ASTDeclarationContextTracker& Instance() {
    if (!DCT.II) {
      ASTDeclarationContextTracker::Init();
    }

    return ASTDeclarationContextTracker::DCT;
  }

  ~ASTDeclarationContextTracker() = default;

  void PopCurrentContext() {
    if (CCV.size() > 1) {
      CCV.back()->SetDead();
      CCV.pop_back();
    }
  }

  const ASTDeclarationContext* GetGlobalContext() const {
    assert(M.size() >= 1 &&
           "Global Declaration Context is not initialized!");
    assert(CCV.size() >= 1 &&
           "Global Declaration Context is not initialized!");

    return &ASTDeclarationContextTracker::GCX;
  }

  const ASTDeclarationContext* GetDefaultCalibrationContext() const {
    return &ASTDeclarationContextTracker::CCX;
  }

  const ASTDeclarationContext* CreateContext() {
    ASTDeclarationContext* DC =
      new ASTDeclarationContext(ASTStringUtils::Instance().GenRandomString(RSL),
                                CIX++, GetCurrentContext());
    assert(DC && "Could not create a valid ASTDeclarationContext!");

    M.insert(std::make_pair(DC->GetIndex(), DC));
    CCV.push_back(DC);
    return DC;
  }

  const ASTDeclarationContext* CreateContext(ASTType CTy) {
    ASTDeclarationContext* DC =
      new ASTDeclarationContext(ASTStringUtils::Instance().GenRandomString(RSL),
                                CIX++, CTy, GetCurrentContext());
    assert(DC && "Could not create a valid ASTDeclarationContext!");

    M.insert(std::make_pair(DC->GetIndex(), DC));
    CCV.push_back(DC);
    return DC;
  }

  const ASTDeclarationContext* GetCurrentContext() const {
    return CCV.back();
  }

  ASTSymbolScope GetCurrentScope() const {
    if (InGlobalContext())
      return ASTSymbolScope::Global;

    return ASTSymbolScope::Local;
  }

  const ASTDeclarationContext* GetDeclarationContext(unsigned IX) const {
    std::map<unsigned, const ASTDeclarationContext*>::const_iterator I =
      M.find(IX);
    return I == M.end() ? nullptr : (*I).second;
  }

  void SetCalibrationContext() {
    CCV.push_back(&ASTDeclarationContextTracker::CCX);
  }

  bool InCalibrationContext() const {
    return CCV.back() == &ASTDeclarationContextTracker::CCX;
  }

  bool InExternContext() const {
    return CCV.back()->GetContextType() == ASTTypeExtern;
  }

  bool IsCalibrationContext(const ASTDeclarationContext* CX) const {
    return CX == &ASTDeclarationContextTracker::CCX;
  }

  ASTDeclarationContext* GetCurrentContext() {
    return const_cast<ASTDeclarationContext*>(CCV.back());
  }

  bool IsGlobalContext(const ASTDeclarationContext* C) const {
    return C == &ASTDeclarationContextTracker::GCX;
  }

  bool CurrentContextIsGlobal() const {
    return CCV.back() == &ASTDeclarationContextTracker::GCX;
  }

  bool InGlobalContext() const {
    return CCV.back() == &ASTDeclarationContextTracker::GCX;
  }
};

} // namespace QASM

#endif // __QASM_AST_DECLARATION_CONTEXT_H

