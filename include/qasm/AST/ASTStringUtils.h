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

#ifndef __QASM_AST_STRING_UTILS_H
#define __QASM_AST_STRING_UTILS_H

#include <algorithm>
#include <cassert>
#include <cctype>
#include <climits>
#include <codecvt>
#include <cstring>
#include <locale>
#include <memory>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace QASM {

class ASTStringUtils {
private:
  // In ASTBuilder.cpp.
  static ASTStringUtils SU;
  static std::regex ZRD;
  static std::regex ZRZ;
  static std::regex MPNAN;
  static std::regex MPINF;
  static std::string ES;

protected:
  ASTStringUtils() = default;

public:
  static ASTStringUtils &Instance() { return SU; }

  ~ASTStringUtils() = default;

  template <typename __Type>
  static __Type StringToUnsigned(const char *S) {
    static_assert(std::is_integral<__Type>::value,
                  "Type is not an integral type!");
    static_assert(!std::is_signed<__Type>::value,
                  "Type is not an unsigned type!");

    __Type X = 0;

    if (S)
      while (*S && std::isdigit(*S))
        X = X * 10 + (*S++ - '0');

    return X;
  }

  template <typename __Type>
  static __Type StringToUnsigned(const char *P, unsigned *J) {
    static_assert(std::is_integral<__Type>::value,
                  "Type is not an integral type!");
    static_assert(!std::is_signed<__Type>::value,
                  "Type is not an unsigned type!");

    *J = 0U;
    __Type X = 0U;

    if (P) {
      while (*P && std::isdigit(*P)) {
        X = X * 10 + (*P++ - '0');
        ++*J;
      }
    }

    return X;
  }

  template <typename __Type>
  static __Type StringToUnsigned(const char *P, char S, unsigned *J) {
    static_assert(std::is_integral<__Type>::value,
                  "Type is not an integral type!");
    static_assert(!std::is_signed<__Type>::value,
                  "Type is not an unsigned type!");

    *J = 0U;
    __Type X = 0;

    if (P) {
      while (*P && std::isdigit(*P) && (*P != S)) {
        X = X * 10 + (*P++ - '0');
        ++*J;
      }
    }

    return X;
  }

  template <typename __Type>
  static __Type StringToSigned(const char *S) {
    static_assert(std::is_integral<__Type>::value,
                  "Type is not an integral type!");
    static_assert(std::is_signed<__Type>::value, "Type is not a signed type!");

    __Type X = 0;
    bool N = false;

    if (S && *S && (*S == '-' || *S == '+'))
      N = *S++ == '-';

    if (S)
      while (std::isdigit(*S))
        X = X * 10 + (*S++ - '0');

    return N ? -X : X;
  }

  template <typename __Type>
  static __Type StringToSigned(const char *P, uint32_t *J) {
    static_assert(std::is_integral<__Type>::value,
                  "Type is not an integral type!");
    static_assert(std::is_signed<__Type>::value, "Type is not a signed type!");

    __Type X = 0;
    bool N = false;
    *J = 0U;

    if (P) {
      if (*P == '-' || *P == '+')
        N = *P++ == '-';

      while (std::isdigit(*P)) {
        X = X * 10 + (*P++ - '0');
        ++*J;
      }
    }

    return N ? -X : X;
  }

  template <typename __Type>
  static __Type StringToSigned(const char *P, char S, unsigned *J) {
    static_assert(std::is_integral<__Type>::value,
                  "Type is not an integral type!");
    static_assert(std::is_signed<__Type>::value, "Type is not a signed type!");

    __Type X = 0;
    bool N = false;
    *J = 0;

    if (P) {
      if (*P == '-' || *P == '+')
        N = *P++ == '-';

      while (std::isdigit(*P) && (*P != S)) {
        X = X * 10 + (*P++ - '0');
        ++*J;
      }
    }

    return N ? -X : X;
  }

