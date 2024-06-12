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

#ifndef __QASM_AST_ARRAY_H
#define __QASM_AST_ARRAY_H

#include <qasm/AST/ASTCBit.h>
#include <qasm/AST/ASTDuration.h>
#include <qasm/AST/ASTInitializerNode.h>
#include <qasm/AST/ASTLength.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseFrame.h>
#include <qasm/AST/OpenPulse/ASTOpenPulsePort.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseWaveform.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <any>
#include <sstream>
#include <string>
#include <vector>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

class ASTArrayNode : public ASTExpressionNode {
private:
  ASTArrayNode() = delete;

protected:
  mutable std::any MM;
  ASTType AType;
  unsigned SZ;
  unsigned EXT;
  const ASTInitializerList *INL;

protected:
  ASTArrayNode(const ASTIdentifierNode *Id, const std::string &ERM, ASTType Ty)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        MM(), AType(Ty), SZ(0U), INL(nullptr) {}

public:
  static const unsigned ArrayBits = 64U;

public:
  ASTArrayNode(const ASTIdentifierNode *Id, ASTType ATy, unsigned Size,
               const ASTInitializerList *IL = nullptr)
      : ASTExpressionNode(Id, ASTTypeArray), MM(), AType(ATy), SZ(Size),
        EXT(1U), INL(IL) {}

  ASTArrayNode(const ASTIdentifierNode *Id, ASTType ATy, unsigned Size,
               unsigned Extents, const ASTInitializerList *IL = nullptr)
      : ASTExpressionNode(Id, ASTTypeArray), MM(), AType(ATy), SZ(Size),
        EXT(Extents), INL(IL) {}

  /// Validate the array access, emitting a diagnostic if invalid.
  void ValidateIndex(unsigned Index, QASM::ASTLocation location) const {
    if (Index >= Size()) {
      std::stringstream M;
      M << "Array index " << Index << " out of range for array of size "
        << Size() << ".";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(location, M.str(),
                                                       DiagLevel::Error);
    }
  }

  virtual ~ASTArrayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeArray; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override = 0;

  virtual ASTType GetElementType() const = 0;

  virtual unsigned GetElementSize() const = 0;

  virtual bool IsAggregate() const override { return true; }

  virtual ASTType GetArrayType() const { return AType; }

  virtual unsigned Size() const { return SZ; }

  virtual unsigned Extents() const { return EXT; }

  virtual std::any &Memory() = 0;

  virtual const std::any &Memory() const = 0;

  virtual bool HasInitializerList() const { return INL; }

  virtual const ASTInitializerList *GetInitializerList() const { return INL; }

