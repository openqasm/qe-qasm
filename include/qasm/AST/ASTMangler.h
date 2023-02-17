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

#ifndef __QASM_AST_MANGLER_H
#define __QASM_AST_MANGLER_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTExpressionEvaluator.h>
#include <qasm/AST/ASTExpressionValidator.h>
#include <qasm/AST/ASTObjectTracker.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <map>
#include <tuple>
#include <type_traits>
#include <memory>
#include <regex>
#include <cassert>

namespace QASM {

namespace Mangler {

struct MToken {
  const char* T;
  unsigned L;

  MToken() : T(nullptr), L(0U) { }

  MToken(const char* TS, unsigned TL)
    : T(TS), L(TL) { }

  MToken(const MToken& RHS)
    : T(RHS.T), L(RHS.L) { }

  MToken& operator=(const MToken& RHS) {
    if (this != &RHS) {
      T = RHS.T;
      L = RHS.L;
    }

    return *this;
  }

  inline const char* Token() const {
    return T;
  }

  inline unsigned Length() const {
    return L;
  }
};

} // namespace Mangler

class ASTMangler {
  friend class ASTDemangler;

private:
  std::stringstream S;

protected:
  static std::map<ASTType, Mangler::MToken> TDMM;
  static std::map<ASTOpType, Mangler::MToken> ODMM;

public:
  ASTMangler() : S() { }

  ASTMangler(const ASTMangler& RHS)
  : S(RHS.S.str()) { }

  ASTMangler& operator=(const ASTMangler& RHS) {
    if (this != &RHS) {
      Clear();
      S << RHS.S.str();
    }

    return *this;
  }

  ~ASTMangler() = default;

  static void Init();

  void Clear() {
    S.clear();
    S.str("");
  }

  inline void Start() {
    Clear();
    S << "_Q";
    if (ASTDeclarationContextTracker::Instance().InCalibrationContext())
      S << ":C:";
  }

  void Underscore() {
    S << '_';
  }

  void EndExpression() {
    S << 'E';
  }

  void End() {
    S << "E_";
  }

  void Type(ASTType Ty);

  void ConstType(ASTType Ty);

  void TypeSize(ASTType Ty, unsigned Size);

  void ConstTypeSize(ASTType Ty, unsigned Size);

  template<typename __Type>
  void TypeValue(ASTType Ty, __Type Value) {
    static_assert(std::is_integral<__Type>::value,
                  "value type is not an integral type!");

    std::ios_base::fmtflags F = S.flags();
    S << TDMM[Ty].Token() << "0x" << std::hex
      << std::setw(sizeof(__Type) * 2U) << std::setfill('0') << Value;
    S.flags(F);
  }

  void MangleMPComplex(ASTType Ty, unsigned CSize, unsigned TSize = 0U);

  void MangleConstMPComplex(ASTType Ty, unsigned CSize, unsigned TSize = 0U);

  void Array(ASTType Ty, unsigned ASize);

  void ConstArray(ASTType Ty, unsigned ASize);

  void Array(ASTType Ty, unsigned ASize, unsigned TSize);

  void ConstArray(ASTType Ty, unsigned ASize, unsigned TSize);

  void ComplexArray(unsigned ASize, unsigned CSize, ASTType ETy, unsigned TSize);

  void ConstComplexArray(unsigned ASize, unsigned CSize, ASTType ETy, unsigned TSize);

  void RValueRef(ASTType Ty);

  void ConstRValueRef(ASTType Ty);

  void LValueRef(ASTType Ty);

  void ConstLValueRef(ASTType Ty);

  void OpType(ASTOpType OTy);

  inline void Identifier(const std::string& Id) {
    assert(!Id.empty() && "Invalid Identifier argument!");
    S << Id.length() << Id;
  }

  inline void Identifier(const char* Id) {
    assert(Id && "Invalid Identifier argument!");
    S << std::strlen(Id) << Id;
  }

  inline void ConstIdentifier(const std::string& Id) {
    S << TDMM[ASTTypeConst].Token();
    Identifier(Id);
  }

  void TypeIdentifier(ASTType Ty, const std::string& Id);

  void TypeIdentifier(ASTType Ty, unsigned Size, const std::string& Id);

  void ConstTypeIdentifier(ASTType Ty, const std::string& Id);

  void ConstTypeIdentifier(ASTType Ty, unsigned Size, const std::string& Id);

  void OpIdentifier(ASTOpType OTy);

  void StringLiteral(const std::string& SL);

  void StringLiteral(const char* SL);

  void String();

  void ConstString();

  void StringValue(const std::string& SV) {
    assert(!SV.empty() && "Invalid string value argument!");
    S << SV.c_str();
  }

  void NumericLiteral(int32_t NL);

  void NumericLiteral(uint32_t NL);

