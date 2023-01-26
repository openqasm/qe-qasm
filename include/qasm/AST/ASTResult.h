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

#ifndef __QASM_AST_RESULT_NODE_H
#define __QASM_AST_RESULT_NODE_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTMPComplexList.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseWaveform.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseFrame.h>

namespace QASM {

class ASTFunctionDefinitionNode;
class ASTKernelNode;
class ASTDefcalNode;

class ASTResultNode : public ASTExpressionNode {
  friend class ASTResultBuilder;

private:
  union {
    const ASTVoidNode* Void;
    const ASTAngleNode* Angle;
    const ASTBoolNode* Bool;
    const ASTCBitNode* CBit;
    const ASTIntNode* Int;
    const ASTFloatNode* Float;
    const ASTDoubleNode* Double;
    const ASTLongDoubleNode* LongDouble;
    const ASTMPIntegerNode* MPInt;
    const ASTMPDecimalNode* MPDec;
    const ASTMPComplexNode* MPComplex;
    const ASTMPComplexList* MPComplexList;
    const ASTDurationNode* Duration;
    const ASTBinaryOpNode* BinaryOp;
    const ASTUnaryOpNode* UnaryOp;
    const OpenPulse::ASTOpenPulseWaveformNode* Waveform;
    const OpenPulse::ASTOpenPulseFrameNode* Frame;
    const OpenPulse::ASTOpenPulsePortNode* Port;
  };

  const ASTIdentifierNode* Id;
  const ASTFunctionDefinitionNode* FN;
  const ASTKernelNode* KN;
  const ASTDefcalNode* DN;
  bool Proto;
  mutable bool P;

private:
  ASTResultNode() = delete;

protected:
  ASTResultNode(const ASTIdentifierNode* IId, const std::string& ERM)
  : ASTExpressionNode(IId, new ASTStringNode(ERM), ASTTypeExpressionError),
  Void(nullptr), Id(IId), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

public:
  static const unsigned ResultBits = 64U;

public:
  ASTResultNode(const ASTIdentifierNode* IId, ASTType Ty = ASTTypeUndefined)
  : ASTExpressionNode(IId, Ty), Void(nullptr), Id(nullptr),
  FN(nullptr), KN(nullptr), DN(nullptr), Proto(false), P(false) { }

  ASTResultNode(const ASTIdentifierNode* IId, const ASTAngleNode* A)
  : ASTExpressionNode(IId, A->GetASTType()), Angle(A),
  Id(nullptr), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

  ASTResultNode(const ASTIdentifierNode* IId, const ASTBoolNode* B)
  : ASTExpressionNode(IId, B->GetASTType()), Bool(B),
  Id(nullptr), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

  ASTResultNode(const ASTIdentifierNode* IId, const ASTCBitNode* C)
  : ASTExpressionNode(IId, C->GetASTType()), CBit(C),
  Id(nullptr), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

  ASTResultNode(const ASTIdentifierNode* IId, const ASTDoubleNode* D)
  : ASTExpressionNode(IId, D->GetASTType()), Double(D),
  Id(nullptr), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

  ASTResultNode(const ASTIdentifierNode* IId, const ASTFloatNode* F)
  : ASTExpressionNode(IId, F->GetASTType()), Float(F),
  Id(nullptr), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

  ASTResultNode(const ASTIdentifierNode* IId, const ASTIntNode* I)
  : ASTExpressionNode(IId, I->GetASTType()), Int(I),
  Id(nullptr), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

  ASTResultNode(const ASTIdentifierNode* IId, const ASTVoidNode* V)
  : ASTExpressionNode(IId, V->GetASTType()), Void(V),
  Id(nullptr), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

  ASTResultNode(const ASTIdentifierNode* IId, const ASTMPIntegerNode* MPI)
  : ASTExpressionNode(IId, MPI->GetASTType()), MPInt(MPI),
  Id(nullptr), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

