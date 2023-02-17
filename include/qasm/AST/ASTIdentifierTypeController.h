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

#ifndef __QASM_AST_IDENTIFIER_TYPE_CONTROLLER_H
#define __QASM_AST_IDENTIFIER_TYPE_CONTROLLER_H

#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTUtils.h>

#include <iostream>

namespace QASM {

class ASTIdentifierTypeController {
private:
  static ASTIdentifierTypeController ITC;
  static std::vector<ASTType> TV;

  static ASTType CT;
  static ASTType PT;
  static ASTType NT;
  static bool IA;

  // Stop Context Requested.
  static bool SCR;

  // Pending Stop Context.
  static bool PSC;

  uint32_t LB;
  uint32_t RB;
  uint32_t LP;
  uint32_t RP;
  uint32_t LBR;
  uint32_t RBR;
  int32_t IV;
  bool IB;
  bool AL;
  bool QL;

protected:
  ASTIdentifierTypeController()
  : LB(0U), RB(0U), LP(0U), RP(0U), LBR(0U), RBR(0U), IV(0),
  IB(false), AL(false), QL(false) { }

public:
  static ASTIdentifierTypeController& Instance() {
    return ITC;
  }

  inline ASTType GetCurrentType() const {
    switch (CT) {
    case ASTTypeQubit:
      return LB == 0 && RB == 0 ? CT : CT = ASTTypeQubitContainer;
      break;
    case ASTTypeQubitArray:
      return LB == 1 && RB == 1 ? CT : CT = ASTTypeQubitArray;
      break;
    case ASTTypeCBitArray:
      return LB == 1 && RB == 1 ? CT : CT = ASTTypeCBitArray;
      break;
    case ASTTypeFloatArray:
      return LB == 1 && RB == 1 ? CT : CT = ASTTypeMPDecimalArray;
      break;
    case ASTTypeIntArray:
      return LB == 1 && RB == 1 ? CT : CT = ASTTypeMPIntegerArray;
      break;
    default:
      break;
    }

    return CT;
  }

  inline void StartContextRecord(ASTType Ty) {
    if (Ty == ASTTypeGate || Ty == ASTTypeDefcal) {
      SCR = false;
      TV.clear();
      TV.push_back(Ty);
    }
  }

  inline void StopContextRecord() {
    SCR = true;
  }

  inline void SetPendingStop() {
    PSC = true;
    SCR = false;
  }

  inline bool StopRequested() const {
    return SCR;
  }

  inline bool PendingStop() const {
    return PSC;
  }

  inline void AddContextRecord(ASTType Ty) {
    if (!TV.empty())
      TV.push_back(Ty);
  }

  inline bool InContextRecord() const {
    return !TV.empty();
  }

  inline ASTType GetContextRecordType() const {
    return TV.size() >= 1 ? TV.front() : ASTTypeUndefined;
  }

  inline ASTType GetContextRecordName() const {
    return TV.size() >= 2 ? TV.at(1) : ASTTypeUndefined;
  }

  inline ASTType GetContextRecord(unsigned IX) const {
    return IX < TV.size() ? TV.at(IX) : ASTTypeUndefined;
  }

  inline unsigned GetContextRecordSize() const {
    return static_cast<unsigned>(TV.size());
  }

  inline void ClearContextRecord() {
    TV.clear();
    SCR = false;
    PSC = false;
    LB = RB = 0;

    if (AL) {
      CT = ASTTypeAngle;
      PT = NT = ASTTypeUndefined;
    } else {
      CT = PT = NT = ASTTypeUndefined;
    }

    IA = false;
  }

  inline ASTType GetPreviousType() const {
    return PT;
  }

  inline ASTType GetNextType() const {
    return NT;
  }

  inline void TransferCurrent() {
    PT = CT;
  }

