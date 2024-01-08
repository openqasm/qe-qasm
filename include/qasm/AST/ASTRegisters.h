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

#ifndef __QASM_AST_REGISTERS_H
#define __QASM_AST_REGISTERS_H

#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTQubit.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <list>

namespace QASM {

class ASTCRegNode : public ASTExpressionNode {
private:
  std::vector<unsigned> Bits;

private:
  ASTCRegNode() = delete;

public:
  ASTCRegNode(const ASTIdentifierNode *Id, unsigned R)
      : ASTExpressionNode(Id, ASTTypeCReg), Bits() {
    for (unsigned I = 0; I < R; ++I)
      Bits.push_back(0U);
  }

  ASTCRegNode(const ASTIdentifierNode *Id, const std::vector<unsigned> &BV)
      : ASTExpressionNode(Id, ASTTypeCReg), Bits(BV) {}

  virtual ~ASTCRegNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeCReg; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual bool IsConcrete() const { return true; }
  virtual bool IsQuantum() const { return false; }

  virtual unsigned GetBit(unsigned Index) {
    assert(Index < Bits.size() && "Bit Index is out-of-range!");
    return Bits[Index];
  }

  virtual unsigned GetBit(unsigned Index) const {
    assert(Index < Bits.size() && "Bit Index is out-of-range!");
    return Bits[Index];
  }

  virtual unsigned operator[](unsigned Index) { return GetBit(Index); }

  virtual unsigned operator[](unsigned Index) const { return GetBit(Index); }

  virtual ASTType GetRegisterType() const { return ASTTypeCReg; }

  virtual unsigned GetNumRegs() const {
    return static_cast<unsigned>(Bits.size());
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::GetIdentifier()->GetName();
  }

  virtual void print() const override {
    std::cout << "<CReg>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;

    for (std::vector<unsigned>::const_iterator I = Bits.begin();
         I != Bits.end(); ++I)
      std::cout << "<Bit>" << *I << "</Bit>" << std::endl;

    std::cout << "</CReg>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTQRegNode : public ASTExpressionNode {
private:
  std::vector<ASTQubitNode *> Qubits;

private:
  ASTQRegNode() = delete;

public:
  using vector_type = std::vector<ASTQubitNode *>;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

public:
  ASTQRegNode(const ASTIdentifierNode *Id, unsigned NQ)
      : ASTExpressionNode(Id, ASTTypeQReg), Qubits() {
    Qubits.reserve(NQ);
  }

  ASTQRegNode(const ASTIdentifierNode *Id,
              const std::vector<ASTQubitNode *> &QV)
      : ASTExpressionNode(Id, ASTTypeQReg), Qubits(QV) {}

  virtual ~ASTQRegNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeQReg; }

  virtual bool IsConcrete() const { return false; }
  virtual bool IsQuantum() const { return true; }

  virtual ASTType GetRegisterType() const { return ASTTypeQReg; }

  virtual bool Empty() const { return Qubits.size() == 0; }

  virtual unsigned GetNumQubits() const {
    return static_cast<unsigned>(Qubits.size());
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::GetIdentifier()->GetName();
  }

  inline ASTQubitNode *GetQubit(unsigned Index) {
    assert(Index < Qubits.size() && "Index is out-of-range!");
    return Qubits[Index];
  }

  inline const ASTQubitNode *GetQubit(unsigned Index) const {
    assert(Index < Qubits.size() && "Index is out-of-range!");
    return Qubits[Index];
  }

  inline ASTQubitNode *operator[](unsigned Index) { return GetQubit(Index); }

  inline const ASTQubitNode *operator[](unsigned Index) const {
    return GetQubit(Index);
  }

  iterator begin() { return Qubits.begin(); }

  const_iterator begin() const { return Qubits.begin(); }

  iterator end() { return Qubits.end(); }

  const_iterator end() const { return Qubits.end(); }

  virtual void AddQubit(ASTQubitNode *QB) {
    assert(QB && "Invalid ASTQubitNode Argument!");
    Qubits.push_back(QB);
  }

  virtual void print() const override {
    std::cout << "<QReg>" << std::endl;
    std::cout << "<Name>" << GetName() << "</Name>" << std::endl;

    for (std::vector<ASTQubitNode *>::const_iterator I = Qubits.begin();
         I != Qubits.end(); ++I)
      (*I)->print();

    std::cout << "</QReg>" << std::endl;
  }

  virtual void push(ASTBase *Node) override {
    if (ASTQubitNode *QBN = dynamic_cast<ASTQubitNode *>(Node))
      Qubits.push_back(QBN);
  }
};

} // namespace QASM

#endif // __QASM_AST_REGISTERS_H