  ASTResultNode(const ASTIdentifierNode* IId, const ASTMPDecimalNode* MPD)
  : ASTExpressionNode(IId, MPD->GetASTType()), MPDec(MPD),
  Id(nullptr), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

  ASTResultNode(const ASTIdentifierNode* IId, const ASTMPComplexNode* MPC)
  : ASTExpressionNode(IId, MPC->GetASTType()), MPComplex(MPC),
  Id(nullptr), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

  ASTResultNode(const ASTIdentifierNode* IId, const ASTMPComplexList* MPCL)
  : ASTExpressionNode(IId, MPCL->GetASTType()), MPComplexList(MPCL),
  Id(nullptr), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

  ASTResultNode(const ASTIdentifierNode* IId,
                const OpenPulse::ASTOpenPulseWaveformNode* WFN)
  : ASTExpressionNode(IId, WFN->GetASTType()), Waveform(WFN),
  Id(nullptr), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

  ASTResultNode(const ASTIdentifierNode* IId,
                const OpenPulse::ASTOpenPulseFrameNode* OFN)
  : ASTExpressionNode(IId, OFN->GetASTType()), Frame(OFN),
  Id(nullptr), FN(nullptr), KN(nullptr), DN(nullptr),
  Proto(false), P(false) { }

  virtual ~ASTResultNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeResult;
  }

  virtual void Mangle() override;

  virtual ASTType GetResultType() const {
    return Type;
  }

  virtual unsigned GetResultBits() const {
    switch (Type) {
    case ASTTypeBool:
      return ASTBoolNode::BoolBits;
      break;
    case ASTTypeInt:
      return ASTIntNode::IntBits;
      break;
    case ASTTypeFloat:
      return ASTFloatNode::FloatBits;
      break;
    case ASTTypeDouble:
      return ASTDoubleNode::DoubleBits;
      break;
    case ASTTypeLongDouble:
      return ASTLongDoubleNode::LongDoubleBits;
      break;
    case ASTTypeVoid:
      return ASTVoidNode::VoidBits;
      break;
    case ASTTypeDuration:
      return ASTDurationNode::DurationBits;
      break;
    case ASTTypeOpenPulseFrame:
      return OpenPulse::ASTOpenPulseFrameNode::FrameBits;
      break;
    case ASTTypeOpenPulseWaveform:
      return OpenPulse::ASTOpenPulseWaveformNode::WaveformBits;
      break;
    case ASTTypeOpenPulsePort:
      return OpenPulse::ASTOpenPulsePortNode::PortBits;
      break;
    case ASTTypeAngle:
      return Angle->GetBits();
      break;
    case ASTTypeMPInteger:
      return MPInt->GetBits();
      break;
    case ASTTypeMPDecimal:
      return MPDec->GetBits();
      break;
    case ASTTypeMPComplex:
      return MPComplex->GetBits();
      break;
    case ASTTypeBitset:
      return CBit->Size();
      break;
    default:
      break;
    }

    return 0U;
  }

  virtual void SetIsProto() {
    Proto = true;
  }

  virtual void SetFunction(const ASTIdentifierNode* I,
                           const ASTFunctionDefinitionNode* F) {
    Id = I;
    FN = F;
  }

  virtual void SetKernel(const ASTIdentifierNode* I,
                         const ASTKernelNode* K) {
    Id = I;
    KN = K;
  }

  virtual void SetDefcal(const ASTIdentifierNode* I,
                         const ASTDefcalNode* D) {
    Id = I;
    DN = D;
  }

  virtual const ASTFunctionDefinitionNode* GetFunction() const {
    return FN;
  }

  virtual const ASTKernelNode* GetKernel() const {
    return KN;
  }

  virtual const ASTDefcalNode* GetDefcal() const {
    return DN;
  }

  virtual const ASTIdentifierNode* GetFunctionIdentifier() const {
    return FN ? Id : nullptr;
  }