  inline void LBracket() {
    LB += 1U;

    if (PT != ASTTypeArray) {
      switch (CT) {
      case ASTTypeInt:
        CT = ASTTypeMPInteger;
        break;
      case ASTTypeFloat:
        CT = ASTTypeMPDecimal;
        break;
      case ASTTypeBitset:
        CT = ASTTypeBitset;
        break;
      case ASTTypeQubit:
        CT = ASTTypeQubitContainer;
          break;
      case ASTTypeAngle:
        CT = ASTTypeAngle;
          break;
      default:
        break;
      }
    }
  }

  inline void RBracket() {
    RB += 1U;
  }

  inline void LParen() {
    LP += 1U;
  }

  inline void RParen() {
    RP += 1U;
  }

  inline void LBrace() {
    LBR += 1U;
  }

  inline void RBrace() {
    RBR += 1U;
  }

  inline bool SeenLBrace() const {
    return LBR != 0U;
  }

  inline bool SeenRBrace() const {
    return RBR != 0U;
  }

  inline bool SeenLBracket() const {
    return LB != 0;
  }

  inline bool SeenRBracket() const {
    return RB != 0;
  }

  inline bool SeenLParen() const {
    return LP != 0U;
  }

  inline bool SeenRParen() const {
    return RP != 0U;
  }

  inline bool BracketsMatched() const {
    return RB && (LB == RB);
  }

  inline bool ParensMatched() const {
    return RP && (LP == RP);
  }

  inline bool BracesMatched() const {
    return RBR && (LBR == RBR);
  }

  inline void SetCurrentType(ASTType Ty) {
    if (Ty == ASTTypeGate || Ty == ASTTypeDefcal) {
      CT = Ty;
      QL = true;
      return;
    }

    switch (CT) {
    case ASTTypeMPInteger:
    case ASTTypeMPDecimal:
      break;
    case ASTTypeMPComplex:
      if (PT == CT) {
        CT = Ty;
      } else if ((PT != CT) && SeenLParen()) {
        PT = CT;
        CT = Ty;
      }
      break;
    default:
      if (CT != ASTTypeMPComplex) {
        PT = CT;
        CT = Ty;
      }
      break;
    }

    if (PT == ASTTypeArray) {
      switch (CT) {
      case ASTTypeInt:
        CT = ASTTypeIntArray;
        IA = true;
        break;
      case ASTTypeFloat:
        CT = ASTTypeFloatArray;
        IA = true;
        break;
      case ASTTypeMPInteger:
        CT = ASTTypeMPIntegerArray;
        IA = true;
        break;
      case ASTTypeMPDecimal:
        CT = ASTTypeMPDecimalArray;
        IA = true;
        break;
      case ASTTypeMPComplex:
        CT = ASTTypeMPComplexArray;
        IA = true;
        break;
      case ASTTypeAngle:
        CT = ASTTypeAngleArray;
        IA = true;
        break;
      case ASTTypeDuration:
        CT = ASTTypeDurationArray;
        IA = true;
        break;
      case ASTTypeBitset:
        CT = ASTTypeCBitArray;
        IA = true;
        break;
      case ASTTypeBool:
        CT = ASTTypeBoolArray;
        IA = true;
        break;
      case ASTTypeQubit:
        CT = ASTTypeQubitArray;
        IA = true;
        break;
      case ASTTypeQubitContainer:
        CT = ASTTypeQubitArray;
        IA = true;
        break;
      case ASTTypeOpenPulseFrame:
        break;
      default:
        IA = false;
        break;
      }
    } else {
      switch (PT) {
      case ASTTypeIntArray:
      case ASTTypeFloatArray:
      case ASTTypeMPIntegerArray:
      case ASTTypeMPDecimalArray:
      case ASTTypeMPComplexArray:
      case ASTTypeAngleArray:
      case ASTTypeDurationArray:
      case ASTTypeLengthArray:
      case ASTTypeCBitArray:
      case ASTTypeCBitNArray:
      case ASTTypeBoolArray:
      case ASTTypeQubitArray:
      case ASTTypeQubitNArray:
      case ASTTypeArray:
        CT = PT;
        IA = true;
        return;
        break;
      default:
        if (Ty == ASTTypeForStatement || Ty == ASTTypeWhileStatement) {
          PT = CT = Ty;
          IA = false;
        } else if (Ty == ASTTypeMPComplex) {
          PT = CT = Ty;
          IA = false;
        } else if (CT != ASTTypeMPComplex) {
          CT = Ty;
          switch (CT) {
          case ASTTypeIntArray:
          case ASTTypeFloatArray:
          case ASTTypeMPIntegerArray:
          case ASTTypeMPDecimalArray:
          case ASTTypeMPComplexArray:
          case ASTTypeAngleArray:
          case ASTTypeDurationArray:
          case ASTTypeLengthArray:
          case ASTTypeCBitArray:
          case ASTTypeCBitNArray:
          case ASTTypeBoolArray:
          case ASTTypeQubitArray:
          case ASTTypeQubitNArray:
          case ASTTypeArray:
            PT = CT;
            IA = true;
            break;
          default:
            IA = false;
            break;
          }
        }
        break;
      }
    }
  }

