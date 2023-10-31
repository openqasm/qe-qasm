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

#ifndef __QASM_AST_ARRAY_SUBSCRIPT_H
#define __QASM_AST_ARRAY_SUBSCRIPT_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTCBit.h>

#include <vector>

namespace QASM {

class ASTArraySubscriptNode : public ASTExpressionNode {
  friend class ASTArraySubscriptList;

private:
  mutable int32_t IX;
  union {
    const ASTIntNode* IN;
    const ASTMPIntegerNode* MPI;
    const ASTIdentifierNode* ID;
    const ASTIdentifierRefNode* IDR;
    const ASTBinaryOpNode* BOP;
    const ASTUnaryOpNode* UOP;
    const ASTCBitNode* CB;
    const ASTExpressionNode* EX;
  };

  ASTType EType;

private:
  ASTArraySubscriptNode() = delete;

private:
  unsigned GetIdentifierIndexValue() const;

protected:
  ASTArraySubscriptNode(const std::string& ERM)
  : ASTExpressionNode(ASTIdentifierNode::ArraySubscript.Clone(),
                      new ASTStringNode(ERM), ASTTypeExpressionError),
  IX(static_cast<unsigned>(~0x0)), IN(nullptr), EType(ASTTypeExpressionError) {
    ASTExpressionNode::SetExpressionType(ASTEXTypeSSA);
  }

public:
  explicit ASTArraySubscriptNode(const ASTIntNode* I)
  : ASTExpressionNode(ASTIdentifierNode::ArraySubscript.Clone(),
                      ASTTypeArraySubscript),
  IX(static_cast<unsigned>(~0x0)), IN(I), EType(I->GetASTType()) {
    IX = I->IsSigned() ? I->GetSignedValue() :
                         static_cast<int32_t>(I->GetUnsignedValue());
    ASTExpressionNode::SetExpressionType(ASTAXTypeIndexIdentifier);
  }

  explicit ASTArraySubscriptNode(const ASTMPIntegerNode* MI)
  : ASTExpressionNode(ASTIdentifierNode::ArraySubscript.Clone(),
                      ASTTypeArraySubscript),
  IX(static_cast<unsigned>(~0x0)), MPI(MI), EType(MI->GetASTType()) {
    IX = MI->IsSigned() ? MI->ToSignedInt() :
                          static_cast<int32_t>(MI->ToUnsignedInt());
    ASTExpressionNode::SetExpressionType(ASTAXTypeIndexIdentifier);
  }

  explicit ASTArraySubscriptNode(const ASTIdentifierNode* Id)
  : ASTExpressionNode(ASTIdentifierNode::ArraySubscript.Clone(),
                      ASTTypeArraySubscript),
  IX(static_cast<unsigned>(~0x0)), ID(Id), EType(Id->GetASTType()) {
    if (Id->IsInductionVariable()) {
      EType = ASTTypeIdentifier;
      ASTExpressionNode::SetExpressionType(ASTIITypeInductionVariable);
    } else {
      if (Id->IsReference()) {
        if (const ASTIdentifierRefNode* IdR =
            dynamic_cast<const ASTIdentifierRefNode*>(Id))
          IX = static_cast<int32_t>(IdR->GetIndex());
      } else {
        IX = static_cast<int32_t>(GetIdentifierIndexValue());
      }

      ASTExpressionNode::SetExpressionType(ASTIITypeIndexIdentifier);
    }
  }

  explicit ASTArraySubscriptNode(const ASTIdentifierRefNode* IdR)
  : ASTExpressionNode(ASTIdentifierNode::ArraySubscript.Clone(),
                      ASTTypeArraySubscript),
  IX(static_cast<int32_t>(IdR->GetIndex())), IDR(IdR),
  EType(IdR->IsInductionVariable() ? ASTTypeIdentifierRef : IdR->GetASTType()) {
    ASTExpressionNode::SetExpressionType(IdR->IsInductionVariable() ?
                                         ASTIITypeInductionVariable :
                                         ASTIITypeIndexIdentifier);
  }

  explicit ASTArraySubscriptNode(const ASTBinaryOpNode* BOp)
  : ASTExpressionNode(&ASTIdentifierNode::ArraySubscript,
                      ASTTypeArraySubscript),
  IX(static_cast<unsigned>(~0x0)), BOP(BOp), EType(BOp->GetASTType()) {
    ASTExpressionNode::SetExpressionType(ASTAXTypeIndexIdentifier);
  }

  explicit ASTArraySubscriptNode(const ASTUnaryOpNode* UOp)
  : ASTExpressionNode(ASTIdentifierNode::ArraySubscript.Clone(),
                      ASTTypeArraySubscript),
  IX(static_cast<unsigned>(~0x0)), UOP(UOp), EType(UOp->GetASTType()) {
    ASTExpressionNode::SetExpressionType(ASTAXTypeIndexIdentifier);
  }

  explicit ASTArraySubscriptNode(const ASTCBitNode* CBN)
  : ASTExpressionNode(ASTIdentifierNode::ArraySubscript.Clone(),
                      ASTTypeArraySubscript),
  IX(static_cast<unsigned>(~0x0)), CB(CBN), EType(CBN->GetASTType()) {
    ASTExpressionNode::SetExpressionType(ASTAXTypeIndexIdentifier);
  }

