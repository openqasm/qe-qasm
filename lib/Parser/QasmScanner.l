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
%x C_Comment

O   [0-7]
D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
H   [a-fA-F0-9]
HP  (0[xX])
E   ([Ee][+-]?{D}+)
P   ([Pp][+-]?{D}+)
FS  (f|F|l|L)
IS  (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
CP  (u|U|L)
SP  (u8|u|U|L)
ES  (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS  [ \t\v\f]+

ASC     [\x00-\x7f]
ASCN    [\x00-\t\v-\x7f]
UC      [\x80-\xbf]
UC2     [\xc2-\xdf]
UC3     [\xe0-\xef]
UC4     [\xf0-\xf4]

UCANY    {ASC}|{UC2}{UC}|{UC3}{UC}{UC}|{UC4}{UC}{UC}{UC}
UCANYN   {ASCN}|{UC2}{UC}|{UC3}{UC}{UC}|{UC4}{UC}{UC}{UC}
UCONLY   {UC2}{UC}|{UC3}{UC}{UC}|{UC4}{UC}{UC}{UC}

CHARCONSTANT ('(([\\]['])|([^']))+')
STRINGLITERAL ["](([\\]["])|([^"]))*["]
INCLUDE [include][ ]{STRINGLITERAL}
PREPROC [#][ ][0-9]+[ ]{STRINGLITERAL}[ 0-9]*

%{

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <locale>
#include <codecvt>
#include <cstdlib>
#include <cassert>

#include "QasmParser.tab.h"

#include <qasm/AST/AST.h>
#include <qasm/AST/ASTBraceMatcher.h>
#include <qasm/AST/ASTStatementBuilder.h>
#include <qasm/AST/ASTDeclarationBuilder.h>
#include <qasm/AST/ASTIfStatementTracker.h>
#include <qasm/AST/ASTStringUtils.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTIdentifierTypeController.h>
#include <qasm/AST/ASTDeclarationContext.h>
#include <qasm/AST/ASTDefcalBuilder.h>
#include <qasm/AST/ASTKernelContextBuilder.h>
#include <qasm/AST/ASTPragmaContextBuilder.h>
#include <qasm/AST/ASTAnnotationContextBuilder.h>
#include <qasm/AST/ASTGateContextBuilder.h>
#include <qasm/AST/ASTDefcalContextBuilder.h>
#include <qasm/AST/ASTCalContextBuilder.h>
#include <qasm/AST/ASTFunctionContextBuilder.h>
#include <qasm/Frontend/QasmScanner.h>
#include <qasm/Frontend/QasmFeatureTester.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Diagnostic/DIAGLineBuffer.h>

#undef  YY_DECL
#define YY_DECL int QASM::ASTScanner::yylex(QASM::Parser::semantic_type* const LVAL, QASM::Parser::location_type*, ASTScanner& /* QASMScanner */)

/* Shorten the token NNS. */
using token = QASM::Parser::token;

/* Define yyterminate as token::END instead of NULL */
#define yyterminate() return token::TOK_END

/* No <unistd.h> on Windows */
#if defined(_MSC_VER)
#define YY_NO_UNISTD_H
#endif

int yylineno = 1;
int prev_yycolno = 1;
int yycolno = 1;
unsigned newlinecount = 0;
char* yytext;
std::stringstream yystream;
bool skip_newline = false;

inline void reset_colno() {
  // Do not set these values to anything other
  // than 0 (zero). That would break the column
  // counter and caret position algorithm.
  yycolno = 0;
  prev_yycolno = 0;
}

#define YY_USER_ACTION {  \
  std::string_view YSVW(yytext);                         \
  Loc->columns(Loc->begin.column + YSVW.length());       \
  prev_yycolno = yycolno;                                \
  yycolno += YSVW.length();                              \
  DIAGLineCounter::Instance().SetColCount(static_cast<uint32_t>(yycolno)); \
  if (yycolno == 1) {     \
    DIAGLineBuffer::Instance().Clear(); \
  } else {                \
    DIAGLineBuffer::Instance().Append(yytext); \
    if (ASTPragmaContextBuilder::Instance().InOpenContext()) { \
      if (*yytext == u8'\\') { \
        skip_newline = true; \
        continue ; \
      } \
      ASTPragmaContextBuilder::Instance().AddDirective(yytext); \
      if (*yytext == u8'\n') { \
        if (skip_newline) { \
          skip_newline = false; \
          continue; \
        } else { \
          yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_STRING_LITERAL); \
          assert(yylval->Tok && "Could not create a valid ASTToken!"); \
          yylval->Token = yylval->Tok->GetInt(); \
          yylval->String = yylval->Tok->GetStringAddress(); \
          yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(), \
                                   prev_yycolno + 1); \
          ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex()); \
          return yylval->Tok->GetInt(); \
        } \
      } \
    } \
    if (ASTAnnotationContextBuilder::Instance().InOpenContext()) { \
      if (*yytext == u8'\\') { \
        skip_newline = true; \
        continue ; \
      } \
      ASTAnnotationContextBuilder::Instance().AddDirective(yytext); \
      if (*yytext == u8'\n') { \
        if (skip_newline) { \
          skip_newline = false; \
          continue; \
        } else { \
          yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_STRING_LITERAL); \
          assert(yylval->Tok && "Could not create a valid ASTToken!"); \
          yylval->Token = yylval->Tok->GetInt(); \
          yylval->String = yylval->Tok->GetStringAddress(); \
          yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(), \
                                   prev_yycolno + 1); \
          ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex()); \
          return yylval->Tok->GetInt(); \
        } \
      } \
    } \
  } \
  if (ASTIdentifierTypeController::Instance().PendingStop()) { \
    ASTIdentifierTypeController::Instance().ClearContextRecord(); \
  } \
  DIAGLineCounter::Instance().SetColCount(static_cast<uint32_t>(yycolno)); \
  if (ASTPragmaContextBuilder::Instance().InOpenContext()) { \
    continue; \
  } \
}

#define sym_type(ident) token::TOK_IDENTIFIER

extern void yyerror(const char* msg);
extern int readinput();

void token_error() {
  std::cerr << "Error: Unrecognized token at line "
  << QASM::DIAGLineCounter::Instance().GetLineCount()
  << ": " << yytext << std::endl;
}