  static unsigned GetElementIndex(const std::string &IS) {
    std::string::size_type X = IS.find_last_of(':');
    if (X != std::string::npos)
      return static_cast<unsigned>(std::stoi(IS.substr(X + 1)));

    return static_cast<unsigned>(~0x0);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  virtual void print() const override = 0;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTInvalidArrayNode : public ASTArrayNode {
private:
  ASTInvalidArrayNode() = delete;

public:
  ASTInvalidArrayNode(const std::string &ERM, const ASTToken *TK)
      : ASTArrayNode(ASTIdentifierNode::InvalidArray.Clone(), ERM,
                     ASTTypeInvalidArray) {
    SetLocation(TK->GetLocation());
  }

  ASTInvalidArrayNode(const ASTIdentifierNode *Id, const std::string &ERM,
                      const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeInvalidArray) {
    SetLocation(TK->GetLocation());
  }

  virtual ~ASTInvalidArrayNode() = default;

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override { return ASTTypeUndefined; }

  virtual bool IsAggregate() const override { return true; }

  virtual unsigned GetElementSize() const override { return 0U; }

  virtual std::any &Memory() override { return MM; }

  virtual std::any &Memory() const override { return MM; }

  virtual bool IsError() const override { return true; }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  virtual void print() const override {
    std::cout << "<InvalidArrayNode>" << std::endl;
    std::cout << "<Error>" << GetError() << "</Error>" << std::endl;
    std::cout << "</InvalidArrayNode>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTCBitArrayNode : public ASTArrayNode {
private:
  std::vector<ASTCBitNode *> BV;
  unsigned BS;

private:
  ASTCBitArrayNode() = delete;

protected:
  ASTCBitArrayNode(const ASTIdentifierNode *Id, const std::string &ERM,
                   const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeCBitArray), BV(), BS(1U) {
    SetLocation(TK->GetLocation());
  }

public:
  using list_type = std::vector<ASTCBitNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTCBitArrayNode(const ASTIdentifierNode *Id, unsigned Size)
      : ASTArrayNode(Id, ASTTypeCBitArray, Size), BV(), BS(1U) {
    // FIXME: Re-Implement for N-Dimensional.
    for (unsigned I = 0; I < Size; ++I)
      BV.push_back(new ASTCBitNode(Id, 1U));
  }

  ASTCBitArrayNode(const ASTIdentifierNode *Id, unsigned Size, unsigned Ext)
      : ASTArrayNode(Id, ASTTypeCBitArray, Size, Ext), BV(), BS(1U) {
    // FIXME: Re-Implement for N-Dimensional.
    for (unsigned I = 0; I < Size; ++I)
      BV.push_back(new ASTCBitNode(Id, 1U));
  }

  ASTCBitArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                   const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeCBitArray, Size, IL), BV(), BS(1U) {
    // FIXME: Re-Implement for N-Dimensional.
    for (unsigned I = 0; I < Size; ++I)
      BV.push_back(new ASTCBitNode(Id, 1U));
  }

  ASTCBitArrayNode(const ASTIdentifierNode *Id, unsigned Size, unsigned Ext,
                   const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeCBitArray, Size, Ext, IL), BV(), BS(1U) {
    // FIXME: Re-Implement for N-Dimensional.
    for (unsigned I = 0; I < Size; ++I)
      BV.push_back(new ASTCBitNode(Id, 1U));
  }

  ASTCBitArrayNode(const ASTIdentifierNode *Id, unsigned Size, unsigned BSize,
                   unsigned Ext)
      : ASTArrayNode(Id, ASTTypeCBitArray, Size, Ext), BV(), BS(BSize) {
    // FIXME: Re-Implement for N-Dimensional.
    for (unsigned I = 0; I < Size; ++I)
      BV.push_back(new ASTCBitNode(Id, 1U));
  }

  ASTCBitArrayNode(const ASTIdentifierNode *Id, unsigned Size, unsigned BSize,
                   unsigned Ext, const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeCBitArray, Size, Ext, IL), BV(), BS(BSize) {
    // FIXME: Re-Implement for N-Dimensional.
    for (unsigned I = 0; I < Size; ++I)
      BV.push_back(new ASTCBitNode(Id, 1U));
  }

  virtual ~ASTCBitArrayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeCBitArray; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override { return ASTTypeBitset; }

  virtual unsigned GetElementSize() const override { return BS; }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = BV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = BV.data();

    return MM;
  }

  iterator begin() { return BV.begin(); }

  iterator end() { return BV.end(); }

  const_iterator begin() const { return BV.begin(); }

  const_iterator end() const { return BV.end(); }

  virtual ASTCBitNode *GetElement(unsigned Index) {
    assert(Index < SZ && "Index is out-of-range!");

    try {
      return BV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTCBitNode *GetElement(unsigned Index) const {
    assert(Index < SZ && "Index is out-of-range!");

    try {
      return BV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual ASTCBitNode *operator[](unsigned Index) { return GetElement(Index); }

  virtual const ASTCBitNode *operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTCBitArrayNode *ExpressionError(const std::string &ERM,
                                           const ASTToken *TK) {
    return new ASTCBitArrayNode(ASTIdentifierNode::CBitArray.Clone(), ERM, TK);
  }

  static ASTCBitArrayNode *ExpressionError(const ASTIdentifierNode *Id,
                                           const std::string &ERM,
                                           const ASTToken *TK) {
    return new ASTCBitArrayNode(Id, ERM, TK);
  }

  virtual void print() const override {
    std::cout << "<ASTCBitArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    for (list_type::const_iterator I = BV.begin(); I != BV.end(); ++I)
      (*I)->print();

    std::cout << "</ASTCBitArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTCBitNArrayNode : public ASTArrayNode {
private:
  std::vector<ASTCBitNode *> BV;
  unsigned NS;

private:
  ASTCBitNArrayNode() = delete;

public:
  using vector_type = std::vector<ASTCBitNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTCBitNArrayNode(const ASTIdentifierNode *Id, unsigned Size, unsigned NSize)
      : ASTArrayNode(Id, ASTTypeCBitNArray, Size), BV(), NS(NSize) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-cbit-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *CId = new ASTIdentifierNode(S.str(), Size);
      assert(CId && "Could not create a valid ASTIdentifierNode!");

      BV.push_back(new ASTCBitNode(CId, NSize));
    }
  }

  virtual ~ASTCBitNArrayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeCBitNArray; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual unsigned GetElementSize() const override { return NS; }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override { return ASTTypeBitset; }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = BV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = BV.data();

    return MM;
  }

  iterator begin() { return BV.begin(); }

  iterator end() { return BV.end(); }

  const_iterator begin() const { return BV.begin(); }

  const_iterator end() const { return BV.end(); }

  virtual ASTCBitNode *GetElement(unsigned Index) {
    assert(Index < BV.size() && "Index is out-of-range!");

    try {
      return BV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTCBitNode *GetElement(unsigned Index) const {
    assert(Index < BV.size() && "Index is out-of-range!");

    try {
      return BV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  inline ASTCBitNode *operator[](unsigned Index) { return GetElement(Index); }

  inline const ASTCBitNode *operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual void print() const override {
    std::cout << "<CBitNArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    unsigned X = 0;
    for (std::vector<ASTCBitNode *>::const_iterator I = BV.begin();
         I != BV.end(); ++I) {
      std::cout << "<Index>" << X++ << "</Index>" << std::endl;
      (*I)->print();
    }

    std::cout << "</CBitNArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTQubitArrayNode : public ASTArrayNode {
private:
  std::vector<ASTQubitContainerNode *> QV;
  unsigned QS;

private:
  ASTQubitArrayNode() = delete;

protected:
  ASTQubitArrayNode(const ASTIdentifierNode *Id, const std::string &ERM,
                    const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeQubitArray), QV(), QS(1U) {
    SetLocation(TK->GetLocation());
  }

public:
  using list_type = std::vector<ASTQubitContainerNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTQubitArrayNode(const ASTIdentifierNode *Id, unsigned Size)
      : ASTArrayNode(Id, ASTTypeQubitArray, Size), QV(), QS(1U) {
    for (unsigned I = 0; I < Size; ++I)
      QV.push_back(new ASTQubitContainerNode(Id, 1, 1, Id->GetName()));
  }

  ASTQubitArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                    const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeQubitArray, Size, IL), QV(), QS(1U) {
    for (unsigned I = 0; I < Size; ++I)
      QV.push_back(new ASTQubitContainerNode(Id, Size, 1, Id->GetName()));
  }

  ASTQubitArrayNode(const ASTIdentifierNode *Id, unsigned Size, unsigned QSize)
      : ASTArrayNode(Id, ASTTypeQubitArray, Size), QV(), QS(QSize) {
    for (unsigned I = 0; I < Size; ++I)
      QV.push_back(new ASTQubitContainerNode(Id, 1, QSize, Id->GetName()));
  }

  ASTQubitArrayNode(const ASTIdentifierNode *Id, unsigned Size, unsigned QSize,
                    const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeQubitArray, Size, IL), QV(), QS(QSize) {
    for (unsigned I = 0; I < Size; ++I)
      QV.push_back(new ASTQubitContainerNode(Id, Size, QSize, Id->GetName()));
  }

  virtual ~ASTQubitArrayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeQubitArray; }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override { return ASTTypeQubit; }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = QV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = QV.data();

    return MM;
  }

  virtual unsigned GetElementSize() const override { return QS; }

  iterator begin() { return QV.begin(); }

  iterator end() { return QV.end(); }

  const_iterator begin() const { return QV.begin(); }

  const_iterator end() const { return QV.end(); }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual ASTQubitContainerNode *GetElement(unsigned Index) {
    assert(Index < QV.size() && "Index is out-of-range");

    try {
      return QV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTQubitContainerNode *GetElement(unsigned Index) const {
    assert(Index < QV.size() && "Index is out-of-range");

    try {
      return QV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual ASTQubitContainerNode *operator[](unsigned Index) {
    return GetElement(Index);
  }

  virtual const ASTQubitContainerNode *operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTQubitArrayNode *ExpressionError(const std::string &ERM,
                                            const ASTToken *TK) {
    return new ASTQubitArrayNode(ASTIdentifierNode::QubitArray.Clone(), ERM,
                                 TK);
  }

  static ASTQubitArrayNode *ExpressionError(const ASTIdentifierNode *Id,
                                            const std::string &ERM,
                                            const ASTToken *TK) {
    return new ASTQubitArrayNode(Id, ERM, TK);
  }

  virtual void print() const override {
    std::cout << "<ASTQubitArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    for (list_type::const_iterator I = QV.begin(); I != QV.end(); ++I)
      (*I)->print();
    std::cout << "</ASTQubitArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTQubitNArrayNode : public ASTArrayNode {
private:
  std::vector<ASTQubitContainerNode *> QV;
  unsigned QS;

private:
  ASTQubitNArrayNode() = delete;

public:
  using vector_type = std::vector<ASTQubitContainerNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTQubitNArrayNode(const ASTIdentifierNode *Id, unsigned Size, unsigned QSize)
      : ASTArrayNode(Id, ASTTypeQubitNArray, Size), QV(), QS(QSize) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.clear();
      S.str("");
      S << Id->GetName() << ':' << I;
      ASTIdentifierNode *QId = new ASTIdentifierNode(S.str(), QSize);
      assert(QId && "Could not create a valid ASTIdentifierNode!");
      QV.push_back(new ASTQubitContainerNode(QId, 1, QSize, S.str()));
    }
  }

  virtual ~ASTQubitNArrayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeQubitNArray; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override {
    return ASTTypeQubitContainer;
  }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = QV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = QV.data();

    return MM;
  }

  virtual unsigned GetElementSize() const override { return QS; }

  iterator begin() { return QV.begin(); }

  iterator end() { return QV.end(); }

  const_iterator begin() const { return QV.begin(); }

  const_iterator end() const { return QV.end(); }

  virtual ASTQubitContainerNode *GetElement(unsigned Index) {
    assert(Index < QV.size() && "Index is out-of-range!");

    try {
      return QV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTQubitContainerNode *GetElement(unsigned Index) const {
    assert(Index < QV.size() && "Index is out-of-range!");

    try {
      return QV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  inline ASTQubitContainerNode *operator[](unsigned Index) {
    return GetElement(Index);
  }

  inline const ASTQubitContainerNode *operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual void print() const override {
    std::cout << "<QubitNArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    unsigned X = 0;
    for (std::vector<ASTQubitContainerNode *>::const_iterator I = QV.begin();
         I != QV.end(); ++I) {
      std::cout << "<Index>" << X++ << "</Index>" << std::endl;
      (*I)->print();
    }

    std::cout << "</QubitNArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTAngleArrayNode : public ASTArrayNode {
private:
  std::vector<ASTAngleNode *> AV;
  unsigned AS;

private:
  ASTAngleArrayNode() = delete;

protected:
  ASTAngleArrayNode(const ASTIdentifierNode *Id, const std::string &ERM,
                    const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeAngleArray), AV(), AS(0U) {
    SetLocation(TK->GetLocation());
  }

public:
  using vector_type = std::vector<ASTAngleNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTAngleArrayNode(const ASTIdentifierNode *Id, unsigned Size)
      : ASTArrayNode(Id, ASTTypeAngleArray, Size), AV(),
        AS(ASTAngleNode::AngleBits) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-angle-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *AId =
          new ASTIdentifierNode(S.str(), ASTAngleNode::AngleBits);
      assert(AId && "Could not create a valid ASTIdentifierNode!");

      AId->SetPolymorphicName("arrayangle");
      AV.push_back(new ASTAngleNode(AId));
      AV.back()->Mangle();
    }
  }

  ASTAngleArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                    const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeAngleArray, Size, IL), AV(),
        AS(ASTAngleNode::AngleBits) {}

  ASTAngleArrayNode(const ASTIdentifierNode *Id, unsigned Size, unsigned Bits)
      : ASTArrayNode(Id, ASTTypeAngleArray, Size), AV(), AS(Bits) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-angle-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *AId = new ASTIdentifierNode(S.str(), Bits);
      assert(AId && "Could not create a valid ASTIdentifierNode!");

      AId->SetPolymorphicName("arrayangle");
      AV.push_back(new ASTAngleNode(AId, ASTAngleTypeGeneric, Bits));
      AV.back()->Mangle();
    }
  }

  ASTAngleArrayNode(const ASTIdentifierNode *Id, unsigned Size, unsigned Bits,
                    const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeAngleArray, Size, IL), AV(), AS(Bits) {}

  virtual ~ASTAngleArrayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeAngleArray; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override { return ASTTypeAngle; }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = AV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = AV.data();

    return MM;
  }

  iterator begin() { return AV.begin(); }

  iterator end() { return AV.end(); }

  const_iterator begin() const { return AV.begin(); }

  const_iterator end() const { return AV.end(); }

  virtual unsigned GetElementSize() const override { return AS; }

  virtual ASTAngleNode *GetElement(unsigned Index) {
    assert(Index < AV.size() && "Index is out-of-range!");

    try {
      return AV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTAngleNode *GetElement(unsigned Index) const {
    assert(Index < AV.size() && "Index is out-of-range!");

    try {
      return AV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  ASTAngleNode *operator[](unsigned Index) { return GetElement(Index); }

  const ASTAngleNode *operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTAngleArrayNode *ExpressionError(const std::string &ERM,
                                            const ASTToken *TK) {
    return new ASTAngleArrayNode(ASTIdentifierNode::AngleArray.Clone(), ERM,
                                 TK);
  }

  static ASTAngleArrayNode *ExpressionError(const ASTIdentifierNode *Id,
                                            const std::string &ERM,
                                            const ASTToken *TK) {
    return new ASTAngleArrayNode(Id, ERM, TK);
  }

  virtual void print() const override {
    std::cout << "<ASTAngleArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    unsigned X = 0;
    for (std::vector<ASTAngleNode *>::const_iterator I = AV.begin();
         I != AV.end(); ++I) {
      std::cout << "<Index>" << X++ << "</Index>" << std::endl;
      (*I)->print();
    }

    std::cout << "</ASTAngleArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTBoolArrayNode : public ASTArrayNode {
private:
  std::vector<ASTBoolNode *> BV;

private:
  ASTBoolArrayNode() = delete;

protected:
  ASTBoolArrayNode(const ASTIdentifierNode *Id, const std::string &ERM,
                   const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeBoolArray), BV() {
    SetLocation(TK->GetLocation());
  }

public:
  using vector_type = std::vector<ASTBoolNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTBoolArrayNode(const ASTIdentifierNode *Id, unsigned Size)
      : ASTArrayNode(Id, ASTTypeBoolArray, Size), BV() {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-bool-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *BId = new ASTIdentifierNode(S.str(), 8);
      assert(BId && "Could not create a valid ASTIdentifierNode!");

      BV.push_back(new ASTBoolNode(BId, false));
    }
  }

  ASTBoolArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                   const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeBoolArray, Size, IL), BV() {}

  ASTBoolArrayNode(const ASTIdentifierNode *Id, unsigned Size, bool Value)
      : ASTArrayNode(Id, ASTTypeBoolArray, Size), BV() {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-bool-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *BId = new ASTIdentifierNode(S.str(), 8);
      assert(BId && "Could not create a valid ASTIdentifierNode!");

      BV.push_back(new ASTBoolNode(BId, Value));
    }
  }

  virtual ~ASTBoolArrayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeBoolArray; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override { return ASTTypeBool; }

  virtual unsigned GetElementSize() const override {
    return ASTBoolNode::BoolBits;
  }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = BV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = BV.data();

    return MM;
  }

  iterator begin() { return BV.begin(); }

  iterator end() { return BV.end(); }

  const_iterator begin() const { return BV.begin(); }

  const_iterator end() const { return BV.end(); }

  virtual ASTBoolNode *GetElement(unsigned Index) {
    assert(Index < BV.size() && "Index is out-of-range!");

    try {
      return BV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTBoolNode *GetElement(unsigned Index) const {
    assert(Index < BV.size() && "Index is out-of-range!");

    try {
      return BV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  inline ASTBoolNode *operator[](unsigned Index) { return GetElement(Index); }

  inline const ASTBoolNode *operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTBoolArrayNode *ExpressionError(const std::string &ERM,
                                           const ASTToken *TK) {
    return new ASTBoolArrayNode(ASTIdentifierNode::BoolArray.Clone(), ERM, TK);
  }

  static ASTBoolArrayNode *ExpressionError(const ASTIdentifierNode *Id,
                                           const std::string &ERM,
                                           const ASTToken *TK) {
    return new ASTBoolArrayNode(Id, ERM, TK);
  }

  virtual void print() const override {
    std::cout << "<ASTBoolArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    unsigned X = 0;
    for (std::vector<ASTBoolNode *>::const_iterator I = BV.begin();
         I != BV.end(); ++I) {
      std::cout << "<Index>" << X++ << "</Index>" << std::endl;
      (*I)->print();
    }

    std::cout << "</ASTBoolArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTIntArrayNode : public ASTArrayNode {
private:
  std::vector<ASTIntNode *> IV;
  bool US;

protected:
  ASTIntArrayNode(const ASTIdentifierNode *Id, const std::string &ERM,
                  bool Unsigned, const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeIntArray), IV(), US(Unsigned) {
    SetLocation(TK->GetLocation());
  }

private:
  ASTIntArrayNode() = delete;

public:
  using vector_type = std::vector<ASTIntNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTIntArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                  bool Unsigned = false)
      : ASTArrayNode(Id, ASTTypeIntArray, Size), IV(), US(Unsigned) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-int-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId =
          new ASTIdentifierNode(S.str(), ASTIntNode::IntBits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      if (Unsigned)
        IV.push_back(new ASTIntNode(IId, uint32_t(0)));
      else
        IV.push_back(new ASTIntNode(IId, int32_t(0)));
    }
  }

  ASTIntArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                  const ASTInitializerList *IL, bool Unsigned = false)
      : ASTArrayNode(Id, ASTTypeIntArray, Size, IL), IV(), US(Unsigned) {}

  ASTIntArrayNode(const ASTIdentifierNode *Id, unsigned Size, int32_t Value)
      : ASTArrayNode(Id, ASTTypeIntArray, Size), IV(), US(false) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-int-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId =
          new ASTIdentifierNode(S.str(), ASTIntNode::IntBits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      IV.push_back(new ASTIntNode(IId, Value));
    }
  }

  ASTIntArrayNode(const ASTIdentifierNode *Id, unsigned Size, uint32_t Value)
      : ASTArrayNode(Id, ASTTypeIntArray, Size), IV(), US(true) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-int-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId =
          new ASTIdentifierNode(S.str(), ASTIntNode::IntBits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      IV.push_back(new ASTIntNode(IId, Value));
    }
  }

  virtual ~ASTIntArrayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeIntArray; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override {
    return IsSigned() ? ASTTypeInt : ASTTypeUInt;
  }

  virtual unsigned GetElementSize() const override {
    return ASTIntNode::IntBits;
  }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = IV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = IV.data();

    return MM;
  }

  iterator begin() { return IV.begin(); }

  iterator end() { return IV.end(); }

  const_iterator begin() const { return IV.begin(); }

  const_iterator end() const { return IV.end(); }

  virtual bool IsSigned() const { return !US; }

  virtual ASTIntNode *GetElement(unsigned Index) {
    assert(Index < IV.size() && "Index is out-of-range!");

    try {
      return IV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTIntNode *GetElement(unsigned Index) const {
    assert(Index < IV.size() && "Index is out-of-range!");

    try {
      return IV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  ASTIntNode *operator[](unsigned Index) { return GetElement(Index); }

  const ASTIntNode *operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTIntArrayNode *ExpressionError(const std::string &ERM, bool Unsigned,
                                          const ASTToken *TK) {
    return new ASTIntArrayNode(ASTIdentifierNode::IntArray.Clone(), ERM,
                               Unsigned, TK);
  }

  static ASTIntArrayNode *ExpressionError(const ASTIdentifierNode *Id,
                                          const std::string &ERM, bool Unsigned,
                                          const ASTToken *TK) {
    return new ASTIntArrayNode(Id, ERM, Unsigned, TK);
  }

  virtual void print() const override {
    std::cout << "<ASTIntArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    unsigned X = 0;
    for (std::vector<ASTIntNode *>::const_iterator I = IV.begin();
         I != IV.end(); ++I) {
      std::cout << "<Index>" << X++ << "</Index>" << std::endl;
      (*I)->print();
    }

    std::cout << "</ASTIntArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTMPIntegerArrayNode : public ASTArrayNode {
private:
  std::vector<ASTMPIntegerNode *> MPV;
  bool US;
  unsigned IB;

protected:
  ASTMPIntegerArrayNode(const ASTIdentifierNode *Id, const std::string &ERM,
                        bool Unsigned, const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeMPIntegerArray), MPV(), US(Unsigned),
        IB(0U) {
    SetLocation(TK->GetLocation());
  }

private:
  ASTMPIntegerArrayNode() = delete;

public:
  using vector_type = std::vector<ASTMPIntegerNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTMPIntegerArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits, bool Unsigned = false)
      : ASTArrayNode(Id, ASTTypeMPIntegerArray, Size), MPV(), US(Unsigned),
        IB(Bits) {
    std::stringstream S;
    ASTSignbit Sign = Unsigned ? ASTSignbit::Unsigned : ASTSignbit::Signed;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-mpinteger-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), Bits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      MPV.push_back(new ASTMPIntegerNode(IId, Sign, Bits));
    }
  }

  ASTMPIntegerArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits, const ASTInitializerList *IL,
                        bool Unsigned = false)
      : ASTArrayNode(Id, ASTTypeMPIntegerArray, Size, IL), MPV(), US(Unsigned),
        IB(Bits) {}

  ASTMPIntegerArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits, const char *String, int Base = 10,
                        bool Unsigned = false)
      : ASTArrayNode(Id, ASTTypeMPIntegerArray, Size), MPV(), US(Unsigned),
        IB(Bits) {
    std::stringstream S;
    ASTSignbit Sign = Unsigned ? ASTSignbit::Unsigned : ASTSignbit::Signed;

    if (String[0] == '-' || String[0] == '+')
      Sign = Signed;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-mpinteger-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), Bits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      MPV.push_back(new ASTMPIntegerNode(IId, Sign, Bits, String, Base));
    }
  }

  virtual ~ASTMPIntegerArrayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeMPIntegerArray; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override {
    return IsSigned() ? ASTTypeMPInteger : ASTTypeMPUInteger;
  }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = MPV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = MPV.data();

    return MM;
  }

  iterator begin() { return MPV.begin(); }

  iterator end() { return MPV.end(); }

  const_iterator begin() const { return MPV.begin(); }

  const_iterator end() const { return MPV.end(); }

  virtual bool IsSigned() const { return !US; }

  virtual unsigned GetElementSize() const override { return IB; }

  virtual ASTMPIntegerNode *GetElement(unsigned Index) {
    assert(Index < MPV.size() && "Index is out-of-range!");

    try {
      return MPV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTMPIntegerNode *GetElement(unsigned Index) const {
    assert(Index < MPV.size() && "Index is out-of-range!");

    try {
      return MPV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  ASTMPIntegerNode *operator[](unsigned Index) { return GetElement(Index); }

  const ASTMPIntegerNode *operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTMPIntegerArrayNode *
  ExpressionError(const std::string &ERM, bool Unsigned, const ASTToken *TK) {
    return new ASTMPIntegerArrayNode(ASTIdentifierNode::MPIntArray.Clone(), ERM,
                                     Unsigned, TK);
  }

  static ASTMPIntegerArrayNode *ExpressionError(const ASTIdentifierNode *Id,
                                                const std::string &ERM,
                                                bool Unsigned,
                                                const ASTToken *TK) {
    return new ASTMPIntegerArrayNode(Id, ERM, Unsigned, TK);
  }

  virtual void print() const override {
    std::cout << "<ASTMPIntegerArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    unsigned X = 0;
    for (std::vector<ASTMPIntegerNode *>::const_iterator I = MPV.begin();
         I != MPV.end(); ++I) {
      std::cout << "<Index>" << X++ << "</Index>" << std::endl;
      (*I)->print();
    }

    std::cout << "</ASTMPIntegerArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTFloatArrayNode : public ASTArrayNode {
private:
  std::vector<ASTFloatNode *> FV;

private:
  ASTFloatArrayNode() = delete;

protected:
  ASTFloatArrayNode(const ASTIdentifierNode *Id, const std::string &ERM,
                    const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeFloatArray), FV() {
    SetLocation(TK->GetLocation());
  }

public:
  using vector_type = std::vector<ASTFloatNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTFloatArrayNode(const ASTIdentifierNode *Id, unsigned Size)
      : ASTArrayNode(Id, ASTTypeFloatArray, Size), FV() {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-float-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), 32);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      FV.push_back(new ASTFloatNode(IId, 0.0f));
    }
  }

  ASTFloatArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                    const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeFloatArray, Size, IL), FV() {}

  ASTFloatArrayNode(const ASTIdentifierNode *Id, unsigned Size, float Value)
      : ASTArrayNode(Id, ASTTypeFloatArray, Size), FV() {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-float-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), 32);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      FV.push_back(new ASTFloatNode(IId, Value));
    }
  }

  virtual ~ASTFloatArrayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeFloatArray; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override { return ASTTypeFloat; }

  virtual unsigned GetElementSize() const override {
    return ASTFloatNode::FloatBits;
  }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = FV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = FV.data();

    return MM;
  }

