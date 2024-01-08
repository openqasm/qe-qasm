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

#ifndef __QASM_AST_SCANNER_H
#define __QASM_AST_SCANNER_H

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <qasm/Frontend/QasmDriver.h>

#include "location.hh"

namespace QASM {

class ASTScanner : public yyFlexLexer {
private:
  QASM::Parser::semantic_type *yylval = nullptr;
  QASM::Parser::location_type *Loc = nullptr;
  std::istream *IS;

private:
  static std::vector<std::string *> SV;

public:
  static int start_openqasm;
  static int start_openpulse;

public:
  ASTScanner() : yyFlexLexer(), yylval(nullptr), Loc(nullptr), IS(nullptr) {}

  ASTScanner(std::istream *In)
      : yyFlexLexer(In), yylval(new QASM::Parser::semantic_type()),
        Loc(new QASM::Parser::location_type()), IS(In) {}

  virtual ~ASTScanner() = default;

  std::istream *GetStream() const { return IS; }

  using yyFlexLexer::yylex;

  virtual int yylex(QASM::Parser::semantic_type *const LVal,
                    QASM::Parser::location_type *Loc, ASTScanner &SC);

  static std::string *Create(const char *Text) {
    if (Text) {
      std::string *S = new std::string(Text);
      SV.push_back(S);
      return S;
    }

    return nullptr;
  }

  static void Release() {
    for (std::vector<std::string *>::iterator I = SV.begin(); I != SV.end();
         ++I) {
      delete *I;
      *I = nullptr;
    }
  }
};

} // namespace QASM

#endif // __QASM_AST_SCANNER_H
