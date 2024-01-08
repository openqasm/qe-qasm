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

#ifndef __QASM_AST_DELAY_H
#define __QASM_AST_DELAY_H

#include <qasm/AST/ASTDuration.h>
#include <qasm/AST/ASTStretch.h>
#include <qasm/AST/ASTTypes.h>

#include <string>

namespace QASM {

class ASTDelayNode : public ASTExpressionNode {
private:
  const ASTIdentifierNode *Id;
  union {
    mutable const void *Void;
    mutable const ASTBinaryOpNode *BOP;
    mutable const ASTUnaryOpNode *UOP;
    mutable const ASTDurationOfNode *DON;
    mutable const ASTDurationNode *DN;
    mutable const ASTStretchNode *STR;
  };

  uint64_t Duration;
  LengthUnit Units;
  ASTType DType;
  ASTIdentifierList IL;

private:
  ASTDelayNode() = delete;
  void ParseDuration(const std::string &Unit);
  void SetDelayType(const ASTIdentifierNode *TId);
  const char *ParseUnits() const;

protected:
  ASTDelayNode(const std::string &ERM)
      : ASTExpressionNode(ASTIdentifierNode::Delay.Clone(),
                          new ASTStringNode(ERM), ASTTypeExpressionError),
        Id(nullptr), Void(nullptr), Duration(static_cast<uint64_t>(~0x0)),
        Units(LengthUnspecified), DType(ASTTypeExpressionError), IL() {}

public:
  static const unsigned DelayBits = 64U;

public:
  ASTDelayNode(const ASTIdentifierNode *DId, const ASTIdentifierNode *TId,
               const std::string &Time = "")
      : ASTExpressionNode(DId, ASTTypeDelay), Id(TId),
        Duration(static_cast<uint64_t>(~0x0)), Units(LengthUnspecified),
        DType(TId->GetASTType()), IL() {
    SetDelayType(TId);
    ParseDuration(Time);
  }

  ASTDelayNode(const ASTIdentifierNode *DId, const ASTBinaryOpNode *BOp)
      : ASTExpressionNode(DId, ASTTypeDelay), Id(nullptr), BOP(BOp),
        Duration(static_cast<uint64_t>(~0x0)), Units(LengthUnspecified),
        DType(BOp->GetASTType()), IL() {}

  ASTDelayNode(const ASTIdentifierNode *DId, const ASTUnaryOpNode *UOp)
      : ASTExpressionNode(DId, ASTTypeDelay), Id(nullptr), UOP(UOp),
        Duration(static_cast<uint64_t>(~0x0)), Units(LengthUnspecified),
        DType(UOp->GetASTType()), IL() {}

  ASTDelayNode(const ASTIdentifierNode *DId, const ASTIdentifierNode *TId,
               const ASTIdentifierNode *QId, const std::string &Time = "")
      : ASTExpressionNode(DId, ASTTypeDelay), Id(TId), Void(nullptr),
        Duration(static_cast<uint64_t>(~0x0)), Units(LengthUnspecified),
        DType(ASTTypeQubit), IL() {
    SetDelayType(TId);
    ParseDuration(Time);
    IL.Append(const_cast<ASTIdentifierNode *>(QId));
  }

  ASTDelayNode(const ASTIdentifierNode *DId, const ASTIdentifierNode *TId,
               const ASTIdentifierList &QIL, const std::string &Time = "")
      : ASTExpressionNode(DId, ASTTypeDelay), Id(TId), Void(nullptr),
        Duration(static_cast<uint64_t>(~0x0)), Units(LengthUnspecified),
        DType(ASTTypeQubit), IL(QIL) {
    SetDelayType(TId);
    ParseDuration(Time);
  }

  ASTDelayNode(const ASTIdentifierNode *DId, const ASTIdentifierList &QIL,
               const std::string &Time = "")
      : ASTExpressionNode(DId, ASTTypeDelay), Id(nullptr), Void(nullptr),
        Duration(static_cast<uint64_t>(~0x0)), Units(LengthUnspecified),
        DType(ASTTypeQubit), IL(QIL) {
    ParseDuration(Time);
  }

  ASTDelayNode(const ASTIdentifierNode *DId, const ASTDurationOfNode *DoN,
               const ASTIdentifierList &QIL)
      : ASTExpressionNode(DId, ASTTypeDelay), Id(nullptr), DON(DoN),
        Duration(static_cast<uint64_t>(~0x0)), Units(LengthUnspecified),
        DType(DoN->GetASTType()), IL(QIL) {}

  ASTDelayNode(const ASTIdentifierNode *DId, const ASTDurationNode *Dn,
               const ASTIdentifierList &QIL)
      : ASTExpressionNode(DId, ASTTypeDelay), Id(nullptr), DN(Dn),
        Duration(Dn->GetDuration()), Units(Dn->GetLengthUnit()),
        DType(Dn->GetASTType()), IL(QIL) {}

  virtual ~ASTDelayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeDelay; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual unsigned Size() const { return IL.Size(); }

  const ASTIdentifierList &GetOperandList() const { return IL; }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual uint64_t GetDuration() const { return Duration; }

  virtual LengthUnit GetLengthUnit() const { return Units; }

  virtual const std::string &GetName() const override {
    return GetIdentifier()->GetName();
  }

