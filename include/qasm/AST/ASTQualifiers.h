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

#ifndef __QASM_AST_QUALIFIERS_H
#define __QASM_AST_QUALIFIERS_H

#include <qasm/AST/ASTExpression.h>
#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTIdentifier.h>

#include <iostream>
#include <bitset>
#include <cassert>

namespace QASM {

class ASTQualifiers {
public:
  ASTQualifiers() = default;

  ASTQualifiers(const ASTQualifiers& RHS) { (void) RHS; }

  ASTQualifiers& operator=(const ASTQualifiers& RHS) {
    (void) RHS;
    return *this;
  }

  virtual ~ASTQualifiers() = default;
};

class ASTCVRQualifiers : public ASTQualifiers {
private:
  // Index 0 - const
  // Index 1 - volatile
  // Index 2 - restrict
  std::bitset<3> CVRQualifiers;

public:
  ASTCVRQualifiers() : ASTQualifiers(), CVRQualifiers(0) { }

  ASTCVRQualifiers(ASTType Q)
  : ASTQualifiers(), CVRQualifiers(0) {
    switch (Q) {
    case ASTTypeConst:
      CVRQualifiers[0] = 1;
      break;
    case ASTTypeVolatile:
      CVRQualifiers[1] = 1;
      break;
    case ASTTypeRestrict:
      CVRQualifiers[2] = 1;
      break;
    default:
      break;
    }
  }

  ASTCVRQualifiers(ASTType Q0, ASTType Q1)
  : ASTQualifiers(), CVRQualifiers(0) {
    assert(Q0 != Q1 && "Pointless duplication of Type Qualifiers!");

    switch (Q0) {
    case ASTTypeConst:
      CVRQualifiers[0] = 1;
      break;
    case ASTTypeVolatile:
      CVRQualifiers[1] = 1;
      break;
    case ASTTypeRestrict:
      CVRQualifiers[2] = 1;
      break;
    default:
      break;
    }

    switch (Q1) {
    case ASTTypeConst:
      CVRQualifiers[0] = 1;
      break;
    case ASTTypeVolatile:
      CVRQualifiers[1] = 1;
      break;
    case ASTTypeRestrict:
      CVRQualifiers[2] = 1;
      break;
    default:
      break;
    }
  }

  ASTCVRQualifiers(ASTType Q0, ASTType Q1, ASTType Q2)
  : ASTQualifiers(), CVRQualifiers(0) {
    assert(Q0 != Q1 && "Pointless duplication of Type Qualifiers!");
    assert(Q1 != Q2 && "Pointless duplication of Type Qualifiers!");
    assert(Q0 != Q2 && "Pointless duplication of Type Qualifiers!");

    switch (Q0) {
    case ASTTypeConst:
      CVRQualifiers[0] = 1;
      break;
    case ASTTypeVolatile:
      CVRQualifiers[1] = 1;
      break;
    case ASTTypeRestrict:
      CVRQualifiers[2] = 1;
      break;
    default:
      break;
    }

    switch (Q1) {
    case ASTTypeConst:
      CVRQualifiers[0] = 1;
      break;
    case ASTTypeVolatile:
      CVRQualifiers[1] = 1;
      break;
    case ASTTypeRestrict:
      CVRQualifiers[2] = 1;
      break;
    default:
      break;
    }

    switch (Q2) {
    case ASTTypeConst:
      CVRQualifiers[0] = 1;
      break;
    case ASTTypeVolatile:
      CVRQualifiers[1] = 1;
      break;
    case ASTTypeRestrict:
      CVRQualifiers[2] = 1;
      break;
    default:
      break;
    }
  }

  ASTCVRQualifiers(const ASTIdentifierNode& ID)
  : ASTQualifiers(), CVRQualifiers(0) {
    const std::string& Id = ID.GetName();
    if (Id == "const")
      SetConst(true);
    else if (Id  == "volatile")
      SetVolatile(true);
    else if (Id == "restrict")
      SetRestrict(true);
  }

  ASTCVRQualifiers(const std::string& Q)
  : ASTQualifiers(), CVRQualifiers(0) {
    if (Q == "const")
      SetConst(true);
    else if (Q == "volatile")
      SetVolatile(true);
    else if (Q == "restrict")
      SetRestrict(true);
  }

  ASTCVRQualifiers(const ASTCVRQualifiers& RHS)
  : ASTQualifiers(RHS), CVRQualifiers(RHS.CVRQualifiers) { }

  ASTCVRQualifiers& operator=(const ASTCVRQualifiers& RHS) {
    if (this != &RHS) {
      ASTQualifiers::operator=(RHS);
      CVRQualifiers = RHS.CVRQualifiers;
    }

    return *this;
  }

  virtual ~ASTCVRQualifiers() = default;

