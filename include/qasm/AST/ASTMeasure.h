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

#ifndef __QASM_AST_MEASURE_H
#define __QASM_AST_MEASURE_H

#include <qasm/AST/ASTGates.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTTypes.h>

namespace QASM {

class ASTMeasureNode : public ASTGateQOpNode {
private:
  ASTQubitContainerNode* Target;

  union {
    ASTCBitNode* Result;
    ASTAngleNode* Angle;
    ASTMPComplexNode* Complex;
  };

  unsigned RI;
  unsigned TI;
  std::vector<unsigned> RIV;
  std::vector<unsigned> TIV;
  ASTType RTy;

private:
  ASTMeasureNode() = delete;

protected:
  ASTMeasureNode(const ASTIdentifierNode* Id, const std::string& ERM)
  : ASTGateQOpNode(Id, ERM), RI(static_cast<unsigned>(~0x0)),
  TI(static_cast<unsigned>(~0x0)), RIV(), TIV(), RTy(ASTTypeExpressionError)
  { }

public:
  static const unsigned MeasureBits = 64U;

public:
  ASTMeasureNode(const ASTIdentifierNode* Id, ASTQubitContainerNode* T,
                 ASTCBitNode* R, unsigned RIX = static_cast<unsigned>(~0x0),
                 unsigned TIX = static_cast<unsigned>(~0x0))
  : ASTGateQOpNode(Id, static_cast<ASTGateNode*>(nullptr)),
  Target(T), Result(R), RI(RIX), TI(TIX), RIV(),
  TIV(), RTy(R->GetASTType()) { }

  ASTMeasureNode(const ASTIdentifierNode* Id, ASTQubitContainerNode* T,
                 ASTAngleNode* R, unsigned RIX = static_cast<unsigned>(~0x0),
                 unsigned TIX = static_cast<unsigned>(~0x0))
  : ASTGateQOpNode(Id, static_cast<ASTGateNode*>(nullptr)),
  Target(T), Angle(R), RI(RIX), TI(TIX), RIV(),
  TIV(), RTy(R->GetASTType()) { }

  ASTMeasureNode(const ASTIdentifierNode* Id, ASTQubitContainerNode* T,
                 ASTMPComplexNode* R, unsigned RIX = static_cast<unsigned>(~0x0),
                 unsigned TIX = static_cast<unsigned>(~0x0))
  : ASTGateQOpNode(Id, static_cast<ASTGateNode*>(nullptr)),
  Target(T), Complex(R), RI(RIX), TI(TIX), RIV(),
  TIV(), RTy(R->GetASTType()) { }

  virtual ~ASTMeasureNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeMeasure;
  }

  virtual void Mangle() override;

  virtual bool HasTarget() const {
    return Target;
  }

  virtual ASTQubitContainerNode* GetTarget() {
    return Target;
  }

  virtual const ASTQubitContainerNode* GetTarget() const {
    return Target;
  }

  virtual ASTType GetResultType() const {
    return RTy;
  }

  virtual bool HasResult() const {
    return RTy == ASTTypeBitset && Result;
  }

  virtual bool HasAngleResult() const {
    return RTy == ASTTypeAngle && Angle;
  }

  virtual bool HasComplexResult() const {
    return RTy == ASTTypeMPComplex && Complex;
  }

  virtual ASTCBitNode* GetResult() {
    return RTy == ASTTypeBitset ? Result : nullptr;
  }

  virtual const ASTCBitNode* GetResult() const {
    return RTy == ASTTypeBitset ? Result : nullptr;
  }

  virtual ASTAngleNode* GetAngleResult() {
    return RTy == ASTTypeAngle ? Angle : nullptr;
  }

  virtual const ASTAngleNode* GetAngleResult() const {
    return RTy == ASTTypeAngle ? Angle : nullptr;
  }

  virtual ASTMPComplexNode* GetComplexResult() {
    return RTy == ASTTypeMPComplex ? Complex : nullptr;
  }

  virtual const ASTMPComplexNode* GetComplexResult() const {
    return RTy == ASTTypeMPComplex ? Complex : nullptr;
  }

  virtual unsigned GetResultSize() const {
    return static_cast<unsigned>(RIV.size());
  }

  virtual unsigned GetTargetSize() const {
    return static_cast<unsigned>(TIV.size());
  }

  virtual void SetResultIndex(unsigned I) {
    RIV.push_back(I);
    RI = I;
  }

  virtual void AddResultIndex(unsigned I) {
    RIV.push_back(I);
  }

  virtual unsigned GetResultIndex(unsigned IX) const {
    assert(IX < RIV.size() && "Index is out-of-range!");
    if (IX < RIV.size())
      return RIV.at(IX);

    return static_cast<unsigned>(~0x0);
  }

  virtual void SetTargetIndex(unsigned I) {
    TIV.push_back(I);
    TI = I;
  }

  virtual void AddTargetIndex(unsigned I) {
    TIV.push_back(I);
  }

  virtual unsigned GetTargetIndex(unsigned IX) const {
    assert(IX < TIV.size() && "Index is out-of-range!");

    if (IX < TIV.size())
      return TIV.at(IX);

    return static_cast<unsigned>(~0x0);
  }

  virtual void AddTargetAndResultIndex(unsigned T, unsigned R) {
    RIV.push_back(R);
    TIV.push_back(T);
  }

  virtual bool HasResultIndex() const {
    if (RIV.size())
      return true;

    return RI != static_cast<unsigned>(~0x0);
  }

  virtual bool HasTargetIndex() const {
    if (TIV.size())
      return true;

    return TI != static_cast<unsigned>(~0x0);
  }

  virtual bool HasTargetVector() const {
    return !TIV.empty();
  }

  virtual bool HasResultVector() const {
    return !RIV.empty();
  }

  virtual const std::vector<unsigned>& GetResultVector() const {
    return RIV;
  }

  virtual const std::vector<unsigned>& GetTargetVector() const {
    return TIV;
  }

  virtual unsigned GetResultVectorSize() const {
    return static_cast<unsigned>(RIV.size());
  }

  virtual unsigned GetTargetVectorSize() const {
    return static_cast<unsigned>(TIV.size());
  }

  virtual const ASTIdentifierNode* GetIdentifier() const override {
    return ASTGateQOpNode::GetOperand();
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual bool IsError() const override {
    return ASTGateOpNode::MTy == ASTTypeExpressionError ||
           ASTGateOpNode::OTy == ASTTypeExpressionError ||
           RTy == ASTTypeExpressionError;
  }

  virtual const std::string& GetError() const override {
    return ASTStatementNode::GetError();
  }

  static ASTMeasureNode* StatementError(const ASTIdentifierNode* Id,
                                        const std::string& ERM) {
    return new ASTMeasureNode(Id, ERM);
  }

  virtual void print() const override;

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM

#endif // __QASM_AST_MEASURE_H