void parse_error() {
  std::cerr << "Error: Parse error at line "
    << QASM::DIAGLineCounter::Instance().GetLineCount()
    << ": " << yytext << std::endl;
}

void comment_error(const char* msg) {
  std::cerr << "Error: Parse error at line "
    << QASM::DIAGLineCounter::Instance().GetLineCount();
  if (msg)
    std::cerr << ": " << msg << ": " << yytext << std::endl;
  else
    std::cerr << ": " << yytext << std::endl;
}

%}

%option debug
%option nodefault
%option noyywrap
%option nounput
%option batch
%option yylineno
%option yyclass="QASM::Scanner"
%option c++

%%

%{
  /* Code executed at the beginning of yylex */
  (void) memset(LVAL, 0, sizeof(*LVAL));
  yylval = LVAL;

  if (QASM::ASTScanner::start_openqasm) {
    int T = QASM::ASTScanner::start_openqasm;
    QASM::ASTScanner::start_openqasm = 0;
    return T;
  }
%}

"OPENQASM" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IBMQASM);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"creg" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_CREG);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeCReg);
  return yylval->Tok->GetInt();
}

"qreg" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_QREG);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeQReg);
  return yylval->Tok->GetInt();
}

"CNOT" {
  ASTGateContextBuilder::Instance().OpenContext();
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_CNOT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeCNotGate);
  return yylval->Tok->GetInt();
}

"h" {
  uint32_t TIX = ASTTokenFactory::GetCurrentIndex();
  const ASTToken* TK = ASTTokenFactory::GetToken(TIX - 1U);

  if (TK->GetInt() == token::TOK_GATE) {
    ASTGateContextBuilder::Instance().OpenContext();
    yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_HADAMARD);
    assert(yylval->Tok && "Could not create a valid ASTToken!");
    yylval->Token = yylval->Tok->GetInt();
    yylval->String = yylval->Tok->GetStringAddress();
    yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                             prev_yycolno + 1);
    ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
    QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeHadamardGate);
  } else {
    yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IDENTIFIER);
    assert(yylval->Tok && "Could not create a valid ASTToken!");
    yylval->Token = yylval->Tok->GetInt();
    yylval->String = yylval->Tok->GetStringAddress();
    yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                             prev_yycolno + 1);
    ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  }

  return yylval->Tok->GetInt();
}

"CCX" {
  ASTGateContextBuilder::Instance().OpenContext();
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_CCX);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeCCXGate);
  return yylval->Tok->GetInt();
}

"CX" {
  ASTGateContextBuilder::Instance().OpenContext();
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_CX);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().StartQubitList();
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeCXGate);
  return yylval->Tok->GetInt();
}

"qubit" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_QUBIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeQubit);
  return yylval->Tok->GetInt();
}

"U" {
  ASTGateContextBuilder::Instance().OpenContext();
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_U);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().Reset();
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUGate);
  QASM::ASTIdentifierTypeController::Instance().StartQubitList();
  return yylval->Tok->GetInt();
}

"gate" {
  ASTGateContextBuilder::Instance().OpenContext();
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_GATE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTGateContextBuilder::Instance().OpenContext();
  ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeGate);
  QASM::ASTIdentifierTypeController::Instance().Reset();
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeGate);
  QASM::ASTIdentifierTypeController::Instance().StartContextRecord(ASTTypeGate);
  QASM::ASTIdentifierTypeController::Instance().StartQubitList();
  ASTGateBraceMatcher::Instance().Reset();
  return yylval->Tok->GetInt();
}

"gphase" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_GPHASE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeGPhaseExpression);
  return yylval->Tok->GetInt();
}

"barrier" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_BARRIER);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeBarrier);
  return yylval->Tok->GetInt();
}

"box" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_BOX);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeBox);
  return yylval->Tok->GetInt();
}

"boxas" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_BOXAS);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeBoxAs);
  return yylval->Tok->GetInt();
}

"boxto" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_BOXTO);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeBoxTo);
  return yylval->Tok->GetInt();
}

"measure" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_MEASURE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeMeasure);
  return yylval->Tok->GetInt();
}

"kernel" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_KERNEL);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeKernel);
  return yylval->Tok->GetInt();
}

"extern" {
  QASM::ASTIdentifierTypeController::Instance().Reset();
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_EXTERN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());

  if (!ASTDeclarationContextTracker::Instance().InCalibrationContext() &&
      !ASTDeclarationContextTracker::Instance().InExternContext()) {
    ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeExtern);
  }

  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().Reset();
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeExtern);
  ASTExternBraceMatcher::Instance().Reset();
  return yylval->Tok->GetInt();
}

"dirty" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_DIRTY);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeDirty);
  return yylval->Tok->GetInt();
}

"frame" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FRAME);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeOpenPulseFrame);
  return yylval->Tok->GetInt();
}

[ ]+[.][ ]+phase {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_PHASE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

".phase" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_PHASE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

[ ]+[.][ ]+time {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_TIME);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

".time" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_TIME);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

[ ]+[.][ ]+frequency {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FREQUENCY);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

".frequency" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FREQUENCY);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

[ ]+[.][ ]+freq {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FREQUENCY);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

".freq" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FREQUENCY);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"opaque" {
  ASTGateContextBuilder::Instance().OpenContext();
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_OPAQUE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeGate);
  QASM::ASTIdentifierTypeController::Instance().Reset();
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeGate);
  QASM::ASTIdentifierTypeController::Instance().StartContextRecord(ASTTypeGate);
  QASM::ASTIdentifierTypeController::Instance().StartQubitList();
  return yylval->Tok->GetInt();
}

"euler" {
  yylval->Tok = ASTTokenFactory::Create("euler", token::TOK_EULER);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeEulerAngle);
  return token::TOK_IDENTIFIER;
}

"lambda" {
  yylval->Tok = ASTTokenFactory::Create("lambda", token::TOK_LAMBDA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  if (ASTAngleContextControl::Instance().InOpenContext())
    QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeLambdaAngle);
  return token::TOK_IDENTIFIER;
}

