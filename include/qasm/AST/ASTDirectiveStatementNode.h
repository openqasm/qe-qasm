/* -*- coding: utf-8 -*-
 *
 * Copyright 2021 IBM RESEARCH. All Rights Reserved.
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

#ifndef __QASM_AST_DIRECTIVE_STATEMENT_NODE_H
#define __QASM_AST_DIRECTIVE_STATEMENT_NODE_H

#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTTypes.h>

#include <string>

namespace QASM {

class ASTDirectiveStatementNode : public ASTStatementNode {
private:
  std::string FileName;
  uint32_t LineNo;

private:
  ASTDirectiveStatementNode() = delete;

public:
  ASTDirectiveStatementNode(uint32_t L, const std::string &F = "")
      : ASTStatementNode(&ASTIdentifierNode::QPPDirective), FileName(F),
        LineNo(L) {}

  ASTDirectiveStatementNode(const std::string &F,
                            uint32_t L = static_cast<uint32_t>(~0x0))
      : ASTStatementNode(&ASTIdentifierNode::QPPDirective), FileName(F),
        LineNo(L) {}

  virtual ~ASTDirectiveStatementNode() = default;

  virtual bool IsDirective() const override { return true; }

  virtual ASTType GetASTType() const override {
    return ASTTypeDirectiveStatement;
  }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return &ASTIdentifierNode::QPPDirective;
  }

  virtual const std::string &GetName() const override {
    return ASTIdentifierNode::QPPDirective.GetName();
  }

  virtual uint32_t GetLineNo() const override { return LineNo; }

  virtual const std::string &GetFileName() const { return FileName; }

  virtual void print() const override {
    std::cout << "<QPPDirectiveStatement>" << std::endl;
    std::cout << "<File>" << FileName << "</File>" << std::endl;
    std::cout << "<Line>" << LineNo << "</Line>" << std::endl;
    std::cout << "</QPPDirectiveStatement>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

} // namespace QASM

#endif // __QASM_AST_DIRECTIVE_STATEMENT_NODE_H