  virtual const ASTIdentifierNode *GetDelayIdentifier() const {
    switch (DType) {
    case ASTTypeIdentifier:
      return Id;
      break;
    case ASTTypeStretch:
      return STR->GetIdentifier();
      break;
    case ASTTypeDuration:
      return DN->GetIdentifier();
      break;
    case ASTTypeDurationOf:
      return DON->GetIdentifier();
      break;
    case ASTTypeBinaryOp:
      return BOP->GetIdentifier();
      break;
    case ASTTypeUnaryOp:
      return UOP->GetIdentifier();
      break;
    default:
      return nullptr;
      break;
    }
  }

  virtual bool IsDelayAll() const { return IL.Empty(); }

  virtual const ASTBinaryOpNode *GetDelayBinaryOp() const {
    return DType == ASTTypeBinaryOp ? BOP : nullptr;
  }

  virtual ASTBinaryOpNode *GetDelayBinaryOp() {
    return DType == ASTTypeBinaryOp ? const_cast<ASTBinaryOpNode *>(BOP)
                                    : nullptr;
  }

  virtual const ASTUnaryOpNode *GetDelayUnaryOp() const {
    return DType == ASTTypeUnaryOp ? UOP : nullptr;
  }

  virtual ASTUnaryOpNode *GetDelayUnaryOp() {
    return DType == ASTTypeUnaryOp ? const_cast<ASTUnaryOpNode *>(UOP)
                                   : nullptr;
  }

  virtual const ASTDurationOfNode *GetDurationOfNode() const {
    return DType == ASTTypeDurationOf ? DON : nullptr;
  }

  virtual ASTDurationOfNode *GetDurationOfNode() {
    return DType == ASTTypeDurationOf ? const_cast<ASTDurationOfNode *>(DON)
                                      : nullptr;
  }

  virtual const ASTDurationNode *GetDurationNode() const {
    return DType == ASTTypeDuration ? DN : nullptr;
  }

  virtual ASTDurationNode *GetDurationNode() {
    return DType == ASTTypeDuration ? const_cast<ASTDurationNode *>(DN)
                                    : nullptr;
  }

  virtual const ASTStretchNode *GetStretchNode() const {
    return DType == ASTTypeStretch ? STR : nullptr;
  }

  virtual ASTStretchNode *GetStretchNode() {
    return DType == ASTTypeStretch ? const_cast<ASTStretchNode *>(STR)
                                   : nullptr;
  }

  virtual const ASTIdentifierNode *GetDelayQubitIdentifier() const {
    return IL.Size() ? IL[0] : nullptr;
  }

  virtual const ASTIdentifierNode *
  GetDelayQubitIdentifier(unsigned Index) const {
    assert(Index < IL.Size() && "Index is out-of-range!");
    return IL[Index];
  }

  virtual ASTType GetDelayType() const { return DType; }

  virtual bool IsError() const override {
    return DType == ASTTypeExpressionError;
  }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTDelayNode *ExpressionError(const std::string &ERM) {
    return new ASTDelayNode(ERM);
  }

  virtual void print() const override {
    std::cout << "<Delay>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;
    std::cout << "<DelayType>" << PrintTypeEnum(DType) << "</DelayType>"
              << std::endl;
    std::cout << "<Duration>" << std::dec << Duration << "</Duration>"
              << std::endl;
    std::cout << "<Units>" << PrintLengthUnit(Units) << "</Units>" << std::endl;
    switch (DType) {
    case ASTTypeIdentifier:
      Id->print();
      break;
    case ASTTypeBinaryOp:
      BOP->print();
      break;
    case ASTTypeUnaryOp:
      UOP->print();
      break;
    case ASTTypeQubit:
      Id->print();
      IL.print();
      break;
    case ASTTypeDurationOf:
      DON->print();
      IL.print();
      break;
    case ASTTypeDuration:
      DN->print();
      IL.print();
      break;
    case ASTTypeStretch:
      STR->print();
      IL.print();
      break;
    default:
      break;
    }
    std::cout << "</Delay>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTDelayStatementNode : public ASTStatementNode {
private:
  const ASTDelayNode *Delay;

private:
  ASTDelayStatementNode() = delete;

protected:
  ASTDelayStatementNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)),
        Delay(nullptr) {}

public:
  ASTDelayStatementNode(const ASTDelayNode *D)
      : ASTStatementNode(D->GetIdentifier()), Delay(D) {}

  ASTDelayStatementNode(const ASTIdentifierNode *DId, const ASTDelayNode *D)
      : ASTStatementNode(DId, D), Delay(D) {}

  virtual ~ASTDelayStatementNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeDelayStatement; }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual const ASTDelayNode *GetDelay() const { return Delay; }

  virtual ASTDelayNode *GetDelay() { return const_cast<ASTDelayNode *>(Delay); }

  bool IsError() const override { return ASTStatementNode::IsError(); }

  const std::string &GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTDelayStatementNode *StatementError(const std::string &ERM) {
    return new ASTDelayStatementNode(ASTIdentifierNode::Delay.Clone(), ERM);
  }

  virtual void print() const override {
    std::cout << "<DelayStatement>" << std::endl;
    Delay->print();
    std::cout << "</DelayStatement>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_DELAY_H
