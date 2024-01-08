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
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTTypes.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <sstream>
#include <string>

namespace QASM {

using DiagLevel = QasmDiagnosticEmitter::DiagLevel;

void ASTMPComplexNode::Evaluate(const ASTMPDecimalNode *R,
                                const ASTMPDecimalNode *I, ASTOpType OT,
                                unsigned NumBits) {
  RBits = R->GetBits();
  IBits = I->GetBits();

  switch (OT) {
  case ASTOpTypeAdd:
    SetBias(
        mpc_set_fr_fr(MPValue, R->GetMPValue(), I->GetMPValue(), MPC_RNDND));
    NE = false;
    return;
    break;
  case ASTOpTypeSub: {
    mpfr_t MPV;
    mpfr_init2(MPV, NumBits);

    (void)mpfr_neg(MPV, I->GetMPValue(), MPFR_RNDN);
    SetBias(mpc_set_fr_fr(MPValue, R->GetMPValue(), MPV, MPC_RNDND));
    mpfr_clear(MPV);
    NE = false;
    return;
  } break;
  case ASTOpTypeMul: {
    mpfr_t MPV;
    mpfr_init2(MPV, NumBits);
    SetBias(mpfr_mul(MPV, R->GetMPValue(), I->GetMPValue(), MPFR_RNDN));

    mpfr_t MPZ;
    mpfr_t MPNZ;
    mpfr_init2(MPZ, NumBits);
    mpfr_init2(MPNZ, NumBits);
    mpfr_set_d(MPZ, 0.0, MPFR_RNDN);
    mpfr_neg(MPNZ, MPZ, MPFR_RNDN);

    if (R->IsNegative() || I->IsNegative())
      SetBias(mpc_set_fr_fr(MPValue, MPNZ, MPV, MPC_RNDND));
    else
      SetBias(mpc_set_fr_fr(MPValue, MPZ, MPV, MPC_RNDND));

    mpfr_clear(MPV);
    mpfr_clear(MPZ);
    mpfr_clear(MPNZ);
    NE = false;
    return;
  } break;
  case ASTOpTypeDiv: {
    mpfr_t MPV;
    mpfr_init2(MPV, NumBits);

    SetBias(mpfr_div(MPV, R->GetMPValue(), I->GetMPValue(), MPFR_RNDN));

    mpfr_t MPZ;
    mpfr_t MPNZ;
    mpfr_init2(MPZ, NumBits);
    mpfr_init2(MPNZ, NumBits);
    mpfr_set_d(MPZ, 0.0, MPFR_RNDN);
    mpfr_neg(MPNZ, MPZ, MPFR_RNDN);

    if (R->IsNegative() || I->IsNegative())
      SetBias(mpc_set_fr_fr(MPValue, MPNZ, MPV, MPC_RNDND));
    else
      SetBias(mpc_set_fr_fr(MPValue, MPZ, MPV, MPC_RNDND));

    mpfr_clear(MPV);
    mpfr_clear(MPZ);
    mpfr_clear(MPNZ);
    NE = false;
    return;
  } break;
  case ASTOpTypeMod: {
    mpfr_t MPVR;
    mpfr_init2(MPVR, NumBits);

    (void)mpfr_fmod(MPVR, R->GetMPValue(), I->GetMPValue(), MPFR_RNDN);

    mpfr_t MPZ;
    mpfr_init2(MPZ, NumBits);
    mpfr_set_d(MPZ, 0.0, MPFR_RNDN);

    SetBias(mpc_set_fr_fr(MPValue, MPZ, MPVR, MPC_RNDND));

    mpfr_clear(MPVR);
    mpfr_clear(MPZ);
    NE = false;
    return;
  } break;
  default: {
    NE = true;
    std::stringstream M;
    M << "Unsupported Complex initialization arithmetic Op "
      << PrintOpTypeEnum(OT);
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return;
  } break;
  }
}

void ASTMPComplexNode::Evaluate(const ASTMPIntegerNode *R,
                                const ASTMPIntegerNode *I, ASTOpType OT,
                                unsigned NumBits) {
  RBits = R->GetBits();
  IBits = I->GetBits();

  switch (OT) {
  case ASTOpTypeAdd:
    SetBias(mpc_set_z_z(MPValue, R->GetMPValue(), I->GetMPValue(), MPC_RNDND));
    NE = false;
    return;
    break;
  case ASTOpTypeSub: {
    mpz_t MPZ;
    mpz_init2(MPZ, NumBits);
    mpz_set_si(MPZ, 0);
    mpz_neg(MPZ, I->GetMPValue());

    SetBias(mpc_set_z_z(MPValue, R->GetMPValue(), MPZ, MPC_RNDND));
    mpz_clear(MPZ);
    NE = false;
    return;
  } break;
  case ASTOpTypeMul: {
    mpz_t MPV;
    mpz_init2(MPV, NumBits);
    mpz_set_si(MPV, 0);

    mpz_mul(MPV, R->GetMPValue(), I->GetMPValue());

    mpz_t MPZ;
    mpz_t MPNZ;
    mpz_init2(MPZ, NumBits);
    mpz_init2(MPNZ, NumBits);
    mpz_set_si(MPZ, 0);
    mpz_neg(MPNZ, MPZ);

    bool NEG = (R->IsNegative() || I->IsNegative()) &&
               !(R->IsNegative() && I->IsNegative());

    SetBias(mpc_set_z_z(MPValue, NEG ? MPNZ : MPZ, MPV, MPC_RNDND));

    mpz_clear(MPV);
    mpz_clear(MPZ);
    mpz_clear(MPNZ);
    NE = false;
    return;
  } break;
  case ASTOpTypeDiv: {
    mpfr_t MPVR;
    mpfr_t MPVI;

    mpfr_init2(MPVR, NumBits);
    mpfr_init2(MPVI, NumBits);

    (void)mpfr_set_z(MPVR, R->GetMPValue(), MPFR_RNDN);
    (void)mpfr_set_z(MPVI, I->GetMPValue(), MPFR_RNDN);

    mpfr_t MPV;
    mpfr_init2(MPV, NumBits);
    (void)mpfr_div(MPV, MPVR, MPVI, MPFR_RNDN);

    mpfr_t MPZ;
    mpfr_t MPNZ;
    mpfr_init2(MPZ, NumBits);
    mpfr_init2(MPNZ, NumBits);
    mpfr_set_d(MPZ, 0.0, MPFR_RNDN);
    mpfr_neg(MPNZ, MPZ, MPFR_RNDN);

    bool NEG = (mpfr_sgn(MPVR) < 0 || mpfr_sgn(MPVI) < 0) &&
               !(mpfr_sgn(MPVR) < 0 && mpfr_sgn(MPVI) < 0);

    SetBias(mpc_set_fr_fr(MPValue, NEG ? MPNZ : MPZ, MPV, MPC_RNDND));

    mpfr_clear(MPVR);
    mpfr_clear(MPVI);
    mpfr_clear(MPV);
    mpfr_clear(MPZ);
    mpfr_clear(MPNZ);
    NE = false;
    return;
  } break;
  case ASTOpTypeMod: {
    mpz_t MPZR;
    mpz_init2(MPZR, NumBits);
    mpz_tdiv_r(MPZR, R->GetMPValue(), I->GetMPValue());

    mpz_t MPZ;
    mpz_init2(MPZ, NumBits);
    mpz_set_si(MPZ, 0L);

    SetBias(mpc_set_z_z(MPValue, MPZ, MPZR, MPC_RNDND));

    mpz_clear(MPZR);
    mpz_clear(MPZ);
    NE = false;
    return;
  } break;
  default: {
    NE = true;
    std::stringstream M;
    M << "Unsupported Complex initialization arithmetic Op "
      << PrintOpTypeEnum(OT);
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  } break;
  }
}

void ASTMPComplexNode::Evaluate(const ASTMPComplexNode *L,
                                const ASTMPComplexNode *R, ASTOpType OT,
                                unsigned NumBits) {
  if (NumBits < L->GetBits() || NumBits < R->GetBits()) {
    std::stringstream M;
    M << "Expression evaluation may result in truncation.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(L), M.str(),
        DiagLevel::Warning);
  }

