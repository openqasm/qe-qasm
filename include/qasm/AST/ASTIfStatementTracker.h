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

#ifndef __QASM_AST_IF_STATEMENT_TRACKER_H
#define __QASM_AST_IF_STATEMENT_TRACKER_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTBraceMatcher.h>
#include <qasm/AST/ASTIfConditionals.h>
#include <qasm/AST/ASTIfStatementBuilder.h>

#include <deque>
#include <algorithm>
#include <cassert>

namespace QASM {

class ASTIfStatementList {
  friend class ASTIfStatementTracker;
  friend class ASTElseIfStatementTracker;
  friend class ASTElseStatementTracker;

private:
  std::deque<ASTIfStatementNode*> Stack;
  std::deque<ASTIfStatementNode*> PopStack;

public:
  using stack_type = std::deque<ASTIfStatementNode*>;
  using iterator = typename stack_type::iterator;
  using const_iterator = typename stack_type::const_iterator;

public:
  ASTIfStatementList() : Stack(), PopStack() { }

  ASTIfStatementList(const ASTIfStatementList& RHS)
  : Stack(RHS.Stack), PopStack(RHS.PopStack) { }

  virtual ~ASTIfStatementList() = default;

  ASTIfStatementList& operator=(const ASTIfStatementList& RHS) {
    if (this != &RHS) {
      Stack = RHS.Stack;
      PopStack = RHS.PopStack;
    }

    return *this;
  }

  std::deque<ASTIfStatementNode*>& GetStack() {
    return Stack;
  }

  const std::deque<ASTIfStatementNode*>& GetStack() const {
    return Stack;
  }

  std::deque<ASTIfStatementNode*>& GetPopStack() {
    return PopStack;
  }

  const std::deque<ASTIfStatementNode*>& GetPopStack() const {
    return PopStack;
  }

  virtual size_t Size() const {
    return Stack.size();
  }

  virtual void Clear() {
    Stack.clear();
    PopStack.clear();
  }

  virtual void Append(ASTIfStatementNode* ISN) {
    Stack.push_back(ISN);
    ISN->SetStackFrame(Stack.size() - 1);
  }

  virtual void Prepend(ASTIfStatementNode* ISN) {
    Stack.push_front(ISN);
    ISN->SetStackFrame(Stack.size() - 1);
  }

  virtual void Pop() {
    if (Stack.size()) {
      PopStack.push_back(Stack.back());
      if (Stack.size())
        Stack.pop_back();
    }
  }

  virtual void Erase(const ASTIfStatementNode* IFN) {
    std::deque<ASTIfStatementNode*>::iterator I =
      std::find(Stack.begin(), Stack.end(), IFN);
    if (I != Stack.end())
      Stack.erase(I);

    I = std::find(PopStack.begin(), PopStack.end(), IFN);
    if (I != PopStack.end())
      PopStack.erase(I);
  }

  iterator begin() { return Stack.begin(); }
  const_iterator begin() const { return Stack.begin(); }

  iterator end() { return Stack.end(); }
  const_iterator end() const { return Stack.end(); }

  ASTIfStatementNode* front() {
    return Stack.size() ? Stack.front() : nullptr;
  }

  const ASTIfStatementNode* front() const {
    return Stack.size() ? Stack.front() : nullptr;
  }

  ASTIfStatementNode* back() {
    return Stack.size() ? Stack.back() : nullptr;
  }

  const ASTIfStatementNode* back() const {
    return Stack.size() ? Stack.back() : nullptr;
  }

  virtual ASTType GetASTType() const {
    return ASTTypeIfStatementList;
  }

  inline ASTIfStatementNode* operator[](size_t Index) {
    assert(Index < Stack.size() && "Index is out-of-range!");
    return Stack[Index];
  }

  inline const ASTIfStatementNode* operator[](size_t Index) const {
    assert(Index < Stack.size() && "Index is out-of-range!");
    return Stack[Index];
  }

  virtual void print() const { }

  virtual void push(ASTBase* /* unused */) { }
};

class ASTIfStatementTracker : public ASTBase {
private:
  static ASTIfStatementList IL;
  static ASTIfStatementTracker TR;
  static ASTIfStatementNode* CIF;
  static bool PendingElseIf;
  static bool PendingElse;
  static bool Braces;

protected:
  ASTIfStatementTracker() { }

public:
  static ASTIfStatementTracker& Instance() {
    return ASTIfStatementTracker::TR;
  }

  static ASTIfStatementList* List() {
    return &ASTIfStatementTracker::IL;
  }

  size_t Size() const {
    return IL.Size();
  }

  void Clear() {
    IL.Clear();
    CIF = nullptr;
    PendingElseIf = false;
    PendingElse = false;
    Braces = false;
  }

  void Push(ASTIfStatementNode* ISN) {
    assert(ISN && "Invalid ASTIfStatementNode argument!");
    IL.Append(ISN);
    CIF = ISN;
  }

  void Pop() {
    IL.Pop();
    CIF = IL.Size() ? IL.back() : nullptr;
  }