"phi" {
  yylval->Tok = ASTTokenFactory::Create("phi", token::TOK_PHI);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  if (ASTAngleContextControl::Instance().InOpenContext())
    QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypePhiAngle);
  return token::TOK_IDENTIFIER;
}

"pi" {
  yylval->Tok = ASTTokenFactory::Create("pi", token::TOK_PI);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  if (ASTAngleContextControl::Instance().InOpenContext())
    QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypePiAngle);
  return token::TOK_IDENTIFIER;
}

"tau" {
  yylval->Tok = ASTTokenFactory::Create("tau", token::TOK_TAU);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  if (ASTAngleContextControl::Instance().InOpenContext())
    QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeTauAngle);
  return token::TOK_IDENTIFIER;
}

"theta" {
  yylval->Tok = ASTTokenFactory::Create("theta", token::TOK_THETA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  if (ASTAngleContextControl::Instance().InOpenContext())
    QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeThetaAngle);
  return token::TOK_IDENTIFIER;
}

"α" {
  yylval->Tok = ASTTokenFactory::Create("alpha", token::TOK_ALPHA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"β" {
  yylval->Tok = ASTTokenFactory::Create("beta", token::TOK_BETA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"γ" {
  yylval->Tok = ASTTokenFactory::Create("gamma", token::TOK_GAMMA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"δ" {
  yylval->Tok = ASTTokenFactory::Create("delta", token::TOK_DELTA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"ε" {
  yylval->Tok = ASTTokenFactory::Create("euler", token::TOK_EULER);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"ζ" {
  yylval->Tok = ASTTokenFactory::Create("zeta", token::TOK_ZETA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"η" {
  yylval->Tok = ASTTokenFactory::Create("eta", token::TOK_ETA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"θ" {
  yylval->Tok = ASTTokenFactory::Create("theta", token::TOK_THETA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"ι" {
  yylval->Tok = ASTTokenFactory::Create("iota", token::TOK_IOTA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"κ" {
  yylval->Tok = ASTTokenFactory::Create("kappa", token::TOK_KAPPA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"λ" {
  yylval->Tok = ASTTokenFactory::Create("lambda", token::TOK_LAMBDA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"μ" {
  yylval->Tok = ASTTokenFactory::Create("mu", token::TOK_MU);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"ν" {
  yylval->Tok = ASTTokenFactory::Create("nu", token::TOK_NU);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"ξ" {
  yylval->Tok = ASTTokenFactory::Create("xi", token::TOK_XI);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"ο" {
  yylval->Tok = ASTTokenFactory::Create("omicron", token::TOK_OMICRON);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"π" {
  yylval->Tok = ASTTokenFactory::Create("pi", token::TOK_PI);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"ρ" {
  yylval->Tok = ASTTokenFactory::Create("rho", token::TOK_RHO);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"σ" {
  yylval->Tok = ASTTokenFactory::Create("sigma", token::TOK_SIGMA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"τ" {
  yylval->Tok = ASTTokenFactory::Create("tau", token::TOK_TAU);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"υ" {
  yylval->Tok = ASTTokenFactory::Create("upsilon", token::TOK_UPSILON);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"φ" {
  yylval->Tok = ASTTokenFactory::Create("phi", token::TOK_PHI);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"χ" {
  yylval->Tok = ASTTokenFactory::Create("chi", token::TOK_CHI);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"ψ" {
  yylval->Tok = ASTTokenFactory::Create("psi", token::TOK_PSI);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"ω" {
  yylval->Tok = ASTTokenFactory::Create("omega", token::TOK_OMEGA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Α" {
  yylval->Tok = ASTTokenFactory::Create("alpha", token::TOK_ALPHA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Β" {
  yylval->Tok = ASTTokenFactory::Create("beta", token::TOK_BETA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Γ" {
  yylval->Tok = ASTTokenFactory::Create("gamma", token::TOK_GAMMA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Δ" {
  yylval->Tok = ASTTokenFactory::Create("delta", token::TOK_DELTA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Ε" {
  yylval->Tok = ASTTokenFactory::Create("epsilon", token::TOK_EPSILON);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Ζ" {
  yylval->Tok = ASTTokenFactory::Create("zeta", token::TOK_ZETA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Η" {
  yylval->Tok = ASTTokenFactory::Create("eta", token::TOK_ETA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Θ" {
  yylval->Tok = ASTTokenFactory::Create("theta", token::TOK_THETA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Ι" {
  yylval->Tok = ASTTokenFactory::Create("iota", token::TOK_IOTA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Κ" {
  yylval->Tok = ASTTokenFactory::Create("kappa", token::TOK_KAPPA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Λ" {
  yylval->Tok = ASTTokenFactory::Create("lambda", token::TOK_LAMBDA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Μ" {
  yylval->Tok = ASTTokenFactory::Create("mu", token::TOK_MU);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Ν" {
  yylval->Tok = ASTTokenFactory::Create("nu", token::TOK_NU);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Ξ" {
  yylval->Tok = ASTTokenFactory::Create("xi", token::TOK_XI);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Ο" {
  yylval->Tok = ASTTokenFactory::Create("omicron", token::TOK_OMICRON);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Π" {
  yylval->Tok = ASTTokenFactory::Create("pi", token::TOK_PI);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Ρ" {
  yylval->Tok = ASTTokenFactory::Create("rho", token::TOK_RHO);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Σ" {
  yylval->Tok = ASTTokenFactory::Create("sigma", token::TOK_SIGMA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Τ" {
  yylval->Tok = ASTTokenFactory::Create("tau", token::TOK_TAU);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Υ" {
  yylval->Tok = ASTTokenFactory::Create("upsilon", token::TOK_UPSILON);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Φ" {
  yylval->Tok = ASTTokenFactory::Create("phi", token::TOK_PHI);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Χ" {
  yylval->Tok = ASTTokenFactory::Create("chi", token::TOK_CHI);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Ψ" {
  yylval->Tok = ASTTokenFactory::Create("psi", token::TOK_PSI);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"Ω" {
  yylval->Tok = ASTTokenFactory::Create("omega", token::TOK_OMEGA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return token::TOK_IDENTIFIER;
}

"sin" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_SIN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeSin);
  return yylval->Tok->GetInt();
}

"cos" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_COS);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeCos);
  return yylval->Tok->GetInt();
}

"tan" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_TAN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeTan);
  return yylval->Tok->GetInt();
}

"exp" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_EXP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeExp);
  return yylval->Tok->GetInt();
}

"ln" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeLn);
  return yylval->Tok->GetInt();
}

"pow" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_POW);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypePow);
  return yylval->Tok->GetInt();
}

"acos" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ARCCOS);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeArcCos);
  return yylval->Tok->GetInt();
}

"asin" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ARCSIN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeArcSin);
  return yylval->Tok->GetInt();
}

"atan" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ARCTAN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeArcTan);
  return yylval->Tok->GetInt();
}

"arccos" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ARCCOS);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeArcCos);
  return yylval->Tok->GetInt();
}

"arcsin" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ARCSIN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeArcSin);
  return yylval->Tok->GetInt();
}

"arctan" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ARCTAN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeArcTan);
  return yylval->Tok->GetInt();
}

"popcount" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_POPCOUNT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypePopcountExpr);
  return yylval->Tok->GetInt();
}

"rotl" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ROTL);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  return yylval->Tok->GetInt();
}

"rotr" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ROTR);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  return yylval->Tok->GetInt();
}

"inv" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INVERSE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeGateInverse);
  QASM::ASTIdentifierTypeController::Instance().StopQubitList();
  QASM::ASTIdentifierTypeController::Instance().StopAngleList();
  return yylval->Tok->GetInt();
}

"ctrl" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_CTRL);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeGateControl);
  QASM::ASTIdentifierTypeController::Instance().StopQubitList();
  QASM::ASTIdentifierTypeController::Instance().StopAngleList();
  return yylval->Tok->GetInt();
}

"negctrl" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_NEGCTRL);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeGateNegControl);
  QASM::ASTIdentifierTypeController::Instance().StopQubitList();
  QASM::ASTIdentifierTypeController::Instance().StopAngleList();
  return yylval->Tok->GetInt();
}

"sqrt" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_SQRT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeSqrt);
  return yylval->Tok->GetInt();
}

"@" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ASSOCIATION_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  return yylval->Tok->GetInt();
}

"[^\"]*"  {
  /* no quotes inside quotes */
}

[\n] {
  DIAGLineBuffer::Instance().Clear();
  DIAGLineCounter::Instance().SetColCount(Loc->end.column - Loc->begin.column);
  ++newlinecount;
  uint32_t LC = DIAGLineCounter::Instance().GetLineCount();
  DIAGLineCounter::Instance().SetLineCount(LC + 1);
  LC = DIAGLineCounter::Instance().GetLineCount();
  Loc->lines(LC + 1);
  reset_colno();
}

"[^\n$]" {
  DIAGLineBuffer::Instance().Clear();
  DIAGLineCounter::Instance().SetColCount(Loc->end.column - Loc->begin.column);
  ++newlinecount;
  uint32_t LC = DIAGLineCounter::Instance().GetLineCount();
  DIAGLineCounter::Instance().SetLineCount(LC + 1);
  Loc->lines(LC + 1);
  reset_colno();
}

[\r\n] {
  DIAGLineBuffer::Instance().Clear();
  DIAGLineCounter::Instance().SetColCount(Loc->end.column - Loc->begin.column);
  ++newlinecount;
  uint32_t LC = DIAGLineCounter::Instance().GetLineCount();
  DIAGLineCounter::Instance().SetLineCount(LC + 1);
  Loc->lines(LC + 1);
  reset_colno();
}

"[^\r\n$]" {
  DIAGLineBuffer::Instance().Clear();
  DIAGLineCounter::Instance().SetColCount(Loc->end.column - Loc->begin.column);
  ++newlinecount;
  uint32_t LC = DIAGLineCounter::Instance().GetLineCount();
  DIAGLineCounter::Instance().SetLineCount(LC + 1);
  Loc->lines(LC + 1);
  reset_colno();
}

^(?:[\t ]*(?:\r?\n|\r))+  {
  std::string_view S = yytext;
  unsigned K = ASTStringUtils::Instance().CountNewlines(S);

  if (K) {
    DIAGLineBuffer::Instance().Clear();
    newlinecount += K;
    uint32_t LC = DIAGLineCounter::Instance().GetLineCount();
    DIAGLineCounter::Instance().SetLineCount(LC + K);
    Loc->lines(LC + K);
    reset_colno();
  }
}

[\t\v\f]+ {
  /* ignore whitespace */
}

"//".* {
  /* C++ single-line */
  reset_colno();
}

"/*"((("*"[^/])?)|[^*])*"*/" {
  /* C single-line */
  std::string_view S = yytext;
  unsigned K = ASTStringUtils::Instance().CountNewlines(S);
  uint32_t LC = DIAGLineCounter::Instance().GetLineCount();

  if (K) {
    DIAGLineBuffer::Instance().Clear();
    newlinecount += K;
    DIAGLineCounter::Instance().SetLineCount(LC + K);
    Loc->lines(LC + K);
  }
}

"(/\*([^*]|[\n][\r\n]|(\*+([^*/]|[\n][\r\n])))*\*+/)|(//.*)" {
  /* C multi-line */
  std::string_view S = yytext;
  unsigned K = ASTStringUtils::Instance().CountNewlines(S);
  uint32_t LC = DIAGLineCounter::Instance().GetLineCount();

  if (K) {
    DIAGLineBuffer::Instance().Clear();
    newlinecount += K;
    LC = DIAGLineCounter::Instance().GetLineCount();
    DIAGLineCounter::Instance().SetLineCount(LC + K);
    Loc->lines(LC + K);
    reset_colno();
  }
}

"/*" {
  BEGIN(C_Comment);

  std::string_view S = yytext;
  unsigned K = ASTStringUtils::Instance().CountNewlines(S);
  uint32_t LC = DIAGLineCounter::Instance().GetLineCount();

  if (K) {
    DIAGLineBuffer::Instance().Clear();
    newlinecount += K;
    LC = DIAGLineCounter::Instance().GetLineCount();
    DIAGLineCounter::Instance().SetLineCount(LC + K);
    Loc->lines(LC + K);
    reset_colno();
  }
}

<C_Comment>[^*]* {
  std::string_view S = yytext;
  unsigned K = ASTStringUtils::Instance().CountNewlines(S);
  uint32_t LC = DIAGLineCounter::Instance().GetLineCount();

  if (K) {
    DIAGLineBuffer::Instance().Clear();
    newlinecount += K;
    LC = DIAGLineCounter::Instance().GetLineCount();
    DIAGLineCounter::Instance().SetLineCount(LC + K);
    Loc->lines(LC + K);
    reset_colno();
  }
}

<C_Comment>"*"+[^*/]* {
  std::string_view S = yytext;
  unsigned K = ASTStringUtils::Instance().CountNewlines(S);
  uint32_t LC = DIAGLineCounter::Instance().GetLineCount();

  if (K) {
    DIAGLineBuffer::Instance().Clear();
    newlinecount += K;
    LC = DIAGLineCounter::Instance().GetLineCount();
    DIAGLineCounter::Instance().SetLineCount(LC + K);
    Loc->lines(LC + K);
    reset_colno();
  }
}

<C_Comment>"*/" {
  BEGIN(INITIAL);

  std::string_view S = yytext;
  unsigned K = ASTStringUtils::Instance().CountNewlines(S);
  uint32_t LC = DIAGLineCounter::Instance().GetLineCount();

  if (K) {
    DIAGLineBuffer::Instance().Clear();
    newlinecount += K;
    LC = DIAGLineCounter::Instance().GetLineCount();
    DIAGLineCounter::Instance().SetLineCount(LC + K);
    Loc->lines(LC + K);
    reset_colno();
  }
}

"angle" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ANGLE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeAngle);
  QASM::ASTRedeclarationController::Instance().AllowRedeclarations(true);
  return yylval->Tok->GetInt();
}

"array" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ARRAY);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeArray);
  return yylval->Tok->GetInt();
}

