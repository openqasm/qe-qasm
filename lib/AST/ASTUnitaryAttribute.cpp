/* -*- coding: utf-8 -*-
 *
 * Licensed under the Apache License, Version 2.0.
 */

#include <qasm/AST/ASTUnitaryAttribute.h>

#include <cassert>
#include <iostream>

namespace QASM {

ASTUnitaryAttributeNode::ASTUnitaryAttributeNode(
    const ASTIdentifierNode *Target,
    ASTUnitaryAttributeKind AttrKind,
    const ASTExpressionNode *Value,
    ASTOpType Op)
    : ASTStatementNode(Target, Value),
      AttributeKind(AttrKind),
      OpType(Op) {

  assert(Target && "Invalid unitary attribute target!");
  assert(Value && "Invalid unitary attribute value!");

  assert((Op == ASTOpTypeAssign ||
          Op == ASTOpTypeAddAssign) &&
         "Invalid unitary attribute assignment operator!");
}
void ASTUnitaryAttributeNode::print() const {
  std::cout << "<UnitaryAttribute>" << std::endl;

  std::cout << "<Target>";
  if (GetTarget())
    std::cout << GetTarget()->GetName();
  std::cout << "</Target>" << std::endl;

  std::cout << "<Attribute>";

  switch (AttributeKind) {
  case ASTUnitaryAttributeBumper:
    std::cout << "bumper";
    break;
  case ASTUnitaryAttributeBumperMax:
    std::cout << "bumper_max";
    break;
  }

  std::cout << "</Attribute>" << std::endl;

  std::cout << "<Operator>";

  switch (OpType) {
  case ASTOpTypeAssign:
    std::cout << "=";
    break;
  case ASTOpTypeAddAssign:
    std::cout << "+=";
    break;
  default:
    std::cout << "unknown";
    break;
  }

  std::cout << "</Operator>" << std::endl;

  std::cout << "<Value>" << std::endl;
  if (GetValue())
    GetValue()->print();
  std::cout << "</Value>" << std::endl;

  std::cout << "</UnitaryAttribute>" << std::endl;
}
} // namespace QASM