  virtual const ASTIdentifierNode* GetKernelIdentifier() const {
    return KN ? Id : nullptr;
  }

  virtual const ASTIdentifierNode* GetDefcalIdentifier() const {
    return DN ? Id : nullptr;
  }

  virtual bool IsKernel() const {
    return KN && ASTExpressionNode::Type == ASTTypeKernel;
  }

  virtual bool IsFunction() const {
    return FN && ASTExpressionNode::Type == ASTTypeFunction;
  }

  virtual bool IsDefcal() const {
    return DN && ASTExpressionNode::Type == ASTTypeDefcal;
  }

  virtual bool IsProto() const {
    return Proto;
  }

  virtual void SetResult(const ASTAngleNode* AN) {
    Angle = AN;
    Type = ASTTypeAngle;
  }

  virtual void SetResult(const ASTVoidNode* VN) {
    Void = VN;
    Type = VN->GetASTType();
  }

  virtual void SetResult(const ASTCBitNode* CB) {
    CBit = CB;
    Type = CB->GetASTType();
  }

  virtual void SetResult(const ASTBoolNode* Val) {
    Bool = Val;
    Type = Val->GetASTType();
  }

  virtual void SetResult(const ASTIntNode* Val) {
    Int = Val;
    Type = Val->GetASTType();
  }

  virtual void SetResult(const ASTFloatNode* Val) {
    Float = Val;
    Type = Val->GetASTType();
  }

  virtual void SetResult(const ASTDoubleNode* Val) {
    Double = Val;
    Type = Val->GetASTType();
  }

  virtual void SetResult(const ASTLongDoubleNode* Val) {
    LongDouble = Val;
    Type = Val->GetASTType();
  }

  virtual void SetResult(const ASTMPIntegerNode* MPI) {
    MPInt = MPI;
    Type = MPI->GetASTType();
  }

  virtual void SetResult(const ASTMPDecimalNode* MPD) {
    MPDec = MPD;
    Type = MPD->GetASTType();
  }

  virtual void SetResult(const ASTMPComplexNode* CX) {
    MPComplex = CX;
    Type = CX->GetASTType();
  }

  virtual void SetResult(const ASTMPComplexList* CXL) {
    MPComplexList = CXL;
    Type = CXL->GetASTType();
  }

  virtual void SetResult(const ASTBinaryOpNode* BOP) {
    BinaryOp = BOP;
    Type = BOP->GetASTType();
  }

  virtual void SetResult(const ASTUnaryOpNode* UOP) {
    UnaryOp = UOP;
    Type = UOP->GetASTType();
  }

  virtual void SetResult(const ASTDurationNode* DRN) {
    Duration = DRN;
    Type = DRN->GetASTType();
  }

  virtual void SetResult(const OpenPulse::ASTOpenPulseWaveformNode* WFN) {
    Waveform = WFN;
    Type = WFN->GetASTType();
  }

  virtual void SetResult(const OpenPulse::ASTOpenPulseFrameNode* OFN) {
    Frame = OFN;
    Type = OFN->GetASTType();
  }

  virtual void SetResult(const OpenPulse::ASTOpenPulsePortNode* OPN) {
    Port = OPN;
    Type = OPN->GetASTType();
  }

  virtual const ASTAngleNode* GetAngleNode() const {
    return Type == ASTTypeAngle ? Angle : nullptr;
  }

  virtual ASTAngleNode* GetAngleNode() {
    return Type == ASTTypeAngle ? const_cast<ASTAngleNode*>(Angle) : nullptr;
  }

  virtual const ASTBoolNode* GetBoolNode() const {
    return Type == ASTTypeBool ? Bool : nullptr;
  }

  virtual ASTBoolNode* GetBoolNode() {
    return Type == ASTTypeBool ? const_cast<ASTBoolNode*>(Bool) : nullptr;
  }

  virtual const ASTVoidNode* GetVoidNode() const {
    return Type == ASTTypeVoid ? Void : nullptr;
  }

