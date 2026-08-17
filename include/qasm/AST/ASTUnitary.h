/* -*- coding: utf-8 -*-
 *
 * Copyright 2022 IBM RESEARCH. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef __QASM_AST_UNITARY_H
#define __QASM_AST_UNITARY_H

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTGates.h>

namespace QASM {

// class ASTUnitaryNode : public ASTExpressionNode {
class ASTInitializerList; // forward declaration
class ASTUnitaryNode : public ASTGateNode {
private:
  ASTUnitaryNode() = delete;
  const ASTInitializerList *INL; // initializer list for unitary matrix

public:
  // unitary u;
  explicit ASTUnitaryNode(const ASTIdentifierNode *Id)
      // : ASTExpressionNode(Id, ASTTypeUnitary) {}
        : ASTGateNode(Id), INL(nullptr) {}

  // unitary u = {{1, 0}, {0, 1}};
  ASTUnitaryNode(const ASTIdentifierNode *Id,
                 const ASTInitializerList *IL)
      : ASTGateNode(Id), INL(IL) {}

  // Existing gate-call constructor
  ASTUnitaryNode(const ASTIdentifierNode *Id,
                 const ASTArgumentNodeList &AL,
                 const ASTAnyTypeList &QL,
                 bool IsGateCall = false)
      : ASTGateNode(Id, AL, QL, IsGateCall), INL(nullptr) {}


  virtual ~ASTUnitaryNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeUnitary;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

    virtual bool HasInitializerList() const {
    return INL != nullptr;
  }

  virtual const ASTInitializerList *GetInitializerList() const {
    return INL;
  }
  //added this block of code
  virtual ASTUnitaryNode *
  CloneCall(const ASTIdentifierNode *Id,
            const ASTArgumentNodeList &AL,
            const ASTAnyTypeList &QL) override;


  virtual void Mangle() override;

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    // return ASTExpressionNode::Ident;
    return ASTGateNode::GetIdentifier();
  }

  virtual void print() const override;
};

} // namespace QASM

#endif