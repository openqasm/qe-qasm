/* -*- coding: utf-8 -*-
 *
 * Copyright 2020 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_TYPE_SPECIFIER_NODE_H
#define __QASM_AST_TYPE_SPECIFIER_NODE_H

#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTTypeEnums.h>

namespace QASM {

class ASTTypeSpecifierNode : public ASTExpression {
private:
  const ASTIdentifierNode *Ident;
  bool Unsigned;
  ASTType Type;

private:
  ASTTypeSpecifierNode() = delete;

public:
  ASTTypeSpecifierNode(const ASTIdentifierNode *IDN, int Token,
                       bool IsUnsigned = false)
      : ASTExpression(), Ident(IDN), Unsigned(IsUnsigned),
        Type(ASTTypeUndefined) {
    Type = ResolveASTType(Token);
  }

  virtual ~ASTTypeSpecifierNode() = default;

  virtual ASTType GetASTType() const override { return Type; }

  virtual ASTSemaType GetSemaType() const { return SemaTypeExpression; }

  virtual const std::string &GetName() const { return Ident->GetName(); }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return Ident;
  }

  virtual const char *GetTypeName() const { return ResolveASTTypeName(); }

  virtual ASTType GetType() const { return Type; }

  virtual void print() const override {
    std::cout << "<TypeSpecifier>";
    std::cout << GetIdentifier();
    std::cout << "</TypeSpecifier>" << std::endl;
    ;
  }

  virtual void push(ASTBase * /* unused */) override {}

private:
  ASTType ResolveASTType(int Token) const;
  const char *ResolveASTTypeName() const;
};

} // namespace QASM

#endif // __QASM_AST_TYPE_SPECIFIER_NODE_H