  void PopCurrentIf() {
    switch (IL.Size()) {
    case 0:
      CIF = nullptr;
      return;
      break;
    case 1: {
      ASTIfStatementList::iterator LI = IL.begin();
      assert(*LI == CIF && "CurrentIf is not last!");

      IL.Erase(*LI);
      CIF = nullptr;
    }
      break;
    default:
      Pop();
      break;
    }
  }

  void Pop(const ASTIfStatementNode* IFN) {
    assert(IFN && "Invalid ASTIfStatementNode argument!");

    if (IFN && IFN != CIF) {
      CIF = const_cast<ASTIfStatementNode*>(IFN)->GetParentIf();
    } else if (IFN && IFN == CIF) {
      ASTIfStatementList::iterator LI = IL.begin();
      if (IL.Size() == 0 || LI == IL.end()) {
        CIF = nullptr;
        return;
      }

      if (IL.Size() == 1 && (*LI) == CIF) {
        CIF = nullptr;
        return;
      }

      while (LI != IL.end()) {
        if ((*LI) == IFN) {
          --LI;
          CIF = *LI;
          break;
        }

        ++LI;
      }
    }
  }

  void Erase(const ASTIfStatementNode* IFN) {
    IL.Erase(IFN);
    CIF = IL.Size() ? IL.back() : nullptr;
  }

  void CheckDeclarationContext() const ;

  void CheckDeclarationContext(const ASTStatementList* SL) const;

  void RemoveOutOfScope(ASTIfStatementNode* IFN);

  void SetPendingElseIf(bool V) {
    PendingElseIf = V;
  }

  void SetPendingElse(bool V) {
    PendingElse = V;
  }

  void SetHasBraces(bool V) {
    Braces = V;
  }

  bool HasPendingElseIf() const {
    return PendingElseIf;
  }

  bool HasPendingElse() const {
    return PendingElse;
  }

  bool HasBraces() const {
    return Braces;
  }

  ASTIfStatementNode* Back() {
    return IL.back();
  }

  const ASTIfStatementNode* Back() const {
    return IL.back();
  }

  ASTIfStatementNode* Front() {
    return IL.front();
  }

  const ASTIfStatementNode* Front() const {
    return IL.front();
  }

  unsigned GetCurrentStackFrame() const {
    return IL.Size() - 1;
  }

  void SetCurrentIf(ASTIfStatementNode* ISN) {
    CIF = ISN;
  }

  ASTIfStatementNode* GetCurrentIf() const {
    return CIF;
  }

  virtual ASTType GetASTType() const override {
    return ASTTypeIfStatementTracker;
  }

  virtual void print() const override { }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTElseIfStatementTracker : public ASTBase {
private:
  static ASTIfStatementList IL;
  static ASTElseIfStatementTracker EITR;
  static std::deque<unsigned> ISCQ;
  static ASTIfStatementNode* CIF;
  static const ASTElseIfStatementNode* CEI;
  static bool POP;
  static unsigned C;
  static bool PendingElseIf;
  static bool PendingElse;

protected:
  ASTElseIfStatementTracker() = default;

public:
  static ASTElseIfStatementTracker& Instance() {
    return ASTElseIfStatementTracker::EITR;
  }

  virtual ~ASTElseIfStatementTracker() = default;

  static ASTIfStatementList* List() {
    return &ASTElseIfStatementTracker::IL;
  }

  size_t Size() const {
    return IL.Size();
  }

  void Clear() {
    IL.Clear();
    PendingElseIf = false;
    PendingElse = false;
  }

  static void SetCurrentElseIf(const ASTElseIfStatementNode* EIF) {
    CEI = EIF;
  }

  void SetPendingElseIf(bool V) {
    PendingElseIf = V;
  }

  void SetPendingElse(bool V) {
    PendingElse = V;
  }

  bool HasPendingElseIf() const {
    return PendingElseIf;
  }

  bool HasPendingElse() const {
    return PendingElse;
  }

  static const ASTElseIfStatementNode* GetCurrentElseIf() {
    return CEI;
  }

  static void ClearCurrentElseIf() {
    CEI = nullptr;
  }

  void Push(ASTIfStatementNode* ISN) {
    assert(ISN && "Invalid ASTIfStatementNode argument!");
    if (ISN) {
      IL.Append(ISN);
      ASTElseIfStatementTracker::ISCQ.push_back(ISN->GetISC());
    }
  }

  void Pop(const ASTIfStatementNode* IFN) {
    assert(IFN && "Invalid ASTIfStatementNode argument!");

    if (IFN && IFN != CIF) {
      CIF = const_cast<ASTIfStatementNode*>(IFN)->GetParentIf();
    } else if (IFN && IFN == CIF) {
      ASTIfStatementList::iterator LI = IL.begin();
      if (IL.Size() == 0 || LI == IL.end()) {
        CIF = nullptr;
        return;
      }

      if (IL.Size() == 1 && (*LI) == CIF) {
        CIF = nullptr;
        return;
      }

      while (LI != IL.end()) {
        if ((*LI) == IFN) {
          --LI;
          CIF = *LI;
          break;
        }

        ++LI;
      }
    }
  }

  void Erase(const ASTIfStatementNode* IFN) {
    IL.Erase(IFN);
    CIF = IL.Size() ? IL.back() : nullptr;
  }

  void Pop() {
    IL.Pop();

    if (ISCQ.size())
      ISCQ.pop_back();
  }

  void CheckDeclarationContext() const;

  void CheckDeclarationContext(const ASTStatementList* SL) const;

  void RemoveOutOfScope(ASTElseIfStatementNode* EIN);

  ASTIfStatementNode* Back() {
    return IL.back();
  }

  const ASTIfStatementNode* Back() const {
    return IL.back();
  }

  ASTIfStatementNode* Front() {
    return IL.front();
  }

  const ASTIfStatementNode* Front() const {
    return IL.front();
  }

  unsigned GetCurrentStackFrame() const {
    return IL.Size() - 1;
  }

  virtual void SetCurrentIf(ASTIfStatementNode* ISN) {
    CIF = ISN;
  }

  virtual ASTIfStatementNode* GetCurrentIf() {
    return CIF;
  }

  virtual const ASTIfStatementNode* GetCurrentIf() const {
    return CIF;
  }

  virtual unsigned GetCurrentISC() const {
    return ISCQ.size() ? ISCQ.back() : static_cast<unsigned>(~0x0);
  }

  virtual ASTType GetASTType() const override {
    return ASTTypeIfStatementTracker;
  }

  virtual void print() const override { }

  virtual void push(ASTBase* /* unused */) override { }
};

class ASTElseStatementTracker : public ASTBase {
private:
  static ASTIfStatementList IL;
  static ASTElseStatementTracker ETR;
  static ASTIfStatementNode* CIF;
  static std::deque<unsigned> ISCQ;
  static std::map<unsigned, ASTStatementList*> ESM;

protected:
  ASTElseStatementTracker() = default;

public:
  static ASTElseStatementTracker& Instance() {
    return ASTElseStatementTracker::ETR;
  }

