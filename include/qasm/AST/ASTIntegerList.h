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

#ifndef __QASM_AST_INTEGER_LIST_H
#define __QASM_AST_INTEGER_LIST_H

#include <vector>

namespace QASM {

class ASTIntegerList {
  friend class ASTQubitContainerNode;
  friend class ASTQubitContainerAliasNode;
  friend class ASTIntegerListBuilder;

private:
  std::vector<int32_t> List;
  char Separator;
  int32_t Stepping;

public:
  using list_type = std::vector<int32_t>;
  using iterator = typename list_type::iterator;
  using const_iterator = typename list_type::const_iterator;

public:
  ASTIntegerList() : List(), Separator(':'), Stepping(1) {}

  ASTIntegerList(const std::string &S);

  virtual ~ASTIntegerList() = default;

  void SetSeparator(char C) { Separator = C; }

  char GetSeparator() const { return Separator; }

  void SetStepping(int32_t V) { Stepping = V; }

  int32_t GetStepping() const { return Stepping; }

  void Clear() { List.clear(); }

  std::size_t Size() const { return List.size(); }

  void Append(int32_t V) { List.insert(List.end(), V); }

  void Append(const ASTIntNode *IV) {
    assert(IV && "Invalid ASTIntNode argument!");
    List.insert(List.end(), IV->GetSignedValue());
  }

  void Prepend(int32_t V) { List.insert(List.begin(), V); }

  void Prepend(const ASTIntNode *IV) {
    assert(IV && "Invalid ASTIntNode argument!");
    List.insert(List.begin(), IV->GetSignedValue());
  }

  bool Empty() const { return List.size() == 0; }

  int32_t First() const { return List.front(); }

  int32_t Last() const { return List.back(); }

  iterator begin() { return List.begin(); }

  const_iterator begin() const { return List.begin(); }

  iterator end() { return List.end(); }

  const_iterator end() const { return List.end(); }

  int32_t front() const { return List.front(); }

  int32_t back() const { return List.back(); }

  int32_t operator[](unsigned Index) const {
    assert(Index < List.size() && "Index is out-of-range!");
    return List[Index];
  }

  virtual void print() const {
    std::cout << "<IntegerList>" << std::endl;
    std::cout << "<Separator>" << Separator << "</Separator>" << std::endl;
    for (ASTIntegerList::const_iterator I = List.begin(); I != List.end(); ++I)
      std::cout << "<int32_t>" << (*I) << "</int32_t>" << std::endl;
    std::cout << "</IntegerList>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_INTEGER_LIST_H