  virtual bool IsConst() const {
    return CVRQualifiers[0];
  }

  virtual bool IsVolatile() const {
    return CVRQualifiers[1];
  }

  virtual bool IsRestrict() const {
    return CVRQualifiers[2];
  }

  virtual void SetConst(bool V = true) {
    CVRQualifiers[0] = V;
  }

  virtual void SetVolatile(bool V = true) {
    CVRQualifiers[1] = V;
  }

  virtual void SetRestrict(bool V = true) {
    CVRQualifiers[2] = V;
  }

  virtual void print() const {
    std::cout << "<TypeQualifiers>" << std::endl;
    if (CVRQualifiers[0])
      std::cout << "<const>" << std::boolalpha << CVRQualifiers[0]
        << "</const>" << std::endl;
    if (CVRQualifiers[1])
      std::cout << "<volatile>" << std::boolalpha << CVRQualifiers[1]
        << "</volatile>" << std::endl;
    if (CVRQualifiers[2])
      std::cout << "<restrict>" << std::boolalpha << CVRQualifiers[2]
        << "</restrict>" << std::endl;
    std::cout << "</TypeQualifiers>" << std::endl;
  }
};

class ASTStorageQualifiers : public ASTQualifiers {
private:
  // Index 0 - static
  // Index 1 - global
  // Index 2 - extern
  std::bitset<3> StorageQualifiers;

public:
  ASTStorageQualifiers() : ASTQualifiers(), StorageQualifiers(0) { }

  ASTStorageQualifiers(ASTType Q0)
  : ASTQualifiers(), StorageQualifiers(0) {
    switch (Q0) {
    case ASTTypeStatic:
      StorageQualifiers[0] = 1;
      break;
    case ASTTypeGlobal:
      StorageQualifiers[1] = 1;
      break;
    case ASTTypeExtern:
      StorageQualifiers[2] = 1;
      break;
    default:
      break;
    }
  }

  ASTStorageQualifiers(ASTType Q0, ASTType Q1)
  : ASTQualifiers(), StorageQualifiers(0) {
    assert(Q0 != Q1 && "Pointless duplication of Storage Qualifiers!");

    switch (Q0) {
    case ASTTypeStatic:
      StorageQualifiers[0] = 1;
      break;
    case ASTTypeGlobal:
      StorageQualifiers[1] = 1;
      break;
    case ASTTypeExtern:
      StorageQualifiers[2] = 1;
      break;
    default:
      break;
    }

    switch (Q1) {
    case ASTTypeStatic:
      StorageQualifiers[0] = 1;
      break;
    case ASTTypeGlobal:
      StorageQualifiers[1] = 1;
      break;
    case ASTTypeExtern:
      StorageQualifiers[2] = 1;
      break;
    default:
      break;
    }
  }

  ASTStorageQualifiers(ASTType Q0, ASTType Q1, ASTType Q2)
  : ASTQualifiers(), StorageQualifiers(0) {
    assert(Q0 != Q1 && "Pointless duplication of Storage Qualifiers!");
    assert(Q1 != Q2 && "Pointless duplication of Storage Qualifiers!");
    assert(Q0 != Q2 && "Pointless duplication of Storage Qualifiers!");

    switch (Q0) {
    case ASTTypeStatic:
      StorageQualifiers[0] = 1;
      break;
    case ASTTypeGlobal:
      StorageQualifiers[1] = 1;
      break;
    case ASTTypeExtern:
      StorageQualifiers[2] = 1;
      break;
    default:
      break;
    }

    switch (Q1) {
    case ASTTypeStatic:
      StorageQualifiers[0] = 1;
      break;
    case ASTTypeGlobal:
      StorageQualifiers[1] = 1;
      break;
    case ASTTypeExtern:
      StorageQualifiers[2] = 1;
      break;
    default:
      break;
    }

    switch (Q2) {
    case ASTTypeStatic:
      StorageQualifiers[0] = 1;
      break;
    case ASTTypeGlobal:
      StorageQualifiers[1] = 1;
      break;
    case ASTTypeExtern:
      StorageQualifiers[2] = 1;
      break;
    default:
      break;
    }
  }

  ASTStorageQualifiers(const ASTIdentifierNode& ID)
  : ASTQualifiers(), StorageQualifiers(0) {
    const std::string& Id = ID.GetName();
    if (Id == "static")
      SetStatic(true);
    else if (Id == "extern")
      SetExtern(true);
  }

  ASTStorageQualifiers(const std::string& Q)
  : ASTQualifiers(), StorageQualifiers(0) {
    if (Q == "static")
      SetStatic(true);
    else if (Q == "extern")
      SetExtern(true);
  }

