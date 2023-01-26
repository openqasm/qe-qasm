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

#ifndef __QASM_AST_SYMBOL_TABLE_H
#define __QASM_AST_SYMBOL_TABLE_H

#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTValue.h>
#include <qasm/AST/ASTObjectTracker.h>
#include <qasm/AST/ASTTypeSystemBuilder.h>
#include <qasm/AST/ASTAngleContextControl.h>
#include <qasm/AST/ASTDeclarationList.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTUtils.h>
#include <qasm/AST/ASTStringUtils.h>
#include <qasm/AST/ASTIdentifierTypeController.h>
#include <qasm/AST/ASTCalContextBuilder.h>
#include <qasm/AST/ASTFunctionContextBuilder.h>
#include <qasm/AST/ASTKernelContextBuilder.h>
#include <qasm/AST/ASTExpressionValidator.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <cassert>

namespace QASM {

class ASTSymbolTableEntry {
  friend class ASTSymbolTable;

protected:
  union {
    const ASTIdentifierNode* Ident;
    const ASTIdentifierRefNode* IdentRef;
  };

  // Identifier type: ASTIdentifierNode or ASTIdentifierRefNode.
  ASTType ITy;
  ASTValue<>* Value;
  const ASTDeclarationContext* CTX;
  ASTSymbolScope Scope;
  ASTType ValueType;
  bool DoNotDelete;

protected:
  inline ASTSymbolScope ResolveScope() const {
    if (ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal())
      return ASTSymbolScope::Global;

    return ASTSymbolScope::Local;
  }

  inline const ASTDeclarationContext* ResolveContext() const {
    return ASTDeclarationContextTracker::Instance().GetCurrentContext();
  }

public:
  ASTSymbolTableEntry()
  : Ident(nullptr), ITy(ASTTypeUndefined), Value(nullptr), CTX(ResolveContext()),
  Scope(ResolveScope()), ValueType(ASTTypeUndefined), DoNotDelete(false) { }

  ASTSymbolTableEntry(const ASTSymbolTableEntry& RHS)
  : Ident(RHS.Ident), ITy(RHS.ITy), Value(RHS.Value), CTX(RHS.CTX),
  Scope(RHS.Scope), ValueType(RHS.ValueType), DoNotDelete(RHS.DoNotDelete) { }

  ASTSymbolTableEntry(const ASTIdentifierNode* Id)
  : Ident(Id), ITy(ASTTypeIdentifier), Value(nullptr), CTX(ResolveContext()),
  Scope(ResolveScope()), ValueType(ASTTypeUndefined), DoNotDelete(false) { }

  ASTSymbolTableEntry(const ASTIdentifierRefNode* IdR)
  : IdentRef(IdR), ITy(ASTTypeIdentifierRef), Value(nullptr), CTX(ResolveContext()),
  Scope(ResolveScope()), ValueType(ASTTypeUndefined), DoNotDelete(false) { }

  ASTSymbolTableEntry(const ASTIdentifierNode* Id, ASTType Ty)
  : Ident(Id), ITy(ASTTypeIdentifier), Value(nullptr), CTX(ResolveContext()),
  Scope(ResolveScope()), ValueType(Ty), DoNotDelete(false) { }

  ASTSymbolTableEntry(const ASTIdentifierRefNode* IdR, ASTType Ty)
  : IdentRef(IdR), ITy(ASTTypeIdentifierRef), Value(nullptr), CTX(ResolveContext()),
  Scope(ResolveScope()), ValueType(Ty), DoNotDelete(false) { }

  ASTSymbolTableEntry(const ASTIdentifierNode* Id, ASTValue<>* V, ASTType Ty)
  : Ident(Id), ITy(ASTTypeIdentifier), Value(V), CTX(ResolveContext()),
  Scope(ResolveScope()), ValueType(Ty), DoNotDelete(false) { }

  ASTSymbolTableEntry(const ASTIdentifierRefNode* IdR, ASTValue<>* V, ASTType Ty)
  : IdentRef(IdR), ITy(ASTTypeIdentifierRef), Value(V), CTX(ResolveContext()),
  Scope(ResolveScope()), ValueType(Ty), DoNotDelete(false) { }

  virtual ~ASTSymbolTableEntry() = default;

  ASTSymbolTableEntry& operator=(const ASTSymbolTableEntry& RHS) {
    if (this != &RHS) {
      if (ITy == RHS.ITy) {
        ITy = RHS.ITy;
        if (ITy == ASTTypeIdentifier)
          Ident = RHS.Ident;
        else
          IdentRef = RHS.IdentRef;
      } else {
        ITy = RHS.ITy;
        if (RHS.ITy == ASTTypeIdentifierRef)
          IdentRef = RHS.IdentRef;
        else if (RHS.ITy == ASTTypeIdentifier)
          Ident = RHS.Ident;
        else
          Ident = RHS.Ident;
      }

      Value = RHS.Value;
      CTX = RHS.CTX;
      Scope = RHS.Scope;
      ValueType = RHS.ValueType;
      DoNotDelete = RHS.DoNotDelete;
    }

    return *this;
  }

  virtual ASTType GetIdentifierType() const {
    return ITy;
  }

  virtual bool HasType(ASTType Ty) const {
    return ValueType == Ty;
  }

  virtual bool HasMap() const {
    return false;
  }

  virtual bool HasValue() const {
    return Value != nullptr;
  }

  virtual void SetValue(ASTValue<>* V, ASTType Ty, bool DND = false) {
    Value = V;
    ValueType = Ty;
    DoNotDelete = DND;

    if (Ident->IsReference()) {
      if (const ASTIdentifierRefNode* IdR =
        dynamic_cast<const ASTIdentifierRefNode*>(Ident))
        const_cast<ASTIdentifierRefNode*>(IdR)->SetSymbolTableEntry(this);
    } else {
      const_cast<ASTIdentifierNode*>(Ident)->SetSymbolTableEntry(this);
    }
  }

  virtual void ResetValue() {
    if (Value &&
        (ASTTypeSystemBuilder::Instance().IsReservedAngle(Ident->GetName()) ||
         ASTTypeSystemBuilder::Instance().IsImplicitAngle(Ident->GetName()) ||
         ASTTypeSystemBuilder::Instance().IsBuiltinFunction(Ident->GetName()) ||
         ASTTypeSystemBuilder::Instance().IsBuiltinGate(Ident->GetName()))) {
      return;
    }

    if (!DoNotDelete) {
      ASTObjectTracker::Instance().Unregister(Value);
      delete Value;
    }

    Value = nullptr;
  }

  virtual ASTValue<>* GetValue() {
    return Value;
  }

  virtual const ASTValue<>* GetValue() const {
    return Value;
  }

  virtual ASTType GetValueType() const {
    return ValueType;
  }

  virtual void SetValueType(ASTType Ty) {
    ValueType = Ty;
  }

  virtual const ASTDeclarationContext* GetContext() const {
    return CTX;
  }

  virtual bool HasOverloadedEntries() const {
    return false;
  }

  virtual bool ValueIsExpression() const {
    return Value && (Value->Value.second == ASTTypeExpression);
  }

  virtual void SetLocalScope() {
    CTX = ASTDeclarationContextTracker::Instance().GetCurrentContext();
    Scope = ASTSymbolScope::Local;
    const_cast<ASTIdentifierNode*>(Ident)->SetLocalScope();
  }

  virtual void SetLocalScope(const ASTDeclarationContext* DC) {
    assert(DC && "Invalid ASTDeclarationContext argument!");
    CTX = DC;
    Scope = ASTSymbolScope::Local;
    const_cast<ASTIdentifierNode*>(Ident)->SetLocalScope(DC);
  }

  virtual void SetGlobalScope() {
    CTX = ASTDeclarationContextTracker::Instance().GetGlobalContext();
    Scope = ASTSymbolScope::Global;
    const_cast<ASTIdentifierNode*>(Ident)->SetGlobalScope();
  }

  virtual void SetScope(ASTSymbolScope S) {
    Scope = S;
    if (S == ASTSymbolScope::Global)
      CTX = ASTDeclarationContextTracker::Instance().GetGlobalContext();
    else
      CTX = ASTDeclarationContextTracker::Instance().GetCurrentContext();
  }

  virtual void SetContext(const ASTDeclarationContext* CX) {
    CTX = CX;
    if (ASTDeclarationContextTracker::Instance().IsGlobalContext(CX))
      Scope = ASTSymbolScope::Global;
    else
      Scope = ASTSymbolScope::Local;
  }

  virtual ASTSymbolScope GetScope() const {
    return Scope;
  }

  virtual bool IsGlobalScope() const {
    return Scope == ASTSymbolScope::Global;
  }

  virtual bool IsLocalScope() const {
    return Scope == ASTSymbolScope::Local;
  }

  virtual void SetDoNotDelete() {
    DoNotDelete = true;
  }

  virtual void SetDelete() {
    DoNotDelete = false;
  }

  virtual bool IsDoNotDelete() const {
    return DoNotDelete;
  }

  virtual const ASTIdentifierNode* GetIdentifier() const {
    return ITy == ASTTypeIdentifier ? Ident : IdentRef;
  }

  virtual ASTIdentifierNode* GetIdentifier() {
    return ITy == ASTTypeIdentifier ?
                  const_cast<ASTIdentifierNode*>(Ident) :
                  const_cast<ASTIdentifierRefNode*>(IdentRef);
  }

  virtual const ASTIdentifierRefNode* GetIdentifierRef() const {
    return ITy == ASTTypeIdentifierRef ? IdentRef : nullptr;
  }

  virtual ASTIdentifierRefNode* GetIdentifierRef() {
    return ITy == ASTTypeIdentifierRef ?
                  const_cast<ASTIdentifierRefNode*>(IdentRef) : nullptr;
  }
};

class ASTMapSymbolTableEntry : public ASTSymbolTableEntry {
private:
  ASTMapSymbolTableEntry(const ASTSymbolTableEntry&) = delete;
  ASTMapSymbolTableEntry& operator=(const ASTSymbolTableEntry&) = delete;

protected:
  // Used only by functions, defcals, gates and kernels,
  // all of which can be overloaded.
  std::map<uint64_t, ASTSymbolTableEntry*> STEM;

public:
  using map_type = std::map<uint64_t, ASTSymbolTableEntry*>;
  using map_iterator = typename map_type::iterator;
  using map_const_iterator = typename map_type::const_iterator;

public:
  ASTMapSymbolTableEntry()
  : ASTSymbolTableEntry(), STEM() { }

  ASTMapSymbolTableEntry(const ASTMapSymbolTableEntry& RHS)
  : ASTSymbolTableEntry(RHS), STEM(RHS.STEM) { }

  ASTMapSymbolTableEntry(const ASTIdentifierNode* Id, ASTType Ty)
  : ASTSymbolTableEntry(Id, Ty), STEM() { }

  ASTMapSymbolTableEntry& operator=(const ASTMapSymbolTableEntry& RHS) {
    if (this != &RHS) {
      ASTSymbolTableEntry::operator=(RHS);
      STEM = RHS.STEM;
    }

    return *this;
  }

  virtual ~ASTMapSymbolTableEntry() = default;

  virtual bool HasMap() const override {
    return true;
  }

  virtual bool HasOverloadedEntries() const override {
    return !STEM.empty();
  }

  virtual std::map<uint64_t, ASTSymbolTableEntry*>& GetMap() {
    return STEM;
  }

  virtual const std::map<uint64_t, ASTSymbolTableEntry*>& GetMap() const {
    return STEM;
  }

  virtual ASTSymbolTableEntry* Find(uint64_t H) {
    map_iterator I = STEM.find(H);
    return I == STEM.end() ? nullptr : (*I).second;
  }

  virtual const ASTSymbolTableEntry* Find(uint64_t H) const {
    map_const_iterator I = STEM.find(H);
    return I == STEM.end() ? nullptr : (*I).second;
  }

  virtual ASTSymbolTableEntry* Find(const std::string& M) {
    assert(!M.empty() && "Empty search key string!");

    uint64_t H = std::hash<std::string>{}(M);
    map_iterator I = STEM.find(H);
    return I == STEM.end() ? nullptr : (*I).second;
  }

  virtual const ASTSymbolTableEntry* Find(const std::string& M) const {
    assert(!M.empty() && "Empty search key string!");

    uint64_t H = std::hash<std::string>{}(M);
    map_const_iterator I = STEM.find(H);
    return I == STEM.end() ? nullptr : (*I).second;
  }

  virtual bool ValueIsExpression() const override {
    return false;
  }

  map_iterator begin() {
    return STEM.begin();
  }

  map_const_iterator begin() const {
    return STEM.begin();
  }

  map_iterator end() {
    return STEM.end();
  }

  map_const_iterator end() const {
    return STEM.end();
  }
};

class ASTSymbolTable {
  friend class ASTGateNode;
  friend class ASTDefcalNode;
  friend class ASTFunctionDefinitionNode;
  friend class ASTKernelNode;

private:
  enum STMapIndex : unsigned {
    XNONE = 0,
    XSTM,
    XASTM,
    XQSTM,
    XGSTM,
    XSGSTM,
    XHGSTM,
    XGPSTM,
    XDSTM,
    XSDSTM,
    XHDSTM,
    XFSTM,
    XSFSTM,
    XHFSTM,
    XCSTM,
    XGLSTM,
    XLSTM,
    XUSTM,
  };

private:
  // Generic:
  static std::multimap<std::string, ASTSymbolTableEntry*> STM;

  // Angles:
  static std::map<std::string, ASTSymbolTableEntry*> ASTM;

  // Qubits:
  static std::map<std::string, ASTSymbolTableEntry*> QSTM;

  // Gates:
  static std::map<std::string, ASTSymbolTableEntry*> GSTM;    // Gates.
  static std::map<std::string, ASTSymbolTableEntry*> SGSTM;   // Staging.
  static std::map<uint64_t, ASTSymbolTableEntry*> HGSTM;      // Hashed.

  static std::map<std::string, ASTSymbolTableEntry*> GPSTM;   // Gate Params.

  // Defcals:
  static std::map<std::string, ASTSymbolTableEntry*> DSTM;    // Defcals.
  static std::map<std::string, ASTSymbolTableEntry*> SDSTM;   // Staging.
  static std::map<uint64_t, ASTSymbolTableEntry*> HDSTM;      // Hashed.

  // Functions:
  static std::map<std::string, ASTSymbolTableEntry*> FSTM;    // Functions.
  static std::map<std::string, ASTSymbolTableEntry*> SFSTM;   // Staging.
  static std::map<uint64_t, ASTSymbolTableEntry*> HFSTM;      // Hashed.

  // Calibration block symbols.
  static std::map<std::string, ASTSymbolTableEntry*> CSTM;    // Cal block symbols.

  // Globals:
  static std::map<std::string, ASTSymbolTableEntry*> GLSTM;

  // Locals:
  static std::map<std::string, ASTSymbolTableEntry*> LSTM;

  // Undefined Types under construction:
  static std::map<std::string, ASTSymbolTableEntry*> USTM;

  static ASTSymbolTable ST;

public:
  using map_type = std::map<std::string, ASTSymbolTableEntry*>;
  using map_iterator = typename map_type::iterator;
  using map_const_iterator = typename map_type::const_iterator;

  using hash_map_type = std::map<uint64_t, ASTSymbolTableEntry*>;
  using hash_map_iterator = typename hash_map_type::iterator;
  using hash_map_const_iterator = typename hash_map_type::const_iterator;

  using multimap_type = std::multimap<std::string, ASTSymbolTableEntry*>;
  using multimap_iterator = typename multimap_type::iterator;
  using multimap_const_iterator = typename multimap_type::const_iterator;

private:
  bool IsComplexPart(const std::string& S) {
    return !(S.find(".creal") == std::string::npos &&
             S.find(".cimag") == std::string::npos);
  }

  bool IsFramePart(const std::string& S) {
    return !(S.find(".phase") == std::string::npos &&
             S.find(".freq") == std::string::npos &&
             S.find(".time") == std::string::npos);
  }

  void EraseFromMap(map_iterator I, STMapIndex XI) {
    switch (XI) {
    case XSTM:
      STM.erase(I);
      break;
    case XASTM:
      ASTM.erase(I);
      break;
    case XQSTM:
      QSTM.erase(I);
      break;
    case XGSTM:
      GSTM.erase(I);
      break;
    case XSGSTM:
      SGSTM.erase(I);
      break;
    case XGPSTM:
      GPSTM.erase(I);
      break;
    case XDSTM:
      DSTM.erase(I);
      break;
    case XSDSTM:
      SDSTM.erase(I);
      break;
    case XFSTM:
      FSTM.erase(I);
      break;
    case XSFSTM:
      SFSTM.erase(I);
      break;
    case XCSTM:
      CSTM.erase(I);
      break;
    case XGLSTM:
      GLSTM.erase(I);
      break;
    case XLSTM:
      LSTM.erase(I);
      break;
    case XUSTM:
      USTM.erase(I);
      break;
    default:
      break;
    }
  }

  void EraseFromMap(hash_map_iterator I, STMapIndex XI) {
    switch (XI) {
    case XHGSTM:
      HGSTM.erase(I);
      break;
    case XHDSTM:
      HDSTM.erase(I);
      break;
    case XHFSTM:
      HFSTM.erase(I);
      break;
    default:
      break;
    }
  }

protected:
  ASTSymbolTable() = default;

  std::map<std::string, ASTSymbolTableEntry*>& GetLSTM() {
    return LSTM;
  }

  const std::map<std::string, ASTSymbolTableEntry*>& GetLSTM() const {
    return LSTM;
  }

  std::map<std::string, ASTSymbolTableEntry*>& GetUSTM() {
    return USTM;
  }

  const std::map<std::string, ASTSymbolTableEntry*>& GetUSTM() const {
    return USTM;
  }

public:
  using DiagLevel = QasmDiagnosticEmitter::DiagLevel;

public:
  ~ASTSymbolTable() = default;

  static ASTSymbolTable& Instance() {
    return ST;
  }

  ASTSymbolTableEntry* CreateSymbolTableEntry(const ASTIdentifierNode* Id,
                                              ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode!");

    return new ASTSymbolTableEntry(Id, Ty);
  }

  bool Insert(const std::string& S, ASTIdentifierNode* Id) {
    if (ASTAngleContextControl::Instance().InOpenContext() &&
        ASTTypeSystemBuilder::Instance().IsImplicitAngle(S)) {
      map_iterator AI = ASTM.find(S);
      if (AI != ASTM.end() && (*AI).second->HasValue() &&
          (*AI).second->GetValueType() == ASTTypeAngle) {
        if ((*AI).second->IsGlobalScope()) {
          std::stringstream M;
          M << "Symbol " << S << " already exists in the SymbolTable "
            << "at Global Scope.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                        DiagLevel::Warning);
          Id->SetSymbolTableEntry((*AI).second);
          return true;
        } else if ((*AI).second->IsLocalScope()) {
          std::stringstream M;
          M << "Symbol " << S << " already exists in the SymbolTable "
            << "at Local Scope.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                        DiagLevel::Warning);
          Id->SetSymbolTableEntry((*AI).second);
          return true;
        }
      } else if (AI != ASTM.end() && (*AI).second->HasValue() &&
                 (*AI).second->GetValueType() != ASTTypeAngle &&
                 (*AI).second->GetValueType() != ASTTypeUndefined) {
        std::stringstream M;
        M << "Symbol " << S << " already exists in the SymbolTable "
          << "with a different Type.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                      DiagLevel::ICE);
        return false;
      } else if (AI != ASTM.end() && !(*AI).second->HasValue() &&
                 (*AI).second->GetValueType() != ASTTypeUndefined &&
                 (*AI).second->GetValueType() != ASTTypeAngle) {
        std::stringstream M;
        M << "Symbol " << S << " already exists in the SymbolTable "
          << "but has no Value!.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                      DiagLevel::ICE);
        return false;
      }

      Id->SetBits(ASTAngleNode::AngleBits);
      ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(Id, ASTTypeAngle);
      assert(STE && "Could not create a valid ASTSymbolTableEntry!");

      STE->SetDoNotDelete();
      if (!ASTM.insert(std::make_pair(S, STE)).second) {
        std::stringstream M;
        M << "Failure inserting Symbol " << Id->GetName()
          << " in the SymbolTable!";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      Id->SetSymbolTableEntry(STE);
      return true;
    } else {
      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);

      if (R.first != STM.end()) {
        std::stringstream M;
        for (multimap_iterator I = R.first; I != R.second; ++I) {
          if ((*I).second->GetScope() == ASTSymbolScope::Global &&
              (*I).second->GetScope() == Id->GetSymbolScope() &&
              (*I).second->GetIdentifier()->GetBits() == Id->GetBits()) {
            M << "Symbol " << S << " already exists in the SymbolTable "
              << "at Global Scope.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                           DiagLevel::Warning);
            break;
          }
        }
      }

      ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(Id);
      assert(STE && "Could not create a valid ASTSymbolTableEntry!");

      STM.insert(std::make_pair(S, STE));
      Id->SetSymbolTableEntry(STE);
    }

    return true;
  }