  virtual ASTVoidNode* GetVoidNode() {
    return Type == ASTTypeVoid ? const_cast<ASTVoidNode*>(Void) : nullptr;
  }

  virtual const ASTDurationNode* GetDurationNode() const {
    return Type == ASTTypeDuration ? Duration : nullptr;
  }

  virtual ASTDurationNode* GetDurationNode() {
    return Type == ASTTypeDuration ?
                   const_cast<ASTDurationNode*>(Duration) : nullptr;
  }

  virtual const ASTCBitNode* GetCBitNode() const {
    return Type == ASTTypeBitset ? CBit : nullptr;
  }

  virtual ASTCBitNode* GetCBitNode() {
    return Type == ASTTypeBitset ? const_cast<ASTCBitNode*>(CBit) : nullptr;
  }

  virtual const ASTIntNode* GetIntNode() const {
    return Type == ASTTypeInt ? Int : nullptr;
  }

  virtual ASTIntNode* GetIntNode() {
    return Type == ASTTypeInt ? const_cast<ASTIntNode*>(Int) : nullptr;
  }

  virtual const ASTFloatNode* GetFloatNode() const {
    return Type == ASTTypeFloat ? Float : nullptr;
  }

  virtual ASTFloatNode* GetFloatNode() {
    return Type == ASTTypeFloat ? const_cast<ASTFloatNode*>(Float) : nullptr;
  }

  virtual const ASTDoubleNode* GetDoubleNode() const {
    return Type == ASTTypeDouble ? Double : nullptr;
  }

  virtual ASTDoubleNode* GetDoubleNode() {
    return Type == ASTTypeDouble ? const_cast<ASTDoubleNode*>(Double) : nullptr;
  }

  virtual const ASTLongDoubleNode* GetLongDoubleNode() const {
    return Type == ASTTypeLongDouble ? LongDouble : nullptr;
  }

  virtual ASTLongDoubleNode* GetLongDoubleNode() {
    return Type == ASTTypeLongDouble ?
                   const_cast<ASTLongDoubleNode*>(LongDouble) : nullptr;
  }

  virtual const ASTMPIntegerNode* GetMPInteger() const {
    return Type == ASTTypeMPInteger ? MPInt : nullptr;
  }

  virtual ASTMPIntegerNode* GetMPInteger() {
    return Type == ASTTypeMPInteger ? const_cast<ASTMPIntegerNode*>(MPInt)
                                    : nullptr;
  }

  virtual const ASTMPDecimalNode* GetMPDecimal() const {
    return Type == ASTTypeMPDecimal ? MPDec : nullptr;
  }

  virtual ASTMPDecimalNode* GetMPDecimal()  {
    return Type == ASTTypeMPDecimal ? const_cast<ASTMPDecimalNode*>(MPDec)
                                    : nullptr;
  }

  virtual const ASTMPComplexNode* GetMPComplex() const {
    return Type == ASTTypeMPComplex ? MPComplex : nullptr;
  }

  virtual ASTMPComplexNode* GetMPComplex() {
    return Type == ASTTypeMPComplex ?
                   const_cast<ASTMPComplexNode*>(MPComplex) : nullptr;
  }

  virtual const ASTMPComplexList* GetMPComplexList() const {
    return Type == ASTTypeComplexList ? MPComplexList : nullptr;
  }

  virtual ASTMPComplexList* GetMPComplexList() {
    return Type == ASTTypeComplexList ?
                   const_cast<ASTMPComplexList*>(MPComplexList) : nullptr;
  }

  virtual const ASTBinaryOpNode* GetBinaryOp() const {
    return Type == ASTTypeBinaryOp ? BinaryOp : nullptr;
  }

  virtual ASTBinaryOpNode* GetBinaryOp() {
    return Type == ASTTypeBinaryOp ? const_cast<ASTBinaryOpNode*>(BinaryOp)
                                   : nullptr;
  }

