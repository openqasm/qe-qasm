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

#include <qasm/AST/ASTTypes.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTAngleNodeBuilder.h>
#include <qasm/AST/ASTAnyTypeBuilder.h>
#include <qasm/AST/ASTArgumentNodeBuilder.h>
#include <qasm/AST/ASTCtrlAssocBuilder.h>
#include <qasm/AST/ASTDeclarationBuilder.h>
#include <qasm/AST/ASTDefcalBuilder.h>
#include <qasm/AST/ASTDefcalParameterBuilder.h>
#include <qasm/AST/ASTDefcalStatementBuilder.h>
#include <qasm/AST/ASTExpressionBuilder.h>
#include <qasm/AST/ASTForRangeInitBuilder.h>
#include <qasm/AST/ASTForStatementBuilder.h>
#include <qasm/AST/ASTFunctionParameterBuilder.h>
#include <qasm/AST/ASTFunctionStatementBuilder.h>
#include <qasm/AST/ASTGateNodeBuilder.h>
#include <qasm/AST/ASTGateOpBuilder.h>
#include <qasm/AST/ASTIdentifierBuilder.h>
#include <qasm/AST/ASTIfStatementTracker.h>
#include <qasm/AST/ASTIntegerListBuilder.h>
#include <qasm/AST/ASTIntegerSequenceBuilder.h>
#include <qasm/AST/ASTInverseAssocBuilder.h>
#include <qasm/AST/ASTKernelStatementBuilder.h>
#include <qasm/AST/ASTParameterBuilder.h>
#include <qasm/AST/ASTQubitNodeBuilder.h>
#include <qasm/AST/ASTStatementBuilder.h>
#include <qasm/AST/ASTWhileStatementBuilder.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTObjectTracker.h>
#include <qasm/AST/ASTHeapSizeController.h>

#include <qasm/Frontend/QasmScanner.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace QASM {

ASTObjectTracker ASTObjectTracker::IOM;

#if defined(__linux__) || defined(__APPLE__)
inline void
ASTObjectTracker::ParseSegment(const std::string& S, ASTSegmentMap& SM) {
  std::string::size_type Dash = S.find_first_of('-');
  std::string::size_type Space = S.find_first_of(' ');

  std::string Start = "0x" + S.substr(0, Dash);
  std::string End = "0x" + S.substr(Dash + 1, Space - Dash);

  SM.Start = std::stoul(Start, 0, 16);
  SM.End = std::stoul(End, 0, 16);
}

void
ASTObjectTracker::InitMemoryMap() {
  if (EnableFree) {
    bool HeapStartNeedsAdjustment = false;

    if (SbrkZero == 0UL) {
      SbrkZero =
        static_cast<uint64_t>(reinterpret_cast<uintptr_t>(sbrk(0)));
      HeapStartNeedsAdjustment = true;
    }

#if defined(__linux__)
    const char* MapFile = "/proc/self/maps";
    std::ifstream InFile;
    std::string Line;

    InFile.open(MapFile, std::fstream::in);
    if (!InFile.good() || InFile.eof()) {
      std::stringstream M;
      M << "Could not open proc mapfile " << MapFile;
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Error);
      return;
    }

    while (std::getline(InFile, Line)) {
      if (Line.find("[heap]") != std::string::npos)
        ParseSegment(Line, Heap);
      else if (Line.find("[stack]") != std::string::npos)
        ParseSegment(Line, Stack);
      else
        continue;
    }

    InFile.close();
#endif

    if (HeapStartNeedsAdjustment) {
      Heap.Start = SbrkZero;
      Heap.End = SbrkZero + ASTHeapSizeController::Instance().GetMaxHeapSize();
    }

    if (RLimitHeap == 0UL) {
      struct rlimit rl = { 0UL, 0UL };
      if (getrlimit(RLIMIT_DATA, &rl) != 0) {
        RLimitHeap = Heap.End;
        std::stringstream M;
        M << "Could not obtain RLIMIT_DATA. This is only a warning.";
        QasmDiagnosticEmitter::Instance().EmitDiagnostic(
          DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                     DiagLevel::Warning);
      } else {
        if (rl.rlim_max == static_cast<uint64_t>(~0x0UL))
          RLimitHeap = Heap.End;
        else
          RLimitHeap = std::min(rl.rlim_max, Heap.End);
      }
    }

    if (Heap.End > RLimitHeap) {
      std::stringstream M;
      M << "Maximum heap size exceeds maximum rlimit heap size. "
        << "Please consider adjusting the process maximum heap size limit "
        << "with the `ulimit' command.";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                   DiagLevel::Warning);
    }
  }
}

