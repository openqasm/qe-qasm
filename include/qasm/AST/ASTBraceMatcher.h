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

#ifndef __QASM_AST_BRACE_MATCHER_H
#define __QASM_AST_BRACE_MATCHER_H

namespace QASM {

class ASTIfBraceMatcher {
private:
  static ASTIfBraceMatcher CM;

private:
  unsigned Left;
  unsigned Right;

protected:
  ASTIfBraceMatcher() : Left(0U), Right(0U) { }

public:
  static ASTIfBraceMatcher& Instance() {
    return CM;
  }

  ~ASTIfBraceMatcher() = default;

  void CountLeft() {
    Left += 1U;
  }

  void CountRight() {
    Right += 1U;
  }

  void Reset() {
    Left = Right = 0U;
  }

  unsigned GetLeft() const {
    return Left;
  }

  unsigned GetRight() const {
    return Right;
  }

  bool IsZero() const {
    return Left == 0U && Right == 0U;
  }
};

class ASTElseIfBraceMatcher {
private:
  static ASTElseIfBraceMatcher CM;

private:
  unsigned Left;
  unsigned Right;

protected:
  ASTElseIfBraceMatcher() : Left(0U), Right(0U) { }

public:
  static ASTElseIfBraceMatcher& Instance() {
    return CM;
  }

  ~ASTElseIfBraceMatcher() = default;

  void CountLeft() {
    Left += 1U;
  }

  void CountRight() {
    Right += 1U;
  }

  void Reset() {
    Left = Right = 0U;
  }

  unsigned GetLeft() const {
    return Left;
  }

  unsigned GetRight() const {
    return Right;
  }

  bool IsZero() const {
    return Left == 0U && Right == 0U;
  }
};

class ASTElseBraceMatcher {
private:
  static ASTElseBraceMatcher CM;

private:
  unsigned Left;
  unsigned Right;

protected:
  ASTElseBraceMatcher() : Left(0U), Right(0U) { }

public:
  static ASTElseBraceMatcher& Instance() {
    return CM;
  }

  ~ASTElseBraceMatcher() = default;

  void CountLeft() {
    Left += 1U;
  }

  void CountRight() {
    Right += 1U;
  }

  void Reset() {
    Left = Right = 0U;
  }

  unsigned GetLeft() const {
    return Left;
  }

  unsigned GetRight() const {
    return Right;
  }

  bool IsZero() const {
    return Left == 0U && Right == 0U;
  }
};

class ASTForBraceMatcher {
private:
  static ASTForBraceMatcher FM;

private:
  unsigned Left;
  unsigned Right;

protected:
  ASTForBraceMatcher() : Left(0U), Right(0U) { }

public:
  static ASTForBraceMatcher& Instance() {
    return FM;
  }

  ~ASTForBraceMatcher() = default;

  void CountLeft() {
    Left += 1U;
  }

  void CountRight() {
    Right += 1U;
  }

  void Reset() {
    Left = Right = 0U;
  }

  unsigned GetLeft() const {
    return Left;
  }

  unsigned GetRight() const {
    return Right;
  }

  bool IsZero() const {
    return Left == 0U && Right == 0U;
  }
};

class ASTWhileBraceMatcher {
private:
  static ASTWhileBraceMatcher WM;

private:
  unsigned Left;
  unsigned Right;

protected:
  ASTWhileBraceMatcher() : Left(0U), Right(0U) { }

public:
  static ASTWhileBraceMatcher& Instance() {
    return WM;
  }

  ~ASTWhileBraceMatcher() = default;

  void CountLeft() {
    Left += 1U;
  }

  void CountRight() {
    Right += 1U;
  }

  void Reset() {
    Left = Right = 0U;
  }

  unsigned GetLeft() const {
    return Left;
  }

  unsigned GetRight() const {
    return Right;
  }

  bool IsZero() const {
    return Left == 0U && Right == 0U;
  }
};

class ASTDoWhileBraceMatcher {
private:
  static ASTDoWhileBraceMatcher DWM;

private:
  unsigned Left;
  unsigned Right;

protected:
  ASTDoWhileBraceMatcher() : Left(0U), Right(0U) { }

public:
  static ASTDoWhileBraceMatcher& Instance() {
    return DWM;
  }

  ~ASTDoWhileBraceMatcher() = default;

  void CountLeft() {
    Left += 1U;
  }

  void CountRight() {
    Right += 1U;
  }

  void Reset() {
    Left = Right = 0U;
  }

  unsigned GetLeft() const {
    return Left;
  }

  unsigned GetRight() const {
    return Right;
  }
};

class ASTGateBraceMatcher {
private:
  static ASTGateBraceMatcher GM;

private:
  unsigned Left;
  unsigned Right;

protected:
  ASTGateBraceMatcher() : Left(0U), Right(0U) { }

public:
  static ASTGateBraceMatcher& Instance() {
    return GM;
  }