  ASTStorageQualifiers(const ASTStorageQualifiers& RHS)
  : ASTQualifiers(RHS), StorageQualifiers(RHS.StorageQualifiers) { }

  ASTStorageQualifiers& operator=(const ASTStorageQualifiers& RHS) {
    if (this != &RHS) {
      ASTQualifiers::operator=(RHS);
      StorageQualifiers = RHS.StorageQualifiers;
    }

    return *this;
  }

  virtual ~ASTStorageQualifiers() = default;

  virtual bool IsStatic() const {
    return StorageQualifiers[0];
  }

  virtual bool IsGlobal() const {
    return StorageQualifiers[1];
  }

  virtual bool IsExtern() const {
    return StorageQualifiers[2];
  }

  virtual void SetStatic(bool V = true) {
    StorageQualifiers[0] = V;
  }

  virtual void SetGlobal(bool V = true) {
    StorageQualifiers[1] = V;
  }

  virtual void SetExtern(bool V = true) {
    StorageQualifiers[2] = V;
  }

  virtual void print() const {
    std::cout << "<StorageQualifiers>" << std::endl;
    if (StorageQualifiers[0])
      std::cout << "<static>" << std::boolalpha
        << StorageQualifiers[0] << "</static>" << std::endl;
    if (StorageQualifiers[1])
      std::cout << "<global>" << std::boolalpha
        << StorageQualifiers[1] << "</global>" << std::endl;
    if (StorageQualifiers[2])
      std::cout << "<extern>" << std::boolalpha
        << StorageQualifiers[2] << "</extern>" << std::endl;
    std::cout << "</StorageQualifiers>" << std::endl;
  }
};

class ASTGateQualifiers : public ASTQualifiers {
private:
  // Index 0 - dirty
  // Index 1 - opaque
  std::bitset<2> GateQualifiers;

public:
  ASTGateQualifiers() : ASTQualifiers(), GateQualifiers(0) { }

  ASTGateQualifiers(ASTType Q)
  : ASTQualifiers(), GateQualifiers(0) {
    if (Q == ASTTypeDirty)
      GateQualifiers[0] = 1;
    else if (Q == ASTTypeOpaque)
      GateQualifiers[1] = 1;
  }

  ASTGateQualifiers(ASTType Q0, ASTType Q1)
  : ASTQualifiers(), GateQualifiers(0) {
    assert(Q0 != Q1 && "Pointless duplication of Gate Qualifiers!");

    switch (Q0) {
    case ASTTypeDirty:
      GateQualifiers[0] = 1;
      break;
    case ASTTypeOpaque:
      GateQualifiers[1] = 1;
      break;
    default:
      break;
    }

    switch (Q1) {
    case ASTTypeDirty:
      GateQualifiers[0] = 1;
      break;
    case ASTTypeOpaque:
      GateQualifiers[1] = 1;
      break;
    default:
      break;
    }
  }

  ASTGateQualifiers(const ASTIdentifierNode& ID)
  : ASTQualifiers(), GateQualifiers(0) {
    const std::string& Id = ID.GetName();
    if (Id == "dirty")
      SetDirty(true);
    else if (Id == "opaque")
      SetOpaque(true);
  }

  ASTGateQualifiers(const std::string& Q)
  : ASTQualifiers(), GateQualifiers(0) {
    if (Q == "dirty")
      SetDirty(true);
    else if (Q == "opaque")
      SetOpaque(true);
  }

  ASTGateQualifiers(const ASTGateQualifiers& RHS)
  : ASTQualifiers(RHS), GateQualifiers(RHS.GateQualifiers) { }

  ASTGateQualifiers& operator=(const ASTGateQualifiers& RHS) {
    if (this != &RHS) {
      ASTQualifiers::operator=(RHS);
      GateQualifiers = RHS.GateQualifiers;
    }

    return *this;
  }

  virtual ~ASTGateQualifiers() = default;

  virtual bool IsDirty() const {
    return GateQualifiers[0];
  }

  virtual bool IsOpaque() const {
    return GateQualifiers[1];
  }

  virtual void SetDirty(bool V = true) {
    GateQualifiers[0] = V;
  }

  virtual void SetOpaque(bool V = true) {
    GateQualifiers[1] = V;
  }

  virtual void print() const {
    std::cout << "<GateQualifiers>" << std::endl;
    if (GateQualifiers[0])
      std::cout << "<dirty>" << std::boolalpha
        << GateQualifiers[0] << "</dirty>" << std::endl;
    if (GateQualifiers[1])
      std::cout << "<opaque>" << std::boolalpha
        << GateQualifiers[1] << "</opaque>" << std::endl;
    std::cout << "</GateQualifiers>" << std::endl;
  }
};

} // namespace QASM

#endif // __QASM_AST_QUALIFIERS_H

