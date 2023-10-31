/* -*- coding: utf-8 -*-
 *
 * Copyright 2022, 2023 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_BASE_H
#define __QASM_AST_BASE_H

#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cassert>

namespace QASM {

class ASTLocation {
public:
  uint32_t LineNo;
  uint32_t ColNo;

public:
  ASTLocation()
  : LineNo(DIAGLineCounter::Instance().GetLineCount()),
  ColNo(DIAGLineCounter::Instance().GetColCount()) { }

  ASTLocation(const ASTLocation& RHS)
  : LineNo(RHS.LineNo), ColNo(RHS.ColNo) { }

  ASTLocation(uint32_t L, uint32_t C)
  : LineNo(L), ColNo(C) { }

  ~ASTLocation() { }

  ASTLocation& operator=(const ASTLocation& RHS) {
    if (this != &RHS) {
      LineNo = RHS.LineNo;
      ColNo = RHS.ColNo;
    }

    return *this;
  }

  inline bool operator<(const ASTLocation& RHS) const {
    if (LineNo < RHS.LineNo)
      return true;
    else if (LineNo == RHS.LineNo)
      return ColNo < RHS.ColNo;

    return false;
  }

  inline uint32_t GetLineNo() const {
    return LineNo;
  }

  inline uint32_t GetColNo() const {
    return ColNo;
  }
};

enum ASTTokenType : unsigned {
  TTUndefined = 0,
  TTInt,
  TTString
};

class ASTToken {
  friend class Parser;
  friend class QasmParser;
  friend class QasmScanner;

protected:
  int32_t TI;
  std::string TS;
  ASTTokenType TTy;
  ASTLocation Loc;
  uint32_t IX;

private:
  ASTToken() = delete;

public:
  ASTToken(int32_t TT, uint32_t XI)
  : TI(TT), TS(), TTy(TTInt), Loc(), IX(XI) { }

  ASTToken(int32_t TT, const char* S, uint32_t XI)
  : TI(TT), TS(S), TTy(TTString), Loc(), IX(XI) { }

  ASTToken(int32_t TT, const std::string& S, uint32_t XI)
  : TI(TT), TS(S), TTy(TTString), Loc(), IX(XI) { }

  ASTToken(const ASTToken& RHS)
  : TI(RHS.TI), TS(RHS.TS), TTy(RHS.TTy), Loc(RHS.Loc),
  IX(RHS.IX) { }

  ~ASTToken() = default;

  ASTToken& operator=(const ASTToken& RHS) {
    if (this != &RHS) {
      TI = RHS.TI;
      TS = RHS.TS;
      TTy = RHS.TTy;
      Loc = RHS.Loc;
      IX = RHS.IX;
    }

    return *this;
  }

  void SetTokenType(ASTTokenType T) {
    TTy = T;
  }

  void SetToken(int32_t I, ASTTokenType TT) {
    TI = I;
    TTy = TT;
  }

  void SetToken(const char* S, ASTTokenType TT) {
    TS = S;
    TTy = TT;
  }

  void SetToken(const std::string& S, ASTTokenType TT) {
    TS = S;
    TTy = TT;
  }

  void SetLocation(const ASTLocation& L) {
    Loc = L;
  }

  void SetLocation(uint32_t L, uint32_t C) {
    Loc.LineNo = L;
    Loc.ColNo = C;
  }

  ASTTokenType GetTokenType() const {
    return TTy;
  }

  int64_t GetInt() const {
    return TI;
  }

  const std::string& GetString() const {
    return TS;
  }

  const char* GetConstChar() const {
    return TS.data();
  }

  const std::string* GetStringAddress() const {
    return &TS;
  }

  uint32_t GetIndex() const {
    return IX;
  }

  const ASTLocation& GetLocation() const {
    return Loc;
  }
};

class ASTTokenFactory {
protected:
  static uint32_t TIX;
  static std::map<uint32_t, ASTToken*> TFM;

protected:
  ASTTokenFactory() = default;

public:
  ~ASTTokenFactory() = default;

  static void Clear() {
    for (std::map<uint32_t, ASTToken*>::reverse_iterator I = TFM.rbegin();
         I != TFM.rend(); ++I)
      delete (*I).second;

    TFM.clear();
    TIX = 1U;
  }

  static ASTToken* Create(int64_t I) {
    return new ASTToken(I, ASTTokenFactory::TIX++);
  }

  static ASTToken* Create(const char* S, int32_t TT) {
    assert(S && "Invalid Token String!");
    return new ASTToken(TT, S, ASTTokenFactory::TIX++);
  }

  static ASTToken* Create(const std::string& S, int32_t TT) {
    assert(!S.empty() && "Invalid Token String!");
    return new ASTToken(TT, S, ASTTokenFactory::TIX++);
  }

  static void Register(ASTToken* T, uint32_t IX) {
    assert(T && "Invalid ASTToken argument!");
    TFM.insert(std::make_pair(IX, T));
  }

  static const ASTToken* GetToken(uint32_t IX) {
    std::map<uint32_t, ASTToken*>::const_iterator MI = TFM.find(IX);
    return MI == TFM.end() ? nullptr : (*MI).second;
  }

  static const ASTToken* GetLastToken() {
    std::map<uint32_t, ASTToken*>::const_reverse_iterator MRI = TFM.rbegin();
    return MRI == TFM.rend() ? nullptr : (*MRI).second;
  }

  static const ASTToken* GetPreviousToken() {
    std::map<uint32_t, ASTToken*>::const_reverse_iterator MRI = TFM.rbegin();
    if (MRI == TFM.rend())
      return nullptr;

    --MRI;
    return MRI == TFM.rend() ? nullptr : (*MRI).second;
  }

  static uint32_t GetCurrentIndex() {
    return TIX;
  }
};

class ASTBase {
private:
  ASTLocation Loc;
  bool Registered;

public:
  ASTBase();

  ASTBase(const ASTBase& RHS);

  ASTBase& operator=(const ASTBase& RHS) {
    if (this != &RHS) {
      Loc.LineNo = RHS.Loc.LineNo;
      Loc.ColNo = RHS.Loc.ColNo;
      Registered = RHS.Registered;
    }

    return *this;
  }

  virtual ~ASTBase() = default;

  virtual ASTType GetASTType() const = 0;

  virtual void print() const = 0;
  virtual void push(ASTBase* Node) = 0;

  virtual void SetLocation(uint32_t L, uint32_t C) {
    Loc.LineNo = L;
    Loc.ColNo = C;
  }

  virtual void SetLocation(const ASTLocation& LOC) {
    Loc.LineNo = LOC.LineNo;
    Loc.ColNo = LOC.ColNo;
  }

  virtual const ASTLocation& GetLocation() const {
    return Loc;
  }

  virtual uint32_t GetLineNo() const {
    return Loc.LineNo;
  }

  virtual uint32_t GetColNo() const {
    return Loc.ColNo;
  }

  virtual bool IsRegistered() const {
    return Registered;
  }
};

} // namespace QASM

#endif // __QASM_AST_BASE_H