  void NumericLiteral(int64_t NL);

  void NumericLiteral(uint64_t NL);

  void NumericLiteral(const ASTMPIntegerNode* MPI);

  void NumericLiteral(float NL);

  void NumericLiteral(double NL);

  void NumericLiteral(long double NL);

  void NumericLiteral(const ASTMPDecimalNode* MPD);

  void NumericLiteral(const ASTMPComplexNode* MPC);

  void Array(unsigned ASize, ASTType Ty);

  void Array(unsigned ASize, ASTType Ty, unsigned TSize);

  void SetCall(const std::string& CS) {
    assert(!CS.empty() && "Invalid call target Identifier!");
    S << CS.c_str();
  }

  void SetModifierCall(const std::string& MS, const std::string& CS) {
    assert(!MS.empty() && "Invalid call modifier Identifier!");
    assert(!CS.empty() && "Invalid call target Identifier!");
    S << MS.c_str() << "E_" << CS.c_str();
  }

  void CallStart() {
    S << "cl";
  }

  void CallEnd() {
    S << 'E';
  }

  void FuncParam(unsigned IX, ASTType Ty) {
    S << "Fp" << IX << '_' << TDMM[Ty].Token() << 'E';
  }

  void FuncParam(unsigned IX, ASTType Ty, unsigned TSize) {
    S << "Fp" << IX << '_' << TDMM[Ty].Token() << TSize << 'E';
  }

  void FuncParam(unsigned IX, ASTType Ty, unsigned TSize,
                 const std::string& Id) {
    assert(!Id.empty() && "Invalid kernel param Identifier!");
    S << "Fp" << IX << '_' << TDMM[Ty].Token() << TSize
      << '_' << Id.length() << Id << 'E';
  }

  void FuncParam(unsigned IX, ASTType Ty, unsigned ASize,
                 ASTType ETy, unsigned ESize, const std::string& Id,
                 bool Const = false) {
    assert(!Id.empty() && "Invalid kernel param Identifier!");
    assert(Ty != ASTTypeUndefined && "Invalid array type!");

    if (ASTExpressionValidator::Instance().IsArrayType(Ty)) {
      if (Const)
        S << "Fp" << IX << '_' << TDMM[ASTTypeConst].Token()
          << TDMM[ASTTypeArray].Token() << ASize
          << '_' << TDMM[ETy].Token() << ESize << '_'<< Id.length() << Id << 'E';
      else
        S << "Fp" << IX << '_' << TDMM[ASTTypeArray].Token() << ASize
          << '_' << TDMM[ETy].Token() << ESize << '_'<< Id.length() << Id << 'E';
    }
  }

  void FuncArg(unsigned IX, ASTType Ty, const std::string& Id) {
    assert(!Id.empty() && "Invalid function arg Identifier!");
    S << "Fa" << IX << '_' << TDMM[Ty].Token() << Id.length() << Id;
  }

  void FuncArg(unsigned IX, ASTType Ty, unsigned Size,
               const std::string& Id) {
    assert(!Id.empty() && "Invalid function arg Identifier!");
    S << "Fa" << IX << '_' << Size << TDMM[Ty].Token()
      << Id.length() << Id << 'E';
  }

  void FuncArg(unsigned IX, ASTType Ty, unsigned ASize,
               ASTType ETy, unsigned ESize, const std::string& Id,
               bool Const = false) {
    assert(!Id.empty() && "Invalid function arg Identifier!");
    assert(Ty != ASTTypeUndefined && "Invalid array type!");

    if (ASTExpressionValidator::Instance().IsArrayType(Ty)) {
      if (Const)
        S << "Fa" << IX << '_' << TDMM[ASTTypeConst].Token()
          << TDMM[ASTTypeArray].Token() << ASize
          << '_' << TDMM[ETy].Token() << ESize << '_'<< Id.length() << Id << 'E';
      else
        S << "Fa" << IX << '_' << TDMM[ASTTypeArray].Token() << ASize
          << '_' << TDMM[ETy].Token() << ESize << '_'<< Id.length() << Id << 'E';
    }
  }

  void FuncArg(unsigned IX, ASTType Ty, unsigned Size,
               const char* Id) {
    assert(Id && "Invalid function arg Identifier!");
    S << "Fa" << IX << '_' << Size << TDMM[Ty].Token()
      << strlen(Id) << Id << 'E';
  }