  template <typename __Type>
  static __Type HexToUnsigned(const char *P, unsigned L, unsigned *J) {
    static_assert(std::is_integral<__Type>::value,
                  "Type is not an integral type!");
    static_assert(!std::is_signed<__Type>::value,
                  "Type is not an unsigned type!");

    __Type X = 0U;
    *J = 0U;

    if (P && *P) {
      const char *R = P;

      if (R[0] == '0' && (R[1] == 'x' || R[1] == 'X'))
        R += 2;

      for (unsigned I = 0; I < L; ++I) {
        switch (R[I]) {
        case 'a':
        case 'A':
          X = X * 16 + 10;
          break;
        case 'b':
        case 'B':
          X = X * 16 + 11;
          break;
        case 'c':
        case 'C':
          X = X * 16 + 12;
          break;
        case 'd':
        case 'D':
          X = X * 16 + 13;
          break;
        case 'e':
        case 'E':
          X = X * 16 + 14;
          break;
        case 'f':
        case 'F':
          X = X * 16 + 15;
          break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          X = X * 16 + (R[I] - '0');
          break;
        default:
          break;
        }

        ++*J;
      }
    }

    return X;
  }

  std::string GenRandomString(unsigned Len) {
    static const char AlNum[] = "0123456789"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                "abcdefghijklmnopqrstuvwxyz"
                                "-_+.";

    static thread_local std::default_random_engine RE(std::random_device{}());
    static thread_local std::uniform_int_distribution<int> RD(0, sizeof(AlNum) -
                                                                     2);

    std::string R(Len < 19 ? 19 : Len, '\0');

    for (std::string::value_type &C : R)
      C = AlNum[RD(RE)];

    return R;
  }

  std::string Sanitize(const std::string &S) const {
    if (S.empty())
      return std::string();

    std::stringstream SR;
    const char *SP = S.c_str();

    for (std::string::size_type I = 0; I < S.size(); ++I) {
      if (*(SP + I) != '\"' && *(SP + I) != '\'' && *(SP + I) != ' ' &&
          *(SP + I) != ';') {
        SR << *(SP + I);
      }
    }

    return SR.str();
  }

  bool IsWhitespace(const std::string &S) const {
    return S == u8" " || S == u8"\n" || S == u8"\t" || S == u8"\v" ||
           S == u8"\f" || S == u8"\r";
  }

  void RemoveWhitespace(std::string &S) const {
    if (!S.empty()) {
      S.erase(std::remove_if(S.begin(), S.end(),
                             [](char C) { return std::isspace(C); }),
              S.end());
    }
  }

  void RemoveTrailingZeros(std::string &S) const {
    if (!S.empty()) {
      std::smatch SM;
      if (std::regex_match(S, SM, ASTStringUtils::MPNAN)) {
        S = "NaN";
        return;
      } else if (std::regex_match(S, SM, ASTStringUtils::MPINF)) {
        S = "Inf";
        return;
      }

      std::smatch ZM;
      if (std::regex_match(S, ZM, ASTStringUtils::ZRD) ||
          std::regex_match(S, ZM, ASTStringUtils::ZRZ)) {
        S = "0.0";
        return;
      }

      std::string R = S.substr(0, S.find_last_not_of(u8'0') + 1);
      if (R[R.length() - 1] == u8'.')
        R += u8'0';

      if (R[R.length() - 1] != (u8'0')) {
        unsigned N = 2U;
        unsigned C = 1U;
        unsigned Z = 0U;

        while (R[R.length() - C] != (u8'0') && R[R.length() - C] != (u8'.'))
          ++C;

        N = C;
        while (R[R.length() - N++] == (u8'0')) {
          ++Z;
          if (Z > C) {
            unsigned K = 1U;
            while (K < C)
              R[R.length() - K++] = (u8'0');

            break;
          }
        }
      }

      R = R.substr(0, R.find_last_not_of(u8'0') + 1);
      if (R[R.length() - 1] == (u8'.'))
        R += '0';

      S = R;
    }
  }