"bit" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_BIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeBitset);
  QASM::ASTRedeclarationController::Instance().AllowRedeclarations(true);
  return yylval->Tok->GetInt();
}

"bool" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_BOOL);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeBool);
  QASM::ASTRedeclarationController::Instance().AllowRedeclarations(true);
  return yylval->Tok->GetInt();
}

"break" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_BREAK);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  yylval->String = QASM::ASTScanner::Create(yytext);

  ASTType CXTy =
    ASTDeclarationContextTracker::Instance().GetCurrentContext()->GetContextType();

  if (CXTy != ASTTypeGlobal) {
    switch (CXTy) {
    case ASTTypeCaseStatement:
    case ASTTypeDefaultStatement:
    case ASTTypeForLoop:
    case ASTTypeWhileLoop:
    case ASTTypeDoWhileLoop:
      ASTDeclarationContextTracker::Instance().PopCurrentContext();
      ASTIdentifierTypeController::Instance().Reset();
      break;
    default:
      break;
    }
  }

  return yylval->Tok->GetInt();
}

"cal" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_CAL);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  OpenPulse::ASTOpenPulseCalibrationBuilder::Instance().OpenContext();
  ASTDeclarationContextTracker::Instance().SetCalibrationContext();
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeOpenPulseCalibration);
  QASM::ASTCalContextBuilder::Instance().OpenContext();
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  return yylval->Tok->GetInt();
}

