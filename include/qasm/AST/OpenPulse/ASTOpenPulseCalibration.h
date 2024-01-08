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

#ifndef __QASM_AST_OPENPULSE_CALIBRATION_H
#define __QASM_AST_OPENPULSE_CALIBRATION_H

#include <qasm/AST/ASTTypes.h>

#include <cassert>
#include <iostream>
#include <string>

namespace QASM {
namespace OpenPulse {

class ASTOpenPulseCalibration : public ASTExpressionNode {
private:
  ASTStatementList CSL;

private:
  ASTOpenPulseCalibration() = delete;

protected:
  ASTOpenPulseCalibration(const ASTIdentifierNode *Id, const std::string &ERM)
      : ASTExpressionNode(Id, new ASTStringNode(ERM), ASTTypeExpressionError),
        CSL() {}

public:
  static const unsigned CalibrationBits = 64U;
  static constexpr const char *DCB = "default-calibration-block";

public:
  ASTOpenPulseCalibration(const ASTIdentifierNode *Id,
                          const ASTStatementList &SL)
      : ASTExpressionNode(Id, ASTTypeOpenPulseCalibration), CSL(SL) {}

  virtual ~ASTOpenPulseCalibration() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeOpenPulseCalibration;
  }

  virtual ASTSemaType GetSemaType() const override {
    return SemaTypeExpression;
  }

  virtual void Mangle() override;

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTExpressionNode::Ident;
  }

  virtual const std::string &GetName() const override {
    return ASTExpressionNode::Ident->GetName();
  }

  const ASTStatementList &GetStatementList() const { return CSL; }

  virtual void Append(const ASTStatementList &SL) { CSL.Append(SL); }

  virtual void SetCalibrationContext();

  static ASTOpenPulseCalibration *ExpressionError(const ASTIdentifierNode *Id,
                                                  const std::string &ERM) {
    return new ASTOpenPulseCalibration(Id, ERM);
  }

  virtual bool IsError() const override { return ASTExpressionNode::IsError(); }

  virtual const std::string &GetError() const override {
    return ASTExpressionNode::GetError();
  }

  virtual void print() const override {
    std::cout << "<OpenPulseCalibration>" << std::endl;
    ASTExpressionNode::print();
    CSL.print();
    std::cout << "</OpenPulseCalibration>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTOpenPulseCalibrationStmt : public ASTStatementNode {
private:
  ASTOpenPulseCalibrationStmt() = delete;

protected:
  ASTOpenPulseCalibrationStmt(const ASTIdentifierNode *Id,
                              const std::string &ERM)
      : ASTStatementNode(Id, ASTExpressionNode::ExpressionError(Id, ERM)) {}

public:
  ASTOpenPulseCalibrationStmt(const ASTOpenPulseCalibration *CE)
      : ASTStatementNode(CE->GetIdentifier(), CE) {}

  virtual ~ASTOpenPulseCalibrationStmt() = default;

  virtual ASTType GetASTType() const override {
    return ASTTypeOpenPulseCalibrationStmt;
  }

  virtual ASTSemaType GetSemaType() const override { return SemaTypeStatement; }

  virtual const ASTIdentifierNode *GetIdentifier() const override {
    return ASTStatementNode::GetIdentifier();
  }

  virtual const std::string &GetName() const override {
    return ASTStatementNode::GetIdentifier()->GetName();
  }

  virtual bool IsError() const override {
    if (const ASTExpressionNode *EN =
            dynamic_cast<const ASTExpressionNode *>(Expr)) {
      if (dynamic_cast<const ASTStringNode *>(EN))
        return true;
    }

    return false;
  }

  virtual const std::string &GetError() const override {
    if (const ASTExpressionNode *EN =
            dynamic_cast<const ASTExpressionNode *>(Expr)) {
      if (dynamic_cast<const ASTStringNode *>(EN))
        return ASTStatementNode::GetError();
    }

    return ASTStringUtils::Instance().EmptyString();
  }

  static ASTOpenPulseCalibrationStmt *
  StatementError(const ASTIdentifierNode *Id, const std::string &ERM) {
    return new ASTOpenPulseCalibrationStmt(Id, ERM);
  }

  virtual void print() const override {
    std::cout << "<OpenPulseCalibrationStatement>" << std::endl;
    ASTStatementNode::print();
    std::cout << "</OpenPulseCalibrationStatement>" << std::endl;
  }

  virtual void push(ASTBase * /* unused */) override {}
};

class ASTOpenPulseCalibrationBuilder {
private:
  static ASTOpenPulseCalibrationBuilder CB;
  static bool CX;

protected:
  ASTOpenPulseCalibrationBuilder() = default;

public:
  static ASTOpenPulseCalibrationBuilder &Instance() { return CB; }

  void OpenContext() { CX = true; }

  void CloseContext() { CX = false; }

  void ValidateContext() const;
};

} // namespace OpenPulse
} // namespace QASM

#endif // __QASM_AST_OPENPULSE_CALIBRATION_H
