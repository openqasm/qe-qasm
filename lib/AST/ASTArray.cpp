/* -*- coding: utf-8 -*-
 *
 * Copyright 2023 IBM RESEARCH. All Rights Reserved.
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

#include <qasm/AST/ASTArray.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

std::any &ASTArrayNode::Memory() { return MM; }

const std::any &ASTArrayNode::Memory() const { return MM; }

void ASTInvalidArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size(), GetElementSize());
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTCBitArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size(), GetElementSize());
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTCBitNArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size(), GetElementSize());
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTQubitArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size(), GetElementSize());
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTQubitNArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size(), GetElementSize());
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTAngleArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size(), GetElementSize());
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTBoolArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size());
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTIntArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size());
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTMPIntegerArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size(), GetElementSize());
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTFloatArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size());
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTMPDecimalArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size(), GetElementSize());
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTMPComplexArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.ComplexArray(Size(), GetElementSize(), GetComplexElementType(),
                 GetComplexElementBits());
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTDurationArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size());
  M.StringValue(GetLengthUnit());
  M.Underscore();
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTOpenPulseFrameArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size());
  M.Underscore();
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTOpenPulsePortArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size());
  M.Underscore();
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

void ASTOpenPulseWaveformArrayNode::Mangle() {
  ASTMangler M;
  M.Start();
  M.Array(GetElementType(), Size());
  M.Underscore();
  M.Identifier(GetName());
  M.EndExpression();
  M.End();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace QASM