  void FuncArg(unsigned IX, ASTType Ty, unsigned ASize,
               ASTType ETy, unsigned ESize, const char* Id,
               bool Const = false) {
    assert(Id && "Invalid function arg Identifier!");
    assert(Ty != ASTTypeUndefined && "Invalid array type!");

    if (ASTExpressionValidator::Instance().IsArrayType(Ty)) {
      if (Const)
        S << "Fa" << IX << '_' << TDMM[ASTTypeConst].Token()
          << TDMM[ASTTypeArray].Token() << ASize
          << '_' << TDMM[ETy].Token() << ESize << '_'<< strlen(Id)
          << Id << 'E';
      else
        S << "Fa" << IX << '_' << TDMM[ASTTypeArray].Token() << ASize
          << '_' << TDMM[ETy].Token() << ESize << '_'<< strlen(Id)
          << Id << 'E';
    }
  }

  void FuncArg(unsigned IX, ASTType Ty, const char* Id) {
    assert(Id && "Invalid function arg Identifier!");
    S << "Fa" << IX << '_' << TDMM[Ty].Token() << strlen(Id) << Id;
  }

  void FuncArg(unsigned IX, const std::string& Id) {
    assert(!Id.empty() && "Invalid function arg Identifier!");
    S << "Fa" << IX << '_' << Id.length() << Id;
  }

  void FuncArg(unsigned IX, const char* Id) {
    assert(Id && "Invalid function arg Identifier!");
    S << "Fa" << IX << '_' << strlen(Id) << Id;
  }

  void FuncReturn(ASTType Ty) {
    S << "Fr" << TDMM[Ty].Token() << 'E';
  }

  void FuncReturn(ASTType Ty, unsigned Size) {
    S << "Fr" << TDMM[Ty].Token() << Size << 'E';
  }

  void FuncReturn(ASTType Ty, const std::string& Id) {
    assert(!Id.empty() && "Invalid function return Identifier!");
    S << "Fr" << TDMM[Ty].Token() << '_' << Id.length() << Id
      << 'E';
  }

  void FuncReturn(ASTType Ty, unsigned Size, const std::string& Id) {
    assert(!Id.empty() && "Invalid function return Identifier!");
    S << "Fr" << TDMM[Ty].Token() << Size << '_' << Id.length()
      << Id << 'E';
  }

  void FuncReturn(ASTType Ty, const char* Id) {
    assert(Id && "Invalid function return Identifier!");
    S << "Fr" << TDMM[Ty].Token() << '_' << strlen(Id) << Id
      << 'E';
  }

  void FuncReturn(ASTType Ty, unsigned Size, const char* Id) {
    assert(Id && "Invalid function return Identifier!");
    S << "Fr" << TDMM[Ty].Token() << Size << '_' << strlen(Id)
      << Id << 'E';
  }

  void KernelParam(unsigned IX, ASTType Ty) {
    S << "Kp" << IX << '_' << TDMM[Ty].Token() << 'E';
  }

  void KernelParam(unsigned IX, const std::string& MN) {
    S << "Kp" << IX << '_' << MN << 'E';
  }

  void KernelParam(unsigned IX, ASTType Ty, unsigned TSize) {
    S << "Kp" << IX << '_' << TDMM[Ty].Token() << TSize << 'E';
  }

  void KernelParam(unsigned IX, ASTType Ty, unsigned TSize,
                 const std::string& Id) {
    assert(!Id.empty() && "Invalid kernel param Identifier!");
    S << "Kp" << IX << '_' << TDMM[Ty].Token() << TSize
      << '_' << Id.length() << Id << 'E';
  }

  void KernelArg(unsigned IX, const std::string& MN) {
    S << "Ka" << IX << '_' << MN << 'E';
  }

  void KernelArg(unsigned IX, ASTType Ty, const std::string& Id) {
    assert(!Id.empty() && "Invalid kernel arg Identifier!");
    S << "Ka" << IX << '_' << TDMM[Ty].Token() << Id.length() << Id;
  }

  void KernelArg(unsigned IX, ASTType Ty, const char* Id) {
    assert(Id && "Invalid kernel arg Identifier!");
    S << "Ka" << IX << '_' << TDMM[Ty].Token() << strlen(Id) << Id;
  }

  void KernelArg(unsigned IX, ASTType Ty, unsigned Size,
                 const std::string& Id) {
    assert(!Id.empty() && "Invalid kernel arg Identifier!");
    S << "Ka" << IX << '_' << Size << TDMM[Ty].Token()
      << Id.length() << Id << 'E';
  }

  void KernelArg(unsigned IX, ASTType Ty, unsigned Size,
                 const char* Id) {
    assert(Id && "Invalid kernel arg Identifier!");
    S << "Ka" << IX << '_' << Size << TDMM[Ty].Token()
      << strlen(Id) << Id << 'E';
  }

  void GateParam(unsigned IX, ASTType Ty, unsigned TSize) {
    S << "Gp" << IX << '_' << TDMM[Ty].Token() << TSize << 'E';
  }

  void GateParam(unsigned IX, const std::string& Id) {
    S << "Gp" << IX << '_' << Id;
  }

