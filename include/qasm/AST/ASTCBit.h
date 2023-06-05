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

#ifndef __QASM_AST_CBIT_H
#define __QASM_AST_CBIT_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTMathUtils.h>

#include <vector>
#include <sstream>
#include <bitset>
#include <climits>
#include <cassert>

namespace QASM {

class ASTCastExpressionNode;
class ASTImplicitConversionNode;

class ASTCBitNode : public ASTExpressionNode {
  friend class ASTCBitNodeBuilder;

protected:
  mutable std::vector<bool> BV;
  size_t NR;
  std::string SR;
  size_t SZ;
  const ASTGateQOpNode* QOP;
  union {
    const ASTBinaryOpNode* BOP;
    const ASTUnaryOpNode* UOP;
    const ASTCastExpressionNode* CST;
    const ASTImplicitConversionNode* ICX;
  };

  ASTType OTy;
  mutable bool P;
  mutable bool NBC;

private:
  ASTCBitNode() = delete;

protected:
  ASTCBitNode(const ASTIdentifierNode* Id, const std::string& ERM)
  : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
  NR(0UL), SR(ERM), SZ(0UL), QOP(nullptr), BOP(nullptr), OTy(ASTTypeUndefined),
  P(false), NBC(false) { }

public:
  using vector_type = std::vector<bool>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;
  using reference = typename vector_type::reference;
  using const_reference = typename vector_type::const_reference;

  static const unsigned BitBits = 1U;
  static const unsigned CBitBits = 1U;

public:
  ASTCBitNode(const ASTIdentifierNode* Id, std::size_t S,
              std::size_t Bitmask = 0UL)
  : ASTExpressionNode(Id, ASTTypeBitset), BV(), NR(Bitmask),
  SR(), SZ(S), QOP(nullptr), BOP(nullptr), OTy(ASTTypeUndefined),
  P(false), NBC(false) {
    assert(S > 0 && "Invalid bitset of size zero!");

    std::bitset<CHAR_BIT * sizeof(Bitmask)> BS = Bitmask;
    SR = BS.to_string();

    for (size_t I = 0; I < S; ++I)
      BV.push_back(BS[I]);
  }

  ASTCBitNode(const ASTIdentifierNode* Id, std::size_t S,
              const std::string& Bitmask)
  : ASTExpressionNode(Id, ASTTypeBitset), BV(),
  NR(static_cast<size_t>(~0x0)), SR(Bitmask), SZ(S),
  QOP(nullptr), BOP(nullptr), OTy(ASTTypeUndefined),
  P(false), NBC(false) {
    assert(S > 0 && "Invalid bitset of size zero!");

    if (Bitmask.length()) {
      BV.reserve(std::max(Bitmask.length(), S));
      if (S <= 64)
        NR = std::stoul(Bitmask, 0, 2);

      for (size_t I = 0; I < Bitmask.length(); ++I) {
        BV.emplace(BV.end(), Bitmask[I] == u8'1');
      }

      if (S > Bitmask.length()) {
        for (size_t I = Bitmask.length(); I < S; ++I) {
          BV.insert(BV.begin(), false);
        }
      }
    } else {
      SR = std::string(S, '0');
      NR = 0UL;
    }
  }

  ASTCBitNode(const ASTIdentifierNode* Id, size_t S,
              const ASTCastExpressionNode* C)
  : ASTExpressionNode(Id, ASTTypeBitset), BV(),
  NR(static_cast<size_t>(~0x0)), SR(), SZ(S),
  QOP(nullptr), CST(C), OTy(ASTTypeCast),
  P(false), NBC(true) {
    assert(S > 0 && "Invalid bitset of size zero!");
    SR.assign(S, u8'1');
    BV.assign(S, true);
  }

  ASTCBitNode(const ASTIdentifierNode* Id, size_t S,
              const ASTImplicitConversionNode* IC)
  : ASTExpressionNode(Id, ASTTypeBitset), BV(),
  NR(static_cast<size_t>(~0x0)), SR(), SZ(S),
  QOP(nullptr), ICX(IC), OTy(ASTTypeImplicitConversion),
  P(false), NBC(true) {
    assert(S > 0 && "Invalid bitset of size zero!");
    SR.assign(S, u8'1');
    BV.assign(S, true);
  }

  ASTCBitNode(const ASTIdentifierNode* Id, size_t S, const ASTBinaryOpNode* B)
  : ASTExpressionNode(Id, ASTTypeBitset), BV(),
  NR(static_cast<size_t>(~0x0)), SR(), SZ(S),
  QOP(nullptr), BOP(B), OTy(B->GetASTType()),
  P(false), NBC(true) {
    assert(S > 0 && "Invalid bitset of size zero!");
    SR.assign(S, u8'1');
    BV.assign(S, true);
  }