"case" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_CASE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().Reset();
  QASM::ASTCaseBraceMatcher::Instance().Reset();
  return yylval->Tok->GetInt();
}

"cbit" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_BIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeBitset);
  return yylval->Tok->GetInt();
}

"char" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_CHAR);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeChar);
  return yylval->Tok->GetInt();
}

"complex" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_COMPLEX);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeMPComplex);
  QASM::ASTRedeclarationController::Instance().AllowRedeclarations(false);
  return yylval->Tok->GetInt();
}

"creal" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_CREAL);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"cimag" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_CIMAG);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"const" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_CONST);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeConst);
  return yylval->Tok->GetInt();
}

"continue" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_CONTINUE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"def" {
  QASM::ASTFunctionContextBuilder::Instance().OpenContext();
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FUNCTION_DEFINITION);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeFunction);
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().Reset();
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeFunction);
  ASTFunctionBraceMatcher::Instance().Reset();
  return yylval->Tok->GetInt();
}

"default" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_DEFAULT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().Reset();
  QASM::ASTDefaultBraceMatcher::Instance().Reset();
  return yylval->Tok->GetInt();
}

"defcal" {
  QASM::ASTDefcalContextBuilder::Instance().OpenContext();
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_DEFCAL);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeDefcal);
  ASTDefcalContextBuilder::Instance().OpenContext();
  QASM::ASTIdentifierTypeController::Instance().Reset();
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeDefcal);
  QASM::ASTIdentifierTypeController::Instance().StartContextRecord(ASTTypeDefcal);
  QASM::ASTIdentifierTypeController::Instance().StartQubitList();
  ASTDefcalBuilder::Instance().OpenContext();
  OpenPulse::ASTOpenPulseCalibrationBuilder::Instance().OpenContext();
  ASTDefcalBraceMatcher::Instance().Reset();
  return yylval->Tok->GetInt();
}

"defcalgrammar" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_DEFCAL_GRAMMAR);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  return yylval->Tok->GetInt();
}

"delay" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_DELAY);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeDelay);
  return yylval->Tok->GetInt();
}

"do" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_DO);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeDoWhileStatement);
  QASM::ASTIdentifierTypeController::Instance().Reset();
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeDoWhileStatement);
  ASTDoWhileBraceMatcher::Instance().Reset();
  return yylval->Tok->GetInt();
}

"double" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_DOUBLE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeDouble);
  QASM::ASTRedeclarationController::Instance().AllowRedeclarations(true);
  return yylval->Tok->GetInt();
}

"duration" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_DURATION);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeDuration);
  return yylval->Tok->GetInt();
}

"durationof" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_DURATIONOF);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeDurationOf);
  return yylval->Tok->GetInt();
}

