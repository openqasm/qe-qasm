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

#ifndef __QASM_AST_DEFCAL_GRAMMAR_BUILDER_H
#define __QASM_AST_DEFCAL_GRAMMAR_BUILDER_H

#include <qasm/AST/ASTStringUtils.h>
#include <qasm/AST/ASTTypes.h>

#include <set>
#include <string>

namespace QASM {

class ASTDefcalGrammarBuilder {
private:
  static ASTDefcalGrammarBuilder GB;
  static std::set<std::string> GS;
  static std::string CG;

private:
  using set_type = std::set<std::string>;
  using iterator = typename set_type::iterator;
  using const_iterator = typename set_type::const_iterator;

protected:
  ASTDefcalGrammarBuilder() = default;

public:
  static ASTDefcalGrammarBuilder &Instance() {
    return ASTDefcalGrammarBuilder::GB;
  }

  void AddGrammar(const std::string &G) {
    std::string S = ASTStringUtils::Instance().Sanitize(G);
    GS.insert(S);
  }

  void SetCurrent(const std::string &G) {
    AddGrammar(G);
    CG = ASTStringUtils::Instance().Sanitize(G);
  }

  const std::string &GetCurrent() const { return CG; }

  bool IsGrammar(const std::string &G) const {
    std::string S = ASTStringUtils::Instance().Sanitize(G);
    return GS.find(S) != GS.end();
  }

  void ValidateContext(const std::string &GS) const;
};

class ASTDefcalGrammarNode : public ASTExpressionNode {
private:
  ASTStringNode *GS;

private:
  ASTDefcalGrammarNode() = delete;

public:
  static const unsigned GrammarBits = 64U;

public:
  ASTDefcalGrammarNode(const ASTIdentifierNode *Id, ASTStringNode *G)
      : ASTExpressionNode(Id, ASTTypeDefcalGrammar), GS(G) {}

  virtual ~ASTDefcalGrammarNode() = default;

  virtual ASTType GetASTType() const override { return ASTTypeDefcalGrammar; }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  virtual const std::string &GetGrammarName() const { return GS->GetValue(); }

  virtual void print() const override {
    std::cout << "<DefcalGrammar>" << std::endl;
    GS->print();
    std::cout << "</DefcalGrammar>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_DEFCAL_GRAMMAR_BUILDER_H