  void AddDecimalPoint(std::string &S) const {
    if (S.empty()) {
      S = "0.0";
    } else {
      std::smatch SM;
      if (std::regex_match(S, SM, ASTStringUtils::MPNAN) ||
          std::regex_match(S, SM, ASTStringUtils::MPINF))
        return;

      if (S.find('.') == std::string::npos)
        S += ".0";
    }
  }

  void SaneDecimal(std::string &S) const {
    AddDecimalPoint(S);
    RemoveWhitespace(S);
    RemoveTrailingZeros(S);
  }

  std::string RemoveManglingTerminator(const std::string_view &SV) {
    std::stringstream S;

    if (!SV.empty()) {
      for (std::string_view::const_iterator I = SV.begin(); I != SV.end();
           ++I) {
        if ((*I) != '_' && (*I) != 'E')
          S << (*I);
        if ((*I) == '_' || (*I) == 'E')
          break;
      }
    }

    return S.str();
  }

  std::string Substring(const char *S, char T) {
    assert(S && "Invalid string argument!");

    std::stringstream SS;
    while (*S && (*S != T))
      SS << *S++;

    return SS.str();
  }

  std::string ToLower(const std::string &S) const {
    if (S.empty())
      return S;

    std::string R = S;
    std::transform(S.begin(), S.end(), R.begin(),
                   [](unsigned char C) { return std::tolower(C); });
    return R;
  }

  std::string ToUpper(const std::string &S) const {
    if (S.empty())
      return S;

    std::string R = S;
    std::transform(S.begin(), S.end(), R.begin(),
                   [](unsigned char C) { return std::toupper(C); });
    return R;
  }

  std::string SanitizeMangled(const std::string &S) const {
    if (S.length() > 2U && S[0] == u8'_' && S[1] == u8'Q') {
      std::string::size_type LE = S.find_first_of(u8'E');
      return LE == std::string::npos ? S.substr(2, std::string::npos)
                                     : S.substr(2, LE - 2);
    }

    return "";
  }

  std::string SanitizeFullMangled(const std::string &S) const {
    std::string::size_type Z = S.length();
    if (Z > 2) {
      std::string::size_type LE = S.find_last_of(u8'E');
      if (LE == std::string::npos)
        return S.substr(2, std::string::npos);

      while (S[LE] == u8'E')
        --LE;

      return S.substr(2, LE - 1);
    }

    return "";
  }

  void EraseLast(std::stringstream &SR) const {
    SR.seekp(-1, std::ios_base::end);
    SR << u8'\0';
  }

  void EraseLastIfBlank(std::stringstream &SR) const {
    // Erase trailing blank characters (' ') from stringstream.
    // Extremely inefficient.
    if (SR.good()) {
      std::string T = SR.str();
      T.erase(T.find_last_not_of(" \r\t\n") + 1);
      SR.str("");
      SR.clear();
      SR << T;
    }
  }

  void Backtrack(std::stringstream &SR, int32_t N = 1) const {
    assert(N > int32_t(0) && "Negative or zero backtrack!");
    if (N > 0)
      SR.seekp(N * -1, SR.cur);
  }

  unsigned GetIdentifierIndex(const std::string &IS) const {
    std::string::size_type X = IS.find_last_of(':');
    if (X != std::string::npos)
      return static_cast<unsigned>(std::stoi(IS.substr(X + 1)));

    X = IS.find_last_of('[');
    std::string::size_type Y = IS.find_last_of('[');
    if (X != std::string::npos && Y != std::string::npos) {
      std::string IXS = IS.substr(X + 1, Y - X - 1);
      return static_cast<unsigned>(std::stoi(IXS));
    }

    return static_cast<unsigned>(~0x0);
  }

  bool IsIndexed(const std::string &S) const {
    return S.find('[') != std::string::npos && S.find(']') != std::string::npos;
  }

