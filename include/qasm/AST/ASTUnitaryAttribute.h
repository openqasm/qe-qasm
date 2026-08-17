/* -*- coding: utf-8 -*-
 *
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef __QASM_AST_UNITARY_ATTRIBUTE_H
#define __QASM_AST_UNITARY_ATTRIBUTE_H

#include <qasm/AST/ASTTypes.h>

namespace QASM {

enum ASTUnitaryAttributeKind {
  ASTUnitaryAttributeBumper,
  ASTUnitaryAttributeBumperMax
};


class ASTUnitaryAttributeNode : public ASTStatementNode {
private:
  const ASTUnitaryAttributeKind AttributeKind;
  ASTOpType OpType;

  ASTUnitaryAttributeNode() = delete;

public:
  ASTUnitaryAttributeNode(const ASTIdentifierNode *Target,
                          const ASTUnitaryAttributeKind AttributeKind,
                          const ASTExpressionNode *Value,
                          ASTOpType OpType);

  virtual ~ASTUnitaryAttributeNode() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeUnitaryAttribute;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeStatement;
  }

  virtual const ASTIdentifierNode *GetTarget() const {
    return GetIdentifier();
  }

  virtual ASTUnitaryAttributeKind GetAttributeKind() const {
  return AttributeKind;
}

  virtual const ASTExpressionNode *GetValue() const {
    return GetExpression();
  }

  virtual ASTOpType GetOpType() const {
    return OpType;
  }

  virtual void print() const override;

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif