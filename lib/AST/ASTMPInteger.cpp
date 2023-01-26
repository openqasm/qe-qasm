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

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTImplicitConversionExpr.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <sstream>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

static void (*gmp_free_mem_func)(void*, size_t);

#ifdef __cplusplus
} // extern "C"
#endif

namespace QASM {

using DiagLevel = QasmDiagnosticEmitter::DiagLevel;

void ASTMPIntegerNode::InitFromString(const char* NS, ASTSignbit SB,
                                      unsigned NumBits, int Base) {
  assert(NS && "Invalid numeric string argument!");
  std::string S = ASTStringUtils::Instance().Sanitize(std::string(NS));
  bool E = false;

  mpz_init2(MPValue, NumBits);

  if (SB == ASTSignbit::Unsigned) {
    std::string SUS;
    mpz_set_ui(MPValue, 0UL);

    if (S[0] == u8'0' && (S[1] == u8'b' || S[1] == u8'B')) {
      SUS = S.substr(2);
      if (mpz_set_str(MPValue, SUS.c_str(), 2) != 0) {
        mpz_set_ui(MPValue, 0UL);
        E = true;
      }
    } else if (S[0] == u8'0' && (S[1] == u8'o' || S[1] == u8'O')) {
      SUS = u8"0" + S.substr(2);
      if (mpz_set_str(MPValue, SUS.c_str(), 8) != 0) {
        mpz_set_ui(MPValue, 0UL);
        E = true;
      }
    } else if (S[0] == u8'0' && (S[1] == u8'x' || S[1] == u8'X')) {
      if (mpz_set_str(MPValue, S.c_str(), 0) != 0) {
        mpz_set_ui(MPValue, 0UL);
        E = true;
      }
    } else {
      if (mpz_set_str(MPValue, S.c_str(), Base) != 0) {
        mpz_set_ui(MPValue, 0UL);
        E = true;
      }
    }
  } else {
    bool N = NS[0] == u8'-';
    mpz_set_si(MPValue, 0L);
    std::string SUS;

    if (S[0] == u8'-' || S[0] == u8'+') {
      if (S[1] == u8'0' && (S[2] == u8'b' || S[2] == u8'B')) {
        SUS = N ? u8"-" + S.substr(3) : S.substr(3);
        if (mpz_set_str(MPValue, SUS.c_str(), 2) != 0) {
          mpz_set_si(MPValue, 0UL);
          E = true;
        }
      } else if (S[1] == u8'0' && (S[2] == u8'o' || S[2] == u8'O')) {
        SUS = N ? u8"-0" + S.substr(3) : u8"0" + S.substr(3);
        if (mpz_set_str(MPValue, SUS.c_str(), 8) != 0) {
          mpz_set_si(MPValue, 0UL);
          E = true;
        }
      } else if (S[1] == u8'0' && (S[2] == u8'x' || S[2] == u8'X')) {
        if (mpz_set_str(MPValue, S.c_str(), 0) != 0) {
          mpz_set_si(MPValue, 0UL);
          E = true;
        }
      } else {
        if (mpz_set_str(MPValue, S.c_str(), Base) != 0) {
          mpz_set_si(MPValue, 0UL);
          E = true;
        }
      }
    } else {
      if (S[0] == u8'0' && (S[1] == u8'b' || S[1] == u8'B')) {
        SUS = S.substr(2);
        if (mpz_set_str(MPValue, SUS.c_str(), 2) != 0) {
          mpz_set_si(MPValue, 0UL);
          E = true;
        }
      } else if (S[0] == u8'0' && (S[1] == u8'o' || S[1] == u8'O')) {
        SUS = u8"0" + S.substr(2);
        if (mpz_set_str(MPValue, SUS.c_str(), 8) != 0) {
          mpz_set_si(MPValue, 0UL);
          E = true;
        }
      } else if (S[0] == u8'0' && (S[1] == u8'x' || S[1] == u8'X')) {
        if (mpz_set_str(MPValue, S.c_str(), 0) != 0) {
          mpz_set_si(MPValue, 0UL);
          E = true;
        }
      } else {
        if (mpz_set_str(MPValue, S.c_str(), Base) != 0) {
          mpz_set_si(MPValue, 0UL);
          E = true;
        }
      }
    }
  }

  if (E) {
    Bits = 0;
    std::stringstream M;
    M << "Invalid string representation of multiple precision numeric value.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(this), M.str(),
                                                     DiagLevel::Warning);
  }
}