  static ASTIfStatementList* List() {
    return &ASTElseStatementTracker::IL;
  }

  virtual ~ASTElseStatementTracker() = default;

  size_t Size() const {
    return IL.Size();
  }

  void Clear() {
    IL.Clear();
  }

  void Push(ASTIfStatementNode* ISN) {
    assert(ISN && "Invalid ASTIfStatementNode argument!");
    if (ISN) {
      IL.Append(ISN);
      ASTElseStatementTracker::ISCQ.push_back(ISN->GetISC());
    }
  }

  void Pop() {
    IL.Pop();

    if (ISCQ.size())
      ISCQ.pop_back();
  }

  void Pop(const ASTIfStatementNode* IFN) {
    assert(IFN && "Invalid ASTIfStatementNode argument!");

    if (IFN && IFN != CIF) {
      CIF = const_cast<ASTIfStatementNode*>(IFN)->GetParentIf();
    } else if (IFN && IFN == CIF) {
      ASTIfStatementList::iterator LI = IL.begin();
      if (IL.Size() == 0 || LI == IL.end()) {
        CIF = nullptr;
        return;
      }

      if (IL.Size() == 1 && (*LI) == CIF) {
        CIF = nullptr;
        return;
      }

      while (LI != IL.end()) {
        if ((*LI) == IFN) {
          --LI;
          CIF = *LI;
          break;
        }

        ++LI;
      }
    }
  }

  void Erase(const ASTIfStatementNode* IFN) {
    IL.Erase(IFN);
    CIF = IL.Size() ? IL.back() : nullptr;
  }

  void CheckDeclarationContext() const;

  void CheckDeclarationContext(const ASTStatementList* SL) const;

  void RemoveOutOfScope(ASTElseStatementNode* ESN);

  ASTIfStatementNode* Back() {
    return IL.back();
  }

  const ASTIfStatementNode* Back() const {
    return IL.back();
  }

  ASTIfStatementNode* Front() {
    return IL.front();
  }

  const ASTIfStatementNode* Front() const {
    return IL.front();
  }

  unsigned GetCurrentStackFrame() const {
    return IL.Size() - 1;
  }

  virtual void SetCurrentIf(ASTIfStatementNode* ISN) {
    CIF = ISN;
  }

  virtual ASTIfStatementNode* GetCurrentIf() {
    return CIF;
  }

  virtual const ASTIfStatementNode* GetCurrentIf() const {
    return CIF;
  }

  virtual unsigned GetCurrentISC() const {
    return ISCQ.size() ? ISCQ.back() : static_cast<unsigned>(~0x0);
  }

  virtual ASTType GetASTType() const override {
    return ASTTypeIfStatementTracker;
  }

  virtual void print() const override { }

  virtual void push(ASTBase* /* unused */) override { }
};

} // namespace QASM

#endif // __QASM_AST_IF_STATEMENT_TRACKER_H

