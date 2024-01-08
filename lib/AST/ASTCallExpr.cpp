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

#include <qasm/AST/ASTCallExpr.h>
#include <qasm/AST/ASTDefcal.h>
#include <qasm/AST/ASTFunctions.h>
#include <qasm/AST/ASTGates.h>
#include <qasm/AST/ASTIdentifierTypeController.h>
#include <qasm/AST/ASTKernel.h>
#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <cassert>
#include <iostream>

namespace QASM {

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

ASTCallExpressionNode::ASTCallExpressionNode(const ASTIdentifierNode *Id,
                                             const ASTIdentifierNode *Callee,
                                             const ASTParameterList &Params,
                                             const ASTIdentifierList &Args)
    : ASTExpressionNode(Id, ASTTypeFunctionCall), CId(Callee), PL(Params),
      IL(Args), CallType(Callee->GetSymbolType()), PST(), AST() {
  ASTIdentifierTypeController::Instance().CheckIsCallable(Callee);
}

void ASTCallExpressionNode::ResolveSymbolTable() {
  if (!PL.Empty()) {
    PST.clear();
    for (ASTParameterList::const_iterator I = PL.begin(); I != PL.end(); ++I) {
      const ASTParameter *P = dynamic_cast<const ASTParameter *>(*I);
      assert(P && "Could not dynamic_cast to an ASTParameter!");

      const ASTIdentifierNode *PId = P->GetIdentifier();
      assert(PId && "Invalid ASTIdentifierNode obtained from ASTParameter!");

      ASTSymbolTableEntry *STE =
          const_cast<ASTSymbolTableEntry *>(PId->GetSymbolTableEntry());
      assert(STE &&
             "Could not obtain a valid SymbolTable Entry for ASTParameter!");

      PST.insert(PST.end(), STE);
    }
  }

  if (!IL.Empty()) {
    AST.clear();
    for (ASTIdentifierList::const_iterator I = IL.begin(); I != IL.end(); ++I) {
      assert(*I && "Invalid ASTIdentifierNode!");

      ASTSymbolTableEntry *STE =
          const_cast<ASTSymbolTableEntry *>((*I)->GetSymbolTableEntry());
      assert(STE && "Could not find a valid SymbolTable Entry for "
                    "Argument Identifier!");

      AST.insert(AST.end(), STE);
    }
  }
}

void ASTCallExpressionNode::Mangle() {
  ASTMangler M;
  M.Start();

  switch (CallType) {
  case ASTTypeGate:
    M.TypeIdentifier(ASTTypeGateCall, GetIdentifier()->GetName());
    break;
  case ASTTypeDefcal:
  case ASTTypeDefcalCallExpression:
    M.TypeIdentifier(ASTTypeDefcalCall, GetIdentifier()->GetName());
    break;
  case ASTTypeFunction:
  case ASTTypeFunctionCallExpression:
    M.TypeIdentifier(ASTTypeFunctionCall, GetIdentifier()->GetName());
    break;
  case ASTTypeKernel:
  case ASTTypeKernelCallExpression:
    M.TypeIdentifier(ASTTypeKernelCall, GetIdentifier()->GetName());
    break;
  default: {
    std::stringstream MM;
    MM << "Object of type " << PrintTypeEnum(CallType) << " is not callable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(CId), MM.str(),
        DiagLevel::Error);
    return;
  } break;
  }

  M.Underscore();
  M.TypeIdentifier(CId->GetASTType(), CId->GetName());

