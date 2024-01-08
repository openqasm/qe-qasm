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

#ifndef __QASM_AST_DECLARATION_H
#define __QASM_AST_DECLARATION_H

#include <qasm/AST/ASTIdentifier.h>
#include <qasm/AST/ASTStatement.h>

#include <cassert>
#include <string>
#include <vector>

namespace QASM {

class ASTDeclaration : public ASTStatement {
protected:
  const ASTIdentifierNode *Ident;
  ASTType Type;

private:
  ASTDeclaration() = delete;

public:
  ASTDeclaration(ASTType Ty, const ASTIdentifierNode *Id)
      : ASTStatement(), Ident(Id), Type(Ty) {}

  ASTDeclaration(const ASTDeclaration &RHS)
      : ASTStatement(RHS), Ident(RHS.Ident), Type(RHS.Type) {}

  virtual ~ASTDeclaration() = default;

  ASTDeclaration &operator=(const ASTDeclaration &RHS) {
    if (this != &RHS) {
      ASTStatement::operator=(RHS);
      Ident = RHS.Ident;
      Type = RHS.Type;
    }

    return *this;
  }

  virtual void SetASTType(ASTType Ty) { Type = Ty; }

  virtual ASTType GetASTType() const override { return Type; }

  virtual ASTSemaType GetSemaType() const { return SemaTypeDeclaration; }

  virtual const ASTIdentifierNode *GetIdentifier() const { return Ident; }

  virtual const std::string &GetName() const { return Ident->GetName(); }
};

class ASTOpenQASMDeclaration : public ASTDeclaration {
private:
  std::string Version;

public:
  ASTOpenQASMDeclaration(const ASTIdentifierNode *Id)
      : ASTDeclaration(ASTTypeOpenQASMDecl, Id) {}

  ASTOpenQASMDeclaration(const ASTIdentifierNode *Id, const std::string &V)
      : ASTDeclaration(ASTTypeOpenQASMDecl, Id), Version(V) {}

  virtual ~ASTOpenQASMDeclaration() = default;

  virtual ASTType GetASTType() const override { return ASTTypeOpenQASMDecl; }

  virtual void SetVersion(const std::string &V) { Version = V; }

  virtual const std::string &GetVersion() const { return Version; }

  virtual double GetVersionAsDouble() const { return std::stod(Version); }

  virtual void print() const override {
    std::cout << "<OpenQASMDeclaration>" << std::endl;
    std::cout << "<Identifier>" << GetName() << "</Identifier>" << std::endl;
    std::cout << "<Version>" << Version << "</Version>" << std::endl;
    std::cout << "</OpenQASMDeclaration>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_DECLARATION_H