  ~ASTGateBraceMatcher() = default;

  void CountLeft() {
    Left += 1U;
  }

  void CountRight() {
    Right += 1U;
  }

  void Reset() {
    Left = Right = 0U;
  }

  unsigned GetLeft() const {
    return Left;
  }

  unsigned GetRight() const {
    return Right;
  }

  bool IsZero() const {
    return Left == 0U && Right == 0U;
  }
};

class ASTDefcalBraceMatcher {
private:
  static ASTDefcalBraceMatcher DM;

private:
  unsigned Left;
  unsigned Right;

protected:
  ASTDefcalBraceMatcher() : Left(0U), Right(0U) { }

public:
  static ASTDefcalBraceMatcher& Instance() {
    return DM;
  }

  ~ASTDefcalBraceMatcher() = default;

  void CountLeft() {
    Left += 1U;
  }

  void CountRight() {
    Right += 1U;
  }

  void Reset() {
    Left = Right = 0U;
  }

  unsigned GetLeft() const {
    return Left;
  }

  unsigned GetRight() const {
    return Right;
  }

  bool IsZero() const {
    return Left == 0U && Right == 0U;
  }
};

class ASTFunctionBraceMatcher {
private:
  static ASTFunctionBraceMatcher FM;

private:
  unsigned Left;
  unsigned Right;

protected:
  ASTFunctionBraceMatcher() : Left(0U), Right(0U) { }

public:
  static ASTFunctionBraceMatcher& Instance() {
    return FM;
  }

  ~ASTFunctionBraceMatcher() = default;

  void CountLeft() {
    Left += 1U;
  }

  void CountRight() {
    Right += 1U;
  }

  void Reset() {
    Left = Right = 0U;
  }

  unsigned GetLeft() const {
    return Left;
  }

  unsigned GetRight() const {
    return Right;
  }

  bool IsZero() const {
    return Left == 0U && Right == 0U;
  }
};

class ASTSwitchBraceMatcher {
private:
  static ASTSwitchBraceMatcher SM;

private:
  unsigned Left;
  unsigned Right;

protected:
  ASTSwitchBraceMatcher() : Left(0U), Right(0U) { }

public:
  static ASTSwitchBraceMatcher& Instance() {
    return SM;
  }

  ~ASTSwitchBraceMatcher() = default;

  void CountLeft() {
    Left += 1U;
  }

  void CountRight() {
    Right += 1U;
  }

  void Reset() {
    Left = Right = 0U;
  }

  unsigned GetLeft() const {
    return Left;
  }

  unsigned GetRight() const {
    return Right;
  }

  bool IsZero() const {
    return Left == 0U && Right == 0U;
  }
};

class ASTCaseBraceMatcher {
private:
  static ASTCaseBraceMatcher CM;

private:
  unsigned Left;
  unsigned Right;

protected:
  ASTCaseBraceMatcher() : Left(0U), Right(0U) { }

public:
  static ASTCaseBraceMatcher& Instance() {
    return CM;
  }

  ~ASTCaseBraceMatcher() = default;

  void CountLeft() {
    Left += 1U;
  }

  void CountRight() {
    Right += 1U;
  }

  void Reset() {
    Left = Right = 0U;
  }

  unsigned GetLeft() const {
    return Left;
  }

  unsigned GetRight() const {
    return Right;
  }

  bool IsZero() const {
    return Left == 0U && Right == 0U;
  }
};

class ASTDefaultBraceMatcher {
private:
  static ASTDefaultBraceMatcher DM;

private:
  unsigned Left;
  unsigned Right;

protected:
  ASTDefaultBraceMatcher() : Left(0U), Right(0U) { }

public:
  static ASTDefaultBraceMatcher& Instance() {
    return DM;
  }

  ~ASTDefaultBraceMatcher() = default;

  void CountLeft() {
    Left += 1U;
  }

  void CountRight() {
    Right += 1U;
  }

  void Reset() {
    Left = Right = 0U;
  }

  unsigned GetLeft() const {
    return Left;
  }

  unsigned GetRight() const {
    return Right;
  }

  bool IsZero() const {
    return Left == 0U && Right == 0U;
  }
};

class ASTExternBraceMatcher {
private:
  static ASTExternBraceMatcher EM;

private:
  unsigned Left;
  unsigned Right;

protected:
  ASTExternBraceMatcher() : Left(0U), Right(0U) { }

public:
  static ASTExternBraceMatcher& Instance() {
    return EM;
  }

  ~ASTExternBraceMatcher() = default;

  void CountLeft() {
    Left += 1U;
  }

  void CountRight() {
    Right += 1U;
  }

  void Reset() {
    Left = Right = 0U;
  }

  unsigned GetLeft() const {
    return Left;
  }

  unsigned GetRight() const {
    return Right;
  }
};

} // namespace QASM

#endif // __QASM_AST_BRACE_MATCHER_H