  ASTCBitNode(const ASTIdentifierNode* Id, size_t S, const ASTUnaryOpNode* U)
  : ASTExpressionNode(Id, ASTTypeBitset), BV(),
  NR(static_cast<size_t>(~0x0)), SR(), SZ(S),
  QOP(nullptr), UOP(U), OTy(U->GetASTType()),
  P(false), NBC(true) {
    assert(S > 0 && "Invalid bitset of size zero!");
    SR.assign(S, u8'1');
    BV.assign(S, true);
  }

  virtual ~ASTCBitNode() = default;

  virtual std::size_t Size() const {
    return BV.size();
  }

  virtual std::size_t GetBits() const {
    return Size();
  }

  virtual unsigned Popcount() const {
    unsigned R = 0U;
    for (const_iterator I = BV.begin(); I != BV.end(); ++I)
      R += *I;

    return R;
  }

  virtual void SetAllTo(bool BM) {
    for (iterator I = BV.begin(); I != BV.end(); ++I)
      *I = BM;
  }

  virtual void Flip() {
    BV.flip();
  }

  virtual const std::string& AsString() const {
    return SR;
  }

  virtual bool ResolveCast();

  virtual void Rotl(int SH) {
    if (SZ == 0) {
      return;
    } else if (SZ && SZ <= 64) {
      std::bitset<64> BS(SR);
      std::bitset<64> BR = ASTMathUtils::Instance().rotl(BS, SH);
      size_t VL = BV.size();
      BV.clear();

      for (size_t I = 0; I < VL; ++I)
        BV.push_back(BR[I]);

      NR = BR.to_ullong();
      SR = BR.to_string();
    } else {
      NR = static_cast<uint64_t>(~0x0);
      size_t VL = BV.size();
      BV.clear();

      if (SZ > 64 && SZ <= 128) {
        std::bitset<128> BS = NR;
        std::bitset<128> BR = ASTMathUtils::Instance().rotl(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      } else if (SZ > 128 && SZ <= 256) {
        std::bitset<256> BS = NR;
        std::bitset<256> BR = ASTMathUtils::Instance().rotl(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      } else if (SZ > 256 && SZ <= 512) {
        std::bitset<512> BS = NR;
        std::bitset<512> BR = ASTMathUtils::Instance().rotl(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      } else if (SZ > 512 && SZ <= 1024) {
        std::bitset<1024> BS = NR;
        std::bitset<1024> BR = ASTMathUtils::Instance().rotl(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      } else if (SZ > 1024 && SZ <= 2048) {
        std::bitset<2048> BS = NR;
        std::bitset<2048> BR = ASTMathUtils::Instance().rotl(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      } else if (SZ > 2048 && SZ <= 4096) {
        std::bitset<4096> BS = NR;
        std::bitset<4096> BR = ASTMathUtils::Instance().rotl(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      } else {
        std::bitset<8192> BS = NR;
        std::bitset<8192> BR = ASTMathUtils::Instance().rotl(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      }
    }
  }

  virtual void Rotr(int SH) {
    if (SZ == 0) {
      return;
    } else if (SZ && SZ <= 64) {
      std::bitset<64> BS(SR);
      std::bitset<64> BR = ASTMathUtils::Instance().rotr(BS, SH);
      size_t VL = BV.size();
      BV.clear();

      for (size_t I = 0; I < VL; ++I)
        BV.push_back(BR[I]);

      NR = BR.to_ullong();
      SR = BR.to_string();
    } else {
      NR = static_cast<uint64_t>(~0x0);
      size_t VL = BV.size();
      BV.clear();

      if (SZ > 64 && SZ <= 128) {
        std::bitset<128> BS = NR;
        std::bitset<128> BR = ASTMathUtils::Instance().rotr(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      } else if (SZ > 128 && SZ <= 256) {
        std::bitset<256> BS = NR;
        std::bitset<256> BR = ASTMathUtils::Instance().rotr(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      } else if (SZ > 256 && SZ <= 512) {
        std::bitset<512> BS = NR;
        std::bitset<512> BR = ASTMathUtils::Instance().rotr(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      } else if (SZ > 512 && SZ <= 1024) {
        std::bitset<1024> BS = NR;
        std::bitset<1024> BR = ASTMathUtils::Instance().rotr(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      } else if (SZ > 1024 && SZ <= 2048) {
        std::bitset<2048> BS = NR;
        std::bitset<2048> BR = ASTMathUtils::Instance().rotr(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      } else if (SZ > 2048 && SZ <= 4096) {
        std::bitset<4096> BS = NR;
        std::bitset<4096> BR = ASTMathUtils::Instance().rotr(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      } else {
        std::bitset<8192> BS = NR;
        std::bitset<8192> BR = ASTMathUtils::Instance().rotr(BS, SH);

        for (size_t I = 0; I < VL; ++I)
          BV.push_back(BR[I]);

        SR = BR.to_string();
      }
    }
  }

  virtual bool IsSet(unsigned Index) const {
    assert(Index < BV.size() && "Index is out-of-range!");
    return BV[Index];
  }

  virtual bool AsBool() const {
    unsigned U = 0U;
    for (const_iterator I = BV.begin(); I != BV.end(); ++I)
      U |= *I;

    return static_cast<bool>(U);
  }

  virtual const std::vector<bool>& AsVector() const {
    return BV;
  }

  const ASTIdentifierNode* GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual void SetGateQOp(const ASTGateQOpNode* QOp) {
    QOP = QOp;
  }

  virtual const ASTGateQOpNode* GetGateQOp() const {
    return QOP;
  }

  virtual void SetBinaryOp(const ASTBinaryOpNode* BOp) {
    BOP = BOp;
    OTy = BOp->GetASTType();
    NBC = true;
  }

  virtual void SetUnaryOp(const ASTUnaryOpNode* UOp) {
    UOP = UOp;
    OTy = UOp->GetASTType();
    NBC = true;
  }

  virtual void SetCastExpression(const ASTCastExpressionNode* CX) {
    CST = CX;
    OTy = ASTTypeCast;
    NBC = true;
  }

  virtual void SetImplicitConversion(const ASTImplicitConversionNode* IC) {
    ICX = IC;
    OTy = ASTTypeImplicitConversion;
    NBC = true;
  }

  virtual const ASTBinaryOpNode* GetBinaryOp() const {
    return OTy == ASTTypeBinaryOp ? BOP : nullptr;
  }

  virtual const ASTUnaryOpNode* GetUnaryOp() const {
    return OTy == ASTTypeUnaryOp ? UOP : nullptr;
  }

  virtual const ASTCastExpressionNode* GetCastExpression() const {
    return OTy == ASTTypeCast ? CST : nullptr;
  }

  virtual const ASTImplicitConversionNode* GetImplicitConversion() const {
    return OTy == ASTTypeImplicitConversion ? ICX : nullptr;
  }

  virtual bool IsBinaryOp() const {
    return OTy == ASTTypeBinaryOp && BOP != nullptr;
  }

  virtual bool IsUnaryOp() const {
    return OTy == ASTTypeUnaryOp && UOP != nullptr;
  }

  virtual bool IsCastExpression() const {
    return OTy == ASTTypeCast && CST != nullptr;
  }

  virtual bool IsImplicitConversion() const {
    return OTy == ASTTypeImplicitConversion && ICX != nullptr;
  }

  virtual bool IsGateQOp() const {
    return QOP != nullptr;
  }

  virtual bool HasEvalOp() const {
    return OTy != ASTTypeUndefined;
  }

  virtual bool NeedsBitcast() const {
    return NBC;
  }

  iterator begin() {
    return BV.begin();
  }

  const_iterator begin() const {
    return BV.begin();
  }

  iterator end() {
    return BV.end();
  }

  const_iterator end() const {
    return BV.end();
  }

  reference operator[](unsigned Index) {
    assert(Index < BV.size() && "Index is out-of-range!");
    return BV[Index];
  }

  const_reference operator[](unsigned Index) const {
    assert(Index < BV.size() && "Index is out-of-range!");
    return BV[Index];
  }

  virtual ASTType GetASTType() const override {
    return ASTTypeBitset;
  }

  virtual void Mangle() override;

  virtual bool IsError() const override {
    return ASTExpressionNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTCBitNode* ExpressionError(const ASTIdentifierNode* Id,
                                      const std::string& ERM) {
    return new ASTCBitNode(Id, ERM);
  }

  static ASTCBitNode* ExpressionError(const std::string& ERM) {
    return new ASTCBitNode(ASTIdentifierNode::Bitset.Clone(), ERM);
  }

  virtual void print() const override {
    if (P) {
      std::cout << "<CBit>" << std::endl;
      Ident->print();
      std::cout << "<Bitmask>";
      for (ASTCBitNode::const_iterator I = BV.begin();
           I != BV.end(); ++I)
        std::cout << std::noboolalpha << *I;
      std::cout << "</Bitmask>" << std::endl;

      if (QOP)
        QOP->print();

      std::cout << "</CBit>" << std::endl;
      return;
    }

    std::cout << "<CBit>" << std::endl;
    Ident->print();
    std::cout << "<Bitmask>";
    for (ASTCBitNode::const_iterator I = BV.begin();
         I != BV.end(); ++I)
      std::cout << std::noboolalpha << *I;
    std::cout << "</Bitmask>" << std::endl;

    if (QOP)
      QOP->print();

    switch (OTy) {
    case ASTTypeBinaryOp:
      P = true;
      BOP->print();
      P = false;
      break;
    case ASTTypeUnaryOp:
      P = true;
      UOP->print();
      P = false;
      break;
    default:
      break;
    }

    std::cout << "</CBit>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM

#endif // __QASM_AST_CBIT_H