  void GateParam(unsigned IX, ASTType Ty, unsigned TSize,
                 const std::string& Id) {
    assert(!Id.empty() && "Invalid gate param Identifier!");
    S << "Gp" << IX << '_' << TDMM[Ty].Token() << TSize
      << '_' << Id.length() << Id << 'E';
  }

  void GateArg(unsigned IX, ASTType Ty, unsigned Size,
               const std::string& Id) {
    assert(!Id.empty() && "Invalid gate arg Identifier!");
    S << "Ga" << IX << '_' << Size << TDMM[Ty].Token()
      << Id.length() << Id << 'E';
  }

  void GateArg(unsigned IX, const std::string& Id) {
    S << "Ga" << IX << '_' << Id;
  }

  void GateArg(unsigned IX, ASTType Ty, unsigned Size,
               const char* Id) {
    assert(Id && "Invalid gate arg Identifier!");
    S << "Ga" << IX << '_' << Size << TDMM[Ty].Token()
      << strlen(Id) << Id << 'E';
  }

  void DefcalParam(unsigned IX, const std::string& MN) {
    S << "Dp" << IX << '_' << MN << 'E';
  }

  void DefcalParam(unsigned IX, ASTType Ty, unsigned TSize) {
    S << "Dp" << IX << '_' << TDMM[Ty].Token() << TSize << 'E';
  }

  void DefcalParam(unsigned IX, ASTType Ty, unsigned TSize,
                   const std::string& Id) {
    assert(!Id.empty() && "Invalid defcal param Identifier!");
    S << "Dp" << IX << '_' << TDMM[Ty].Token() << TSize
      << '_' << Id.length() << Id << 'E';
  }

  void DefcalArg(unsigned IX, const std::string& MN) {
    S << "Da" << IX << '_' << MN << 'E';
  }

  void DefcalArg(unsigned IX, ASTType Ty, unsigned TSize,
                 const std::string& Id) {
    assert(!Id.empty() && "Invalid defcal arg Identifier!");
    S << "Da" << IX << '_' << TSize << TDMM[Ty].Token()
      << Id.length() << Id << 'E';
  }

  void DefcalArg(unsigned IX, ASTType Ty, unsigned TSize,
                 const char* Id) {
    assert(Id && "Invalid defcal arg Identifier!");
    S << "Da" << IX << '_' << TSize << TDMM[Ty].Token()
      << strlen(Id) << Id << 'E';
  }

  void QubitTarget(unsigned IX, const std::string& Id) {
    assert(!Id.empty() && "Invalid qubit target Identifier!");
    S << "Qt" << IX << '_' << Id.length() << Id << 'E';
  }

  void MangleMPDecimal(const ASTMPDecimalNode* MPD);

  void MangleConstMPDecimal(const ASTMPDecimalNode* MPD);

  const char* GetTypeEncoding(ASTType Ty) const {
    std::map<ASTType, Mangler::MToken>::const_iterator TI = TDMM.find(Ty);
    return TI == TDMM.end() ? nullptr : (*TI).second.Token();
  }

  unsigned GetTypeEncodingLength(ASTType Ty) const {
    std::map<ASTType, Mangler::MToken>::const_iterator TI = TDMM.find(Ty);
    return TI == TDMM.end() ?
           static_cast<unsigned>(~0x0) : (*TI).second.Length();
  }

  const char* GetOpEncoding(ASTOpType Ty) const {
    std::map<ASTOpType, Mangler::MToken>::const_iterator TI = ODMM.find(Ty);
    return TI == ODMM.end() ? nullptr : (*TI).second.Token();
  }

  unsigned GetOpEncodingLength(ASTOpType Ty) const {
    std::map<ASTOpType, Mangler::MToken>::const_iterator TI = ODMM.find(Ty);
    return TI == ODMM.end() ?
           static_cast<unsigned>(~0x0) : (*TI).second.Length();
  }

  static std::string MangleIdentifier(const ASTIdentifierNode* Id);

  static std::string MangleIdentifier(const ASTIdentifierRefNode* Id);

  void Print() const {
    std::cout << S.str().c_str() << std::endl;
  }

  void Print(std::ostream& OS) const {
    OS << S.str().c_str();
  }

  std::string AsString() {
    return S.str();
  }
};

namespace Mangler {

struct NWP {
  std::string Name;
  ASTType Ty;
  ASTOpType OpTy;
  LengthUnit LU;

  union {
    int32_t SBits;
    uint32_t UBits;
  };

  union {
    int32_t SIVal;
    uint32_t UIVal;
    int64_t SLVal;
    uint64_t ULVal;
    float FVal;
    double DVal;
    long double LDVal;
  };

  std::string SVal;
  unsigned IX;
  bool L;
  char S;
  char V;

