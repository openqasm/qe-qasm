/* -*- coding: utf-8 -*-
 *
 * Copyright 2022 IBM RESEARCH. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#include <qasm/AST/ASTUnitary.h>
#include <qasm/AST/ASTInitializerNode.h>
#include <qasm/AST/ASTMangler.h>
#include <cassert>
#include <iostream>

namespace QASM {

void ASTUnitaryNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  M.EndExpression();
  M.End();

  const_cast<ASTIdentifierNode *>(GetIdentifier())
      ->SetMangledName(M.AsString());
}


ASTUnitaryNode *
ASTUnitaryNode::CloneCall(const ASTIdentifierNode *Id,
                          const ASTArgumentNodeList &AL,
                          const ASTAnyTypeList &QL) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  ASTIdentifierNode *UId =
      GateCallIdentifier(Id->GetName(),
                         Id->GetSymbolType(),
                         Id->GetBits());

  assert(UId &&
         "Could not create a valid Unitary Call ASTIdentifierNode!");

  UId->SetSymbolTableEntry(
      const_cast<ASTSymbolTableEntry *>(Id->GetSymbolTableEntry()));

  ASTUnitaryNode *RU = new ASTUnitaryNode(UId, AL, QL, true);

  assert(RU && "Could not create a valid ASTUnitaryNode call!");

  RU->OpList = OpList;
  RU->GDId = Id;
  RU->Void = Void;
  RU->ControlType = ControlType;
  RU->Opaque = Opaque;
  RU->GateCall = true;

  RU->Mangle();
  return RU;
}

void ASTUnitaryNode::print() const {
  std::cout << "<Unitary>" << std::endl;
  std::cout << "<Identifier>" << GetName() << "</Identifier>" << std::endl;
  std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
            << std::endl;

  if (INL)
    INL->print();

  std::cout << "</Unitary>" << std::endl;
}


} // namespace QASM