  virtual const ASTUnaryOpNode* GetUnaryOp() const {
    return Type == ASTTypeUnaryOp ? UnaryOp : nullptr;
  }

  virtual ASTUnaryOpNode* GetUnaryOp() {
    return Type == ASTTypeUnaryOp ? const_cast<ASTUnaryOpNode*>(UnaryOp)
                                  : nullptr;
  }

  virtual const OpenPulse::ASTOpenPulseWaveformNode* GetWaveform() const {
    return Type == ASTTypeOpenPulseWaveform ? Waveform : nullptr;
  }

  virtual OpenPulse::ASTOpenPulseWaveformNode* GetWaveform() {
    return Type == ASTTypeOpenPulseWaveform ?
                   const_cast<OpenPulse::ASTOpenPulseWaveformNode*>(Waveform) :
                   nullptr;
  }

  virtual const OpenPulse::ASTOpenPulseFrameNode* GetFrame() const {
    return Type == ASTTypeOpenPulseFrame ? Frame : nullptr;
  }

  virtual OpenPulse::ASTOpenPulseFrameNode* GetFrame() {
    return Type == ASTTypeOpenPulseFrame ?
                   const_cast<OpenPulse::ASTOpenPulseFrameNode*>(Frame) :
                   nullptr;
  }

  virtual const OpenPulse::ASTOpenPulsePortNode* GetPort() const {
    return Type == ASTTypeOpenPulsePort ?
                   const_cast<OpenPulse::ASTOpenPulsePortNode*>(Port) :
                   nullptr;
  }

  virtual OpenPulse::ASTOpenPulsePortNode* GetPort() {
    return Type == ASTTypeOpenPulsePort ?
                   const_cast<OpenPulse::ASTOpenPulsePortNode*>(Port) :
                   nullptr;
  }

  virtual bool IsAngle() const {
    return Type == ASTTypeAngle;
  }

  virtual bool IsCBit() const {
    return Type == ASTTypeBitset;
  }

  virtual bool IsQubit() const {
    return Type == ASTTypeQubitContainer;
  }

  virtual bool IsInt() const {
    return Type == ASTTypeInt;
  }

  virtual bool IsFloat() const {
    return Type == ASTTypeFloat;
  }

  virtual bool IsDouble() const {
    return Type == ASTTypeDouble;
  }

  virtual bool IsVoid() const {
    return Type == ASTTypeVoid;
  }

  virtual bool IsMPInteger() const {
    return Type == ASTTypeMPInteger;
  }

  virtual bool IsMPDecimal() const {
    return Type == ASTTypeMPDecimal;
  }

  virtual bool IsMPComplex() const {
    return Type == ASTTypeMPComplex;
  }

  virtual bool IsMPComplexList() const {
    return Type == ASTTypeComplexList;
  }

  virtual bool IsWaveform() const {
    return Type == ASTTypeOpenPulseWaveform;
  }

  virtual bool IsFrame() const {
    return Type == ASTTypeOpenPulseFrame;
  }

  virtual bool IsPort() const {
    return Type == ASTTypeOpenPulsePort;
  }

  virtual bool IsBinaryOp() const {
    return Type == ASTTypeBinaryOp;
  }

  virtual bool IsUnaryOp() const {
    return Type == ASTTypeUnaryOp;
  }

  virtual bool HasResult() const {
    return Type != ASTTypeUndefined &&
           Void != nullptr;
  }

  virtual bool IsError() const override {
    return ASTExpressionNode::IsError();
  }

  virtual const std::string& GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTResultNode* ExpressionError(const std::string& ERM) {
    return new ASTResultNode(ASTIdentifierNode::Result.Clone(), ERM);
  }

  static ASTResultNode* ExpressionError(const ASTIdentifierNode* Id,
                                        const std::string& ERM) {
    return new ASTResultNode(Id, ERM);
  }

  virtual void print() const override;

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM

#endif // __QASM_AST_RESULT_NODE_H

