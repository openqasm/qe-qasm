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

#ifndef __QASM_AST_QUBIT_H
#define __QASM_AST_QUBIT_H

#include <qasm/AST/ASTIntegerList.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTTypes.h>

#include <string>
#include <vector>

namespace QASM {

class ASTQubitNode : public ASTExpressionNode {
private:
  unsigned Index;
  std::string GQN;

private:
  ASTQubitNode() = delete;

protected:
  ASTQubitNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        Index(static_cast<unsigned>(~0x0)), GQN(ERM) {}

public:
  static const unsigned QubitBits = 1U;

public:
  ASTQubitNode(const ASTIdentifierNode *Id,
               unsigned Idx = static_cast<unsigned>(~0x0))
      : ASTExpressionNode(Id, ASTTypeQubit), Index(Idx), GQN() {}

  ASTQubitNode(const ASTIdentifierNode *Id, unsigned Idx, const std::string &QN)
      : ASTExpressionNode(Id, ASTTypeQubit), Index(Idx), GQN(QN) {}

  virtual ~ASTQubitNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeQubit; }

  virtual ASTType GetQubitType() const { return Type; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual bool IsIndexed() const {
    return Index != static_cast<unsigned>(~0x0);
  }

  virtual unsigned GetIndex() const { return Index; }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual const std::string &GetMangledName() const override {
    return ASTExpressionNode::Ident->GetMangledName();
  }

  virtual void SetGateQubitName(const std::string &GN) { GQN = GN; }

  virtual const std::string &GetGateQubitName() const { return GQN; }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTQubitNode *ExpressionError(const ASTIdentifierNode *Id,
                                       const std::string &ERM) {
    return new ASTQubitNode(Id, ERM);
  }

  static ASTQubitNode *ExpressionError(const std::string &ERM) {
    return new ASTQubitNode(ASTIdentifierNode::Qubit.Clone(), ERM);
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTGateQubitParamNode : public ASTExpressionNode {
private:
  unsigned Bits;
  unsigned IX;
  std::string QName;

private:
  ASTGateQubitParamNode() = delete;

protected:
  ASTGateQubitParamNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        Bits(0U), IX(static_cast<unsigned>(~0x0)), QName(ERM) {}

public:
  ASTGateQubitParamNode(const ASTIdentifierNode *Id)
      : ASTExpressionNode(Id, ASTTypeGateQubitParam), Bits(0U),
        IX(static_cast<unsigned>(~0x0)), QName() {}

  ASTGateQubitParamNode(const ASTIdentifierNode *Id, unsigned Index,
                        unsigned QBits, const std::string &Name)
      : ASTExpressionNode(Id, ASTTypeGateQubitParam), Bits(QBits), IX(Index),
        QName(Name) {}

  virtual ~ASTGateQubitParamNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeGateQubitParam; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual unsigned GetBits() const { return Bits; }

  virtual unsigned GetIndex() const { return IX; }

  virtual const std::string &GetQubitName() const { return QName; }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::GetName();
  }

  virtual const std::string &GetMangledName() const override {
    return ASTExpressionNode::GetMangledName();
  }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTGateQubitParamNode *ExpressionError(const ASTIdentifierNode *Id,
                                                const std::string &ERM) {
    return new ASTGateQubitParamNode(Id, ERM);
  }

  static ASTGateQubitParamNode *ExpressionError(const std::string &ERM) {
    return new ASTGateQubitParamNode(ASTIdentifierNode::QubitParam.Clone(),
                                     ERM);
  }

  virtual void print() const override {
    std::cout << "<QubitParameter>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</Name>" << std::endl;
    std::cout << "</QubitParameter>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTQubitContainerNode;

class ASTQubitContainerAliasNode : public ASTExpressionNode {
  friend class ASTQubitBuilder;
  friend class ASTQubitContainerNode;

private:
  const ASTIdentifierNode *IdOf;
  unsigned Bits;
  mutable unsigned AliasIndex;
  std::vector<ASTQubitNode *> QAL;
  mutable std::vector<const ASTIdentifierNode *> IdAliasList;

private:
  ASTQubitContainerAliasNode() = delete;

protected:
  ASTQubitContainerAliasNode(const ASTIdentifierNode *Id,
                             const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        IdOf(nullptr), Bits(0U), AliasIndex(static_cast<unsigned>(~0x0)), QAL(),
        IdAliasList() {}

public:
  static const unsigned QubitContainerAliasBits = 1U;

public:
  using list_type = std::vector<ASTQubitNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTQubitContainerAliasNode(const ASTIdentifierNode *Id, unsigned Index)
      : ASTExpressionNode(Id, ASTTypeQubitContainerAlias), IdOf(nullptr),
        Bits(0), AliasIndex(Index), QAL(), IdAliasList() {}

  ASTQubitContainerAliasNode(const ASTIdentifierNode *Id,
                             const ASTIdentifierNode *CId, unsigned Index,
                             const std::vector<ASTQubitNode *> &Qubits)
      : ASTExpressionNode(Id, ASTTypeQubitContainerAlias), IdOf(CId),
        Bits(Qubits.size()), AliasIndex(Index), QAL(Qubits), IdAliasList() {
    IdAliasList.push_back(CId);
  }

  virtual ~ASTQubitContainerAliasNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeQubitContainerAlias;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::GetName();
  }

  virtual const std::string &GetMangledName() const override {
    return ASTExpressionNode::GetMangledName();
  }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName(unsigned Index) {
    assert(Index < IdAliasList.size() && "Index is out-of-range!");
    return IdAliasList[Index]->GetName();
  }

  virtual unsigned GetIndex() const { return AliasIndex; }

  virtual unsigned Size() const { return static_cast<unsigned>(QAL.size()); }

  virtual void Clear() {
    QAL.clear();
    IdAliasList.clear();
  }

  virtual void AddQubit(ASTQubitNode *QN) {
    assert(QN && "Invalid ASTQubitNode argument!");
    if (QN)
      QAL.push_back(QN);
  }

  virtual ASTQubitNode *GetQubit(unsigned Index) const {
    assert(Index < QAL.size() && "Index is out-of-range!");
    return QAL[Index];
  }

  virtual ASTQubitContainerAliasNode *CreateAlias(const ASTIdentifierNode *AId,
                                                  const ASTIntegerList &IL,
                                                  char Method) const;

  virtual ASTQubitContainerAliasNode *CreateAlias(const ASTIdentifierNode *AId,
                                                  int32_t Index) const;

  virtual ASTQubitContainerAliasNode *
  CreateAlias(const ASTIdentifierNode *AId) const;

  virtual unsigned AliasSize() const {
    return static_cast<unsigned>(IdAliasList.size());
  }

  virtual void ComputeBits() { Bits = QAL.size(); }

  void operator+=(const ASTQubitContainerAliasNode &Op);

  void operator+=(const ASTQubitContainerNode &Op);

  iterator begin() { return QAL.begin(); }

  const_iterator begin() const { return QAL.begin(); }

  iterator end() { return QAL.end(); }

  const_iterator end() const { return QAL.end(); }

  ASTQubitNode *front() { return QAL.front(); }

  const ASTQubitNode *front() const { return QAL.front(); }

  ASTQubitNode *back() { return QAL.back(); }

  const ASTQubitNode *back() const { return QAL.back(); }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTQubitContainerAliasNode *
  ExpressionError(const ASTIdentifierNode *Id, const std::string &ERM) {
    return new ASTQubitContainerAliasNode(Id, ERM);
  }

  static ASTQubitContainerAliasNode *ExpressionError(const std::string &ERM) {
    return new ASTQubitContainerAliasNode(ASTIdentifierNode::QCAlias.Clone(),
                                          ERM);
  }

  virtual void print() const override {
    std::cout << "<QubitContainerAlias>" << std::endl;
    std::cout << "<Identifier>" << GetName() << "</Identifier>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;
    std::cout << "<Size>" << Size() << "</Size>" << std::endl;
    std::cout << "<AliasQubits>" << std::endl;

    for (const_iterator I = QAL.begin(); I != QAL.end(); ++I)
      if (*I)
        (*I)->print();

    std::cout << "</AliasQubits>" << std::endl;
    std::cout << "</QubitContainerAlias>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTQubitContainerNode : public ASTExpressionNode {
  friend class ASTQubitBuilder;
  friend class ASTGateNode;
  friend class ASTQubitArrayNode;
  friend class ASTQubitNArrayNode;

private:
  unsigned Bits;
  std::vector<ASTQubitNode *> List;
  std::vector<ASTQubitNode *> Clones;

private:
  static unsigned AliasIndex;

private:
  ASTQubitContainerNode() = delete;

protected:
  ASTQubitContainerNode(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        List(), Clones() {}

public:
  static const unsigned QubitContainerBits = 1U;

public:
  using list_type = std::vector<ASTQubitNode *>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

protected:
  ASTQubitContainerNode(const ASTIdentifierNode *Id, unsigned Size,
                        unsigned QSize, const std::string &QS)
      : ASTExpressionNode(Id, ASTTypeQubitContainer), Bits(Size), List(),
        Clones() {
    std::stringstream S;
    for (unsigned I = 0; I < QSize; ++I) {
      S.clear();
      S.str("");
      S << QS.c_str() << ':' << I;
      ASTIdentifierNode *QId = new ASTIdentifierNode(S.str(), 1);
      assert(QId && "Could not create a valid ASTIdentifierNode!");
      List.push_back(new ASTQubitNode(QId, I, Id->GetName()));
    }
  }

public:
  ASTQubitContainerNode(const ASTIdentifierNode *Id, unsigned Size)
      : ASTExpressionNode(Id, ASTTypeQubitContainer), Bits(Size), List(),
        Clones() {}

  ASTQubitContainerNode(const ASTIdentifierNode *Id,
                        const std::vector<ASTQubitNode *> &QL)
      : ASTExpressionNode(Id, ASTTypeQubitContainer), Bits(QL.size()), List(QL),
        Clones() {}

  ASTQubitContainerNode(const ASTIdentifierNode *Id, unsigned NumBits,
                        const std::vector<ASTQubitNode *> &QL)
      : ASTExpressionNode(Id, ASTTypeQubitContainer), Bits(NumBits), List(),
        Clones() {
    if (NumBits >= QL.size())
      List.insert(List.begin(), QL.begin(), QL.end());
    else
      for (unsigned I = 0; I < NumBits; ++I)
        List.push_back(QL.at(I));
  }

  virtual ~ASTQubitContainerNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeQubitContainer; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual const std::string &GetMangledName() const override {
    return ASTExpressionNode::Ident->GetMangledName();
  }

  virtual void Mangle() override;

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual unsigned Size() const { return static_cast<unsigned>(List.size()); }

  virtual unsigned GetBits() const { return Bits; }

  virtual void LocalFunctionArgument();

  virtual void EraseFromLocalSymbolTable();

  virtual const std::vector<ASTQubitNode *> &CloneQubits() {
    std::stringstream QS;
    Clones.clear();

    if (List.size())
      for (unsigned I = 0; I < List.size(); ++I)
        Clones.push_back(List[I]);

    return Clones;
  }

  void AddQubit(ASTQubitNode *QN) {
    assert(QN && "Invalid ASTQubitNode argument!");
    if (QN) {
      List.push_back(QN);
      Bits = List.size();
    }
  }

  virtual ASTQubitNode *GetQubit(unsigned Index) const {
    assert(Index < List.size() && "Index is out-of-range!");
    return List[Index];
  }

  virtual const std::vector<ASTQubitNode *> &GetQubitVector() const {
    return List;
  }

  virtual ASTQubitContainerAliasNode *CreateAlias(const ASTIdentifierNode *AId,
                                                  const ASTIntegerList &IL,
                                                  char Method) const;

  virtual ASTQubitContainerAliasNode *CreateAlias(const ASTIdentifierNode *AId,
                                                  int32_t Index) const;

  virtual ASTQubitContainerAliasNode *
  CreateAlias(const ASTIdentifierNode *AId) const;

  iterator begin() { return List.begin(); }

  const_iterator begin() const { return List.begin(); }

  iterator end() { return List.end(); }

  const_iterator end() const { return List.end(); }

  ASTQubitNode *front() { return List.front(); }

  const ASTQubitNode *front() const { return List.front(); }

  ASTQubitNode *back() { return List.back(); }

  const ASTQubitNode *back() const { return List.back(); }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  static ASTQubitContainerNode *ExpressionError(const ASTIdentifierNode *Id,
                                                const std::string &ERM) {
    return new ASTQubitContainerNode(Id, ERM);
  }

  static ASTQubitContainerNode *ExpressionError(const std::string &ERM) {
    return new ASTQubitContainerNode(ASTIdentifierNode::QC.Clone(), ERM);
  }

  virtual void print() const override {
    std::cout << "<QubitContainer>" << std::endl;
    std::cout << "<Identifier>" << GetName() << "</Identifier>" << std::endl;
    std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
              << std::endl;
    std::cout << "<Size>" << Size() << "</Size>" << std::endl;
    std::cout << "<Qubits>" << std::endl;

    for (std::vector<ASTQubitNode *>::const_iterator I = List.begin();
         I != List.end(); ++I)
      (*I)->print();

    std::cout << "</Qubits>" << std::endl;
    std::cout << "</QubitContainer>" << std::endl;
  }

  virtual void push(ASTBase *Node) override {
    assert((List.size() + 1) <= Bits &&
           "ASTQubit List Size would exceed the number of available Bits!");

    if (ASTQubitNode *QN = dynamic_cast<ASTQubitNode *>(Node))
      List.push_back(QN);
  }
};

} // namespace QASM

#endif // __QASM_AST_QUBIT_H