  switch (CallType) {
  case ASTTypeFunction: {
    const ASTSymbolTableEntry *STE = CId->GetSymbolTableEntry();
    assert(STE && "Invalid SymbolTable Entry for function call!");

    ASTFunctionDefinitionNode *FDN =
        STE->GetValue()->GetValue<ASTFunctionDefinitionNode *>();
    assert(FDN && "Invalid function definition obtained from SymbolTable!");

    const ASTResultNode *RN = FDN->GetResult();
    assert(RN && "Invalid function result obtained from function definition!");

    M.FuncReturn(RN->GetResultType());
    M.Underscore();
  } break;
  case ASTTypeKernel: {
    const ASTSymbolTableEntry *STE = CId->GetSymbolTableEntry();
    assert(STE && "Invalid SymbolTable Entry for kernel call!");

    ASTKernelNode *KN = STE->GetValue()->GetValue<ASTKernelNode *>();
    assert(KN && "Invalid ASTKernelNode obtained from SymbolTable!");

    const ASTResultNode *RN = KN->GetResult();
    assert(RN && "Invalid kernel result obtained from kernel node!");

    M.FuncReturn(RN->GetResultType());
    M.Underscore();
  } break;
  case ASTTypeDefcal: {
    const ASTSymbolTableEntry *STE = CId->GetSymbolTableEntry();
    assert(STE && "Invalid SymbolTable Entry for defcal call!");

    ASTDefcalNode *DCN = STE->GetValue()->GetValue<ASTDefcalNode *>();
    assert(DCN && "Invalid defcal obtained from Symbol Table!");

    if (DCN->IsMeasure()) {
      const ASTMeasureNode *MN = DCN->GetMeasure();
      assert(MN && "Invalid ASTMeasureNode obtained from defcal!");

      if (MN->HasResult()) {
        const ASTCBitNode *CBN = MN->GetResult();
        assert(CBN &&
               "Invalid ASTCBitNode result obtained from ASTMeasureNode!");

        M.FuncReturn(CBN->GetASTType());
        M.Underscore();
      } else if (MN->HasAngleResult()) {
        const ASTAngleNode *AN = MN->GetAngleResult();
        assert(AN &&
               "Invalid ASTAngleNode result obtained from ASTMeasureNode!");

        M.FuncReturn(AN->GetASTType());
        M.Underscore();
      }
    }
  } break;
  default:
    break;
  }

  unsigned PX = 0U;

  for (unsigned I = 0; I < PST.size(); ++I) {
    PX = I;
    switch (CallType) {
    case ASTTypeGate:
      M.GateArg(I, PST[I]->GetValueType(), PST[I]->GetIdentifier()->GetBits(),
                PST[I]->GetIdentifier()->GetName());
      break;
    case ASTTypeDefcal:
      M.DefcalArg(I, PST[I]->GetValueType(), PST[I]->GetIdentifier()->GetBits(),
                  PST[I]->GetIdentifier()->GetName());
      break;
    case ASTTypeFunction:
      M.FuncArg(I, PST[I]->GetValueType(), PST[I]->GetIdentifier()->GetBits(),
                PST[I]->GetIdentifier()->GetName());
      break;
    case ASTTypeKernel:
      M.KernelArg(I, PST[I]->GetValueType(), PST[I]->GetIdentifier()->GetBits(),
                  PST[I]->GetIdentifier()->GetName());
      break;
    default:
      break;
    }
  }

  for (unsigned I = 0; I < AST.size(); ++I) {
    switch (CallType) {
    case ASTTypeGate:
      M.GateArg(PX + I, AST[I]->GetValueType(),
                AST[I]->GetIdentifier()->GetBits(),
                AST[I]->GetIdentifier()->GetName());
      break;
    case ASTTypeDefcal:
      M.DefcalArg(PX + I, AST[I]->GetValueType(),
                  AST[I]->GetIdentifier()->GetBits(),
                  AST[I]->GetIdentifier()->GetName());
      break;
    case ASTTypeFunction:
      M.FuncArg(PX + I, AST[I]->GetValueType(),
                AST[I]->GetIdentifier()->GetBits(),
                AST[I]->GetIdentifier()->GetName());
      break;
    case ASTTypeKernel:
      M.KernelArg(PX + I, AST[I]->GetValueType(),
                  AST[I]->GetIdentifier()->GetBits(),
                  AST[I]->GetIdentifier()->GetName());
      break;
    default:
      break;
    }
  }

  M.CallEnd();
  GetIdentifier()->SetMangledName(M.AsString());
}

} // namespace QASM
