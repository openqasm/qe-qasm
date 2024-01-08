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

#include <qasm/AST/ASTFunctionCallExpr.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTResult.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseWaveform.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <cassert>
#include <iostream>

namespace QASM {
namespace OpenPulse {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

void ASTOpenPulseWaveformNode::print() const {
  std::cout << "<OpenPulseWaveform>" << std::endl;
  ASTExpressionNode::GetIdentifier()->print();
  if (AMP) {
    std::cout << "<Amplitude>" << std::endl;
    AMP->print();
    std::cout << "</Amplitude>" << std::endl;
  }

  if (D) {
    std::cout << "<Duration>" << std::endl;
    D->print();
    std::cout << "</Duration>" << std::endl;
  }

  if (SQW) {
    std::cout << "<SquareWidth>" << std::endl;
    SQW->print();
    std::cout << "</SquareWidth>" << std::endl;
  }

  if (SIG) {
    std::cout << "<Sigma>" << std::endl;
    SIG->print();
    std::cout << "</Sigma>" << std::endl;
  }

  if (BTA) {
    std::cout << "<Beta>" << std::endl;
    BTA->print();
    std::cout << "</Beta>" << std::endl;
  }

  if (FRQ) {
    std::cout << "<Frequency>" << std::endl;
    FRQ->print();
    std::cout << "</Frequency>" << std::endl;
  }

  if (PHS) {
    std::cout << "<Phase>" << std::endl;
    PHS->print();
    std::cout << "</Phase>" << std::endl;
  }

  CXV.print();

  if (FC) {
    std::cout << "<FunctionCall>" << std::endl;
    FC->print();
    std::cout << "</FunctionCall>" << std::endl;
  }

  if (FR) {
    std::cout << "<FunctionResult>" << std::endl;
    FR->print();
    std::cout << "</FunctionResult>" << std::endl;
  }

  std::cout << "</OpenPulseWaveform>" << std::endl;
}

void ASTOpenPulseWaveformNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.TypeIdentifier(GetASTType(), GetName());
  M.Underscore();
  M.StringValue("TE");
  M.StringValue(std::to_string(TE));
  M.Underscore();

  switch (TE) {
  case 0U:
    M.StringValue(ASTStringUtils::Instance().SanitizeMangled(CXV.Mangle()));
    break;
  case 1U:
    const_cast<ASTMPComplexNode *>(AMP)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(AMP->GetMangledName()));
    const_cast<ASTDurationNode *>(D)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(D->GetMangledName()));
    const_cast<ASTDurationNode *>(SIG)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(SIG->GetMangledName()));
    break;
  case 2U:
    const_cast<ASTMPComplexNode *>(AMP)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(AMP->GetMangledName()));
    const_cast<ASTDurationNode *>(D)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(D->GetMangledName()));
    const_cast<ASTDurationNode *>(SQW)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(SQW->GetMangledName()));
    const_cast<ASTDurationNode *>(SIG)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(SIG->GetMangledName()));
    break;
  case 3U:
    const_cast<ASTMPComplexNode *>(AMP)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(AMP->GetMangledName()));
    const_cast<ASTDurationNode *>(D)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(D->GetMangledName()));
    const_cast<ASTMPDecimalNode *>(BTA)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(BTA->GetMangledName()));
    break;
  case 4U:
    const_cast<ASTMPComplexNode *>(AMP)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(AMP->GetMangledName()));
    const_cast<ASTDurationNode *>(D)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(D->GetMangledName()));
    break;
  case 5U:
    const_cast<ASTMPComplexNode *>(AMP)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(AMP->GetMangledName()));
    const_cast<ASTDurationNode *>(D)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(D->GetMangledName()));
    const_cast<ASTMPDecimalNode *>(FRQ)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(FRQ->GetMangledName()));
    const_cast<ASTAngleNode *>(PHS)->Mangle();
    M.StringValue(
        ASTStringUtils::Instance().SanitizeMangled(PHS->GetMangledName()));
    break;
  default:
    break;
  }

  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace OpenPulse
} // namespace QASM