  ASTArraySubscriptNode(const ASTExpressionNode* EXp)
  : ASTExpressionNode(ASTIdentifierNode::ArraySubscript.Clone(),
                      ASTTypeArraySubscript),
  IX(static_cast<unsigned>(~0x0)), EX(EXp), EType(EXp->GetASTType()) {
    ASTExpressionNode::SetExpressionType(ASTAXTypeIndexIdentifier);
  }

  ASTArraySubscriptNode(const ASTArraySubscriptNode& RHS)
  : ASTExpressionNode(RHS), IX(RHS.IX), EType(RHS.EType) { }

  virtual ~ASTArraySubscriptNode() = default;

  ASTArraySubscriptNode& operator=(const ASTArraySubscriptNode& RHS) {
    if (this != &RHS) {
      ASTExpressionNode::operator=(RHS);

      switch (RHS.EType) {
      case ASTTypeInt:
      case ASTTypeUInt:
        IN = RHS.IN;
        break;
      case ASTTypeMPInteger:
      case ASTTypeMPUInteger:
        MPI = RHS.MPI;
        break;
      case ASTTypeIdentifier:
        ID = RHS.ID;
        break;
      case ASTTypeIdentifierRef:
        IDR = RHS.IDR;
        break;
      case ASTTypeBinaryOp:
        BOP = RHS.BOP;
        break;
      case ASTTypeUnaryOp:
        UOP = RHS.UOP;
        break;
      case ASTTypeBitset:
        CB = RHS.CB;
        break;
      default:
        EX = RHS.EX;
        break;
      }

      IX = RHS.IX;
      EType = RHS.EType;
    }

    return *this;
  }

  virtual void SetIndex(int32_t IV) const {
    IX = IV;
  }

  virtual ASTType GetASTType() const override {
    return ASTTypeArraySubscript;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual int32_t GetIndex() const {
    return IX;
  }

  virtual ASTType GetExpressionType() const {
    return EType;
  }

  virtual bool IsIdentifier() const override {
    return ASTExpressionNode::EXTy == ASTIITypeIndexIdentifier ||
           ASTExpressionNode::EXTy == ASTAXTypeIndexIdentifier;
  }

  virtual const ASTIdentifierRefNode* GetIndexIdentifierRef() const {
    return dynamic_cast<const ASTIdentifierRefNode*>(
                        ASTExpressionNode::GetIndexIdentifier());
  }

  virtual std::string AsIndexedString() const;

  virtual int32_t GetSignedIndexValue() const;

  virtual uint32_t GetUnsignedIndexValue() const;

  virtual bool IsError() const override {
    return ASTExpressionNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTArraySubscriptNode* ExpressionError(const std::string& ERM) {
    return new ASTArraySubscriptNode(ERM);
  }

  virtual void print() const override {
    std::cout << "<ArraySubscript>" << std::endl;
    std::cout << "<Index>" << IX << "</Index>" << std::endl;
    std::cout << "<ExpressionType>" << PrintTypeEnum(EType)
      << "</ExpressionType>" << std::endl;

    switch (EType) {
    case ASTTypeInt:
      IN->print();
      break;
    case ASTTypeIdentifier:
      ID->print();
      break;
    case ASTTypeIdentifierRef:
      IDR->print();
      break;
    case ASTTypeBinaryOp:
      BOP->print();
      break;
    case ASTTypeUnaryOp:
      UOP->print();
      break;
    case ASTTypeBitset:
      CB->print();
      break;
    case ASTTypeExpression:
      EX->print();
      break;
    default:
      break;
    }

    std::cout << "</ArraySubscript>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTArraySubscriptList : public ASTBase {
private:
  std::vector<const ASTArraySubscriptNode*> SV;

public:
  using list_type = std::vector<const ASTArraySubscriptNode*>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTArraySubscriptList() = default;

  ASTArraySubscriptList(const ASTArraySubscriptList& RHS)
  : ASTBase(RHS), SV(RHS.SV) { }

  virtual ~ASTArraySubscriptList() = default;

  ASTArraySubscriptList& operator=(const ASTArraySubscriptList& RHS) {
    if (this != &RHS) {
      ASTBase::operator=(RHS);
      SV = RHS.SV;
    }

    return *this;
  }

  virtual ASTType GetASTType() const override {
    return ASTTypeArraySubscriptList;
  }

  virtual void Append(const ASTArraySubscriptNode* ASN);

  virtual void Prepend(const ASTArraySubscriptNode* ASN);

  virtual unsigned Size() const {
    return static_cast<unsigned>(SV.size());
  }

  virtual bool Empty() const {
    return SV.empty();
  }

  virtual void Clear() {
    SV.clear();
  }

  const ASTArraySubscriptNode* At(unsigned Index) const {
    assert(Index < SV.size() && "Index is out-of-range!");
    return Index < SV.size() ? SV.at(Index) : nullptr;
  }

  virtual std::string AsIndexedString() const;

  iterator begin() {
    return SV.begin();
  }

  const_iterator begin() const {
    return SV.begin();
  }

  iterator end() {
    return SV.end();
  }

  const_iterator end() const {
    return SV.end();
  }

  virtual void print() const override {
    std::cout << "<ArraySubscriptList>" << std::endl;
    for (ASTArraySubscriptList::const_iterator I = SV.begin();
         I != SV.end(); ++I)
      (*I)->print();
    std::cout << "</ArraySubscriptList>" << std::endl;
  }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM

#endif // __QASM_AST_ARRAY_SUBSCRIPT_H

