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

#ifndef __QASM_AST_ANY_TYPE_LIST_H
#define __QASM_AST_ANY_TYPE_LIST_H

#include <qasm/AST/ASTBase.h>
#include <qasm/AST/ASTAnyType.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTIdentifier.h>

#include <any>
#include <functional>
#include <vector>
#include <cassert>

namespace QASM {

class ASTAnyTypeList {
  friend class ASTAnyTypeBuilder;

protected:
  std::vector<ASTAnyType> Graph;

public:
  using list_type = std::vector<ASTAnyType>;
  using iterator = typename std::vector<ASTAnyType>::iterator;
  using const_iterator = typename std::vector<ASTAnyType>::const_iterator;

public:
  ASTAnyTypeList() : Graph() { }

  ASTAnyTypeList(const ASTAnyTypeList& RHS)
  : Graph(RHS.Graph) { }

  virtual ~ASTAnyTypeList() = default;

  ASTAnyTypeList& operator=(const ASTAnyTypeList& RHS) {
    if (this != &RHS) {
      Graph = RHS.Graph;
    }

    return *this;
  }

  virtual unsigned Size() const {
    return static_cast<unsigned>(Graph.size());
  }

  virtual bool Empty() const {
    return Graph.empty();
  }

  iterator begin() { return Graph.begin(); }

  const_iterator begin() const { return Graph.begin(); }

  iterator end() { return Graph.end(); }

  const_iterator end() const { return Graph.end(); }

  virtual ASTType GetASTType() const {
    return ASTTypeStatementList;
  }

  virtual ASTType GetASTType(size_t Index) const {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index].second;
  }

#if defined(BINDER_CODEGEN_PYBIND11)
  inline virtual ASTAnyType operator[](size_t Index) const {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index];
  }
#else
  inline virtual ASTAnyType& operator[](size_t Index) {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index];
  }

  inline virtual const ASTAnyType& operator[](size_t Index) const {
    assert(Index < Graph.size() && "Index is out-of-range!");
    return Graph[Index];
  }