  NWP() : Name(), Ty(ASTTypeUndefined), OpTy(ASTOpTypeUndefined),
  LU(LengthUnspecified), UBits(0U), LDVal(0.0), SVal(),
  IX(static_cast<unsigned>(~0x0)), L(false), S('U'), V('\0') { }

  NWP(const NWP& RHS)
  : Name(RHS.Name), Ty(RHS.Ty), OpTy(RHS.OpTy), LU(RHS.LU),
  UBits(RHS.UBits), LDVal(RHS.LDVal), SVal(RHS.SVal),
  IX(RHS.IX), L(RHS.L), S(RHS.S), V(RHS.V) { }

  NWP& operator=(const NWP& RHS) {
    if (this != &RHS) {
      Name = RHS.Name;
      Ty = RHS.Ty;
      OpTy = RHS.OpTy;
      LU = RHS.LU;
      UBits = RHS.UBits;
      LDVal = RHS.LDVal;
      SVal = RHS.SVal;
      IX = RHS.IX;
      L = RHS.L;
      S = RHS.S;
      V = RHS.V;
    }

    return *this;
  }

  ~NWP() = default;

  inline bool operator==(const NWP& RHS) const {
    return Name == RHS.Name && OpTy == RHS.OpTy &&
      LU == RHS.LU && Ty == RHS.Ty && UBits == RHS.UBits &&
      LDVal == RHS.LDVal && SVal == RHS.SVal &&
      S == RHS.S;
  }

  inline bool operator!= (const NWP& RHS) const {
    return !(*this == RHS);
  }

  void Print() const {
    std::cout << "<NWP>" << (const void*) this << "</NWP>" << std::endl;
    if (Ty != ASTTypeUndefined)
      std::cout << "<Type>" << PrintTypeEnum(Ty) << "</Type>" << std::endl;

    if (OpTy != ASTOpTypeUndefined)
      std::cout << "<OpType>" << PrintOpTypeEnum(OpTy) << "</OpType>"
        << std::endl;

    if (LU != LengthUnit::LengthUnspecified)
      std::cout << "<LengthUnit>" << PrintLengthUnit(LU)
        << "</LengthUnit>" << std::endl;

    if (!Name.empty())
      std::cout << "<Name>" << Name << "</Name>" << std::endl;

    if (S == 'S')
      std::cout << "<SBits>" << SBits << "</SBits>" << std::endl;
    else if (S == 'U')
      std::cout << "<UBits>" << UBits << "</UBits>" << std::endl;

    if (!SVal.empty())
      std::cout << "<SVal>" << SVal << "</SVal>" << std::endl;

    if (IX != static_cast<unsigned>(~0x0))
      std::cout << "<Index>" << IX << "</Index>" << std::endl;

    switch (V) {
    case 'i':
      std::cout << "<Value>" << SIVal << "</Value>" << std::endl;
      break;
    case 'u':
      std::cout << "<Value>" << UIVal << "</Value>" << std::endl;
      break;
    case 'I':
      std::cout << "<Value>" << SLVal << "</Value>" << std::endl;
      break;
    case 'U':
      std::cout << "<Value>" << ULVal << "</Value>" << std::endl;
      break;
    case 'F':
      std::cout << "<Value>" << FVal << "</Value>" << std::endl;
      break;
    case 'D':
      std::cout << "<Value>" << DVal << "</Value>" << std::endl;
      break;
    case 'L':
      std::cout << "<Value>" << LDVal << "</Value>" << std::endl;
      break;
    default:
      break;
    }
  }
};

} // namespace Mangler

class ASTDemangled;

class ASTDemangledRegistry {
private:
  static ASTDemangledRegistry ADR;
  static std::set<const ASTDemangled*> RS;

protected:
  ASTDemangledRegistry() = default;

public:
  static ASTDemangledRegistry& Instance() {
    return ADR;
  }

  void Register(const ASTDemangled* DM) {
    if (DM && ASTObjectTracker::Instance().IsOnHeap(DM)) {
      RS.insert(DM);
    }
  }

  void Unregister(const ASTDemangled* DM) {
    if (DM)
      RS.erase(DM);
  }

  void Release();
};

class ASTDemangled {
  friend class ASTDemangler;

protected:
  // Type Description;
  Mangler::NWP TD;

  // 1st Dependent Type Description.
  Mangler::NWP DTD0;

  // 2nd Dependent Type Description.
  Mangler::NWP DTD1;

  // Vector of dependent types.
  std::vector<ASTDemangled*> DTV;

public:
  ASTDemangled() : TD(), DTD0(), DTD1(), DTV() { }

  ASTDemangled(const ASTDemangled& RHS)
  : TD(RHS.TD), DTD0(RHS.DTD0), DTD1(RHS.DTD1), DTV(RHS.DTV) { }

