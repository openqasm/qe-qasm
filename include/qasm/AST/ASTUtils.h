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

#ifndef __QASM_AST_UTILS_H
#define __QASM_AST_UTILS_H

#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTTypes.h>

#include <limits>

namespace QASM {

template <typename __Type>
inline bool IsOdd(__Type X) noexcept {
  static_assert(std::is_integral<__Type>::value,
                "attempting odd test of non-integer type!");
  return (X & 0x01) != 0;
}

template <typename __Type>
inline bool IsEven(__Type X) noexcept {
  static_assert(std::is_integral<__Type>::value,
                "attempting even test of non-integer type!");
  return (X & 0x01) == 0;
}

class ASTUtils {
private:
  static ASTUtils Utils;

protected:
  ASTUtils() = default;

public:
  static ASTUtils &Instance() { return Utils; }

  virtual ~ASTUtils() = default;

  unsigned GetUnsignedValue(const ASTIdentifierNode *Id) const;
  unsigned GetUnsignedValue(const ASTIntNode *I) const;
  bool GetBooleanValue(const ASTIntNode *I) const;
  bool GetBooleanValue(const ASTMPIntegerNode *MPI) const;
  bool GetVariantIntegerValue(
      const ASTIdentifierNode *IId,
      std::variant<const ASTIntNode *, const ASTMPIntegerNode *,
                   const ASTCBitNode *> &IIV) const;

  unsigned AdjustZeroBitWidth(const ASTIdentifierNode *Id,
                              unsigned NumBits) const;

  bool IsQubitType(ASTType Ty) const {
    return Ty == ASTTypeQubit || Ty == ASTTypeQubitContainer ||
           Ty == ASTTypeQubitContainerAlias;
  }

  bool IsQubitParamType(ASTType Ty) const {
    return IsQubitType(Ty) || Ty == ASTTypeGateQubitParam;
  }

  bool IsAngleType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeAngle:
    case ASTTypeLambdaAngle:
    case ASTTypePhiAngle:
    case ASTTypeThetaAngle:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsReservedAngleType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypePiAngle:
    case ASTTypeTauAngle:
    case ASTTypeEulerAngle:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsScalarNumericType(ASTType Ty) const {
    return Ty == ASTTypeInt || Ty == ASTTypeUInt || Ty == ASTTypeFloat ||
           Ty == ASTTypeDouble || Ty == ASTTypeLongDouble ||
           Ty == ASTTypeMPInteger || Ty == ASTTypeMPUInteger ||
           Ty == ASTTypeMPDecimal;
  }

  bool IsNumericType(ASTType Ty) const {
    return IsScalarNumericType(Ty) || Ty == ASTTypeMPComplex;
  }

  bool IsIntegralType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeBool:
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsUnpromotedIntegralType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeInt:
    case ASTTypeUInt:
    case ASTTypeMPInteger:
    case ASTTypeMPUInteger:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  ASTType GetArrayElementType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeCBitArray:
      return ASTTypeBitset;
      break;
    case ASTTypeQubitArray:
      return ASTTypeQubit;
      break;
    case ASTTypeAngleArray:
      return ASTTypeAngle;
      break;
    case ASTTypeBoolArray:
      return ASTTypeBool;
      break;
    case ASTTypeIntArray:
      return ASTTypeInt;
      break;
    case ASTTypeMPIntegerArray:
      return ASTTypeMPInteger;
      break;
    case ASTTypeFloatArray:
      return ASTTypeFloat;
      break;
    case ASTTypeMPDecimalArray:
      return ASTTypeMPDecimal;
      break;
    case ASTTypeMPComplexArray:
      return ASTTypeMPComplex;
      break;
    case ASTTypeDurationArray:
      return ASTTypeDuration;
      break;
    case ASTTypeOpenPulseFrameArray:
      return ASTTypeOpenPulseFrame;
      break;
    case ASTTypeOpenPulsePortArray:
      return ASTTypeOpenPulsePort;
      break;
    case ASTTypeOpenPulseWaveformArray:
      return ASTTypeOpenPulseWaveform;
      break;
    default:
      break;
    }

    return ASTTypeUndefined;
  }

  bool IsLeftParenType(ASTOpType OTy) const {
    return OTy == ASTOpTypeUnaryLeftFold || OTy == ASTOpTypeBinaryLeftFold;
  }

  bool IsRightParenType(ASTOpType OTy) const {
    return OTy == ASTOpTypeUnaryRightFold || OTy == ASTOpTypeBinaryRightFold;
  }
};

} // namespace QASM

#endif // __QASM_AST_UTILS_H