  bool Insert(const std::string& S, ASTIdentifierNode* Id, ASTType Ty) {
    if (ASTAngleContextControl::Instance().InOpenContext() &&
        ASTTypeSystemBuilder::Instance().IsImplicitAngle(S) &&
        Ty == ASTTypeAngle) {
      return Insert(S, Id);
    } else if (Ty == ASTTypeGate) {
      map_iterator GI = GSTM.find(Id->GetName());
      if (GI != GSTM.end()) {
        std::stringstream M;
        M << "A Gate with name " << Id->GetName() << " already exists.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
        return false;
      }

      ASTSymbolTableEntry* GSTE = new ASTSymbolTableEntry(Id, Ty);
      assert(GSTE && "Could not create a valid ASTSymbolTableEntry!");

      Id->SetGlobalScope();
      GSTE->SetGlobalScope();
      GSTE->SetDoNotDelete();

      if (!GSTM.insert(std::make_pair(Id->GetName(), GSTE)).second) {
        std::stringstream M;
        M << "Gate " << Id->GetName() << " Symbol Table insertion failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
            DiagLevel::ICE);
        return false;
      }

      Id->SetSymbolTableEntry(GSTE);
      return true;
    } else if (Ty == ASTTypeFunction || Ty == ASTTypeKernel) {
      map_iterator FI = FSTM.find(Id->GetName());
      if (FI != FSTM.end()) {
        std::stringstream M;
        M << "A Function with name " << Id->GetName() << " already exists.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
        return false;
      }

      ASTSymbolTableEntry* FSTE = new ASTSymbolTableEntry(Id, Ty);
      assert(FSTE && "Could not create a valid ASTSymbolTable Entry!");

      Id->SetGlobalScope();
      FSTE->SetDoNotDelete();
      FSTE->SetGlobalScope();

      if (!FSTM.insert(std::make_pair(Id->GetName(), FSTE)).second) {
        std::stringstream M;
        M << "Function " << Id->GetName() << " Symbol Table insertion failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
        return false;
      }

      Id->SetSymbolTableEntry(FSTE);
      return true;
    } else if (Ty == ASTTypeDefcal) {
      map_iterator DI = DSTM.find(Id->GetName());
      if (DI != DSTM.end()) {
        std::stringstream M;
        M << "A Defcal with name " << Id->GetName() << " already exists.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
        return false;
      }

      ASTSymbolTableEntry* DSTE = new ASTSymbolTableEntry(Id, Ty);
      assert(DSTE && "Could not create a valid ASTSymbolTable Entry!");

      Id->SetGlobalScope();
      DSTE->SetDoNotDelete();
      DSTE->SetGlobalScope();

      if (!DSTM.insert(std::make_pair(Id->GetName(), DSTE)).second) {
        std::stringstream M;
        M << "Defcal " << Id->GetName() << " Symbol Table insertion failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
        return false;
      }

      Id->SetSymbolTableEntry(DSTE);
      return true;
    } else if (Ty == ASTTypeQubit ||
               Ty == ASTTypeQubitContainer ||
               Ty == ASTTypeQubitContainerAlias) {
      map_iterator QI = QSTM.find(Id->GetName());
      if (QI != QSTM.end()) {
        if ((*QI).second && (*QI).second->GetValueType() == Ty &&
            (*QI).second->GetIdentifier()->GetBits() == Id->GetBits()) {
          return true;
        }

        std::stringstream M;
        const char* QT;
        if ((*QI).second) {
          switch ((*QI).second->GetValueType()) {
          case ASTTypeQubit:
            QT = "Qubit";
            break;
          case ASTTypeQubitContainer:
            QT = "QubitContainer";
            break;
          default:
            QT = "QubitContainerAlias";
            break;
          }
        } else {
          switch (Ty) {
          case ASTTypeQubit:
            QT = "Qubit";
            break;
          case ASTTypeQubitContainer:
            QT = "QubitContainer";
            break;
          default:
            QT = "QubitContainerAlias";
            break;
          }
        }

        M << "A " << QT << " with name " << Id->GetName() << " already exists.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
        return false;
      }

      ASTSymbolTableEntry* QSTE = new ASTSymbolTableEntry(Id, Ty);
      assert(QSTE && "Could not create a valid ASTSymbolTable Entry!");

      QSTE->SetDoNotDelete();

      if (!QSTM.insert(std::make_pair(Id->GetName(), QSTE)).second) {
        std::stringstream M;
        M << "Qubit " << Id->GetName() << " Symbol Table insertion failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
        return false;
      }

      Id->SetSymbolTableEntry(QSTE);
      return true;
    } else {
      if (ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal()) {
        map_iterator GLI = GLSTM.find(Id->GetName());
        if (GLI != GLSTM.end()) {
          if ((*GLI).second && (*GLI).second->GetValueType() == Ty &&
              (*GLI).second->GetIdentifier()->GetBits() == Id->GetBits()) {
            return true;
          }
        }

        ASTSymbolTableEntry* GLSTE = new ASTSymbolTableEntry(Id, Ty);
        assert(GLSTE && "Could not create a valid ASTSymbolTable Entry!");

        if (!GLSTM.insert(std::make_pair(Id->GetName(), GLSTE)).second) {
          std::stringstream M;
          M << "Global Symbol " << Id->GetName() << " Symbol Table insertion "
            << "failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        Id->SetSymbolTableEntry(GLSTE);
        return true;
      } else {
        map_iterator LI = LSTM.find(Id->GetName());
        if (LI != LSTM.end()) {
          if ((*LI).second && (*LI).second->GetValueType() == Ty &&
              (*LI).second->GetIdentifier()->GetBits() == Id->GetBits()) {
            return true;
          }
        }

        ASTSymbolTableEntry* LSTE = new ASTSymbolTableEntry(Id, Ty);
        assert(LSTE && "Could not create a valid ASTSymbolTable Entry!");

        if (!LSTM.insert(std::make_pair(Id->GetName(), LSTE)).second) {
          std::stringstream M;
          M << "Symbol " << Id->GetName() << " Symbol Table insertion failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        Id->SetSymbolTableEntry(LSTE);
        return true;
      }
    }

    std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);

    if (R.first != STM.end()) {
      std::stringstream M;

      for (multimap_iterator I = R.first; I != R.second; ++I) {
        if ((*I).second->GetScope() == ASTSymbolScope::Global &&
            (*I).second->GetValueType() == Ty) {
          M << "Symbol " << S << " already exists in the SymbolTable "
            << "at Global Scope.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Warning);
          break;
        }
      }
    }

    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(S))
      Id->SetBits(ASTAngleNode::AngleBits);

    ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(Id, Ty);
    assert(STE && "Could not create a valid ASTSymbolTableEntry!");

    multimap_const_iterator H = STM.lower_bound(S);
    STM.insert(H, std::make_pair(S, STE));
    Id->SetSymbolTableEntry(STE);
    return true;
  }

