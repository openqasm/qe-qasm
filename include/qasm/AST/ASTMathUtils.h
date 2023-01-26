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

#ifndef __QASM_AST_MATH_UTILS_H
#define __QASM_AST_MATH_UTILS_H

#include <vector>
#include <limits>
#include <type_traits>

namespace QASM {

class ASTMathUtils {
private:
  static ASTMathUtils MU;

protected:
  ASTMathUtils() = default;

public:
  static ASTMathUtils& Instance() {
    return MU;
  }

  ~ASTMathUtils() = default;

  template<typename __Type>
  __Type BoolVectorToIntegral(const std::vector<bool>& BV) const {
    static_assert(std::is_integral<__Type>::value,
                  "type is not an integral type!");
    __Type R = 0;

    for (unsigned I = 0; I < BV.size(); ++I) {
      R |= BV[I] << I;
    }

    return R;
  }

  template<typename __Type>
  unsigned popcount(__Type X) noexcept {
    static_assert(!std::is_floating_point<__Type>::value,
                  "attempting popcount of floating-point type!");

    unsigned R = 0U;

#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC ivdep
#elif defined(__clang__)
#pragma clang loop vectorize(enable)
#endif

    for (unsigned I = 0; I < std::numeric_limits<__Type>::digits; ++I)
      R += (X & (1UL << I)) != 0;

    return R;
  }

  template<typename __Type>
  __Type rotl(__Type X, int S) noexcept {
    static_assert(!std::is_floating_point<__Type>::value,
                  "attempting rotl of floating-point type!");
    if (std::is_integral<__Type>::value)
      static_assert(!std::is_signed<__Type>::value,
                    "attempting rotl of signed integer type!");

    if (std::is_integral<__Type>::value) {
      if (S == std::numeric_limits<__Type>::digits || S == 0) {
        return X;
      } else if (S < 0) {
        return rotr(X, -S);
      } else {
        return (X << S) | (X >> (std::numeric_limits<__Type>::digits - S));
      }
    } else {
      if (S == static_cast<int>(X.size()) || S == 0) {
        return X;
      } else if (S < 0) {
        return rotr(X, -S);
      } else {
        __Type T = X;
        X <<= S;
        X |= T >> (std::numeric_limits<__Type>::digits - S);
      }
    }

    return X;
  }

  template<typename __Type>
  __Type rotr(__Type X, int S) noexcept {
    static_assert(!std::is_floating_point<__Type>::value,
                  "attempting rotr of floating-point type!");
    if (std::is_integral<__Type>::value)
      static_assert(!std::is_signed<__Type>::value,
                    "attempting rotr of signed integer type!");

    if (std::is_integral<__Type>::value) {
      if (S == std::numeric_limits<__Type>::digits || S == 0) {
        return X;
      } else if (S < 0) {
        return rotl(X, -S);
      } else {
        return (X >> S) | (X << (std::numeric_limits<__Type>::digits - S));
      }
    } else {
      if (S == static_cast<int>(X.size()) || S == 0) {
        return X;
      } else if (S < 0) {
        return rotl(X, -S);
      } else {
        __Type T = X;
        X >>= S;
        X |= T << (std::numeric_limits<__Type>::digits - S);
      }

      return X;
    }
  }

  template<typename __Type>
  inline bool IsPowerOfTwo(__Type X) noexcept {
    static_assert(std::is_integral<__Type>::value,
                  "type is not an integral type!");
    return (X > 0) && (X & (X - 1)) == 0;
  }

  template<typename __Type>
  inline bool IsOdd(__Type X) noexcept {
    static_assert(std::is_integral<__Type>::value,
                  "attempting odd test of non-integer type!");
    return (X & 0x01) != 0;
  }

  template<typename __Type>
  inline bool IsEven(__Type X) noexcept {
    static_assert(std::is_integral<__Type>::value,
                  "attempting even test of non-integer type!");
    return (X & 0x01) == 0;
  }

  // boost::hash_combine.
  template<typename __Type>
  static inline void hash_combine(uint64_t& S, const __Type& V) {
    S ^= V + 0x9e3779b9 + (S << 6) + (S >> 2);
  }

  template<typename __Type>
  struct vector_hash {
    struct S {
      uint32_t Y;
      uint32_t Z;
    };

    union {
      uint64_t X;
      S U;
    };

    inline uint64_t operator()(const std::vector<__Type>& V) {
      static_assert(std::is_integral<__Type>::value,
                    "vector type is not an integral type!");

      uint64_t R = 0UL;
      //      X = reinterpret_cast<uint64_t>(reinterpret_cast<uintptr_t>(
      //          reinterpret_cast<const void*>(&V)));
      X = reinterpret_cast<uint64_t>(&V);

      for (size_t I = 0; I < V.size(); ++I)
        hash_combine<__Type>(R, V[I]);

#if defined(HASH_COMBINE_VECTOR_ADDRESS)
      hash_combine<__Type>(R, static_cast<__Type>(U.Z));
      hash_combine<__Type>(R, static_cast<__Type>(U.Y));
#endif

      return R;
    }
  };
};

} // namespace QASM

#endif // __QASM_AST_MATH_UTILS_H