  std::string GetIdentifierBase(const std::string &IS) const {
    if (ASTStringUtils::IsIndexed(IS)) {
      std::string::size_type X = IS.find_first_of('[');
      if (X != std::string::npos)
        return IS[0] == '%' ? IS.substr(1, X - 1) : IS.substr(0, X);
    }

    return IS;
  }

  bool IsMangled(const std::string &S) const {
    return S.length() >= 3U && S[0] == '_' && S[1] == 'Q';
  }

  bool IsReservedSuffix(const std::string &S) const {
    if (S.length()) {
      std::string::size_type D = S.find_last_of(u8'.');
      if (D != std::string::npos) {
        std::string F = S.substr(D);
        return F == ".creal" || F == ".cimag" || F == ".freq" ||
               F == ".frequency" || F == ".phase" || F == ".time";
      }
    }

    return false;
  }

  std::string GetReservedSuffix(const std::string &S) const {
    if (S.length()) {
      std::string::size_type D = S.find_last_of(u8'.');
      if (D != std::string::npos)
        return S.substr(D);
    }

    return ES;
  }

  std::string GetReservedBase(const std::string &S) const {
    if (S.length()) {
      std::string::size_type D = S.find_last_of(u8'.');
      if (D != std::string::npos)
        return S.substr(0, D);
    }

    return ES;
  }

  bool IsOpenPulseFramePhase(const std::string &S) const {
    if (S.length()) {
      std::string::size_type D = S.find_last_of(u8'.');
      if (D != std::string::npos && D < S.length())
        return S.substr(D + 1) == u8"phase";
    }

    return false;
  }

  bool IsOpenPulseFrameTime(const std::string &S) const {
    if (S.length()) {
      std::string::size_type D = S.find_last_of(u8'.');
      if (D != std::string::npos && D < S.length())
        return S.substr(D + 1) == u8"time";
    }

    return false;
  }

  bool IsOpenPulseFrameFrequency(const std::string &S) const {
    if (S.length()) {
      std::string::size_type D = S.find_last_of(u8'.');
      if (D != std::string::npos && D < S.length()) {
        std::string SS = S.substr(D + 1);
        return SS == u8"freq" || SS == u8"frequency";
      }
    }

    return false;
  }

  bool IsComplexCReal(const std::string &S) const {
    if (S.length()) {
      std::string::size_type D = S.find_last_of(u8'.');
      if (D != std::string::npos && D < S.length())
        return S.substr(D + 1) == u8"creal";
    }

    return false;
  }

  bool IsComplexCImag(const std::string &S) const {
    if (S.length()) {
      std::string::size_type D = S.find_last_of(u8'.');
      if (D != std::string::npos && D < S.length())
        return S.substr(D + 1) == u8"cimag";
    }

    return false;
  }

  bool IsOpenPulseFrameReservedSuffix(const std::string &S) const {
    if (S.length()) {
      std::string::size_type D = S.find_last_of(u8'.');
      if (D != std::string::npos) {
        std::string F = S.substr(D);
        return F == ".freq" || F == ".frequency" || F == ".phase" ||
               F == ".time";
      }
    }

    return false;
  }

  bool IsMPComplexReservedSuffix(const std::string &S) const {
    if (S.length()) {
      std::string::size_type D = S.find_last_of(u8'.');
      if (D != std::string::npos) {
        std::string F = S.substr(D);
        return F == ".creal" || F == ".cimag";
      }
    }

    return false;
  }

