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

#ifndef __QASM_AST_OBJECT_TRACKER_H
#define __QASM_AST_OBJECT_TRACKER_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTTypes.h>

#include <map>
#include <string>
#include <memory>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>

#if defined(__APPLE__)
#include <malloc/malloc.h>
#endif

#if defined(__APPLE__)
extern "C" {
  extern volatile uint64_t DSS __asm("section$start$__DATA$__data");
  extern volatile uint64_t DSE __asm("section$end$__DATA$__data");
}
#elif defined(__linux__)
extern "C" {
  extern char etext, edata, end;
}
#endif

namespace QASM {

struct ASTSegmentMap {
  uint64_t Start;
  uint64_t End;
};

struct ASTMapObject {
  const ASTBase* O;
  bool D;

  ASTMapObject() : O(nullptr), D(false) { }

  ASTMapObject(ASTBase* B) : O(B), D(false) { }

  ASTMapObject(const ASTBase* B) : O(B), D(false) { }

  ASTMapObject(const ASTMapObject& RHS)
  : O(RHS.O), D(RHS.D) { }

  ASTMapObject& operator=(const ASTMapObject& RHS) {
    if (this != &RHS) {
      O = RHS.O;
      D = RHS.D;
    }

    return *this;
  }
};

class ASTObjectTracker {
  friend class ASTIdentifierNode;
  friend class ASTDemangledRegistry;

private:
  std::map<std::uintptr_t, ASTMapObject> OM;
  ASTSegmentMap Stack;
  ASTSegmentMap Heap;
  uint64_t RLimitHeap;
  uint64_t SbrkZero;
  bool EnableFree;

private:
  static ASTObjectTracker IOM;

private:
  ASTObjectTracker()
  : OM(), Stack(), Heap(), RLimitHeap(0UL), SbrkZero(0UL),
  EnableFree(false) { }

  bool IsOnHeap(const ASTBase* O) {
#if defined(__APPLE__)
    return malloc_zone_from_ptr(reinterpret_cast<const void*>(O));
#elif defined(__linux__)
    volatile uint64_t XA = static_cast<uint64_t>(
                           reinterpret_cast<uintptr_t>(O));
    return XA >= Heap.Start && XA < RLimitHeap;
#else
#error "Not implemented on this OS."
#endif
  }

  bool IsOnHeap(const void* O) {
#if defined(__APPLE__)
    return malloc_zone_from_ptr(O);
#elif defined(__linux__)
    volatile uint64_t XA = static_cast<uint64_t>(
                           reinterpret_cast<uintptr_t>(O));
    return XA >= Heap.Start && XA < RLimitHeap;
#else
#error "Not implemented on this OS."
#endif
  }

  bool IsStatic(const void* O) {
#if defined(__APPLE__)
    extern volatile uint64_t LDSS __asm("section$start$__DATA$__data");
    extern volatile uint64_t LDSE __asm("section$end$__DATA$__data");
    DSS = LDSS;
    DSE = LDSE;

    volatile uint64_t XA = static_cast<uint64_t>(
                           reinterpret_cast<uintptr_t>(O));
    return (XA >= DSS && XA < DSE) ||
           !(XA >= Heap.Start && XA < RLimitHeap);
#elif defined(__linux__)
    volatile uint64_t XA = static_cast<uint64_t>(
      reinterpret_cast<uintptr_t>(O));
    return (XA >= reinterpret_cast<uint64_t>(&edata) &&
            XA < reinterpret_cast<uint64_t>(&end)) ||
      !(XA >= Heap.Start && XA < RLimitHeap);
#else
#error "Not implemented on this OS."
#endif
  }

  void ParseSegment(const std::string& S, ASTSegmentMap& SM);

  void InitMemoryMap();

  void ClearMemoryMap() {
    Stack.Start = Stack.End = Heap.Start = Heap.End = 0UL;
    RLimitHeap = 0UL;
  }

public:
  static ASTObjectTracker& Instance() {
    return IOM;
  }

  ~ASTObjectTracker() = default;

  void Enable() {
    EnableFree = true;
    InitMemoryMap();
  }

  void Disable() {
    EnableFree = false;
    ClearMemoryMap();
  }

  bool IsEnabled() const {
    return EnableFree;
  }

  void Clear() {
    OM.clear();
  }

  void Register(ASTBase* O) {
    if (EnableFree && O && IsOnHeap(O)) {
      uintptr_t H = reinterpret_cast<uintptr_t>(O);
      ASTMapObject MO(O);
      OM.insert(std::make_pair(H, MO));
    }
  }

  void Register(const ASTBase* O) {
    if (EnableFree && O && IsOnHeap(O)) {
      uintptr_t H = reinterpret_cast<uintptr_t>(O);
      ASTMapObject MO(O);
      OM.insert(std::make_pair(H, MO));
    }
  }

  void Unregister(ASTBase* O) {
    if (EnableFree && O && IsOnHeap(O)) {
      uintptr_t H = reinterpret_cast<uintptr_t>(O);
      OM.erase(H);
    }
  }

  bool IsRegistered(const ASTBase* O) const {
    uintptr_t H = reinterpret_cast<uintptr_t>(O);
    return OM.find(H) != OM.end();
  }

  void Release();

  void print() {
    std::cout << "<ASTObjectTracker>" << std::endl;
    std::cout << "<Stack>" << std::endl;
    std::cout << "<Start>0x" << std::hex << std::setw(16) << std::setfill('0')
      << Stack.Start << "</Start>" << std::endl;
    std::cout << "<End>0x" << std::hex << std::setw(16) << std::setfill('0')
      << Stack.End << "</End>" << std::endl;
    std::cout << "</Stack>" << std::endl;
    std::cout << "<Heap>" << std::endl;
    std::cout << "<Start>0x" << std::hex << std::setw(16) << std::setfill('0')
      << Heap.Start << "</Start>" << std::endl;
    std::cout << "<End>0x" << std::hex << std::setw(16) << std::setfill('0')
      << Heap.End << "</End>" << std::endl;
    std::cout << "</Heap>" << std::endl;
    std::cout << "<RLimitHeap>0x" << std::hex << std::setw(16)
      << std::setfill('0') << RLimitHeap << "</RLimitHeap>" << std::endl;
    std::cout << "</ASTObjectTracker>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_OBJECT_TRACKER_H