  inline void SetPreviousType(ASTType Ty) {
    PT = Ty;
  }

  inline void SetPreviousCurrent() {
    PT = CT;
    CT = ASTTypeUndefined;
  }

  inline void StartAngleList() {
    switch (CT) {
    case ASTTypeGate:
    case ASTTypeUGate:
    case ASTTypeCXGate:
    case ASTTypeCCXGate:
    case ASTTypeCNotGate:
    case ASTTypeHadamardGate:
    case ASTTypeDefcal:
      AL = true;
      break;
    default:
      AL = false;
      break;
    }
  }

  inline void StopAngleList() {
    LP = RP = 0U;
    AL = false;
  }

  inline void StartQubitList() {
    QL = true;
  }

  inline void StopQubitList() {
    QL = false;
  }

  inline bool InAngleList() const {
    return AL;
  }

  inline bool InQubitList() const {
    return QL;
  }

  inline void SetNextType(ASTType Ty) {
    NT = Ty;
  }

  inline void SetIndexValue(int32_t V) {
    IV = V;
    IB = true;
  }

  inline void Reset() {
    PT = CT = ASTTypeUndefined;
    IA = IB = false;
    LB = RB = 0U;
    LP = RP = 0U;
    LBR = RBR = 0U;
    IV = 0;
    AL = QL = false;
  }

  inline void ResetKeepTypes() {
    IA = IB = false;
    LB = RB = 0U;
    LP = RP = 0U;
    LBR = RBR = 0U;
    IV = 0;
    AL = QL = false;
  }

  inline uint32_t GetLBracket() const {
    return LB;
  }

  inline uint32_t GetRBracket() const {
    return RB;
  }

  inline int32_t GetIndexValue() const {
    return IV;
  }

  inline bool IsIndexed() const {
    return LB > 0 && RB > 0 && IB;
  }

  void CheckIdentifier(const ASTIdentifierNode* Id) const;

  void CheckMeasureTarget(const ASTIdentifierNode* Id) const;

  void CheckMeasureResult(const ASTIdentifierNode* Id) const;

  void CheckIdentifierType(const ASTIdentifierNode* Id, ASTType Ty) const;

  void CheckIdentifierType(const ASTIdentifierNode* Id,
                           ASTType Ty0, ASTType Ty1) const;

  void CheckIdentifierType(const ASTIdentifierNode* Id, ASTType Ty0,
                           ASTType Ty1, ASTType Ty2) const;

  void CheckIdentifierType(const ASTIdentifierNode* Id, ASTType Ty0,
                           ASTType Ty1, ASTType Ty2, ASTType Ty3) const;

  void CheckGateQubitParamType(const ASTIdentifierNode* Id) const;

  void CheckGateQubitParamType(const ASTIdentifierList& IL) const;

  void CheckIsCallable(const ASTIdentifierNode* Id) const;

  void CheckUndefinedType(const ASTIdentifierNode* Id) const;