int ASTMPIntegerNode::InitMPZFromString(mpz_t& MPV, const char* NS,
                                        ASTSignbit SB) {
  assert(NS && "Invalid numeric string argument!");
  std::string S = ASTStringUtils::Instance().Sanitize(std::string(NS));
  bool E = false;

  if (SB == ASTSignbit::Unsigned) {
    std::string SUS;
    mpz_set_ui(MPV, 0UL);

    if (S[0] == u8'0' && (S[1] == u8'b' || S[1] == u8'B')) {
      SUS = S.substr(2);
      if (mpz_set_str(MPV, SUS.c_str(), 2) != 0) {
        mpz_set_ui(MPV, 0UL);
        E = true;
      }
    } else if (S[0] == u8'0' && (S[1] == u8'o' || S[1] == u8'O')) {
      SUS = u8"0" + S.substr(2);
      if (mpz_set_str(MPV, SUS.c_str(), 0) != 0) {
        mpz_set_ui(MPV, 0UL);
        E = true;
      }
    } else if (S[0] == u8'0' && (S[1] == u8'x' || S[1] == u8'X')) {
      if (mpz_set_str(MPV, S.c_str(), 0) != 0) {
        mpz_set_ui(MPV, 0UL);
        E = true;
      }
    } else {
      if (mpz_set_str(MPV, S.c_str(), 10) != 0) {
        mpz_set_ui(MPV, 0UL);
        E = true;
      }
    }
  } else {
    bool N = NS[0] == u8'-';
    mpz_set_si(MPV, 0L);
    std::string SUS;

    if (S[0] == u8'-' || S[0] == u8'+') {
      if (S[1] == u8'0' && (S[2] == u8'b' || S[2] == u8'B')) {
        SUS = N ? u8"-" + S.substr(3) : S.substr(3);
        if (mpz_set_str(MPV, SUS.c_str(), 2) != 0) {
          mpz_set_si(MPV, 0L);
          E = true;
        }
      } else if (S[1] == u8'0' && (S[2] == u8'o' || S[2] == u8'O')) {
        SUS = N ? u8"-0" + S.substr(3) : u8"0" + S.substr(3);
        if (mpz_set_str(MPV, SUS.c_str(), 0) != 0) {
          mpz_set_si(MPV, 0L);
          E = true;
        }
      } else if (S[1] == u8'0' && (S[2] == u8'x' || S[2] == u8'X')) {
        if (mpz_set_str(MPV, S.c_str(), 0) != 0) {
          mpz_set_si(MPV, 0L);
          E = true;
        }
      } else {
        if (mpz_set_str(MPV, S.c_str(), 10) != 0) {
          mpz_set_si(MPV, 0L);
          E = true;
        }
      }
    } else {
      if (S[0] == u8'0' && (S[1] == u8'b' || S[1] == u8'B')) {
        SUS = S.substr(2);
        if (mpz_set_str(MPV, SUS.c_str(), 2) != 0) {
          mpz_set_si(MPV, 0L);
          E = true;
        }
      } else if (S[0] == u8'0' && (S[1] == u8'o' || S[1] == u8'O')) {
        SUS = u8"0" + S.substr(2);
        if (mpz_set_str(MPV, SUS.c_str(), 0) != 0) {
          mpz_set_si(MPV, 0L);
          E = true;
        }
      } else if (S[0] == u8'0' && (S[1] == u8'x' || S[1] == u8'X')) {
        if (mpz_set_str(MPV, S.c_str(), 0) != 0) {
          mpz_set_si(MPV, 0L);
          E = true;
        }
      } else {
        if (mpz_set_str(MPV, S.c_str(), 10) != 0) {
          mpz_set_si(MPV, 0L);
          E = true;
        }
      }
    }
  }

  if (E) {
    std::stringstream M;
    M << "Invalid string representation of multiple precision numeric value.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Warning);
    return -1;
  }

  return 0;
}

std::string
ASTMPIntegerNode::GetValue(int Base) const {
  if ((Base > 0 && (Base < 2 || Base > 62)) ||
      (Base < 0 && (Base > -2 || Base < -36))) {
    std::stringstream M;
    M << "Base representation exceeds allowed limits!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Error);
    return "0";
  }

  char* S = mpz_get_str(NULL, Base, MPValue);
  std::string R;
  if (S) {
    R = S;
    mp_get_memory_functions(NULL, NULL, &gmp_free_mem_func);
    gmp_free_mem_func(S, std::strlen(S) + 1);
  }

  return R;
}

std::string
ASTMPIntegerNode::GetValue(const mpz_t& MPZ, int Base) {
  if ((Base > 0 && (Base < 2 || Base > 62)) ||
      (Base < 0 && (Base > -2 || Base < -36))) {
    std::stringstream M;
    M << "Base representation exceeds allowed limits!";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Error);
    return "0";
  }

  char* S = mpz_get_str(NULL, Base, MPZ);
  std::string R;

  if (S) {
    R = S;
    mp_get_memory_functions(NULL, NULL, &gmp_free_mem_func);
    gmp_free_mem_func(S, std::strlen(S) + 1);
  }

  return R;
}

bool ASTMPIntegerNode::IsImplicitConversion() const {
  return Expr && (Expr->GetASTType() == ASTTypeImplicitConversion);
}

void ASTMPIntegerNode::SetImplicitConversion(const ASTImplicitConversionNode* ICX) {
  Expr = ICX;
}

ASTMPDecimalNode*
ASTMPIntegerNode::AsMPDecimal() const {
  mpfr_t MPV;
  mpfr_init2(MPV, ASTMPDecimalNode::DefaultBits);
  mpfr_set_z(MPV, MPValue, MPFR_RNDN);

  ASTMPDecimalNode* MPD =
    new ASTMPDecimalNode(ASTIdentifierNode::MPDec.Clone(),
                         ASTMPDecimalNode::DefaultBits, MPV);
  return MPD;
}

void ASTMPIntegerNode::Mangle() {
  ASTMangler M;
  M.Start();
  if (IsSigned())
    M.TypeIdentifier(GetASTType(), GetBits(), GetName());
  else
    M.TypeIdentifier(ASTTypeMPUInteger, GetBits(), GetName());

  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTMPIntegerNode::MangleLiteral() {
  ASTMangler M;
  M.Start();
  M.NumericLiteral(this);
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledLiteralName(M.AsString());
}

} // namespace QASM