else[ \t\r\n]+if {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ELSEIF);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());

  unsigned K =
    ASTStringUtils::Instance().CountNewlines(yylval->Tok->GetString());

  if (K) {
    DIAGLineBuffer::Instance().Clear();
    Loc->lines();
    reset_colno();
    newlinecount += K;
    uint32_t LC = DIAGLineCounter::Instance().GetLineCount();
    DIAGLineCounter::Instance().SetLineCount(LC + K);
  }

  QASM::QasmFeatureTester::Instance().ValidateFeature("else if");
  ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeElseIfStatement);
  ASTElseIfBraceMatcher::Instance().Reset();
  ASTIdentifierTypeController::Instance().Reset();
  return yylval->Tok->GetInt();
}

"else" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ELSE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  yylval->String = QASM::ASTScanner::Create(yytext);
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeElseStatement);
  ASTElseBraceMatcher::Instance().Reset();
  ASTIdentifierTypeController::Instance().Reset();
  return yylval->Tok->GetInt();
}

"enum" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ENUM);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"false" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_BOOLEAN_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  yylval->String = QASM::ASTScanner::Create(yytext);
  return yylval->Tok->GetInt();
}

"#file" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FILE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  yylval->String = QASM::ASTScanner::Create(yytext);
  return yylval->Tok->GetInt();
}

"#[ ]+file" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FILE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  yylval->String = QASM::ASTScanner::Create(yytext);
  return yylval->Tok->GetInt();
}

"fixed" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FIXED);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  return yylval->Tok->GetInt();
}

"float" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FLOAT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeFloat);
  QASM::ASTRedeclarationController::Instance().AllowRedeclarations(true);
  return yylval->Tok->GetInt();
}

"for" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FOR);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeForStatement);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeForStatement);
  ASTForBraceMatcher::Instance().Reset();
  return yylval->Tok->GetInt();
}

"goto" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_GOTO);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"im" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IMAGINARY);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"if" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IF);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeIfStatement);
  ASTIfBraceMatcher::Instance().Reset();
  ASTIdentifierTypeController::Instance().Reset();
  return yylval->Tok->GetInt();
}

"implements" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IMPLEMENTS);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  return yylval->Tok->GetInt();
}

"in" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeForStatement);
  return yylval->Tok->GetInt();
}

"input" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INPUT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  return yylval->Tok->GetInt();
}

"include" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INCLUDE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"int" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeInt);
  QASM::ASTRedeclarationController::Instance().AllowRedeclarations(true);
  return yylval->Tok->GetInt();
}

"integer" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeInt);
  QASM::ASTRedeclarationController::Instance().AllowRedeclarations(true);
  return yylval->Tok->GetInt();
}

"length" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LENGTH);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeLength);
  return yylval->Tok->GetInt();
}

"lengthof" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LENGTHOF);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeLengthOf);
  return yylval->Tok->GetInt();
}

"let" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LET);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeLet);
  return yylval->Tok->GetInt();
}

"#line" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LINE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  yylval->String = QASM::ASTScanner::Create(yytext);
  return yylval->Tok->GetInt();
}

"#[ ]+line" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LINE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"long" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LONG);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeLong);
  QASM::ASTRedeclarationController::Instance().AllowRedeclarations(true);
  return yylval->Tok->GetInt();
}

"newframe" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_NEWFRAME);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeLong);
  return yylval->Tok->GetInt();
}

"output" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_OUTPUT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  return yylval->Tok->GetInt();
}

"play" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_PLAY);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeOpenPulsePlay);
  return yylval->Tok->GetInt();
}

"port" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_PORT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeOpenPulsePort);
  return yylval->Tok->GetInt();
}

"pragma" {
  ASTPragmaContextBuilder::Instance().OpenContext();
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_PRAGMA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypePragma);
  return yylval->Tok->GetInt();
}

@[a-zA-Z0-9]+ {
  ASTAnnotationContextBuilder::Instance().OpenContext();
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ANNOTATION);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeAnnotation);
  return yylval->Tok->GetInt();
}

"real" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_DOUBLE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeDouble);
  return yylval->Tok->GetInt();
}

"reset" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_RESET);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeReset);
  return yylval->Tok->GetInt();
}

"return" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_RETURN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeReturn);
  return yylval->Tok->GetInt();
}

"short" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_SHORT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeShort);
  return yylval->Tok->GetInt();
}

"signed" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_SIGNED);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeSigned);
  return yylval->Tok->GetInt();
}

"sizeof" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_SIZEOF);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  yylval->String = QASM::ASTScanner::Create(yytext);
  return yylval->Tok->GetInt();
}

"stretch" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_STRETCH);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeStretch);
  return yylval->Tok->GetInt();
}

"stretchinf" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_STRETCHINF);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  return yylval->Tok->GetInt();
}

"stretch"[0-9]+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_STRETCH_N);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature("stretch");
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeStretch);
  return yylval->Tok->GetInt();
}

"struct" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_STRUCT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  yylval->String = QASM::ASTScanner::Create(yytext);
  return yylval->Tok->GetInt();
}

"switch" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_SWITCH);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeSwitchStatement);
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().Reset();
  QASM::ASTSwitchBraceMatcher::Instance().Reset();
  return yylval->Tok->GetInt();
}

"true" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_BOOLEAN_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"uint" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_UINT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeInt);
  QASM::ASTRedeclarationController::Instance().AllowRedeclarations(true);
  return yylval->Tok->GetInt();
}

"union" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_UNION);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"unsigned" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_UNSIGNED);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeUnsigned);
  return yylval->Tok->GetInt();
}

"verbatim" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_VERBATIM);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  return yylval->Tok->GetInt();
}

"void" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_VOID);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"waveform" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_WAVEFORM);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);
  QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeOpenPulseWaveform);
  return yylval->Tok->GetInt();
}

"while" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_WHILE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::QasmFeatureTester::Instance().ValidateFeature(yytext);

  // A while loop may exist inside or outside a do-while loop.
  if (ASTDoWhileBraceMatcher::Instance().GetLeft() &&
      !ASTDoWhileBraceMatcher::Instance().GetRight()) {
    QASM::ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeWhileStatement);
    QASM::ASTIdentifierTypeController::Instance().Reset();
    QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeWhileStatement);
    ASTWhileBraceMatcher::Instance().Reset();
  } else if (!ASTDoWhileBraceMatcher::Instance().GetLeft() &&
             !ASTDoWhileBraceMatcher::Instance().GetRight()) {
    QASM::ASTDeclarationContextTracker::Instance().CreateContext(ASTTypeWhileStatement);
    QASM::ASTIdentifierTypeController::Instance().Reset();
    QASM::ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeWhileStatement);
  }

  return yylval->Tok->GetInt();
}