  iterator begin() { return FV.begin(); }

  iterator end() { return FV.end(); }

  const_iterator begin() const { return FV.begin(); }

  const_iterator end() const { return FV.end(); }

  virtual ASTFloatNode *GetElement(unsigned Index) {
    assert(Index < FV.size() && "Index is out-of-range!");

    try {
      return FV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTFloatNode *GetElement(unsigned Index) const {
    assert(Index < FV.size() && "Index is out-of-range!");

    try {
      return FV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  inline ASTFloatNode *operator[](unsigned Index) { return GetElement(Index); }

  inline const ASTFloatNode *operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTFloatArrayNode *ExpressionError(const std::string &ERM,
                                            const ASTToken *TK) {
    return new ASTFloatArrayNode(ASTIdentifierNode::FloatArray.Clone(), ERM,
                                 TK);
  }

  static ASTFloatArrayNode *ExpressionError(const ASTIdentifierNode *Id,
                                            const std::string &ERM,
                                            const ASTToken *TK) {
    return new ASTFloatArrayNode(Id, ERM, TK);
  }

  virtual void print() const override {
    std::cout << "<ASTFloatArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    unsigned X = 0;
    for (std::vector<ASTFloatNode *>::const_iterator I = FV.begin();
         I != FV.end(); ++I) {
      std::cout << "<Index>" << X++ << "</Index>" << std::endl;
      (*I)->print();
    }

    std::cout << "</ASTFloatArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTMPDecimalArrayNode : public ASTArrayNode {
private:
  std::vector<ASTMPDecimalNode *> MPV;
  unsigned DB;

private:
  ASTMPDecimalArrayNode() = delete;

protected:
  ASTMPDecimalArrayNode(const ASTIdentifierNode *Id, const std::string &ERM,
                        const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeMPDecimalArray), MPV() {
    SetLocation(TK->GetLocation());
  }

public:
  using vector_type = std::vector<ASTMPDecimalNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTMPDecimalArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits)
      : ASTArrayNode(Id, ASTTypeMPDecimalArray, Size), MPV(), DB(Bits) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-mpdecimal-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), Bits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      MPV.push_back(new ASTMPDecimalNode(IId, Bits));
    }
  }

  ASTMPDecimalArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits, const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeMPDecimalArray, Size, IL), MPV(), DB(Bits) {}

  ASTMPDecimalArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits, const char *String, int Base = 10)
      : ASTArrayNode(Id, ASTTypeMPDecimalArray, Size), MPV(), DB(Bits) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-mpdecimal-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), Bits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      MPV.push_back(new ASTMPDecimalNode(IId, Bits, String, Base));
    }
  }

  virtual ~ASTMPDecimalArrayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeMPDecimalArray; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override { return ASTTypeMPDecimal; }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = MPV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = MPV.data();

    return MM;
  }

  iterator begin() { return MPV.begin(); }

  iterator end() { return MPV.end(); }

  const_iterator begin() const { return MPV.begin(); }

  const_iterator end() const { return MPV.end(); }

  virtual unsigned GetElementSize() const override { return DB; }

  virtual ASTMPDecimalNode *GetElement(unsigned Index) {
    assert(Index < MPV.size() && "Index is out-of-range!");

    try {
      return MPV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTMPDecimalNode *GetElement(unsigned Index) const {
    assert(Index < MPV.size() && "Index is out-of-range!");

    try {
      return MPV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  inline ASTMPDecimalNode *operator[](unsigned Index) {
    return GetElement(Index);
  }

  inline const ASTMPDecimalNode *operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTMPDecimalArrayNode *ExpressionError(const std::string &ERM,
                                                const ASTToken *TK) {
    return new ASTMPDecimalArrayNode(ASTIdentifierNode::MPDecArray.Clone(), ERM,
                                     TK);
  }

  static ASTMPDecimalArrayNode *ExpressionError(const ASTIdentifierNode *Id,
                                                const std::string &ERM,
                                                const ASTToken *TK) {
    return new ASTMPDecimalArrayNode(Id, ERM, TK);
  }

  virtual void print() const override {
    std::cout << "<ASTMPDecimalArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    unsigned X = 0;
    for (std::vector<ASTMPDecimalNode *>::const_iterator I = MPV.begin();
         I != MPV.end(); ++I) {
      std::cout << "<Index>" << X++ << "</Index>" << std::endl;
      (*I)->print();
    }

    std::cout << "</ASTMPDecimalArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTMPComplexArrayNode : public ASTArrayNode {
private:
  std::vector<ASTMPComplexNode *> MPV;
  unsigned CB;
  ASTType CTy;
  unsigned CTB;

private:
  ASTMPComplexArrayNode() = delete;

protected:
  ASTMPComplexArrayNode(const ASTIdentifierNode *Id, const std::string &ERM,
                        const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeMPComplexArray), MPV(), CB(0U),
        CTy(ASTTypeMPDecimal), CTB(0U) {
    SetLocation(TK->GetLocation());
  }

public:
  using vector_type = std::vector<ASTMPComplexNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTMPComplexArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits)
      : ASTArrayNode(Id, ASTTypeMPComplexArray, Size), MPV(), CB(Bits),
        CTy(ASTTypeMPDecimal), CTB(Bits) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-mpcomplex-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), Bits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      MPV.push_back(new ASTMPComplexNode(IId, Bits));
    }
  }

  ASTMPComplexArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits, const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeMPComplexArray, Size, IL), MPV(), CB(Bits),
        CTy(ASTTypeMPDecimal), CTB(Bits) {}

  ASTMPComplexArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits, const char *String)
      : ASTArrayNode(Id, ASTTypeMPComplexArray, Size), MPV(), CB(Bits),
        CTy(ASTTypeMPDecimal), CTB(Bits) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-mpcomplex-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), Bits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      MPV.push_back(new ASTMPComplexNode(IId, String, Bits));
    }
  }

  ASTMPComplexArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits, const ASTComplexExpressionNode *CE)
      : ASTArrayNode(Id, ASTTypeMPComplexArray, Size), MPV(), CB(Bits),
        CTy(ASTTypeMPDecimal), CTB(Bits) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-mpcomplex-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), Bits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      MPV.push_back(new ASTMPComplexNode(IId, CE, Bits));
    }
  }

  ASTMPComplexArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits, const ASTFunctionCallNode *FN)
      : ASTArrayNode(Id, ASTTypeMPComplexArray, Size), MPV(), CB(Bits),
        CTy(ASTTypeMPDecimal), CTB(Bits) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-mpcomplex-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), Bits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      MPV.push_back(new ASTMPComplexNode(IId, FN, Bits));
    }
  }

  ASTMPComplexArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits, const ASTMPDecimalNode *RD,
                        const ASTMPDecimalNode *ID, ASTOpType OT)
      : ASTArrayNode(Id, ASTTypeMPComplexArray, Size), MPV(), CB(Bits),
        CTy(ASTTypeMPDecimal), CTB(std::max(RD->GetBits(), ID->GetBits())) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-mpcomplex-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), Bits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      MPV.push_back(new ASTMPComplexNode(IId, RD, ID, OT, Bits));
    }
  }

  ASTMPComplexArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits, const ASTMPIntegerNode *RD,
                        const ASTMPIntegerNode *ID, ASTOpType OT)
      : ASTArrayNode(Id, ASTTypeMPComplexArray, Size), MPV(), CB(Bits),
        CTy(ASTTypeMPInteger), CTB(std::max(RD->GetBits(), ID->GetBits())) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-mpcomplex-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), Bits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      MPV.push_back(new ASTMPComplexNode(IId, RD, ID, OT, Bits));
    }
  }

  ASTMPComplexArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits, const ASTMPIntegerNode *RD,
                        const ASTMPDecimalNode *ID, ASTOpType OT)
      : ASTArrayNode(Id, ASTTypeMPComplexArray, Size), MPV(), CB(Bits),
        CTy(ASTTypeMPDecimal), CTB(std::max(RD->GetBits(), ID->GetBits())) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-mpcomplex-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), Bits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      MPV.push_back(new ASTMPComplexNode(IId, RD, ID, OT, Bits));
    }
  }

  ASTMPComplexArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned Bits, const ASTMPDecimalNode *RD,
                        const ASTMPIntegerNode *ID, ASTOpType OT)
      : ASTArrayNode(Id, ASTTypeMPComplexArray, Size), MPV(), CB(Bits),
        CTy(ASTTypeMPDecimal), CTB(std::max(RD->GetBits(), ID->GetBits())) {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S << "ast-array-mpcomplex-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(S.str(), Bits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      MPV.push_back(new ASTMPComplexNode(IId, RD, ID, OT, Bits));
    }
  }

  virtual ~ASTMPComplexArrayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeMPComplexArray; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override { return ASTTypeMPComplex; }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = MPV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = MPV.data();

    return MM;
  }

  virtual void SetElementType(ASTType Ty) { CTy = Ty; }

  virtual void SetElementBits(unsigned EB) { CTB = EB; }

  iterator begin() { return MPV.begin(); }

  iterator end() { return MPV.end(); }

  const_iterator begin() const { return MPV.begin(); }

  const_iterator end() const { return MPV.end(); }

  virtual unsigned GetElementSize() const override { return CB; }

  virtual ASTType GetComplexElementType() const { return CTy; }

  virtual unsigned GetComplexElementBits() const { return CTB; }

  virtual ASTMPComplexNode *GetElement(unsigned Index) {
    assert(Index < MPV.size() && "Index is out-of-range!");
    try {
      return MPV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTMPComplexNode *GetElement(unsigned Index) const {
    assert(Index < MPV.size() && "Index is out-of-range!");
    try {
      return MPV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  inline ASTMPComplexNode *operator[](unsigned Index) {
    return GetElement(Index);
  }

  inline const ASTMPComplexNode *operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTMPComplexArrayNode *ExpressionError(const std::string &ERM,
                                                const ASTToken *TK) {
    return new ASTMPComplexArrayNode(ASTIdentifierNode::MPComplexArray.Clone(),
                                     ERM, TK);
  }

  static ASTMPComplexArrayNode *ExpressionError(const ASTIdentifierNode *Id,
                                                const std::string &ERM,
                                                const ASTToken *TK) {
    return new ASTMPComplexArrayNode(Id, ERM, TK);
  }

  virtual void print() const override {
    std::cout << "<ASTMPComplexArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    unsigned X = 0;
    for (std::vector<ASTMPComplexNode *>::const_iterator I = MPV.begin();
         I != MPV.end(); ++I) {
      std::cout << "<Index>" << X++ << "</Index>" << std::endl;
      (*I)->print();
    }

    std::cout << "</ASTMPComplexArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTDurationArrayNode : public ASTArrayNode {
private:
  std::vector<ASTDurationNode *> DV;
  std::string LU;

private:
  ASTDurationArrayNode() = delete;

protected:
  ASTDurationArrayNode(const ASTIdentifierNode *Id, const std::string &ERM,
                       const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeDurationArray), DV(), LU() {
    SetLocation(TK->GetLocation());
  }

public:
  using vector_type = std::vector<ASTDurationNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTDurationArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                       const std::string &LengthUnit)
      : ASTArrayNode(Id, ASTTypeDurationArray, Size), DV(), LU(LengthUnit) {
    std::stringstream S;

    if (LengthUnit == u8"dt" || LengthUnit == u8"DT")
      LU = u8"0dt";

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S.clear();
      S << "ast-array-duration-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(
          S.str(), ASTTypeDuration, ASTDurationNode::DurationBits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      DV.push_back(new ASTDurationNode(IId, LU));
    }
  }

  ASTDurationArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                       const ASTDurationOfNode *DON)
      : ASTArrayNode(Id, ASTTypeDurationArray, Size), DV(), LU("0dt") {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.str("");
      S.clear();
      S << "ast-array-duration-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(
          S.str(), ASTTypeDuration, ASTDurationNode::DurationBits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");

      ASTDurationNode *DN = new ASTDurationNode(IId, DON);
      assert(DN && "Could not create a valid ASTDurationNode!");

      DN->SetLengthUnit(DT);
      DN->SetDuration(0UL);
      DV.push_back(DN);
    }
  }

  ASTDurationArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                       const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeDurationArray, Size, IL), DV(), LU("0dt") {}

  virtual ~ASTDurationArrayNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeDurationArray; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override { return ASTTypeDuration; }

  virtual unsigned GetElementSize() const override {
    return ASTDurationNode::DurationBits;
  }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = DV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = DV.data();

    return MM;
  }

  iterator begin() { return DV.begin(); }

  iterator end() { return DV.end(); }

  const_iterator begin() const { return DV.begin(); }

  const_iterator end() const { return DV.end(); }

  virtual const std::string &GetLengthUnit() const { return LU; }

  virtual ASTDurationNode *GetElement(unsigned Index) {
    assert(Index < DV.size() && "Index is out-of-range!");

    try {
      return DV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const ASTDurationNode *GetElement(unsigned Index) const {
    assert(Index < DV.size() && "Index is out-of-range!");

    try {
      return DV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  inline ASTDurationNode *operator[](unsigned Index) {
    return GetElement(Index);
  }

  inline const ASTDurationNode *operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTDurationArrayNode *ExpressionError(const std::string &ERM,
                                               const ASTToken *TK) {
    return new ASTDurationArrayNode(ASTIdentifierNode::DurationArray.Clone(),
                                    ERM, TK);
  }

  static ASTDurationArrayNode *ExpressionError(const ASTIdentifierNode *Id,
                                               const std::string &ERM,
                                               const ASTToken *TK) {
    return new ASTDurationArrayNode(Id, ERM, TK);
  }

  virtual void print() const override {
    std::cout << "<ASTDurationArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    unsigned X = 0;
    for (std::vector<ASTDurationNode *>::const_iterator I = DV.begin();
         I != DV.end(); ++I) {
      std::cout << "<Index>" << X++ << "</Index>" << std::endl;
      (*I)->print();
    }

    std::cout << "</ASTDurationArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTOpenPulseFrameArrayNode : public ASTArrayNode {
private:
  std::vector<OpenPulse::ASTOpenPulseFrameNode *> FV;

private:
  ASTOpenPulseFrameArrayNode() = delete;

protected:
  ASTOpenPulseFrameArrayNode(const ASTIdentifierNode *Id,
                             const std::string &ERM, const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeOpenPulseFrameArray), FV() {
    SetLocation(TK->GetLocation());
  }

public:
  using vector_type = std::vector<OpenPulse::ASTOpenPulseFrameNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTOpenPulseFrameArrayNode(const ASTIdentifierNode *Id, unsigned Size)
      : ASTArrayNode(Id, ASTTypeOpenPulseFrameArray, Size), FV() {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.clear();
      S.str("");
      S << "ast-array-frame-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId =
          new ASTIdentifierNode(S.str(), ASTTypeOpenPulseFrame,
                                OpenPulse::ASTOpenPulseFrameNode::FrameBits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");
      FV.push_back(new OpenPulse::ASTOpenPulseFrameNode(IId));
    }
  }

  ASTOpenPulseFrameArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                             const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeOpenPulseFrameArray, Size, IL), FV() {}

  virtual ~ASTOpenPulseFrameArrayNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeOpenPulseFrameArray;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override {
    return ASTTypeOpenPulseFrame;
  }

  virtual unsigned GetElementSize() const override {
    return OpenPulse::ASTOpenPulseFrameNode::FrameBits;
  }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = FV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = FV.data();

    return MM;
  }

  iterator begin() { return FV.begin(); }

  iterator end() { return FV.end(); }

  const_iterator begin() const { return FV.begin(); }

  const_iterator end() const { return FV.end(); }

  virtual OpenPulse::ASTOpenPulseFrameNode *GetElement(unsigned Index) {
    assert(Index < FV.size() && "Index is out-of-range!");

    try {
      return FV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const OpenPulse::ASTOpenPulseFrameNode *
  GetElement(unsigned Index) const {
    assert(Index < FV.size() && "Index is out-of-range!");

    try {
      return FV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  inline OpenPulse::ASTOpenPulseFrameNode *operator[](unsigned Index) {
    return GetElement(Index);
  }

  inline const OpenPulse::ASTOpenPulseFrameNode *
  operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTOpenPulseFrameArrayNode *ExpressionError(const std::string &ERM,
                                                     const ASTToken *TK) {
    return new ASTOpenPulseFrameArrayNode(ASTIdentifierNode::FrameArray.Clone(),
                                          ERM, TK);
  }

  static ASTOpenPulseFrameArrayNode *
  ExpressionError(const ASTIdentifierNode *Id, const std::string &ERM,
                  const ASTToken *TK) {
    return new ASTOpenPulseFrameArrayNode(Id, ERM, TK);
  }

  virtual void print() const override {
    std::cout << "<ASTOpenPulseFrameArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    unsigned X = 0;
    for (std::vector<OpenPulse::ASTOpenPulseFrameNode *>::const_iterator I =
             FV.begin();
         I != FV.end(); ++I) {
      std::cout << "<Index>" << X++ << "</Index>" << std::endl;
      (*I)->print();
    }

    std::cout << "</ASTOpenPulseFrameArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTOpenPulsePortArrayNode : public ASTArrayNode {
private:
  std::vector<OpenPulse::ASTOpenPulsePortNode *> PV;

private:
  ASTOpenPulsePortArrayNode() = delete;

protected:
  ASTOpenPulsePortArrayNode(const ASTIdentifierNode *Id, const std::string &ERM,
                            const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeOpenPulsePortArray), PV() {
    SetLocation(TK->GetLocation());
  }

public:
  using vector_type = std::vector<OpenPulse::ASTOpenPulsePortNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTOpenPulsePortArrayNode(const ASTIdentifierNode *Id, unsigned Size)
      : ASTArrayNode(Id, ASTTypeOpenPulsePortArray, Size), PV() {
    std::stringstream S;

    for (unsigned I = 0; I < Size; ++I) {
      S.clear();
      S.str("");
      S << "ast-array-port-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId =
          new ASTIdentifierNode(S.str(), ASTTypeOpenPulsePort,
                                OpenPulse::ASTOpenPulsePortNode::PortBits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");
      PV.push_back(new OpenPulse::ASTOpenPulsePortNode(IId));
    }
  }

  ASTOpenPulsePortArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                            const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeOpenPulsePortArray, Size, IL), PV() {}

  virtual ~ASTOpenPulsePortArrayNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeOpenPulsePortArray;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override {
    return ASTTypeOpenPulsePort;
  }

  virtual unsigned GetElementSize() const override {
    return OpenPulse::ASTOpenPulsePortNode::PortBits;
  }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = PV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = PV.data();

    return MM;
  }

  iterator begin() { return PV.begin(); }

  iterator end() { return PV.end(); }

  const_iterator begin() const { return PV.begin(); }

  const_iterator end() const { return PV.end(); }

  virtual OpenPulse::ASTOpenPulsePortNode *GetElement(unsigned Index) {
    assert(Index < PV.size() && "Index is out-of-range!");

    try {
      return PV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const OpenPulse::ASTOpenPulsePortNode *
  GetElement(unsigned Index) const {
    assert(Index < PV.size() && "Index is out-of-range!");

    try {
      return PV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  inline OpenPulse::ASTOpenPulsePortNode *operator[](unsigned Index) {
    return GetElement(Index);
  }

  inline const OpenPulse::ASTOpenPulsePortNode *
  operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTOpenPulsePortArrayNode *ExpressionError(const std::string &ERM,
                                                    const ASTToken *TK) {
    return new ASTOpenPulsePortArrayNode(ASTIdentifierNode::PortArray.Clone(),
                                         ERM, TK);
  }

  static ASTOpenPulsePortArrayNode *ExpressionError(const ASTIdentifierNode *Id,
                                                    const std::string &ERM,
                                                    const ASTToken *TK) {
    return new ASTOpenPulsePortArrayNode(Id, ERM, TK);
  }

  virtual void print() const override {
    std::cout << "<ASTOpenPulsePortArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    unsigned X = 0;
    for (std::vector<OpenPulse::ASTOpenPulsePortNode *>::const_iterator I =
             PV.begin();
         I != PV.end(); ++I) {
      std::cout << "<Index>" << X++ << "</Index>" << std::endl;
      (*I)->print();
    }

    std::cout << "</ASTOpenPulsePortArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTOpenPulseWaveformArrayNode : public ASTArrayNode {
private:
  std::vector<OpenPulse::ASTOpenPulseWaveformNode *> WV;

private:
  ASTOpenPulseWaveformArrayNode() = delete;

protected:
  ASTOpenPulseWaveformArrayNode(const ASTIdentifierNode *Id,
                                const std::string &ERM, const ASTToken *TK)
      : ASTArrayNode(Id, ERM, ASTTypeOpenPulseWaveformArray), WV() {
    SetLocation(TK->GetLocation());
  }

public:
  using vector_type = std::vector<OpenPulse::ASTOpenPulseWaveformNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTOpenPulseWaveformArrayNode(const ASTIdentifierNode *Id, unsigned Size)
      : ASTArrayNode(Id, ASTTypeOpenPulseWaveformArray, Size), WV() {
    std::stringstream S;
    ASTMPComplexList CPXL;

    for (unsigned I = 0; I < Size; ++I) {
      S.clear();
      S.str("");
      S << "ast-array-waveform-" << Id->GetName() << ':' << I;
      ASTIdentifierNode *IId = new ASTIdentifierNode(
          S.str(), ASTTypeOpenPulseWaveform,
          OpenPulse::ASTOpenPulseWaveformNode::WaveformBits);
      assert(IId && "Could not create a valid ASTIdentifierNode!");
      WV.push_back(new OpenPulse::ASTOpenPulseWaveformNode(IId, CPXL));
    }
  }

  ASTOpenPulseWaveformArrayNode(const ASTIdentifierNode *Id, unsigned Size,
                                const ASTInitializerList *IL)
      : ASTArrayNode(Id, ASTTypeOpenPulseWaveformArray, Size, IL), WV() {}

  virtual ~ASTOpenPulseWaveformArrayNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeOpenPulseWaveformArray;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual ASTType GetElementType() const override {
    return ASTTypeOpenPulseWaveform;
  }

  virtual unsigned GetElementSize() const override {
    return OpenPulse::ASTOpenPulseWaveformNode::WaveformBits;
  }

  virtual bool IsAggregate() const override { return true; }

  virtual std::any &Memory() override {
    if (!MM.has_value())
      MM = WV.data();

    return MM;
  }

  virtual const std::any &Memory() const override {
    if (!MM.has_value())
      MM = WV.data();

    return MM;
  }

  iterator begin() { return WV.begin(); }

  iterator end() { return WV.end(); }

  const_iterator begin() const { return WV.begin(); }

  const_iterator end() const { return WV.end(); }

  virtual OpenPulse::ASTOpenPulseWaveformNode *GetElement(unsigned Index) {
    assert(Index < WV.size() && "Index is out-of-range!");

    try {
      return WV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  virtual const OpenPulse::ASTOpenPulseWaveformNode *
  GetElement(unsigned Index) const {
    assert(Index < WV.size() && "Index is out-of-range!");

    try {
      return WV.at(Index);
    } catch (const std::out_of_range &E) {
      (void)E;
    } catch (...) {
    }

    return nullptr;
  }

  inline OpenPulse::ASTOpenPulseWaveformNode *operator[](unsigned Index) {
    return GetElement(Index);
  }

  inline const OpenPulse::ASTOpenPulseWaveformNode *
  operator[](unsigned Index) const {
    return GetElement(Index);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTOpenPulseWaveformArrayNode *ExpressionError(const std::string &ERM,
                                                        const ASTToken *TK) {
    return new ASTOpenPulseWaveformArrayNode(
        ASTIdentifierNode::PortArray.Clone(), ERM, TK);
  }

  static ASTOpenPulseWaveformArrayNode *
  ExpressionError(const ASTIdentifierNode *Id, const std::string &ERM,
                  const ASTToken *TK) {
    return new ASTOpenPulseWaveformArrayNode(Id, ERM, TK);
  }

  virtual void print() const override {
    std::cout << "<ASTOpenPulseWaveformArray>" << std::endl;
    std::cout << "<ArrayType>" << PrintTypeEnum(GetArrayType())
              << "</ArrayType>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;

    unsigned X = 0;
    for (std::vector<OpenPulse::ASTOpenPulseWaveformNode *>::const_iterator I =
             WV.begin();
         I != WV.end(); ++I) {
      std::cout << "<Index>" << X++ << "</Index>" << std::endl;
      (*I)->print();
    }

    std::cout << "</ASTOpenPulseWaveformArray>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_ARRAY_H