  switch (OT) {
  case ASTOpTypeAdd:
    SetBias(mpc_add(MPValue, L->GetMPValue(), R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSub:
    SetBias(mpc_sub(MPValue, L->GetMPValue(), R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeMul:
    SetBias(mpc_mul(MPValue, L->GetMPValue(), R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeDiv:
    SetBias(mpc_div(MPValue, L->GetMPValue(), R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypePow:
    SetBias(mpc_pow(MPValue, L->GetMPValue(), R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSqrt:
    SetBias(mpc_sqrt(MPValue, L->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  default: {
    mpc_set_nan(MPValue);
    std::stringstream M;
    M << "Invalid operands to binary expression '" << PrintOpTypeEnum(OT)
      << "'.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  } break;
  }
}

void ASTMPComplexNode::Evaluate(const ASTMPComplexNode *L,
                                const ASTMPDecimalNode *R, ASTOpType OT) {
  switch (OT) {
  case ASTOpTypeAdd:
    SetBias(mpc_add_fr(MPValue, L->GetMPValue(), R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSub:
    SetBias(mpc_sub_fr(MPValue, L->GetMPValue(), R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeMul:
    SetBias(mpc_mul_fr(MPValue, L->GetMPValue(), R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeDiv:
    SetBias(mpc_div_fr(MPValue, L->GetMPValue(), R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypePow:
    SetBias(mpc_pow_fr(MPValue, L->GetMPValue(), R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSqrt:
    SetBias(mpc_sqrt(MPValue, L->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  default: {
    mpc_set_nan(MPValue);
    std::stringstream M;
    M << "Invalid operands to binary expression '" << PrintOpTypeEnum(OT)
      << "'.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  } break;
  }
}

void ASTMPComplexNode::Evaluate(const ASTMPComplexNode *L,
                                const ASTMPIntegerNode *R, ASTOpType OT) {
  RBits = R->GetBits();

  mpfr_t MPVR;
  mpfr_init2(MPVR, R->GetBits());

  (void)mpfr_set_z(MPVR, R->GetMPValue(), MPFR_RNDN);

  switch (OT) {
  case ASTOpTypeAdd:
    SetBias(mpc_add_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSub:
    SetBias(mpc_sub_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeMul:
    SetBias(mpc_mul_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeDiv:
    SetBias(mpc_div_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypePow:
    SetBias(mpc_pow_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSqrt:
    SetBias(mpc_sqrt(MPValue, L->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  default: {
    mpc_set_nan(MPValue);
    std::stringstream M;
    M << "Invalid operands to binary expression '" << PrintOpTypeEnum(OT)
      << "'.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  } break;
  }
}

void ASTMPComplexNode::Evaluate(const ASTMPComplexNode *L,
                                const ASTFloatNode *R, ASTOpType OT) {
  RBits = R->GetBits();

  mpfr_t MPVR;
  mpfr_init2(MPVR, L->GetBits());

  (void)mpfr_set_flt(MPVR, R->GetValue(), MPFR_RNDN);

  switch (OT) {
  case ASTOpTypeAdd:
    SetBias(mpc_add_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSub:
    SetBias(mpc_sub_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeMul:
    SetBias(mpc_mul_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeDiv:
    SetBias(mpc_div_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypePow:
    SetBias(mpc_pow_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSqrt:
    SetBias(mpc_sqrt(MPValue, L->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  default: {
    mpc_set_nan(MPValue);
    std::stringstream M;
    M << "Invalid operands to binary expression '" << PrintOpTypeEnum(OT)
      << "'.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  } break;
  }
}

void ASTMPComplexNode::Evaluate(const ASTMPComplexNode *L, const ASTIntNode *R,
                                ASTOpType OT) {
  RBits = R->GetBits();

  mpfr_t MPVR;
  mpfr_init2(MPVR, L->GetBits());

  if (R->IsSigned())
    (void)mpfr_set_si(MPVR, R->GetSignedValue(), MPFR_RNDN);
  else
    (void)mpfr_set_ui(MPVR, R->GetUnsignedValue(), MPFR_RNDN);

  switch (OT) {
  case ASTOpTypeAdd:
    SetBias(mpc_add_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSub:
    SetBias(mpc_sub_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeMul:
    SetBias(mpc_mul_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeDiv:
    SetBias(mpc_div_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypePow:
    SetBias(mpc_pow_fr(MPValue, L->GetMPValue(), MPVR, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSqrt:
    SetBias(mpc_sqrt(MPValue, L->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  default: {
    mpc_set_nan(MPValue);
    std::stringstream M;
    M << "Invalid operands to binary expression '" << PrintOpTypeEnum(OT)
      << "'.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  } break;
  }
}

void ASTMPComplexNode::Evaluate(const ASTMPDecimalNode *L,
                                const ASTMPComplexNode *R, ASTOpType OT) {
  RBits = L->GetBits();

  switch (OT) {
  case ASTOpTypeAdd:
    SetBias(mpc_add_fr(MPValue, R->GetMPValue(), L->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSub:
    SetBias(mpc_fr_sub(MPValue, L->GetMPValue(), R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeMul:
    SetBias(mpc_mul_fr(MPValue, R->GetMPValue(), L->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeDiv:
    SetBias(mpc_fr_div(MPValue, L->GetMPValue(), R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypePow: {
    mpfr_t MPVR;
    mpfr_init2(MPVR, R->GetBits());
    mpfr_t MPVP;
    mpfr_init2(MPVP, R->GetBits());

    (void)mpc_real(MPVR, R->GetMPValue(), MPFR_RNDN);
    SetBias(mpfr_pow(MPVP, L->GetMPValue(), MPVR, MPFR_RNDN));
    mpc_set_fr(MPValue, MPVP, MPC_RNDND);
    NE = false;

    mpfr_clear(MPVR);
    mpfr_clear(MPVP);
  } break;
  case ASTOpTypeSqrt:
    SetBias(mpc_sqrt(MPValue, R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  default: {
    mpc_set_nan(MPValue);
    std::stringstream M;
    M << "Invalid operands to binary expression '" << PrintOpTypeEnum(OT)
      << "'.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  } break;
  }
}

void ASTMPComplexNode::Evaluate(const ASTMPIntegerNode *L,
                                const ASTMPComplexNode *R, ASTOpType OT) {
  RBits = L->GetBits();

  mpfr_t MPVL;
  mpfr_init2(MPVL, L->GetBits());

  (void)mpfr_set_z(MPVL, L->GetMPValue(), MPFR_RNDN);

  switch (OT) {
  case ASTOpTypeAdd:
    SetBias(mpc_add_fr(MPValue, R->GetMPValue(), MPVL, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSub:
    SetBias(mpc_fr_sub(MPValue, MPVL, R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeMul:
    SetBias(mpc_mul_fr(MPValue, R->GetMPValue(), MPVL, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeDiv:
    SetBias(mpc_fr_div(MPValue, MPVL, R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypePow: {
    mpfr_t MPVR;
    mpfr_init2(MPVR, R->GetBits());
    mpfr_t MPVP;
    mpfr_init2(MPVP, R->GetBits());

    (void)mpc_real(MPVR, R->GetMPValue(), MPFR_RNDN);
    SetBias(mpfr_pow(MPVP, MPVL, MPVR, MPFR_RNDN));
    (void)mpc_set_fr(MPValue, MPVP, MPC_RNDND);
    NE = false;

    mpfr_clear(MPVR);
    mpfr_clear(MPVP);
  } break;
  case ASTOpTypeSqrt:
    SetBias(mpc_sqrt(MPValue, R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  default: {
    mpc_set_nan(MPValue);
    std::stringstream M;
    M << "Invalid operands to binary expression '" << PrintOpTypeEnum(OT)
      << "'.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  } break;
  }

  mpfr_clear(MPVL);
}

void ASTMPComplexNode::Evaluate(const ASTFloatNode *L,
                                const ASTMPComplexNode *R, ASTOpType OT) {
  RBits = L->GetBits();

  mpfr_t MPVL;
  mpfr_init2(MPVL, L->GetBits());

  (void)mpfr_set_flt(MPVL, L->GetValue(), MPFR_RNDN);

  switch (OT) {
  case ASTOpTypeAdd:
    SetBias(mpc_add_fr(MPValue, R->GetMPValue(), MPVL, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSub:
    SetBias(mpc_fr_sub(MPValue, MPVL, R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeMul:
    SetBias(mpc_mul_fr(MPValue, R->GetMPValue(), MPVL, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeDiv:
    SetBias(mpc_fr_div(MPValue, MPVL, R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypePow: {
    mpfr_t MPVR;
    mpfr_init2(MPVR, R->GetBits());
    mpfr_t MPVP;
    mpfr_init2(MPVP, R->GetBits());

    (void)mpc_real(MPVR, R->GetMPValue(), MPFR_RNDN);
    SetBias(mpfr_pow(MPVP, MPVL, MPVR, MPFR_RNDN));
    (void)mpc_set_fr(MPValue, MPVP, MPC_RNDND);
    NE = false;

    mpfr_clear(MPVR);
    mpfr_clear(MPVP);
  } break;
  case ASTOpTypeSqrt:
    SetBias(mpc_sqrt(MPValue, R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  default: {
    mpc_set_nan(MPValue);
    std::stringstream M;
    M << "Invalid operands to binary expression '" << PrintOpTypeEnum(OT)
      << "'.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  } break;
  }

  mpfr_clear(MPVL);
}

void ASTMPComplexNode::Evaluate(const ASTIntNode *L, const ASTMPComplexNode *R,
                                ASTOpType OT) {
  RBits = L->GetBits();

  mpfr_t MPVL;
  mpfr_init2(MPVL, L->GetBits());

  if (L->IsSigned())
    (void)mpfr_set_si(MPVL, L->GetSignedValue(), MPFR_RNDN);
  else
    (void)mpfr_set_ui(MPVL, L->GetUnsignedValue(), MPFR_RNDN);

  switch (OT) {
  case ASTOpTypeAdd:
    SetBias(mpc_add_fr(MPValue, R->GetMPValue(), MPVL, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeSub:
    SetBias(mpc_fr_sub(MPValue, MPVL, R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeMul:
    SetBias(mpc_mul_fr(MPValue, R->GetMPValue(), MPVL, MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypeDiv:
    SetBias(mpc_fr_div(MPValue, MPVL, R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  case ASTOpTypePow: {
    mpfr_t MPVR;
    mpfr_init2(MPVR, R->GetBits());
    mpfr_t MPVP;
    mpfr_init2(MPVP, R->GetBits());

    (void)mpc_real(MPVR, R->GetMPValue(), MPFR_RNDN);
    SetBias(mpfr_pow(MPVP, MPVL, MPVR, MPFR_RNDN));
    mpc_set_fr(MPValue, MPVP, MPC_RNDND);
    NE = false;

    mpfr_clear(MPVP);
    mpfr_clear(MPVR);
  } break;
  case ASTOpTypeSqrt:
    SetBias(mpc_sqrt(MPValue, R->GetMPValue(), MPC_RNDND));
    NE = false;
    break;
  default: {
    mpc_set_nan(MPValue);
    std::stringstream M;
    M << "Invalid operands to binary expression '" << PrintOpTypeEnum(OT)
      << "'.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
  } break;
  }

  mpfr_clear(MPVL);
}

void ASTMPComplexNode::Evaluate(const ASTMPDecimalNode *R,
                                const ASTMPIntegerNode *I, ASTOpType OT,
                                unsigned NumBits) {
  RBits = R->GetBits();
  IBits = I->GetBits();

  mpfr_t MPVI;
  mpfr_init2(MPVI, NumBits);

  (void)mpfr_set_z(MPVI, I->GetMPValue(), MPFR_RNDN);

  ASTMPDecimalNode *MPDI =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, MPVI);
  assert(MPDI && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(R, MPDI, OT, NumBits);
  mpfr_clear(MPVI);
}

void ASTMPComplexNode::Evaluate(const ASTMPIntegerNode *R,
                                const ASTMPDecimalNode *I, ASTOpType OT,
                                unsigned NumBits) {
  RBits = R->GetBits();
  IBits = I->GetBits();

  mpfr_t MPVR;
  mpfr_init2(MPVR, NumBits);

  (void)mpfr_set_z(MPVR, R->GetMPValue(), MPFR_RNDN);

  ASTMPDecimalNode *MPD =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, MPVR);
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(MPD, I, OT, NumBits);
  mpfr_clear(MPVR);
}

void ASTMPComplexNode::Evaluate(const ASTMPIntegerNode *R,
                                const ASTFloatNode *I, ASTOpType OT,
                                unsigned NumBits) {
  RBits = R->GetBits();
  IBits = I->GetBits();

  mpfr_t MPVR;
  mpfr_init2(MPVR, NumBits);

  (void)mpfr_set_z(MPVR, R->GetMPValue(), MPFR_RNDN);

  ASTMPDecimalNode *MPDR =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, MPVR);
  assert(MPDR && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(MPDR, I, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTMPIntegerNode *R,
                                const ASTDoubleNode *I, ASTOpType OT,
                                unsigned NumBits) {
  RBits = R->GetBits();
  IBits = I->GetBits();

  mpfr_t MPVR;
  mpfr_init2(MPVR, NumBits);

  (void)mpfr_set_z(MPVR, R->GetMPValue(), MPFR_RNDN);

  ASTMPDecimalNode *MPDR =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, MPVR);
  assert(MPDR && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(MPDR, I, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTMPIntegerNode *R, const ASTIntNode *I,
                                ASTOpType OT, unsigned NumBits) {
  RBits = R->GetBits();
  IBits = I->GetBits();

  mpfr_t MPVR;
  mpfr_init2(MPVR, NumBits);

  (void)mpfr_set_z(MPVR, R->GetMPValue(), MPFR_RNDN);

  ASTMPDecimalNode *MPDR =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, MPVR);
  assert(MPDR && "Could not create a valid ASTMPDecimalNode!");

  double ID = I->IsSigned() ? static_cast<double>(I->GetSignedValue())
                            : static_cast<double>(I->GetUnsignedValue());

  ASTMPDecimalNode *MPDI =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, ID);
  assert(MPDI && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(MPDR, MPDI, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTFloatNode *R,
                                const ASTMPDecimalNode *I, ASTOpType OT,
                                unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  ASTMPDecimalNode *MPD =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, R->GetValue());
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(MPD, I, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTDoubleNode *R,
                                const ASTMPDecimalNode *I, ASTOpType OT,
                                unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  ASTMPDecimalNode *MPD =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, R->GetValue());
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(MPD, I, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTIntNode *R, const ASTMPDecimalNode *I,
                                ASTOpType OT, unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  double RD = R->IsSigned() ? static_cast<double>(R->GetSignedValue())
                            : static_cast<double>(R->GetUnsignedValue());
  ASTMPDecimalNode *MPD =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, RD);
  assert(MPD && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(MPD, I, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTMPDecimalNode *R,
                                const ASTFloatNode *I, ASTOpType OT,
                                unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  ASTMPDecimalNode *MPDI =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, I->GetValue());
  assert(MPDI && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(R, MPDI, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTMPDecimalNode *R,
                                const ASTDoubleNode *I, ASTOpType OT,
                                unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  ASTMPDecimalNode *MPDI =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, I->GetValue());
  assert(MPDI && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(R, MPDI, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTMPDecimalNode *R, const ASTIntNode *I,
                                ASTOpType OT, unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  double ID = I->IsSigned() ? static_cast<double>(I->GetSignedValue())
                            : static_cast<double>(I->GetUnsignedValue());
  ASTMPDecimalNode *MPDI =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, ID);
  assert(MPDI && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(R, MPDI, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTFloatNode *R,
                                const ASTMPIntegerNode *I, ASTOpType OT,
                                unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  ASTMPDecimalNode *MPDR =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, R->GetValue());
  assert(MPDR && "Could not create a valid ASTMPDecimalNode!");

  mpfr_t MPVR;
  mpfr_init2(MPVR, NumBits);

  (void)mpfr_set_z(MPVR, I->GetMPValue(), MPFR_RNDN);

  ASTMPDecimalNode *MPDI =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, MPVR);
  assert(MPDI && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(MPDR, MPDI, OT, NumBits);
  mpfr_clear(MPVR);
}

void ASTMPComplexNode::Evaluate(const ASTDoubleNode *R,
                                const ASTMPIntegerNode *I, ASTOpType OT,
                                unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  ASTMPDecimalNode *MPDR =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, R->GetValue());
  assert(MPDR && "Could not create a valid ASTMPDecimalNode!");

  mpfr_t MPVR;
  mpfr_init2(MPVR, NumBits);

  (void)mpfr_set_z(MPVR, I->GetMPValue(), MPFR_RNDN);

  ASTMPDecimalNode *MPDI =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, MPVR);
  assert(MPDI && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(MPDR, MPDI, OT, NumBits);
  mpfr_clear(MPVR);
}

void ASTMPComplexNode::Evaluate(const ASTIntNode *R, const ASTMPIntegerNode *I,
                                ASTOpType OT, unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  double RD = R->IsSigned() ? static_cast<double>(R->GetSignedValue())
                            : static_cast<double>(R->GetUnsignedValue());
  ASTMPDecimalNode *MPDR =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, RD);
  assert(MPDR && "Could not create a valid ASTMPDecimalNode!");

  mpfr_t MPVR;
  mpfr_init2(MPVR, NumBits);

  (void)mpfr_set_z(MPVR, I->GetMPValue(), MPFR_RNDN);

  ASTMPDecimalNode *MPDI =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, MPVR);
  assert(MPDI && "Could not create a valid ASTMPDecimalNode!");

  NE = false;
  Evaluate(MPDR, MPDI, OT, NumBits);
  mpfr_clear(MPVR);
}

void ASTMPComplexNode::Evaluate(const ASTFloatNode *R, const ASTFloatNode *I,
                                ASTOpType OT, unsigned NumBits) {
  RBits = R->GetBits();

  ASTMPDecimalNode *MPDR =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, R->GetValue());
  ASTMPDecimalNode *MPDI =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, I->GetValue());

  NE = false;
  Evaluate(MPDR, MPDI, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTDoubleNode *R, const ASTDoubleNode *I,
                                ASTOpType OT, unsigned NumBits) {
  RBits = R->GetBits();

  ASTMPDecimalNode *MPDR =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, R->GetValue());
  ASTMPDecimalNode *MPDI =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, I->GetValue());

  NE = false;
  Evaluate(MPDR, MPDI, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTIntNode *R, const ASTIntNode *I,
                                ASTOpType OT, unsigned NumBits) {
  RBits = R->GetBits();

  ASTMPIntegerNode *MPDR =
      new ASTMPIntegerNode(&ASTIdentifierNode::MPInt, R, NumBits);
  ASTMPIntegerNode *MPDI =
      new ASTMPIntegerNode(&ASTIdentifierNode::MPInt, I, NumBits);

  NE = false;
  Evaluate(MPDR, MPDI, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTDoubleNode *R, const ASTIntNode *I,
                                ASTOpType OT, unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  ASTMPDecimalNode *MPDR =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, R->GetValue());
  ASTMPIntegerNode *MPDI =
      new ASTMPIntegerNode(&ASTIdentifierNode::MPInt, I, NumBits);

  NE = false;
  Evaluate(MPDR, MPDI, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTIntNode *R, const ASTDoubleNode *I,
                                ASTOpType OT, unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  ASTMPIntegerNode *MPDR =
      new ASTMPIntegerNode(&ASTIdentifierNode::MPInt, R, NumBits);
  ASTMPDecimalNode *MPDI =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, I->GetValue());

  NE = false;
  Evaluate(MPDR, MPDI, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTFloatNode *R, const ASTIntNode *I,
                                ASTOpType OT, unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  ASTMPDecimalNode *MPDR =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, R->GetValue());
  ASTMPIntegerNode *MPDI =
      new ASTMPIntegerNode(&ASTIdentifierNode::MPInt, I, NumBits);

  NE = false;
  Evaluate(MPDR, MPDI, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTIntNode *R, const ASTFloatNode *I,
                                ASTOpType OT, unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  ASTMPIntegerNode *MPDR =
      new ASTMPIntegerNode(&ASTIdentifierNode::MPInt, R, NumBits);
  ASTMPDecimalNode *MPDI =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, I->GetValue());

  NE = false;
  Evaluate(MPDR, MPDI, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTFloatNode *R, const ASTDoubleNode *I,
                                ASTOpType OT, unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  ASTMPDecimalNode *MPDR = new ASTMPDecimalNode(
      &ASTIdentifierNode::MPDec, NumBits, static_cast<double>(R->GetValue()));
  ASTMPDecimalNode *MPDI =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, I->GetValue());

  NE = false;
  Evaluate(MPDR, MPDI, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTDoubleNode *R, const ASTFloatNode *I,
                                ASTOpType OT, unsigned NumBits) {
  RBits = std::max(R->GetBits(), I->GetBits());

  ASTMPDecimalNode *MPDR =
      new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, NumBits, R->GetValue());
  ASTMPDecimalNode *MPDI = new ASTMPDecimalNode(
      &ASTIdentifierNode::MPDec, NumBits, static_cast<double>(I->GetValue()));

  NE = false;
  Evaluate(MPDR, MPDI, OT, NumBits);
}

void ASTMPComplexNode::Evaluate(const ASTIdentifierNode *RId,
                                const ASTIdentifierNode *IId, ASTOpType OT,
                                unsigned NumBits) {
  if (!RId || !IId)
    return;

  ASTSymbolTableEntry *RSTE = ASTSymbolTable::Instance().Lookup(
      RId, RId->GetBits(), RId->GetSymbolType());
  ASTSymbolTableEntry *ISTE = ASTSymbolTable::Instance().Lookup(
      IId, IId->GetBits(), IId->GetSymbolType());

  if (!RSTE || !ISTE)
    return;

  RBits = std::max(RId->GetBits(), IId->GetBits());

  if (RSTE->GetValueType() == ISTE->GetValueType()) {
    switch (RSTE->GetValueType()) {
    case ASTTypeMPDecimal: {
      Evaluate(RSTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
               ISTE->GetValue()->GetValue<ASTMPDecimalNode *>(), OT, NumBits);
      return;
    } break;
    case ASTTypeMPInteger: {
      Evaluate(RSTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
               ISTE->GetValue()->GetValue<ASTMPIntegerNode *>(), OT, NumBits);
      return;
    } break;
    case ASTTypeInt: {
      Evaluate(RSTE->GetValue()->GetValue<ASTIntNode *>(),
               ISTE->GetValue()->GetValue<ASTIntNode *>(), OT);
      return;
    } break;
    case ASTTypeFloat: {
      Evaluate(RSTE->GetValue()->GetValue<ASTFloatNode *>(),
               ISTE->GetValue()->GetValue<ASTFloatNode *>(), OT);
      return;
    } break;
    case ASTTypeDouble: {
      Evaluate(RSTE->GetValue()->GetValue<ASTDoubleNode *>(),
               ISTE->GetValue()->GetValue<ASTDoubleNode *>(), OT);
      return;
    } break;
    case ASTTypeMPComplex: {
      Evaluate(RSTE->GetValue()->GetValue<ASTMPComplexNode *>(),
               ISTE->GetValue()->GetValue<ASTMPComplexNode *>(), OT, NumBits);
    } break;
    default: {
      NE = true;
      std::stringstream M;
      M << "Impossible initialization of complex value from Types R="
        << PrintTypeEnum(RSTE->GetValueType())
        << " I=" << PrintTypeEnum(ISTE->GetValueType()) << "!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    } break;
    }
  } else if (RSTE->GetValueType() == ASTTypeMPDecimal &&
             ISTE->GetValueType() == ASTTypeMPInteger) {
    Evaluate(RSTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
             ISTE->GetValue()->GetValue<ASTMPIntegerNode *>(), OT, NumBits);
    return;
  } else if (RSTE->GetValueType() == ASTTypeMPInteger &&
             ISTE->GetValueType() == ASTTypeMPDecimal) {
    Evaluate(RSTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
             ISTE->GetValue()->GetValue<ASTMPDecimalNode *>(), OT, NumBits);
    return;
  } else if (RSTE->GetValueType() == ASTTypeDouble &&
             ISTE->GetValueType() == ASTTypeInt) {
    Evaluate(RSTE->GetValue()->GetValue<ASTDoubleNode *>(),
             ISTE->GetValue()->GetValue<ASTIntNode *>(), OT);
    return;
  } else if (RSTE->GetValueType() == ASTTypeInt &&
             ISTE->GetValueType() == ASTTypeDouble) {
    Evaluate(RSTE->GetValue()->GetValue<ASTIntNode *>(),
             ISTE->GetValue()->GetValue<ASTDoubleNode *>(), OT);
    return;
  } else if (RSTE->GetValueType() == ASTTypeFloat &&
             ISTE->GetValueType() == ASTTypeInt) {
    Evaluate(RSTE->GetValue()->GetValue<ASTFloatNode *>(),
             ISTE->GetValue()->GetValue<ASTIntNode *>(), OT);
    return;
  } else if (RSTE->GetValueType() == ASTTypeInt &&
             ISTE->GetValueType() == ASTTypeFloat) {
    Evaluate(RSTE->GetValue()->GetValue<ASTIntNode *>(),
             ISTE->GetValue()->GetValue<ASTFloatNode *>(), OT);
    return;
  } else if (RSTE->GetValueType() == ASTTypeDouble &&
             ISTE->GetValueType() == ASTTypeFloat) {
    Evaluate(RSTE->GetValue()->GetValue<ASTDoubleNode *>(),
             ISTE->GetValue()->GetValue<ASTFloatNode *>(), OT);
    return;
  } else if (RSTE->GetValueType() == ASTTypeFloat &&
             ISTE->GetValueType() == ASTTypeDouble) {
    Evaluate(RSTE->GetValue()->GetValue<ASTFloatNode *>(),
             ISTE->GetValue()->GetValue<ASTDoubleNode *>(), OT);
  } else {
    NE = true;
    std::stringstream M;
    M << "Impossible Complex value initialization from R="
      << PrintTypeEnum(RSTE->GetValueType())
      << " I=" << PrintTypeEnum(ISTE->GetValueType()) << ".";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return;
  }
}

void ASTMPComplexNode::Evaluate(const ASTComplexExpressionNode *E,
                                unsigned NumBits) {
  RTy = E->GetLeftASTType();
  ITy = E->GetRightASTType();

  if (RTy == ITy) {
    switch (RTy) {
    case ASTTypeMPDecimal: {
      Evaluate(
          dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetLeft()),
          dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeMPInteger: {
      Evaluate(
          dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetLeft()),
          dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeInt: {
      Evaluate(dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetLeft()),
               dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetRight()),
               E->GetBinaryOp()->GetOpType());
      return;
    } break;
    case ASTTypeFloat: {
      Evaluate(dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetLeft()),
               dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetRight()),
               E->GetBinaryOp()->GetOpType());
      return;
    } break;
    case ASTTypeDouble: {
      Evaluate(
          dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetLeft()),
          dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType());
      return;
    } break;
    case ASTTypeIdentifier: {
      const ASTIdentifierNode *RId = dynamic_cast<const ASTIdentifierNode *>(
          E->GetBinaryOp()->GetLeft()->GetIdentifier());
      const ASTIdentifierNode *IId = dynamic_cast<const ASTIdentifierNode *>(
          E->GetBinaryOp()->GetRight()->GetIdentifier());

      if (!RId || !IId)
        return;

      ASTSymbolTableEntry *RSTE = ASTSymbolTable::Instance().Lookup(
          RId, RId->GetBits(), RId->GetSymbolType());
      ASTSymbolTableEntry *ISTE = ASTSymbolTable::Instance().Lookup(
          IId, IId->GetBits(), IId->GetSymbolType());

      if (!RSTE || !ISTE)
        return;

      RTy = RId->GetSymbolType();
      ITy = IId->GetSymbolType();
      RBits = std::max(RId->GetBits(), IId->GetBits());

      if (RSTE->GetValueType() == ISTE->GetValueType()) {
        switch (RSTE->GetValueType()) {
        case ASTTypeMPDecimal: {
          Evaluate(RSTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
                   ISTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
                   E->GetBinaryOp()->GetOpType(), NumBits);
          return;
        } break;
        case ASTTypeMPInteger: {
          Evaluate(RSTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
                   ISTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
                   E->GetBinaryOp()->GetOpType(), NumBits);
          return;
        } break;
        case ASTTypeMPComplex: {
          Evaluate(RSTE->GetValue()->GetValue<ASTMPComplexNode *>(),
                   ISTE->GetValue()->GetValue<ASTMPComplexNode *>(),
                   E->GetBinaryOp()->GetOpType(), NumBits);
          return;
        } break;
        case ASTTypeInt: {
          Evaluate(RSTE->GetValue()->GetValue<ASTIntNode *>(),
                   ISTE->GetValue()->GetValue<ASTIntNode *>(),
                   E->GetBinaryOp()->GetOpType());
          return;
        } break;
        case ASTTypeFloat: {
          Evaluate(RSTE->GetValue()->GetValue<ASTFloatNode *>(),
                   ISTE->GetValue()->GetValue<ASTFloatNode *>(),
                   E->GetBinaryOp()->GetOpType());
          return;
        } break;
        case ASTTypeDouble: {
          Evaluate(RSTE->GetValue()->GetValue<ASTDoubleNode *>(),
                   ISTE->GetValue()->GetValue<ASTDoubleNode *>(),
                   E->GetBinaryOp()->GetOpType());
          return;
        } break;
        default: {
          NE = true;
          std::stringstream M;
          M << "Impossible initialization of complex value from Types R="
            << PrintTypeEnum(RSTE->GetValueType())
            << " I=" << PrintTypeEnum(ISTE->GetValueType()) << "!";
          QasmDiagnosticEmitter::Instance().EmitDiagnostic(
              DIAGLineCounter::Instance().GetLocation(), M.str(),
              DiagLevel::Error);
        } break;
        }
      } else if (RSTE->GetValueType() == ASTTypeMPDecimal &&
                 ISTE->GetValueType() == ASTTypeMPInteger) {
        Evaluate(RSTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
                 ISTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
                 E->GetBinaryOp()->GetOpType(), NumBits);
        return;
      } else if (RSTE->GetValueType() == ASTTypeMPInteger &&
                 ISTE->GetValueType() == ASTTypeMPDecimal) {
        Evaluate(RSTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
                 ISTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
                 E->GetBinaryOp()->GetOpType(), NumBits);
        return;
      } else if (RSTE->GetValueType() == ASTTypeDouble &&
                 ISTE->GetValueType() == ASTTypeInt) {
        Evaluate(RSTE->GetValue()->GetValue<ASTDoubleNode *>(),
                 ISTE->GetValue()->GetValue<ASTIntNode *>(),
                 E->GetBinaryOp()->GetOpType());
        return;
      } else if (RSTE->GetValueType() == ASTTypeInt &&
                 ISTE->GetValueType() == ASTTypeDouble) {
        Evaluate(RSTE->GetValue()->GetValue<ASTIntNode *>(),
                 ISTE->GetValue()->GetValue<ASTDoubleNode *>(),
                 E->GetBinaryOp()->GetOpType());
        return;
      } else if (RSTE->GetValueType() == ASTTypeFloat &&
                 ISTE->GetValueType() == ASTTypeInt) {
        Evaluate(RSTE->GetValue()->GetValue<ASTFloatNode *>(),
                 ISTE->GetValue()->GetValue<ASTIntNode *>(),
                 E->GetBinaryOp()->GetOpType());
        return;
      } else if (RSTE->GetValueType() == ASTTypeInt &&
                 ISTE->GetValueType() == ASTTypeFloat) {
        Evaluate(RSTE->GetValue()->GetValue<ASTIntNode *>(),
                 ISTE->GetValue()->GetValue<ASTFloatNode *>(),
                 E->GetBinaryOp()->GetOpType());
        return;
      } else if (RSTE->GetValueType() == ASTTypeMPComplex &&
                 ISTE->GetValueType() == ASTTypeMPDecimal) {
        Evaluate(RSTE->GetValue()->GetValue<ASTMPComplexNode *>(),
                 ISTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
                 E->GetBinaryOp()->GetOpType());
        return;
      } else if (RSTE->GetValueType() == ASTTypeMPComplex &&
                 ISTE->GetValueType() == ASTTypeMPInteger) {
        Evaluate(RSTE->GetValue()->GetValue<ASTMPComplexNode *>(),
                 ISTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
                 E->GetBinaryOp()->GetOpType());
        return;
      } else if (RSTE->GetValueType() == ASTTypeMPComplex &&
                 ISTE->GetValueType() == ASTTypeFloat) {
        Evaluate(RSTE->GetValue()->GetValue<ASTMPComplexNode *>(),
                 ISTE->GetValue()->GetValue<ASTFloatNode *>(),
                 E->GetBinaryOp()->GetOpType());
        return;
      } else if (RSTE->GetValueType() == ASTTypeMPComplex &&
                 ISTE->GetValueType() == ASTTypeInt) {
        // FIXME: IMPLEMENT.
        Evaluate(RSTE->GetValue()->GetValue<ASTMPComplexNode *>(),
                 ISTE->GetValue()->GetValue<ASTIntNode *>(),
                 E->GetBinaryOp()->GetOpType());
        return;
      } else if (RSTE->GetValueType() == ASTTypeMPDecimal &&
                 ISTE->GetValueType() == ASTTypeMPComplex) {
        // FIXME: IMPLEMENT.
        Evaluate(RSTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
                 ISTE->GetValue()->GetValue<ASTMPComplexNode *>(),
                 E->GetBinaryOp()->GetOpType());
        return;
      } else if (RSTE->GetValueType() == ASTTypeMPInteger &&
                 ISTE->GetValueType() == ASTTypeMPComplex) {
        // FIXME: IMPLEMENT.
        Evaluate(RSTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
                 ISTE->GetValue()->GetValue<ASTMPComplexNode *>(),
                 E->GetBinaryOp()->GetOpType());
        return;
      } else if (RSTE->GetValueType() == ASTTypeFloat &&
                 ISTE->GetValueType() == ASTTypeMPComplex) {
        // FIXME: IMPLEMENT.
        Evaluate(RSTE->GetValue()->GetValue<ASTFloatNode *>(),
                 ISTE->GetValue()->GetValue<ASTMPComplexNode *>(),
                 E->GetBinaryOp()->GetOpType());
        return;
      } else if (RSTE->GetValueType() == ASTTypeInt &&
                 ISTE->GetValueType() == ASTTypeMPComplex) {
        // FIXME: IMPLEMENT.
        Evaluate(RSTE->GetValue()->GetValue<ASTIntNode *>(),
                 ISTE->GetValue()->GetValue<ASTMPComplexNode *>(),
                 E->GetBinaryOp()->GetOpType());
        return;
      } else {
        NE = true;
        std::stringstream M;
        M << "Impossible Complex value initialization from R="
          << PrintTypeEnum(RSTE->GetValueType())
          << " I=" << PrintTypeEnum(ISTE->GetValueType()) << ".";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
            DIAGLineCounter::Instance().GetLocation(), M.str(),
            DiagLevel::Error);
        return;
      }
    } break;
    case ASTTypeBinaryOp: {
      const ASTBinaryOpNode *LOp =
          dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetLeft());
      const ASTBinaryOpNode *ROp =
          dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetRight());

      ASTMPDecimalNode *LDV =
          new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
      assert(LDV && "Could not create a valid ASTMPDecimalNode!");

      ASTMPDecimalNode *RDV =
          new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
      assert(RDV && "Could not create a valid ASTMPDecimalNode!");

      Evaluate(LDV, RDV, E->GetBinaryOp()->GetOpType(), 128U);
      NE = true;
      return;
    } break;
    case ASTTypeUnaryOp: {
      const ASTUnaryOpNode *LOp =
          dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetLeft());
      const ASTUnaryOpNode *ROp =
          dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetRight());

      ASTMPDecimalNode *LDV =
          new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
      assert(LDV && "Could not create a valid ASTMPDecimalNode!");

      ASTMPDecimalNode *RDV =
          new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
      assert(RDV && "Could not create a valid ASTMPDecimalNode!");

      Evaluate(LDV, RDV, E->GetBinaryOp()->GetOpType(), 128U);
      NE = true;
      return;
    } break;
    default: {
      NE = true;
      std::stringstream M;
      M << "Impossible initialization of complex value from Types R="
        << PrintTypeEnum(RTy) << " I=" << PrintTypeEnum(ITy) << "!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    } break;
    }
  } else if (RTy == ASTTypeMPDecimal && ITy == ASTTypeMPInteger) {
    Evaluate(
        dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetLeft()),
        dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetRight()),
        E->GetBinaryOp()->GetOpType(), NumBits);
    return;
  } else if (RTy == ASTTypeMPInteger && ITy == ASTTypeMPDecimal) {
    Evaluate(
        dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetLeft()),
        dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetRight()),
        E->GetBinaryOp()->GetOpType(), NumBits);
    return;
  } else if (RTy == ASTTypeDouble && ITy == ASTTypeInt) {
    Evaluate(dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetLeft()),
             dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetRight()),
             E->GetBinaryOp()->GetOpType());
    return;
  } else if (RTy == ASTTypeInt && ITy == ASTTypeDouble) {
    Evaluate(dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetLeft()),
             dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetRight()),
             E->GetBinaryOp()->GetOpType());
    return;
  } else if (RTy == ASTTypeFloat && ITy == ASTTypeInt) {
    Evaluate(dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetLeft()),
             dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetRight()),
             E->GetBinaryOp()->GetOpType());
    return;
  } else if (RTy == ASTTypeInt && ITy == ASTTypeFloat) {
    Evaluate(dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetLeft()),
             dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetRight()),
             E->GetBinaryOp()->GetOpType());
    return;
  } else if (RTy == ASTTypeDouble && ITy == ASTTypeFloat) {
    Evaluate(dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetLeft()),
             dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetRight()),
             E->GetBinaryOp()->GetOpType());
    return;
  } else if (RTy == ASTTypeFloat && ITy == ASTTypeDouble) {
    Evaluate(dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetLeft()),
             dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetRight()),
             E->GetBinaryOp()->GetOpType());
    return;
  } else if (RTy == ASTTypeBinaryOp && ITy == ASTTypeUnaryOp) {
    const ASTBinaryOpNode *LOp =
        dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetLeft());
    const ASTUnaryOpNode *ROp =
        dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetRight());

    ASTMPDecimalNode *LDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
    assert(LDV && "Could not create a valid ASTMPDecimalNode!");

    ASTMPDecimalNode *RDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
    assert(RDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(LDV, RDV, E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
    return;
  } else if (RTy == ASTTypeUnaryOp && ITy == ASTTypeBinaryOp) {
    const ASTUnaryOpNode *LOp =
        dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetLeft());
    const ASTBinaryOpNode *ROp =
        dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetLeft());

    ASTMPDecimalNode *LDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
    assert(LDV && "Could not create a valid ASTMPDecimalNode!");

    ASTMPDecimalNode *RDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
    assert(RDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(LDV, RDV, E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
    return;
  } else if (RTy == ASTTypeInt && ITy == ASTTypeBinaryOp) {
    const ASTBinaryOpNode *ROp =
        dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetRight());

    ASTMPDecimalNode *RDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
    assert(RDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetLeft()), RDV,
             E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeBinaryOp && ITy == ASTTypeInt) {
    const ASTBinaryOpNode *LOp =
        dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetLeft());
    ASTMPDecimalNode *LDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
    assert(LDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(LDV,
             dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetRight()),
             E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeInt && ITy == ASTTypeUnaryOp) {
    const ASTUnaryOpNode *ROp =
        dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetRight());
    assert(ROp && "Could not dynamic_cast to a valid ASTUnaryOpNode!");

    ASTMPDecimalNode *RDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
    assert(RDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetLeft()), RDV,
             E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeUnaryOp && ITy == ASTTypeInt) {
    const ASTUnaryOpNode *LOp =
        dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetLeft());
    assert(LOp && "Could not dynamic_cast to a valid ASTUnaryOpNode!");

    ASTMPDecimalNode *LDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
    assert(LDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(LDV,
             dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetRight()),
             E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeFloat && ITy == ASTTypeBinaryOp) {
    const ASTBinaryOpNode *ROp =
        dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetRight());

    ASTMPDecimalNode *RDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
    assert(RDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetLeft()),
             RDV, E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeBinaryOp && ITy == ASTTypeFloat) {
    const ASTBinaryOpNode *LOp =
        dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetLeft());
    ASTMPDecimalNode *LDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
    assert(LDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(LDV,
             dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetRight()),
             E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeFloat && ITy == ASTTypeUnaryOp) {
    const ASTUnaryOpNode *ROp =
        dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetRight());
    assert(ROp && "Could not dynamic_cast to a valid ASTUnaryOpNode!");

    ASTMPDecimalNode *RDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
    assert(RDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetLeft()),
             RDV, E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeUnaryOp && ITy == ASTTypeFloat) {
    const ASTUnaryOpNode *LOp =
        dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetLeft());
    assert(LOp && "Could not dynamic_cast to a valid ASTUnaryOpNode!");

    ASTMPDecimalNode *LDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
    assert(LDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(LDV,
             dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetRight()),
             E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeDouble && ITy == ASTTypeBinaryOp) {
    const ASTBinaryOpNode *ROp =
        dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetRight());

    ASTMPDecimalNode *RDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
    assert(RDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetLeft()),
             RDV, E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeBinaryOp && ITy == ASTTypeDouble) {
    const ASTBinaryOpNode *LOp =
        dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetLeft());
    ASTMPDecimalNode *LDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
    assert(LDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(LDV,
             dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetRight()),
             E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeDouble && ITy == ASTTypeUnaryOp) {
    const ASTUnaryOpNode *ROp =
        dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetRight());
    assert(ROp && "Could not dynamic_cast to a valid ASTUnaryOpNode!");

    ASTMPDecimalNode *RDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
    assert(RDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetLeft()),
             RDV, E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeUnaryOp && ITy == ASTTypeDouble) {
    const ASTUnaryOpNode *LOp =
        dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetLeft());
    assert(LOp && "Could not dynamic_cast to a valid ASTUnaryOpNode!");

    ASTMPDecimalNode *LDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
    assert(LDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(LDV,
             dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetRight()),
             E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeMPInteger && ITy == ASTTypeBinaryOp) {
    const ASTBinaryOpNode *ROp =
        dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetRight());

    ASTMPDecimalNode *RDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
    assert(RDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(
        dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetLeft()),
        RDV, E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeBinaryOp && ITy == ASTTypeMPInteger) {
    const ASTBinaryOpNode *LOp =
        dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetLeft());
    ASTMPDecimalNode *LDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
    assert(LDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(
        LDV,
        dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetRight()),
        E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeMPInteger && ITy == ASTTypeUnaryOp) {
    const ASTUnaryOpNode *ROp =
        dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetRight());
    assert(ROp && "Could not dynamic_cast to a valid ASTUnaryOpNode!");

    ASTMPDecimalNode *RDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
    assert(RDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(
        dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetLeft()),
        RDV, E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeUnaryOp && ITy == ASTTypeMPInteger) {
    const ASTUnaryOpNode *LOp =
        dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetLeft());
    assert(LOp && "Could not dynamic_cast to a valid ASTUnaryOpNode!");

    ASTMPDecimalNode *LDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
    assert(LDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(
        LDV,
        dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetRight()),
        E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeMPDecimal && ITy == ASTTypeBinaryOp) {
    const ASTBinaryOpNode *ROp =
        dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetRight());

    ASTMPDecimalNode *RDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
    assert(RDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(
        dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetLeft()),
        RDV, E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeBinaryOp && ITy == ASTTypeMPDecimal) {
    const ASTBinaryOpNode *LOp =
        dynamic_cast<const ASTBinaryOpNode *>(E->GetBinaryOp()->GetLeft());
    ASTMPDecimalNode *LDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
    assert(LDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(
        LDV,
        dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetRight()),
        E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeMPDecimal && ITy == ASTTypeUnaryOp) {
    const ASTUnaryOpNode *ROp =
        dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetRight());
    assert(ROp && "Could not dynamic_cast to a valid ASTUnaryOpNode!");

    ASTMPDecimalNode *RDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, ROp);
    assert(RDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(
        dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetLeft()),
        RDV, E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeUnaryOp && ITy == ASTTypeMPDecimal) {
    const ASTUnaryOpNode *LOp =
        dynamic_cast<const ASTUnaryOpNode *>(E->GetBinaryOp()->GetLeft());
    assert(LOp && "Could not dynamic_cast to a valid ASTUnaryOpNode!");

    ASTMPDecimalNode *LDV =
        new ASTMPDecimalNode(&ASTIdentifierNode::MPDec, 128, LOp);
    assert(LDV && "Could not create a valid ASTMPDecimalNode!");

    Evaluate(
        LDV,
        dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetRight()),
        E->GetBinaryOp()->GetOpType(), 128U);
    NE = true;
  } else if (RTy == ASTTypeIdentifier && ITy == ASTTypeMPDecimal) {
    const ASTIdentifierNode *RId = dynamic_cast<const ASTIdentifierNode *>(
        E->GetBinaryOp()->GetLeft()->GetIdentifier());
    if (!RId) {
      NE = true;
      std::stringstream M;
      M << "Identifier does not have a corresponding known Symbol!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ASTSymbolTableEntry *RSTE = ASTSymbolTable::Instance().Lookup(
        RId, RId->GetBits(), RId->GetSymbolType());
    if (!RSTE) {
      NE = true;
      std::stringstream M;
      M << "Identifier " << RId->GetName() << " does not have a "
        << "SymbolTable Entry!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    RTy = RId->GetSymbolType();

    switch (RSTE->GetValueType()) {
    case ASTTypeMPDecimal: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
          dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeMPInteger: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
          dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeFloat: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTFloatNode *>(),
          dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeDouble: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTDoubleNode *>(),
          dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeInt: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTIntNode *>(),
          dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    default: {
      NE = true;
      std::stringstream M;
      M << "Impossible initialization of complex value from Types R="
        << PrintTypeEnum(RTy) << " I=" << PrintTypeEnum(ITy) << "!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    } break;
    }
  } else if (RTy == ASTTypeIdentifier && ITy == ASTTypeMPInteger) {
    const ASTIdentifierNode *RId = dynamic_cast<const ASTIdentifierNode *>(
        E->GetBinaryOp()->GetLeft()->GetIdentifier());
    if (!RId) {
      NE = true;
      std::stringstream M;
      M << "Identifier does not have a corresponding known Symbol!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ASTSymbolTableEntry *RSTE = ASTSymbolTable::Instance().Lookup(
        RId, RId->GetBits(), RId->GetSymbolType());
    if (!RSTE) {
      NE = true;
      std::stringstream M;
      M << "Identifier " << RId->GetName() << " does not have a "
        << "SymbolTable Entry!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    RTy = RId->GetSymbolType();

    switch (RSTE->GetValueType()) {
    case ASTTypeMPDecimal: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
          dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeMPInteger: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
          dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeFloat: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTFloatNode *>(),
          dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeDouble: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTDoubleNode *>(),
          dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeInt: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTIntNode *>(),
          dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    default: {
      NE = true;
      std::stringstream M;
      M << "Impossible initialization of complex value from Types R="
        << PrintTypeEnum(RTy) << " I=" << PrintTypeEnum(ITy) << "!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    } break;
    }
  } else if (RTy == ASTTypeIdentifier && ITy == ASTTypeFloat) {
    const ASTIdentifierNode *RId = dynamic_cast<const ASTIdentifierNode *>(
        E->GetBinaryOp()->GetLeft()->GetIdentifier());
    if (!RId) {
      NE = true;
      std::stringstream M;
      M << "Identifier does not have a corresponding known Symbol!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ASTSymbolTableEntry *RSTE = ASTSymbolTable::Instance().Lookup(
        RId, RId->GetBits(), RId->GetSymbolType());
    if (!RSTE) {
      NE = true;
      std::stringstream M;
      M << "Identifier " << RId->GetName() << " does not have a "
        << "SymbolTable Entry!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    RTy = RId->GetSymbolType();

    switch (RSTE->GetValueType()) {
    case ASTTypeMPDecimal: {
      Evaluate(RSTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
               dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetRight()),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeMPInteger: {
      Evaluate(RSTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
               dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetRight()),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeFloat: {
      Evaluate(RSTE->GetValue()->GetValue<ASTFloatNode *>(),
               dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetRight()),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeDouble: {
      Evaluate(RSTE->GetValue()->GetValue<ASTDoubleNode *>(),
               dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetRight()),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeInt: {
      Evaluate(RSTE->GetValue()->GetValue<ASTIntNode *>(),
               dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetRight()),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    default: {
      NE = true;
      std::stringstream M;
      M << "Impossible initialization of complex value from Types R="
        << PrintTypeEnum(RTy) << " I=" << PrintTypeEnum(ITy) << "!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    } break;
    }
  } else if (RTy == ASTTypeIdentifier && ITy == ASTTypeDouble) {
    const ASTIdentifierNode *RId = dynamic_cast<const ASTIdentifierNode *>(
        E->GetBinaryOp()->GetLeft()->GetIdentifier());
    if (!RId) {
      NE = true;
      std::stringstream M;
      M << "Identifier does not have a corresponding known Symbol!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ASTSymbolTableEntry *RSTE = ASTSymbolTable::Instance().Lookup(
        RId, RId->GetBits(), RId->GetSymbolType());
    if (!RSTE) {
      NE = true;
      std::stringstream M;
      M << "Identifier " << RId->GetName() << " does not have a "
        << "SymbolTable Entry!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    RTy = RId->GetSymbolType();

    switch (RSTE->GetValueType()) {
    case ASTTypeMPDecimal: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
          dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeMPInteger: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
          dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeFloat: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTFloatNode *>(),
          dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeDouble: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTDoubleNode *>(),
          dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeInt: {
      Evaluate(
          RSTE->GetValue()->GetValue<ASTIntNode *>(),
          dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetRight()),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    default: {
      std::stringstream M;
      NE = true;
      M << "Impossible initialization of complex value from Types R="
        << PrintTypeEnum(RTy) << " I=" << PrintTypeEnum(ITy) << "!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    } break;
    }
  } else if (RTy == ASTTypeIdentifier && ITy == ASTTypeInt) {
    const ASTIdentifierNode *RId = dynamic_cast<const ASTIdentifierNode *>(
        E->GetBinaryOp()->GetLeft()->GetIdentifier());
    if (!RId) {
      NE = true;
      std::stringstream M;
      M << "Identifier does not have a corresponding known Symbol!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ASTSymbolTableEntry *RSTE = ASTSymbolTable::Instance().Lookup(
        RId, RId->GetBits(), RId->GetSymbolType());
    if (!RSTE) {
      NE = true;
      std::stringstream M;
      M << "Identifier " << RId->GetName() << " does not have a "
        << "SymbolTable Entry!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    RTy = RId->GetSymbolType();

    switch (RSTE->GetValueType()) {
    case ASTTypeMPDecimal: {
      Evaluate(RSTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
               dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetRight()),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeMPInteger: {
      Evaluate(RSTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
               dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetRight()),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeFloat: {
      Evaluate(RSTE->GetValue()->GetValue<ASTFloatNode *>(),
               dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetRight()),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeDouble: {
      Evaluate(RSTE->GetValue()->GetValue<ASTDoubleNode *>(),
               dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetRight()),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeInt: {
      Evaluate(RSTE->GetValue()->GetValue<ASTIntNode *>(),
               dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetRight()),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    default: {
      NE = true;
      std::stringstream M;
      M << "Impossible initialization of complex value from Types R="
        << PrintTypeEnum(RTy) << " I=" << PrintTypeEnum(ITy) << "!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    } break;
    }
  } else if (RTy == ASTTypeMPDecimal && ITy == ASTTypeIdentifier) {
    const ASTIdentifierNode *IId = dynamic_cast<const ASTIdentifierNode *>(
        E->GetBinaryOp()->GetRight()->GetIdentifier());
    if (!IId) {
      NE = true;
      std::stringstream M;
      M << "Identifier does not have a corresponding known Symbol!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ASTSymbolTableEntry *ISTE = ASTSymbolTable::Instance().Lookup(
        IId, IId->GetBits(), IId->GetSymbolType());
    if (!ISTE) {
      NE = true;
      std::stringstream M;
      M << "Identifier " << IId->GetName() << " does not have a "
        << "SymbolTable Entry!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ITy = IId->GetSymbolType();

    switch (ISTE->GetValueType()) {
    case ASTTypeMPDecimal: {
      Evaluate(
          dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetLeft()),
          ISTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeMPInteger: {
      Evaluate(
          dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetLeft()),
          ISTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeFloat: {
      Evaluate(
          dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetLeft()),
          ISTE->GetValue()->GetValue<ASTFloatNode *>(),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeDouble: {
      Evaluate(
          dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetLeft()),
          ISTE->GetValue()->GetValue<ASTDoubleNode *>(),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeInt: {
      Evaluate(
          dynamic_cast<const ASTMPDecimalNode *>(E->GetBinaryOp()->GetLeft()),
          ISTE->GetValue()->GetValue<ASTIntNode *>(),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    default: {
      NE = true;
      std::stringstream M;
      M << "Impossible initialization of complex value from Types R="
        << PrintTypeEnum(RTy) << " I=" << PrintTypeEnum(ITy) << "!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    } break;
    }
  } else if (RTy == ASTTypeMPInteger && ITy == ASTTypeIdentifier) {
    const ASTIdentifierNode *IId = dynamic_cast<const ASTIdentifierNode *>(
        E->GetBinaryOp()->GetRight()->GetIdentifier());
    if (!IId) {
      NE = true;
      std::stringstream M;
      M << "Identifier does not have a corresponding known Symbol!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ASTSymbolTableEntry *ISTE = ASTSymbolTable::Instance().Lookup(
        IId, IId->GetBits(), IId->GetSymbolType());
    if (!ISTE) {
      NE = true;
      std::stringstream M;
      M << "Identifier " << IId->GetName() << " does not have a "
        << "SymbolTable Entry!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    IId->GetSymbolType();

    switch (ISTE->GetValueType()) {
    case ASTTypeMPDecimal: {
      Evaluate(
          dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetLeft()),
          ISTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeMPInteger: {
      Evaluate(
          dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetLeft()),
          ISTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeFloat: {
      Evaluate(
          dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetLeft()),
          ISTE->GetValue()->GetValue<ASTFloatNode *>(),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeDouble: {
      Evaluate(
          dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetLeft()),
          ISTE->GetValue()->GetValue<ASTDoubleNode *>(),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeInt: {
      Evaluate(
          dynamic_cast<const ASTMPIntegerNode *>(E->GetBinaryOp()->GetLeft()),
          ISTE->GetValue()->GetValue<ASTIntNode *>(),
          E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    default: {
      NE = true;
      std::stringstream M;
      M << "Impossible initialization of complex value from Types R="
        << PrintTypeEnum(RTy) << " I=" << PrintTypeEnum(ITy) << "!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    } break;
    }
  } else if (RTy == ASTTypeFloat && ITy == ASTTypeIdentifier) {
    const ASTIdentifierNode *IId = dynamic_cast<const ASTIdentifierNode *>(
        E->GetBinaryOp()->GetRight()->GetIdentifier());
    if (!IId) {
      NE = true;
      std::stringstream M;
      M << "Identifier does not have a corresponding known Symbol!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ASTSymbolTableEntry *ISTE = ASTSymbolTable::Instance().Lookup(
        IId, IId->GetBits(), IId->GetSymbolType());
    if (!ISTE) {
      NE = true;
      std::stringstream M;
      M << "Identifier " << IId->GetName() << " does not have a "
        << "SymbolTable Entry!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ITy = IId->GetSymbolType();

    switch (ISTE->GetValueType()) {
    case ASTTypeMPDecimal: {
      Evaluate(dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeMPInteger: {
      Evaluate(dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeFloat: {
      Evaluate(dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTFloatNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeDouble: {
      Evaluate(dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTDoubleNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeInt: {
      Evaluate(dynamic_cast<const ASTFloatNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTIntNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    default: {
      NE = true;
      std::stringstream M;
      M << "Impossible initialization of complex value from Types R="
        << PrintTypeEnum(RTy) << " I=" << PrintTypeEnum(ITy) << "!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    } break;
    }
  } else if (RTy == ASTTypeDouble && ITy == ASTTypeIdentifier) {
    const ASTIdentifierNode *IId = dynamic_cast<const ASTIdentifierNode *>(
        E->GetBinaryOp()->GetRight()->GetIdentifier());
    if (!IId) {
      NE = true;
      std::stringstream M;
      M << "Identifier does not have a corresponding known Symbol!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ASTSymbolTableEntry *ISTE = ASTSymbolTable::Instance().Lookup(
        IId, IId->GetBits(), IId->GetSymbolType());
    if (!ISTE) {
      NE = true;
      std::stringstream M;
      M << "Identifier " << IId->GetName() << " does not have a "
        << "SymbolTable Entry!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ITy = IId->GetSymbolType();

    switch (ISTE->GetValueType()) {
    case ASTTypeMPDecimal: {
      Evaluate(dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeMPInteger: {
      Evaluate(dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeFloat: {
      Evaluate(dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTFloatNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeDouble: {
      Evaluate(dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTDoubleNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeInt: {
      Evaluate(dynamic_cast<const ASTDoubleNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTIntNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    default: {
      std::stringstream M;
      NE = true;
      M << "Impossible initialization of complex value from Types R="
        << PrintTypeEnum(RTy) << " I=" << PrintTypeEnum(ITy) << "!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    } break;
    }
  } else if (RTy == ASTTypeInt && ITy == ASTTypeIdentifier) {
    const ASTIdentifierNode *IId = dynamic_cast<const ASTIdentifierNode *>(
        E->GetBinaryOp()->GetRight()->GetIdentifier());
    if (!IId) {
      NE = true;
      std::stringstream M;
      M << "Identifier does not have a corresponding known Symbol!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ASTSymbolTableEntry *ISTE = ASTSymbolTable::Instance().Lookup(
        IId, IId->GetBits(), IId->GetSymbolType());
    if (!ISTE) {
      NE = true;
      std::stringstream M;
      M << "Identifier " << IId->GetName() << " does not have a "
        << "SymbolTable Entry!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    }

    ITy = IId->GetSymbolType();

    switch (ISTE->GetValueType()) {
    case ASTTypeMPDecimal: {
      Evaluate(dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTMPDecimalNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeMPInteger: {
      Evaluate(dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTMPIntegerNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeFloat: {
      Evaluate(dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTFloatNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeDouble: {
      Evaluate(dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTDoubleNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    case ASTTypeInt: {
      Evaluate(dynamic_cast<const ASTIntNode *>(E->GetBinaryOp()->GetLeft()),
               ISTE->GetValue()->GetValue<ASTIntNode *>(),
               E->GetBinaryOp()->GetOpType(), NumBits);
      return;
    } break;
    default: {
      NE = true;
      std::stringstream M;
      M << "Impossible initialization of complex value from Types R="
        << PrintTypeEnum(RTy) << " I=" << PrintTypeEnum(ITy) << "!";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return;
    } break;
    }
  } else {
    NE = true;
    std::stringstream M;
    M << "Impossible initialization of complex value from Types R="
      << PrintTypeEnum(RTy) << " I=" << PrintTypeEnum(ITy) << "!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
    return;
  }
}

ASTMPComplexNode::ASTMPComplexNode(const ASTIdentifierNode *Id,
                                   const ASTMPDecimalNode *R,
                                   const ASTMPDecimalNode *I, ASTOpType OT,
                                   unsigned NumBits)
    : ASTExpressionNode(Id, ASTTypeMPComplex), Bits(NumBits),
      Precision(NumBits), DeclBits(NumBits), OpType(OT), MPValue(),
      Expr(nullptr), FC(nullptr), NE(true), RTy(R->GetASTType()),
      ITy(I->GetASTType()), RBits(R->GetBits()), IBits(I->GetBits()) {
  Id->SetBits(NumBits);
  mpc_init2(MPValue, NumBits);

  if (R->GetBits() != I->GetBits()) {
    std::stringstream M;
    M << "Inconsistent precision between Real and Imaginary components!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  if (NumBits < R->GetBits()) {
    std::stringstream M;
    M << "Constructing a complex number with a bit width smaller than "
      << "that of its real or imaginary components may result in "
      << "truncation and/or loss of precision.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  Evaluate(R, I, OT, NumBits);
}

ASTMPComplexNode::ASTMPComplexNode(const ASTIdentifierNode *Id,
                                   const ASTMPIntegerNode *R,
                                   const ASTMPIntegerNode *I, ASTOpType OT,
                                   unsigned NumBits)
    : ASTExpressionNode(Id, ASTTypeMPComplex), Bits(NumBits),
      Precision(NumBits), DeclBits(NumBits), OpType(OT), MPValue(),
      Expr(nullptr), FC(nullptr), NE(true), RTy(R->GetASTType()),
      ITy(I->GetASTType()), RBits(R->GetBits()), IBits(I->GetBits()) {
  Id->SetBits(NumBits);
  mpc_init2(MPValue, NumBits);

  if (R->GetBits() != I->GetBits()) {
    std::stringstream M;
    M << "Inconsistent precision between Real and Imaginary components!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  if (NumBits < R->GetBits()) {
    std::stringstream M;
    M << "Constructing a complex number with a bit width smaller than "
      << "that of its real or imaginary components may result in "
      << "truncation and/or loss of precision.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  Evaluate(R, I, OT, NumBits);
}

ASTMPComplexNode::ASTMPComplexNode(const ASTIdentifierNode *Id,
                                   const ASTMPDecimalNode *R,
                                   const ASTMPIntegerNode *I, ASTOpType OT,
                                   unsigned NumBits)
    : ASTExpressionNode(Id, ASTTypeMPComplex), Bits(NumBits),
      Precision(NumBits), DeclBits(NumBits), OpType(OT), MPValue(),
      Expr(nullptr), FC(nullptr), NE(true), RTy(R->GetASTType()),
      ITy(I->GetASTType()), RBits(R->GetBits()), IBits(I->GetBits()) {
  Id->SetBits(NumBits);
  mpc_init2(MPValue, NumBits);

  if (NumBits < R->GetBits()) {
    std::stringstream M;
    M << "Constructing a complex number with a bit width smaller than "
      << "that of its real or imaginary components might result in "
      << "truncation and/or loss of precision.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  Evaluate(R, I, OT, NumBits);
}

ASTMPComplexNode::ASTMPComplexNode(const ASTIdentifierNode *Id,
                                   const ASTMPIntegerNode *R,
                                   const ASTMPDecimalNode *I, ASTOpType OT,
                                   unsigned NumBits)
    : ASTExpressionNode(Id, ASTTypeMPComplex), Bits(NumBits),
      Precision(NumBits), DeclBits(NumBits), OpType(OT), Bias(Exact), MPValue(),
      Expr(nullptr), FC(nullptr), NE(true), RTy(R->GetASTType()),
      ITy(I->GetASTType()), RBits(R->GetBits()), IBits(I->GetBits()) {
  Id->SetBits(NumBits);
  mpc_init2(MPValue, NumBits);

  if (NumBits < R->GetBits()) {
    std::stringstream M;
    M << "Constructing a complex number with a bit width smaller than "
      << "that of its real or imaginary components may result in "
      << "truncation and/or loss of precision.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
  }

  Evaluate(R, I, OT, NumBits);
}

ASTMPComplexNode::ASTMPComplexNode(const ASTIdentifierNode *Id,
                                   const std::string &REP, unsigned NumBits)
    : ASTExpressionNode(Id, ASTTypeMPComplex), Bits(NumBits),
      Precision(NumBits), DeclBits(NumBits), OpType(ASTOpTypeNone), Bias(Exact),
      MPValue(), Expr(nullptr), FC(nullptr), NE(true), RTy(ASTTypeMPDecimal),
      ITy(ASTTypeImaginary), RBits(DefaultBits), IBits(DefaultBits) {
  Id->SetBits(NumBits);
  mpc_init2(MPValue, NumBits);

  if (mpc_strtoc(MPValue, REP.c_str(), nullptr, 0, MPC_RNDND) == -1) {
    std::stringstream M;
    M << "Unable to parse the string representation of complex number: "
      << REP.c_str() << ". The value will be set to NaN.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
    mpc_set_nan(MPValue);
  }
}

ASTMPComplexNode::ASTMPComplexNode(const ASTIdentifierNode *Id, const char *REP,
                                   unsigned NumBits)
    : ASTExpressionNode(Id, ASTTypeMPComplex), Bits(NumBits),
      Precision(NumBits), DeclBits(NumBits), OpType(ASTOpTypeNone), Bias(Exact),
      MPValue(), Expr(nullptr), FC(nullptr), NE(true), RTy(ASTTypeMPDecimal),
      ITy(ASTTypeImaginary), RBits(DefaultBits), IBits(DefaultBits) {
  Id->SetBits(NumBits);
  mpc_init2(MPValue, NumBits);

  if (mpc_strtoc(MPValue, REP, nullptr, 0, MPC_RNDND) == -1) {
    std::stringstream M;
    M << "Unable to parse the string representation of complex number: " << REP
      << ". The value will be set to NaN.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Warning);
    mpc_set_nan(MPValue);
  }
}

ASTMPComplexNode::ASTMPComplexNode(const ASTIdentifierNode *Id,
                                   const ASTFunctionCallNode *FN,
                                   unsigned NumBits)
    : ASTExpressionNode(Id, ASTTypeMPComplex), Bits(NumBits),
      Precision(NumBits), DeclBits(NumBits), OpType(ASTOpTypeNone), Bias(Exact),
      MPValue(), Expr(nullptr), FC(FN), NE(true), RTy(ASTTypeMPDecimal),
      ITy(ASTTypeImaginary), RBits(DefaultBits), IBits(DefaultBits) {
  Id->SetBits(NumBits);
  mpc_init2(MPValue, NumBits);
  mpc_set_nan(MPValue);
}

ASTMPComplexNode::ASTMPComplexNode(const ASTIdentifierNode *Id,
                                   const ASTMPComplexNode &MPCC,
                                   unsigned NumBits)
    : ASTExpressionNode(Id, ASTTypeMPComplex), Bits(NumBits),
      Precision(MPCC.Precision), DeclBits(MPCC.DeclBits), OpType(MPCC.OpType),
      Bias(MPCC.Bias), MPValue(), Expr(MPCC.Expr), FC(MPCC.FC), NE(MPCC.NE),
      RTy(MPCC.RTy), ITy(MPCC.ITy), RBits(MPCC.RBits), IBits(MPCC.IBits) {
  mpc_init2(MPValue, NumBits);
  SetBias(mpc_set(MPValue, MPCC.MPValue, MPC_RNDND));
}

void ASTMPComplexNode::CReal(const ASTIntNode *I) {
  assert(I && "Invalid ASTIntNode argument!");
  ASTMPDecimalNode *IMPD = GetImagAsMPDecimal();
  assert(IMPD && "Invalid Imaginary ASTMPDecimalNode!");
  Evaluate(I, IMPD, GetOpType(), GetBits());
}

void ASTMPComplexNode::CReal(const ASTFloatNode *F) {
  assert(F && "Invalid ASTFloatNode argument!");
  ASTMPDecimalNode *IMPD = GetImagAsMPDecimal();
  assert(IMPD && "Invalid Imaginary ASTMPDecimalNode!");
  Evaluate(F, IMPD, GetOpType(), GetBits());
}

void ASTMPComplexNode::CReal(const ASTDoubleNode *D) {
  assert(D && "Invalid ASTDoubleNode argument!");
  ASTMPDecimalNode *IMPD = GetImagAsMPDecimal();
  assert(IMPD && "Invalid Imaginary ASTMPDecimalNode!");
  Evaluate(D, IMPD, GetOpType(), GetBits());
}

void ASTMPComplexNode::CReal(const ASTMPIntegerNode *MPI) {
  assert(MPI && "Invalid ASTMPIntegerNode argument");
  ASTMPDecimalNode *IMPD = GetImagAsMPDecimal();
  assert(IMPD && "Invalid Imaginary ASTMPDecimalNode!");
  Evaluate(MPI, IMPD, GetOpType(), GetBits());
}

void ASTMPComplexNode::CReal(const ASTMPDecimalNode *MPD) {
  assert(MPD && "Invalid ASTMPDecimalNode argument!");
  ASTMPDecimalNode *IMPD = GetImagAsMPDecimal();
  assert(IMPD && "Invalid Imaginary ASTMPDecimalNode!");
  Evaluate(MPD, IMPD, GetOpType(), GetBits());
}

void ASTMPComplexNode::CImag(const ASTIntNode *I) {
  assert(I && "Invalid ASTIntNode argument!");
  ASTMPDecimalNode *RMPD = GetRealAsMPDecimal();
  assert(RMPD && "Invalid Real ASTMPDecimalNode!");
  Evaluate(RMPD, I, GetOpType(), GetBits());
}

void ASTMPComplexNode::CImag(const ASTFloatNode *F) {
  assert(F && "Invalid ASTFloatNode argument!");
  ASTMPDecimalNode *RMPD = GetRealAsMPDecimal();
  assert(RMPD && "Invalid Real ASTMPDecimalNode!");
  Evaluate(RMPD, F, GetOpType(), GetBits());
}

void ASTMPComplexNode::CImag(const ASTDoubleNode *D) {
  assert(D && "Invalid ASTDoubleNode argument!");
  ASTMPDecimalNode *RMPD = GetRealAsMPDecimal();
  assert(RMPD && "Invalid Real ASTMPDecimalNode!");
  Evaluate(RMPD, D, GetOpType(), GetBits());
}

void ASTMPComplexNode::CImag(const ASTMPIntegerNode *MPI) {
  assert(MPI && "Invalid ASTMPIntegerNode argument!");
  ASTMPDecimalNode *RMPD = GetRealAsMPDecimal();
  assert(RMPD && "Invalid Real ASTMPDecimalNode!");
  Evaluate(RMPD, MPI, GetOpType(), GetBits());
}

void ASTMPComplexNode::CImag(const ASTMPDecimalNode *MPD) {
  assert(MPD && "Invalid ASTMPDecimalNode argument!");
  ASTMPDecimalNode *RMPD = GetRealAsMPDecimal();
  assert(RMPD && "Invalid Real ASTMPDecimalNode!");
  Evaluate(RMPD, MPD, GetOpType(), GetBits());
}

void ASTMPComplexNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (GetBits() == GetRealBits())
    M.MangleMPComplex(GetRealType(), GetBits(), GetRealBits());
  else
    M.MangleMPComplex(GetRealType(), GetBits(), GetBits());
  M.Underscore();
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTMPComplexNode::MangleLiteral() {
  ASTMangler M;
  M.Start();
  if (GetBits() == GetRealBits())
    M.MangleMPComplex(GetRealType(), GetBits(), GetRealBits());
  else
    M.MangleMPComplex(GetRealType(), GetBits(), GetBits());
  M.Underscore();
  M.Identifier(GetName());
  M.NumericLiteral(this);
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledLiteralName(M.AsString());
}

void ASTMPComplexNode::print() const {
  std::cout << "<MPComplex>" << std::endl;
  std::cout << "<Name>" << GetName() << "</Name>" << std::endl;
  std::cout << "<MangledName>" << GetMangledName() << "</MangledName>"
            << std::endl;
  std::cout << "<Bits>" << std::dec << Bits << "</Bits>" << std::endl;

  std::cout << "<RealBits>" << std::dec << RBits << "</RealBits>" << std::endl;
  std::cout << "<ImaginaryBits>" << std::dec << IBits << "</ImaginaryBits>"
            << std::endl;

  std::cout << "<Precision>" << std::dec << Precision << "</Precision>"
            << std::endl;
  std::cout << "<Value>" << GetValue() << "</Value>" << std::endl;
  std::cout << "<Bias>" << GetBiasAsString() << "</Bias>" << std::endl;

  if (Expr) {
    std::cout << "<Expression>" << std::endl;
    Expr->print();
    std::cout << "</Expression>" << std::endl;
  }

  if (FC) {
    std::cout << "<FunctionCall>" << std::endl;
    FC->print();
    std::cout << "</FunctionCall>" << std::endl;
  }

  std::cout << "</MPComplex>" << std::endl;
}

} // namespace QASM
