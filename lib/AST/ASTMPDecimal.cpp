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

#include <qasm/AST/ASTImplicitConversionExpr.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <cassert>
#include <iostream>
#include <sstream>

#include <mpfr.h>

namespace QASM {

using DiagLevel = QasmDiagnosticEmitter::DiagLevel;

int ASTMPDecimalNode::InitMPFRFromString(mpfr_t &MPV, const char *S, int Base) {
  assert(S && "Invalid numeric decimal constant string!");

  std::string DS = ASTStringUtils::Instance().Sanitize(std::string(S));
  if (mpfr_set_str(MPV, DS.c_str(), Base, MPFR_RNDN) != 0) {
    std::stringstream M;
    M << "Invalid string representation of arbitrary precision decimal value.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
    return -1;
  }

  return 0;
}

ASTMPDecimalNode *ASTMPDecimalNode::Pi(int Bits) {
  mpfr_t Pi;
  mpfr_init2(Pi, Bits);

  int R = mpfr_const_pi(Pi, MPFR_RNDN);
  if (R != 0 && Bits < 32) {
    std::stringstream M;
    M << "Possible loss of precision in calculating "
      << "multiple-precision Pi.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  ASTMPDecimalNode *MPD =
      new ASTMPDecimalNode(&ASTIdentifierNode::Pi, Bits, Pi);
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  mpfr_clear(Pi);
  return MPD;
}

ASTMPDecimalNode *ASTMPDecimalNode::NegPi(int Bits) {
  mpfr_t Pi;
  mpfr_init2(Pi, Bits);

  int R = mpfr_const_pi(Pi, MPFR_RNDN);
  if (R != 0 && Bits < 32) {
    std::stringstream M;
    M << "Possible loss of precision in calculating "
      << "multiple-precision Pi.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  mpfr_t NegPi;
  mpfr_init2(NegPi, Bits);
  R = mpfr_mul_d(NegPi, Pi, -1.0, MPFR_RNDN);

  if (R != 0 && Bits < 32) {
    std::stringstream M;
    M << "Possible loss of precision in calculating "
      << "multiple-precision negative Pi.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  ASTMPDecimalNode *MPD =
      new ASTMPDecimalNode(&ASTIdentifierNode::Pi, Bits, NegPi);
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  mpfr_clear(Pi);
  mpfr_clear(NegPi);
  return MPD;
}

ASTMPDecimalNode *ASTMPDecimalNode::Pi(int Bits, int Prec) {
  int Precision = ASTMPDecimalNode::GetDefaultPrecision();
  ASTMPDecimalNode::SetDefaultPrecision(Prec);
  ASTMPDecimalNode *MPD = ASTMPDecimalNode::Pi(Bits);
  ASTMPDecimalNode::SetDefaultPrecision(Precision);
  return MPD;
}

ASTMPDecimalNode *ASTMPDecimalNode::NegPi(int Bits, int Prec) {
  int Precision = ASTMPDecimalNode::GetDefaultPrecision();
  ASTMPDecimalNode::SetDefaultPrecision(Prec);
  ASTMPDecimalNode *MPD = ASTMPDecimalNode::NegPi(Bits);
  ASTMPDecimalNode::SetDefaultPrecision(Precision);
  return MPD;
}

ASTMPDecimalNode *ASTMPDecimalNode::Tau(int Bits) {
  mpfr_t Pi;
  mpfr_t Tau;
  mpfr_init2(Pi, Bits);
  mpfr_init2(Tau, Bits);

  int R = mpfr_const_pi(Pi, MPFR_RNDN);
  if (R != 0 && Bits < 32) {
    std::stringstream M;
    M << "Possible loss of precision in calculating "
      << "multiple-precision Pi.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  R = mpfr_mul_d(Tau, Pi, 2.0, MPFR_RNDN);
  if (R != 0 && Bits < 32) {
    std::stringstream M;
    M << "Possible loss of precision in calculating "
      << "multiple-precision Tau.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  ASTMPDecimalNode *MPD =
      new ASTMPDecimalNode(&ASTIdentifierNode::Tau, Bits, Tau);
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  mpfr_clear(Pi);
  mpfr_clear(Tau);
  return MPD;
}

ASTMPDecimalNode *ASTMPDecimalNode::NegTau(int Bits) {
  mpfr_t Pi;
  mpfr_t Tau;
  mpfr_init2(Pi, Bits);
  mpfr_init2(Tau, Bits);

  int R = mpfr_const_pi(Pi, MPFR_RNDN);
  if (R != 0 && Bits < 32) {
    std::stringstream M;
    M << "Possible loss of precision in calculating "
      << "multiple-precision Pi.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  R = mpfr_mul_d(Tau, Pi, 2.0, MPFR_RNDN);
  if (R != 0 && Bits < 32) {
    std::stringstream M;
    M << "Possible loss of precision in calculating "
      << "multiple-precision Tau.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  mpfr_t NegTau;
  mpfr_init2(NegTau, Bits);

  R = mpfr_mul_d(NegTau, Tau, -1.0, MPFR_RNDN);
  if (R != 0 && Bits < 32) {
    std::stringstream M;
    M << "Possible loss of precision in calculating "
      << "multiple-precision negative Tau.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  ASTMPDecimalNode *MPD =
      new ASTMPDecimalNode(&ASTIdentifierNode::Tau, Bits, NegTau);
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  mpfr_clear(Pi);
  mpfr_clear(Tau);
  mpfr_clear(NegTau);
  return MPD;
}

ASTMPDecimalNode *ASTMPDecimalNode::Tau(int Bits, int Prec) {
  int Precision = ASTMPDecimalNode::GetDefaultPrecision();
  ASTMPDecimalNode::SetDefaultPrecision(Prec);
  ASTMPDecimalNode *MPD = ASTMPDecimalNode::Tau(Bits);
  ASTMPDecimalNode::SetDefaultPrecision(Precision);
  return MPD;
}

ASTMPDecimalNode *ASTMPDecimalNode::NegTau(int Bits, int Prec) {
  int Precision = ASTMPDecimalNode::GetDefaultPrecision();
  ASTMPDecimalNode::SetDefaultPrecision(Prec);
  ASTMPDecimalNode *MPD = ASTMPDecimalNode::NegTau(Bits);
  ASTMPDecimalNode::SetDefaultPrecision(Precision);
  return MPD;
}

ASTMPDecimalNode *ASTMPDecimalNode::Euler(int Bits) {
  mpfr_t Gamma;
  mpfr_t Val;
  mpfr_t Sinh;
  mpfr_t Cosh;

  mpfr_init2(Gamma, Bits);
  mpfr_init2(Val, Bits);
  mpfr_init2(Sinh, Bits);
  mpfr_init2(Cosh, Bits);

  // Euler Number == sinh(1.0) + cosh(1.0).
  mpfr_set_d(Val, 1.0, MPFR_RNDN);

  int R = mpfr_sinh_cosh(Sinh, Cosh, Val, MPFR_RNDN);
  if (R != 0 && Bits < 32) {
    std::stringstream M;
    M << "Possible loss of precision in calculating "
      << "multiple-precision sinh(1) and cosh(1).";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  R = mpfr_add(Gamma, Sinh, Cosh, MPFR_RNDN);
  if (R != 0 && Bits < 32) {
    std::stringstream M;
    M << "Possible loss of precision in calculating "
      << "multiple-precision Euler Number.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  ASTMPDecimalNode *MPD =
      new ASTMPDecimalNode(&ASTIdentifierNode::EulerNumber, Bits, Gamma);
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  mpfr_clear(Gamma);
  mpfr_clear(Val);
  mpfr_clear(Sinh);
  mpfr_clear(Cosh);
  return MPD;
}

ASTMPDecimalNode *ASTMPDecimalNode::NegEuler(int Bits) {
  mpfr_t Gamma;
  mpfr_t Val;
  mpfr_t Sinh;
  mpfr_t Cosh;

  mpfr_init2(Gamma, Bits);
  mpfr_init2(Val, Bits);
  mpfr_init2(Sinh, Bits);
  mpfr_init2(Cosh, Bits);

  // Euler Number == sinh(1.0) + cosh(1.0).
  mpfr_set_d(Val, 1.0, MPFR_RNDN);

  int R = mpfr_sinh_cosh(Sinh, Cosh, Val, MPFR_RNDN);
  if (R != 0 && Bits < 32) {
    std::stringstream M;
    M << "Possible loss of precision in calculating "
      << "multiple-precision sinh(1) and cosh(1).";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  R = mpfr_add(Gamma, Sinh, Cosh, MPFR_RNDN);
  if (R != 0 && Bits < 32) {
    std::stringstream M;
    M << "Possible loss of precision in calculating "
      << "multiple-precision Euler Number.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  mpfr_t NegGamma;
  mpfr_init2(NegGamma, Bits);

  R = mpfr_mul_d(NegGamma, Gamma, -1.0, MPFR_RNDN);
  if (R != 0 && Bits < 32) {
    std::stringstream M;
    M << "Possible loss of precision in calculating "
      << "multiple-precision negative Euler Number.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  ASTMPDecimalNode *MPD =
      new ASTMPDecimalNode(&ASTIdentifierNode::EulerNumber, Bits, NegGamma);
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  mpfr_clear(Gamma);
  mpfr_clear(Val);
  mpfr_clear(Sinh);
  mpfr_clear(Cosh);
  return MPD;
}

ASTMPDecimalNode *ASTMPDecimalNode::Euler(int Bits, int Prec) {
  int Precision = ASTMPDecimalNode::GetDefaultPrecision();
  ASTMPDecimalNode::SetDefaultPrecision(Prec);
  ASTMPDecimalNode *MPD = ASTMPDecimalNode::Euler(Bits);
  ASTMPDecimalNode::SetDefaultPrecision(Precision);
  return MPD;
}

ASTMPDecimalNode *ASTMPDecimalNode::NegEuler(int Bits, int Prec) {
  int Precision = ASTMPDecimalNode::GetDefaultPrecision();
  ASTMPDecimalNode::SetDefaultPrecision(Prec);
  ASTMPDecimalNode *MPD = ASTMPDecimalNode::NegEuler(Bits);
  ASTMPDecimalNode::SetDefaultPrecision(Precision);
  return MPD;
}

ASTMPComplexNode *ASTMPDecimalNode::ToMPComplex() const {
  ASTMPDecimalNode *MPCI =
      new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(), Bits, 1.0);
  assert(MPCI && "Could not create a valid ASTMPDecimalNode!");

  ASTMPDecimalNode *MPCR =
      new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(), Bits, MPValue);
  assert(MPCI && "Could not create a valid ASTMPDecimalNode!");

  ASTMPComplexNode *MPC = new ASTMPComplexNode(
      ASTIdentifierNode::MPComplex.Clone(), MPCR, MPCI, ASTOpTypeMul, Bits);
  assert(MPC && "Could not create a valid ASTMPComplexNode!");

  return MPC;
}

bool ASTMPDecimalNode::IsImplicitConversion() const {
  return Expr && (Expr->GetASTType() == ASTTypeImplicitConversion);
}

void ASTMPDecimalNode::SetImplicitConversion(
    const ASTImplicitConversionNode *ICX) {
  Expr = ICX;
}

void ASTMPDecimalNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.MangleMPDecimal(this);
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTMPDecimalNode::MangleLiteral() {
  ASTMangler M;
  M.Start();
  M.NumericLiteral(this);
  M.End();
  GetIdentifier()->SetMangledLiteralName(M.AsString());
}

} // namespace QASM