  std::string GetDefcalBaseName(const std::string &S) const {
    if (IsMangled(S)) {
      const char *NP = S.c_str();
      NP += 2U;

      if (NP[0] == 'D' && NP[1] == 'C' && std::isdigit(NP[2]))
        NP += 2U;
      else if (NP[0] == 'D' && NP[1] == 'R' && std::isdigit(NP[2]))
        NP += 2U;
      else if (NP[0] == 'D' && NP[1] == 'M' && NP[2] == 'C' &&
               std::isdigit(NP[3]))
        NP += 3U;
      else if (NP[0] == 'D' && NP[1] == 'M' && NP[2] == 'R' &&
               std::isdigit(NP[3]))
        NP += 3U;
      else if (NP[0] == 'D' && NP[1] == 'M' && std::isdigit(NP[2]))
        NP += 2U;
      else if (NP[0] == 'D' && std::isdigit(NP[1]))
        NP += 1U;

      std::stringstream DS;
      DS.write(NP, StringToUnsigned<uint32_t>(NP));
      return DS.str();
    }

    return S;
  }

  std::string IndexedIdentifierToQCElement(const std::string &S) const {
    if (IsIndexed(S)) {
      std::stringstream QSS;
      QSS << GetIdentifierBase(S).c_str() << ':' << GetIdentifierIndex(S);
      return QSS.str();
    }

    return std::string();
  }

  std::string GetGateQubitParamName(const std::string &S) const {
    if (S.find("ast-gate-qubit-param-") != std::string::npos) {
      std::string::size_type E = S.find_last_of('-');
      if (E != std::string::npos) {
        std::string QS = S.substr(std::strlen("ast-gate-qubit-param-"), E);
        E = QS.find_last_of('-');
        return QS.substr(0, E);
      }
    }

    return "";
  }

  bool IsIndexedQubit(const std::string &S) const {
    if (S.empty())
      return false;

    return S.find_last_of(':') != std::string::npos;
  }

  std::string GetQubitIndex(const std::string &S) const {
    if (S.empty())
      return S;

    std::string::size_type P = S.find_last_of(':');
    if (P != std::string::npos)
      return S.substr(P + 1);

    return "";
  }

  std::string GetBaseQubitName(const std::string &S) const {
    if (S.empty())
      return S;

    std::string::size_type P = S.find_last_of(':');
    if (P != std::string::npos)
      return S[0] == '%' ? S.substr(1, P - 1) : S.substr(0, P);

    return S[0] == '%' ? S.substr(1) : S;
  }

  std::string BracketedQubit(const std::string &S) const {
    if (IsIndexedQubit(S)) {
      std::stringstream QS;
      QS << GetBaseQubitName(S) << '[' << GetQubitIndex(S) << ']';
      return QS.str();
    }

    return "";
  }

  template <typename __Type>
  std::string ToBinary(__Type X) const {
    static_assert(std::is_integral<__Type>::value,
                  "Attempting bitwise conversion of non-integral type!");
    std::stringstream B;
    for (unsigned I = 0; I < CHAR_BIT * sizeof(__Type); ++I)
      B << ((X & (1U << I)) ? '1' : '0');

    return B.str();
  }

  bool IsQCElement(const std::string &S) const {
    return ((S[0] == '%' || S[0] == '$') && (S.find(':') != std::string::npos));
  }

  bool IsBoundQubit(const std::string &S) const {
    return S.length() && S[0] == '$';
  }

  bool IsBinary(const std::string &S) const {
    return !S.empty() && S[0] == '0' && (S[1] == 'b' || S[1] == 'B');
  }

  bool IsHex(const std::string &S) const {
    return !S.empty() && S[0] == '0' && (S[1] == 'x' || S[1] == 'X');
  }

  bool IsQuoted(const std::string &S) const {
    return !S.empty() && (S[0] == '"' || S[0] == '\'');
  }

  bool ValidateBinary(const std::string &S) const {
    if (S.empty())
      return false;

    if (S[0] == '0' && S[1] == 'b') {
      for (unsigned I = 2; I < S.length(); ++I)
        if (S[I] != '0' && S[I] != '1')
          return false;

      return true;
    }

    for (unsigned I = 0; I < S.length(); ++I)
      if (S[I] != '0' && S[I] != '1')
        return false;

    return true;
  }