  ASTDemangled& operator=(const ASTDemangled& RHS) {
    if (this != &RHS) {
      TD = RHS.TD;
      DTD0 = RHS.DTD0;
      DTD1 = RHS.DTD1;
      DTV = RHS.DTV;
    }

    return *this;
  }

  ~ASTDemangled() = default;

  void Clear() {
    TD = Mangler::NWP();
    DTD0 = Mangler::NWP();
    DTD1 = Mangler::NWP();
    DTV.clear();
  }

  void* operator new(size_t S) {
    void* P = malloc(S);
    assert(P && "Failed to allocate memory!");
    ASTDemangledRegistry::Instance().Register(reinterpret_cast<ASTDemangled*>(P));
    return P;
  }

  void operator delete(void* P) {
    if (P) {
      ASTDemangledRegistry::Instance().Unregister(reinterpret_cast<ASTDemangled*>(P));
      free(P);
    }
  }

  const std::string& GetName() const {
    return TD.Name;
  }

  const std::string& GetFirstDependentName() const {
    return DTD0.Name;
  }

  const std::string& GetSecondDependentName() const {
    return DTD1.Name;
  }

  bool HasType() const {
    return TD.Ty != ASTTypeUndefined;
  }

  ASTType GetType() const {
    return TD.Ty;
  }

  ASTType GetFirstDependentType() const {
    return DTD0.Ty;
  }

  ASTType GetSecondDependentType() const {
    return DTD1.Ty;
  }

  const Mangler::NWP& GetTypeDescription() const {
    return TD;
  }

  const Mangler::NWP& GetFirstDependentTypeDescription() const {
    return DTD0;
  }

  const Mangler::NWP& GetSecondDependentTypeDescription() const {
    return DTD1;
  }

  void Print() const {
    std::cout << "<DemangledType>" << std::endl;
    if (TD.Ty != ASTTypeUndefined) {
      std::cout << "<TypeDescription>" << std::endl;
      TD.Print();
      std::cout << "</TypeDescription>" << std::endl;
    } else if (TD.OpTy != ASTOpTypeUndefined) {
      std::cout << "<OpTypeDescription>" << std::endl;
      TD.Print();
      std::cout << "</OpTypeDescription>" << std::endl;
    } else if (TD.LU != LengthUnit::LengthUnspecified) {
      std::cout << "<OpTypeDescription>" << std::endl;
      TD.Print();
      std::cout << "</OpTypeDescription>" << std::endl;
    }

    if (DTD0.Ty != ASTTypeUndefined) {
      std::cout << "<DependentTypeDescription0>" << std::endl;
      DTD0.Print();
      std::cout << "</DependentTypeDescription0>" << std::endl;
    } else if (DTD0.OpTy != ASTOpTypeUndefined) {
      std::cout << "<OpTypeDescription>" << std::endl;
      DTD0.Print();
      std::cout << "</OpTypeDescription>" << std::endl;
    } else if (DTD0.LU != LengthUnit::LengthUnspecified) {
      std::cout << "<OpTypeDescription>" << std::endl;
      DTD0.Print();
      std::cout << "</OpTypeDescription>" << std::endl;
    }

    if (DTD1.Ty != ASTTypeUndefined) {
      std::cout << "<DependentTypeDescription1>" << std::endl;
      DTD1.Print();
      std::cout << "</DependentTypeDescription1>" << std::endl;
    } else if (DTD1.OpTy != ASTOpTypeUndefined) {
      std::cout << "<OpTypeDescription>" << std::endl;
      DTD1.Print();
      std::cout << "</OpTypeDescription>" << std::endl;
    } else if (DTD1.LU != LengthUnit::LengthUnspecified) {
      std::cout << "<OpTypeDescription>" << std::endl;
      DTD1.Print();
      std::cout << "</OpTypeDescription>" << std::endl;
    }

    std::cout << "</DemangledType>" << std::endl;
  }
};

struct DMGate {
  std::string Name;
  ASTType GTy;
  std::vector<std::tuple<ASTType, unsigned, std::string>> Params;
  std::vector<std::tuple<ASTType, unsigned, std::string>> Qubits;

  DMGate() : Name(), GTy(ASTTypeGate), Params(), Qubits() { }

  DMGate(const DMGate& RHS)
  : Name(RHS.Name), GTy(RHS.GTy), Params(RHS.Params),
  Qubits(RHS.Qubits) { }

  ~DMGate() = default;

  DMGate& operator=(const DMGate& RHS) {
    if (this != &RHS) {
      Name = RHS.Name;
      GTy = RHS.GTy;
      Params = RHS.Params;
      Qubits = RHS.Qubits;
    }

    return *this;
  }
};

struct DMMeasure {
  std::string Target;
  std::string Result;
  std::vector<unsigned> TIV;
  std::vector<unsigned> RIV;