  void CheckIsHardwareQubit(const ASTIdentifierNode* Id) const;

  bool TypeScopeIsAlwaysGlobal(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeQubit:
    case ASTTypeQubitContainer:
    case ASTTypeQubitContainerAlias:
    case ASTTypeGate:
    case ASTTypeDefcal:
    case ASTTypeKernel:
      return true;
    default:
      return false;
    }

    return false;
  }

  bool TypeScopeIsAlwaysGlobal(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");
    return TypeScopeIsAlwaysGlobal(Id->GetSymbolType());
  }

  bool TypeScopeIsAlwaysGlobal(const ASTSymbolTableEntry* STE) const;

  bool CanReadIndexedIdentifier(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeAngle:
    case ASTTypeMeasure:
    case ASTTypeReset:
    case ASTTypeBitset:
    case ASTTypeDefcalCall:
    case ASTTypeGateCall:
    case ASTTypeFunctionCall:
    case ASTTypeKernelCall:
    case ASTTypeQubitContainer:
    case ASTTypeQubitContainerAlias:
    case ASTTypeAngleArray:
    case ASTTypeBoolArray:
    case ASTTypeCBitArray:
    case ASTTypeIntArray:
    case ASTTypeMPIntegerArray:
    case ASTTypeFloatArray:
    case ASTTypeMPDecimalArray:
    case ASTTypeMPComplexArray:
    case ASTTypeDurationArray:
    case ASTTypeQubitArray:
    case ASTTypeOpenPulseFrameArray:
    case ASTTypeOpenPulsePortArray:
      return true;
      break;
    default:
      break;
    }

    return false;
  }

  bool IsGateType(ASTType Ty) const {
    switch (Ty) {
    case ASTTypeGate:
    case ASTTypeCXGate:
    case ASTTypeCCXGate:
    case ASTTypeCNotGate:
    case ASTTypeHadamardGate:
    case ASTTypeUGate:
      return true;
      break;
    default:
      return false;
      break;
    }

    return false;
  }

  bool CheckGateTypeMatch(ASTType OTy, ASTType NTy) const {
    switch (OTy) {
    case ASTTypeUGate:
      return NTy == ASTTypeUGate;
      break;
    case ASTTypeHadamardGate:
      return NTy == ASTTypeHadamardGate;
      break;
    case ASTTypeCXGate:
      return NTy == ASTTypeCXGate;
    case ASTTypeCCXGate:
      return NTy == ASTTypeCCXGate;
      break;
    case ASTTypeCNotGate:
      return NTy == ASTTypeCNotGate;
      break;
    case ASTTypeGate:
      return IsGateType(NTy);
      break;
    default:
      return false;
      break;
    }

    return false;
  }

  bool MustHaveASymbolTableEntry(ASTType Ty) const {
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
    case ASTTypeKernel:
    case ASTTypeExtern:
    case ASTTypeAngle:
      return true;
      break;
    default:
      return false;
      break;
    }

    return false;
  }

  bool CanTypeUpdate(ASTType OTy, ASTType NTy) const {
    switch (OTy) {
    case ASTTypeUndefined:
      return true;
      break;
    case ASTTypeGate:
      return IsGateType(NTy);
      break;
    default:
      return OTy == NTy;
      break;
    }

    return false;
  }

  bool IsBuiltinUGate(ASTType Ty) const {
    return Ty == ASTTypeUGate;
  }

  bool IsBuiltinUGate(const ASTIdentifierNode* Id) const {
    assert(Id && "Invalid ASTIdentifierNode argument!");

    return Id->GetName() == u8"U" &&
           IsBuiltinUGate(Id->GetSymbolType());
  }

  bool IsFunctionArgument(const ASTToken* TK,
                          const ASTIdentifierNode* Id,
                          ASTType Ty,
                          const ASTDeclarationContext* CTX) const;
};

} // namespace QASM

#endif // __QASM_AST_IDENTIFIER_TYPE_CONTROLLER_H