  unsigned GetDuration(const std::string &S) const {
    if (S.empty())
      return static_cast<unsigned>(~0x0);

    unsigned I = 0;
    const char *P = S.c_str();

    while (*P && !isdigit(*P)) {
      ++P;
      ++I;
    }

    return static_cast<unsigned>(std::stoi(S.substr(I)));
  }

  const std::string &EmptyString() const { return ES; }

  unsigned CountNewlines(const std::string &TS) const {
    return std::count_if(TS.begin(), TS.end(), [](unsigned I) {
      return I == u8'\n' || I == u8'\r' || I == 0x0a;
    });
  }

  unsigned CountNewlines(const std::string_view &TS) const {
    return std::count_if(TS.begin(), TS.end(), [](unsigned I) {
      return I == u8'\n' || I == u8'\r' || I == 0x0a;
    });
  }

  bool IsValidUTF8(const std::string &S) const {
    if (S.empty())
      return true;

    const unsigned char *B = reinterpret_cast<const unsigned char *>(S.c_str());
    unsigned CP;
    unsigned N;

    while (*B != 0x00) {
      if ((*B & 0x80) == 0x00) {
        // U+0000 to U+007f
        CP = (*B & 0x7f);
        N = 1;
      } else if ((*B & 0xe0) == 0xc0) {
        // U+0080 to U+07ff
        CP = (*B & 0x1f);
        N = 2;
      } else if ((*B & 0xf0) == 0xe0) {
        // U+0800 to U+ffff
        CP = (*B & 0x0f);
        N = 3;
      } else if ((*B & 0xf8) == 0xf0) {
        // U+10000 to U+10ffff
        CP = (*B & 0x07);
        N = 4;
      } else
        return false;

      B += 1;

      for (unsigned I = 1; I < N; ++I) {
        if ((*B & 0xc0) != 0x80)
          return false;

        CP = (CP << 6) | (*B & 0x3f);
        B += 1;
      }

      if ((CP > 0x10ffff) || ((CP >= 0xd800) && (CP <= 0xdfff)) ||
          ((CP <= 0x007f) && (N != 1U)) ||
          ((CP >= 0x0080) && (CP <= 0x07ff) && (N != 2U)) ||
          ((CP >= 0x0800) && (CP <= 0xffff) && (N != 3U)) ||
          ((CP >= 0x10000) && (CP <= 0x1fffff) && (N != 4U)))
        return false;
    }

    return true;
  }

  size_t UTF8Len(const char *US) const {
    size_t R = 0UL;
    const uint8_t *P = reinterpret_cast<const uint8_t *>(US);

    if (P && *P)
      while (*P)
        R += ((*P++ & 0xc0) != 0x80);

    return R;
  }

  size_t UTF8Len(const std::string &US) const {
    size_t R = 0UL;
    const uint8_t *P = reinterpret_cast<const uint8_t *>(US.data());

    if (P && *P)
      while (*P)
        R += ((*P++ & 0xc0) != 0x80);

    return R;
  }

  std::string EncodeUTF8(const std::wstring &W) const {
    try {
      return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(W);
    } catch (const std::range_error &E) {
      (void)E;
    } catch (...) {
    }

    return std::string();
  }

  std::string EncodeUTF16(const std::wstring &W) const {
    try {
      return std::wstring_convert<std::codecvt_utf16<wchar_t>>().to_bytes(W);
    } catch (const std::range_error &E) {
      (void)E;
    } catch (...) {
    }

    return std::string();
  }

  std::wstring DecodeUTF8(const std::string &S) const {
    try {
      return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(S);
    } catch (const std::range_error &E) {
      (void)E;
    } catch (...) {
    }

    return std::wstring();
  }

  std::wstring DecodeUTF16(const std::string &S) const {
    try {
      return std::wstring_convert<std::codecvt_utf16<wchar_t>>().from_bytes(S);
    } catch (const std::range_error &E) {
      (void)E;
    } catch (...) {
    }

    return std::wstring();
  }
};

} // namespace QASM

#endif // __QASM_AST_STRING_UTILS_H