  DMMeasure() : Target(), Result(), TIV(), RIV() { }

  DMMeasure(const DMMeasure& RHS)
  : Target(RHS.Target), Result(RHS.Result),
  TIV(RHS.TIV), RIV(RHS.RIV) { }

  ~DMMeasure() = default;

  DMMeasure& operator=(const DMMeasure& RHS) {
    if (this != &RHS) {
      Target = RHS.Target;
      Result = RHS.Result;
      TIV = RHS.TIV;
      RIV = RHS.RIV;
    }

    return *this;
  }
};

struct DMReset {
  std::string Target;

  DMReset() : Target() { }

  DMReset(const DMReset& RHS) : Target(RHS.Target) { }

  ~DMReset() = default;

  DMReset& operator=(const DMReset& RHS) {
    if (this != &RHS) {
      Target = RHS.Target;
    }

    return *this;
  }
};

struct DMDefcal {
  std::string Name;

  union {
    void* V;
    DMMeasure* M;
    DMReset* R;
  };

  ASTType DTy;
  std::vector<std::tuple<ASTType, unsigned, std::string>> Params;
  std::vector<std::tuple<ASTType, unsigned, std::string>> Qubits;

  DMDefcal() : Name(), V(nullptr), DTy(ASTTypeDefcal), Params(), Qubits() { }

  DMDefcal(DMMeasure* DMM)
  : Name(), M(DMM), DTy(ASTTypeDefcalMeasure), Params(), Qubits() { }

  DMDefcal(DMReset* DMR)
  : Name(), R(DMR), DTy(ASTTypeDefcalReset), Params(), Qubits() { }

  DMDefcal(const DMDefcal& RHS)
  : Name(RHS.Name), V(RHS.V), DTy(RHS.DTy), Params(RHS.Params),
  Qubits(RHS.Qubits) { }

  ~DMDefcal() {
    switch (DTy) {
    case ASTTypeDefcalMeasure:
      delete M;
      break;
    case ASTTypeDefcalReset:
      delete R;
      break;
    default:
      break;
    }
  }
};

enum RXParam : unsigned;
enum RXIndex : unsigned;

class ASTDemangler {
  friend class ASTMangler;

private:
  struct RXM {
    unsigned I;
    std::regex R;
    ASTType T;
    bool N;

    RXM() : I(0), R(), T(ASTTypeUndefined), N(false) { }
    RXM(const RXM& RHS) : I(RHS.I), R(RHS.R), T(RHS.T), N(RHS.N) { }
    RXM(unsigned IX, const std::regex& RX, ASTType Ty, bool NEG = false)
    : I(IX), R(RX), T(Ty), N(NEG) { }
    ~RXM() = default;
    RXM& operator=(const RXM& RHS) {
      if (this != &RHS) {
        I = RHS.I;
        R = RHS.R;
        T = RHS.T;
        N = RHS.N;
      }

      return *this;
    }
  };

private:
  ASTDemangled DM;

  // FIXME: I DON'T THINK THIS WILL EVER BE USED.
  ASTDemangled DTDM;               // Dependent Type (Complex, Arrays).

  std::vector<ASTDemangled*> VDMT; // Additional types and expressions.
  std::set<ASTDemangled*> DMS;
  bool InCalBlock;

protected:
  static std::map<ASTType, Mangler::MToken> TDMM;
  static std::map<ASTOpType, Mangler::MToken> ODMM;
  static std::vector<ASTDemangler::RXM> TYMM;
  static std::map<RXParam, std::regex> PAMM;

protected:
  const char* ParseType(const char* S, ASTDemangled* DMP);
  const char* ParseReturnType(const char* S, ASTDemangled* DMP);
  const char* ParseOpType(const char* S, ASTDemangled* DMP,
                          std::vector<ASTDemangled*>& TV);
  const char* ParseOpFoldType(const char* S,
                              std::vector<ASTDemangled*>& FV);
  const char* ParseName(const char* S, ASTDemangled* DMP);
  const char* ParseTimeUnit(const char* S, ASTDemangled* DMP);
  std::string ParseTimeUnit(const char* S, unsigned* J);
  const char* ParseParam(const char* S, ASTType Ty, ASTDemangled* DMP);
  const char* ParseArg(const char* S, ASTType Ty, ASTDemangled* DMP);
  const char* ParseNumericLiteral(const char* S, ASTDemangled* DMP);
  const char* ParseBinaryOp(const char* S, ASTDemangled* DMP,
                            std::vector<ASTDemangled*>& VDM);
  const char* ParseUnaryOp(const char* S, ASTDemangled* DMP,
                           std::vector<ASTDemangled*>& VDM);
  const char* ParseMeasure(const char* S, ASTDemangled* DMP);
  const char* ParseDefcalMeasure(const char* S, ASTDemangled* DMP);
  const char* ParseDefcalReset(const char* S, ASTDemangled* DMP);
  const char* ParseFunctionDefinition(const char* S, ASTDemangled* DMP,
                                      bool Extern = false);
  const char* ParseFunctionDeclaration(const char* S, ASTDemangled* DMP,
                                       bool Extern = false);
  const char* ParseDurationOfTarget(const char* S, ASTDemangled* DMP);