  bool Insert(ASTIdentifierNode* Id, unsigned Bits, ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    if (ASTAngleContextControl::Instance().InOpenContext() &&
        ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id->GetName()) &&
        Ty == ASTTypeAngle) {
      map_iterator AI = ASTM.find(Id->GetName());
      if (AI == ASTM.end()) {
        ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(Id, Ty);
        assert(STE && "Could not create a valid ASTSymbolTableEntry!");

        STE->SetDoNotDelete();
        if (!ASTM.insert(std::make_pair(Id->GetName(),
                                                       STE)).second) {
          std::stringstream M;
          M << "Failure inserting Symbol " << Id->GetName()
            << " in the SymbolTable!";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        Id->SetSymbolTableEntry(STE);
        return true;
      }

      if (!(*AI).second) {
        (*AI).second = new ASTSymbolTableEntry(Id, Ty);
        assert((*AI).second && "Could not create a valid ASTSymbolTableEntry!");

        (*AI).second->SetDoNotDelete();
        Id->SetSymbolTableEntry((*AI).second);
        return true;
      } else if ((*AI).second->HasValue() && (*AI).second->GetValueType() == Ty) {
        std::stringstream M;
        M << "A Symbol " << Id->GetName() << " of the same Type "
          << "already exists in the symbolTable ";
        if ((*AI).second->IsGlobalScope())
          M << "at Global Scope.";
        else
          M << "at Local Scope.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                      DiagLevel::Warning);
        Id->SetSymbolTableEntry((*AI).second);
        return true;
      } else if (!(*AI).second->HasValue()) {
        std::stringstream M;
        M << "A Symbol " << Id->GetName() << " of the same Type "
          << "already exists in the symbolTable but has no Value!";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }
    } else if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
      map_iterator AI = ASTM.find(Id->GetName());
      if (AI == ASTM.end()) {
        ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(Id, Ty);
        assert(STE && "Could not create a valid ASTSymbolTableEntry!");

        STE->SetGlobalScope();
        STE->SetDoNotDelete();
        if (!ASTM.insert(std::make_pair(Id->GetName(), STE)).second) {
          std::stringstream M;
          M << "Failure inserting Symbol " << Id->GetName()
            << " in the SymbolTable!";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        Id->SetGlobalScope();
        Id->SetSymbolTableEntry(STE);
        return true;
      }

      if (!(*AI).second) {
        (*AI).second = new ASTSymbolTableEntry(Id, Ty);
        assert((*AI).second && "Could not create a valid ASTSymbolTableEntry!");

        (*AI).second->SetDoNotDelete();
        Id->SetSymbolTableEntry((*AI).second);
        return true;
      } else if ((*AI).second->HasValue() && (*AI).second->GetValueType() == Ty) {
        std::stringstream M;
        M << "A Symbol " << Id->GetName() << " of the same Type "
          << "already exists in the symbolTable ";
        if ((*AI).second->IsGlobalScope())
          M << "at Global Scope.";
        else
          M << "at Local Scope.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                      DiagLevel::Warning);
        Id->SetSymbolTableEntry((*AI).second);
        return true;
      } else if (!(*AI).second->HasValue()) {
        std::stringstream M;
        M << "A Symbol " << Id->GetName() << " of the same Type "
          << "already exists in the symbolTable but has no Value!";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }
    } else if (Ty == ASTTypeGate) {
      map_iterator GI = GSTM.find(Id->GetName());
      if (GI != GSTM.end()) {
        std::stringstream M;
        M << "A Gate with name " << Id->GetName() << " already exists.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
        return false;
      }

      ASTSymbolTableEntry* GSTE = new ASTSymbolTableEntry(Id, Ty);
      assert(GSTE && "Could not create a valid ASTSymbolTableEntry!");

      Id->SetGlobalScope();
      GSTE->SetGlobalScope();
      GSTE ->SetDoNotDelete();

      if (!GSTM.insert(std::make_pair(Id->GetName(), GSTE)).second) {
        std::stringstream M;
        M << "Gate " << Id->GetName() << " Symbol Table insertion failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      Id->SetSymbolTableEntry(GSTE);
      return true;
    } else if (Ty == ASTTypeFunction || Ty == ASTTypeKernel) {
      map_iterator FI = FSTM.find(Id->GetName());
      if (FI == FSTM.end()) {
        ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(Id, Ty);
        assert(STE && "Could not create a valid ASTSymbolTableEntry!");

        if (!FSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
          std::stringstream M;
          M << "Failure inserting Symbol " << Id->GetName()
            << " in the SymbolTable!";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        STE->SetDoNotDelete();
        STE->SetGlobalScope();
        Id->SetGlobalScope();
        Id->SetSymbolTableEntry(STE);
        return true;
      }

      if (!(*FI).second) {
        (*FI).second = new ASTSymbolTableEntry(Id, Ty);
        assert((*FI).second && "Could not create a valid ASTSymbolTable Entry!");

        (*FI).second->SetDoNotDelete();
        (*FI).second->SetGlobalScope();
        Id->SetGlobalScope();
        Id->SetSymbolTableEntry((*FI).second);
        return true;
      } else if ((*FI).second->HasValue() && (*FI).second->GetValueType() == Ty) {
        std::stringstream M;
        M << "A Symbol " << Id->GetName() << " of the same Type "
          << "already exists in the SymbolTable ";
        if ((*FI).second->IsGlobalScope())
          M << "at Global Scope.";
        else
          M << "at Local Scope.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Warning);
        if ((*FI).second->IsLocalScope()) {
          M.clear();
          M.str("");
          M << "Functions cannot have Local Scope.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        Id->SetSymbolTableEntry((*FI).second);
        return true;
      } else if (!(*FI).second->HasValue()) {
        std::stringstream M;
        M << "A Symbol " << Id->GetName() << " of the same Type "
          << "already exists in the SymbolTable but has no Value!";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
      }
    } else if (Ty == ASTTypeDefcal) {
      map_iterator DI = DSTM.find(Id->GetName());
      if (DI == DSTM.end()) {
        ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(Id, Ty);
        assert(STE && "Could not create a valid ASTSymbolTableEntry!");

        if (!DSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
          std::stringstream M;
          M << "Failure inserting Symbol " << Id->GetName()
            << " in the SymbolTable!";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        STE->SetDoNotDelete();
        STE->SetGlobalScope();
        Id->SetGlobalScope();
        Id->SetSymbolTableEntry(STE);
        return true;
      }

      if (!(*DI).second) {
        (*DI).second = new ASTSymbolTableEntry(Id, Ty);
        assert((*DI).second && "Could not create a valid ASTSymbolTable Entry!");

        (*DI).second->SetDoNotDelete();
        (*DI).second->SetGlobalScope();
        Id->SetGlobalScope();
        Id->SetSymbolTableEntry((*DI).second);
        return true;
      } else if ((*DI).second->HasValue() && (*DI).second->GetValueType() == Ty) {
        std::stringstream M;
        M << "A Symbol " << Id->GetName() << " of the same Type "
          << "already exists in the SymbolTable ";
        if ((*DI).second->IsGlobalScope())
          M << "at Global Scope.";
        else
          M << "at Local Scope.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Warning);
        if ((*DI).second->IsLocalScope()) {
          M.clear();
          M.str("");
          M << "Functions cannot have Local Scope.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        Id->SetSymbolTableEntry((*DI).second);
        return true;
      } else if (!(*DI).second->HasValue()) {
        std::stringstream M;
        M << "A Symbol " << Id->GetName() << " of the same Type "
          << "already exists in the SymbolTable but has no Value!";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
      }
    } else if (Ty == ASTTypeQubit || Ty == ASTTypeQubitContainer ||
               Ty == ASTTypeQubitContainerAlias) {
      map_iterator QI = QSTM.find(Id->GetName());
      if (QI != QSTM.end()) {
        if ((*QI).second && (*QI).second->GetValueType() == Ty &&
            (*QI).second->GetIdentifier()->GetBits() == Bits) {
          return true;
        }

        std::stringstream M;
        const char* QT;
        if ((*QI).second) {
          switch ((*QI).second->GetValueType()) {
          case ASTTypeQubit:
            QT = "Qubit";
            break;
          case ASTTypeQubitContainer:
            QT = "QubitContainer";
            break;
          default:
            QT = "QubitContainerAlias";
            break;
          }
        } else {
          switch (Ty) {
          case ASTTypeQubit:
            QT = "Qubit";
            break;
          case ASTTypeQubitContainer:
            QT = "QubitContainer";
            break;
          default:
            QT = "QubitContainerAlias";
            break;
          }
        }

        M << "A " << QT << " with name " << Id->GetName() << " already exists.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
        return false;
      }

      ASTSymbolTableEntry* QSTE = new ASTSymbolTableEntry(Id, Ty);
      assert(QSTE && "Could not create a valid ASTSymbolTable Entry!");

      QSTE->SetDoNotDelete();

      if (!QSTM.insert(std::make_pair(Id->GetName(), QSTE)).second) {
        std::stringstream M;
        M << "Qubit " << Id->GetName() << " Symbol Table insertion failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      Id->SetSymbolTableEntry(QSTE);
      return true;
    }

    const std::string& S = Id->GetName();
    std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);

    if (R.first != STM.end()) {
      std::stringstream M;

      for (multimap_iterator I = R.first; I != R.second; ++I) {
        if ((*I).second->GetScope() == ASTSymbolScope::Global &&
            (*I).second->GetValueType() == Ty &&
            (*I).second->GetIdentifier()->GetBits() == Bits) {
          M << "Symbol " << S << " already exists in the SymbolTable "
            << "at Global Scope.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::Warning);
          return true;
        }
      }
    }

    ASTSymbolTableEntry* STE = new ASTSymbolTableEntry(Id, Ty);
    assert(STE && "Could not create a valid ASTSymbolTableEntry!");

    multimap_const_iterator H = STM.lower_bound(S);

    if (STM.insert(H, std::make_pair(S, STE)) == STM.end()) {
      std::stringstream M;
      M << "Failure inserting Symbol " << S << " in the SymbolTable!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    Id->SetSymbolTableEntry(STE);
    return true;
  }

  bool Insert(ASTIdentifierNode* Id, ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    const std::string& S = Id->GetName();
    ASTType Ty = STE->GetValueType();

    if ((ASTAngleContextControl::Instance().InOpenContext() &&
         ASTTypeSystemBuilder::Instance().IsImplicitAngle(S) &&
         Ty == ASTTypeAngle) ||
        ASTTypeSystemBuilder::Instance().IsReservedAngle(S)) {
      map_iterator AI = ASTM.find(Id->GetName());
      if (AI == ASTM.end()) {
        STE->SetDoNotDelete();
        if (!ASTM.insert(std::make_pair(S, STE)).second) {
          std::stringstream M;
          M << "Failure inserting Symbol " << S << " in the SymbolTable!";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        Id->SetSymbolTableEntry(STE);
        return true;
      }

      if (!(*AI).second) {
        STE->SetDoNotDelete();
        (*AI).second = STE;
        Id->SetSymbolTableEntry((*AI).second);
        return true;
      }

      if ((*AI).second->GetValueType() == ASTTypeUndefined) {
        if (!(*AI).second->IsDoNotDelete())
          delete (*AI).second;

        STE->SetDoNotDelete();
        (*AI).second = STE;
        Id->SetSymbolTableEntry((*AI).second);
        return true;
      } else if ((*AI).second->GetValueType() == ASTTypeAngle) {
        (*AI).second = STE;
        Id->SetSymbolTableEntry((*AI).second);
        (*AI).second->SetDoNotDelete();
        return true;
      } else {
        std::stringstream M;
        M << "Cannot replace the existing SymbolTable Entry of Type "
          << PrintTypeEnum((*AI).second->GetValueType()) << " with "
          << "an Entry of Type " << PrintTypeEnum(STE->GetValueType())
          << "!";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }
    } else if (Ty == ASTTypeFunction || Ty == ASTTypeKernel) {
      map_iterator FI = FSTM.find(Id->GetName());
      if (FI != FSTM.end()) {
        (*FI).second = STE;
        (*FI).second->SetDoNotDelete();
        (*FI).second->SetGlobalScope();
        Id->SetGlobalScope();
        Id->SetSymbolTableEntry((*FI).second);
        return true;
      }

      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
      std::vector<multimap_iterator> IV;

      if (R.first != STM.end()) {
        for (multimap_iterator LI = R.first; LI != R.second; ++LI) {
          if ((*LI).first == Id->GetName() &&
              (*LI).second->GetIdentifier()->GetBits() == Id->GetBits() &&
              ((*LI).second->GetValueType() == Ty ||
               (*LI).second->GetValueType() == ASTTypeUndefined)) {
            IV.push_back(LI);
            break;
          }
        }
      }

      if (!IV.empty()) {
        for (std::vector<multimap_iterator>::iterator VI = IV.begin();
             VI != IV.end(); ++VI)
          STM.erase(*VI);
      }

      if (!FSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
        std::stringstream M;
        M << "Insertion into the Function SymbolTable failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      FI = FSTM.find(Id->GetName());
      assert((*FI).second && "Could not create a valid ASTSymbolTableEntry!");

      (*FI).second->SetDoNotDelete();
      (*FI).second->SetGlobalScope();
      Id->SetGlobalScope();
      Id->SetSymbolTableEntry((*FI).second);
      return true;
    } else if (Ty == ASTTypeDefcal) {
      map_iterator DI = DSTM.find(Id->GetName());
      if (DI != DSTM.end()) {
        (*DI).second = STE;
        (*DI).second->SetDoNotDelete();
        (*DI).second->SetGlobalScope();
        Id->SetGlobalScope();
        Id->SetSymbolTableEntry((*DI).second);
        return true;
      }

      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
      std::vector<multimap_iterator> IV;

      if (R.first != STM.end()) {
        for (multimap_iterator LI = R.first; LI != R.second; ++LI) {
          if ((*LI).first == Id->GetName() &&
              (*LI).second->GetIdentifier()->GetBits() == Id->GetBits() &&
              ((*LI).second->GetValueType() == Ty ||
               (*LI).second->GetValueType() == ASTTypeUndefined)) {
            IV.push_back(LI);
            break;
          }
        }
      }

      if (!IV.empty()) {
        for (std::vector<multimap_iterator>::iterator VI = IV.begin();
             VI != IV.end(); ++VI)
          STM.erase(*VI);
      }

      if (!DSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
        std::stringstream M;
        M << "Insertion into the Defcal SymbolTable failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      DI = DSTM.find(Id->GetName());
      assert((*DI).second && "Could not create a valid ASTSymbolTableEntry!");

      (*DI).second->SetDoNotDelete();
      (*DI).second->SetGlobalScope();
      Id->SetGlobalScope();
      Id->SetSymbolTableEntry((*DI).second);
      return true;
    } else if (Ty == ASTTypeGate || Ty == ASTTypeCXGate ||
               Ty == ASTTypeCCXGate || Ty == ASTTypeCNotGate ||
               Ty == ASTTypeHadamardGate || Ty == ASTTypeUGate) {
      map_iterator GI = GSTM.find(Id->GetName());
      if (GI != GSTM.end()) {
        if ((*GI).second != STE) {
          (*GI).second = STE;
          (*GI).second->SetDoNotDelete();
          (*GI).second->SetGlobalScope();
          Id->SetGlobalScope();
          Id->SetSymbolTableEntry((*GI).second);
        }

        return true;
      }

      if (!GSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
        std::stringstream M;
        M << "Insertion into the Gate SymbolTable failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      GI = GSTM.find(Id->GetName());
      assert((*GI).second && "Could not create a valid ASTSymbolTableEntry!");

      (*GI).second->SetDoNotDelete();
      Id->SetSymbolTableEntry((*GI).second);
      return true;
    } else if (Ty == ASTTypeQubit || Ty == ASTTypeQubitContainer ||
               Ty == ASTTypeQubitContainerAlias) {
      map_iterator QI = QSTM.find(Id->GetName());
      if (QI != QSTM.end()) {
        (*QI).second = STE;
        (*QI).second->SetDoNotDelete();
        Id->SetSymbolTableEntry((*QI).second);
        return true;
      }

      if (!QSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
        std::stringstream M;
        M << "Insertion into the Qubit SymbolTable failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      QI = QSTM.find(Id->GetName());
      assert((*QI).second && "Could not create a valid ASTSymbolTableEntry!");

      (*QI).second->SetDoNotDelete();
      Id->SetSymbolTableEntry((*QI).second);
      return true;
    } else if (Ty != ASTTypeUndefined) {
      if (ASTDeclarationContextTracker::Instance().IsGlobalContext(
                                                    Id->GetDeclarationContext())) {
        map_iterator GLI = GLSTM.find(Id->GetName());
        if (GLI != GLSTM.end()) {
          STE->SetGlobalScope();
          (*GLI).second = STE;
          Id->SetSymbolTableEntry((*GLI).second);
          return true;
        }

        if (!GLSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
          std::stringstream M;
          M << "Insertion into the Global SymbolTable failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        GLI = GLSTM.find(Id->GetName());
        assert((*GLI).second && "Could not create a valid ASTSymbolTableEntry!");

        STE->SetGlobalScope();
        Id->SetSymbolTableEntry((*GLI).second);
        return true;
      } else {
        map_iterator LI = LSTM.find(Id->GetName());
        if (LI != LSTM.end()) {
          STE->SetLocalScope(Id->GetDeclarationContext());
          (*LI).second = STE;
          Id->SetSymbolTableEntry((*LI).second);
          return true;
        }

        if (!LSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
          std::stringstream M;
          M << "Insertion into the Local SymbolTable failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        LI = LSTM.find(Id->GetName());
        assert((*LI).second && "Could not create a valid ASTSymbolTableEntry!");

        STE->SetLocalScope(Id->GetDeclarationContext());
        Id->SetSymbolTableEntry((*LI).second);
        return true;
      }
    }

    multimap_iterator R;
    multimap_iterator I = STM.find(Id->GetName());
    if (I == STM.end()) {
      R = STM.insert(std::make_pair(Id->GetName(), STE));
      if (R == STM.end())
        return false;

      Id->SetSymbolTableEntry((*R).second);
    } else {
      (*I).second = STE;
      Id->SetSymbolTableEntry((*I).second);
    }

    return true;
  }

  bool InsertGlobal(const ASTIdentifierNode* Id, ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    map_iterator GI = GLSTM.find(Id->GetName());
    if (GI != GLSTM.end()) {
      if ((*GI).second &&
          (*GI).second->GetIdentifier()->GetSymbolType() == Id->GetSymbolType() &&
          (*GI).second->GetIdentifier()->GetBits() == Id->GetBits()) {
        (*GI).second = STE;
        return true;
      } else if (!(*GI).second) {
        (*GI).second = STE;
        return true;
      } else {
        std::stringstream M;
        M << "A symbol with Identifier " << Id->GetName() << " already "
          << "exists in the Global Symbol Table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }
    }

    if (!GLSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
      std::stringstream M;
      M << "Failure inserting symbol " << Id->GetName()
        << " into the Global Symbol Table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    return true;
  }

  bool InsertLocal(const ASTIdentifierNode* Id, ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    map_iterator LI = LSTM.find(Id->GetName());
    if (LI != LSTM.end()) {
      if ((*LI).second &&
          (*LI).second->GetIdentifier()->GetSymbolType() == Id->GetSymbolType() &&
          (*LI).second->GetIdentifier()->GetBits() == Id->GetBits()) {
        (*LI).second = STE;
        return true;
      } else if (!(*LI).second) {
        (*LI).second = STE;
        return true;
      } else {
        std::stringstream M;
        M << "A symbol with Identifier " << Id->GetName() << " already "
          << "exists in the Local Symbol Table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }
    }

    if (!LSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
      std::stringstream M;
      M << "Failure inserting symbol " << Id->GetName()
        << " into the Local Symbol Table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    return true;
  }

  bool InsertDefcal(const ASTIdentifierNode* Id, ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    if (Id->GetSymbolType() != ASTTypeDefcal ||
        STE->GetValueType() != ASTTypeDefcal) {
      std::stringstream M;
      M << "Only Defcal or Identifiers can be inserted "
         << "in DSTM by this method.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (STE->GetIdentifier() != Id) {
      std::stringstream M;
      M << "Inconsistent Identifier and SymbolTable Entry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    map_iterator DI = DSTM.find(Id->GetDefcalGroupName());
    if (DI == DSTM.end()) {
      std::stringstream M;
      M << "Cannot create a defcal symbol without a defcal group.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    ASTMapSymbolTableEntry* MSTE =
      dynamic_cast<ASTMapSymbolTableEntry*>((*DI).second);
    assert(MSTE && "Could not dynamic_cast to an ASTMapSymbolTableEntry!");

    std::map<uint64_t, ASTSymbolTableEntry*>& MM = MSTE->GetMap();
    if (Id->GetMHash()) {
      if (!MM.insert(std::make_pair(Id->GetMHash(), STE)).second) {
        std::stringstream M;
        M << "Failure inserting defcal SymbolTable Entry into DSTM.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
        return false;
      }
    }

    return true;
  }

  bool InsertFunction(const ASTIdentifierNode* Id, ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    if ((Id->GetSymbolType() != ASTTypeFunction &&
         Id->GetSymbolType() != ASTTypeFunctionDeclaration) ||
        (STE->GetValueType() != ASTTypeFunction &&
         STE->GetValueType() != ASTTypeFunctionDeclaration)) {
      std::stringstream M;
      M << "Only Functions or Function Declarations can be inserted "
         << "in USTM by this method.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (STE->GetIdentifier() != Id) {
      std::stringstream M;
      M << "Inconsistent Identifier and SymbolTable Entry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    map_iterator FI = FSTM.find(Id->GetName());
    if (FI != FSTM.end()) {
      std::stringstream M;
      M << "An entry with Identifier " << Id->GetName() << " already "
        << "exists in the FSTM.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (!FSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
      std::stringstream M;
      M << "Failure inserting SymbolTable Entry into FSTM.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    return true;
  }

  bool InsertGate(const ASTIdentifierNode* Id, ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    if (!ASTIdentifierTypeController::Instance().IsGateType(Id->GetSymbolType())) {
      std::stringstream M;
      M << "Identifier does not represent a Gate Type.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }

    if (!ASTIdentifierTypeController::Instance().IsGateType(STE->GetValueType())) {
      std::stringstream M;
      M << "SymbolTable Entry does not represent a Gate Type.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }

    if (STE->GetIdentifier() != Id) {
      std::stringstream M;
      M << "Inconsistent Identifier <-> SymbolTable Entry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }

    map_iterator GI = GSTM.find(Id->GetName());
    if (GI != GSTM.end()) {
      if (ASTIdentifierTypeController::Instance().IsBuiltinUGate(Id)) {
        if (!STE->HasValue()) {
          std::stringstream M;
          M << "SymbolTable Entry for builtin Unitary Gate has no Value.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
          return false;
        }

        (*GI).second = STE;
        return true;
      }

      if (!ASTIdentifierTypeController::Instance().IsGateType(STE->GetValueType())) {
        std::stringstream M;
        M << "Updating SymbolTable Entry Value is not a Gate Type ("
          << PrintTypeEnum(STE->GetValueType()) << '.';
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
        return false;
      }

      if (!ASTIdentifierTypeController::Instance().CheckGateTypeMatch(
                                        (*GI).second->GetValueType(),
                                        STE->GetValueType())) {
        std::stringstream M;
        M << "ValueType mismatch between existing Gate SymbolTable "
          << "Entry and updating Gate SymbolTable Entry.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
        return false;
      }

      (*GI).second = STE;
      return true;
    }

    if (!GSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
      std::stringstream M;
      M << "Failure inserting SymbolTable Entry into GSTM.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }

    return true;
  }

  bool InsertUndefined(const ASTIdentifierNode* Id, ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    if (Id->GetSymbolType() != ASTTypeUndefined) {
      std::stringstream M;
      M << "Only ASTTypeUndefined Identifiers can be inserted in the "
         << "USTM SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (Id->GetBits() != 0) {
      std::stringstream M;
      M << "Invalid number of bits for ASTTypeUndefined.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (STE->GetValueType() != ASTTypeUndefined) {
      std::stringstream M;
      M << "Only ASTTypeUndefined SymbolTable Entries can be inserted in the "
         << "USTM SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (STE->GetIdentifier() != Id) {
      std::stringstream M;
      M << "Inconsistent relation between Identifier and SymbolTable Entry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    map_iterator UI = USTM.find(Id->GetName());
    if (UI != USTM.end()) {
      std::stringstream M;
      M << "An entry with Identifier " << Id->GetName() << " already "
        << "exists in the USTM.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (!USTM.insert(std::make_pair(Id->GetName(), STE)).second) {
      std::stringstream M;
      M << "Failure inserting SymbolTable Entry into USTM.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    return true;
  }

  bool Insert(ASTIdentifierRefNode* Id, ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    const std::string& S = Id->GetName();
    ASTType Ty = STE->GetValueType();

    if ((ASTAngleContextControl::Instance().InOpenContext() &&
        ASTTypeSystemBuilder::Instance().IsImplicitAngle(S) &&
        Ty == ASTTypeAngle) ||
        (ASTTypeSystemBuilder::Instance().IsReservedAngle(S))) {
      map_iterator AI = ASTM.find(Id->GetName());
      if (AI == ASTM.end()) {
        STE->SetDoNotDelete();
        if (!ASTM.insert(std::make_pair(S, STE)).second) {
          std::stringstream M;
          M << "Failure inserting Symbol " << S << " in the SymbolTable!";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        Id->SetSymbolTableEntry(STE);
        return true;
      }

      if (!(*AI).second) {
        STE->SetDoNotDelete();
        (*AI).second = STE;
        Id->SetSymbolTableEntry((*AI).second);
        return true;
      }

      if ((*AI).second->GetValueType() == ASTTypeUndefined) {
        if (!(*AI).second->IsDoNotDelete())
          delete (*AI).second;

        STE->SetDoNotDelete();
        (*AI).second = STE;
        Id->SetSymbolTableEntry((*AI).second);
        return true;
      } else if ((*AI).second->GetValueType() == ASTTypeAngle) {
        STE->SetDoNotDelete();
        (*AI).second = STE;
        Id->SetSymbolTableEntry((*AI).second);
        return true;
      } else {
        std::stringstream M;
        M << "Cannot replace the existing SymbolTable Entry of Type "
          << PrintTypeEnum((*AI).second->GetValueType()) << " with "
          << "an Entry of Type " << PrintTypeEnum(STE->GetValueType())
          << "!";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }
    } else if (Ty == ASTTypeAngle) {
      map_iterator AI = ASTM.find(Id->GetName());
      if (AI == ASTM.end()) {
        if (!ASTM.insert(std::make_pair(Id->GetName(), STE)).second) {
          std::stringstream M;
          M << "Failure inserting ASTAngleNode Reference into the "
            "angle Symbol Table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        return true;
      } else {
        (*AI).second = STE;
        Id->SetSymbolTableEntry((*AI).second);
        return true;
      }
    } else if (Ty == ASTTypeFunction || Ty == ASTTypeKernel) {
      map_iterator FI = FSTM.find(Id->GetName());
      if (FI != FSTM.end()) {
        (*FI).second = STE;
        (*FI).second->SetDoNotDelete();
        (*FI).second->SetGlobalScope();
        Id->SetGlobalScope();
        Id->SetSymbolTableEntry((*FI).second);
        return true;
      }

      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
      std::vector<multimap_iterator> IV;

      if (R.first != STM.end()) {
        for (multimap_iterator LI = R.first; LI != R.second; ++LI) {
          if ((*LI).first == Id->GetName() &&
              (*LI).second->GetIdentifier()->GetBits() == Id->GetBits() &&
              ((*LI).second->GetValueType() == Ty ||
               (*LI).second->GetValueType() == ASTTypeUndefined)) {
            IV.push_back(LI);
            break;
          }
        }
      }

      if (!IV.empty()) {
        for (std::vector<multimap_iterator>::iterator VI = IV.begin();
             VI != IV.end(); ++VI)
          STM.erase(*VI);
      }

      if (!FSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
        std::stringstream M;
        M << "Insertion into the Function SymbolTable failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      FI = FSTM.find(Id->GetName());
      assert((*FI).second && "Could not create a valid ASTSymbolTableEntry!");

      (*FI).second->SetDoNotDelete();
      (*FI).second->SetGlobalScope();
      Id->SetGlobalScope();
      Id->SetSymbolTableEntry((*FI).second);
      return true;
    } else if (Ty == ASTTypeDefcal) {
      map_iterator DI = DSTM.find(Id->GetName());
      if (DI != DSTM.end()) {
        (*DI).second = STE;
        (*DI).second->SetDoNotDelete();
        (*DI).second->SetGlobalScope();
        Id->SetGlobalScope();
        Id->SetSymbolTableEntry((*DI).second);
        return true;
      }

      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
      std::vector<multimap_iterator> IV;

      if (R.first != STM.end()) {
        for (multimap_iterator LI = R.first; LI != R.second; ++LI) {
          if ((*LI).first == Id->GetName() &&
              (*LI).second->GetIdentifier()->GetBits() == Id->GetBits() &&
              ((*LI).second->GetValueType() == Ty ||
               (*LI).second->GetValueType() == ASTTypeUndefined)) {
            IV.push_back(LI);
            break;
          }
        }
      }

      if (!IV.empty()) {
        for (std::vector<multimap_iterator>::iterator VI = IV.begin();
             VI != IV.end(); ++VI)
          STM.erase(*VI);
      }

      if (!DSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
        std::stringstream M;
        M << "Insertion into the Defcal SymbolTable failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      DI = DSTM.find(Id->GetName());
      assert((*DI).second && "Could not create a valid ASTSymbolTableEntry!");

      (*DI).second->SetDoNotDelete();
      (*DI).second->SetGlobalScope();
      Id->SetGlobalScope();
      Id->SetSymbolTableEntry((*DI).second);
      return true;
    } else if (Ty == ASTTypeGate) {
      map_iterator GI = GSTM.find(Id->GetName());
      if (GI != GSTM.end()) {
        (*GI).second = STE;
        (*GI).second->SetDoNotDelete();
        (*GI).second->SetGlobalScope();
        Id->SetGlobalScope();
        Id->SetSymbolTableEntry((*GI).second);
        return true;
      }

      if (!GSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
        std::stringstream M;
        M << "Insertion into the Gate SymbolTable failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      GI = GSTM.find(Id->GetName());
      assert((*GI).second && "Could not create a valid ASTSymbolTableEntry!");

      (*GI).second->SetDoNotDelete();
      Id->SetSymbolTableEntry((*GI).second);
      return true;
    } else if (Ty == ASTTypeQubit || Ty == ASTTypeQubitContainer ||
               Ty == ASTTypeQubitContainerAlias) {
      map_iterator QI = QSTM.find(Id->GetName());
      if (QI != QSTM.end()) {
        (*QI).second = STE;
        (*QI).second->SetDoNotDelete();
        Id->SetSymbolTableEntry((*QI).second);
        return true;
      }

      if (!QSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
        std::stringstream M;
        M << "Insertion into the Qubit SymbolTable failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      QI = QSTM.find(Id->GetName());
      assert((*QI).second && "Could not create a valid ASTSymbolTableEntry!");

      (*QI).second->SetDoNotDelete();
      Id->SetSymbolTableEntry((*QI).second);
      return true;
    }

    if (Id->GetSymbolScope() == ASTSymbolScope::Global) {
      map_iterator GI = GLSTM.find(Id->GetIdentifier()->GetName());
      if (GI != GLSTM.end()) {
        map_iterator GRI = GLSTM.find(Id->GetName());
        if (GRI == GLSTM.end()) {
          if (!GLSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
            std::stringstream M;
            M << "Insertion into the Global SymbolTable failed.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                           DiagLevel::ICE);
            return false;
          }

          GRI = GLSTM.find(Id->GetName());
          assert((*GRI).second &&
                 "Invalid ASTSymbolTableEntry in the Global SymbolTable!");
          (*GRI).second->SetDoNotDelete();
          Id->SetSymbolTableEntry((*GRI).second);
          return true;
        }

        return true;
      } else {
        if (!GLSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
          std::stringstream M;
          M << "Insertion into the Global SymbolTable failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        GI = GLSTM.find(Id->GetName());
        assert((*GI).second &&
               "Invalid ASTSymbolTableEntry in the Global SymbolTable!");
        (*GI).second->SetDoNotDelete();
        Id->SetSymbolTableEntry((*GI).second);
        return true;
      }
    } else if (Id->GetSymbolScope() == ASTSymbolScope::Local) {
      map_iterator LI = LSTM.find(Id->GetIdentifier()->GetName());
      if (LI != LSTM.end()) {
        if (!LSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
          std::stringstream M;
          M << "Insertion into the Global SymbolTable failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        LI = LSTM.find(Id->GetName());
        assert((*LI).second &&
               "Invalid ASTSymbolTableEntry in the Global SymbolTable!");
        (*LI).second->SetDoNotDelete();
        Id->SetSymbolTableEntry((*LI).second);
        return true;
      } else {
        if (!LSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
          std::stringstream M;
          M << "Insertion into the Global SymbolTable failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        LI = LSTM.find(Id->GetName());
        assert((*LI).second &&
               "Invalid ASTSymbolTableEntry in the Global SymbolTable!");
        (*LI).second->SetDoNotDelete();
        Id->SetSymbolTableEntry((*LI).second);
        return true;
      }
    }

    multimap_iterator R;
    multimap_iterator I = STM.find(Id->GetName());
    if (I == STM.end()) {
      R = STM.insert(std::make_pair(Id->GetName(), STE));
      if (R == STM.end())
        return false;

      Id->SetSymbolTableEntry((*R).second);
    } else {
      (*I).second = STE;
      Id->SetSymbolTableEntry((*I).second);
    }

    return true;
  }

  bool InsertLocalSymbol(const ASTIdentifierNode* Id,
                         ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    map_iterator LI = LSTM.find(Id->GetName());
    if (LI != LSTM.end()) {
      std::stringstream M;
      M << "A symbol of type " << PrintTypeEnum((*LI).second->GetValueType())
        << " already exists in the Local SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    return LSTM.insert(std::make_pair(Id->GetName(), STE)).second;
  }

  bool InsertLocalSymbol(const ASTIdentifierRefNode* IdR,
                         ASTSymbolTableEntry* STE) {
    assert(IdR && "Invalid ASTIdentifierRefNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    map_iterator LI = LSTM.find(IdR->GetName());
    if (LI != LSTM.end()) {
      std::stringstream M;
      M << "A symbol of type " << PrintTypeEnum((*LI).second->GetValueType())
        << " already exists in the Local SymbolTable.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(IdR), M.str(),
                                                      DiagLevel::ICE);
      return false;
    }

    return LSTM.insert(std::make_pair(IdR->GetName(), STE)).second;
  }

  bool InsertDefcalGroup(const ASTIdentifierNode* Id,
                         ASTMapSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTMapSymbolTableEntry argument!");

    map_iterator DI = DSTM.find(Id->GetName());
    if (DI != DSTM.end()) {
      std::stringstream M;
      M << "A Defcal Group with Identifier " << Id->GetName()
        << " already exists in DSTM.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (Id->GetSymbolType() != ASTTypeDefcalGroup) {
      std::stringstream M;
      M << "Identifier Symbol Type is not "
        << PrintTypeEnum(ASTTypeDefcalGroup) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
    }

    if (STE->GetValueType() != ASTTypeDefcalGroup) {
      std::stringstream M;
      M << "Symbol Table ValueType is not "
        << PrintTypeEnum(ASTTypeDefcalGroup) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
    }

    if (!DSTM.insert(std::make_pair(Id->GetName(), STE)).second) {
      std::stringstream M;
      M << "Failure inserting defcal group into DSTM.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    return true;
  }

  bool Update(const ASTIdentifierNode* Id, ASTSymbolTableEntry* OSTE,
              ASTSymbolTableEntry* NSTE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(OSTE && "Invalid ASTSymbolTableEntry argument!");
    assert(NSTE && "Invalid ASTSymbolTableEntry argument!");

    if (ASTCalContextBuilder::Instance().InOpenContext() ||
        ASTDefcalContextBuilder::Instance().InOpenContext()) {
      map_iterator CI = CSTM.find(Id->GetName());
      if (CI != CSTM.end()) {
        if ((*CI).second == OSTE) {
          (*CI).second = NSTE;
          const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(NSTE);
          return true;
        }
      }
    }

    map_iterator MI = GLSTM.find(Id->GetName());
    if (MI != GLSTM.end()) {
      if ((*MI).second == OSTE) {
        (*MI).second = NSTE;
        const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(NSTE);
        return true;
      }
    }

    MI = LSTM.find(Id->GetName());
    if (MI != LSTM.end()) {
      if ((*MI).second == OSTE) {
        (*MI).second = NSTE;
        const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(NSTE);
        return true;
      }
    }

    MI = USTM.find(Id->GetName());
    if (MI != USTM.end()) {
      if ((*MI).second == OSTE) {
        (*MI).second = NSTE;
        const_cast<ASTIdentifierNode*>(Id)->SetSymbolTableEntry(NSTE);
        return true;
      }
    }

    multimap_iterator I = STM.lower_bound(Id->GetName());
    assert(I != STM.end() && "Identifier not found in the SymbolTable!");

    multimap_iterator E = STM.upper_bound(Id->GetName());
    assert(E != STM.end() && "Identifier not found in the SymbolTable!");

    while (I != E) {
      if ((*I).second == OSTE) {
        (*I).second = NSTE;
        return true;
      }

      ++I;
    }

    return false;
  }

  bool UpdateGlobal(const ASTIdentifierNode* Id, ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    if (Id->GetSymbolType() != STE->GetValueType()) {
      std::stringstream M;
      M << "Type mismatch ASTIdentifierNode <-> ASTSymbolTableEntry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }

    map_iterator GI = GLSTM.find(Id->GetName());

    if (GI == GLSTM.end()) {
      std::stringstream M;
      M << "Identifier " << Id->GetName() << " does not have a "
        << "Global SymbolTable Entry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }

    if (!ASTIdentifierTypeController::Instance().CanTypeUpdate(
                                                 (*GI).second->GetValueType(),
                                                 STE->GetValueType())) {
      std::stringstream M;
      M << "Type update of SymbolTable Entry from "
        << PrintTypeEnum((*GI).second->GetValueType()) << "to "
        << PrintTypeEnum(STE->GetValueType()) << " is not allowed.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }

    (*GI).second = STE;
    return true;
  }

  bool UpdateLocal(const ASTIdentifierNode* Id, ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    if (Id->GetSymbolType() != STE->GetValueType()) {
      std::stringstream M;
      M << "Type mismatch ASTIdentifierNode <-> ASTSymbolTableEntry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }

    map_iterator LI = LSTM.find(Id->GetName());

    if (LI == LSTM.end()) {
      std::stringstream M;
      M << "Identifier " << Id->GetName() << " does not have a "
        << "Local SymbolTable Entry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }

    if (!ASTIdentifierTypeController::Instance().CanTypeUpdate(
                                                 (*LI).second->GetValueType(),
                                                 STE->GetValueType())) {
      std::stringstream M;
      M << "Type update of SymbolTable Entry from "
        << PrintTypeEnum((*LI).second->GetValueType()) << "to "
        << PrintTypeEnum(STE->GetValueType()) << " is not allowed.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }

    (*LI).second = STE;
    return true;
  }

  ASTSymbolTableEntry* Lookup(const std::string& S) const {
    if (S.empty())
      return nullptr;

    if (ASTCalContextBuilder::Instance().InOpenContext() ||
        ASTDefcalContextBuilder::Instance().InOpenContext()) {
      map_iterator CI = CSTM.find(S);
      if (CI != CSTM.end()) {
        ASTType Ty = (*CI).second->GetIdentifier()->GetSymbolType();

        switch (Ty) {
        case ASTTypeGate:
        case ASTTypeCXGate:
        case ASTTypeCCXGate:
        case ASTTypeCNotGate:
        case ASTTypeHadamardGate:
        case ASTTypeUGate:
        case ASTTypeDefcal:
        case ASTTypeDefcalGroup:
        case ASTTypeFunction:
        case ASTTypeFunctionDeclaration:
        case ASTTypeQubit:
        case ASTTypeQubitContainer:
        case ASTTypeQubitContainerAlias:
        case ASTTypeGateQubitParam:
          break;
        default:
          return (*CI).second;
          break;
      }
      }
    }

    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(S)) {
      map_iterator I = ASTM.find(S);
      if (I != ASTM.end())
        return (*I).second;
    }

    if (ASTAngleContextControl::Instance().InOpenContext() &&
        ASTTypeSystemBuilder::Instance().IsImplicitAngle(S)) {
      map_iterator I = ASTM.find(S);
      if (I != ASTM.end())
        return (*I).second;
    }

    map_iterator I = LSTM.find(S);
    if (I != LSTM.end())
      return (*I).second;

    I = GSTM.find(S);
    if (I != GSTM.end())
      return (*I).second;

    I = QSTM.find(S);
    if (I != QSTM.end())
      return (*I).second;

    I = FSTM.find(S);
    if (I != FSTM.end())
      return (*I).second;

    I = DSTM.find(S);
    if (I != DSTM.end())
      return (*I).second;

    I = GLSTM.find(S);
    if (I != GLSTM.end())
      return (*I).second;

    I = USTM.find(S);
    if (I != USTM.end())
      return (*I).second;

    multimap_iterator MI = STM.find(S);
    return MI == STM.end() ? nullptr : (*MI).second;
  }

  ASTSymbolTableEntry* Lookup(const std::string& S, unsigned Bits,
                              ASTType Ty) {
    if (S.empty())
      return nullptr;

    if (ASTCalContextBuilder::Instance().InOpenContext() ||
        ASTDefcalContextBuilder::Instance().InOpenContext()) {
      switch (Ty) {
      case ASTTypeGate:
      case ASTTypeCXGate:
      case ASTTypeCCXGate:
      case ASTTypeCNotGate:
      case ASTTypeHadamardGate:
      case ASTTypeUGate:
      case ASTTypeDefcal:
      case ASTTypeDefcalGroup:
      case ASTTypeFunction:
      case ASTTypeFunctionDeclaration:
      case ASTTypeQubit:
      case ASTTypeQubitContainer:
      case ASTTypeQubitContainerAlias:
      case ASTTypeGateQubitParam:
        break;
      default: {
        map_iterator CI = CSTM.find(S);
        if (CI != CSTM.end()) {
          if ((*CI).second && (*CI).second->GetValueType() == Ty)
            return (*CI).second;
        }
      }
        break;
      }
    }

    if (ASTAngleContextControl::Instance().InOpenContext() &&
        ASTTypeSystemBuilder::Instance().IsImplicitAngle(S)) {
      map_iterator I = ASTM.find(S);
      if (I != ASTM.end()) {
        if ((*I).second && (*I).second->GetValueType() == Ty &&
            (*I).second->GetIdentifier()->GetBits() == Bits) {
          return (*I).second;
        }
      } else {
        I = GLSTM.find(S);
        if (I != GLSTM.end()) {
          if ((*I).second && (*I).second->GetValueType() == Ty &&
              (*I).second->GetIdentifier()->GetBits() == Bits) {
            if (!ASTM.insert(std::make_pair((*I).first, (*I).second)).second) {
              std::stringstream M;
              M << "Failed to transfer angle " << S << " from Global "
                << "Symbol Table to Angle Symbol Table.";
              QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                           DiagLevel::ICE);
            }

            GLSTM.erase(I);
            I = ASTM.find(S);
            assert(I != ASTM.end() && "Angle not found in the Angle SymbolTable!");
            return (*I).second;
          }
        } else {
          I = LSTM.find(S);
          if (I != LSTM.end()) {
            if ((*I).second && (*I).second->GetValueType() == Ty &&
                (*I).second->GetIdentifier()->GetBits() == Bits) {
              return (*I).second;
            }
          }
        }
      }

      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
      std::vector<multimap_iterator> IV;

      ASTSymbolTableEntry* STE = nullptr;

      if (R.first != STM.end()) {
        for (multimap_iterator MI = R.first; MI != R.second; ++MI) {
          if ((*MI).first == S &&
              (*MI).second->GetIdentifier()->GetBits() == Bits &&
              (*MI).second->GetValueType() == Ty) {
            if (ASTM.insert(std::make_pair((*MI).first, (*MI).second)).second) {
              IV.push_back(MI);
              STE = (*MI).second;
            } else {
              std::stringstream M;
              M << "Unexpected failure inserting a reserved Angle "
                << "into the SymbolTable!";
              QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                           DiagLevel::ICE);
              return nullptr;
            }
          }
        }
      }

      if (!IV.empty()) {
        for (std::vector<multimap_iterator>::iterator VI = IV.begin();
             VI != IV.end(); ++VI)
          STM.erase(*VI);
      }

      if (STE) return STE;
    }

    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(S)) {
      map_iterator I = ASTM.find(S);
      if (I != ASTM.end()) {
        if ((*I).second && (*I).second->GetValueType() == Ty &&
            (*I).second->GetIdentifier()->GetBits() == Bits)
          return (*I).second;
      }

      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
      std::vector<multimap_iterator> IV;

      ASTSymbolTableEntry* STE = nullptr;

      if (R.first != STM.end()) {
        for (multimap_iterator MI = R.first; MI != R.second; ++MI) {
          if ((*MI).first == S &&
              (*MI).second->GetIdentifier()->GetBits() == Bits &&
              (*MI).second->GetValueType() == Ty) {
            if (ASTM.insert(std::make_pair((*MI).first, (*MI).second)).second) {
              IV.push_back(MI);
              STE = (*MI).second;
            } else {
              std::stringstream M;
              M << "Unexpected failure inserting a reserved Angle "
                << "into the SymbolTable!";
              QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                           DiagLevel::ICE);
              return nullptr;
            }
          }
        }
      }

      if (!IV.empty()) {
        for (std::vector<multimap_iterator>::iterator VI = IV.begin();
             VI != IV.end(); ++VI)
          STM.erase(*VI);
      }

      if (STE) return STE;
    }

    map_iterator LI = LSTM.find(S);

    if (LI != LSTM.end() && (*LI).second) {
      if ((*LI).second->GetValueType() == Ty &&
          (*LI).second->GetIdentifier()->GetBits() == Bits)
        return (*LI).second;
    }

    map_iterator GI = GLSTM.find(S);

    if (GI != GLSTM.end() && (*GI).second) {
      if ((*GI).second->GetValueType() == Ty &&
          (*GI).second->GetIdentifier()->GetBits() == Bits) {
        const ASTDeclarationContext* CTX =
          (*GI).second->GetIdentifier()->GetDeclarationContext();
        if (!ASTDeclarationContextTracker::Instance().IsGlobalContext(CTX) &&
            !ASTIdentifierTypeController::Instance().TypeScopeIsAlwaysGlobal(Ty)) {
          if (!LSTM.insert(std::make_pair((*GI).first, (*GI).second)).second) {
            std::stringstream M;
            M << "Failed transfer to Local Symbol Table from Global "
              << "Symbol Table.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
            return nullptr;
          }

          GLSTM.erase(GI);
          LI = LSTM.find(S);
          return (*LI).second;
        }

        return (*GI).second;
      }
    }

    switch (Ty) {
    case ASTTypeAngle: {
      map_iterator AI = ASTM.find(S);
      if (AI != ASTM.end()) {
        if (((*AI).second && (*AI).second->GetValueType() == Ty &&
            (*AI).second->GetIdentifier()->GetBits() == Bits) ||
            ((*AI).second && (*AI).second->GetValueType() == Ty &&
             ASTStringUtils::Instance().IsIndexed(S) &&
             Bits < (*AI).second->GetIdentifier()->GetBits()))
          return (*AI).second;
      } else {
        std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
        std::vector<multimap_iterator> IV;

        if (R.first != STM.end()) {
          for (multimap_iterator I = R.first; I != R.second; ++I) {
            IV.push_back(I);
          }
        }

        if (!IV.empty()) {
          for (std::vector<multimap_iterator>::iterator I = IV.begin();
               I != IV.end(); ++I) {
            AI = *I;
            if ((*AI).second && (*AI).second->GetValueType() == Ty &&
                (*AI).second->GetIdentifier()->GetBits() == Bits &&
                ((*AI).second->IsGlobalScope() ||
                 (*AI).second->GetIdentifier()->IsGlobalScope() ||
                 ASTTypeSystemBuilder::Instance().IsReservedAngle(S))) {
              if (!ASTM.insert(std::make_pair((*AI).first, (*AI).second)).second) {
                std::stringstream M;
                const ASTIdentifierNode* Id = (*AI).second->GetIdentifier();
                M << "Failure relocating Angle " << S << " to the Global "
                  << "Symbol Table.";
                QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                  DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                               DiagLevel::ICE);
                return nullptr;
              }

              STM.erase(*I);
              return (*AI).second;
            } else if ((*AI).second && (*AI).second->GetValueType() == Ty &&
                       (*AI).second->GetIdentifier()->GetBits() == Bits &&
                       ((*AI).second->IsLocalScope() ||
                         (*AI).second->GetIdentifier()->IsLocalScope())) {
              return (*AI).second;
            }
          }
        }
      }

      return nullptr;
    }
      break;
    case ASTTypeCNotGate:
    case ASTTypeCCXGate:
    case ASTTypeCXGate:
    case ASTTypeHadamardGate:
    case ASTTypeUGate:
    case ASTTypeGate: {
      GI = GSTM.find(S);
      if (GI != GSTM.end()) {
        if ((*GI).second &&
            ((*GI).second->GetValueType() == Ty ||
             (*GI).second->GetValueType() == ASTTypeGate ||
             (*GI).second->GetValueType() == ASTTypeCXGate ||
             (*GI).second->GetValueType() == ASTTypeCCXGate ||
             (*GI).second->GetValueType() == ASTTypeCNotGate ||
             (*GI).second->GetValueType() == ASTTypeHadamardGate ||
             (*GI).second->GetValueType() == ASTTypeUGate) &&
            (*GI).second->GetIdentifier()->GetBits() == Bits)
          return (*GI).second;
      } else {
        std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
        std::vector<multimap_iterator> IV;

        if (R.first != STM.end()) {
          for (multimap_iterator I = R.first; I != R.second; ++I) {
            IV.push_back(I);
          }
        }

        if (!IV.empty()) {
          for (std::vector<multimap_iterator>::iterator I = IV.begin();
               I != IV.end(); ++I) {
            GI = *I;
            if ((*GI).second && (*GI).second->GetValueType() == Ty &&
                (*GI).second->GetIdentifier()->GetBits() == Bits) {
              if (!GSTM.insert(std::make_pair((*GI).first, (*GI).second)).second) {
                std::stringstream M;
                const ASTIdentifierNode* Id = (*GI).second->GetIdentifier();
                M << "Failure relocating Angle " << S << " to the Global "
                  << "Symbol Table.";
                QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                  DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                               DiagLevel::ICE);
                return nullptr;
              }

              STM.erase(*I);
              return (*GI).second;
            }
          }
        }
      }

      return nullptr;
    }
      break;
    case ASTTypeKernel:
    case ASTTypeFunction: {
      map_iterator FI = FSTM.find(S);

      if (FI != FSTM.end()) {
        if ((*FI).second && (*FI).second->GetValueType() == Ty &&
            (*FI).second->GetIdentifier()->GetBits() == Bits)
          return (*FI).second;
      } else {
        std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
        std::vector<multimap_iterator> IV;

        if (R.first != STM.end()) {
          for (multimap_iterator I = R.first; I != R.second; ++I) {
            IV.push_back(I);
          }
        }

        if (!IV.empty()) {
          for (std::vector<multimap_iterator>::iterator I = IV.begin();
               I != IV.end(); ++I) {
            FI = *I;
            if ((*FI).second && (*FI).second->GetValueType() == Ty &&
                (*FI).second->GetIdentifier()->GetBits() == Bits) {
              if (!FSTM.insert(std::make_pair((*FI).first, (*FI).second)).second) {
                std::stringstream M;
                const ASTIdentifierNode* Id = (*FI).second->GetIdentifier();
                M << "Failure relocating Function " << S << " to the Global "
                  << "Symbol Table.";
                QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                  DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                               DiagLevel::ICE);
                return nullptr;
              }

              STM.erase(*I);
              return (*FI).second;
            }
          }
        }
      }

      return nullptr;
    }
      break;
    case ASTTypeDefcal:
    case ASTTypeDefcalGroup: {
      map_iterator DI = DSTM.find(S);

      if (DI != DSTM.end()) {
        if ((*DI).second && (*DI).second->GetValueType() == Ty &&
            (*DI).second->GetIdentifier()->GetBits() == Bits) {
          return (*DI).second;
        }
      } else {
        std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
        std::vector<multimap_iterator> IV;

        if (R.first != STM.end()) {
          for (multimap_iterator I = R.first; I != R.second; ++I) {
            IV.push_back(I);
          }
        }

        if (!IV.empty()) {
          for (std::vector<multimap_iterator>::iterator I = IV.begin();
               I != IV.end(); ++I) {
            DI = *I;
            if ((*DI).second && (*DI).second->GetValueType() == Ty &&
                (*DI).second->GetIdentifier()->GetBits() == Bits) {
              if (!DSTM.insert(std::make_pair((*DI).first, (*DI).second)).second) {
                std::stringstream M;
                const ASTIdentifierNode* Id = (*DI).second->GetIdentifier();
                M << "Failure relocating Defcal " << S << " to the Global "
                  << "Symbol Table.";
                QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                  DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                               DiagLevel::ICE);
                return nullptr;
              }

              STM.erase(*I);
              return (*DI).second;
            }
          }
        }
      }

      return nullptr;
    }
      break;
    case ASTTypeQubit:
    case ASTTypeQubitContainer:
    case ASTTypeQubitContainerAlias: {
      map_iterator QI = QSTM.find(S);

      if (QI != QSTM.end()) {
        switch (Ty) {
        case ASTTypeQubit:
        case ASTTypeQubitContainer:
        case ASTTypeQubitContainerAlias:
          if ((*QI).second && ((*QI).second->GetValueType() == ASTTypeQubit ||
               (*QI).second->GetValueType() == ASTTypeQubitContainer ||
               (*QI).second->GetValueType() == ASTTypeQubitContainerAlias) &&
              ((*QI).second->GetIdentifier()->GetBits() == Bits ||
               Bits == 0)) {
            return (*QI).second;
          } else if (ASTStringUtils::Instance().IsQCElement(S)) {
            if ((*QI).second && (*QI).second->GetValueType() == Ty &&
                Bits <= (*QI).second->GetIdentifier()->GetBits())
              return (*QI).second;
          }
          break;
        default:
          if (ASTStringUtils::Instance().IsIndexed(S)) {
            if ((*QI).second && (*QI).second->GetValueType() == Ty &&
                Bits < (*QI).second->GetIdentifier()->GetBits())
              return (*QI).second;
          } else if (ASTStringUtils::Instance().IsQCElement(S)) {
            if ((*QI).second && (*QI).second->GetValueType() == Ty &&
                Bits <= (*QI).second->GetIdentifier()->GetBits())
              return (*QI).second;
          } else {
            if ((*QI).second && (*QI).second->GetValueType() == Ty &&
                Bits <= (*QI).second->GetIdentifier()->GetBits())
              return (*QI).second;
          }
          break;
        }
      }

      return nullptr;
    }
      break;
    case ASTTypeGateQubitParam: {
      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);

      if (R.first != STM.end()) {
        for (multimap_iterator I = R.first; I != R.second; ++I) {
          if ((*I).second &&
              (*I).second->GetIdentifier()->GetBits() == Bits &&
              (*I).second->GetValueType() == Ty)
            return (*I).second;
        }

        return nullptr;
      }
    }
      break;
    case ASTTypeUndefined: {
      if (S[0] == '$' || S[0] == '%') {
        map_iterator QI = QSTM.find(S);

        if (QI != QSTM.end()) {
          ASTIdentifierNode* QId =
            const_cast<ASTIdentifierNode*>((*QI).second->GetIdentifier());
          assert(QId && "Invalid ASTSymbolTableEntry ASTIdentifierNode!");

          if (Bits == 0) {
            if ((*QI).second->GetIdentifier()->GetBits() > 1) {
              (*QI).second->SetValueType(ASTTypeQubitContainer);
              QId->SetSymbolType(ASTTypeQubitContainer);
              QId->SetSymbolTableEntry((*QI).second);
            } else {
              (*QI).second->SetValueType(ASTTypeQubit);
              QId->SetSymbolType(ASTTypeQubit);
              QId->SetSymbolTableEntry((*QI).second);
            }
          } else if (Bits == 1) {
            if ((*QI).second->GetIdentifier()->GetBits() > 1) {
              (*QI).second->SetValueType(ASTTypeQubit);
              QId->SetSymbolType(ASTTypeQubit);
            } else {
              (*QI).second->SetValueType(ASTTypeQubitContainer);
              QId->SetSymbolType(ASTTypeQubitContainer);
            }
          } else {
            if ((*QI).second->GetIdentifier()->GetBits() > 1) {
              (*QI).second->SetValueType(ASTTypeQubitContainer);
              QId->SetSymbolType(ASTTypeQubitContainer);
              QId->SetSymbolTableEntry((*QI).second);
            } else {
              (*QI).second->SetValueType(ASTTypeQubit);
              QId->SetSymbolType(ASTTypeQubit);
              QId->SetSymbolTableEntry((*QI).second);
            }
          }

          return (*QI).second;
        }

        return nullptr;
      } else {
        map_iterator QI = QSTM.find(S);
        if (QI != QSTM.end() && (*QI).second) {
          switch ((*QI).second->GetValueType()) {
          case ASTTypeUndefined:
            (*QI).second->SetValueType(ASTTypeQubitContainer);
            break;
          default:
            break;
          }

          return (*QI).second;
        }

        GI = GSTM.find(S);
        if (GI != GSTM.end() && (*GI).second) {
          switch ((*GI).second->GetValueType()) {
          case ASTTypeUndefined:
            (*GI).second->SetValueType(ASTTypeGate);
            break;
          default:
            break;
          }

          return (*GI).second;
        }

        map_iterator AI = ASTM.find(S);
        if (AI != ASTM.end() && (*AI).second) {
          switch ((*AI).second->GetValueType()) {
          case ASTTypeUndefined:
            (*AI).second->SetValueType(ASTTypeAngle);
            break;
          default:
            break;
          }

          return (*AI).second;
        }

        LI = LSTM.find(S);
        if (LI != LSTM.end() && (*LI).second)
          return (*LI).second;

        map_iterator UI = USTM.find(S);
        if (UI != USTM.end() && (*UI).second)
          return (*UI).second;
      }
    }
      break;
    default: {
      LI = LSTM.find(S);
      if (LI != LSTM.end() && (*LI).second)
        return (*LI).second;

      map_iterator UI = USTM.find(S);
      if (UI != USTM.end() && (*UI).second) {
        if ((*UI).second->GetValueType() != ASTTypeUndefined) {
          if (!LSTM.insert(std::make_pair((*UI).first, (*UI).second)).second) {
            std::stringstream M;
            M << "Failure transferring symbol to the Local Symbol Table.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
          }

          LI = LSTM.find(S);
          if (LI != LSTM.end() && (*LI).second) {
            USTM.erase(UI);
            return (*LI).second;
          }
        } else {
          return (*UI).second;
        }
      }
    }
      break;
    }

    std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);

    if (R.first != STM.end()) {
      for (multimap_iterator I = R.first; I != R.second; ++I) {
        if (const ASTIdentifierRefNode* IdR =
            dynamic_cast<const ASTIdentifierRefNode*>((*I).second->GetIdentifier())) {
          if ((*I).first == S && IdR->GetBits() == Bits &&
              (*I).second->GetValueType() == Ty)
            return (*I).second;
        } else if (ASTStringUtils::Instance().IsIndexed(S) &&
                   ASTStringUtils::Instance().IsIndexed((*I).first) &&
                   (*I).first == S &&
                   Bits < (*I).second->GetIdentifier()->GetBits() &&
                   (*I).second->GetValueType() == Ty) {
          return (*I).second;
        } else {
          if ((*I).first == S &&
              (*I).second->GetIdentifier()->GetBits() == Bits &&
              (*I).second->GetValueType() == Ty)
            return (*I).second;
        }
      }
    }

    return nullptr;
  }

  ASTSymbolTableEntry* Lookup(const ASTIdentifierNode* Id, unsigned Bits,
                              ASTType Ty) {
    return Id ? Lookup(Id->GetName(), Bits, Ty) : nullptr;
  }

  ASTSymbolTableEntry* Lookup(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid ASTIdentifierNode!");

    if (Id->GetBits() != static_cast<unsigned>(~0x0) &&
        Id->GetSymbolTableEntry()->GetValueType() != ASTTypeUndefined)
      return Lookup(Id->GetName(), Id->GetBits(),
                    Id->GetSymbolTableEntry()->GetValueType());

    return Lookup(Id->GetName());
  }

  ASTSymbolTableEntry* Lookup(const std::string& S, ASTType Ty) const {
    if (!S.empty()) {
      map_iterator SI;

      if (ASTCalContextBuilder::Instance().InOpenContext() ||
          ASTDefcalContextBuilder::Instance().InOpenContext()) {
        switch (Ty) {
        case ASTTypeCXGate:
        case ASTTypeCCXGate:
        case ASTTypeCNotGate:
        case ASTTypeHadamardGate:
        case ASTTypeUGate:
        case ASTTypeGate:
        case ASTTypeDefcal:
        case ASTTypeDefcalGroup:
        case ASTTypeFunction:
        case ASTTypeFunctionDeclaration:
        case ASTTypeQubit:
        case ASTTypeQubitContainer:
        case ASTTypeQubitContainerAlias:
        case ASTTypeGateQubitParam:
          break;
        default: {
          if (ASTDefcalContextBuilder::Instance().InOpenContext()) {
            SI = CSTM.find(S);
            if (SI != CSTM.end()) {
              if ((*SI).second && (*SI).second->GetValueType() == Ty)
                return (*SI).second;
            }
          }
        }
          break;
        }
      }

      switch (Ty) {
      case ASTTypeGate:
      case ASTTypeCXGate:
      case ASTTypeCCXGate:
      case ASTTypeCNotGate:
      case ASTTypeHadamardGate:
      case ASTTypeUGate:
        SI = GSTM.find(S);
        if (SI != GSTM.end())
          return (*SI).second;
        break;
      case ASTTypeQubit:
      case ASTTypeQubitContainer:
      case ASTTypeQubitContainerAlias:
        SI = QSTM.find(S);
        if (SI != QSTM.end())
          return (*SI).second;
        break;
      case ASTTypeKernel:
      case ASTTypeFunction:
        SI = FSTM.find(S);
        if (SI != FSTM.end())
          return (*SI).second;
        break;
      case ASTTypeDefcal:
      case ASTTypeDefcalGroup:
        SI = DSTM.find(S);
        if (SI != DSTM.end())
          return (*SI).second;
        break;
      case ASTTypeAngle:
      case ASTTypeLambdaAngle:
      case ASTTypePhiAngle:
      case ASTTypeThetaAngle:
        SI = ASTM.find(S);
        if (SI != ASTM.end())
          return (*SI).second;
        // Angles can be polymorphical to a representation
        // consisting of three bitsets or three integer points.
        SI = GLSTM.find(S);
        if (SI != GLSTM.end() &&
            ((*SI).second->GetValueType() == Ty ||
             (*SI).second->GetValueType() == ASTTypeBitset ||
             (*SI).second->GetValueType() == ASTTypeInt))
          return (*SI).second;
        break;
      default:
        SI = GLSTM.find(S);
        if (SI != GLSTM.end() &&
            ((*SI).second->GetValueType() == Ty ||
             (*SI).second->GetIdentifier()->GetSymbolType() == Ty))
          return (*SI).second;
        SI = LSTM.find(S);
        if (SI != LSTM.end() &&
            ((*SI).second->GetValueType() == Ty ||
             (*SI).second->GetIdentifier()->GetSymbolType() == Ty))
          return (*SI).second;
        SI = USTM.find(S);
        if (SI != USTM.end() &&
            ((*SI).second->GetValueType() == Ty ||
             (*SI).second->GetIdentifier()->GetSymbolType() == Ty))
          return (*SI).second;
        break;
      }
    }

    map_iterator LI = LSTM.find(S);
    if (LI != LSTM.end()) {
      if ((*LI).second->GetValueType() == Ty &&
          (*LI).second->GetIdentifier()->GetSymbolType() == Ty)
        return (*LI).second;
    }

    return nullptr;
  }

  const ASTIdentifierNode* LookupGate(const std::string& S) const {
    if (S.empty())
      return nullptr;

    map_const_iterator I = GSTM.find(S);
    return I == GSTM.end() ? nullptr : (*I).second->GetIdentifier();
  }

  const ASTIdentifierNode* LookupDefcal(const std::string& S) const {
    if (S.empty())
      return nullptr;

    map_const_iterator I = DSTM.find(S);
    return I == DSTM.end() ? nullptr : (*I).second->GetIdentifier();
  }

  const ASTIdentifierNode* LookupDefcalGroup(const std::string& S) const {
    return LookupDefcal(S);
  }

  std::vector<const ASTSymbolTableEntry*>
  GetDefcalGroup(const std::string& S) const {
    std::vector<const ASTSymbolTableEntry*> RV;

    if (S.empty())
      return RV;

    map_iterator DI = DSTM.find(S);

    if (DI != DSTM.end()) {
      if (ASTMapSymbolTableEntry* DSTE =
        dynamic_cast<ASTMapSymbolTableEntry*>((*DI).second)) {
        ASTMapSymbolTableEntry::map_const_iterator DMI;
        for (DMI = DSTE->GetMap().begin();
             DMI != DSTE->GetMap().end(); ++DMI) {
          RV.push_back((*DMI).second);
        }
      }
    }

    return RV;
  }

  const ASTIdentifierNode* LookupFunction(const std::string& S) const {
    if (S.empty())
      return nullptr;

    map_const_iterator I = FSTM.find(S);
    return I == FSTM.end() ? nullptr : (*I).second->GetIdentifier();
  }

  const ASTIdentifierNode* LookupAngle(const std::string& S) const {
    if (S.empty())
      return nullptr;

    map_const_iterator I = ASTM.find(S);
    return I == ASTM.end() ? nullptr : (*I).second->GetIdentifier();
  }

  const ASTIdentifierNode* LookupQubit(const std::string& S) const {
    map_const_iterator I = QSTM.find(S);
    return I == QSTM.end() ? nullptr : (*I).second->GetIdentifier();
  }

  const ASTIdentifierNode* LookupLocal(const std::string& S) const {
    map_const_iterator I = LSTM.find(S);
    return I == LSTM.end() ? nullptr : (*I).second->GetIdentifier();
  }

  const ASTSymbolTableEntry* LookupLocal(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    map_const_iterator I = LSTM.find(Id->GetName());
    return I == LSTM.end() ? nullptr : (*I).second;
  }

  const ASTSymbolTableEntry* LookupLocal(const std::string& N, unsigned Bits,
                                         ASTType Ty) const {
    map_const_iterator I = LSTM.find(N);

    if (I != LSTM.end()) {
      if ((*I).second && (*I).second->GetValueType() == Ty &&
          (*I).second->GetIdentifier()->GetBits() == Bits &&
          (*I).second->GetIdentifier()->GetSymbolType() == Ty)
        return (*I).second;
    }

    return nullptr;
  }

  ASTSymbolTableEntry* LookupLocal(const std::string& N, unsigned Bits,
                                   ASTType Ty) {
    map_iterator I = LSTM.find(N);

    if (I != LSTM.end()) {
      if ((*I).second && (*I).second->GetValueType() == Ty &&
          (*I).second->GetIdentifier()->GetBits() == Bits &&
          (*I).second->GetIdentifier()->GetSymbolType() == Ty)
        return (*I).second;
    }

    return nullptr;
  }

  const ASTIdentifierNode* LookupGlobal(const std::string& S) const {
    map_const_iterator I = GLSTM.find(S);
    return I == GLSTM.end() ? nullptr : (*I).second->GetIdentifier();
  }

  const ASTSymbolTableEntry* LookupGlobal(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    map_const_iterator I = GLSTM.find(Id->GetName());
    return I == GLSTM.end() ? nullptr : (*I).second;
  }

  const ASTIdentifierNode* LookupUndefined(const std::string& S) const {
    map_const_iterator I = USTM.find(S);
    return I == USTM.end() ? nullptr : (*I).second->GetIdentifier();
  }

  std::vector<const ASTIdentifierNode*> LookupGeneric(const std::string& S) const {
    std::vector<const ASTIdentifierNode*> V;

    if (!S.empty()) {
      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
      if (R.first != STM.end()) {
        for (multimap_iterator I = R.first; I != R.second; ++I) {
          if ((*I).second->GetIdentifierType() == ASTTypeIdentifierRef) {
            V.push_back((*I).second->GetIdentifierRef());
          } else {
            V.push_back((*I).second->GetIdentifier());
          }
        }
      }

      if (ASTStringUtils::Instance().IsIndexed(S) && V.empty()) {
        std::string SB = ASTStringUtils::Instance().GetIdentifierBase(S);
        R = STM.equal_range(SB);
        if (R.first != STM.end()) {
          for (multimap_iterator I = R.first; I != R.second; ++I) {
            if ((*I).second->GetIdentifierType() == ASTTypeIdentifierRef) {
              V.push_back((*I).second->GetIdentifierRef());
            } else {
              V.push_back((*I).second->GetIdentifier());
            }
          }
        }
      }
    }

    return V;
  }

  std::vector<ASTSymbolTableEntry*> LookupRange(const std::string& S) {
    std::vector<ASTSymbolTableEntry*> V;

    if (S.empty())
      return V;

    if (ASTAngleContextControl::Instance().InOpenContext() &&
        ASTTypeSystemBuilder::Instance().IsImplicitAngle(S)) {
      map_iterator I = ASTM.find(S);
      if (I != ASTM.end())
        V.push_back((*I).second);
    } else {
      map_iterator MI = GSTM.find(S);
      if (MI != GSTM.end()) {
        V.push_back((*MI).second);
        return V;
      }

      MI = QSTM.find(S);
      if (MI != QSTM.end()) {
        V.push_back((*MI).second);
        return V;
      }

      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);

      if (R.first != STM.end()) {
        for (multimap_iterator I = R.first; I != R.second; ++I)
          V.push_back((*I).second);
      }
    }

    return V;
  }

  std::vector<ASTSymbolTableEntry*> LookupRange(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return LookupRange(Id->GetName());
  }

  void Erase(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    const std::string& S = Id->GetName();

    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(S)) {
      return;
    } else if (ASTTypeSystemBuilder::Instance().IsBuiltinFunction(S)) {
      return;
    }

    if (ASTAngleContextControl::Instance().InOpenContext() &&
        ASTTypeSystemBuilder::Instance().IsImplicitAngle(S)) {
      map_iterator I = ASTM.find(S);
      if (I != ASTM.end()) {
        ASTM.erase(S);
      }
    } else {
      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
      std::vector<multimap_iterator> IV;

      if (R.first != STM.end()) {
        for (multimap_iterator I = R.first; I != R.second; ++I) {
          if (!(*I).second->IsDoNotDelete()) {
            IV.push_back(I);
          }
        }
      }

      if (!IV.empty()) {
        for (std::vector<multimap_iterator>::iterator I = IV.begin();
             I != IV.end(); ++I) {
          STM.erase(*I);
        }
      }
    }
  }

  void EraseQubit(const std::string& Q) {
    if (!Q.empty()) {
      if (Q[0] == '$') {
        std::stringstream M;
        M << "Bound Qubits cannot be deleted.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
        return;
      }

      map_iterator QI = QSTM.find(Q);
      if (QI != QSTM.end()) {
        unsigned QS = (*QI).second->GetIdentifier()->GetBits();
        std::stringstream QSS;

        for (unsigned I = 0; I < QS; ++I) {
          QSS.str("");
          QSS.clear();
          QSS << '%' << Q << ':' << I;
          QSTM.erase(QSS.str());

          QSS.str("");
          QSS.clear();
          QSS << Q << '[' << I << ']';
          QSTM.erase(QSS.str());
        }

        QSTM.erase(QI);
      }
    }
  }

  void EraseLocalQubit(const std::string& Q) {
    if (!Q.empty()) {
      if (Q[0] == u8'$') {
        std::stringstream M;
        M << "Bound Qubits cannot be deleted.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
        return;
      }

      map_iterator QI = LSTM.find(Q);
      if (QI != LSTM.end()) {
        unsigned QS = (*QI).second->GetIdentifier()->GetBits();
        std::stringstream QSS;

        for (unsigned I = 0; I < QS; ++I) {
          QSS.str("");
          QSS.clear();
          QSS << '%' << Q << ':' << I;
          LSTM.erase(QSS.str());

          QSS.str("");
          QSS.clear();
          QSS << Q << '[' << I << ']';
          LSTM.erase(QSS.str());
        }

        LSTM.erase(QI);
      }
    }
  }

  void EraseLocalQubit(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    if (Id->GetName()[0] == u8'$') {
      std::stringstream M;
      M << "Bound Qubits cannot be deleted.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::Error);
      return;
    }

    switch (Id->GetSymbolType()) {
    case ASTTypeQubit:
    case ASTTypeQubitContainer:
    case ASTTypeQubitContainerAlias:
      break;
    default:
      return;
      break;
    }

    const std::string& Q = Id->GetName();

    map_iterator QI = LSTM.find(Q);
    if (QI != LSTM.end()) {
      std::stringstream QSS;

      for (unsigned I = 0; I < Id->GetBits(); ++I) {
        QSS.str("");
        QSS.clear();
        QSS << '%' << Q << ':' << I;
        LSTM.erase(QSS.str());

        QSS.str("");
        QSS.clear();
        QSS << Q << '[' << I << ']';
        LSTM.erase(QSS.str());
      }

      LSTM.erase(QI);
    }
  }

  void EraseLocalQubitParam(const std::string& Q) {
    if (!Q.empty()) {
      if (Q[0] == u8'$') {
        std::stringstream M;
        M << "Bound Qubits cannot be deleted.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
        return;
      }

      map_iterator QI = LSTM.find(Q);
      if (QI != LSTM.end()) {
        if ((*QI).second &&
            ((*QI).second->GetIdentifier()->GetSymbolType() == ASTTypeGateQubitParam ||
             (*QI).second->GetValueType() == ASTTypeGateQubitParam))
          LSTM.erase(Q);
      }
    }
  }

  void EraseLocalQubitParam(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(Id->GetSymbolType() == ASTTypeGateQubitParam &&
           "Identifier Type is not an ASTTypeGateQubitParam!");

    if (Id->GetSymbolType() == ASTTypeGateQubitParam)
      EraseLocalQubitParam(Id->GetName());
  }

  void EraseQubit(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    switch (Id->GetSymbolType()) {
    case ASTTypeQubit:
    case ASTTypeQubitContainer:
    case ASTTypeQubitContainerAlias:
      EraseQubit(Id->GetName());
      break;
    default:
      break;
    }
  }

  void EraseGateLocalQubit(const std::string& Q) {
    if (!Q.empty()) {
      if (Q[0] == u8'$') {
        std::stringstream M;
        M << "Bound Qubits cannot be deleted.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
        return;
      }

      map_iterator QI = LSTM.find(Q);
      if (QI != LSTM.end()) {
        if ((*QI).second && ((*QI).second->GetValueType() == ASTTypeQubit ||
                             (*QI).second->GetValueType() == ASTTypeQubitContainer ||
                             (*QI).second->GetValueType() == ASTTypeQubitContainerAlias ||
                             (*QI).second->GetValueType() == ASTTypeGateQubitParam)) {
          LSTM.erase(QI);
        }
      }
    }
  }

  void EraseGateLocalQubit(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    EraseGateLocalQubit(Id->GetName());
  }

  void EraseGateQubitParam(const std::string& Q, unsigned Bits, ASTType Ty) {
    if (Ty != ASTTypeGateQubitParam)
      return;

    map_iterator QI = QSTM.find(Q);
    if (QI != QSTM.end()) {
      if ((*QI).second->GetIdentifier()->GetBits() == Bits &&
          (*QI).second->GetValueType() == Ty) {
        QSTM.erase(Q);
      }
    } else {
      QI = LSTM.find(Q);
      if (QI != LSTM.end()) {
        if ((*QI).second->GetIdentifier()->GetBits() == Bits &&
            (*QI).second->GetValueType() == Ty) {
          LSTM.erase(Q);
        }
      } else {
        QI = GLSTM.find(Q);
        if (QI != GLSTM.end()) {
          if ((*QI).second->GetIdentifier()->GetBits() == Bits &&
              (*QI).second->GetValueType() == Ty) {
            GLSTM.erase(Q);
          }
        } else {
          QI = GSTM.find(Q);
          if (QI != GSTM.end()) {
            if ((*QI).second->GetIdentifier()->GetBits() == Bits &&
                (*QI).second->GetValueType() == Ty) {
              GSTM.erase(Q);
            }
          }
        }
      }
    }
  }

  void EraseGateQubitParam(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    EraseGateQubitParam(Id->GetName(), Id->GetBits(), Id->GetSymbolType());
  }

  void LocalScope(const std::string& S, unsigned NumBits, ASTType Ty) {
    const ASTDeclarationContext* DC =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
    assert(DC && "Could not obtain the current Declaration Context!");

    switch (Ty) {
    case ASTTypeKernel: {
      std::stringstream M;
      M << "Extern functions cannot have Local Scope.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
      return;
    }
      break;
    case ASTTypeFunction: {
      std::stringstream M;
      M << "Functions cannot have Local Scope.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
      return;
    }
      break;
    case ASTTypeDefcal:
    case ASTTypeDefcalGroup: {
      std::stringstream M;
      M << "Defcals cannot have Local Scope.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
      return;
    }
      break;
    case ASTTypeCNotGate:
    case ASTTypeCCXGate:
    case ASTTypeCXGate:
    case ASTTypeHadamardGate:
    case ASTTypeUGate:
    case ASTTypeGate: {
      std::stringstream M;
      M << "Gates cannot have Local Scope.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
      return;
    }
      break;
    case ASTTypeQubit:
    case ASTTypeQubitContainer:
    case ASTTypeQubitContainerAlias: {
      map_iterator QI = QSTM.find(S);
      if (QI != QSTM.end()) {
        if ((*QI).second &&
            (*QI).second->GetIdentifier()->GetBits() == NumBits &&
            (*QI).second->GetValueType() == Ty) {
          (*QI).second->SetLocalScope(DC);
          LSTM.insert(std::make_pair(S, (*QI).second));
          QSTM.erase(S);
        }
      }
    }
      break;
    case ASTTypeAngle: {
      if (!ASTTypeSystemBuilder::Instance().IsReservedAngle(S)) {
        map_iterator AI = ASTM.find(S);
        if (AI != ASTM.end()) {
          if ((*AI).second &&
              (*AI).second->GetIdentifier()->GetBits() == NumBits &&
              (*AI).second->GetValueType() == Ty) {
            if (!ASTDeclarationContextTracker::Instance().IsGlobalContext(DC))
              (*AI).second->SetLocalScope(DC);
            else
              (*AI).second->SetLocalScope();
            LSTM.insert(std::make_pair(S, (*AI).second));
            ASTM.erase(S);
          }
        }
      }
    }
      break;
    default: {
      map_iterator LI = LSTM.find(S);
      if (LI != LSTM.end()) {
        if ((*LI).second &&
            (*LI).second->GetIdentifier()->GetBits() == NumBits &&
            (*LI).second->GetValueType() == Ty) {
            if (!ASTDeclarationContextTracker::Instance().IsGlobalContext(DC))
              (*LI).second->SetLocalScope(DC);
            else
              (*LI).second->SetLocalScope();
          return;
        }
      }

      map_iterator UI = USTM.find(S);
      if (UI != USTM.end()) {
        if ((*UI).second &&
            (*UI).second->GetIdentifier()->GetBits() == NumBits &&
            (*UI).second->GetValueType() == Ty) {
            if (!ASTDeclarationContextTracker::Instance().IsGlobalContext(DC))
              (*UI).second->SetLocalScope(DC);
            else
              (*UI).second->SetLocalScope();
          LSTM.insert(std::make_pair(S, (*UI).second));
          USTM.erase(S);
          return;
        }
      }

      map_iterator GI = GLSTM.find(S);
      if (GI != GLSTM.end()) {
        if ((*GI).second &&
            (*GI).second->GetIdentifier()->GetBits() == NumBits &&
            (*GI).second->GetValueType() == Ty) {
          if (!ASTDeclarationContextTracker::Instance().IsGlobalContext(DC))
            (*GI).second->SetLocalScope(DC);
          else
            (*GI).second->SetLocalScope();
          LSTM.insert(std::make_pair(S, (*GI).second));
          GLSTM.erase(S);
          return;
        }
      }

      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
      std::vector<multimap_iterator> IV;

      if (R.first != STM.end()) {
        for (multimap_iterator I = R.first; I != R.second; ++I) {
          if ((*I).second->GetIdentifier()->GetBits() == NumBits &&
              (*I).second->GetValueType() == Ty) {
            if (!ASTDeclarationContextTracker::Instance().IsGlobalContext(DC))
              (*I).second->SetLocalScope(DC);
            else
              (*I).second->SetLocalScope();
            LSTM.insert(std::make_pair(S, (*I).second));
            IV.push_back(I);
          }
        }

        for (std::vector<multimap_iterator>::iterator I = IV.begin();
             I != IV.end(); ++I)
          STM.erase(*I);
      }
    }
      break;
    }
  }

  void LocalScope(const ASTIdentifierNode* Id, unsigned NumBits, ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    LocalScope(Id->GetName(), NumBits, Ty);
  }

  void Erase(const ASTIdentifierNode* Id, ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    const std::string& S = Id->GetName();
    std::stringstream M;

    if (ASTTypeSystemBuilder::Instance().IsBuiltinFunction(S)) {
      M << "Builtin functions cannot be erased.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return;
    }

    switch (Ty) {
    case ASTTypeFunction:
      M << "Declared Functions cannot be erased.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return;
      break;
    case ASTTypeKernel:
      M << "Declared extern Functions cannot be erased.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return;
      break;
    case ASTTypeDefcal:
    case ASTTypeDefcalGroup:
      M << "Declared Defcals cannot be erased.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return;
      break;
    case ASTTypeCNotGate:
    case ASTTypeCCXGate:
    case ASTTypeCXGate:
    case ASTTypeHadamardGate:
    case ASTTypeUGate:
    case ASTTypeGate:
      M << "Declared Gates cannot be erased.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return;
      break;
    case ASTTypeQubit:
    case ASTTypeQubitContainer:
    case ASTTypeQubitContainerAlias:
      if (Id->IsBoundQubit()) {
        M << "Declared Qubits or Qubit Containers cannot be erased.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
        return;
      }
      break;
    default:
      break;
    }

    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(S) ||
        ASTTypeSystemBuilder::Instance().IsBuiltinFunction(S)) {
      return;
    }

    if (ASTAngleContextControl::Instance().InOpenContext() &&
        ASTTypeSystemBuilder::Instance().IsImplicitAngle(S)) {
      map_iterator I = ASTM.find(S);
      if (I != ASTM.end() && (*I).second &&
          (*I).second->GetValueType() == Ty) {
        ASTM.erase(S);
      }
    } else {
      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
      std::vector<multimap_iterator> IV;

      if (R.first != STM.end()) {
        for (multimap_iterator I = R.first; I != R.second; ++I) {
          if ((*I).second->GetValueType() == Ty && !(*I).second->IsDoNotDelete()) {
            IV.push_back(I);
          }
        }
      }

      if (!IV.empty()) {
        for (std::vector<multimap_iterator>::iterator I = IV.begin();
             I != IV.end(); ++I) {
          STM.erase(*I);
        }
      }
    }
  }

  void Erase(const std::string& S, unsigned Bits, ASTType Ty) {
    if (S.empty())
      return;

    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(S) ||
        ASTTypeSystemBuilder::Instance().IsBuiltinFunction(S)) {
      return;
    }

    if (ASTAngleContextControl::Instance().InOpenContext() &&
        ASTTypeSystemBuilder::Instance().IsImplicitAngle(S)) {
      map_iterator I = ASTM.find(S);
      if (I != ASTM.end() && (*I).second &&
          (*I).second->HasValue() && Ty == ASTTypeAngle &&
          (*I).second->GetValueType() == Ty) {
        ASTAngleNode* AN = (*I).second->GetValue()->GetValue<ASTAngleNode*>();
        if (AN && AN->GetBits() == Bits) {
          ASTM.erase(S);
        }
      }
    } else {
      if (Ty == ASTTypeQubit ||
          Ty == ASTTypeQubitContainer ||
          Ty == ASTTypeGateQubitParam ||
          Ty == ASTTypeQubitContainerAlias) {
        map_iterator QI = QSTM.find(S);
        if (QI != QSTM.end()) {
        }
      } else if (Ty == ASTTypeGate || Ty == ASTTypeDefcal ||
                 Ty == ASTTypeFunction || Ty == ASTTypeKernel) {
        std::stringstream M;
        M << "Gates, Defcals and/or Functions cannot be erased." << std::endl;
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                     DiagLevel::ICE);
        return;
      }

      std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
      std::vector<multimap_iterator> IV;

      if (R.first != STM.end()) {
        for (multimap_iterator I = R.first; I != R.second; ++I) {
          if ((*I).first == S &&
              (*I).second->GetIdentifier()->GetBits() == Bits &&
              (*I).second->GetValueType() == Ty) {
            IV.push_back(I);
          } else if ((*I).first == S &&
                     (*I).second->GetIdentifier()->GetBits() == Bits &&
                     (Ty == ASTTypeQubit || Ty == ASTTypeQubitContainer ||
                      Ty == ASTTypeGateQubitParam) &&
                     ((*I).second->GetValueType() == Ty ||
                      (*I).second->GetValueType() == ASTTypeUndefined)) {
            IV.push_back(I);
          }
        }
      }

      if (!IV.empty()) {
        for (std::vector<multimap_iterator>::iterator I = IV.begin();
             I != IV.end(); ++I) {
          STM.erase(*I);
        }
      }
    }
  }

  void Erase(const ASTIdentifierNode* Id, unsigned Bits, ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    Erase(Id->GetName(), Bits, Ty);
  }

  void EraseLocalSymbol(const ASTIdentifierNode* Id, unsigned Bits,
                        ASTType Ty) {
    map_iterator LI = LSTM.find(Id->GetName());
    if (LI != LSTM.end()) {
      if ((*LI).second && (*LI).second->GetValueType() == Ty &&
          (*LI).second->GetIdentifier()->GetBits() == Bits) {
        LSTM.erase(Id->GetName());
        return;
      } else {
        LSTM.erase(Id->GetName());
        return;
      }
    } else {
      map_iterator MI = GLSTM.find(Id->GetName());
      if (MI != GLSTM.end()) {
        return;
      }

      MI = QSTM.find(Id->GetName());
      if (MI != QSTM.end()) {
        return;
      }

      MI = USTM.find(Id->GetName());
      if (MI != USTM.end()) {
        return;
      }

      MI = GSTM.find(Id->GetName());
      if (MI != GSTM.end()) {
        return;
      }

      MI = FSTM.find(Id->GetName());
      if (MI != FSTM.end()) {
        return;
      }

      MI = DSTM.find(Id->GetName());
      if (MI != DSTM.end()) {
        return;
      }

      MI = GSTM.find(Id->GetName());
      if (MI != GSTM.end()) {
        return;
      }
    }
  }

  void EraseLocalSymbol(const std::string& S, unsigned Bits, ASTType Ty) {
    if (S.empty())
      return;

    map_iterator LI = LSTM.find(S);
    if (LI != LSTM.end()) {
      if ((*LI).second && (*LI).second->GetIdentifier()->GetBits() == Bits &&
          (*LI).second->GetValueType() == Ty) {
        LSTM.erase(LI);
      } else if (LI != LSTM.end()) {
        LSTM.erase(LI);
      }
    } else {
      map_iterator MI = GLSTM.find(S);
      if (MI != GLSTM.end()) {
        return;
      }

      MI = QSTM.find(S);
      if (MI != QSTM.end()) {
        return;
      }

      MI = USTM.find(S);
      if (MI != USTM.end()) {
        return;
      }

      MI = GSTM.find(S);
      if (MI != GSTM.end()) {
        return;
      }

      MI = FSTM.find(S);
      if (MI != FSTM.end()) {
        return;
      }

      MI = DSTM.find(S);
      if (MI != DSTM.end()) {
        return;
      }

      MI = GSTM.find(S);
      if (MI != GSTM.end()) {
        return;
      }
    }
  }

  void EraseLocalSymbol(const std::string& S) {
      LSTM.erase(S);
  }

  void EraseGlobalSymbol(const ASTIdentifierNode* Id, unsigned Bits,
                         ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    map_iterator GI = GLSTM.find(Id->GetName());
    if (GI != GLSTM.end() && (*GI).second) {
      const ASTIdentifierNode* SId = (*GI).second->GetIdentifier();
      if (SId->GetBits() == Bits && SId->GetSymbolType() == Ty) {
        GLSTM.erase(GI);
      }
    }
  }

  void EraseGlobalSymbol(const std::string& S) {
    map_iterator GI = GLSTM.find(S);
    if (GI != GLSTM.end()) {
      GLSTM.erase(GI);
    }
  }

  void EraseLocalAngle(const std::string& S, unsigned Bits,
                       ASTType Ty) {
    if (!S.empty()) {
      if (ASTTypeSystemBuilder::Instance().IsReservedAngle(S))
        return;

      map_iterator AI = ASTM.find(S);
      if (AI != ASTM.end()) {
        if ((*AI).second && (*AI).second->GetValueType() == Ty &&
            (*AI).second->GetIdentifier()->GetBits() == Bits) {
          std::stringstream AS;
          for (unsigned I = 0; I < 4; ++I) {
            AS.clear();
            AS.str("");
            AS << S.c_str() << '[' << I << ']';
            ASTM.erase(AS.str());
          }

          ASTM.erase(S);
        }
      } else {
        AI = LSTM.find(S);
        if (AI != LSTM.end() && (*AI).second &&
            (*AI).second->GetValueType() == Ty &&
            (*AI).second->GetIdentifier()->GetBits() == Bits) {
          std::stringstream AS;
          for (unsigned I = 0; I < 4; ++I) {
            AS.clear();
            AS.str("");
            AS << S.c_str() << '[' << I << ']';
            LSTM.erase(AS.str());
          }

          LSTM.erase(S);
        }
      }
    }
  }

  void EraseLocalAngle(const std::string& S) {
    map_iterator AI = LSTM.find(S);
    if (AI != LSTM.end()) {
      std::stringstream AS;
      for (unsigned I = 0; I < 4; ++I) {
        AS.clear();
        AS.str("");
        AS << S.c_str() << '[' << I << ']';
        LSTM.erase(AS.str());
      }

      LSTM.erase(S);
    }
  }

  void EraseLocalAngle(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    EraseLocalAngle(Id->GetName(), Id->GetBits(), Id->GetSymbolType());
  }

  void EraseLocal(const std::string& Id, unsigned Bits, ASTType Ty) {
    if (!Id.empty()) {
      map_iterator LI = LSTM.find(Id);
      if (LI != LSTM.end()) {
        if ((*LI).second->GetIdentifier()->GetBits() == Bits &&
            (*LI).second->GetIdentifier()->GetSymbolType() == Ty) {
          if (ASTExpressionValidator::Instance().IsAngleType(Ty))
            EraseLocalAngle(Id);
          else
            LSTM.erase(Id);
        }
      }
    }
  }

  void EraseLocal(const ASTIdentifierNode* Id, unsigned Bits, ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    EraseLocal(Id->GetName(), Bits, Ty);
  }

  void ClearLocal() {
    LSTM.clear();
  }

  void SwapQSTMSymbolTableEntry(const std::string& Id,
                                ASTSymbolTableEntry* STE) {
    assert(!Id.empty() && "Invalid Identifier argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    map_iterator MI = QSTM.find(Id);
    if (MI != QSTM.end()) {
      (*MI).second = STE;
    }
  }

  void SwapQSTMSymbolTableEntry(const ASTIdentifierNode* Id,
                                ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    SwapQSTMSymbolTableEntry(Id->GetName(), STE);
  }

  void SwapLSTMSymbolTableEntry(const std::string& Id,
                                ASTSymbolTableEntry* STE) {
    assert(!Id.empty() && "Invalid Identifier argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    map_iterator LI = LSTM.find(Id);
    if (LI != LSTM.end()) {
      (*LI).second = STE;
    }
  }

  void SwapLSTMSymbolTableEntry(const ASTIdentifierNode* Id,
                                ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    SwapLSTMSymbolTableEntry(Id->GetName(), STE);
  }

  void SwapGLSTMSymbolTableEntry(const std::string& Id,
                                 ASTSymbolTableEntry* STE) {
    assert(!Id.empty() && "Invalid Identifier argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    map_iterator GLI = GLSTM.find(Id);
    if (GLI != GLSTM.end()) {
      (*GLI).second = STE;
    }
  }

  void SwapGLSTMSymbolTableEntry(const ASTIdentifierNode* Id,
                                 ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    SwapGLSTMSymbolTableEntry(Id->GetName(), STE);
  }

  bool InsertToCalibrationTable(const std::string& Id,
                                ASTSymbolTableEntry* STE) {
    assert(!Id.empty() && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    const ASTIdentifierNode* IId = STE->GetIdentifier();

    if (!ASTCalContextBuilder::Instance().InOpenContext() &&
        !ASTFunctionContextBuilder::Instance().InOpenContext() &&
        !ASTKernelContextBuilder::Instance().InOpenContext() &&
        !ASTDefcalContextBuilder::Instance().InOpenContext()) {
      std::stringstream M;
      M << "OpenPulse symbols are only visible within an open "
        << "calibration context.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(IId), M.str(),
                                                      DiagLevel::Error);
      return false;
    }

    map_iterator CI = CSTM.find(Id);
    if (CI != CSTM.end()) {
      if (STE == (*CI).second)
        return true;

      std::stringstream M;
      M << "Symbol " << Id.c_str() << " already exists in the "
        << "CSTM.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(IId), M.str(),
                                                      DiagLevel::Error);
      return false;
    }

    if (!CSTM.insert(std::make_pair(Id, STE)).second) {
      std::stringstream M;
      M << "Failure inserting SymbolTable Entry into CSTM.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(IId), M.str(),
                                                      DiagLevel::ICE);
      return false;
    }

    ASTSymbolTable::Instance().EraseLocalSymbol(IId, IId->GetBits(),
                                                IId->GetSymbolType());
    return true;
  }

  bool InsertToCalibrationTable(const ASTIdentifierNode* Id,
                                ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");
    assert(STE->GetIdentifier() == Id &&
           "Inconsistent ASTSymbolTableEntry ASTIdentifierNode!");
    return InsertToCalibrationTable(Id->GetName(), STE);
  }

  void TransferLocal(std::map<std::string, ASTSymbolTableEntry*>& TSTM) {
    TSTM.clear();
    TSTM = LSTM;
  }

  void TransferLocalContextSymbols(const ASTDeclarationContext* CTX,
                                   std::map<std::string,
                                   const ASTSymbolTableEntry*>& MM) {
    assert(CTX && "Invalid ASTDeclarationContext argument!");

    std::vector<const ASTIdentifierNode*> DV;

    for (map_iterator LI = LSTM.begin(); LI != LSTM.end(); ++LI) {
      const ASTIdentifierNode* Id = (*LI).second->GetIdentifier();
      assert(Id && "Could not obtain a valid ASTIdentifierNode!");

      if (Id->GetDeclarationContext()->GetIndex() == CTX->GetIndex() &&
          (*LI).second->GetContext()->GetIndex() == CTX->GetIndex()) {
        if (!MM.insert(std::make_pair(Id->GetName(), (*LI).second)).second) {
          std::map<std::string, const ASTSymbolTableEntry*>::const_iterator MI =
            MM.find((*LI).first);
          if (MI != MM.end()) {
            if (!(*MI).second && (*MI).second == (*LI).second) {
              DV.push_back(Id);
              continue;
            } else if ((*MI).second && (*MI).second->GetIdentifier() == Id &&
                (*MI).second == (*LI).second) {
              DV.push_back(Id);
              continue;
            }
          } else {
            std::stringstream M;
            M << "Failure inserting Local Symbol to the external map.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
            return;
          }
        } else {
          DV.push_back(Id);
        }
      }
    }

    if (!DV.empty()) {
      for (std::vector<const ASTIdentifierNode*>::const_iterator VI = DV.begin();
           VI != DV.end(); ++VI) {
        EraseLocalSymbol((*VI), (*VI)->GetBits(), (*VI)->GetSymbolType());
      }
    }
  }

  void TransferQubit(const std::string& S, ASTType Ty) {
    if (S.empty())
      return;

    map_iterator MI = QSTM.find(S);
    if (MI != QSTM.end()) {
      if ((*MI).second->GetIdentifier()->GetName() == S &&
          (*MI).second->GetValueType() == Ty) {
        return;
      } else {
        std::stringstream M;
        M << "Qubit Identifier " << S << " exists in the Qubit Symbol "
          << "Table with a different Type.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                     DiagLevel::ICE);
      }
    }

    MI = LSTM.find(S);
    if (MI != LSTM.end()) {
      map_iterator QI = QSTM.find(S);
      if (QI == QSTM.end()) {
        if (!QSTM.insert(std::make_pair(S, (*MI).second)).second) {
          std::stringstream M;
          M << "Failure transferring Qubit " << S << " to the Qubit "
            << "Symbol Table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::ICE);
        }

        LSTM.erase(S);
        return;
      } else {
        if ((*QI).second->GetValueType() == Ty) {
          std::stringstream M;
          M << "Re-declaration of " << PrintTypeEnum(Ty)
            << " " << S << ".";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Error);
        }
      }
    }

    MI = GLSTM.find(S);
    if (MI != GLSTM.end()) {
      map_iterator QI = QSTM.find(S);
      if (QI == QSTM.end()) {
        if (!QSTM.insert(std::make_pair(S, (*MI).second)).second) {
          std::stringstream M;
          M << "Failure transferring Qubit " << S << " to the Qubit "
            << "Symbol Table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::ICE);
        }

        GLSTM.erase(S);
        return;
      } else {
        if ((*QI).second->GetValueType() == Ty) {
          std::stringstream M;
          M << "Re-declaration of " << PrintTypeEnum(Ty)
            << " " << S << ".";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Error);
        }
      }
    }

    std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
    std::vector<multimap_iterator> IV;

    if (R.first != STM.end()) {
      for (multimap_iterator I = R.first; I != R.second; ++I) {
        if ((*I).first == S && (*I).second &&
            ((*I).second->GetValueType() == Ty ||
             (*I).second->GetValueType() == ASTTypeUndefined)) {
          (*I).second->SetValueType(Ty == ASTTypeUndefined ?
                                          ASTTypeQubitContainer : Ty);
          if (!QSTM.insert(std::make_pair(S, (*I).second)).second) {
            continue;
          }

          (*I).second->SetGlobalScope();
          IV.push_back(I);
        }
      }
    }

    for (std::vector<multimap_iterator>::iterator I = IV.begin();
         I != IV.end(); ++I)
      STM.erase(*I);
  }

  void TransferQubit(const ASTIdentifierNode* Id, ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    TransferQubit(Id->GetName(), Ty);
  }

  void TransferQubit(const std::string& S, unsigned Bits, ASTType Ty) {
    if (S.empty())
      return;

    map_iterator MI = QSTM.find(S);
    if (MI != QSTM.end()) {
      if ((*MI).second->GetIdentifier()->GetName() == S &&
          (*MI).second->GetIdentifier()->GetBits() == Bits &&
          (*MI).second->GetValueType() == Ty) {
        return;
      } else {
        std::stringstream M;
        M << "Qubit Identifier " << S << " exists in the Qubit Symbol "
          << "Table with a different Type.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                     DiagLevel::ICE);
      }
    }

    MI = LSTM.find(S);
    if (MI != LSTM.end()) {
      map_iterator QI = QSTM.find(S);
      if (QI == QSTM.end()) {
        if (!QSTM.insert(std::make_pair(S, (*MI).second)).second) {
          std::stringstream M;
          M << "Failure transferring Qubit " << S << " to the Qubit "
            << "Symbol Table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::ICE);
        }

        LSTM.erase(S);
        return;
      } else {
        if ((*QI).second->GetValueType() == Ty &&
            (*QI).second->GetIdentifier()->GetBits() == Bits) {
          std::stringstream M;
          M << "Re-declaration of " << PrintTypeEnum(Ty)
            << " " << S << ".";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Error);
        } else if ((*QI).second->GetValueType() == Ty &&
                   (*QI).second->GetIdentifier()->GetBits() != Bits) {
          std::stringstream M;
          M << "Re-declaration of " << PrintTypeEnum(Ty)
            << " " << S << " with different bits.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Error);
        } else {
          std::stringstream M;
          M << "Re-declaration of " << S << " with a different type.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Error);
        }
      }
    }

    MI = GLSTM.find(S);
    if (MI != GLSTM.end()) {
      map_iterator QI = QSTM.find(S);
      if (QI == QSTM.end()) {
        if (!QSTM.insert(std::make_pair(S, (*MI).second)).second) {
          std::stringstream M;
          M << "Failure transferring Qubit " << S << " to the Qubit "
            << "Symbol Table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::ICE);
        }

        GLSTM.erase(S);
        return;
      } else {
        if ((*QI).second->GetValueType() == Ty &&
            (*QI).second->GetIdentifier()->GetBits() == Bits) {
          std::stringstream M;
          M << "Re-declaration of " << PrintTypeEnum(Ty)
            << " " << S << ".";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Error);
        } else if ((*QI).second->GetValueType() == Ty &&
                   (*QI).second->GetIdentifier()->GetBits() != Bits) {
          std::stringstream M;
          M << "Re-declaration of " << PrintTypeEnum(Ty)
            << " " << S << " with different bits.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Error);
        } else {
          std::stringstream M;
          M << "Re-declaration of " << S << " with a different type.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::Error);
        }
      }
    }

    std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
    std::vector<multimap_iterator> IV;

    if (R.first != STM.end()) {
      for (multimap_iterator I = R.first; I != R.second; ++I) {
        if (((*I).first == S) && (*I).second &&
            (*I).second->GetIdentifier()->GetBits() == Bits &&
            ((*I).second->GetValueType() == Ty ||
             (*I).second->GetValueType() == ASTTypeUndefined)) {
          (*I).second->SetValueType(Ty == ASTTypeUndefined ?
                                          ASTTypeQubitContainer : Ty);
          if (!QSTM.insert(std::make_pair(S, (*I).second)).second) {
            continue;
          }

          (*I).second->SetGlobalScope();
          IV.push_back(I);
        }
      }
    }

    for (std::vector<multimap_iterator>::iterator I = IV.begin();
         I != IV.end(); ++I)
      STM.erase(*I);
  }

  void TransferQubit(const ASTIdentifierNode* Id, unsigned Bits, ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    TransferQubit(Id->GetName(), Bits, Ty);
  }

  void TransferQubitToLSTM(const ASTIdentifierNode* Id, unsigned Bits,
                           ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    if (Id->GetName()[0] == '$') {
      std::stringstream M;
      M << "Physical Qubits have inherent global scope "
        << "and cannot be transfered to the local symbol table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
    }

    map_iterator LI = LSTM.find(Id->GetName());
    if (LI != LSTM.end()) {
      if (!(*LI).second) {
        std::stringstream M;
        M << "Invalid " << PrintTypeEnum(Ty) << " " << Id->GetName()
          << " without a SymbolTable Entry.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
      }

      if ((*LI).second->GetIdentifier()->GetBits() == Bits &&
          (*LI).second->GetValueType() == Ty) {
        map_iterator QI = QSTM.find(Id->GetName());
        if (QI != QSTM.end()) {
          if (!(*QI).second) {
            std::stringstream M;
            M << "Invalid " << PrintTypeEnum(Ty) << " " << Id->GetName()
              << " without a SymbolTable Entry.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                           DiagLevel::ICE);
          }

          QSTM.erase(QI);
        }

        return;
      } else {
        std::stringstream M;
        M << " A symbol " << Id->GetName() << " already exists "
          << "in the local symbol table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
      }
    }

    bool T = false;
    map_iterator QI = QSTM.find(Id->GetName());
    if (QI != QSTM.end()) {
      if (!(*QI).second) {
        std::stringstream M;
        M << "Invalid " << PrintTypeEnum(Ty) << " " << Id->GetName()
          << " without a SymbolTable Entry.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
      }

      if ((*QI).second->GetIdentifier()->GetBits() == Bits &&
          (*QI).second->GetValueType() == Ty) {
        if (!LSTM.insert(std::make_pair(Id->GetName(), (*QI).second)).second) {
          std::stringstream M;
          M << "Failure inserting Qubit " << Id->GetName()
            << " into the local symbol table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
        }

        QSTM.erase(QI);
        T = true;
      }
    }

    if (!T) {
      std::pair<multimap_iterator, multimap_iterator> R =
        STM.equal_range(Id->GetName());

      if (R.first != STM.end()) {
        for (multimap_iterator I = R.first; I != R.second; ++I) {
          if ((*I).first == Id->GetName() && (*I).second &&
              (*I).second->GetValueType() == Ty &&
              (*I).second->GetIdentifier()->GetBits() == Bits) {
            if (!LSTM.insert(std::make_pair(Id->GetName(), (*I).second)).second) {
              std::stringstream M;
              M << "Failure inserting Qubit " << Id->GetName()
                << " into the local symbol table.";
              QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                             DiagLevel::ICE);
            }

            STM.erase(I);
            break;
          }
        }
      }
    }
  }

  void TransferQubitToLSTM(const std::string& Id, unsigned Bits,
                           ASTType Ty) {
    assert(!Id.empty() && "Invalid ASTIdentifierNode argument!");

    if (Id[0] == '$') {
      std::stringstream M;
      M << "Physical Qubits have inherent global scope "
        << "and cannot be transfered to the local symbol table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::ICE);
    }

    map_iterator LI = LSTM.find(Id);
    if (LI != LSTM.end()) {
      if (!(*LI).second) {
        std::stringstream M;
        M << "Invalid " << PrintTypeEnum(Ty) << " " << Id
          << " without a SymbolTable Entry.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::ICE);
      }

      if ((*LI).second->GetIdentifier()->GetBits() == Bits &&
          (*LI).second->GetValueType() == Ty) {
        map_iterator QI = QSTM.find(Id);
        if (QI != QSTM.end()) {
          if (!(*QI).second) {
            std::stringstream M;
            M << "Invalid " << PrintTypeEnum(Ty) << " " << Id
              << " without a SymbolTable Entry.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                         DiagLevel::ICE);
          }

          QSTM.erase(QI);
        }

        return;
      } else {
        std::stringstream M;
        M << " A symbol " << Id << " already exists "
          << "in the local symbol table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                     DiagLevel::Error);
      }
    }

    bool T = false;
    map_iterator QI = QSTM.find(Id);
    if (QI != QSTM.end()) {
      if (!(*QI).second) {
        std::stringstream M;
        M << "Invalid " << PrintTypeEnum(Ty) << " " << Id
          << " without a SymbolTable Entry.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                     DiagLevel::ICE);
      }

      if ((*QI).second->GetIdentifier()->GetBits() == Bits &&
          (*QI).second->GetValueType() == Ty) {
        if (!LSTM.insert(std::make_pair(Id, (*QI).second)).second) {
          std::stringstream M;
          M << "Failure inserting Qubit " << Id
            << " into the local symbol table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                       DiagLevel::ICE);
        }

        QSTM.erase(QI);
        T = true;
      }
    }

    if (!T) {
      std::pair<multimap_iterator, multimap_iterator> R =
        STM.equal_range(Id);

      if (R.first != STM.end()) {
        for (multimap_iterator I = R.first; I != R.second; ++I) {
          if ((*I).first == Id && (*I).second &&
              (*I).second->GetValueType() == Ty &&
              (*I).second->GetIdentifier()->GetBits() == Bits) {
            if (!LSTM.insert(std::make_pair(Id, (*I).second)).second) {
              std::stringstream M;
              M << "Failure inserting Qubit " << Id
                << " into the local symbol table.";
              QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                           DiagLevel::ICE);
            }

            STM.erase(I);
            break;
          }
        }
      }
    }
  }

  void TransferAngleToLSTM(const ASTIdentifierNode* Id, unsigned Bits,
                           ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    if (ASTStringUtils::Instance().IsIndexedQubit(Id->GetName())) {
      if (ASTStringUtils::Instance().IsBoundQubit(
          ASTStringUtils::Instance().GetBaseQubitName(Id->GetName())))
        return;
    } else {
      if (ASTStringUtils::Instance().IsBoundQubit(Id->GetName()))
        return;
    }

    map_iterator LI = LSTM.find(Id->GetName());
    if (LI != LSTM.end()) {
      if (!(*LI).second) {
        std::stringstream M;
        M << "Invalid angle " << Id->GetName() << " without a "
          << "SymbolTable Entry.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
      }

      if ((*LI).second->GetIdentifier()->GetBits() == Bits &&
          (*LI).second->GetValueType() == Ty) {
        return;
      } else {
        std::stringstream M;
        M << " A symbol " << Id->GetName() << " already exists "
          << "in the local symbol table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
      }
    }

    bool T = false;
    map_iterator AI = ASTM.find(Id->GetName());
    if (AI != ASTM.end()) {
      if (!(*AI).second) {
        std::stringstream M;
        M << "Invalid angle " << Id->GetName() << " without a "
          << "SymbolTable Entry.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
      }

      if ((*AI).second->GetIdentifier()->GetBits() == Bits &&
          (*AI).second->GetValueType() == Ty) {
        if (!LSTM.insert(std::make_pair(Id->GetName(), (*AI).second)).second) {
          std::stringstream M;
          M << "Failure inserting Angle " << Id->GetName()
            << " into the local symbol table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
        }

        ASTM.erase(Id->GetName());
        T = true;
        std::stringstream AS;

        for (unsigned I = 0; I < 4U; ++I) {
          AS.str("");
          AS.clear();
          AS << Id->GetName() << '[' << I << ']';
          const std::string& ANS = AS.str();
          AI = ASTM.find(ANS);
          if (AI != ASTM.end()) {
            if (!LSTM.insert(std::make_pair(ANS, (*AI).second)).second) {
              std::stringstream M;
              M << "Failure inserting Angle " << ANS << " into the local "
                << "symbol table.";
              QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                             DiagLevel::ICE);
            }

            ASTM.erase(ANS);
          }
        }
      }
    }

    std::stringstream AAS;
    for (unsigned I = 0; I < 4U; ++I) {
      AAS.str("");
      AAS.clear();
      AAS << Id->GetName() << '[' << I << ']';
      const std::string& ANS = AAS.str();
      AI = ASTM.find(ANS);
      if (AI != ASTM.end()) {
        if (!LSTM.insert(std::make_pair(ANS, (*AI).second)).second) {
          std::stringstream M;
          M << "Failure inserting Angle " << ANS << " into the local "
            << "symbol table.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
        }

        ASTM.erase(ANS);
      }
    }

    if (!T) {
      AI = GLSTM.find(Id->GetName());
      if (AI != GLSTM.end()) {
        if (!(*AI).second) {
          std::stringstream M;
          M << "Invalid angle " << Id->GetName() << " without a "
            << "SymbolTable Entry.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::Error);
        }

        if ((*AI).second->GetIdentifier()->GetBits() == Bits &&
            (*AI).second->GetValueType() == Ty) {

          if (!LSTM.insert(std::make_pair(Id->GetName(), (*AI).second)).second) {
            std::stringstream M;
            M << "Failure inserting Angle " << Id->GetName()
              << " into the local symbol table.";
            QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                           DiagLevel::ICE);
          }

          GLSTM.erase(AI);
          T = true;
          std::stringstream AS;

          for (unsigned I = 0; I < 4U; ++I) {
            AS.str("");
            AS.clear();
            AS << Id->GetName() << '[' << I << ']';
            const std::string& ANS = AS.str();
            AI = GLSTM.find(ANS);
            if (AI != GLSTM.end()) {
              if (!LSTM.insert(std::make_pair(ANS, (*AI).second)).second) {
                std::stringstream M;
                M << "Failure inserting Angle " << ANS << " into the local "
                  << "symbol table.";
                QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                  DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                               DiagLevel::ICE);
              }

              GLSTM.erase(AI);
            }
          }
        }
      }
    }

    if (!T) {
      std::pair<multimap_iterator, multimap_iterator> R =
        STM.equal_range(Id->GetName());

      if (R.first != STM.end()) {
        for (multimap_iterator I = R.first; I != R.second; ++I) {
          if ((*I).first == Id->GetName() && (*I).second &&
              (*I).second->GetValueType() == Ty &&
              (*I).second->GetIdentifier()->GetBits() == Bits) {
            if (!LSTM.insert(std::make_pair(Id->GetName(), (*I).second)).second) {
              std::stringstream M;
              M << "Failure inserting Angle " << Id->GetName()
                << " into the local symbol table.";
              QasmDiagnosticEmitter::Instance().EmitDiagnostic(
                DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                             DiagLevel::ICE);
            }

            STM.erase(I);
            break;
          }
        }
      }
    }
  }

  void TransferGate(const std::string& S, ASTType Ty) {
    if (S.empty())
      return;

    map_iterator GI = GSTM.find(S);
    std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
    std::vector<multimap_iterator> IV;

    if (R.first != STM.end()) {
      for (multimap_iterator I = R.first; I != R.second; ++I) {
        if ((*I).first == S && (*I).second &&
            (*I).second != (*GI).second &&
            ((*I).second->GetValueType() == Ty ||
             (*I).second->GetValueType() == ASTTypeUndefined)) {
          if (GI != GSTM.end() && (*GI).second &&
              (*GI).second->GetValue() != (*I).second->GetValue()) {
            (*I).second->SetValueType(Ty);
            (*I).second->SetGlobalScope();
            (*GI).second->SetValue((*I).second->GetValue(), Ty);
          } else if (GI != GSTM.end() && !(*GI).second) {
            (*I).second->SetValueType(Ty);
            (*I).second->SetGlobalScope();
            (*I).second->GetIdentifier()->SetSymbolTableEntry((*I).second);
            (*GI).second = (*I).second;
          } else if (GI == GSTM.end()) {
            if (!GSTM.insert(std::make_pair(S, (*I).second)).second) {
              continue;
            }
          }

          IV.push_back(I);
        }
      }
    }

    for (std::vector<multimap_iterator>::iterator I = IV.begin();
         I != IV.end(); ++I)
      STM.erase(*I);
  }

  void TransferGate(const ASTIdentifierNode* Id, ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    TransferGate(Id->GetName(), Ty);
  }

  void TransferGate(const std::string& S, unsigned Bits, ASTType Ty) {
    if (S.empty())
      return;

    map_iterator GI = GSTM.find(S);
    std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
    std::vector<multimap_iterator> IV;

    if (R.first != STM.end()) {
      for (multimap_iterator I = R.first; I != R.second; ++I) {
        if ((*I).first == S && (*I).second &&
            (*I).second != (*GI).second &&
            (*I).second->GetIdentifier()->GetBits() == Bits &&
            ((*I).second->GetValueType() == Ty ||
             (*I).second->GetValueType() == ASTTypeUndefined)) {
          if (GI != GSTM.end() && (*GI).second &&
              (*GI).second->GetValue() != (*I).second->GetValue()) {
            (*I).second->SetValueType(Ty);
            (*I).second->SetGlobalScope();
            (*GI).second->SetValue((*I).second->GetValue(), Ty);
          } else if (GI != GSTM.end() && !(*GI).second) {
            (*I).second->SetValueType(Ty);
            (*I).second->SetGlobalScope();
            (*I).second->GetIdentifier()->SetSymbolTableEntry((*I).second);
            (*GI).second = (*I).second;
          } else if (GI == GSTM.end()) {
            if (!GSTM.insert(std::make_pair(S, (*I).second)).second) {
              continue;
            }
          }

          IV.push_back(I);
        }
      }
    }

    for (std::vector<multimap_iterator>::iterator I = IV.begin();
         I != IV.end(); ++I)
      STM.erase(*I);
  }

  void TransferGate(const ASTIdentifierNode* Id, unsigned Bits, ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    TransferGate(Id->GetName(), Bits, Ty);
  }

  void TransferDefcal(const std::string& S, ASTType Ty) {
    if (S.empty())
      return;

    map_iterator DI = DSTM.find(S);
    std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
    std::vector<multimap_iterator> IV;

    if (R.first != STM.end()) {
      for (multimap_iterator I = R.first; I != R.second; ++I) {
        if ((*I).first == S && (*I).second &&
            (*I).second != (*DI).second &&
            ((*I).second->GetValueType() == Ty ||
             (*I).second->GetValueType() == ASTTypeUndefined)) {
          if (DI != DSTM.end() && (*DI).second &&
              (*DI).second->GetValue() != (*I).second->GetValue()) {
            (*I).second->SetValueType(Ty);
            (*I).second->SetGlobalScope();
            (*DI).second->SetValue((*I).second->GetValue(), Ty);
          } else if (DI != DSTM.end() && !(*DI).second) {
            (*I).second->SetValueType(Ty);
            (*I).second->SetGlobalScope();
            (*I).second->GetIdentifier()->SetSymbolTableEntry((*I).second);
            (*DI).second = (*I).second;
          } else if (DI == DSTM.end()) {
            if (!DSTM.insert(std::make_pair(S, (*I).second)).second) {
              continue;
            }
          }

          IV.push_back(I);
        }
      }
    }

    for (std::vector<multimap_iterator>::iterator I = IV.begin();
         I != IV.end(); ++I)
      STM.erase(*I);
  }

  void TransferDefcal(const ASTIdentifierNode* Id, ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    TransferDefcal(Id->GetName(), Ty);
  }

  void TransferDefcal(const std::string& S, unsigned Bits, ASTType Ty) {
    if (S.empty())
      return;

    map_iterator DI = DSTM.find(S);
    std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
    std::vector<multimap_iterator> IV;

    if (R.first != STM.end()) {
      for (multimap_iterator I = R.first; I != R.second; ++I) {
        if ((*I).first == S && (*I).second &&
            (*I).second != (*DI).second &&
            (*I).second->GetIdentifier()->GetBits() == Bits &&
            ((*I).second->GetValueType() == Ty ||
             (*I).second->GetValueType() == ASTTypeUndefined)) {
          if (DI != DSTM.end() && (*DI).second &&
              (*DI).second->GetValue() != (*I).second->GetValue()) {
            (*I).second->SetValueType(Ty);
            (*I).second->SetGlobalScope();
            (*DI).second->SetValue((*I).second->GetValue(), Ty);
          } else if (DI != DSTM.end() && !(*DI).second) {
            (*I).second->SetValueType(Ty);
            (*I).second->SetGlobalScope();
            (*I).second->GetIdentifier()->SetSymbolTableEntry((*I).second);
            (*DI).second = (*I).second;
          } else if (DI == DSTM.end()) {
            if (!DSTM.insert(std::make_pair(S, (*I).second)).second) {
              continue;
            }
          }

          IV.push_back(I);
        }
      }
    }

    for (std::vector<multimap_iterator>::iterator I = IV.begin();
         I != IV.end(); ++I)
      STM.erase(*I);
  }

  void TransferDefcal(const ASTIdentifierNode* Id, unsigned Bits, ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    TransferDefcal(Id->GetName(), Bits, Ty);
  }

  void TransferAngle(const std::string& S, ASTType Ty) {
    if (S.empty())
      return;

    std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
    std::vector<multimap_iterator> IV;

    if (R.first != STM.end()) {
      for (multimap_iterator I = R.first; I != R.second; ++I) {
        if ((*I).first == S && (*I).second &&
            ((*I).second->GetValueType() == Ty ||
             (*I).second->GetValueType() == ASTTypeUndefined)) {
          (*I).second->SetValueType(ASTTypeAngle);
          if (!ASTM.insert(std::make_pair(S, (*I).second)).second) {
            continue;
          }

          IV.push_back(I);
        }
      }
    }

    for (std::vector<multimap_iterator>::iterator I = IV.begin();
         I != IV.end(); ++I)
      STM.erase(*I);
  }

  void TransferAngle(const ASTIdentifierNode* Id, ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    TransferAngle(Id->GetName(), Ty);
  }

  void TransferAngle(const std::string& S, unsigned Bits, ASTType Ty) {
    if (S.empty())
      return;

    std::pair<multimap_iterator, multimap_iterator> R = STM.equal_range(S);
    std::vector<multimap_iterator> IV;

    if (R.first != STM.end()) {
      for (multimap_iterator I = R.first; I != R.second; ++I) {
        if ((*I).first == S && (*I).second &&
            (*I).second->GetIdentifier()->GetBits() == Bits &&
            ((*I).second->GetValueType() == Ty ||
             (*I).second->GetValueType() == ASTTypeUndefined)) {
          (*I).second->SetValueType(ASTTypeAngle);
          if (!ASTM.insert(std::make_pair(S, (*I).second)).second) {
            continue;
          }

          IV.push_back(I);
        }
      }
    }

    for (std::vector<multimap_iterator>::iterator I = IV.begin();
         I != IV.end(); ++I)
      STM.erase(*I);
  }

  void TransferAngle(const ASTIdentifierNode* Id, unsigned Bits, ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    TransferAngle(Id->GetName(), Bits, Ty);
  }

  bool TransferUndefinedSymbol(const ASTIdentifierNode* Id, unsigned Bits,
                               ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    const ASTDeclarationContext* DCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
    (void) DCX; // Quiet for now; will be used for scoping later.

    // Certain specific types are handled in a special way here.
    switch (Id->GetSymbolType()) {
    case ASTTypeUndefined:
    case ASTTypeAngle:
    case ASTTypeDefcal:
    case ASTTypeDefcalGroup:
    case ASTTypeGate:
    case ASTTypeHadamardGate:
    case ASTTypeCXGate:
    case ASTTypeCCXGate:
    case ASTTypeCNotGate:
    case ASTTypeUGate:
    case ASTTypeGateQubitParam:
    case ASTTypeKernel:
    case ASTTypeFunction:
    case ASTTypeFunctionDeclaration:
      break;
    default:
      break;
    }

    if (Ty == ASTTypeUndefined) {
      std::stringstream M;
      M << "Transfer of undefined type to undefined type is meaningless.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (Id->GetSymbolType() != ASTTypeUndefined && Id->GetSymbolType() != Ty) {
      ASTSymbolTableEntry* STE =
        ASTSymbolTable::Instance().Lookup(Id->GetName(), Id->GetBits(),
                                          Id->GetSymbolType());
      if (STE && STE->GetIdentifier()->GetSymbolType() != Ty &&
          STE->GetIdentifier()->GetBits() != Bits) {
        std::stringstream M;
        M << "Symbol " << Id->GetName() << " already exists in the "
          << "SymbolTable with a different type ("
          << PrintTypeEnum(Id->GetSymbolType()) << ").";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
        return false;
      }
    }

    STMapIndex MIX = XNONE;
    map_iterator UI;

    if (ASTCalContextBuilder::Instance().InOpenContext() ||
        ASTDefcalContextBuilder::Instance().InOpenContext()) {
      UI = CSTM.find(Id->GetName());
      if (UI != CSTM.end()) {
        MIX = XCSTM;
        goto Found;
      }
    }

    UI = USTM.find(Id->GetName());
    if (UI != USTM.end()) {
      MIX = XUSTM;
      goto Found;
    }

    UI = LSTM.find(Id->GetName());
    if (UI != LSTM.end()) {
      MIX = XLSTM;
      goto Found;
    }

    UI = GLSTM.find(Id->GetName());
    if (UI != GLSTM.end()) {
      MIX = XGLSTM;
      goto Found;
    }

    if (ASTStringUtils::Instance().IsBoundQubit(Id->GetName())) {
      UI = QSTM.find(Id->GetName());
      if (UI != QSTM.end()) {
        MIX = XQSTM;
        goto Found;
      }
    }

    if (Id->GetSymbolType() == ASTTypeQubit ||
        Id->GetSymbolType() == ASTTypeQubitContainer ||
        Id->GetSymbolType() == ASTTypeQubitContainerAlias) {
      UI = QSTM.find(Id->GetName());
      if (UI != QSTM.end()) {
        MIX = XQSTM;
        goto Found;
      }
    }

    if (ASTTypeSystemBuilder::Instance().IsReservedAngle(Id->GetName())) {
      UI = ASTM.find(Id->GetName());
      if (UI != ASTM.end()) {
        MIX = XASTM;
        goto Found;
      }
    }

    if (ASTTypeSystemBuilder::Instance().IsImplicitAngle(Id->GetName())) {
      UI = ASTM.find(Id->GetName());
      if (UI != ASTM.end()) {
        MIX = XASTM;
        goto Found;
      }
    }

    UI = DSTM.find(Id->GetName());
    if (UI != DSTM.end()) {
      MIX = XDSTM;
      goto Found;
    }

    UI = GSTM.find(Id->GetName());
    if (UI != GSTM.end()) {
      MIX = XGSTM;
      goto Found;
    }

    UI = FSTM.find(Id->GetName());
    if (UI != FSTM.end()) {
      MIX = XFSTM;
      goto Found;
    }

    {
      std::stringstream M;
      M << "Symbol " << Id->GetName() << " was not found.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
      return false;
    }

Found:
    if (!(*UI).second) {
      std::stringstream M;
      M << "Identifier " << Id->GetName() << " does not have a "
        << "Symbol Table Entry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if ((*UI).second->GetIdentifier() != Id) {
      std::stringstream M;
      M << "ASTIdentifierNode mismatch with the SymbolTable Entry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (Id->IsReference()) {
      ASTIdentifierRefNode* IdR = const_cast<ASTIdentifierRefNode*>(
                                  dynamic_cast<const ASTIdentifierRefNode*>(Id));
      assert(IdR && "Could not dynamic_cast to an ASTIdentifierRefNode!");
      IdR->SetSymbolType(Ty);
      IdR->SetBits(Bits);
    } else {
      const_cast<ASTIdentifierNode*>(Id)->SetSymbolType(Ty);
      const_cast<ASTIdentifierNode*>(Id)->SetBits(Bits);
    }

    assert((*UI).second->GetValueType() == Ty &&
           "Inconsistent Type of SymbolTable Entry with Identifier!");
    assert((*UI).second->GetIdentifier()->GetBits() == Bits &&
           "Inconsistent Bits of SymbolTable Entry with Identifier!");

    if (Ty == ASTTypeGate || Ty == ASTTypeHadamardGate ||
        Ty == ASTTypeCXGate || Ty == ASTTypeCCXGate ||
        Ty == ASTTypeCNotGate || Ty == ASTTypeUGate) {
      map_iterator GI = GSTM.find(Id->GetName());
      if (GI != GSTM.end()) {
        if ((*UI).second == (*GI).second &&
            (*UI).second->GetValueType() == (*GI).second->GetValueType() &&
            (*GI).second->GetValueType() == Ty &&
            (*UI).second->GetIdentifier() == (*GI).second->GetIdentifier())
          return true;

        std::stringstream M;
        M << "A Gate with identifier " << Id->GetName() << " already "
          << " exists with different properties.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
        return false;
      }

      if (!GSTM.insert(std::make_pair((*UI).first, (*UI).second)).second) {
        std::stringstream M;
        M << "Insertion into the Gate Symbol Table failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      GI = GSTM.find(Id->GetName());
      const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
      (*GI).second->SetGlobalScope();
      EraseFromMap(UI, MIX);
      return true;
    } else if (Ty == ASTTypeQubit ||
               Ty == ASTTypeQubitContainer ||
               Ty == ASTTypeQubitContainerAlias) {
      map_iterator QI = QSTM.find(Id->GetName());
      if (QI != QSTM.end()) {
        if ((*UI).second == (*QI).second &&
            (*UI).second->GetValueType() == (*QI).second->GetValueType() &&
            (*QI).second->GetValueType() == Ty &&
            (*UI).second->GetIdentifier() == (*QI).second->GetIdentifier())
          return true;

        std::stringstream M;
        const char* QT;

        switch (Ty) {
        case ASTTypeQubit:
          QT = "Qubit";
          break;
        case ASTTypeQubitContainer:
          QT = "QubitContainer";
          break;
        default:
          QT = "QubitContainer alias ";
          break;
        }

        M << "A " << QT << " with identifier " << Id->GetName()
          << " already exists with different properties.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
        return false;
      }

      if (!QSTM.insert(std::make_pair((*UI).first, (*UI).second)).second) {
        std::stringstream M;
        M << "Insertion into the Qubit Symbol Table failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      if ((*UI).first[0] == '$') {
        const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
        (*UI).second->SetGlobalScope();
      }

      EraseFromMap(UI, MIX);
      return true;
    } else if (Ty == ASTTypeFunction || Ty == ASTTypeKernel) {
      map_iterator FI = FSTM.find(Id->GetName());
      if (FI != FSTM.end()) {
        if ((*UI).second == (*FI).second &&
            (*UI).second->GetValueType() == (*FI).second->GetValueType() &&
            (*FI).second->GetValueType() == Ty &&
            (*UI).second->GetIdentifier() == (*FI).second->GetIdentifier())
          return true;

        std::stringstream M;
        M << "A Function with identifier " << Id->GetName() << " already "
          << " exists with different properties.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
        return false;
      }

      if (!FSTM.insert(std::make_pair((*UI).first, (*UI).second)).second) {
        std::stringstream M;
        M << "Insertion into the Function Symbol Table failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
      (*UI).second->SetGlobalScope();
      EraseFromMap(UI, MIX);
      return true;
    } else if (Ty == ASTTypeDefcal) {
      map_iterator DI = DSTM.find(Id->GetName());
      if (DI != DSTM.end()) {
        if ((*UI).second == (*DI).second &&
            (*UI).second->GetValueType() == (*DI).second->GetValueType() &&
            (*DI).second->GetValueType() == Ty &&
            (*UI).second->GetIdentifier() == (*DI).second->GetIdentifier())
          return true;

        std::stringstream M;
        M << "A Defcal with identifier " << Id->GetName() << " already "
          << "exists with different properties.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
        return false;
      }

      if (!DSTM.insert(std::make_pair((*UI).first, (*UI).second)).second) {
        std::stringstream M;
        M << "Insertion into the Defcal Symbol Table failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
      (*UI).second->SetGlobalScope();
      EraseFromMap(UI, MIX);
      return true;
    } else if (Ty == ASTTypeAngle &&
               ASTDeclarationContextTracker::Instance().CurrentContextIsGlobal()) {
      map_iterator AI = ASTM.find(Id->GetName());
      if (AI != ASTM.end()) {
        if ((*UI).second == (*AI).second &&
            (*UI).second->GetValueType() == (*AI).second->GetValueType() &&
            (*AI).second->GetValueType() == Ty &&
            (*UI).second->GetIdentifier() == (*AI).second->GetIdentifier())
          return true;

        std::stringstream M;
        M << "An Angle with identifier " << Id->GetName()
          << " already exists at global scope.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      if (!ASTM.insert(std::make_pair((*UI).first, (*UI).second)).second) {
        std::stringstream M;
        M << "Insertion into the Angle Symbol Table failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
      (*UI).second->SetGlobalScope();
      EraseFromMap(UI, MIX);
      return true;
    } else {
      if (ASTCalContextBuilder::Instance().InOpenContext()) {
        map_iterator CI = CSTM.find(Id->GetName());
        if (CI != CSTM.end()) {
          if ((*UI).second == (*CI).second &&
              (*UI).second->GetValueType() == (*CI).second->GetValueType() &&
              (*CI).second->GetValueType() == Ty &&
              (*UI).second->GetIdentifier() == (*CI).second->GetIdentifier())
            return true;

          std::stringstream M;
          M << "A declaration with identifier " << Id->GetName()
            << " already exists at calibration scope.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::Error);
          return false;
        }

        if (!CSTM.insert(std::make_pair((*UI).first, (*UI).second)).second) {
          std::stringstream M;
          M << "Insertion into the Calibration Symbol Table failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        const_cast<ASTIdentifierNode*>(Id)->SetGlobalScope();
        (*UI).second->SetGlobalScope();
        EraseFromMap(UI, MIX);
        return true;
      }

      if (Id->InGlobalContext()) {
        map_iterator GLI = GLSTM.find(Id->GetName());
        if (GLI != GLSTM.end()) {
          if ((*GLI).second == (*UI).second &&
              (*GLI).second->GetValueType() == (*UI).second->GetValueType() &&
              (*GLI).second->GetValueType() == Ty)
            return true;

          std::stringstream M;
          M << "A declaration with identifier " << Id->GetName()
            << " already exists at global scope.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::Error);
          return false;
        }

        if (!GLSTM.insert(std::make_pair((*UI).first, (*UI).second)).second) {
          std::stringstream M;
          M << "Insertion into the Global Symbol Table failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        EraseFromMap(UI, MIX);
        return true;
      } else {
        map_iterator LI = LSTM.find(Id->GetName());
        if (LI != LSTM.end()) {
          if ((*LI).second == (*UI).second &&
              (*LI).second->GetValueType() == (*UI).second->GetValueType() &&
              (*LI).second->GetValueType() == Ty)
            return true;

          std::stringstream M;
          M << "Declaration with identifier " << Id->GetName()
            << " shadows a previous declaration.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        if (!LSTM.insert(std::make_pair((*UI).first, (*UI).second)).second) {
          std::stringstream M;
          M << "Insertion into the Local Symbol Table failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                         DiagLevel::ICE);
          return false;
        }

        EraseFromMap(UI, MIX);
        return true;
      }
    }

    return false;
  }

  bool TransferLocalUndefinedSymbol(const ASTIdentifierNode* Id, unsigned Bits,
                                    ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    const ASTDeclarationContext* DCX =
      ASTDeclarationContextTracker::Instance().GetCurrentContext();
    (void) DCX; // Quiet for now; will be used for scoping later.

    switch (Id->GetSymbolType()) {
    case ASTTypeQubit:
    case ASTTypeQubitContainer:
    case ASTTypeQubitContainerAlias:
      break;
    case ASTTypeGate:
    case ASTTypeHadamardGate:
    case ASTTypeCXGate:
    case ASTTypeCCXGate:
    case ASTTypeCNotGate:
    case ASTTypeUGate:
    case ASTTypeFunction:
    case ASTTypeDefcal:
    case ASTTypeDefcalGroup:
    case ASTTypeOpenPulseCalibration: {
      std::stringstream M;
      M << "A symbol of type " << PrintTypeEnum(Id->GetSymbolType())
        << " cannot be transferred to LSTM.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
      return false;
    }
      break;
    default:
      break;
    }

    if (Ty == ASTTypeUndefined) {
      std::stringstream M;
      M << "Transfer of undefined type to undefined type is meaningless.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::Error);
      return false;
    }

    if (Id->GetSymbolType() != ASTTypeUndefined && Id->GetSymbolType() != Ty) {
      ASTSymbolTableEntry* STE =
        ASTSymbolTable::Instance().Lookup(Id->GetName(), Id->GetBits(),
                                          Id->GetSymbolType());
      if (STE && STE->GetIdentifier()->GetSymbolType() != Ty &&
          STE->GetIdentifier()->GetBits() != Bits) {
        std::stringstream M;
        M << "Symbol " << Id->GetName() << " already exists in the "
          << "SymbolTable with a different type ("
          << PrintTypeEnum(Id->GetSymbolType()) << ").";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::Error);
        return false;
      }
    }

    STMapIndex MIX = XNONE;
    map_iterator UI;

    UI = LSTM.find(Id->GetName());
    if (UI != LSTM.end())
      return true;

    UI = USTM.find(Id->GetName());
    if (UI != USTM.end()) {
      MIX = XUSTM;
      goto Found;
    }

    UI = GLSTM.find(Id->GetName());
    if (UI != GLSTM.end()) {
      MIX = XGLSTM;
      goto Found;
    }

    if (Ty == ASTTypeGateQubitParam) {
      UI = GSTM.find(Id->GetName());
      if (UI != GLSTM.end()) {
        MIX = XGSTM;
        goto Found;
      }

      UI = QSTM.find(Id->GetName());
      if (UI != QSTM.end()) {
        MIX = XQSTM;
        goto Found;
      }
    }

    if (MIX == XNONE) {
      return true;
    }

Found:
    if (!(*UI).second) {
      std::stringstream M;
      M << "Identifier " << Id->GetName() << " does not have a "
        << "Symbol Table Entry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if ((*UI).second->GetIdentifier() != Id) {
      std::stringstream M;
      M << "ASTIdentifierNode mismatch with the SymbolTable Entry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (Id->IsReference()) {
      ASTIdentifierRefNode* IdR = const_cast<ASTIdentifierRefNode*>(
                                  dynamic_cast<const ASTIdentifierRefNode*>(Id));
      assert(IdR && "Could not dynamic_cast to an ASTIdentifierRefNode!");
      IdR->SetSymbolType(Ty);
      IdR->SetBits(Bits);
    } else {
      const_cast<ASTIdentifierNode*>(Id)->SetSymbolType(Ty);
      const_cast<ASTIdentifierNode*>(Id)->SetBits(Bits);
    }

    assert((*UI).second->GetValueType() == Ty &&
           "Inconsistent Type of SymbolTable Entry with Identifier!");
    assert((*UI).second->GetIdentifier()->GetBits() == Bits &&
           "Inconsistent Bits of SymbolTable Entry with Identifier!");

    if (!LSTM.insert(std::make_pair((*UI).first, (*UI).second)).second) {
      std::stringstream M;
      M << "Insertion into the Local Symbol Table failed.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    EraseFromMap(UI, MIX);
    return true;
  }

  bool TransferGlobalUndefinedSymbol(const ASTIdentifierNode* Id, unsigned Bits,
                                     ASTType Ty) {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    if (Ty == ASTTypeUndefined) {
      std::stringstream M;
      M << "Transfer of undefined type to undefined type is meaningless.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    map_iterator UI = USTM.find(Id->GetName());
    if (UI == USTM.end()) {
      std::stringstream M;
      M << "Symbol " << Id->GetName() << " was not found in the temp "
        << "Symbol Table.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (!(*UI).second) {
      std::stringstream M;
      M << "Identifier " << Id->GetName() << " does not have a "
        << "Symbol Table Entry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if ((*UI).second->GetIdentifier() != Id) {
      std::stringstream M;
      M << "ASTIdentifierNode mismatch with the SymbolTable Entry.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (Id->IsReference()) {
      ASTIdentifierRefNode* IdR = const_cast<ASTIdentifierRefNode*>(
                                  dynamic_cast<const ASTIdentifierRefNode*>(Id));
      assert(IdR && "Could not dynamic_cast to an ASTIdentifierRefNode!");
      IdR->SetSymbolType(Ty);
      IdR->SetBits(Bits);
    } else {
      const_cast<ASTIdentifierNode*>(Id)->SetSymbolType(Ty);
      const_cast<ASTIdentifierNode*>(Id)->SetBits(Bits);
    }

    assert((*UI).second->GetValueType() == Ty &&
           "Inconsistent Type of SymbolTable Entry with Identifier!");
    assert((*UI).second->GetIdentifier()->GetBits() == Bits &&
           "Inconsistent Bits of SymbolTable Entry with Identifier!");

    map_iterator GLI = GLSTM.find(Id->GetName());
    if (GLI != GLSTM.end()) {
      std::stringstream M;
      M << "A declaration with identifier " << Id->GetName()
        << " already exists at global scope.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    if (!GLSTM.insert(std::make_pair((*UI).first, (*UI).second)).second) {
      std::stringstream M;
      M << "Insertion into the Global Symbol Table failed.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                     DiagLevel::ICE);
      return false;
    }

    USTM.erase(UI);
    return true;
  }

  bool TransferGateQubitParam(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    map_iterator GI = GLSTM.find(Id->GetName());
    if (GI != GLSTM.end()) {
      map_iterator LI = LSTM.find(Id->GetName());
      if (LI != LSTM.end()) {
        if ((*LI).second == Id->GetSymbolTableEntry())
          return true;

        std::stringstream M;
        M << "Gate Qubit Parameter " << Id->GetName() << " already "
          << "exists in the Local Symbol Table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      if (!LSTM.insert(std::make_pair((*GI).first, (*GI).second)).second) {
        std::stringstream M;
        M << "Transfer of Gate Qubit Parameter " << Id->GetName()
          << " to the Local Symbol Table failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
                                                       DiagLevel::ICE);
        return false;
      }

      GLSTM.erase(Id->GetName());
      return true;
    }

    GI = GSTM.find(Id->GetName());
    if (GI != GSTM.end()) {
      map_iterator LI = LSTM.find(Id->GetName());
      if (LI != LSTM.end()) {
        if ((*LI).second == Id->GetSymbolTableEntry())
          return true;

        std::stringstream M;
        M << "Gate Qubit Parameter " << Id->GetName() << " already "
          << "exists in the Local Symbol Table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(),
          DiagLevel::ICE);
        return false;
      }

      if (!LSTM.insert(std::make_pair((*GI).first, (*GI).second)).second) {
        std::stringstream M;
        M << "Transfer of Gate Qubit Parameter " << Id->GetName()
          << " to the Local Symbol Table failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
        return false;
      }

      return true;
    }

    GI = QSTM.find(Id->GetName());
    if (GI != QSTM.end()) {
      if (ASTStringUtils::Instance().IsIndexedQubit(Id->GetName())) {
        if (ASTStringUtils::Instance().IsBoundQubit(
            ASTStringUtils::Instance().GetBaseQubitName(Id->GetName())))
          return true;
      } else {
        if (ASTStringUtils::Instance().IsBoundQubit(Id->GetName()))
          return true;
      }

      map_iterator LI = LSTM.find(Id->GetName());
      if (LI != LSTM.end()) {
        if ((*LI).second == Id->GetSymbolTableEntry())
          return true;

        std::stringstream M;
        M << "Gate Qubit Parameter " << Id->GetName() << " already "
          << "exists in the Local Symbol Table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
        return false;
      }

      if (!LSTM.insert(std::make_pair((*GI).first, (*GI).second)).second) {
        std::stringstream M;
        M << "Transfer of Gate Qubit Parameter " << Id->GetName()
          << " to the Local Symbol Table failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
        return false;
      }

      QSTM.erase(Id->GetName());
      return true;
    }

    GI = USTM.find(Id->GetName());
    if (GI != USTM.end()) {
      if (ASTStringUtils::Instance().IsIndexedQubit(Id->GetName())) {
        if (ASTStringUtils::Instance().IsBoundQubit(
            ASTStringUtils::Instance().GetBaseQubitName(Id->GetName())))
          return true;
      } else {
        if (ASTStringUtils::Instance().IsBoundQubit(Id->GetName()))
          return true;
      }

      map_iterator LI = LSTM.find(Id->GetName());
      if (LI != LSTM.end()) {
        if ((*LI).second == Id->GetSymbolTableEntry())
          return true;

        std::stringstream M;
        M << "Gate Qubit Parameter " << Id->GetName() << " already "
          << "exists in the Local Symbol Table.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
        return false;
      }

      if (!LSTM.insert(std::make_pair((*GI).first, (*GI).second)).second) {
        std::stringstream M;
        M << "Transfer of Gate Qubit Parameter " << Id->GetName()
          << " to the Local Symbol Table failed.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
        return false;
      }

      USTM.erase(Id->GetName());
      return true;
    }

    GSTM.erase(Id->GetName());
    return true;
  }

  bool TransferGlobalSymbolToLocal(const ASTIdentifierNode* Id,
                                   const ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    map_iterator GI = GLSTM.find(Id->GetName());
    if (GI != GLSTM.end()) {
      map_iterator LI = LSTM.find(Id->GetName());
      if (LI == LSTM.end()) {
        if (!LSTM.insert(std::make_pair(Id->GetName(),
                                        const_cast<ASTSymbolTableEntry*>(STE))).second) {
          std::stringstream M;
          M << "Insertion of symbol " << Id->GetName() << " to the Local "
            << "Symbol Table failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
          return false;
        }

        GLSTM.erase(GI);
        return true;
      } else {
        LSTM[Id->GetName()] = const_cast<ASTSymbolTableEntry*>(STE);
        GLSTM.erase(GI);
        return true;
      }
    }

    return true;
  }

  bool TransferLocalSymbolToGlobal(const ASTIdentifierNode* Id,
                                   const ASTSymbolTableEntry* STE) {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    map_iterator LI = LSTM.find(Id->GetName());
    if (LI != LSTM.end()) {
      map_iterator GI = GLSTM.find(Id->GetName());
      if (GI == GLSTM.end()) {
        if (!GLSTM.insert(std::make_pair(Id->GetName(),
                                         const_cast<ASTSymbolTableEntry*>(STE))).second) {
          std::stringstream M;
          M << "Insertion of symbol " << Id->GetName() << " to the Global "
            << "Symbol Table failed.";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(Id), M.str(), DiagLevel::ICE);
          return false;
        }

        LSTM.erase(LI);
        return true;
      } else {
        GLSTM[Id->GetName()] = const_cast<ASTSymbolTableEntry*>(STE);
        LSTM.erase(LI);
        return true;
      }
    }

    return true;
  }

  void EraseGateQubitParam(const std::string& Id) {
    assert(!Id.empty() && "Invalid ASTIdentifierNode argument!");

    map_iterator GI = GSTM.find(Id);
    if (GI != GSTM.end())
      GSTM.erase(GI);
  }

  void EraseAngle(const std::string& Id) {
    assert(!Id.empty() && "Invalid ASTIdentifierNode argument!");

    map_iterator AI = ASTM.find(Id);
    if (AI != ASTM.end()) {
      std::stringstream AS;

      for (unsigned I = 0; I < 4; ++I) {
        AS.str("");
        AS.clear();
        AS << Id << '[' << I << ']';
        ASTM.erase(AS.str());
      }

      ASTM.erase(Id);
    }
  }

  const ASTSymbolTableEntry* FindGate(const std::string& S) const {
    if (!S.empty()) {
      map_iterator GI = GSTM.find(S);
      return GI == GSTM.end() ? nullptr : (*GI).second;
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindGate(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return FindGate(Id->GetName());
  }

  // FIXME: WORK-IN-PROGRESS HERE:
  // Implemented in ASTSymbolTable.cpp.
  ASTMapSymbolTableEntry* CreateDefcalGroup(const std::string& DN);

  ASTMapSymbolTableEntry* FindDefcalGroup(const std::string& S) {
    assert(!S.empty() && "Invalid defcal group identifier argument!");

    map_iterator DMI = DSTM.find(S);

    if (DMI == DSTM.end())
      return nullptr;

    ASTMapSymbolTableEntry* MSTE =
      dynamic_cast<ASTMapSymbolTableEntry*>((*DMI).second);
    return MSTE && MSTE->HasMap() &&
           MSTE->GetValueType() == ASTTypeDefcalGroup ? MSTE : nullptr;
  }

  const ASTMapSymbolTableEntry* FindDefcalGroup(const std::string& S) const {
    assert(!S.empty() && "Invalid defcal group identifier argument!");

    map_const_iterator DMI = DSTM.find(S);

    if (DMI == DSTM.end())
      return nullptr;

    const ASTMapSymbolTableEntry* MSTE =
      dynamic_cast<const ASTMapSymbolTableEntry*>((*DMI).second);
    return MSTE && MSTE->HasMap() &&
           MSTE->GetValueType() == ASTTypeDefcalGroup ? MSTE : nullptr;
  }

  const ASTSymbolTableEntry* FindDefcal(const std::string& S) const {
    assert(!S.empty() && "Invalid defcal identifier argument!");

    if (ASTStringUtils::Instance().IsMangled(S)) {
      std::string DN = ASTStringUtils::Instance().GetDefcalBaseName(S);
      if (!DN.empty()) {
        if (const ASTMapSymbolTableEntry* MSTE = FindDefcalGroup(DN))
          return MSTE->Find(std::hash<std::string>{}(S));
      }
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindDefcal(const std::string& GS,
                                        const std::string& MS) const {
    assert(!GS.empty() && "Invalid defcal group identifier argument!");
    assert(!MS.empty() && "Invalid defcal identifier argument!");

    if (ASTStringUtils::Instance().IsMangled(MS)) {
      if (const ASTMapSymbolTableEntry* MSTE = FindDefcalGroup(GS)) {
        return MSTE->Find(std::hash<std::string>{}(MS));
      }
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindDefcal(const std::string& GS,
                                        uint64_t H) const {
    assert(!GS.empty() && "Invalid defcal group identifier argument!");

    if (const ASTMapSymbolTableEntry* MSTE = FindDefcalGroup(GS)) {
      return MSTE->Find(H);
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindDefcal(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid defcal ASTIdentifierNode argument!");

    if (!Id->GetMangledName().empty()) {
      if (const ASTMapSymbolTableEntry* MSTE = FindDefcalGroup(Id->GetName())) {
        return MSTE->Find(Id->GetMHash());
      }
    }

    return nullptr;
  }

  bool InsertDefinedDefcal(const std::string& DG, uint64_t H,
                           ASTSymbolTableEntry* STE) {
    assert(!DG.empty() && "Invalid defcal group identifier argument!");
    assert(STE && "Invalid defcal ASTSymbolTableEntry!");

    ASTMapSymbolTableEntry* MSTE = FindDefcalGroup(DG);

    if (!MSTE) {
      MSTE = CreateDefcalGroup(DG);
      assert(MSTE && "Could not create a valid defcal group!");
    }

    return MSTE->GetMap().insert(std::make_pair(H, STE)).second;
  }

  bool InsertDefinedDefcal(ASTMapSymbolTableEntry* MSTE,
                           uint64_t H,
                           ASTSymbolTableEntry* STE) {
    assert(MSTE && "Invalid ASTMapSymbolTableEntry argument!");
    assert(STE && "Invalid ASTSymbolTableEntry argument!");

    if (MSTE->GetIdentifier()->GetName() !=
        STE->GetIdentifier()->GetDefcalGroupName()) {
      std::stringstream M;
      M << "Invalid mismatch between defcal group names.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(STE->GetIdentifier()),
                                                M.str(), DiagLevel::Error);
        return false;
    }

    return MSTE->GetMap().insert(std::make_pair(H, STE)).second;
  }

  void EraseUndefinedDefcal(const std::string& Id) {
    assert(!Id.empty() && "Invalid defcal identifier argument!");

    map_iterator DI = USTM.find(Id);
    if (DI != USTM.end()) {
      if ((*DI).second->GetIdentifier()->GetName() == Id &&
          (*DI).second->GetIdentifier()->GetSymbolType() == ASTTypeDefcal) {
        USTM.erase(DI);
      }
    }
  }

  void EraseUndefinedSymbol(const std::string& Id) {
    assert(!Id.empty() && "Invalid defcal identifier argument!");

    map_iterator DI = USTM.find(Id);
    if (DI != USTM.end()) {
      USTM.erase(Id);
    }
  }

  void ClearUndefinedSymbolTable() {
    USTM.clear();
  }

  ASTSymbolTableEntry* FindUndefinedDefcal(const std::string& Id) {
    assert(!Id.empty() && "Invalid defcal ASTIdentifierNode argument!");

    map_iterator DI = USTM.find(Id);
    return DI == USTM.end() ? nullptr : (*DI).second;
  }

  ASTSymbolTableEntry* FindUndefinedDefcal(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid defcal ASTIdentifierNode argument!");

    return FindUndefinedDefcal(Id->GetName());
  }

  ASTSymbolTableEntry* FindCalibrationBlock(const std::string& CS) {
    if (!CS.empty() && ASTCalContextBuilder::Instance().InOpenContext()) {
      map_iterator CI = CSTM.find(CS);
      if (CI != CSTM.end())
        return (*CI).second;
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindCalibrationSymbol(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    if (ASTCalContextBuilder::Instance().InOpenContext() ||
        ASTDefcalContextBuilder::Instance().InOpenContext()) {
      map_iterator CI = CSTM.find(Id->GetName());
      if (CI != CSTM.end()) {
        if ((*CI).second && (*CI).second->GetIdentifier() == Id &&
            (*CI).second->GetIdentifier()->GetBits() == Id->GetBits() &&
            (*CI).second->GetValueType() == Id->GetSymbolType())
          return (*CI).second;
      }
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindCalibrationSymbol(const std::string& CS,
                                                   unsigned Bits,
                                                   ASTType Ty) const {
    assert(!CS.empty() && "Invalid std::string argument!");

    if (ASTCalContextBuilder::Instance().InOpenContext() ||
        ASTDefcalContextBuilder::Instance().InOpenContext()) {
      map_iterator CI = CSTM.find(CS);
      if (CI != CSTM.end()) {
        if ((*CI).second && (*CI).second->GetIdentifier()->GetName() == CS &&
            (*CI).second->GetIdentifier()->GetBits() == Bits &&
            (*CI).second->GetValueType() == Ty &&
            (*CI).second->GetIdentifier()->GetSymbolType() == Ty)
          return (*CI).second;
      }
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindFunction(const std::string& S) const {
    if (!S.empty()) {
      map_iterator FI = FSTM.find(S);
      return FI == FSTM.end() ? nullptr : (*FI).second;
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindFunction(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return FindFunction(Id->GetName());
  }

  const ASTSymbolTableEntry* FindQubit(const std::string& S) const {
    if (!S.empty()) {
      map_iterator QI;
      if (ASTStringUtils::Instance().IsIndexed(S)) {
        std::string QS =
          ASTStringUtils::Instance().IndexedIdentifierToQCElement(S);
        QI = QSTM.find(QS);
      } else {
        QI = QSTM.find(S);
      }

      return QI == QSTM.end() ? nullptr : (*QI).second;
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindQubit(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return FindQubit(Id->GetName());
  }

  const ASTSymbolTableEntry* FindQubit(const ASTIdentifierRefNode* IdR) const {
    assert(IdR && "Invalid ASTIdentifierRefNode argument!");
    return FindQubit(IdR->GetName());
  }

  const ASTSymbolTableEntry* FindAngle(const std::string& S) const {
    if (!S.empty()) {
      map_iterator AI = ASTM.find(S);
      return AI == ASTM.end() ? nullptr : (*AI).second;
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindAngle(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return FindAngle(Id->GetName());
  }

  const ASTSymbolTableEntry* FindAngle(const ASTIdentifierRefNode* IdR) const {
    assert(IdR && "Invalid ASTIdentifierRefNode argument!");
    return FindAngle(IdR->GetName());
  }

  const ASTSymbolTableEntry* FindGlobal(const std::string& S) const {
    if (!S.empty()) {
      map_iterator GI = GLSTM.find(S);
      return GI == GLSTM.end() ? nullptr : (*GI).second;
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindGlobal(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return FindGlobal(Id->GetName());
  }

  const ASTSymbolTableEntry* FindGlobal(const ASTIdentifierRefNode* IdR) const {
    assert(IdR && "Invalid ASTIdentifierRefNode argument!");
    return FindGlobal(IdR->GetName());
  }

  const ASTSymbolTableEntry* FindGlobalSymbol(const std::string& S,
                                              unsigned Bits, ASTType Ty) const {
    map_iterator GI = GLSTM.find(S);
    if (GI != GLSTM.end()) {
      if ((*GI).second && (*GI).second->GetValueType() == Ty &&
          (*GI).second->GetIdentifier()->GetSymbolType() == Ty &&
          (*GI).second->GetIdentifier()->GetBits() == Bits)
        return (*GI).second;
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindGlobalSymbol(const std::string& S,
                                              ASTType Ty) const {
    map_iterator GI = GLSTM.find(S);
    if (GI != GLSTM.end()) {
      if ((*GI).second && (*GI).second->GetValueType() == Ty &&
          (*GI).second->GetIdentifier()->GetSymbolType() == Ty)
        return (*GI).second;
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindGlobalSymbol(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return FindGlobalSymbol(Id->GetName(), Id->GetBits(), Id->GetSymbolType());
  }

  const ASTSymbolTableEntry* FindLocal(const std::string& S) const {
    if (!S.empty()) {
      map_iterator LI = LSTM.find(S);
      return LI == LSTM.end() ? nullptr : (*LI).second;
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindLocal(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return FindLocal(Id->GetName());
  }

  const ASTSymbolTableEntry* FindLocal(const ASTIdentifierRefNode* IdR) const {
    assert(IdR && "Invalid ASTIdentifierRefNode argument!");
    return FindLocal(IdR->GetName());
  }

  const ASTSymbolTableEntry* FindLocalSymbol(const std::string& S,
                                             unsigned Bits, ASTType Ty) const {
    map_iterator LI = LSTM.find(S);
    if (LI != LSTM.end()) {
      if ((*LI).second && (*LI).second->GetValueType() == Ty &&
          (*LI).second->GetIdentifier()->GetSymbolType() == Ty &&
          (*LI).second->GetIdentifier()->GetBits() == Bits)
        return (*LI).second;
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindLocalSymbol(const std::string& S,
                                             ASTType Ty) const {
    map_iterator LI = LSTM.find(S);
    if (LI != LSTM.end()) {
      if ((*LI).second && (*LI).second->GetValueType() == Ty &&
          (*LI).second->GetIdentifier()->GetSymbolType() == Ty)
        return (*LI).second;
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindLocalSymbol(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return FindLocalSymbol(Id->GetName(), Id->GetBits(), Id->GetSymbolType());
  }

  const ASTSymbolTableEntry* FindOpenPulseSymbol(const std::string& Id) const {
    if (!ASTCalContextBuilder::Instance().InOpenContext() &&
        !ASTDefcalContextBuilder::Instance().InOpenContext())
      return nullptr;

    assert(!Id.empty() && "Invalid Identifier argument!");

    map_const_iterator CI = CSTM.find(Id);
    return CI == CSTM.end() ? nullptr : (*CI).second;
  }

  const ASTSymbolTableEntry* FindOpenPulseSymbol(const std::string& Id,
                                                 ASTType Ty) const {
    if (!ASTCalContextBuilder::Instance().InOpenContext() &&
        !ASTDefcalContextBuilder::Instance().InOpenContext())
      return nullptr;

    assert(!Id.empty() && "Invalid Identifier argument!");

    map_const_iterator CI = CSTM.find(Id);
    if (CI != CSTM.end()) {
      if ((*CI).second->GetIdentifier()->GetSymbolType() == Ty &&
          (*CI).second->GetValueType() == Ty)
      return (*CI).second;
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindOpenPulseSymbol(const std::string& Id,
                                                 unsigned Bits,
                                                 ASTType Ty) const {
    if (!ASTCalContextBuilder::Instance().InOpenContext() &&
        !ASTDefcalContextBuilder::Instance().InOpenContext())
      return nullptr;

    assert(!Id.empty() && "Invalid Identifier argument!");

    map_const_iterator CI = CSTM.find(Id);
    if (CI != CSTM.end()) {
      if ((*CI).second->GetIdentifier()->GetSymbolType() == Ty &&
          (*CI).second->GetValueType() == Ty &&
          (*CI).second->GetIdentifier()->GetBits() == Bits)
      return (*CI).second;
    }

    return nullptr;
  }

  ASTSymbolTableEntry* FindOpenPulseSymbol(const std::string& Id) {
    if (!ASTCalContextBuilder::Instance().InOpenContext() &&
        !ASTDefcalContextBuilder::Instance().InOpenContext())
      return nullptr;

    assert(!Id.empty() && "Invalid Identifier argument!");

    map_iterator CI = CSTM.find(Id);
    return CI == CSTM.end() ? nullptr : (*CI).second;
  }

  ASTSymbolTableEntry* FindOpenPulseSymbol(const std::string& Id,
                                           ASTType Ty) {
    if (!ASTCalContextBuilder::Instance().InOpenContext() &&
        !ASTDefcalContextBuilder::Instance().InOpenContext())
      return nullptr;

    assert(!Id.empty() && "Invalid Identifier argument!");

    map_iterator CI = CSTM.find(Id);
    if (CI != CSTM.end()) {
      if ((*CI).second->GetIdentifier()->GetSymbolType() == Ty &&
          (*CI).second->GetValueType() == Ty)
      return (*CI).second;
    }

    return nullptr;
  }

  ASTSymbolTableEntry* FindOpenPulseSymbol(const std::string& Id,
                                           unsigned Bits,
                                           ASTType Ty) {
    if (!ASTCalContextBuilder::Instance().InOpenContext() &&
        !ASTDefcalContextBuilder::Instance().InOpenContext())
      return nullptr;

    assert(!Id.empty() && "Invalid Identifier argument!");

    map_iterator CI = CSTM.find(Id);
    if (CI != CSTM.end()) {
      if ((*CI).second->GetIdentifier()->GetSymbolType() == Ty &&
          (*CI).second->GetValueType() == Ty &&
          (*CI).second->GetIdentifier()->GetBits() == Bits)
      return (*CI).second;
    }

    return nullptr;
  }

  ASTSymbolTableEntry* FindOpenPulseSymbol(const ASTIdentifierNode* Id) {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    if (!ASTCalContextBuilder::Instance().InOpenContext() &&
        !ASTDefcalContextBuilder::Instance().InOpenContext())
      return nullptr;

    map_iterator CI = CSTM.find(Id->GetName());
    if (CI != CSTM.end()) {
      if ((*CI).second->GetIdentifier()->GetSymbolType() == Id->GetSymbolType() &&
          (*CI).second->GetIdentifier()->GetBits() == Id->GetBits())
        return (*CI).second;
    }

    return nullptr;
  }

  const ASTSymbolTableEntry* FindOpenPulseSymbol(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    if (!ASTCalContextBuilder::Instance().InOpenContext() &&
        !ASTDefcalContextBuilder::Instance().InOpenContext())
      return nullptr;

    map_const_iterator CI = CSTM.find(Id->GetName());
    if (CI != CSTM.end()) {
      if ((*CI).second->GetIdentifier()->GetSymbolType() == Id->GetSymbolType() &&
          (*CI).second->GetIdentifier()->GetBits() == Id->GetBits())
        return (*CI).second;
    }

    return nullptr;
  }


  void Release();
};

} // namespace QASM

#endif // __QASM_AST_SYMBOL_TABLE_H