#endif

  inline virtual bool IsIdentifier(unsigned Index) const {
    assert(Index < Graph.size() && "Index is out-of-range!");
    try {
      const ASTAnyType& AT = Graph.at(Index);
      return AT.second == ASTTypeIdentifier;
    } catch (const std::out_of_range& E) {
      (void) E;
    } catch ( ... ) {
    }

    return false;
  }

  inline virtual bool IsIdentifierRef(unsigned Index) const {
    assert(Index < Graph.size() && "Index is out-of-range!");
    try {
      const ASTAnyType& AT = Graph.at(Index);
      return AT.second == ASTTypeIdentifierRef;
    } catch (const std::out_of_range& E) {
      (void) E;
    } catch ( ... ) {
    }

    return false;
  }

  virtual const ASTIdentifierNode* GetIdentifier(unsigned Index) const {
    assert(Index < Graph.size() && "Index is out-of-range!");
    const ASTIdentifierNode* Id = nullptr;

    try {
      const ASTAnyType& AT = Graph.at(Index);
      if (AT.second == ASTTypeIdentifier) {
        try {
          Id = std::any_cast<ASTIdentifierNode*>(AT.first);
          assert(Id && "Invalid ASTIdentifierNode cast from std::any!");
          return Id;
        } catch (const std::bad_any_cast& E) {
          (void) E;
          return nullptr;
        } catch ( ... ) {
          return nullptr;
        }
      }
    } catch (const std::out_of_range& E) {
      (void) E;
      return nullptr;
    } catch ( ... ) {
      return nullptr;
    }

    return nullptr;
  }

  virtual ASTIdentifierNode* GetIdentifier(unsigned Index) {
    assert(Index < Graph.size() && "Index is out-of-range!");
    ASTIdentifierNode* Id = nullptr;

    try {
      ASTAnyType& AT = Graph.at(Index);
      if (AT.second == ASTTypeIdentifier) {
        try {
          Id = std::any_cast<ASTIdentifierNode*>(AT.first);
          assert(Id && "Invalid ASTIdentifierNode cast from std::any!");
          return Id;
        } catch (const std::bad_any_cast& E) {
          return nullptr;
        } catch ( ... ) {
          return nullptr;
        }
      }
    } catch (const std::out_of_range& E) {
      (void) E;
      return nullptr;
    } catch ( ... ) {
      return nullptr;
    }

    return nullptr;
  }

  virtual const ASTIdentifierRefNode* GetIdentifierRef(unsigned Index) const {
    assert(Index < Graph.size() && "Index is out-of-range!");
    const ASTIdentifierRefNode* Id = nullptr;

    try {
      const ASTAnyType& AT = Graph.at(Index);
      if (AT.second == ASTTypeIdentifierRef) {
        try {
          Id = std::any_cast<ASTIdentifierRefNode*>(AT.first);
          assert(Id && "Invalid ASTIdentifierRefNode cast from std::any!");
          return Id;
        } catch (const std::bad_any_cast& E) {
          (void) E;
          return nullptr;
        } catch ( ... ) {
          return nullptr;
        }
      }
    } catch (const std::out_of_range& E) {
      (void) E;
      return nullptr;
    } catch ( ... ) {
      return nullptr;
    }

    return nullptr;
  }

  virtual ASTIdentifierRefNode* GetIdentifierRef(unsigned Index) {
    assert(Index < Graph.size() && "Index is out-of-range!");
    ASTIdentifierRefNode* Id = nullptr;

    try {
      ASTAnyType& AT = Graph.at(Index);
      if (AT.second == ASTTypeIdentifierRef) {
        try {
          Id = std::any_cast<ASTIdentifierRefNode*>(AT.first);
          assert(Id && "Invalid ASTIdentifierRefNode cast from std::any!");
          return Id;
        } catch (const std::bad_any_cast& E) {
          (void) E;
          return nullptr;
        } catch ( ... ) {
          return nullptr;
        }
      }
    } catch (const std::out_of_range& E) {
      (void) E;
      return nullptr;
    } catch ( ... ) {
      return nullptr;
    }

    return nullptr;
  }

  virtual void print() const {
    unsigned C = 0;
    std::cout << "<AnyTypeList>" << std::endl;

    for (std::vector<ASTAnyType>::const_iterator I = Graph.begin();
         I != Graph.end(); ++I) {
      // FIXME: Implement ASTAnyTypePrinter.
      // (*I).first.print();

      std::cout << "<Element>" << std::endl;
      std::cout << "<ElementType>" << PrintTypeEnum((*I).second)
        << "</ElementType>" << std::endl;
      if ((*I).second == ASTTypeIdentifier) {
        try {
          const ASTIdentifierNode* Id =
            std::any_cast<ASTIdentifierNode*>((*I).first);
          assert(Id && "Invalid ASTIdentifierNode cast from std::any!");
          std::cout << "<Index>" << C++ << "</Index>" << std::endl;
          Id->print();
        } catch (const std::bad_any_cast& E) {
          std::cout << "<Index>" << C++ << "</Index>" << std::endl;
          std::cout << "<Error>" << E.what() << "</Error>" << std::endl;
        }
      } else if ((*I).second == ASTTypeIdentifierRef) {
        try {
          const ASTIdentifierRefNode* Id =
            std::any_cast<ASTIdentifierRefNode*>((*I).first);
          assert(Id && "Invalid ASTIdentifierRefNode cast from std::any!");
          std::cout << "<Index>" << C++ << "</Index>" << std::endl;
          Id->print();
        } catch (const std::bad_any_cast& E) {
          std::cout << "<Index>" << C++ << "</Index>" << std::endl;
          std::cout << "<Error>" << E.what() << "</Error>" << std::endl;
        }
      } else {
        std::cout << "<Placeholder>" << std::endl;
        std::cout << "<Index>" << C++ << "</Index>" << std::endl;
        std::cout << "</Placeholder>" << std::endl;
      }
      std::cout << "</Element>" << std::endl;
    }

    std::cout << "</AnyTypeList>" << std::endl;
  }

  virtual void push(const ASTAnyType& Ty) {
    Graph.push_back(Ty);
  }
};

} // namespace QASM

#endif // __QASM_AST_ANY_TYPE_LIST_H