  const char* ResolveLiteral(const char* S, ASTType& RTy,
                             ASTDemangled* DMP, bool& N);
  const char* ResolveLiteralCReal(const char* S, ASTDemangled* DMP);
  const char* ResolveLiteralCImag(const char* S, ASTDemangled* DMP);

  const char* ValidateEndOfExpression(const char* S);
  const char* SkipExpressionTerminator(const char* S);
  const char* ParseSubsequent(const char* S);

  bool IsParamOrArgument(const char* S) const;
  bool IsParamOrArgument(ASTType Ty) const;
  bool IsBinaryOrUnaryOp(const char* S) const;
  bool IsBinaryOrUnaryOp(ASTType Ty) const;
  bool IsBinaryOp(const char* S) const;
  bool IsUnaryOp(const char* S) const;
  bool IsOperator(const char* S);

  void DeserializeDefcal(std::stringstream& SR);
  void DeserializeDefcalMeasure(std::stringstream& SR);
  void DeserializeDefcalReset(std::stringstream& SR);
  void DeserializeFunction(std::stringstream& SR);
  void DeserializeGate(std::stringstream& SR);
  void DeserializeKernel(std::stringstream& SR);
  void DeserializeScalar(std::stringstream& SR, const ASTDemangled& DMG);
  void DeserializeScalar(std::stringstream& SR);
  void DeserializeNonScalar(std::stringstream& SR, const ASTDemangled& DMG);
  void DeserializeArray(std::stringstream& SR, ASTType Ty);
  void DeserializeDurationOf(std::stringstream& SR);
  void DeserializeDurationOf(std::stringstream& SR, const ASTDemangled& DMG);
  void DeserializeNonScalar(std::stringstream& SR);

  void DeserializeBinaryOp(std::stringstream& SR);
  void DeserializeBinaryOp(std::stringstream& SR,
                           std::vector<ASTDemangled*>::const_iterator B,
                           std::vector<ASTDemangled*>::const_iterator E);
  void DeserializeUnaryOp(std::stringstream& SR);
  void DeserializeUnaryOp(std::stringstream& SR,
                          std::vector<ASTDemangled*>::const_iterator B,
                          std::vector<ASTDemangled*>::const_iterator);

  void DeserializeAggregate(std::stringstream& SR);
  void DeserializeComplex(std::stringstream& SR);
  void DeserializeQubit(std::stringstream& SR);

  void PrintVDMT(const std::vector<ASTDemangled*>& V) const;

public:
  ASTDemangler() : DM(), DTDM(), VDMT(), DMS(), InCalBlock(false)
  { }

  ASTDemangler(const ASTDemangler& RHS)
  : DM(RHS.DM), DTDM(RHS.DTDM), VDMT(RHS.VDMT),
  DMS(RHS.DMS), InCalBlock(RHS.InCalBlock) { }

  ASTDemangler& operator=(const ASTDemangler& RHS) {
    if (this != &RHS) {
      DM = RHS.DM;
      DTDM = RHS.DTDM;
      VDMT = RHS.VDMT;
      DMS = RHS.DMS;
      InCalBlock = RHS.InCalBlock;
    }

    return *this;
  }

  ~ASTDemangler() = default;

  static void Init();

  static const char* TypeName(ASTType Ty) {
    std::map<ASTType, Mangler::MToken>::iterator I = TDMM.find(Ty);
    return I == TDMM.end() ? nullptr : (*I).second.Token();
  }

  static const char* OpName(ASTOpType Ty) {
    std::map<ASTOpType, Mangler::MToken>::iterator I = ODMM.find(Ty);
    return I == ODMM.end() ? nullptr : (*I).second.Token();
  }

  std::string AsString();

  bool Demangle(const std::string& N);

  std::unique_ptr<DMGate> Gate(const std::string& N);

  std::unique_ptr<DMDefcal> Defcal(const std::string& N);

  bool HasDependentTypes() const {
    return !VDMT.empty();
  }

  const ASTDemangled& GetType() const {
    return DM;
  }

  const ASTDemangled& GetDependentType() const {
    return DTDM;
  }

  const std::vector<ASTDemangled*>& GetDependentTypes() const {
    return VDMT;
  }

  void Print() const;
};

} // namespace QASM

#endif // __QASM_AST_MANGLER_H