"__func__" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FUNC_NAME);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

%[0-9]+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_BOUND_QUBIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

%[a-zA-Z][0-9]+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_UNBOUND_QUBIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

[0-9]+ms {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_TIME_UNIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

[0-9]+ns {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_TIME_UNIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

[0-9]+us {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_TIME_UNIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

[0-9]+μs {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_TIME_UNIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

[0-9]+dt {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_TIME_UNIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

[0-9]+s {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_TIME_UNIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

$[0-9]+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_BOUND_QUBIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

$[a-zA_Z]+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_UNBOUND_QUBIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

$[a-zA-Z][0-9]+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_UNBOUND_QUBIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

%[a-zA-Z_]+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_UNBOUND_QUBIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

$[a-zA-Z_][0-9]+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_UNBOUND_QUBIT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

{L}{A}* {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IDENTIFIER);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

{UCONLY}+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IDENTIFIER);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());

  if (!ASTStringUtils::Instance().IsValidUTF8(*(yylval->String))) {
    std::stringstream M;
    M << "UTF8 encoding may be incorrect.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Warning);
  }

  return yylval->Tok->GetInt();
}

{UCONLY}+[a-zA-Z_]+[0-9]+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IDENTIFIER);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());

  if (!ASTStringUtils::Instance().IsValidUTF8(*(yylval->String))) {
    std::stringstream M;
    M << "UTF8 encoding may be incorrect.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Warning);
  }

  return yylval->Tok->GetInt();
}

[a-zA-Z_]+[0-9]+{UCONLY}+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IDENTIFIER);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());

  if (!ASTStringUtils::Instance().IsValidUTF8(*(yylval->String))) {
    std::stringstream M;
    M << "UTF8 encoding may be incorrect.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Warning);
  }

  return yylval->Tok->GetInt();
}

[a-zA-Z0-9_]+{UCONLY}+[a-zA-Z0-9_]+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IDENTIFIER);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());

  if (!ASTStringUtils::Instance().IsValidUTF8(*(yylval->String))) {
    std::stringstream M;
    M << "UTF8 encoding may be incorrect.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Warning);
  }

  return yylval->Tok->GetInt();
}

{A}+{UCONLY}+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IDENTIFIER);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());

  if (!ASTStringUtils::Instance().IsValidUTF8(*(yylval->String))) {
    std::stringstream M;
    M << "UTF8 encoding may be incorrect.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Warning);
  }

  return yylval->Tok->GetInt();
}

{UCONLY}+{A}+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IDENTIFIER);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());

  if (!ASTStringUtils::Instance().IsValidUTF8(*(yylval->String))) {
    std::stringstream M;
    M << "UTF8 encoding may be incorrect.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Warning);
  }

  return yylval->Tok->GetInt();
}

{UCONLY}+{A}+{UCONLY}+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IDENTIFIER);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());

  if (!ASTStringUtils::Instance().IsValidUTF8(*(yylval->String))) {
    std::stringstream M;
    M << "UTF8 encoding may be incorrect.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Warning);
  }

  return yylval->Tok->GetInt();
}