void ASTObjectTracker::Release() {
  if (EnableFree) {
    InitMemoryMap();
    for (std::map<std::size_t, ASTMapObject>::reverse_iterator I = OM.rbegin();
         I != OM.rend(); ++I) {
      const ASTBase* OB = (*I).second.O;

      if (OB && !(*I).second.D && IsOnHeap(OB) && OB->IsRegistered()) {
        if (const ASTDefcalNode* DCN = dynamic_cast<const ASTDefcalNode*>(OB)) {
          (void) DCN; // silence compiler warning
          (*I).second.O = nullptr;
          (*I).second.D = true;
        } else if (const ASTStatementNode* SN =
                   dynamic_cast<const ASTStatementNode*>(OB)) {
          if (SN && !SN->IsDirective()) {
            delete (*I).second.O;
            (*I).second.O = nullptr;
            (*I).second.D = true;
          } else if (SN && SN->IsDeclaration()) {
            if (const ASTDeclarationNode* DN =
                dynamic_cast<const ASTDeclarationNode*>(SN)) {
              (void) DN; // silence compiler warning
              delete (*I).second.O;
              (*I).second.O = nullptr;
              (*I).second.D = true;
            }
          }
        } else if (const ASTExpressionNode* EN =
                   dynamic_cast<const ASTExpressionNode*>(OB)) {
          (void) EN; // silence compiler warning
          delete (*I).second.O;
          (*I).second.O = nullptr;
          (*I).second.D = true;
        } else if (const ASTParameter* PN = dynamic_cast<const ASTParameter*>(OB)) {
          (void) PN; // silence compiler warning
          delete (*I).second.O;
          (*I).second.O = nullptr;
          (*I).second.D = true;
        }
      }
    }

    ASTAngleNodeBuilder::Instance().Clear();
    ASTAngleNodeBuilder::Instance().Map()->Clear();
    ASTAnyTypeBuilder::Instance().Clear();
    ASTArgumentNodeBuilder::Instance().Clear();
    ASTBinaryOpAssignBuilder::Instance().Clear();
    ASTBoxStatementBuilder::Instance().Clear();
    ASTCtrlAssocListBuilder::Instance().Clear();
    ASTDeclarationBuilder::Instance().Clear();
    ASTDefcalBuilder::Instance().Clear();
    ASTDefcalParameterBuilder::Instance().Clear();
    ASTDefcalStatementBuilder::Instance().Clear();
    ASTExpressionBuilder::Instance().Clear();
    ASTForRangeInitListBuilder::Instance().Clear();
    ASTForStatementBuilder::Instance().Clear();
    ASTFunctionParameterBuilder::Instance().Clear();
    ASTFunctionStatementBuilder::Instance().Clear();
    ASTGateNodeBuilder::Instance().Clear();
    ASTGateOpBuilder::Instance().Clear();
    ASTGateQubitParamBuilder::Instance().Clear();
    ASTIdentifierBuilder::Instance().Clear();
    ASTIfStatementTracker::Instance().Clear();
    ASTIntegerListBuilder::Instance().Clear();
    ASTIntegerSequenceBuilder::Instance().Clear();
    ASTInverseAssocListBuilder::Instance().Clear();
    ASTKernelStatementBuilder::Instance().Clear();
    ASTNamedTypeDeclarationBuilder::Instance().Clear();
    ASTParameterBuilder::Instance().Clear();
    ASTQubitNodeBuilder::Instance().Clear();
    ASTWhileStatementBuilder::Instance().Clear();
    ASTStatementBuilder::Instance().Clear();
    ASTSymbolTable::Instance().Release();
    ASTScanner::Release();
    ASTObjectTracker::Instance().Clear();
    ASTTokenFactory::Clear();
  }
}

#else

void ASTObjectTracker::ParseSegment(const std::string& S, ASTSegmentMap& SM)
{ }

void ASTObjectTracker::InitMemoryMap() { }

void ASTObjectTracker::Release() { }

#endif // defined(__linux__) || defined(__APPLE__)

} // namespace QASM