{HP}{H}+{IS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INTEGER_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

{D}+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INTEGER_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

{NZ}{D}*{IS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INTEGER_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"0"[bBoOxX]{O}*{IS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INTEGER_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"-0"[bBoOxX]{O}*{IS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INTEGER_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

{CP}?"'"([^'\\\n]|{ES})+"'"  {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INTEGER_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());

  unsigned K =
    ASTStringUtils::Instance().CountNewlines(yylval->Tok->GetString());

  if (K) {
    DIAGLineBuffer::Instance().Clear();
    Loc->lines();
    reset_colno();
    newlinecount += K;
    uint32_t LC = DIAGLineCounter::Instance().GetLineCount();
    DIAGLineCounter::Instance().SetLineCount(LC + K);
  }

  return yylval->Tok->GetInt();
}

-{HP}{H}+{IS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INTEGER_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

-{NZ}{D}*{IS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INTEGER_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

-"0"{O}*{IS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INTEGER_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

-{CP}?"'"([^'\\\n]|{ES})+"'" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INTEGER_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

{D}+{E}{FS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FP_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

{D}*"."{D}+{E}?{FS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FP_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

{D}+"."{E}?{FS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FP_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

{HP}{H}+{P}{FS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FP_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

{HP}{H}*"."{H}+{P}{FS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FP_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

{HP}{H}+"."{P}{FS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FP_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

-{D}+{E}{FS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FP_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

-{D}*"."{D}+{E}?{FS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FP_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

-{D}+"."{E}?{FS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FP_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

-{HP}{H}+{P}{FS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FP_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

-{HP}{H}*"."{H}+{P}{FS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FP_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

-{HP}{H}+"."{P}{FS}? {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_FP_CONSTANT);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_STRING_LITERAL);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"..." {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ELLIPSIS);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

">>=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_RIGHT_SHIFT_ASSIGN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"<<=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LEFT_SHIFT_ASSIGN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"+=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ADD_ASSIGN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"-=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_SUB_ASSIGN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"*=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_MUL_ASSIGN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"/=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_DIV_ASSIGN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"%=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_MOD_ASSIGN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"&=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_AND_ASSIGN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"^=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_XOR_ASSIGN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"|=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_OR_ASSIGN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

">>" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_RIGHT_SHIFT_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"<<" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LEFT_SHIFT_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"++" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_INC_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"--" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_DEC_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"->" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_RIGHT_ARROW);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"<-" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LEFT_ARROW);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"&&" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_AND_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"||" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_OR_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"<=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LE_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

">=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_GE_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"==" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_EQ_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"!=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_NE_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

";" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_SEMICOLON);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  ASTIdentifierTypeController::Instance().SetPendingStop();
  yylval->Semicolon = true;
  return ';';
}

("{"|"<%") {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LEFT_CURLY);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());

  const ASTDeclarationContext* DC =
    ASTDeclarationContextTracker::Instance().GetCurrentContext();

  switch (DC->GetContextType()) {
  case ASTTypeIfStatement:
    ASTIfBraceMatcher::Instance().CountLeft();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeElseIfStatement:
    ASTElseIfBraceMatcher::Instance().CountLeft();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeElseStatement:
    ASTElseBraceMatcher::Instance().CountLeft();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeWhileStatement:
    ASTWhileBraceMatcher::Instance().CountLeft();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeDoWhileStatement:
    ASTDoWhileBraceMatcher::Instance().CountLeft();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeForStatement:
    ASTForBraceMatcher::Instance().CountLeft();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeSwitchStatement:
    ASTSwitchBraceMatcher::Instance().CountLeft();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeCaseStatement:
    ASTCaseBraceMatcher::Instance().CountLeft();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeDefaultStatement:
    ASTDefaultBraceMatcher::Instance().CountLeft();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeGate:
    ASTGateBraceMatcher::Instance().CountLeft();
    break;
  case ASTTypeDefcal:
    ASTDefcalBraceMatcher::Instance().CountLeft();
    break;
  case ASTTypeFunction:
    ASTFunctionBraceMatcher::Instance().CountLeft();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeExtern:
  case ASTTypeKernel:
    ASTExternBraceMatcher::Instance().CountLeft();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  default:
    break;
  }

  ASTIdentifierTypeController::Instance().LBrace();
  ASTType CTy = ASTIdentifierTypeController::Instance().GetCurrentType();

  switch (CTy) {
  case ASTTypeGateQubitParam:
    break;
  default:
    ASTIdentifierTypeController::Instance().StopContextRecord();
    break;
  }

  return '{';
}

("}"|"%>") {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_RIGHT_CURLY);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());

  const ASTDeclarationContext* DC =
    ASTDeclarationContextTracker::Instance().GetCurrentContext();

  switch (DC->GetContextType()) {
  case ASTTypeIfStatement:
    ASTIfBraceMatcher::Instance().CountRight();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeElseIfStatement:
    ASTElseIfBraceMatcher::Instance().CountRight();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeElseStatement:
    ASTElseBraceMatcher::Instance().CountRight();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeWhileStatement:
    ASTWhileBraceMatcher::Instance().CountRight();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeDoWhileStatement:
    ASTDoWhileBraceMatcher::Instance().CountRight();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeForStatement:
    ASTForBraceMatcher::Instance().CountRight();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeSwitchStatement:
    ASTSwitchBraceMatcher::Instance().CountRight();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeCaseStatement:
    ASTCaseBraceMatcher::Instance().CountRight();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeDefaultStatement:
    ASTDefaultBraceMatcher::Instance().CountRight();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeGate:
    ASTGateBraceMatcher::Instance().CountRight();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeDefcal:
    ASTDefcalBraceMatcher::Instance().CountRight();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeFunction:
    ASTFunctionBraceMatcher::Instance().CountRight();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  case ASTTypeExtern:
  case ASTTypeKernel:
    ASTExternBraceMatcher::Instance().CountRight();
    ASTIdentifierTypeController::Instance().Reset();
    break;
  default:
    break;
  }

  ASTIdentifierTypeController::Instance().RBrace();
  ASTIdentifierTypeController::Instance().StopContextRecord();
  return '}';
}

"," {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_COMMA);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return ',';
}

":" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_SEMICOLON);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return ':';
}

"=" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_EQUAL_ASSIGN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return '=';
}

"(" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LEFT_PAREN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  ASTIdentifierTypeController::Instance().StartAngleList();
  ASTIdentifierTypeController::Instance().LParen();
  return '(';
}

")" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_RIGHT_PAREN);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  ASTIdentifierTypeController::Instance().RParen();
  return ')';
}

("["|"<:") {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LEFT_BRACKET);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().LBracket();
  return '[';
}

("]"|":>") {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_RIGHT_BRACKET);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  QASM::ASTIdentifierTypeController::Instance().RBracket();
  return ']';
}

"." {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_PERIOD);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return '.';
}

"&" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_AMPERSAND);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return '&';
}

"!" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_BANG);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"~" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_TILDE);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return '~';
}

"-" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_SUB_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return '-';
}

"+" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_ADD_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"*" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_MUL_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"/" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_DIV_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"%" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_MOD_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

%[0-9a-zA-Z_]+ {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_IDENTIFIER);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return yylval->Tok->GetInt();
}

"<" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_LT_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return '<';
}

">" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_GT_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return '>';
}

"^" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_XOR_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return '^';
}

"|" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_OR_OP);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return '|';
}

"?" {
  yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_QUESTION);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  return '?';
}

<<EOF>> {
  if (ASTPragmaContextBuilder::Instance().InOpenContext()) {
    yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_STRING_LITERAL);
    assert(yylval->Tok && "Could not create a valid ASTToken!");
    yylval->Token = yylval->Tok->GetInt();
    yylval->String = yylval->Tok->GetStringAddress();
    yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                             prev_yycolno + 1);
    ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
    return yylval->Tok->GetInt();
  } else if (ASTAnnotationContextBuilder::Instance().InOpenContext()) {
    yylval->Tok = ASTTokenFactory::Create(yytext, token::TOK_STRING_LITERAL);
    assert(yylval->Tok && "Could not create a valid ASTToken!");
    yylval->Token = yylval->Tok->GetInt();
    yylval->String = yylval->Tok->GetStringAddress();
    yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                             prev_yycolno + 1);
    ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
    ASTIdentifierTypeController::Instance().SetCurrentType(ASTTypeAnnotation);
    return yylval->Tok->GetInt();
  }

  yylval->Tok = ASTTokenFactory::Create(";", token::TOK_END);
  assert(yylval->Tok && "Could not create a valid ASTToken!");
  yylval->Token = yylval->Tok->GetInt();
  yylval->String = yylval->Tok->GetStringAddress();
  yylval->Tok->SetLocation(DIAGLineCounter::Instance().GetLineCount(),
                           prev_yycolno + 1);
  ASTTokenFactory::Register(yylval->Tok, yylval->Tok->GetIndex());
  yyterminate();
  return yylval->Tok->GetInt();
}

{WS}+ {
  /* Whitespace is token separator */
}

. {
  /* Match anything else */
}

%%

