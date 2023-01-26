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

#include <qasm/AST/ASTMangler.h>
#include <qasm/AST/ASTFunctions.h>
#include <qasm/AST/ASTKernel.h>
#include <qasm/AST/ASTMeasure.h>
#include <qasm/AST/ASTReturn.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseFrame.h>
#include <qasm/AST/OpenPulse/ASTOpenPulsePort.h>
#include <qasm/AST/OpenPulse/ASTOpenPulseWaveform.h>
#include <qasm/AST/ASTTypeSystemBuilder.h>
#include <qasm/AST/ASTUtils.h>
#include <qasm/AST/ASTStringUtils.h>
#include <qasm/AST/ASTExpressionValidator.h>
#include <qasm/Diagnostic/DIAGLineCounter.h>
#include <qasm/Frontend/QasmDiagnosticEmitter.h>

#include <vector>
#include <regex>
#include <limits>
#include <type_traits>
#include <climits>
#include <cstring>

#if defined(__APPLE__)
#include <machine/endian.h>
#else
#include <endian.h>
#endif

extern "C" {
  typedef struct {
    uint64_t X;
    uint64_t Y;
  } SLD;
}

namespace QASM {

enum RXIndex : unsigned {
  RXInt32Pos = 1,
  RXInt32Neg,
  RXUInt32,
  RXInt64Pos,
  RXInt64Neg,
  RXUInt64,
  RXMPIntPos,
  RXMPIntNeg,
  RXMPUInt,
  RXFloat,
  RXDouble,
  RXLongDouble,
  RXFloatNeg,
  RXDoubleNeg,
  RXLongDoubleNeg,
  RXMPDecPos,
  RXMPDecNeg,
  RXMPComplex,
  RXString,
  RXConstString,
};

enum RXParam : unsigned {
  RXDa = 1,
  RXDp,
  RXFa,
  RXFp,
  RXGa,
  RXGp,
  RXKa,
  RXKp,
};

ASTDemangledRegistry ASTDemangledRegistry::ADR;
std::set<const ASTDemangled*> ASTDemangledRegistry::RS;

std::map<ASTType, Mangler::MToken> ASTMangler::TDMM;
std::map<ASTOpType, Mangler::MToken> ASTMangler::ODMM;

std::map<ASTType, Mangler::MToken> ASTDemangler::TDMM;
std::map<ASTOpType, Mangler::MToken> ASTDemangler::ODMM;
std::vector<ASTDemangler::RXM> ASTDemangler::TYMM;
std::map<RXParam, std::regex> ASTDemangler::PAMM;

using DiagLevel = QASM::QasmDiagnosticEmitter::DiagLevel;

// Mangling is based on the Itanium C++ ABI mangling, with many
// changes and adjustments for Quantum types.
// The startup sequence for IBM Quantum mangling is '_Q', avoiding
// conflicts with the C++ ABI mangling namespace.
void ASTMangler::Init() {
  if (TDMM.empty()) {
    TDMM = {
      { ASTTypeBool, { "b", 1 }, },          // C bool
      { ASTTypeChar, { "c", 1 }, },          // C char
      { ASTTypeUTF8, { "u8", 2 }, },         // UTF-8 char
      { ASTTypeInt, { "i", 1 }, },           // C 32-bit signed integer
      { ASTTypeUInt, { "j", 1 }, },          // C 32-bit unsigned integer
      { ASTTypeUnsignedInt, { "j", 1 }, },   // C 32-bit unsigned integer
      { ASTTypeLong, { "x", 1 }, },          // C 64-bit signed integer
      { ASTTypeULong, { "y", 1 }, },         // C 64-bit unsigned integer
      { ASTTypeUnsignedLong, { "y", 1 }, },  // C 64-bit unsigned integer
      { ASTTypeFloat, { "f", 1 }, },         // C/IEEE-754 32-bit float
      { ASTTypeDouble, { "d", 1 }, },        // C/IEEE-754 64-bit double
      { ASTTypeLongDouble, { "e", 1 }, },    // C 128-bit long double
      { ASTTypeVoid, { "v", 1 }, },          // void
      { ASTTypeEllipsis, { "z", 1 }, },      // ellipsis
      { ASTTypeForLoopRange, { "flr", 3 }, }, // for loop range expression
      { ASTTypeBracedInitializerList,
        { "il", 2 }, },                      // braced-initializer-list
                                             // # [braced-expression 'E']
      { ASTTypePragma, { "p", 1 }, },        // pragma
      { ASTTypeAnnotation, { "at", 2 }, },   // annotation
      { ASTTypePointer, { "P", 1 }, },       // pointer (arrays as function arguments)
      { ASTTypeLValueReference,
        { "LVR", 3 }, },                     // lvalue reference
      { ASTTypeRValueReference,
        { "RVR", 3 }, },                     // rvalue reference
      { ASTTypeRotateExpr,
        { "r", 2 }, },                       // # ['L' <number> '_'] rotate left
                                             // # ['R' <number> '_'] rotate right
      { ASTTypePopcountExpr,
        { "pC", 2 }, },                      // popcount
      { ASTTypeMPInteger, { "II", 2 }, },    // # [<number> '_'] arbitrary precision
                                             // integer
      { ASTTypeMPUInteger, { "JJ", 2 }, },   // # [<number> '_'] arbitrary precision
                                             // unsigned integer
      { ASTTypeMPDecimal, { "DD", 2 }, },    // # ['F' <number> '_'] ISO:18661
                                             // arbitrary precision floating-point
                                             // # ['d' '_'] IEEE-754 64-bit
                                             // decimal floating-point
                                             // # ['e' '_'] IEEE-754 128-bit
                                             // decimal floating-point
                                             // # ['f' '_'] IEEE-754 32-bit
                                             // decimal floating-point
      { ASTTypeFunction, { "F", 1 }, },      // function
      { ASTTypeFunctionDeclaration,
        { "FD", 2 }, },                      // function prototype declaration
      { ASTTypeFunctionCall, { "FC", 2 }, }, // function call
      { ASTTypeExtern, { "Y", 1 }, },        // extern declaration qualifier
      { ASTTypeKernel, { "K", 1 }, },        // kernel
      { ASTTypeKernelCall, { "KC", 2 }, },   // kernel call
      { ASTTypeKernelDeclaration,
        { "KD", 2 }, },                      // kernel prototype declaration
      { ASTTypeCast, { "CXS", 3 }, },        // cast expression
      { ASTTypeImplicitConversion,
        { "IXC", 3 }, },                     // implicit conversion
      { ASTTypeAngle, { "X", 1 }, },         // # [<number> '_'] angle
      { ASTTypeMPComplex, { "C", 1 }, },     // # [<number> '_' 'i' <number> '_']
                                             // arbitrary precision complex pair int
                                             // # [<number> '_' 'i']
                                             // arbitrary precision complex pair int
                                             // # [<number> '_' 'j' <number> '_']
                                             // arbitrary precision complex pair uint
                                             // # [<number> '_' 'j']
                                             // arbitrary precision complex pair uint
                                             // # [<number> '_' 'f' <number> '_']
                                             // arbitrary precision complex pair float
                                             // # [<number> '_' 'f']
                                             // arbitrary precision complex pair float
                                             // # [<number> '_' 'd' <number> '_']
                                             // arbitrary precision complex pair double
                                             // # [<number> '_' 'd']
                                             // arbitrary precision complex pair double
      { ASTTypeComplexList,                  // arbitrary precistion complex list
        { "CXL", 3 }, },
      { ASTTypeMPComplexRep,                 // arbitrary precision complex
        { "CXR", 3 }, },                     // representation
      { ASTTypeComplexExpression,            // complex initializer expression
        { "CXX", 3 }, },
      { ASTTypeMPCReal, { "Cr", 2 }, },      // arbitrary precision complex real part
      { ASTTypeMPCImag, { "Ci", 2 }, },      // arbitrary precision complex imaginary part
      { ASTTypeImaginary, { "g", 1 }, },     // C99/C++ imaginary
      { ASTTypeGate, { "G", 1 }, },          // gate
      { ASTTypeCNotGate, { "G", 1 }, },      // gate
      { ASTTypeCXGate, { "G", 1 }, },        // gate
      { ASTTypeCCXGate, { "G", 1 }, },       // gate
      { ASTTypeHadamardGate, { "G", 1 }, },  // gate
      { ASTTypeUGate, { "G", 1 }, },         // gate
      { ASTTypeGateCall, { "GC", 2 }, },     // gate call
      { ASTTypeGateOpNode,
        { "Go", 2 }, },                      // Gate Op
      { ASTTypeGateQOpNode,
        { "Gq", 2 }, },                      // Gate QOp
      { ASTTypeGateUOpNode,
        { "Gu", 2 }, },                      // Gate UOp
      { ASTTypeGateGenericOpNode,
        { "Gg", 2 }, },                      // Gate Generic Op
      { ASTTypeGateHOpNode,
        { "Gh", 2 }, },                      // Gate HOp
      { ASTTypeCXGateOpNode,
        { "Gcx", 3 }, },                     // Gate CX Op
      { ASTTypeCCXGateOpNode,
        { "Gccx", 4 }, },                    // Gate CCX Op
      { ASTTypeCNotGateOpNode,
        { "Gcnot", 5 }, },                   // Gate CNOT Op
      { ASTTypeGateControl, { "Gc", 2 }, },  // gate control
      { ASTTypeGateInverse, { "Gi", 2 }, },  // gate inverse
      { ASTTypeGateNegControl,
        { "Gn", 2 }, },                      // gate negative control
      { ASTTypeGatePower, { "Gw", 2 }, },    // gate power
      { ASTTypeHash, { "H", 1 }, },          // hash of some other thing
      { ASTTypeQubit, { "Q", 1 }, },         // # [<number> '_'] qubit
      { ASTTypeBoundQubit, { "QB", 2 }, },   // bound qubit
      { ASTTypeUnboundQubit, { "QU", 2 }, }, // unbound qubit
      { ASTTypeQubitContainer,
        { "QC", 2 }, },                      // # [<number> '_'] qubit container
      { ASTTypeQubitContainerAlias,
        { "QCa", 3 }, },                     // # [<number> '_'] qubit container alias
      { ASTTypeGateQubitParam,
        { "GQP", 3 }, },                     // gate qubit parameter
      { ASTTypeQReg, { "RQ", 2 }, },         // quantum register
      { ASTTypeCReg, { "RB", 2 }, },         // classical register
      { ASTTypeConst, { "k", 1 }, },         // const qualifier
      { ASTTypeBitset, { "B", 1 }, },        // # [<number> '_'] bitset
      { ASTTypeDefcal, { "D", 1 }, },        // defcal
      { ASTTypeDefcalCall, { "DC", 2 }, },   // defcal call
      { ASTTypeDefcalDelay, { "DL", 2 }, },  // defcal delay
      { ASTTypeDefcalMeasure,
        { "DM", 2 }, },                      // defcal measure
      { ASTTypeDefcalMeasureCall,
        { "DMC", 3 }, },                     // defcal measure call
      { ASTTypeDefcalReset,
        { "DR", 2 }, },                      // defcal reset
      { ASTTypeDefcalResetCall,
        { "DRC", 3 }, },                     // defcal reset call
      { ASTTypeDefcalGrammar,
        { "DGM", 3 }, },                     // defcal grammar
      { ASTTypeDefcalGroup,
        { "DGR", 3 }, },                     // defcal group
      { ASTTypeDelay, { "Dl", 2 }, },        // delay
      { ASTTypeDuration, { "Du", 2 }, },     // duration
      { ASTTypeDurationOf, { "DuO", 3 }, },  // durationof
      { ASTTypeBox, { "Bo", 2 }, },          // box
      { ASTTypeBoxAs, { "BoA", 3 }, },       // boxas
      { ASTTypeBoxTo, { "BoT", 3 }, },       // boxas
      { ASTTypeBarrier, { "Bar", 3 }, },     // barrier
      { ASTTypeBinaryOp, { "BOp", 3 }, },    // binary op
      { ASTTypeUnaryOp, { "UOp", 3 }, },     // unary op
      { ASTTypeArray, { "A", 1 }, },         // # [<number> '_' <type> '_'] array
                                             // # [<number> '_' <type> <number> '_'] array
      { ASTTypeInputModifier,
        { "Im", 2 }, },                      // input modifier
      { ASTTypeOutputModifier,
        { "Om", 2 }, },                      // output modifier
      { ASTTypeOpaque, { "Oq", 2 }, },       // opaque qualifier
      { ASTTypeOpTy, { "OX", 2 }, },         // operator type
      { ASTTypeOpndTy, { "OY", 2 }, },       // operand type
      { ASTTypeLength, { "Lh", 2 }, },       // length
      { ASTTypeLengthOf, { "LhO", 3 }, },    // lengthof
      { ASTTypeStretch, { "S", 1 }, },       // stretch
      { ASTTypeReset, { "Rs", 2 }, },        // reset
      { ASTTypeReturn, { "Rt", 2 }, },       // return type
      { ASTTypeResult, { "R", 1 }, },        // result type
      { ASTTypeEnum, { "Te", 2 }, },         // enum type
      { ASTTypeTimeUnit, { "TU", 2 }, },     // time unit (duration)
      { ASTTypeMeasure, { "Mj", 2 }, },      // measure
      { ASTTypeGPhaseExpression,
        { "GPh", 3 }, },                     // gphase
      { ASTTypeGateGPhaseExpression,
        { "GGPh", 4 }, },                    // gphase
      { ASTTypeStringLiteral,
        { "L", 1 }, },                       // literal (string or numeric)
      { ASTTypeOpenPulseCalibration,
        { "OPC", 3 }, },                     // OpenPulse calibration
      { ASTTypeOpenPulseFrame,
        { "OPF", 3 }, },                     // OpenPulse frame
      { ASTTypeOpenPulsePlay,
        { "OPP", 3 }, },                     // OpenPulse play
      { ASTTypeOpenPulsePort,
        { "OPR", 3 }, },                     // OpenPulse port
      { ASTTypeOpenPulseWaveform,
        { "OPW", 3 }, },                     // OpenPulse waveform
      { ASTTypeUndefined, { "none", 4 }, },  // unknown / undefined
    };
  }

  if (ODMM.empty()) {
    ODMM = {
      { ASTOpTypeAdd, { "pl", 2 }, },               // plus (add)
      { ASTOpTypePositive, { "ps", 2 }, },          // plus (positive)
      { ASTOpTypeNegative, { "ng", 2 }, },          // minus (negative)
      { ASTOpTypeSub, { "mi", 2 }, },               // minus (subtract)
      { ASTOpTypeMul, { "ml", 2 }, },               // multiply
      { ASTOpTypeDiv, { "dv", 2 }, },               // divide
      { ASTOpTypeMod, { "rm", 2 }, },               // modulo
      { ASTOpTypeNegate, { "nt", 2 }, },            // logical negate
      { ASTOpTypeLogicalNot, { "nt", 2 }, },        // logical negate
      { ASTOpTypeBitAnd, { "an", 2 }, },            // bitwise and
      { ASTOpTypeBitOr, { "or", 2 }, },             // bitwise or
      { ASTOpTypeXor, { "eo", 2 }, },               // xor
      { ASTOpTypeAssign, { "aS", 2 }, },            // assignment
      { ASTOpTypeAddAssign, { "pL", 2 }, },         // add self-assign
      { ASTOpTypeSubAssign, { "mI", 2 }, },         // subtract self-assign
      { ASTOpTypeMulAssign, { "mL", 2 }, },         // multiply self-assign
      { ASTOpTypeDivAssign, { "dV", 2 }, },         // divide self-assign
      { ASTOpTypeModAssign, { "rM", 2 }, },         // modulo self-assign
      { ASTOpTypeBitAndAssign, { "aN", 2 }, },      // bitwise and self-assign
      { ASTOpTypeBitOrAssign, { "oR", 2 }, },       // bitwise or self-assign
      { ASTOpTypeXorAssign, { "eO", 2 }, },         // xor self-assign
      { ASTOpTypeLeftShift, { "ls", 2 }, },         // left shift
      { ASTOpTypeRightShift, { "rs", 2 }, },        // right shift
      { ASTOpTypeLeftShiftAssign, { "lS", 2 }, },   // left shift self-assign
      { ASTOpTypeRightShiftAssign, { "rS", 2 }, },  // right shift self-assign
      { ASTOpTypeCompEq, { "eq", 2 }, },            // equality comparison
      { ASTOpTypeCompNeq, { "ne", 2 }, },           // inequality comparison
      { ASTOpTypeLT, { "lt", 2 }, },                // less than
      { ASTOpTypeGT, { "gt", 2 }, },                // greater than
      { ASTOpTypeLE, { "le", 2 }, },                // less than or equal
      { ASTOpTypeGE, { "ge", 2 }, },                // greater than or equal
      { ASTOpTypeLogicalNot, { "nt", 2 }, },        // logical not
      { ASTOpTypeLogicalAnd, { "aa", 2 }, },        // logical and
      { ASTOpTypeLogicalOr, { "oo", 2 }, },         // logical or
      { ASTOpTypePreInc, { "_pp", 3 }, },           // pre-increment
      { ASTOpTypePreDec, { "_mm", 3 }, },           // pre-decrement
      { ASTOpTypePostInc, { "pp_", 3 }, },          // post-increment
      { ASTOpTypePostDec, { "mm_", 3 }, },          // post-decrement
      { ASTOpTypeSin, { "Ts", 2 }, },               // sin
      { ASTOpTypeCos, { "Tc", 2 }, },               // cos
      { ASTOpTypePow, { "Tp", 2 }, },               // pow
      { ASTOpTypeTan, { "Tt", 2 }, },               // tan
      { ASTOpTypeArcSin, { "Tas", 3 }, },           // arcsin
      { ASTOpTypeArcCos, { "Tac", 3 }, },           // arccos
      { ASTOpTypeArcTan, { "Tat", 3 }, },           // arctan
      { ASTOpTypeExp, { "Tex", 3 }, },              // exp
      { ASTOpTypeLn, { "Tln", 3 }, },               // ln
      { ASTOpTypeSqrt, { "Tsq", 3 }, },             // sqrt
      { ASTOpTypeRotation, { "Rot", 3 }, },         // rotation
      { ASTOpTypeRotl, { "Rtl", 3 }, },             // rotate left
      { ASTOpTypeRotr, { "Rtr", 3 }, },             // rotate right
      { ASTOpTypePopcount, { "Ppc", 3 }, },         // popcount
      { ASTOpTypeBinaryLeftFold, { "fL", 2 }, },    // binary left fold
      { ASTOpTypeBinaryRightFold, { "fR", 2 }, },   // binary right fold
      { ASTOpTypeUnaryLeftFold, { "fl", 2 }, },     // unary left fold
      { ASTOpTypeUnaryRightFold, { "fr", 2 }, },    // unary right fold
      { ASTOpTypeNone, { "Non", 3 }, },             // OpType None
    };
  }
}

void ASTMangler::Type(ASTType Ty) {
  S << TDMM[Ty].Token();
}

void ASTMangler::ConstType(ASTType Ty) {
  S << TDMM[ASTTypeConst].Token();
  Type(Ty);
}

void ASTMangler::TypeSize(ASTType Ty, unsigned SZ) {
  switch (Ty) {
  case ASTTypeBool:
  case ASTTypeInt:
  case ASTTypeUInt:
  case ASTTypeFloat:
  case ASTTypeDouble:
  case ASTTypeLongDouble:
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
    S << TDMM[Ty].Token() << '_' << SZ << '_';
    break;
  case ASTTypeMPDecimal:
    switch (SZ) {
    case 32:
      S << TDMM[Ty].Token() << 'f' << '_';
      break;
    case 64:
      S << TDMM[Ty].Token() << 'd' << '_';
      break;
    case 128:
      S << TDMM[Ty].Token() << 'e' << '_';
      break;
    default:
      S << TDMM[Ty].Token() << 'F' << SZ << '_';
      break;
    }
    break;
  case ASTTypeAngle:
  case ASTTypeBitset:
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias:
  case ASTTypeCReg:
  case ASTTypeQReg:
  case ASTTypeHash:
    S << TDMM[Ty].Token() << SZ << '_';
    break;
  case ASTTypeMPComplex:
    S << TDMM[Ty].Token() << SZ << '_';
    break;
  case ASTTypeArray:
  case ASTTypeCBitArray:
  case ASTTypeCBitNArray:
  case ASTTypeQubitArray:
  case ASTTypeQubitNArray:
  case ASTTypeAngleArray:
  case ASTTypeBoolArray:
  case ASTTypeIntArray:
  case ASTTypeMPIntegerArray:
  case ASTTypeFloatArray:
  case ASTTypeMPDecimalArray:
  case ASTTypeMPComplexArray:
  case ASTTypeLengthArray:
  case ASTTypeDurationArray:
  case ASTTypeOpenPulseFrameArray:
  case ASTTypeOpenPulsePortArray:
    S << TDMM[Ty].Token() << SZ << '_';
    break;
  default: {
    std::stringstream M;
    M << "Type " << PrintTypeEnum(Ty) << " does not require a size.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
  }
    break;
  }
}

void ASTMangler::ConstTypeSize(ASTType Ty, unsigned SZ) {
  S << TDMM[ASTTypeConst].Token();
  TypeSize(Ty, SZ);
}

void ASTMangler::MangleMPComplex(ASTType Ty, unsigned CS, unsigned TS) {
  switch (Ty) {
  case ASTTypeInt:
    if (TS <= 32U)
      S << TDMM[ASTTypeMPComplex].Token() << CS << "i32";
    else
      S << TDMM[ASTTypeMPComplex].Token() << CS << "II" << TS;
    break;
  case ASTTypeUInt:
    if (TS <= 32U)
      S << TDMM[ASTTypeMPComplex].Token() << CS << "j32";
    else
      S << TDMM[ASTTypeMPComplex].Token() << CS << "JJ" << TS;
    break;
  case ASTTypeMPInteger:
    S << TDMM[ASTTypeMPComplex].Token() << CS << "II" << TS;
    break;
  case ASTTypeMPUInteger:
    S << TDMM[ASTTypeMPComplex].Token() << CS << "JJ" << TS;
    break;
  case ASTTypeFloat:
    if (TS <= 32U)
      S << TDMM[ASTTypeMPComplex].Token() << CS << "f32";
    else
      S << TDMM[ASTTypeMPComplex].Token() << CS << 'f' << TS;
    break;
  case ASTTypeDouble:
  case ASTTypeLongDouble:
      S << TDMM[ASTTypeMPComplex].Token() << CS << 'f' << TS;
    break;
  case ASTTypeMPDecimal:
  case ASTTypeMPComplex:
      S << TDMM[ASTTypeMPComplex].Token() << CS << 'f' << TS;
    break;
  case ASTTypeBinaryOp:
      S << TDMM[ASTTypeMPComplex].Token() << CS << 'f' << TS;
    break;
  case ASTTypeUnaryOp:
      S << TDMM[ASTTypeMPComplex].Token() << CS << 'f' << TS;
    break;
  default: {
    std::stringstream M;
    M << "Impossible mangling of complex type instantiated from type "
      << PrintTypeEnum(Ty) << '.';
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
  }
    break;
  }
}

void ASTMangler::MangleConstMPComplex(ASTType Ty, unsigned CS, unsigned TS) {
  S << TDMM[ASTTypeConst].Token();
  MangleMPComplex(Ty, CS, TS);
}

void ASTMangler::Array(ASTType Ty, unsigned AS) {
  if (ASTExpressionValidator::Instance().CanBeArrayType(Ty)) {
    S << TDMM[ASTTypeArray].Token() << AS << TDMM[Ty].Token();
  } else {
    std::stringstream M;
    M << "Type " << PrintTypeEnum(Ty) << " cannot be used to "
      << "instantiate arrays.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
  }
}

void ASTMangler::ConstArray(ASTType Ty, unsigned AS) {
  S << TDMM[ASTTypeConst].Token();
  Array(Ty, AS);
}

void ASTMangler::Array(ASTType Ty, unsigned AS, unsigned TS) {
  if (ASTExpressionValidator::Instance().IsArbitraryWidthType(Ty)) {
    S << TDMM[ASTTypeArray].Token() << AS << TDMM[Ty].Token() << TS
      << '_';
  } else {
    std::stringstream M;
    M << "Type " << PrintTypeEnum(Ty) << " cannot be used to "
      << "instantiate arrays.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
  }
}

void ASTMangler::ConstArray(ASTType Ty, unsigned AS, unsigned TS) {
  S << TDMM[ASTTypeConst].Token();
  Array(Ty, AS, TS);
}

void ASTMangler::ComplexArray(unsigned AS, unsigned CS,
                              ASTType ETy, unsigned TS) {
  S << TDMM[ASTTypeArray].Token() << AS << TDMM[ASTTypeMPComplex].Token()
    << CS << TDMM[ETy].Token() << TS << '_';
}

void ASTMangler::ConstComplexArray(unsigned AS, unsigned CS,
                                   ASTType ETy, unsigned TS) {
  S << TDMM[ASTTypeConst].Token();
  ComplexArray(AS, CS, ETy, TS);
}

void ASTMangler::RValueRef(ASTType Ty) {
  switch (Ty) {
  case ASTTypeBool:
  case ASTTypeInt:
  case ASTTypeUInt:
  case ASTTypeFloat:
  case ASTTypeDouble:
  case ASTTypeLongDouble:
  case ASTTypeBitset:
  case ASTTypeQubit:
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias:
  case ASTTypeDuration:
  case ASTTypeLength:
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
  case ASTTypeMPDecimal:
  case ASTTypeMPComplex:
  case ASTTypeOpenPulseFrame:
    S << TDMM[ASTTypeRValueReference].Token() << TDMM[Ty].Token();
    break;
  default:
    std::stringstream M;
    M << "Type " << PrintTypeEnum(Ty) << " cannot be used to "
      << "instantiate a rvalue reference.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
    break;
  }
}

void ASTMangler::ConstRValueRef(ASTType Ty) {
  S << TDMM[ASTTypeConst].Token();
  RValueRef(Ty);
}

void ASTMangler::LValueRef(ASTType Ty) {
  switch (Ty) {
  case ASTTypeBool:
  case ASTTypeInt:
  case ASTTypeUInt:
  case ASTTypeFloat:
  case ASTTypeDouble:
  case ASTTypeLongDouble:
  case ASTTypeBitset:
  case ASTTypeQubit:
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias:
  case ASTTypeDuration:
  case ASTTypeLength:
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
  case ASTTypeMPDecimal:
  case ASTTypeMPComplex:
  case ASTTypeOpenPulseFrame:
    S << TDMM[ASTTypeLValueReference].Token() << TDMM[Ty].Token();
    break;
  default:
    std::stringstream M;
    M << "Type " << PrintTypeEnum(Ty) << " cannot be used to "
      << "instantiate a lvalue reference.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
    break;
  }
}

void ASTMangler::ConstLValueRef(ASTType Ty) {
  S << TDMM[ASTTypeConst].Token();
  LValueRef(Ty);
}

void ASTMangler::TypeIdentifier(ASTType Ty, unsigned TS, const std::string& Id) {
  assert(!Id.empty() && "Invalid Identifier argument!");

  switch (Ty) {
  case ASTTypeAngle:
  case ASTTypeBitset:
  case ASTTypeCReg:
  case ASTTypeQReg:
  case ASTTypeQubit:
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias:
  case ASTTypeGateQubitParam:
  case ASTTypeBool:
  case ASTTypeInt:
  case ASTTypeUInt:
  case ASTTypeFloat:
  case ASTTypeDouble:
  case ASTTypeLongDouble:
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
  case ASTTypeMPDecimal:
  case ASTTypeMPComplex:
  case ASTTypeResult:
  case ASTTypeOpenPulseFrame:
  case ASTTypeOpenPulsePort:
  case ASTTypeOpenPulseWaveform:
    S << TDMM[Ty].Token() << TS << '_' << Id.length() << Id;
    break;
  default:
    std::stringstream M;
    M << "Type " << PrintTypeEnum(Ty) << " is not sizeable.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::ICE);
    break;
  }
}

void ASTMangler::TypeIdentifier(ASTType Ty, const std::string& Id) {
  assert(!Id.empty() && "Invalid Identifier argument!");
  S << TDMM[Ty].Token() << Id.length() << Id;
}

void ASTMangler::ConstTypeIdentifier(ASTType Ty, unsigned TS, const std::string& Id) {
  S << TDMM[ASTTypeConst].Token();
  TypeIdentifier(Ty, TS, Id);
}

void ASTMangler::ConstTypeIdentifier(ASTType Ty, const std::string& Id) {
  S << TDMM[ASTTypeConst].Token();
  TypeIdentifier(Ty, Id);
}

void ASTMangler::OpIdentifier(ASTOpType OTy) {
  S << std::strlen(ODMM[OTy].Token()) << ASTMangler::ODMM[OTy].Token();
}

void ASTMangler::StringLiteral(const std::string& SL) {
  S << 'L' << TDMM[ASTTypeArray].Token() << TDMM[ASTTypeUTF8].Token()
    << TDMM[ASTTypeConst].Token() << SL.length() << SL.c_str()
    << 'E';
}

void ASTMangler::StringLiteral(const char* SL) {
  if (SL)
    StringLiteral(std::string(SL));
}

void ASTMangler::String() {
  S << TDMM[ASTTypeArray].Token() << TDMM[ASTTypeUTF8].Token() << 'E';
}

void ASTMangler::ConstString() {
  S << TDMM[ASTTypeArray].Token() << TDMM[ASTTypeUTF8].Token()
    << TDMM[ASTTypeConst].Token() << 'E';
}

void ASTMangler::NumericLiteral(int32_t NL) {
  if (NL >= 0) {
    S << 'L' << TDMM[ASTTypeInt].Token() << sizeof(int32_t) * CHAR_BIT
      << '_' << NL << 'E';
  } else {
    S << 'L' << TDMM[ASTTypeInt].Token()
      << sizeof(int32_t) * CHAR_BIT << "n_" << std::abs(NL) << 'E';
  }
}

void ASTMangler::NumericLiteral(uint32_t NL) {
  S << 'L' << TDMM[ASTTypeUInt].Token() << sizeof(uint32_t) * CHAR_BIT
    << '_' << NL << 'E';
}

void ASTMangler::NumericLiteral(int64_t NL) {
  if (NL >= 0) {
    S << 'L' << TDMM[ASTTypeInt].Token() << sizeof(int64_t) * CHAR_BIT
      << '_' << NL << 'E';
  } else {
    S << 'L' << TDMM[ASTTypeInt].Token()
      << sizeof(int64_t) * CHAR_BIT << "n_" << std::abs(NL) << 'E';
  }
}

void ASTMangler::NumericLiteral(uint64_t NL) {
  S << 'L' << TDMM[ASTTypeUInt].Token() << sizeof(uint64_t) * CHAR_BIT
    << '_' << NL << 'E';
}

void ASTMangler::NumericLiteral(const ASTMPIntegerNode* MPI) {
  assert(MPI && "Invalid ASTMPIntegerNode argument!");

  if (MPI->IsSigned()) {
    if (MPI->IsNegative()) {
      std::string NS = MPI->GetValue();
      // Remove leading '-' for numeric literal encoding.
      NS = NS.substr(1, std::string::npos);

      S << 'L' << TDMM[ASTTypeMPInteger].Token()
        << MPI->GetBits() << "n_" << NS << 'E';
    } else {
      S << 'L' << TDMM[ASTTypeMPInteger].Token() << MPI->GetBits()
        << '_' << MPI->GetValue() << 'E';
    }
  } else {
    S << 'L' << TDMM[ASTTypeMPUInteger].Token() << MPI->GetBits()
      << '_' << MPI->GetValue() << 'E';
  }
}

void ASTMangler::NumericLiteral(float NL) {
  if (std::signbit(NL))
    S << 'L' << TDMM[ASTTypeFloat].Token()
      << sizeof(float) * CHAR_BIT << "n_" << NL << 'E';
  else
    S << 'L' << TDMM[ASTTypeFloat].Token()
      << sizeof(float) * CHAR_BIT << '_' << NL << 'E';
}

void ASTMangler::NumericLiteral(double NL) {
  if (std::signbit(NL))
    S << 'L' << TDMM[ASTTypeDouble].Token()
      << sizeof(double) * CHAR_BIT << "n_" << NL << 'E';
  else
    S << 'L' << TDMM[ASTTypeDouble].Token()
      << sizeof(double) * CHAR_BIT << '_' << NL << 'E';
}

void ASTMangler::NumericLiteral(long double NL) {
  if (std::signbit(NL))
    S << 'L' << TDMM[ASTTypeLongDouble].Token()
      << sizeof(long double) * CHAR_BIT << "n_"
      << NL << 'E';
  else
    S << 'L' << TDMM[ASTTypeLongDouble].Token()
      << sizeof(long double) * CHAR_BIT << '_'
      << NL << 'E';
}

void ASTMangler::NumericLiteral(const ASTMPDecimalNode* MPD) {
  assert(MPD && "Invalid ASTMPDecimalNode argument!");

  std::string MDS;
  if (MPD->IsNan())
    MDS = "nan";
  else if (MPD->IsInf())
    MDS = "inf";
  else {
    MDS = MPD->GetValue();
    ASTStringUtils::Instance().RemoveTrailingZeros(MDS);
  }

  if (MPD->IsNegative()) {
    // Remove leading '-' for numeric literal negative encoding.
    MDS = MDS.substr(1, std::string::npos);
    S << 'L' << TDMM[ASTTypeMPDecimal].Token()
      << MPD->GetBits() << "n_" << MDS << 'E';
  } else {
    S << 'L' << TDMM[ASTTypeMPDecimal].Token()
      << MPD->GetBits() << '_' << MDS << 'E';
  }
}

void ASTMangler::MangleMPDecimal(const ASTMPDecimalNode* MPD) {
  assert(MPD && "Invalid ASTMPDecimalNode argument!");

  switch (MPD->GetBits()) {
  case 32:
    S << TDMM[ASTTypeMPDecimal].Token() << 'f' << MPD->GetBits()
        << '_' << MPD->GetName().length() << MPD->GetName() << 'E';
    break;
  case 64:
    S << TDMM[ASTTypeMPDecimal].Token() << 'd' << MPD->GetBits()
      << '_' << MPD->GetName().length() << MPD->GetName() << 'E';
    break;
  case 128:
    S << TDMM[ASTTypeMPDecimal].Token() << 'e' << MPD->GetBits()
      << '_' << MPD->GetName().length() << MPD->GetName() << 'E';
    break;
  default:
    S << TDMM[ASTTypeMPDecimal].Token() << 'F' << MPD->GetBits()
      << '_' << MPD->GetName().length() << MPD->GetName() << 'E';
    break;
  }
}

void ASTMangler::MangleConstMPDecimal(const ASTMPDecimalNode* MPD) {
  S << TDMM[ASTTypeConst].Token();
  MangleMPDecimal(MPD);
}

void ASTMangler::NumericLiteral(const ASTMPComplexNode* MPC) {
  const ASTMPDecimalNode* MPDR = MPC->GetRealAsMPDecimal();
  const ASTMPDecimalNode* MPDI = MPC->GetImagAsMPDecimal();
  assert(MPDR && "Invalid real ASTMPDecimalNode!");
  assert(MPDI && "Invalid imaginary ASTMPDecimalNode!");

  S << 'L' << TDMM[ASTTypeMPCReal].Token();
  NumericLiteral(MPDR);
  S << ODMM[MPC->GetOpType()].Token();
  S << 'L' << TDMM[ASTTypeMPCImag].Token();
  NumericLiteral(MPDI);
  S << TDMM[ASTTypeImaginary].Token() << 'E';
}

void ASTMangler::Array(unsigned AS, ASTType Ty) {
  S << TDMM[ASTTypeArray].Token() << AS << '_' << TDMM[Ty].Token()
    << '_';
}

void ASTMangler::Array(unsigned AS, ASTType Ty, unsigned TS) {
  S << TDMM[ASTTypeArray].Token() << AS << '_' << TDMM[Ty].Token()
    << TS << '_';
}

void ASTMangler::OpType(ASTOpType OTy) {
  S << ODMM[OTy].Token();
}

std::string ASTMangler::MangleIdentifier(const ASTIdentifierNode* Id) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->GetSymbolType() == ASTTypeUndefined)
    return std::string();

  ASTMangler M;
  M.Start();
  if (!Id->GetPolymorphicName().empty())
    M.TypeIdentifier(Id->GetSymbolType(), Id->GetBits(), Id->GetPolymorphicName());
  else
    M.TypeIdentifier(Id->GetSymbolType(), Id->GetBits(), Id->GetName());
  M.EndExpression();
  M.End();
  return M.AsString();
}

std::string ASTMangler::MangleIdentifier(const ASTIdentifierRefNode* Id) {
  assert(Id && "Invalid ASTIdentifierNode argument!");

  if (Id->GetSymbolType() == ASTTypeUndefined)
    return std::string();

  ASTMangler M;
  M.Start();

   if (!Id->GetPolymorphicName().empty()) {
     switch (Id->GetSymbolType()) {
     case ASTTypeBool:
     case ASTTypeInt:
     case ASTTypeUInt:
     case ASTTypeFloat:
     case ASTTypeDouble:
     case ASTTypeLongDouble:
     case ASTTypeLength:
     case ASTTypeDuration:
     case ASTTypeGate:
     case ASTTypeDefcal:
     case ASTTypeStretch:
     case ASTTypeBox:
     case ASTTypeBoxAs:
     case ASTTypeBoxTo:
     case ASTTypePragma:
     case ASTTypeEllipsis:
     case ASTTypeOpenPulseFrame:
     case ASTTypeOpenPulsePort:
     case ASTTypeOpenPulseWaveform:
     case ASTTypeOpenPulseCalibration:
       M.TypeIdentifier(Id->GetSymbolType(), Id->GetPolymorphicName());
       break;
     default:
       M.TypeIdentifier(Id->GetSymbolType(), Id->GetBits(),
                        Id->GetPolymorphicName());
       break;
     }
  } else {
    switch (Id->GetSymbolType()) {
     case ASTTypeBool:
     case ASTTypeInt:
     case ASTTypeUInt:
     case ASTTypeFloat:
     case ASTTypeDouble:
     case ASTTypeLongDouble:
     case ASTTypeLength:
     case ASTTypeDuration:
     case ASTTypeGate:
     case ASTTypeDefcal:
     case ASTTypeStretch:
     case ASTTypeBox:
     case ASTTypeBoxAs:
     case ASTTypeBoxTo:
     case ASTTypePragma:
     case ASTTypeEllipsis:
     case ASTTypeOpenPulseFrame:
     case ASTTypeOpenPulsePort:
     case ASTTypeOpenPulseWaveform:
     case ASTTypeOpenPulseCalibration:
       M.TypeIdentifier(Id->GetSymbolType(), Id->GetName());
       break;
     default:
       M.TypeIdentifier(Id->GetSymbolType(), Id->GetBits(), Id->GetName());
       break;
    }
  }

  M.EndExpression();
  M.End();
  return M.AsString();
}

void ASTDemangler::Init() {
  if (TDMM.empty()) {
    TDMM = {
      { ASTTypeBool, { "bool", 4 }, },
      { ASTTypeChar, { "char", 4 }, },
      { ASTTypeUTF8, { "char", 4 }, },
      { ASTTypeULong, { "unsigned long", 13 }, },
      { ASTTypeInt, { "int", 3 }, },
      { ASTTypeUInt, { "uint", 4 }, },
      { ASTTypeFloat, { "float", 5 }, },
      { ASTTypeDouble, { "double", 6 }, },
      { ASTTypeLong, { "long", 4 }, },
      { ASTTypeLongDouble, { "longdouble", 10 }, },
      { ASTTypeVoid, { "void", 4 }, },
      { ASTTypeEllipsis, { "ellipsis", 8 }, },
      { ASTTypePragma, { "pragma", 6 }, },
      { ASTTypePointer, { "pointer", 7 }, },
      { ASTTypeLValueReference, { "&", 1 }, },
      { ASTTypeRValueReference, { "&&", 2 }, },
      { ASTTypePopcountExpr, { "popcount", 8 }, },
      { ASTTypeMPInteger, { "int", 3 }, },
      { ASTTypeMPUInteger, { "uint", 4 }, },
      { ASTTypeMPDecimal, { "float", 5 }, },
      { ASTTypeFunction, { "def", 3 }, },
      { ASTTypeFunctionDeclaration, { "def", 3 }, },
      { ASTTypeFunctionCall, { "functioncall", 12 }, },
      { ASTTypeExtern, { "extern", 6 }, },
      { ASTTypeKernel, { "kernel", 6 }, },
      { ASTTypeKernelCall, { "kernelcall", 10 }, },
      { ASTTypeKernelDeclaration, { "extern", 6 }, },
      { ASTTypeAngle, { "angle", 5 }, },
      { ASTTypeMPComplex, { "complex", 7 }, },
      { ASTTypeMPCReal, { "creal", 5 }, },
      { ASTTypeMPCImag, { "cimag", 5 }, },
      { ASTTypeImaginary, { "im", 2 }, },
      { ASTTypeGate, { "gate", 4 }, },
      { ASTTypeGateCall, { "gatecall", 8 }, },
      { ASTTypeGateOpNode, { "gateopnode", 10 }, },
      { ASTTypeGateQOpNode, { "gateqopnode", 11 }, },
      { ASTTypeGateUOpNode, { "gateuopnode", 11 }, },
      { ASTTypeGateGenericOpNode, { "gategenericopnode", 17 }, },
      { ASTTypeGateHOpNode, { "gatehopnode", 11 }, },
      { ASTTypeHash, { "hash", 4 }, },
      { ASTTypeCXGateOpNode, { "cx", 2 }, },
      { ASTTypeCCXGateOpNode, { "ccx", 3 }, },
      { ASTTypeCNotGateOpNode, { "cnot", 4 }, },
      { ASTTypeGateControl, { "ctrl@", 5 }, },
      { ASTTypeGateInverse, { "inv@", 4 }, },
      { ASTTypeGateNegControl, { "neg@", 4 }, },
      { ASTTypeGatePower, { "pow@", 4 }, },
      { ASTTypeQubit, { "qubit", 5 }, },
      { ASTTypeBoundQubit, { "boundqubit", 10 }, },
      { ASTTypeUnboundQubit, { "unboundqubit", 12 }, },
      { ASTTypeQubitContainer, { "qubitcontainer", 14 }, },
      { ASTTypeQubitContainerAlias, { "qubitcontaineralias", 19 }, },
      { ASTTypeGateQubitParam, { "gatequbitparam", 15 }, },
      { ASTTypeQReg, { "qreg", 4 }, },
      { ASTTypeCReg, { "creg", 4 }, },
      { ASTTypeConst, { "const", 5 }, },
      { ASTTypeBitset, { "bit", 3 }, },
      { ASTTypeDefcal, { "defcal", 6 }, },
      { ASTTypeDefcalCall, { "defcalcall", 10 }, },
      { ASTTypeDefcalMeasure, { "defcalmeasure", 13 }, },
      { ASTTypeDefcalMeasureCall, { "defcalmeasurecall", 17 }, },
      { ASTTypeDefcalReset, { "defcalreset", 11 }, },
      { ASTTypeDefcalResetCall, { "defcalresetcall", 15 }, },
      { ASTTypeDefcalGrammar, { "defcalgrammar", 13 }, },
      { ASTTypeDelay, { "delay", 5 }, },
      { ASTTypeDuration, { "duration", 8 }, },
      { ASTTypeDurationOf, { "durationof", 10 }, },
      { ASTTypeBox, { "box", 3 }, },
      { ASTTypeBoxAs, { "boxas", 5 }, },
      { ASTTypeBoxTo, { "boxto", 5 }, },
      { ASTTypeBarrier, { "barrier", 7 }, },
      { ASTTypeBinaryOp, { "binaryop", 8 }, },
      { ASTTypeUnaryOp, { "unaryop", 7 }, },
      { ASTTypeArray, { "array", 5 }, },
      { ASTTypeInputModifier, { "input", 5 }, },
      { ASTTypeOutputModifier, { "output", 6 }, },
      { ASTTypeOpaque, { "opaque", 6 }, },
      { ASTTypeOpTy, { "operator", 8 }, },
      { ASTTypeOpndTy, { "operand", 7 }, },
      { ASTTypeLength, { "length", 6 }, },
      { ASTTypeLengthOf, { "lengthof", 8 }, },
      { ASTTypeStretch, { "stretch", 7 }, },
      { ASTTypeReset, { "reset", 5 }, },
      { ASTTypeReturn, { "return", 6 }, },
      { ASTTypeResult, { "result", 6 }, },
      { ASTTypeEnum, { "enum", 4 }, },
      { ASTTypeMeasure, { "measure", 7 }, },
      { ASTTypeGPhaseExpression, { "gphase", 6 }, },
      { ASTTypeStringLiteral, { "stringliteral", 13 }, },
      { ASTTypeOpenPulseCalibration, { "calibration", 11 }, },
      { ASTTypeOpenPulseFrame, { "frame", 5 }, },
      { ASTTypeOpenPulsePlay, { "play", 4 }, },
      { ASTTypeOpenPulsePort, { "port", 4 }, },
      { ASTTypeOpenPulseWaveform, { "waveform", 8 }, },
    };
  }

  if (ODMM.empty()) {
    ODMM = {
      { ASTOpTypeAdd, { "+", 1 }, },
      { ASTOpTypePositive, { "+", 1 }, },
      { ASTOpTypeSub, { "-", 1 }, },
      { ASTOpTypeNegate, { "!", 1 }, },
      { ASTOpTypeLogicalNot, { "!", 1 }, },
      { ASTOpTypeNegative, { "-", 1 }, },
      { ASTOpTypeMul, { "*", 1 }, },
      { ASTOpTypeDiv, { "/", 1 }, },
      { ASTOpTypeMod, { "%", 1 }, },
      { ASTOpTypeBitAnd, { "&", 1 }, },
      { ASTOpTypeBitOr, { "|", 1 }, },
      { ASTOpTypeXor, { "^", 1 }, },
      { ASTOpTypeAssign, { "=", 1 }, },
      { ASTOpTypeAddAssign, { "+=", 2 }, },
      { ASTOpTypeSubAssign, { "-=", 2 }, },
      { ASTOpTypeMulAssign, { "*=", 2 }, },
      { ASTOpTypeDivAssign, { "/=", 2 }, },
      { ASTOpTypeModAssign, { "%=", 2 }, },
      { ASTOpTypeBitAndAssign, { "&=", 2 }, },
      { ASTOpTypeBitOrAssign, { "|=", 2 }, },
      { ASTOpTypeXorAssign, { "^=", 2 }, },
      { ASTOpTypeLeftShift, { "<<", 2 }, },
      { ASTOpTypeRightShift, { ">>", 2 }, },
      { ASTOpTypeLeftShiftAssign, { "<<=", 3 }, },
      { ASTOpTypeRightShiftAssign, { ">>=", 3 }, },
      { ASTOpTypeCompEq, { "==", 2 }, },
      { ASTOpTypeCompNeq, { "!=", 2 }, },
      { ASTOpTypeLT, { "<", 1 }, },
      { ASTOpTypeGT, { ">", 1 }, },
      { ASTOpTypeLE, { "<=", 2 }, },
      { ASTOpTypeGE, { ">=", 2 }, },
      { ASTOpTypeLogicalNot, { "!", 1 }, },
      { ASTOpTypeLogicalAnd, { "&&", 2 }, },
      { ASTOpTypeLogicalOr, { "||", 2 }, },
      { ASTOpTypePreInc, { "++", 2 }, },
      { ASTOpTypePreDec, { "--", 2 }, },
      { ASTOpTypePostInc, { "++", 2 }, },
      { ASTOpTypePostDec, { "--", 2 }, },
      { ASTOpTypeSin, { "sin", 3 }, },
      { ASTOpTypeCos, { "cos", 3 }, },
      { ASTOpTypeTan, { "tan", 3 }, },
      { ASTOpTypeArcSin, { "arcsin", 6 }, },
      { ASTOpTypeArcCos, { "arccos", 6 }, },
      { ASTOpTypeArcTan, { "arctan", 6 }, },
      { ASTOpTypeExp, { "exp", 3 }, },
      { ASTOpTypeLn, { "ln", 2 }, },
      { ASTOpTypeSqrt, { "sqrt", 4 }, },
      { ASTOpTypeRotation, { "", 0 }, },
      { ASTOpTypeRotl, { "rotl", 4 }, },
      { ASTOpTypeRotr, { "rotr", 4 }, },
      { ASTOpTypePopcount, { "popcount", 8 }, },
      { ASTOpTypeBinaryLeftFold, { "(", 1 }, },
      { ASTOpTypeBinaryRightFold, { ")", 1 }, },
      { ASTOpTypeUnaryLeftFold, { "(", 1 }, },
      { ASTOpTypeUnaryRightFold, { ")", 1 }, },
      { ASTOpTypeNone, { "", 0 }, },
    };

    if (TYMM.empty()) {
      TYMM = {
        { RXInt32Pos, std::regex("i32"), ASTTypeInt },
        { RXInt32Neg, std::regex("i32n"), ASTTypeInt, true },
        { RXUInt32, std::regex("j32"), ASTTypeUInt },
        { RXInt64Pos, std::regex("i64"), ASTTypeLong },
        { RXInt64Neg, std::regex("i64n"), ASTTypeLong, true },
        { RXUInt64, std::regex("j64"), ASTTypeULong },
        { RXMPIntPos, std::regex("II[0-9]+"), ASTTypeMPInteger },
        { RXMPIntNeg, std::regex("II[0-9]+n"), ASTTypeMPInteger, true },
        { RXMPUInt, std::regex("JJ[0-9]+"), ASTTypeMPUInteger },
        { RXFloat, std::regex("f32"), ASTTypeFloat },
        { RXDouble, std::regex("d64"), ASTTypeDouble },
        { RXLongDouble, std::regex("e128"), ASTTypeDouble },
        { RXFloatNeg, std::regex("f32n"), ASTTypeMPDecimal },
        { RXDoubleNeg, std::regex("d64n"), ASTTypeMPDecimal },
        { RXLongDoubleNeg, std::regex("e128n"), ASTTypeMPDecimal },
        { RXMPDecPos, std::regex("DD[0-9]+"), ASTTypeMPDecimal },
        { RXMPDecNeg, std::regex("DD[0-9]+n"), ASTTypeMPDecimal, true },
        { RXMPComplex, std::regex("C[0-9]+"), ASTTypeMPComplex },
        { RXString, std::regex("Au8[0-9]+"), ASTTypeStringLiteral },
        { RXConstString, std::regex("Au8k[0-9]+"), ASTTypeStringLiteral },
      };
    }

    if (PAMM.empty()) {
      PAMM = {
        { RXDa, std::regex("^Da[0-9]+"), },
        { RXDp, std::regex("^Dp[0-9]+"), },
        { RXFa, std::regex("^Fa[0-9]+"), },
        { RXFp, std::regex("^Fp[0-9]+"), },
        { RXGa, std::regex("^Ga[0-9]+"), },
        { RXGp, std::regex("^Gp[0-9]+"), },
        { RXKa, std::regex("^Ka[0-9]+"), },
        { RXKp, std::regex("^Kp[0-9]+"), },
      };
    }
  }
}

bool ASTDemangler::IsParamOrArgument(const char* S) const {
  if (S && *S) {
    switch (S[0]) {
    case 'D':
    case 'F':
    case 'G':
    case 'K':
      if ((S[1] == 'p' || S[1] == 'a') && std::isdigit(S[2]))
        return true;
      break;
    default:
      return false;
      break;
    }
  }

  return false;
}

bool ASTDemangler::IsParamOrArgument(ASTType Ty) const {
  switch (Ty) {
  case ASTTypeDefcalArg:
  case ASTTypeDefcalParam:
  case ASTTypeGateArg:
  case ASTTypeGateParam:
  case ASTTypeFuncArg:
  case ASTTypeFuncParam:
  case ASTTypeKernelArg:
  case ASTTypeKernelParam:
    return true;
    break;
  default:
    return false;
    break;
  }

  return false;
}

bool ASTDemangler::IsBinaryOrUnaryOp(const char* S) const {
  if (S && *S) {
    switch (S[0]) {
    case 'B':
      if (S[1] == 'O' && S[2] == 'p')
        return true;
      break;
    case 'U':
      if (S[1] == 'O' && S[2] == 'p')
        return true;
      break;
    default:
      return false;
      break;
    }
  }

  return false;
}

bool ASTDemangler::IsBinaryOrUnaryOp(ASTType Ty) const {
  switch (Ty) {
  case ASTTypeBinaryOp:
  case ASTTypeUnaryOp:
    return true;
    break;
  default:
    return false;
    break;
  }

  return false;
}

bool ASTDemangler::IsBinaryOp(const char* S) const {
  if (S && *S) {
    switch (S[0]) {
    case 'B':
      if (S[1] == 'O' && S[2] == 'p')
        return true;
      break;
    default:
      return false;
      break;
    }
  }

  return false;
}

bool ASTDemangler::IsUnaryOp(const char* S) const {
  if (S && *S) {
    switch (S[0]) {
    case 'U':
      if (S[1] == 'O' && S[2] == 'p')
        return true;
      break;
    default:
      return false;
      break;
    }
  }

  return false;
}

bool ASTDemangler::IsOperator(const char* S) {
  assert(S && "Invalid mangled string argument!");

  const char* R = ValidateEndOfExpression(S);
  if (!R) return false;

  switch (R[0]) {
  case 'a':
    if (R[1] == 'a' || R[1] == 'n' || R[1] == 'N' || R[1] == 'S')
      return true;
    break;
  case 'd':
    if (R[1] == 'v' || R[1] == 'V')
      return true;
    break;
  case 'e':
    if (R[1] == 'o' || R[1] == 'O' || R[1] == 'q')
      return true;
    break;
  case 'g':
    if (R[1] == 'e' || R[1] == 't')
      return true;
    break;
  case 'l':
    if (R[1] == 's' || R[1] == 'S')
      return true;
    break;
  case 'm':
    if (R[1] == 'l' || R[1] == 'i' || R[1] == 'L' || R[1] == 'I' ||
        (R[1] == 'm' && R[2] == '_'))
      return true;
    break;
  case 'n':
    if (R[1] == 'e' || R[1] == 'g' || R[1] == 't')
      return true;
    break;
  case 'N':
    if (R[1] == 'o' && R[2] == 'n')
      return true;
    break;
  case 'o':
    if (R[1] == 'o' || R[1] == 'r' || R[1] == 'R')
      return true;
    break;
  case 'p':
    if (R[1] == 'p' && R[2] == '_')
      return true;
    else if (R[1] == 'l' || R[1] == 'L')
      return true;
    break;
  case 'P':
    if (R[1] == 'p' && R[2] == 'c')
      return true;
    break;
  case 'r':
    if (R[1] == 'm' || R[1] == 'M' || R[1] == 's' || R[1] == 'S')
      return true;
    break;
  case 'R':
    if (R[1] == 'o' && R[2] == 't')
      return true;
    else if (R[1] == 't' && R[2] == 'l')
      return true;
    else if (R[1] == 't' && R[2] == 'r')
      return true;
    break;
  case 'T': {
    if (R[1] == 'a') {
      if (R[2] == 's') {
        return true;
      } else if (R[2] == 'c') {
        return true;
      } else if (R[2] == 't') {
        return true;
      }
    } else if (R[1] == 'e' && R[2] == 'x') {
      return true;
    } else if (R[1] == 'l' && R[2] == 'n') {
      return true;
    } else {
      if (R[2] == 's') {
        return true;
      } else if (R[2] == 'c') {
        return true;
      } else if (R[2] == 't') {
        return true;
      }
    }
  }
    break;
  case '_':
    if (R[1] == 'p' && R[2] == 'p')
      return true;
    else if (R[1] == 'm' && R[2] == 'm')
      return true;
    break;
  default:
    return false;
    break;
  }

  return false;
}

const char* ASTDemangler::ParseType(const char* N, ASTDemangled* DMP) {
  assert(DMP && "Invalid ASTDemangled argument!");

  if (!N) return N;

  const char* NP = nullptr;

  if (*N) {
    NP = N;
    unsigned J = 0U;

    switch (NP[0]) {
    case 'b':
      DMP->TD.Ty = ASTTypeBool;
      DMP->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
      NP += 1;
      NP = ParseName(NP, DMP);
      break;
    case 'c':
      DMP->TD.Ty = ASTTypeChar;
      DMP->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
      NP += 1;
      NP = ParseName(NP, DMP);
      break;
    case 'u':
      if (NP[1] == '8') {
        DMP->TD.Ty = ASTTypeUTF8;
        DMP->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
        NP += 2;
        NP = ParseName(NP, DMP);
      }
      break;
    case 'i':
      DMP->TD.Ty = ASTTypeInt;
      DMP->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
      if (DMP->TD.UBits == 32U)
        DMP->TD.V = 'i';
      else
        DMP->TD.V = 'I';
      NP += 1;
      NP = ParseName(NP, DMP);
      break;
    case 'j':
      DMP->TD.Ty = ASTTypeUInt;
      DMP->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
      if (DMP->TD.UBits == 32U)
        DMP->TD.V = 'u';
      else
        DMP->TD.V = 'U';
      NP += 1;
      NP = ParseName(NP, DMP);
      break;
    case 'x':
      DMP->TD.Ty = ASTTypeLong;
      DMP->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
      DMP->TD.V = 'I';
      NP += 1;
      NP = ParseName(NP, DMP);
      break;
    case 'y':
      DMP->TD.Ty = ASTTypeULong;
      DMP->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
      DMP->TD.V = 'U';
      NP += 1;
      NP = ParseName(NP, DMP);
      break;
    case 'f':
      DMP->TD.Ty = ASTTypeFloat;
      DMP->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
      DMP->TD.V = 'F';
      NP += 1;
      NP = ParseName(NP, DMP);
      break;
    case 'd':
      DMP->TD.Ty = ASTTypeDouble;
      DMP->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
      DMP->TD.V = 'D';
      NP += 1;
      NP = ParseName(NP, DMP);
      break;
    case 'p':
      if (NP[1] == 'C') {
        DMP->TD.Ty = ASTTypePopcountExpr;
        NP += 2;
      } else if (IsOperator(NP)) {
        std::vector<ASTDemangled*> VDM;
        NP = ParseOpType(NP, DMP, VDM);
      } else {
        DMP->TD.Ty = ASTTypePragma;
        NP += 1;
      }
      break;
    case 'P':
      // FIXME: ADD POINTEE TYPE TO DEPENDENT TYPE.
      DMP->TD.Ty = ASTTypePointer;
      DMP->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
      NP += 1;
      NP = ParseName(NP, DMP);
      break;
    case 'r':
      DMP->TD.S = 'S';
      DMP->TD.Ty = ASTTypeRotateExpr;
      if (NP[1] == 'L') {
        DMP->TD.OpTy = ASTOpTypeRotl;
        DMP->TD.SBits = ASTStringUtils::StringToSigned<int32_t>(&NP[4], '_', &J);
        DMP->TD.S = 'S';
        NP += J + 2;
      } else if (NP[1] == 'R') {
        DMP->TD.OpTy = ASTOpTypeRotr;
        DMP->TD.SBits = ASTStringUtils::StringToSigned<int32_t>(&NP[4], '_', &J);
        DMP->TD.S = 'S';
        NP += J + 2;
      } else {
        DMP->TD.OpTy = ASTOpTypeUndefined;
        DMP->TD.UBits = 0U;
        DMP->TD.S = 'U';
        NP += 1;
      }
      break;
    case 'v':
      DMP->TD.Ty = ASTTypeVoid;
      DMP->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
      NP += 1;
      NP = ParseName(NP, DMP);
      break;
    case 'z':
      DMP->TD.Ty = ASTTypeEllipsis;
      NP += 1;
      break;
    case 'H':
      DMP->TD.Ty = ASTTypeHash;
      DMP->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
      NP += 1;
      break;
    case 'R':
      if (NP[1] == 'V' && NP[2] == 'R') {
        DMP->TD.Ty = ASTTypeRValueReference;
        NP += 3;
      } else if (NP[1] == 'Q') {
        DMP->TD.Ty = ASTTypeQReg;
        NP += 2;
      } else if (NP[1] == 'B') {
        DMP->TD.Ty = ASTTypeCReg;
        NP += 2;
      } else if (NP[1] == 's') {
        DMP->TD.Ty = ASTTypeReset;
        NP += 2;
      } else if (NP[1] == 't') {
        DMP->TD.Ty = ASTTypeReturn;
        NP += 2;
      } else {
        // FIXME: ADD RESULT TYPE TO DEPENDENT TYPE.
        DMP->TD.Ty = ASTTypeResult;
        NP += 1;
      }
      break;
    case 'O':
      if (NP[1] == 'P') {
        if (NP[2] == 'c') {
          DMP->TD.Ty = ASTTypeOpenPulseCalibration;
          NP += 3;
        } else if (NP[2] == 'f') {
          DMP->TD.Ty = ASTTypeOpenPulseFrame;
          NP += 3;
        } else if (NP[2] == 'p') {
          // FIXME: ADD WAVEFORM TO DEPENDENT TYPE.
          DMP->TD.Ty = ASTTypeOpenPulsePlay;
          NP += 3;
        } else if (NP[2] == 'r') {
          DMP->TD.Ty = ASTTypeOpenPulsePort;
          NP += 3;
        } else if (NP[2] == 'w') {
          DMP->TD.Ty = ASTTypeOpenPulseWaveform;
          NP += 3;
        }
      } else if (NP[1] == 'm') {
        DMP->TD.Ty = ASTTypeOutputModifier;
        NP += 2;
      } else if (NP[1] == 'q') {
        DMP->TD.Ty = ASTTypeOpaque;
        NP += 2;
      } else {
        DMP->TD.Ty = ASTTypeRValueReference;
        NP += 1;
      }
      break;
    case 'I':
      if (NP[1] == 'I') {
        DMP->TD.Ty = ASTTypeMPInteger;
        DMP->DTD0.Ty = ASTTypeMPInteger;
        DMP->TD.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
        DMP->TD.S = 'U';
        NP += J + 3;
        NP = ParseName(NP, DMP);
      } else if (NP[1] == 'm') {
        DMP->TD.Ty = ASTTypeInputModifier;
        NP += 2;
      }
      break;
    case 'J':
      if (NP[1] == 'J') {
        DMP->TD.Ty = ASTTypeMPInteger;
        DMP->DTD0.Ty = ASTTypeMPUInteger;
        DMP->TD.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
        DMP->TD.S = 'U';
        NP += J + 3;
        NP = ParseName(NP, DMP);
      }
      break;
    case 'L':
      if (NP[1] == 'h') {
        if (NP[2] == 'O') {
          DMP->TD.Ty = ASTTypeLengthOf;
          NP += 3;
        } else {
          DMP->TD.Ty = ASTTypeLength;
          NP += 2;
        }
      } else if (NP[1] == 'V' && NP[2] == 'R') {
        DMP->TD.Ty = ASTTypeLValueReference;
        NP += 3;
      } else {
        bool Neg = false;
        NP = ResolveLiteral(NP, DMP->TD.Ty, DMP, Neg);
        NP += 1;
      }
      break;
    case 'C':
      if (NP[1] == 'X' && NP[2] == 'L') {
        DMP->TD.Ty = ASTTypeComplexList;
        DMP->TD.Name = "complexexpressionlist";
        NP += 3;
      } else if (NP[1] == 'X' && NP[2] == 'X') {
        DMP->TD.Ty = ASTTypeComplexExpression;
        DMP->TD.Name = "complexexpression";
        NP += 3;
      } else {
        DMP->TD.Ty = ASTTypeMPComplex;
        DMP->TD.UBits =
          ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], &J);
        DMP->TD.S = 'U';
        DMP->DTD0.S = 'U';
        NP += J + 1;
        if (NP[0] == 'i') {
          DMP->DTD0.Ty = ASTTypeInt;
          DMP->DTD0.UBits =
            ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
          NP += J + 2;
        } else if (NP[0] == 'I' && NP[1] == 'I') {
          DMP->DTD0.Ty = ASTTypeMPInteger;
          DMP->DTD0.UBits =
            ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
          NP += J + 3;
        } else if (NP[0] == 'j') {
          DMP->DTD0.Ty = ASTTypeUInt;
          DMP->DTD0.UBits =
            ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
          NP += J + 2;
        } else if (NP[0] == 'J' && NP[1] == 'J') {
          DMP->DTD0.Ty = ASTTypeMPUInteger;
          DMP->DTD0.UBits =
            ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
          NP += J + 3;
        } else if (NP[0] == 'f') {
          DMP->DTD0.Ty = ASTTypeFloat;
          DMP->DTD0.UBits =
            ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
          if (DMP->DTD0.UBits > 32U)
            DMP->DTD0.Ty = ASTTypeMPDecimal;
          NP += J + 2;
        } else if (NP[0] == 'd') {
          DMP->DTD0.Ty = ASTTypeDouble;
          DMP->DTD0.UBits =
            ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
          if (DMP->DTD0.UBits > 64U)
            DMP->DTD0.Ty = ASTTypeMPDecimal;
          NP += J + 2;
        } else if (NP[0] == 'e') {
          DMP->DTD0.Ty = ASTTypeMPDecimal;
          DMP->DTD0.UBits =
            ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
          NP += J + 2;
        }
      }
      break;
    case 'F':
      if (NP[1] == 'D') {
        DMP->TD.Ty = ASTTypeFunctionDeclaration;
        DMP->TD.UBits = ASTFunctionDefinitionNode::FunctionBits;
        if (NP[2] == 'Y') {
          DMP->DTD0.Ty = ASTTypeExtern;
          NP += 3;
          NP = ParseFunctionDeclaration(NP, DMP, true);
        } else {
          NP += 2;
          NP = ParseFunctionDeclaration(NP, DMP);
        }
      } else if (NP[1] == 'C') {
        DMP->TD.Ty = ASTTypeFunctionCall;
        NP += 2;
      } else if (NP[1] == 'p') {
        // FIXME: IMPLEMENT.
        DMP->TD.Ty = ASTTypeFuncParam;
        DMP->TD.IX = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
        NP += 3 + J;
        NP = ParseParam(NP, DMP->TD.Ty, DMP);
      } else if (NP[1] == 'a') {
        // FIXME: IMPLEMENT.
        DMP->TD.Ty = ASTTypeFuncArg;
        DMP->TD.IX = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
        NP += 3 + J;
        NP = ParseArg(NP, DMP->TD.Ty, DMP);
      } else if (NP[1] == 'r') {
        DMP->TD.Ty = ASTTypeReturn;
        DMP->TD.UBits = ASTReturnStatementNode::ReturnBits;
        DMP->TD.S = 'U';
        NP += 2;
        NP = ParseReturnType(NP, DMP);
      } else if (NP[1] == 'Y') {
        DMP->TD.Ty = ASTTypeFunction;
        DMP->TD.UBits = ASTFunctionDefinitionNode::FunctionBits;
        DMP->DTD0.Ty = ASTTypeExtern;
        NP += 2;
        NP = ParseFunctionDefinition(NP, DMP, true);
      } else {
        DMP->TD.Ty = ASTTypeFunction;
        DMP->TD.UBits = ASTFunctionDefinitionNode::FunctionBits;
        NP += 1;
        NP = ParseFunctionDefinition(NP, DMP);
      }
      break;
    case 'K':
      if (NP[1] == 'D') {
        DMP->TD.Ty = ASTTypeKernelDeclaration;
        NP += 2;
      } else if (NP[1] == 'C') {
        DMP->TD.Ty = ASTTypeKernelCall;
        NP += 2;
      } else if (NP[1] == 'p') {
        // FIXME: IMPLEMENT.
        DMP->TD.Ty = ASTTypeKernelParam;
        DMP->TD.IX = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[4], '_', &J);
        NP += 3 + J;
        NP = ParseParam(NP, DMP->TD.Ty, DMP);
      } else if (NP[1] == 'a') {
        // FIXME: IMPLEMENT.
        DMP->TD.Ty = ASTTypeKernelArg;
        DMP->TD.IX = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[4], '_', &J);
        NP += 3 + J;
        NP = ParseArg(NP, DMP->TD.Ty, DMP);
      } else if (NP[1] == 'Y') {
        DMP->TD.Ty = ASTTypeKernel;
        DMP->TD.UBits = ASTKernelNode::KernelBits;
        DMP->DTD0.Ty = ASTTypeExtern;
        NP += 2;
      } else {
        DMP->TD.Ty = ASTTypeKernel;
        DMP->TD.UBits = ASTKernelNode::KernelBits;
        NP += 1;
      }
      break;
    case 'X':
      DMP->TD.Ty = ASTTypeAngle;
      DMP->TD.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
      DMP->TD.S = 'U';
      NP += J + 2;
      NP = ParseName(NP, DMP);
      if (NP[0] == 'L') {
        ASTDemangled DDM;
        ASTType RTy = ASTTypeUndefined;
        bool NE = false;
        NP = ResolveLiteral(NP, RTy, &DDM, NE);
        DMP->DTD0 = DDM.TD;
        DMP->DTD0.Name = DMP->TD.Name;
      }
      break;
    case 'G':
      DMP->TD.Ty = ASTTypeGate;
      switch (NP[1]) {
      case 'C':
        DMP->TD.Ty = ASTTypeGateCall;
        NP += 2;
        break;
      case 'Q':
        DMP->TD.Ty = ASTTypeGateQubitParam;
        NP += 2;
        break;
      case 'o':
        DMP->TD.Ty = ASTTypeGateOpNode;
        NP += 2;
        break;
      case 'q':
        DMP->TD.Ty = ASTTypeGateQOpNode;
        NP += 2;
        break;
      case 'u':
        DMP->TD.Ty = ASTTypeGateUOpNode;
        NP += 2;
        break;
      case 'g':
        DMP->TD.Ty = ASTTypeGateGenericOpNode;
        NP += 2;
        break;
      case 'h':
        DMP->TD.Ty = ASTTypeGateHOpNode;
        NP += 2;
        break;
      case 'c':
        if (NP[2] == 'x') {
          DMP->TD.Ty = ASTTypeCXGateOpNode;
          NP += 3;
        } else if (NP[2] == 'c' && NP[3] == 'x') {
          DMP->TD.Ty = ASTTypeCCXGateOpNode;
          NP += 4;
        } else if (NP[2] == 'c' && NP[3] == 'n' && NP[4] == 'o' && NP[5] == 't') {
          DMP->TD.Ty = ASTTypeCNotGateOpNode;
          NP += 5;
        } else {
          DMP->TD.Ty = ASTTypeGateControl;
          NP += 2;
        }
        break;
      case 'i':
        DMP->TD.Ty = ASTTypeGateInverse;
        NP += 2;
        break;
      case 'n':
        DMP->TD.Ty = ASTTypeGateNegControl;
        NP += 2;
        break;
      case 'P':
        if (NP[2] == 'h') {
          DMP->TD.Ty = ASTTypeGPhaseExpression;
          NP += 3;
        }
        break;
      case 'w':
        DMP->TD.Ty = ASTTypeGatePower;
        NP += 2;
        break;
      case 'p': {
        // FIXME: IMPLEMENT.
        DMP->TD.Ty = ASTTypeGateParam;
        DMP->TD.IX = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
        std::stringstream GS;
        GS << "Gp" << DMP->TD.IX;
        DMP->TD.Name = GS.str();
        NP += 3 + J;
        NP = ParseParam(NP, DMP->TD.Ty, DMP);
      }
        break;
      case 'a': {
        // FIXME: IMPLEMENT.
        DMP->TD.Ty = ASTTypeGateArg;
        DMP->TD.IX = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
        std::stringstream GS;
        GS << "Ga" << DMP->TD.IX;
        DMP->TD.Name = GS.str();
        NP += 3 + J;
        NP = ParseArg(NP, DMP->TD.Ty, DMP);
      }
        break;
      default:
        NP += 1;
        break;
      }
      break;
    case 'Q':
      if (NP[1] == 'B') {
        DMP->TD.Ty = ASTTypeBoundQubit;
        NP += 2;
      } else if (NP[1] == 'U') {
        DMP->TD.Ty = ASTTypeUnboundQubit;
        NP += 2;
      } else if (NP[1] == 'C') {
        if (NP[2] == 'a') {
          DMP->TD.Ty = ASTTypeQubitContainerAlias;
          DMP->TD.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[3], '_', &J);
          DMP->TD.S = 'U';
          NP += J + 4;
        } else {
          DMP->TD.Ty = ASTTypeQubitContainer;
          DMP->TD.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
          DMP->TD.S = 'U';
          NP += J + 3;
        }
      } else if (NP[1] == 't') {
        DMP->TD.Ty = ASTTypeQubit;
        if (std::isdigit(NP[2])) {
          NP += 2;
          DMP->TD.IX = ASTStringUtils::StringToUnsigned<uint32_t>(NP, '_', &J);
          NP += J + 1;
        }
      } else {
        DMP->TD.Ty = ASTTypeQubit;
        if (std::isdigit(NP[1])) {
          DMP->TD.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
          DMP->TD.S = 'U';
          NP += J + 2;
        } else {
          DMP->TD.UBits = 1U;
          DMP->TD.S = 'U';
          NP += 1;
        }
      }
      break;
    case 'k':
      DMP->TD.Ty = ASTTypeConst;
      NP += 1;
      break;
    case 'B':
      if (NP[1] == 'o' && NP[2] == 'A') {
        DMP->TD.Ty = ASTTypeBoxAs;
        NP += 3;
      } else if (NP[1] == 'o' && NP[2] == 'T') {
        DMP->TD.Ty = ASTTypeBoxTo;
        NP += 3;
        NP = ParseName(NP, DMP);
        if (NP[0] == 'T' && NP[1] == 'U') {
          ASTDemangled DDM;
          NP += 2;
          NP = ParseTimeUnit(NP, &DDM);
          DMP->DTD0 = DDM.TD;
        }
      } else if (NP[1] == 'o') {
        DMP->TD.Ty = ASTTypeBox;
        NP += 2;
      } else if (NP[1] == 'a' && NP[2] == 'r') {
        DMP->TD.Ty = ASTTypeBarrier;
        NP += 3;
      } else if (NP[1] == 'O' && NP[2] == 'p') {
        DMP->TD.Ty = ASTTypeBinaryOp;
        DMP->TD.Name = "binaryop";
        NP += 3;
        NP = ParseName(NP, DMP);

        // Use a clean ASTDemangled for the binary op's dependent types.
        ASTDemangled* BDMP = new ASTDemangled();
        assert(BDMP && "Could not create a valid ASTDemangled!");

        BDMP->TD.Ty = DMP->TD.Ty;
        BDMP->TD.Name = DMP->TD.Name;
        NP = ParseBinaryOp(NP, BDMP, BDMP->DTV);
        BDMP->DTV.push_back(nullptr);
        VDMT.push_back(BDMP);
        if (BDMP->DTV.size())
          VDMT.insert(VDMT.end(), BDMP->DTV.begin(), BDMP->DTV.end());
      } else {
        DMP->TD.Ty = ASTTypeBitset;
        if (std::isdigit(NP[1])) {
          DMP->TD.Ty = ASTTypeBitset;
          DMP->TD.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
          DMP->TD.S = 'U';
          NP += J + 2;
        } else {
          DMP->TD.UBits = 1U;
          DMP->TD.S = 'U';
          NP += 1;
        }

        NP = ParseName(NP, DMP);
      }
      break;
    case 'D':
      if (NP[1] == 'C') {
        DMP->TD.Ty = ASTTypeDefcalCall;
        NP += 2;
      } else if (NP[1] == 'D') {
        DMP->TD.Ty = ASTTypeMPDecimal;
        DMP->TD.S = 'U';

        switch (NP[2]) {
        case 'F':
        case 'd':
        case 'e':
        case 'f':
          DMP->TD.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[3], '_', &J);
          NP += J + 3 + 1;
          NP = ParseName(NP, DMP);
          break;
        default:
          break;
        }
      } else if (NP[1] == 'M' && NP[2] == 'C') {
        DMP->TD.Ty = ASTTypeDefcalMeasureCall;
        NP += 3;
        NP = ParseName(NP, DMP);
        NP += 1;
        ASTDemangled DMM;
        NP = ParseType(NP, &DMM);
        NP = ParseName(NP, &DMM);
        DMP->DTD0.Ty = DMM.TD.Ty;
        DMP->DTD0.Name = DMM.TD.Name;
        DMP->DTD0.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMM.TD.Ty);
      } else if (NP[1] == 'M') {
        DMP->TD.Ty = ASTTypeDefcalMeasure;
        NP += 2;
        NP = ParseName(NP, DMP);
        NP = ParseDefcalMeasure(NP, DMP);
      } else if (NP[1] == 'R' && NP[2] == 'C') {
        DMP->TD.Ty = ASTTypeDefcalResetCall;
        NP += 3;
      } else if (NP[1] == 'R') {
        DMP->TD.Ty = ASTTypeDefcalReset;
        NP += 2;
        NP = ParseName(NP, DMP);
        NP = ParseDefcalReset(NP, DMP);
      } else if (NP[1] == 'G' && NP[2] == 'M') {
        DMP->TD.Ty = ASTTypeDefcalGrammar;
        DMP->TD.UBits =
          ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
        NP += 3;
      } else if (NP[1] == 'G' && NP[2] == 'R') {
        DMP->TD.Ty = ASTTypeDefcalGroup;
        DMP->TD.UBits =
          ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
        NP += 3;
      } else if (NP[1] == 'l') {
        DMP->TD.Ty = ASTTypeDelay;
        NP += 2;
        ASTDemangled DDM;
        NP = ParseName(NP, &DDM);
        DDM.Clear();
        NP = ParseType(NP, &DDM);
        if (DDM.TD.Ty == ASTTypeTimeUnit) {
          DMP->DTD0.Ty = DDM.TD.Ty;
          DDM.Clear();
          NP = ParseNumericLiteral(NP, &DDM);
          DMP->DTD1.Ty = DDM.TD.Ty;
          DMP->DTD1.UIVal = DDM.TD.UIVal;
          DMP->DTD1.V = 'u';
          DDM.Clear();
          NP = ParseType(NP, &DDM);
          NP = ParseName(NP, &DDM);
          DMP->DTD1.Name = DDM.TD.Name;
          NP += 1;
        } else if (DDM.TD.Ty == ASTTypeDuration ||
                   DDM.TD.Ty == ASTTypeDurationOf ||
                   DDM.TD.Ty == ASTTypeStretch) {
          DMP->DTD0.Ty = DDM.TD.Ty;
          NP = ParseName(NP, &DDM);
          DMP->DTD0.Name = DDM.TD.Name;
          NP += 1;
        } else if (DDM.TD.Ty == ASTTypeBinaryOp) {
          DMP->DTD0.Ty = DDM.TD.Ty;
          DDM.Clear();
          ASTDemangled* BDMP = new ASTDemangled();
          assert(BDMP && "Could not create a valid ASTDemangled!");

          NP = ParseBinaryOp(NP, BDMP, BDMP->DTV);
          VDMT.push_back(BDMP);
        } else if (DDM.TD.Ty == ASTTypeUnaryOp) {
          DMP->DTD0.Ty = DDM.TD.Ty;
          DDM.Clear();
          std::vector<ASTDemangled*> VDM;
          NP = ParseUnaryOp(NP, &DDM, VDM);
          VDMT.insert(VDMT.begin(), VDM.begin(), VDM.end());
        }

        while (NP) {
          DDM.Clear();
          if ((NP = ValidateEndOfExpression(NP))) {
              NP = ParseType(NP, &DDM);

              switch (DDM.TD.Ty) {
              case ASTTypeQubit:
              case ASTTypeQubitContainer:
              case ASTTypeQubitContainerAlias:
              case ASTTypeGateQubitParam:
                break;
              default:
                if ((NP = ValidateEndOfExpression(NP))) {
                  ParseType(NP, &DDM);
                } else {
                  continue;
                }
                break;
              }

              NP = ParseName(NP, &DDM);
              VDMT.push_back(new ASTDemangled(DDM));
              NP += 1;
          }
        }

        VDMT.push_back(nullptr);
      } else if (NP[1] == 'u' && NP[2] == 'O') {
        DMP->TD.Ty = ASTTypeDurationOf;
        NP += 3;
        NP = ParseName(NP, DMP);
        ASTDemangled DDM;
        NP = ParseDurationOfTarget(NP, &DDM);
        DMP->DTD0 = DDM.TD;
        VDMT.push_back(nullptr);
        NP = ValidateEndOfExpression(NP);
      } else if (NP[1] == 'u') {
        DMP->TD.Ty = ASTTypeDuration;
        NP += 2;
        NP = ParseName(NP, DMP);
        ASTDemangled DDM;
        NP = ParseType(NP, &DDM);

        switch (DDM.TD.Ty) {
        case ASTTypeTimeUnit:
          NP = ParseTimeUnit(NP, &DDM);
          DMP->DTD0 = DDM.TD;
          break;
        case ASTTypeDurationOf:
          NP = ParseType(NP, &DDM);
          DMP->DTD0 = DDM.TD;
          DMP->DTD1 = DDM.DTD0;
          if (DMP->DTD1.Ty == ASTTypeGate ||
              DMP->DTD1.Ty == ASTTypeDefcal ||
              DMP->DTD1.Ty == ASTTypeDefcalGroup) {
            NP = ValidateEndOfExpression(NP);
            while (NP) {
              ASTDemangled* DDMP = new ASTDemangled();
              assert(DDMP && "Could not create a valid ASTDemangled!");
              NP = ParseType(NP, DDMP);
              NP = ParseName(NP, DDMP);
              VDMT.push_back(DDMP);
              NP = ValidateEndOfExpression(NP);
            }
          }
          break;
        case ASTTypeUnaryOp: {
          DM.DTD0.Ty = DDM.TD.Ty;
          VDMT.pop_back();
        }
          break;
        case ASTTypeBinaryOp: {
          DM.DTD0.Ty = DDM.TD.Ty;
          VDMT.pop_back();
        }
          break;
        default:
          NP = ParseType(NP, &DDM);
          break;
        }
      } else if (NP[1] == 'a') {
        // FIXME: IMPLEMENT.
        DMP->TD.Ty = ASTTypeDefcalArg;
        DMP->TD.Name = ASTStringUtils::Instance().Substring(&NP[0], '_');
        DMP->TD.IX = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
        NP += 3 + J;
        NP = ParseArg(NP, DMP->TD.Ty, DMP);
      } else if (NP[1] == 'p') {
        DMP->TD.Ty = ASTTypeDefcalParam;
        DMP->TD.Name = ASTStringUtils::Instance().Substring(&NP[0], '_');
        DMP->TD.IX = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
        NP += 3 + J;
        NP = ParseParam(NP, DMP->TD.Ty, DMP);
      } else {
        DMP->TD.Ty = ASTTypeDefcal;
        DMP->TD.UBits =
          ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
        NP += 1;
      }
      break;
    case 'U':
      if (NP[1] == 'O' && NP[2] == 'p') {
        DMP->TD.Ty = ASTTypeUnaryOp;
        DMP->TD.Name = "unaryop";
        if (NP[3] == 'n' && NP[4] == 'g') {
          DMP->TD.OpTy = ASTOpTypeNegative;
          NP += 5;
        } else if (NP[3] == 'n' && NP[4] == 't') {
          DMP->TD.OpTy = ASTOpTypeNegate;
          NP += 5;
        } else if (NP[3] == 'p' && NP[4] == 's') {
          DMP->TD.OpTy = ASTOpTypePositive;
          NP += 5;
        } else if (NP[3] == '_' && NP[4] == 'p' && NP[5] == 'p') {
          DMP->TD.OpTy = ASTOpTypePreInc;
          NP += 6;
        } else if (NP[3] == '_' && NP[4] == 'm' && NP[5] == 'm') {
          DMP->TD.OpTy = ASTOpTypePreDec;
          NP += 6;
        } else if (NP[3] == 'p' && NP[4] == 'p' && NP[5] == '_') {
          DMP->TD.OpTy = ASTOpTypePostInc;
          NP += 6;
        } else if (NP[3] == 'm' && NP[4] == 'm' && NP[5] == '_') {
          DMP->TD.OpTy = ASTOpTypePostDec;
          NP += 6;
        } else {
          std::vector<ASTDemangled*> DMV;
          VDMT.push_back(DMP);
          NP = ParseUnaryOp(NP, DMP, DMV);
        }
      }
      break;
    case 'A':
      DMP->TD.Ty = ASTTypeArray;
      DMP->TD.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
      DMP->TD.S = 'U';
      NP += J + 1;

      switch (NP[0]) {
      case 'I':
        if (NP[1] == 'I') {
          DMP->TD.Ty = ASTTypeMPIntegerArray;
          DMP->DTD0.Ty = ASTTypeMPInteger;
          DMP->DTD0.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
          DMP->DTD0.S = 'S';
          NP += J + 3;
        }
        break;
      case 'J':
        if (NP[1] == 'J') {
          DMP->TD.Ty = ASTTypeMPIntegerArray;
          DMP->DTD0.Ty = ASTTypeMPUInteger;
          DMP->DTD0.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
          DMP->DTD0.S = 'U';
          NP += J + 3;
        }
        break;
      case 'D':
        if (NP[1] == 'D') {
          DMP->TD.Ty = ASTTypeMPDecimalArray;
          DMP->DTD0.Ty = ASTTypeMPDecimal;
          DMP->DTD0.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
          DMP->DTD0.S = 'S';
          NP += J + 3;
        } else if (NP[1] == 'u') {
          if (NP[2] == 'O') {
            DMP->TD.Ty = ASTTypeDurationArray;
            DMP->DTD0.Ty = ASTTypeDurationOf;
            NP += 3;
          } else {
            DMP->TD.Ty = ASTTypeDurationArray;
            DMP->DTD0.Ty = ASTTypeDuration;
            NP += 2;
            DMP->DTD1.Ty = ASTTypeTimeUnit;
            DMP->DTD1.SVal = ParseTimeUnit(NP, &J);
            NP += J + 1;
          }
        }
        break;
      case 'X':
        DMP->TD.Ty = ASTTypeAngleArray;
        DMP->DTD0.Ty = ASTTypeAngle;
        DMP->DTD0.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
        DMP->DTD0.S = 'U';
        NP += J + 2;
        break;
      case 'b':
        DMP->TD.Ty = ASTTypeBoolArray;
        DMP->DTD0.Ty = ASTTypeBool;
        DMP->DTD0.UBits = 8U;
        DMP->DTD0.S = 'U';
        NP += 1;
        break;
      case 'i':
        DMP->TD.Ty = ASTTypeIntArray;
        DMP->DTD0.Ty = ASTTypeInt;
        DMP->DTD0.UBits = 32U;
        DMP->DTD0.S = 'S';
        NP += 1;
        break;
      case 'j':
        DMP->TD.Ty = ASTTypeIntArray;
        DMP->DTD0.Ty = ASTTypeUInt;
        DMP->DTD0.UBits = 32U;
        DMP->DTD0.S = 'U';
        NP += 1;
        break;
      case 'f':
        DMP->TD.Ty = ASTTypeFloatArray;
        DMP->DTD0.Ty = ASTTypeFloat;
        DMP->DTD0.UBits = 32U;
        DMP->DTD0.S = 'S';
        NP += 1;
        break;
      case 'd':
        DMP->TD.Ty = ASTTypeFloatArray;
        DMP->DTD0.Ty = ASTTypeDouble;
        DMP->DTD0.UBits = 64U;
        DMP->DTD0.S = 'S';
        NP += 1;
        break;
      case 'e':
        DMP->TD.Ty = ASTTypeFloatArray;
        DMP->DTD0.Ty = ASTTypeLongDouble;
        DMP->DTD0.UBits = 128U;
        DMP->DTD0.S = 'S';
        NP += 1;
        break;
      case 'Q':
        DMP->TD.Ty = ASTTypeQubitArray;
        DMP->DTD0.Ty = ASTTypeQubitContainer;
        DMP->DTD0.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
        DMP->DTD0.S = 'U';
        NP += J + 2;
        break;
      case 'C':
        DMP->TD.Ty = ASTTypeMPComplexArray;
        DMP->DTD0.Ty = ASTTypeMPComplex;
        DMP->DTD0.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], &J);
        DMP->TD.S = 'U';
        NP += J + 1;
        switch (NP[0]) {
        case 'i':
          if (std::isdigit(NP[1])) {
            DMP->DTD1.Ty = ASTTypeMPInteger;
            DMP->DTD1.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
            DMP->DTD1.S = 'U';
            NP += J + 2;
          } else {
            DMP->DTD1.Ty = ASTTypeInt;
            DMP->DTD1.UBits = ASTIntNode::IntBits;
            DMP->DTD1.S = 'U';
            NP += 2;
          }
          break;
        case 'j':
          if (std::isdigit(NP[1])) {
            DMP->DTD1.Ty = ASTTypeMPUInteger;
            DMP->DTD1.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
            DMP->DTD1.S = 'U';
            NP += J + 2;
          } else {
            DMP->DTD1.Ty = ASTTypeUInt;
            DMP->DTD1.UBits = ASTIntNode::IntBits;
            DMP->DTD1.S = 'U';
            NP += 2;
          }
          break;
        case 'f':
          if (std::isdigit(NP[1])) {
            DMP->DTD1.Ty = ASTTypeMPDecimal;
            DMP->DTD1.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
            DMP->DTD1.S = 'U';
            NP += J + 2;
          } else {
            DMP->DTD1.Ty = ASTTypeFloat;
            DMP->DTD1.UBits = ASTFloatNode::FloatBits;
            DMP->DTD1.S = 'U';
            NP += 2;
          }
          break;
        case 'd':
          if (std::isdigit(NP[1])) {
            DMP->DTD1.Ty = ASTTypeMPDecimal;
            DMP->DTD1.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
            DMP->DTD1.S = 'U';
            NP += J + 2;
          } else {
            DMP->DTD1.Ty = ASTTypeDouble;
            DMP->DTD1.UBits = ASTDoubleNode::DoubleBits;
            DMP->DTD1.S = 'U';
            NP += 2;
          }
          break;
        case 'I':
          if (NP[1] == 'I') {
            DMP->DTD1.Ty = ASTTypeMPInteger;
            DMP->DTD1.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], &J);
            DMP->DTD1.S = 'S';
            NP += J + 3;
          }
          break;
        case 'J':
          if (NP[1] == 'J') {
            DMP->DTD1.Ty = ASTTypeMPUInteger;
            DMP->DTD1.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], &J);
            DMP->DTD1.S = 'U';
            NP += J + 3;
          }
          break;
        case 'D':
          if (NP[1] == 'D') {
            DMP->DTD1.Ty = ASTTypeMPDecimal;
            DMP->DTD1.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], &J);
            DMP->DTD1.S = 'S';
            NP += J + 3;
          }
          break;
        default:
          break;
        }
        break;
      case 'B':
        DMP->TD.Ty = ASTTypeCBitArray;
        DMP->DTD0.Ty = ASTTypeBitset;
        DMP->DTD0.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[1], '_', &J);
        DMP->DTD0.S = 'U';
        NP += J + 2;
        break;
      case 'O':
        if (NP[1] == 'P' && NP[2] == 'F') {
          DMP->TD.Ty = ASTTypeOpenPulseFrameArray;
          DMP->DTD0.Ty = ASTTypeOpenPulseFrame;
          DMP->DTD0.UBits = OpenPulse::ASTOpenPulseFrameNode::FrameBits;
          DMP->DTD0.S = 'U';
          NP += 4;
        } else if (NP[1] == 'P' && NP[2] == 'R') {
          DMP->TD.Ty = ASTTypeOpenPulsePortArray;
          DMP->DTD0.Ty = ASTTypeOpenPulsePort;
          DMP->DTD0.UBits = OpenPulse::ASTOpenPulsePortNode::PortBits;
          DMP->DTD0.S = 'U';
          NP += 4;
        }
        break;
      }
      break;
    case 'M':
      if (NP[1] == 'j') {
        DMP->TD.Ty = ASTTypeMeasure;
        NP += 2;
        NP = ParseName(NP, DMP);
        DMP->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMP->TD.Ty);
        NP = ParseMeasure(NP, DMP);
      }
      break;
    case 'S': {
      DMP->TD.Ty = ASTTypeStretch;
      NP += 1;
      NP = ParseName(NP, DMP);
      ASTDemangled DDM;
      NP += 2;
      NP = ParseTimeUnit(NP, &DDM);
      DMP->DTD0 = DDM.TD;
    }
      break;
    case 'T':
      if (NP[1] == 'e') {
        DMP->TD.Ty = ASTTypeEnum;
        NP += 2;
      } else if (NP[1] == 'U') {
        DMP->TD.Ty = ASTTypeTimeUnit;
        NP += 2;
      }
      break;
    case 'g':
      if (NP[1] == 'E') {
        DMP->TD.Ty = ASTTypeImaginary;
        DMP->TD.Name = "im";
        NP += 1;
      }
      break;
    default: {
      if (IsOperator(NP)) {
        std::vector<ASTDemangled*> TV;
        NP = ParseOpType(NP, DMP, TV);
        assert(TV.empty() && "No elements should be stored in the TV vector!");
      }
    }
      break;
    }
  }

  return NP;
}

const char* ASTDemangler::ParseName(const char* S, ASTDemangled* DMP) {
  assert(DMP && "Invalid ASTDemangled argument!");

  if (!S) return S;

  if (!(S = ValidateEndOfExpression(S)))
    return nullptr;

  if (*S) {
    uint32_t J;

    if (uint32_t L = ASTStringUtils::StringToUnsigned<uint32_t>(S, &J)) {
      const char* R = S + J;
      std::stringstream SS;
      SS.write(R, L);
      DMP->TD.Name = SS.str();
      R += L;
      return R;
    }
  }

  return S;
}

std::string ASTDemangler::ParseTimeUnit(const char* S, unsigned* J) {
  assert(S && "Invalid mangled string argument!");

  *J = 0U;
  std::stringstream SS;

  while (*S && *S != u8'_' && *S != u8'E') {
    SS << *S++;
    *J += 1U;
  }

  return SS.str();
}

const char* ASTDemangler::ParseTimeUnit(const char* S, ASTDemangled* DMP) {
  assert(DMP && "Invalid ASTDemangled argument!");

  if (!S) return S;

  if (!(S = ValidateEndOfExpression(S)))
    return nullptr;

  const char* R = S;

  DMP->TD.ULVal = ASTStringUtils::StringToUnsigned<uint64_t>(R);
  const char* C = R;

  while (*C && std::isdigit(*C++));
  std::string_view U = --C;

  using std::operator""sv;

  if (U.compare(0, 2, u8"ns"sv) == 0)
    DMP->TD.LU = LengthUnit::Nanoseconds;
  else if (U.compare(0, 2, u8"ms"sv) == 0)
    DMP->TD.LU = LengthUnit::Milliseconds;
  else if (U.compare(0, 2, u8"us"sv) == 0)
    DMP->TD.LU = LengthUnit::Microseconds;
  else if (U.compare(0, 1, u8"s"sv) == 0)
    DMP->TD.LU = LengthUnit::Seconds;
  else if (U.compare(0, 2, u8"μs"sv) == 0)
    DMP->TD.LU = LengthUnit::Microseconds;
  else if (U.compare(0, 2, u8"dt"sv) == 0)
    DMP->TD.LU = LengthUnit::DT;

  std::stringstream SS;
  while (*R != 'E')
    SS << *R++;

  DMP->TD.Name = SS.str();
  DMP->TD.Ty = ASTTypeTimeUnit;
  DMP->TD.UBits = 64U;
  DMP->TD.S = 'U';
  return R;
}

const char* ASTDemangler::ParseSubsequent(const char* S) {
  // FIXME: IMPLEMENT.
  return S;
}

const char* ASTDemangler::ValidateEndOfExpression(const char* S) {
  if (!S) return S;
  if (!*S) return nullptr;

  const char* R = S;

  while (*R == 'E') {
    if (*(R + 1) == '_' || *(R + 1) == '\0')
      return nullptr;
    else if (*(R + 1) == 'E' && ((*(R + 2) == '_') || *(R + 2) == '\0'))
      return nullptr;

    R += 1;
  }

  return R;
}

const char* ASTDemangler::SkipExpressionTerminator(const char* S) {
  if (S && *S) {
    const char* R = S;

    while (*R == u8'E')
      ++R;

    return R[0] == u8'_' && R[1] == u8'\0' ? nullptr : R;
  }

  return S;
}

const char* ASTDemangler::ParseParam(const char* S, ASTType Ty,
                                     ASTDemangled* DMP) {
  // FIXME: IMPLEMENT.
  assert(S && "Invalid mangled string argument!");
  assert(DMP && "Invalid ASTDemangled argument!");

  const char* R = ValidateEndOfExpression(S);
  if (!R) return nullptr;

  switch (Ty) {
  case ASTTypeDefcalParam:
  case ASTTypeFuncParam:
  case ASTTypeGateParam:
  case ASTTypeKernelParam: {
    if (VDMT.size()) {
      if (const ASTDemangled* BDMP = VDMT.back()) {
        if (BDMP->TD.Ty == DMP->TD.Ty && BDMP->TD.Name != DMP->TD.Name)
          VDMT.pop_back();
      }
    }

    VDMT.push_back(DMP);

    ASTDemangled* PDM = new ASTDemangled();
    assert(PDM && "Could not create a valid ASTDemangled!");

    R = ParseType(R, PDM);
    if (VDMT.size() && !VDMT.back())
      VDMT.pop_back();

    if (IsBinaryOrUnaryOp(PDM->TD.Ty)) {
      return R;
    } else if ((PDM->TD.Ty == ASTTypeQubit || PDM->TD.Ty == ASTTypeQubitContainer) &&
               Ty == ASTTypeGateParam) {
      PDM->TD.Ty = ASTTypeGateQubitParam;
    } else if (PDM->TD.Ty == ASTTypeAngle && Ty == ASTTypeGateParam) {
      PDM->TD.Ty = ASTTypeGateAngleParam;
    }

    R = ParseName(R, PDM);
    VDMT.push_back(PDM);
  }
    break;
  default:
    break;
  }

  return R;
}

const char* ASTDemangler::ParseReturnType(const char* S, ASTDemangled* DMP) {
  // FIXME: IMPLEMENT.
  assert(S && "Invalid mangled string argument!");
  assert(DMP && "Invalid ASTDemangled argument!");

  const char* R = ValidateEndOfExpression(S);
  if (!R) return nullptr;

  ASTDemangled TDM;
  R = ParseType(R, &TDM);

  switch (TDM.TD.Ty) {
  case ASTTypeMPComplex:
  case ASTTypeAngleArray:
  case ASTTypeBoolArray:
  case ASTTypeCBitArray:
  case ASTTypeCBitNArray:
  case ASTTypeDurationArray:
  case ASTTypeFloatArray:
  case ASTTypeIntArray:
  case ASTTypeUIntArray:
  case ASTTypeMPComplexArray:
  case ASTTypeMPDecimalArray:
  case ASTTypeMPIntegerArray:
  case ASTTypeMPUIntegerArray:
  case ASTTypeOpenPulseFrameArray:
  case ASTTypeOpenPulsePortArray:
  case ASTTypeQubitArray:
  case ASTTypeQubitNArray:
    DMP->DTD0 = TDM.TD;
    DMP->DTD1 = TDM.DTD0;
    break;
  default:
    DMP->DTD0 = TDM.TD;
    break;
  }

  return R;
}

const char* ASTDemangler::ParseOpFoldType(const char* S,
                          std::vector<ASTDemangled*>& FV) {
  assert(S && "Invalid mangled string argument!");

  const char* R = ValidateEndOfExpression(S);
  if (!R) return nullptr;

  while (R[0] == 'f') {
    ASTDemangled* N = new ASTDemangled();
    assert(N && "Could not create a valid ASTDemangled!");

    if (R[1] == 'l') {
      N->TD.Ty = ASTTypeOpTy;
      N->TD.OpTy = ASTOpTypeUnaryLeftFold;
      N->TD.Name = "(";
      FV.push_back(N);
      R += 2;
    } else if (R[1] == 'L') {
      N->TD.Ty = ASTTypeOpTy;
      N->TD.OpTy = ASTOpTypeBinaryLeftFold;
      N->TD.Name = "(";
      FV.push_back(N);
      R += 2;
    } else if (R[1] == 'r') {
      N->TD.Ty = ASTTypeOpTy;
      N->TD.OpTy = ASTOpTypeUnaryRightFold;
      N->TD.Name = ")";
      FV.push_back(N);
      R += 2;
    } else if (R[1] == 'R') {
      N->TD.Ty = ASTTypeOpTy;
      N->TD.OpTy = ASTOpTypeBinaryRightFold;
      N->TD.Name = ")";
      FV.push_back(N);
      R += 2;
    }
  }

  return R;
}

const char* ASTDemangler::ParseOpType(const char* S, ASTDemangled* DMP,
                          std::vector<ASTDemangled*>& TV) {
  assert(S && "Invalid mangled string argument!");
  assert(DMP && "Could not create a valid ASTDemangled!");

  const char* R = ValidateEndOfExpression(S);
  if (!R) return nullptr;

  DMP->TD.Ty = ASTTypeUndefined;
  DMP->TD.OpTy = ASTOpTypeUndefined;

  switch (R[0]) {
  case 'a': {
    if (R[1] == 'a') {
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.OpTy = ASTOpTypeLogicalAnd;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    } else if (R[1] == 'n') {
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.OpTy = ASTOpTypeBitAnd;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    } else if (R[1] == 'N') {
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.OpTy = ASTOpTypeBitAndAssign;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    } else if (R[1] == 'S') {
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.OpTy = ASTOpTypeAssign;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    }
  }
    break;
  case 'd': {
    if (R[1] == 'v') {
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.OpTy = ASTOpTypeDiv;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    } else if (R[1] == 'V') {
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.OpTy = ASTOpTypeDivAssign;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    }
  }
    break;
  case 'e': {
    if (R[1] == 'o') {
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.OpTy = ASTOpTypeXor;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    } else if (R[1] == 'O') {
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.OpTy = ASTOpTypeXorAssign;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    } else if (R[1] == 'q') {
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.OpTy = ASTOpTypeCompEq;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    }
  }
    break;
  case 'f': {
    DMP->TD.Ty = ASTTypeUndefined;
    DMP->TD.OpTy = ASTOpTypeUndefined;
    R = ParseOpFoldType(R, TV);
  }
    break;
  case 'g': {
    if (R[1] == 'e') {
      DMP->TD.OpTy = ASTOpTypeGE;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    } else if (R[1] == 't') {
      DMP->TD.OpTy = ASTOpTypeGT;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    }

    R += 2;
  }
    break;
  case 'l': {
    if (R[1] == 's') {
      DMP->TD.OpTy = ASTOpTypeLeftShift;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    } else if (R[1] == 'S') {
      DMP->TD.OpTy = ASTOpTypeLeftShiftAssign;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    }

    R += 2;
  }
    break;
  case 'm': {
    if (R[1] == 'l') {
      DMP->TD.OpTy = ASTOpTypeMul;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    } else if (R[1] == 'i') {
      DMP->TD.OpTy = ASTOpTypeSub;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    } else if (R[1] == 'L') {
      DMP->TD.OpTy = ASTOpTypeMulAssign;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    } else if (R[1] == 'I') {
      DMP->TD.OpTy = ASTOpTypeSubAssign;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    } else if (R[1] == 'm' && R[2] == '_') {
      DMP->TD.OpTy = ASTOpTypePostDec;
      DMP->TD.Name = TV.back()->TD.Name;
      DMP->TD.Name += PrintOpTypeOperator(DMP->TD.OpTy);
      DMP->TD.Ty = ASTTypeOpTy;
      R += 3;
    }
  }
    break;
  case 'n': {
    if (R[1] == 'e') {
      DMP->TD.OpTy = ASTOpTypeCompNeq;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    } else if (R[1] == 'g') {
      DMP->TD.OpTy = ASTOpTypeNegative;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    } else if (R[1] == 't') {
      DMP->TD.OpTy = ASTOpTypeLogicalNot;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    }

    R += 2;
  }
    break;
  case 'N': {
    if (R[1] == 'o' && R[2] == 'n') {
      DMP->TD.OpTy = ASTOpTypeNone;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = '?';
    }

    R += 3;
  }
    break;
  case 'o': {
    if (R[1] == 'o') {
      DMP->TD.OpTy = ASTOpTypeLogicalOr;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    } else if (R[1] == 'r') {
      DMP->TD.OpTy = ASTOpTypeBitOr;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    } else if (R[1] == 'R') {
      DMP->TD.OpTy = ASTOpTypeBitOrAssign;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    }

    R += 2;
  }
    break;
  case 'p': {
    if (R[1] == 'p' && R[2] == '_') {
      DMP->TD.OpTy = ASTOpTypePostInc;
      DMP->TD.Name = TV.back()->TD.Name;
      DMP->TD.Name += PrintOpTypeOperator(DMP->TD.OpTy);
      DMP->TD.Ty = ASTTypeOpTy;
      R += 3;
    } else if (R[1] == 'l') {
      DMP->TD.OpTy = ASTOpTypeAdd;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    } else if (R[1] == 'L') {
      DMP->TD.OpTy = ASTOpTypeAddAssign;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 2;
    }
  }
    break;
  case 'P': {
    if (R[1] == 'p' && R[2] == 'c') {
      DMP->TD.OpTy = ASTOpTypePopcount;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 3;
    }
  }
    break;
  case 'r': {
    if (R[1] == 'm') {
      DMP->TD.OpTy = ASTOpTypeMod;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    } else if (R[1] == 'M') {
      DMP->TD.OpTy = ASTOpTypeModAssign;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    } else if (R[1] == 's') {
      DMP->TD.OpTy = ASTOpTypeRightShift;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    } else if (R[1] == 'S') {
      DMP->TD.OpTy = ASTOpTypeRightShiftAssign;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    }

    R += 2;
  }
    break;
  case 'R': {
    if (R[1] == 'o' && R[2] == 't') {
      DMP->TD.OpTy = ASTOpTypeRotation;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    } else if (R[1] == 't' && R[2] == 'l') {
      DMP->TD.OpTy = ASTOpTypeRotl;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    } else if (R[1] == 't' && R[2] == 'r') {
      DMP->TD.OpTy = ASTOpTypeRotr;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
    }

    R += 3;
  }
    break;
  case 'T': {
    if (R[1] == 'a') {
      if (R[2] == 's') {
        DMP->TD.OpTy = ASTOpTypeArcSin;
        DMP->TD.Ty = ASTTypeOpTy;
        DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
        R += 3;
      } else if (R[2] == 'c') {
        DMP->TD.OpTy = ASTOpTypeArcCos;
        DMP->TD.Ty = ASTTypeOpTy;
        DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
        R += 3;
      } else if (R[2] == 't') {
        DMP->TD.OpTy = ASTOpTypeArcTan;
        DMP->TD.Ty = ASTTypeOpTy;
        DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
        R += 3;
      }
    } else if (R[1] == 'e' && R[2] == 'x') {
      DMP->TD.OpTy = ASTOpTypeExp;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 3;
    } else if (R[1] == 'l' && R[2] == 'n') {
      DMP->TD.OpTy = ASTOpTypeLn;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 3;
    } else {
      if (R[2] == 's') {
        DMP->TD.OpTy = ASTOpTypeSin;
        DMP->TD.Ty = ASTTypeOpTy;
        DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
        R += 2;
      } else if (R[2] == 'c') {
        DMP->TD.OpTy = ASTOpTypeCos;
        DMP->TD.Ty = ASTTypeOpTy;
        DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
        R += 2;
      } else if (R[2] == 't') {
        DMP->TD.OpTy = ASTOpTypeTan;
        DMP->TD.Ty = ASTTypeOpTy;
        DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
        R += 2;
      }
    }
  }
    break;
  case '_': {
    if (R[1] == 'p' && R[2] == 'p') {
      DMP->TD.OpTy = ASTOpTypePreInc;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 3;
    } else if (R[1] == 'm' && R[2] == 'm') {
      DMP->TD.OpTy = ASTOpTypePreDec;
      DMP->TD.Ty = ASTTypeOpTy;
      DMP->TD.Name = PrintOpTypeOperator(DMP->TD.OpTy);
      R += 3;
    }
  }
    break;
  default:
    break;
  }

  return R;
}

const char* ASTDemangler::ParseArg(const char* S, ASTType Ty,
                                   ASTDemangled* DMP) {
  assert(S && "Invalid mangled string argument!");
  assert(DMP && "Invalid ASTDemangled argument!");

  const char* R = ValidateEndOfExpression(S);
  if (!R) return nullptr;

  switch (Ty) {
  case ASTTypeDefcalArg:
  case ASTTypeFuncArg:
  case ASTTypeGateArg:
  case ASTTypeKernelArg: {
    if (VDMT.size()) {
      if (const ASTDemangled* BDMP = VDMT.back()) {
        if (BDMP->TD.Ty == DMP->TD.Ty && BDMP->TD.Name != DMP->TD.Name)
          VDMT.pop_back();
      }
    }

    VDMT.push_back(DMP);

    ASTDemangled* PDM = new ASTDemangled();
    assert(PDM && "Could not create a valid ASTDemangled!");

    R = ParseType(R, PDM);
    if (VDMT.size() && !VDMT.back())
      VDMT.pop_back();

    if (IsBinaryOrUnaryOp(PDM->TD.Ty)) {
      return R;
    }

    R = ParseName(R, PDM);
    VDMT.push_back(PDM);

  }
    break;
  default:
    break;
  }

  return R;
}

const char* ASTDemangler::ResolveLiteralCReal(const char* S, ASTDemangled* DMP) {
  assert(S && "Invalid mangled string argument!");

  if (S[0] != 'C' && S[1] != 'r' && S[2] != 'L')
    return S;

  const char* R = S + 2;
  ASTDemangled DMR;
  ASTType RRTy = ASTTypeUndefined;
  bool NR = false;

  R = ResolveLiteral(R, RRTy, &DMR, NR);
  DMP->DTD0 = DMR.TD;
  DMP->DTD0.Name = "creal";

  ASTDemangled* ODMP = new ASTDemangled();
  assert(ODMP && "Could not create a valid ASTDemangled!");

  std::vector<ASTDemangled*> D;
  R = ParseOpType(R, ODMP, D);
  DMP->TD.OpTy = ODMP->TD.OpTy;
  delete ODMP;

  return R;
}

const char* ASTDemangler::ResolveLiteralCImag(const char* S, ASTDemangled* DMP) {
  assert(S && "Invalid mangled string argument!");

  if (S[0] != 'C' && S[1] != 'i' && S[2] != 'L')
    return S;

  const char* R = S + 2;
  ASTDemangled DMI;
  ASTType IRTy = ASTTypeUndefined;
  bool NI = false;

  R = ResolveLiteral(R, IRTy, &DMI, NI);
  DMP->DTD1 = DMI.TD;

  return R;
}

const char* ASTDemangler::ResolveLiteral(const char* S, ASTType& RTy,
                                         ASTDemangled* DMP, bool& N) {
  assert(S && "Invalid mangled string argument!");

  if (*S != 'L')
    return S;

  const char* R = S + 1;
  const char* B = R;
  std::stringstream SS;
  std::string CN;
  bool XK = true;

  if (*R == 'C') {
    if ((R[1] == 'r' || R[1] == 'i') && R[2] == 'L') {
      if (R[1] == 'r')
        CN = "creal";
      else if (R[1] == 'i')
        CN = "cimag";

      R += 3;
      B = R;

      while (*R != '_')
        SS << *R++;
      XK = false;
    } else {
      SS << *R++;
      B = R;
      while (*R != 'C')
        SS << *R++;
      XK = false;
    }
  } else {
    while (*R != '_')
      SS << *R++;

    while (!std::isdigit(*B)) B++;
  }

  std::string LS = SS.str();
  RTy = ASTTypeUndefined;
  unsigned I = 0;
  std::smatch M;

  for (std::vector<ASTDemangler::RXM>::iterator LI = TYMM.begin();
       LI != TYMM.end(); ++LI) {
    if (std::regex_match(LS, M, (*LI).R)) {
      I = (*LI).I;
      RTy = (*LI).T;
      N = (*LI).N;
      break;
    }
  }

  unsigned J;
  if (XK)
    R += 1;

  switch (I) {
  case RXInt32Pos:      // Signed positive 32-bit integer.
    DMP->TD.Ty = ASTTypeInt;
    DMP->TD.SBits = 32;
    DMP->TD.SIVal = ASTStringUtils::StringToSigned<int32_t>(R, 'E', &J);
    DMP->TD.S = 'S';
    DMP->TD.V = 'i';
    DMP->TD.L = true;
    DMP->TD.SVal = std::to_string(DMP->TD.SIVal);
    R += J;
    break;
  case RXInt32Neg:      // Signed negative 32-bit integer.
    DMP->TD.Ty = ASTTypeInt;
    DMP->TD.SBits = 32;
    DMP->TD.SIVal = -1 * ASTStringUtils::StringToSigned<int32_t>(R, 'E', &J);
    DMP->TD.S = 'S';
    DMP->TD.V = 'i';
    DMP->TD.L = true;
    DMP->TD.SVal = std::to_string(DMP->TD.SIVal);
    R += J;
    break;
  case RXUInt32:        // Unsigned 32-bit integer.
    DMP->TD.Ty = ASTTypeUInt;
    DMP->TD.UBits = 32U;
    DMP->TD.UIVal = ASTStringUtils::StringToUnsigned<uint32_t>(R, 'E', &J);
    DMP->TD.S = 'U';
    DMP->TD.V = 'u';
    DMP->TD.L = true;
    DMP->TD.SVal = std::to_string(DMP->TD.UIVal);
    R += J;
    break;
  case RXInt64Pos:      // Signed positive 64-bit integer.
    DMP->TD.Ty = ASTTypeInt;
    DMP->TD.SBits = 64;
    DMP->TD.SLVal = ASTStringUtils::StringToSigned<int64_t>(R, 'E', &J);
    DMP->TD.S = 'S';
    DMP->TD.V = 'I';
    DMP->TD.L = true;
    DMP->TD.SVal = std::to_string(DMP->TD.SLVal);
    R += J;
    break;
  case RXInt64Neg:      // Signed negative 64-bit integer.
    DMP->TD.Ty = ASTTypeInt;
    DMP->TD.SBits = 64;
    DMP->TD.SLVal = -1L * ASTStringUtils::StringToSigned<int64_t>(R, 'E', &J);
    DMP->TD.S = 'S';
    DMP->TD.V = 'i';
    DMP->TD.L = true;
    DMP->TD.SVal = std::to_string(DMP->TD.SLVal);
    R += J;
    break;
  case RXUInt64:        // Unsigned 64-bit integer.
    DMP->TD.Ty = ASTTypeUInt;
    DMP->TD.UBits = 64U;
    DMP->TD.ULVal = ASTStringUtils::StringToUnsigned<uint64_t>(R, 'E', &J);
    DMP->TD.S = 'U';
    DMP->TD.V = 'u';
    DMP->TD.L = true;
    DMP->TD.SVal = std::to_string(DMP->TD.ULVal);
    R += J;
    break;
  case RXMPIntPos:      // Arbitrary precision positive integer.
    DMP->TD.Ty = ASTTypeMPInteger;
    DMP->TD.SBits = ASTStringUtils::StringToSigned<int32_t>(B, '_', &J);
    DMP->TD.S = 'S';
    DMP->TD.L = true;
    DMP->TD.V = 'I';
    DMP->TD.SVal =
      ASTStringUtils::Instance().RemoveManglingTerminator(std::string_view(R));
    while (*R != 'E') R++;
    break;
  case RXMPIntNeg:      // Arbitrary precision signed negative integer.
    DMP->TD.Ty = ASTTypeMPInteger;
    DMP->TD.SBits = ASTStringUtils::StringToSigned<int32_t>(B, 'n', &J);
    DMP->TD.S = 'S';
    DMP->TD.L = true;
    DMP->TD.V = 'I';
    DMP->TD.SVal = '-';
    DMP->TD.SVal +=
      ASTStringUtils::Instance().RemoveManglingTerminator(std::string_view(R));
    while (*R != 'E') R++;
    break;
  case RXMPUInt:        // Arbitrary precision unsigned integer.
    DMP->TD.Ty = ASTTypeMPUInteger;
    DMP->TD.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(B, '_', &J);
    DMP->TD.S = 'U';
    DMP->TD.L = true;
    DMP->TD.V = 'U';
    DMP->TD.SVal =
      ASTStringUtils::Instance().RemoveManglingTerminator(std::string_view(R));
    while (*R != 'E') R++;
    break;
  case RXFloat:         // 32-bit floating-point.
    DMP->TD.Ty = ASTTypeFloat;
    DMP->TD.SBits = ASTStringUtils::StringToSigned<int32_t>(B, '_', &J);
    DMP->TD.S = 'S';
    DMP->TD.L = true;
    DMP->TD.V = 'f';
    DMP->TD.FVal = std::stof(R);
    while (*R != 'E') R++;
    break;
  case RXDouble:        // 64-bit floating-point.
    DMP->TD.Ty = ASTTypeDouble;
    DMP->TD.SBits = ASTStringUtils::StringToSigned<int32_t>(B, '_', &J);
    DMP->TD.S = 'S';
    DMP->TD.L = true;
    DMP->TD.V = 'd';
    DMP->TD.DVal = std::stod(R);
    while (*R != 'E') R++;
    break;
  case RXLongDouble:    // 128-bit floating-point.
    DMP->TD.Ty = ASTTypeLongDouble;
    DMP->TD.SBits = ASTStringUtils::StringToSigned<int32_t>(B, '_', &J);
    DMP->TD.S = 'S';
    DMP->TD.L = true;
    DMP->TD.V = 'l';
    DMP->TD.LDVal = std::stold(R);
    while (*R != 'E') R++;
    break;
  case RXFloatNeg:      // Negative 32-bit floating-point.
    DMP->TD.Ty = ASTTypeFloat;
    DMP->TD.SBits = ASTStringUtils::StringToSigned<int32_t>(B, '_', &J);
    DMP->TD.S = 'S';
    DMP->TD.L = true;
    DMP->TD.V = 'f';
    DMP->TD.FVal = std::copysignf(std::stof(R), -1.0f);
    break;
  case RXDoubleNeg:     // Negative 64-bit floating-point.
    DMP->TD.Ty = ASTTypeDouble;
    DMP->TD.SBits = ASTStringUtils::StringToSigned<int32_t>(B, '_', &J);
    DMP->TD.S = 'S';
    DMP->TD.L = true;
    DMP->TD.V = 'd';
    DMP->TD.FVal = std::copysign(std::stod(R), -1.0);
    break;
  case RXLongDoubleNeg: // Negative 128-bit floating-point.
    DMP->TD.Ty = ASTTypeLongDouble;
    DMP->TD.SBits = ASTStringUtils::StringToSigned<int32_t>(B, '_', &J);
    DMP->TD.S = 'S';
    DMP->TD.L = true;
    DMP->TD.V = 'l';
    DMP->TD.LDVal = std::copysignl(std::stold(R), -1.0);
    break;
  case RXMPDecPos:      // Arbitrary precision decimal.
    DMP->TD.Ty = ASTTypeMPDecimal;
    if (!CN.empty())
      DMP->TD.Name = CN;

    while (!std::isdigit(*B))
      B++;

    DMP->TD.SBits = ASTStringUtils::StringToSigned<int32_t>(B, '_', &J);
    DMP->TD.S = 'S';
    DMP->TD.L = true;
    DMP->TD.V = 'D';

    while (*R == '_')
      R++;

    if (R[0] == 'n' && R[1] == 'a' && R[2] == 'n')
      DMP->TD.SVal = "NaN";
    else if (R[0] == 'i' && R[1] == 'n' && R[2] == 'f')
      DMP->TD.SVal = "Inf";
    else
      DMP->TD.SVal =
        ASTStringUtils::Instance().RemoveManglingTerminator(std::string_view(R));

    while (*R != 'E') R++;
    break;
  case RXMPDecNeg:      // Arbitrary precision negative decimal.
    DMP->TD.Ty = ASTTypeMPDecimal;
    if (!CN.empty())
      DMP->TD.Name = CN;

    while (!std::isdigit(*B))
      B++;

    DMP->TD.SBits = ASTStringUtils::StringToSigned<int32_t>(B, 'n', &J);
    DMP->TD.S = 'S';
    DMP->TD.L = true;
    DMP->TD.V = 'D';

    while (*R == '_')
      R++;

    DMP->TD.SVal = '-';

    if (R[0] == 'n' && R[1] == 'a' && R[2] == 'n')
      DMP->TD.SVal += "NaN";
    else if (R[0] == 'i' && R[1] == 'n' && R[2] == 'f')
      DMP->TD.SVal += "Inf";
    else
      DMP->TD.SVal +=
        ASTStringUtils::Instance().RemoveManglingTerminator(std::string_view(R));

    while (*R != 'E') R++;
    break;
  case RXMPComplex:     // Arbitrary precision complex.
    DMP->TD.Ty = ASTTypeMPComplex;
    DMP->TD.SBits = ASTStringUtils::StringToSigned<int32_t>(B, &J);
    DMP->TD.S = 'S';
    DMP->TD.L = true;
    DMP->TD.V = 'C';

    R = ResolveLiteralCReal(R, DMP);
    R = ResolveLiteralCImag(R, DMP);
    while (*R != 'E') R++;

    if (R[0] == 'E' && R[1] == 'g') {
      DMP->DTD1.Name = "cimag";
      R += 2U;
    }
    break;
  case RXString: {      // String literal.
    // FIXME: NOT FULLY IMPLEMENTED.
    B += 2U;
    DMP->TD.Ty = ASTTypeStringLiteral;
    DMP->TD.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(B, '_', &J);
    DMP->TD.S = 'U';
    DMP->TD.L = true;
    std::stringstream SC;
    SC.write(R, DMP->TD.UBits);
    DMP->TD.SVal = SC.str();
    R += DMP->TD.UBits;
  }
    break;
  case RXConstString: { // String literal.
    B += 2U;
    DMP->TD.Ty = ASTTypeStringLiteral;
    DMP->TD.UBits = ASTStringUtils::StringToUnsigned<uint32_t>(B, '_', &J);
    DMP->TD.S = 'U';
    DMP->TD.L = true;
    std::stringstream SC;
    SC.write(R, DMP->TD.UBits);
    DMP->TD.SVal = SC.str();
    R += DMP->TD.UBits;
  }
    break;
  default:
    break;
  }

  return R;
}

const char* ASTDemangler::ParseNumericLiteral(const char* S, ASTDemangled* DMP) {
  assert(S && "Invalid mangled string argument!");
  assert(DMP && "Invalid ASTDemangled argument!");

  if (*S != 'L')
    return S;

  const char* R = S + 1;

  R = ParseType(R, DMP);
  unsigned J;

  switch (DMP->TD.Ty) {
  case ASTTypeInt:
    DMP->TD.SIVal = ASTStringUtils::StringToSigned<int32_t>(R, 'E', &J);
    DMP->TD.V = 'i';
    R += J + 1;
    break;
  case ASTTypeUInt:
    DMP->TD.UIVal = ASTStringUtils::StringToUnsigned<uint32_t>(R, 'E', &J);
    DMP->TD.V = 'u';
    R += J + 1;
    break;
  case ASTTypeFloat:
    DMP->TD.UIVal = ASTStringUtils::StringToUnsigned<uint32_t>(R, 'E', &J);
    DMP->TD.V = 'F';
    R += J + 1;
    break;
  case ASTTypeDouble:
    DMP->TD.ULVal = ASTStringUtils::StringToUnsigned<uint64_t>(R, 'E', &J);
    DMP->TD.V = 'D';
    R += J + 1;
    break;
  case ASTTypeLongDouble: {
    SLD* LDS = reinterpret_cast<SLD*>(&DMP->TD.LDVal);
    LDS->X = ASTStringUtils::StringToUnsigned<uint64_t>(R, ':', &J);
    R += J + 1;
    LDS->Y = ASTStringUtils::StringToUnsigned<uint64_t>(R, 'E', &J);
    R += J + 1;
    DMP->TD.V = 'L';
  }
    break;
  default:
    break;
  }

  return R;
}

const char* ASTDemangler::ParseBinaryOp(const char* S, ASTDemangled* DMP,
                                        std::vector<ASTDemangled*>& VDM) {
  assert(S && "Invalid mangled string argument!");
  assert(DMP && "Invalid ASTDemangled argument!");

  const char* NP = S;
  NP = ValidateEndOfExpression(S);
  if (!NP) return nullptr;

  if (IsParamOrArgument(NP))
    return NP;

  if (DMP->TD.Ty != ASTTypeUndefined || DMP->TD.OpTy != ASTOpTypeUndefined)
    VDM.push_back(DMP);

  if (IsBinaryOp(NP)) {
    ASTDemangled* BODM = new ASTDemangled();
    assert(BODM && "Could not create a valid ASTDemangled!");
    BODM->TD.Ty = ASTTypeBinaryOp;
    BODM->TD.Name = "binaryop";
    NP += 3U;
    NP = ParseName(NP, BODM);
    if (IsBinaryOp(NP)) {
      NP = ParseBinaryOp(NP, BODM, VDM);
    } else if (IsUnaryOp(NP)) {
      NP = ParseUnaryOp(NP, BODM, VDM);
    }

    // VDM will be appended to VDMT after returning from this function.
    VDM.push_back(BODM);
  } else if (IsUnaryOp(NP)) {
    ASTDemangled* UODM = new ASTDemangled();
    assert(UODM && "Could not create a valid ASTDemangled!");
    UODM->TD.Ty = ASTTypeUnaryOp;
    UODM->TD.Name = "unaryop";
    NP += 3U;
    NP = ParseName(NP, UODM);
    if (IsBinaryOp(NP))
      NP = ParseBinaryOp(NP, UODM, VDM);
    else if (IsUnaryOp(NP))
      NP = ParseUnaryOp(NP, UODM, VDM);

    // VDM will be appended to VDMT after returning from this function.
    VDM.push_back(UODM);
  }

  while (NP) {
    if (IsParamOrArgument(NP))
      return NP;

    NP = ParseOpFoldType(NP, VDM);

    if (NP && IsOperator(NP)) {
      ASTDemangled* ODM = new ASTDemangled();
      assert(ODM && "Could not create a valid ASTDemangled!");

      NP = ParseOpType(NP, ODM, VDM);
      if (ODM->TD.OpTy != ASTOpTypeUndefined)
        VDM.push_back(ODM);
      else
        delete ODM;
    }

    if (NP) {
      ASTDemangled* DDM = new ASTDemangled();
      assert(DDM && "Could not create a valid ASTDemangled!");

      for (std::vector<ASTDemangled*>::iterator I = VDMT.begin();
           I != VDMT.end(); ++I) {
        DMS.insert(*I);
      }

      NP = ParseType(NP, DDM);
      std::pair<std::set<ASTDemangled*>::const_iterator, bool> I =
        DMS.insert(DDM);
      if (I.second) {
        VDM.push_back(DDM);
      }
    }

    while (*NP == u8'E')
      ++NP;

    if (NP) {
      ASTDemangled* ODM = new ASTDemangled();
      assert(ODM && "Could not create a valid ASTDemangled!");

      NP = ParseOpType(NP, ODM, VDM);
      if (ODM->TD.OpTy != ASTOpTypeUndefined)
        VDM.push_back(ODM);
      else
        delete ODM;
    }
  }

  if (!VDMT.empty()) {
    // Increment and decrement operators need to know
    // which operand they apply to.

    // 1. Post-operators.
    std::vector<ASTDemangled*>::iterator PI;
    for (std::vector<ASTDemangled*>::iterator VI = VDMT.begin();
         VI != VDMT.end(); ++VI) {
      if ((*VI)->TD.Ty == ASTTypeOpTy &&
          ((*VI)->TD.OpTy == ASTOpTypePostInc ||
           (*VI)->TD.OpTy == ASTOpTypePostDec)) {
        (*VI)->TD.Name = (*PI)->TD.Name;
      }

      PI = VI;
    }

    // 2. Pre-operators.
    std::vector<ASTDemangled*>::reverse_iterator RI;
    for (std::vector<ASTDemangled*>::reverse_iterator VI = VDMT.rbegin();
         VI != VDMT.rend(); ++VI) {
      if ((*VI)->TD.Ty == ASTTypeOpTy &&
          ((*VI)->TD.OpTy == ASTOpTypePreInc ||
           (*VI)->TD.OpTy == ASTOpTypePreDec)) {
        (*VI)->TD.Name = (*RI)->TD.Name;
      }

      RI = VI;
    }
  }

  return NP;
}

const char* ASTDemangler::ParseUnaryOp(const char* S, ASTDemangled* DMP,
                                       std::vector<ASTDemangled*>& VDM) {
  assert(S && "Invalid mangled string argument!");
  assert(DMP && "Invalid ASTDemangled argument!");

  const char* NP = S;
  NP = ValidateEndOfExpression(S);
  if (!NP) return nullptr;

  if (IsParamOrArgument(NP))
    return NP;

  if (IsUnaryOp(NP)) {
    NP += 3U;
    NP = ParseName(NP, DMP);

    ASTOpType OTy = ASTOpTypeNone;

    if (NP[0] == 'T') {
      if (NP[1] == 's' && NP[2] == 'q') {
        OTy = ASTOpTypeSqrt;
        NP += 3;
      } else if (NP[1] == 'a' && NP[2] == 's') {
        OTy = ASTOpTypeArcSin;
        NP += 3;
      } else if (NP[1] == 'a' && NP[2] == 'c') {
        OTy = ASTOpTypeArcCos;
        NP += 3;
      } else if (NP[1] == 'a' && NP[2] == 't') {
        OTy = ASTOpTypeArcTan;
        NP += 3;
      } else if (NP[1] == 'e' && NP[2] == 'x') {
        OTy = ASTOpTypeExp;
        NP += 3;
      } else if (NP[1] == 'l' && NP[2] == 'n') {
        OTy = ASTOpTypeLn;
        NP += 3;
      } else if (NP[1] == 's') {
        OTy = ASTOpTypeSin;
        NP += 2;
      } else if (NP[1] == 'c') {
        NP += 2;
        OTy = ASTOpTypeCos;
      } else if (NP[1] == 't') {
        NP += 2;
        OTy = ASTOpTypeTan;
      }
    } else if (NP[0] == 'R') {
      if (NP[1] == 'o' && NP[2] == 't') {
        OTy = ASTOpTypeRotation;
        NP += 3;
      } else if (NP[1] == 't' && NP[2] == 'l') {
        OTy = ASTOpTypeRotl;
        NP += 3;
      } else if (NP[1] == 't' && NP[2] == 'r') {
        OTy = ASTOpTypeRotr;
        NP += 3;
      }
    } else if (NP[0] == 'P' && NP[1] == 'p' && NP[2] == 'c') {
      OTy = ASTOpTypePopcount;
      NP += 3;
    }

    if (OTy == ASTOpTypeNone) {
      std::stringstream MS;
      MS << "Unknown UnaryOp Type " << PrintOpTypeEnum(OTy) << '.';
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), MS.str(), DiagLevel::Error);
    }

    static std::map<ASTOpType, Mangler::MToken> OTMM = {
      { ASTOpTypeSin, { "sin", 3 }, },
      { ASTOpTypeCos, { "cos", 3 }, },
      { ASTOpTypeTan, { "tan", 3 }, },
      { ASTOpTypeArcSin, { "arcsin", 6 }, },
      { ASTOpTypeArcCos, { "arccos", 6 }, },
      { ASTOpTypeArcTan, { "arctan", 6 }, },
      { ASTOpTypeExp, { "exp", 3 }, },
      { ASTOpTypeLn, { "ln", 2 }, },
      { ASTOpTypeSqrt, { "sqrt", 4 }, },
      { ASTOpTypeRotation, { "rot", 3 }, },
      { ASTOpTypeRotl, { "rotl", 4 }, },
      { ASTOpTypeRotr, { "rotr", 4 }, },
      { ASTOpTypePopcount, { "popcount", 8 }, },
    };

    ASTDemangled* DMM0 = new ASTDemangled();
    assert(DMM0 && "Could not create a valid ASTDemangled!");
    DMM0->TD.Ty = ASTTypeOpTy;
    DMM0->TD.OpTy = OTy;
    DMM0->TD.Name = OTMM[OTy].Token();
    VDMT.push_back(DMM0);

    if (NP[0] == 'f' && NP[1] == 'l') {
      ASTDemangled* DMM1 = new ASTDemangled();
      assert(DMM1 && "Could not create a valid ASTDemangled!");
      DMM1->TD.Ty = ASTTypeOpTy;
      DMM1->TD.OpTy = ASTOpTypeUnaryLeftFold;
      DMM1->TD.Name = '(';
      VDMT.push_back(DMM1);
      NP += 2;
    }

    ASTDemangled* DMM2 = new ASTDemangled();
    assert(DMM2 && "Could not create a valid ASTDemangled!");

    NP = ParseType(NP, DMM2);
    VDMT.push_back(DMM2);

    while (*NP == u8'E')
      ++NP;

    if (OTy == ASTOpTypeRotl || OTy == ASTOpTypeRotr) {
      ASTDemangled* DMMI = new ASTDemangled();
      assert(DMMI && "Could not create a valid ASTDemangled!");

      NP = ParseType(NP, DMMI);
      DMMI->TD.UBits = ASTIntNode::IntBits;
      NP += 1;
      unsigned J = 0U;
      DMMI->TD.SIVal = ASTStringUtils::StringToSigned<int32_t>(NP, &J);
      DMMI->TD.SVal = std::to_string(DMMI->TD.SIVal);
      VDMT.push_back(DMMI);
      NP += J;
    }

    while (*NP == u8'E')
      ++NP;

    if (NP[0] == 'f' && NP[1] == 'r') {
      ASTDemangled* DMM3 = new ASTDemangled();
      assert(DMM3 && "Could not create a valid ASTDemangled!");
      DMM3->TD.Ty = ASTTypeOpTy;
      DMM3->TD.OpTy = ASTOpTypeUnaryRightFold;
      DMM3->TD.Name = ')';
      VDMT.push_back(DMM3);
      NP += 2;
    }
  }

  return NP;
}

const char* ASTDemangler::ParseMeasure(const char* S, ASTDemangled* DMP) {
  assert(S && "Invalid mangled string argument!");
  assert(DMP && "Invalid ASTDemangled argument!");

  const char* NP = S;

  // Measure Qubit targets.
  while (NP[0] == 'Q' && NP[1] == 'C' && std::isdigit(NP[2])) {
    ASTDemangled* DDMP = new ASTDemangled();
    assert(DDMP && "Could not create a valid ASTDemangled!");

    NP = ParseType(NP, DDMP);
    NP = ParseName(NP, DDMP);
    VDMT.push_back(DDMP);
  }

  // Target and Result vectors.
  unsigned J;
  unsigned X = 0U;

  do {
    if (NP[0] == 'H') {
      ASTDemangled* HDMP = new ASTDemangled();
      assert(HDMP && "Could not create a valid ASTDemangled!");

      NP = ParseType(NP, HDMP);
      HDMP->TD.ULVal = ASTStringUtils::HexToUnsigned<uint64_t>(NP, 16U, &J);
      HDMP->TD.V = 'U';
      HDMP->TD.UBits = sizeof(uint64_t) * CHAR_BIT;
      NP += J + 2;
      VDMT.push_back(HDMP);
    }

    while (NP[0] == 'j') {
      ASTDemangled* JDMP = new ASTDemangled();
      assert(JDMP && "Could not create a valid ASTDemangled!");

      NP = ParseType(NP, JDMP);
      JDMP->TD.UIVal = ASTStringUtils::HexToUnsigned<uint32_t>(NP, 8U, &J);
      JDMP->TD.V = 'u';
      JDMP->TD.UBits = sizeof(uint32_t) * CHAR_BIT;
      NP += J + 2;
      VDMT.push_back(JDMP);
      NP = ValidateEndOfExpression(NP);
    }

    X += 1U;
  } while (X < 2U);

  // Measure result.
  ASTDemangled* DDMP = new ASTDemangled();
  assert(DDMP && "Could not create a valid ASTDemangled!");

  NP = ParseType(NP, DDMP);
  NP = ParseName(NP, DDMP);

  VDMT.push_back(DDMP);
  return NP = ValidateEndOfExpression(NP);
}

const char* ASTDemangler::ParseDefcalMeasure(const char* S, ASTDemangled* DMP) {
  assert(S && "Invalid mangled string argument!");
  assert(DMP && "Invalid ASTDemangled argument!");

  ASTDemangled DMM;
  const char* NP = S;

  // Defcal Grammar.
  NP = ParseType(NP, &DMM);
  NP = ParseName(NP, &DMM);
  DMP->DTD0.Ty = DMM.TD.Ty;
  DMP->DTD0.Name = DMM.TD.Name;
  DMP->DTD0.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMM.TD.Ty);

  // Defcal Measure.
  ASTDemangled* DMMP = new ASTDemangled();
  assert(DMMP && "Could not create a valid ASTDemangled!");

  NP = ParseType(NP, DMMP);
  VDMT.insert(VDMT.begin(), DMMP);

  // Defcal return.
  ASTDemangled* TDMP = new ASTDemangled();
  assert(TDMP && "Could not create a valid ASTDemangled!");

  NP = ParseType(NP, TDMP);
  VDMT.push_back(TDMP);

  ASTDemangled PDMP;

  // Defcal params.
  while (NP[0] == 'D' && NP[1] == 'p' && std::isdigit(NP[2])) {
    unsigned J;
    ASTDemangled* DDMP = new ASTDemangled();
    assert(DDMP && "Could not create a valid ASTDemangled!");

    DDMP->TD.Ty = ASTTypeDefcalParam;
    DDMP->TD.IX = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
    NP += 3 + J;

    PDMP.Clear();
    NP = ParseType(NP, &PDMP);
    NP = ParseName(NP, &PDMP);
    DDMP->DTD0.Ty = PDMP.TD.Ty;
    DDMP->DTD0.Name = PDMP.TD.Name;
    DDMP->DTD0.UBits = PDMP.TD.UBits;
    VDMT.push_back(DDMP);
  }

  VDMT.push_back(nullptr);
  return NP = ValidateEndOfExpression(NP);
}

const char* ASTDemangler::ParseDefcalReset(const char* S, ASTDemangled* DMP) {
  assert(S && "Invalid mangled string argument!");
  assert(DMP && "Invalid ASTDemangled argument!");

  ASTDemangled DMM;
  const char* NP = S;

  // Defcal Grammar.
  NP = ParseType(NP, &DMM);
  NP = ParseName(NP, &DMM);
  DMP->DTD0.Ty = DMM.TD.Ty;
  DMP->DTD0.Name = DMM.TD.Name;
  DMP->DTD0.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMM.TD.Ty);

  // Defcal Reset.
  ASTDemangled* DMR = new ASTDemangled();
  assert(DMR && "Could not create a valid ASTDemangled!");

  NP = ParseType(NP, DMR);
  NP = ParseName(NP, DMR);
  DMR->TD.UBits = ASTTypeSystemBuilder::Instance().GetTypeBits(DMM.TD.Ty);
  VDMT.push_back(DMR);

  std::vector<ASTDemangled> DMV;

  // Reset Qubit targets.
  while (NP[0] == 'Q' && NP[1] == 'C' && std::isdigit(NP[2])) {
    DMM.Clear();
    NP = ParseType(NP, &DMM);
    NP = ParseName(NP, &DMM);
    DMV.push_back(DMM);
    NP = ValidateEndOfExpression(NP);
  }

  ASTDemangled TDMP;
  unsigned J = 0U;
  unsigned X = 0U;

  // Defcal params.
  while (NP[0] == 'D' && NP[1] == 'p' && std::isdigit(NP[2])) {
    ASTDemangled* DDMP = new ASTDemangled();
    assert(DDMP && "Could not create a valid ASTDemangled!");

    DDMP->TD.Ty = ASTTypeDefcalParam;
    DDMP->TD.Name = ASTStringUtils::Instance().Substring(NP, '_');
    DDMP->TD.IX = ASTStringUtils::StringToUnsigned<uint32_t>(&NP[2], '_', &J);
    NP += 3 + J;

    TDMP.Clear();
    NP = ParseType(NP, &TDMP);
    NP = ParseName(NP, &TDMP);
    assert((DMV[X].TD.Ty == TDMP.TD.Ty && DMV[X].TD.Name == TDMP.TD.Name) &&
           "Ordering mismatch between reset parameters and defcal parameters!");

    DDMP->DTD0 = TDMP.TD;
    VDMT.push_back(DDMP);
    X += 1U;
  }

  VDMT.push_back(nullptr);
  return NP = ValidateEndOfExpression(NP);
}

const char*
ASTDemangler::ParseFunctionDefinition(const char* S, ASTDemangled* DMP,
                                      bool Extern) {
  // FIXME: IMPLEMENT.
  assert(S && "Invalid mangled string argument!");
  assert(DMP && "Invalid ASTDemangled argument!");

  // Silence compiler warnings.
  (void) Extern;
  ASTDemangled DMM;
  const char* NP = S;
  return NP;
}

const char*
ASTDemangler::ParseFunctionDeclaration(const char* S, ASTDemangled* DMP,
                                       bool Extern) {
  // FIXME: IMPLEMENT.
  assert(S && "Invalid mangled string argument!");
  assert(DMP && "Invalid ASTDemangled argument!");

  // Silence compiler warnings.
  (void) Extern;
  ASTDemangled DMM;
  const char* NP = S;
  return NP;
}

const char*
ASTDemangler::ParseDurationOfTarget(const char* S, ASTDemangled* DMP) {
  assert(S && "Invalid mangled string argument!");
  assert(DMP && "Invalid ASTDemangled argument!");

  std::regex GR(u8"^G[0-9]+[a-zA-Z_]+(.*)");
  std::regex GCR(u8"^GC[0-9]+[a-zA-Z_]+(.*)");
  std::regex DR(u8"^D[0-9]+[a-zA-Z_]+(.*)");
  std::regex DMR(u8"^DM[0-9]+[a-zA-Z_]+(.*)");
  std::regex DRR(u8"^DR[0-9]+[a-zA-Z_]+(.*)");
  std::regex DCR(u8"^DC[0-9]+[a-zA-Z_]+(.*)");
  std::regex DMCR(u8"^DMC[0-9]+[a-zA-Z_]+(.*)");
  std::regex DRCR(u8"^DRC[0-9]+[a-zA-Z_]+(.*)");

  std::regex GP(u8"^Gp[0-9]+_(.*)");
  std::regex GA(u8"^Ga[0-9]+_(.*)");
  std::regex DP(u8"^Dp[0-9]+_(.*)");
  std::regex DA(u8"^Da[0-9]+_(.*)");

  std::smatch SM;
  std::string MS = S;
  const char* NP = S;

  if (std::regex_match(MS, SM, GR)) {
    NP = ParseType(NP, DMP);
    NP = ParseName(NP, DMP);
    MS = NP ? NP : "";
    while (!MS.empty() && std::regex_match(MS, SM, GP)) {
      ASTDemangled* DDM = new ASTDemangled();
      assert(DDM && "Could not create a valid ASTDemangled!");

      NP = ParseType(NP, DDM);
      NP = ParseName(NP, DDM);
      VDMT.push_back(DDM);

      if (IsBinaryOrUnaryOp(NP)) {
        ASTDemangled DMA;
        if (IsBinaryOp(NP)) {
          std::vector<ASTDemangled*> DMV;
          NP = ParseBinaryOp(NP, &DMA, DMV);
          VDMT.insert(VDMT.end(), DMV.begin(), DMV.end());
          assert(VDMT.size() >= DMV.size() && "Vector insertion failed!");
        } else {
          NP = ParseType(NP, &DMA);
        }

        if (VDMT.size() && !VDMT.back())
          VDMT.pop_back();
      } else {
        ASTDemangled* DMA = new ASTDemangled();
        assert(DMA && "Could not create a valid ASTDemangled!");
        NP = ParseType(NP, DMA);
        NP = ParseName(NP, DMA);

        if (VDMT.size() && !VDMT.back())
          VDMT.pop_back();

        VDMT.push_back(DMA);
      }

      NP = SkipExpressionTerminator(NP);
      MS = NP ? NP : "";
    }
  } else if (std::regex_match(MS, SM, GCR)) {
    NP = ParseType(NP, DMP);
    NP = ParseName(NP, DMP);
    MS = NP ? NP : "";
    while (!MS.empty() && std::regex_match(MS, SM, GA)) {
      ASTDemangled* DDM = new ASTDemangled();
      assert(DDM && "Could not create a valid ASTDemangled!");

      NP = ParseType(NP, DDM);
      NP = ParseName(NP, DDM);
      VDMT.push_back(DDM);

      if (IsBinaryOrUnaryOp(NP)) {
        ASTDemangled DMA;
        if (IsBinaryOp(NP)) {
          std::vector<ASTDemangled*> DMV;
          NP = ParseBinaryOp(NP, &DMA, DMV);
          VDMT.insert(VDMT.end(), DMV.begin(), DMV.end());
          assert(VDMT.size() >= DMV.size() && "Vector insertion failed!");
        } else {
          NP = ParseType(NP, &DMA);
        }

        if (VDMT.size() && !VDMT.back())
          VDMT.pop_back();
      } else {
        ASTDemangled* DMA = new ASTDemangled();
        assert(DMA && "Could not create a valid ASTDemangled!");
        NP = ParseType(NP, DMA);
        NP = ParseName(NP, DMA);

        if (VDMT.size() && !VDMT.back())
          VDMT.pop_back();

        VDMT.push_back(DMA);
      }

      NP = SkipExpressionTerminator(NP);
      MS = NP ? NP : "";
    }
  } else if (std::regex_match(MS, SM, DR)) {
    NP = ParseType(NP, DMP);
    NP = ParseName(NP, DMP);
    MS = NP ? NP : "";
    while (!MS.empty() && std::regex_match(MS, SM, DP)) {
      ASTDemangled* DDM = new ASTDemangled();
      assert(DDM && "Could not create a valid ASTDemangled!");

      NP = ParseType(NP, DDM);
      NP = ParseName(NP, DDM);
      VDMT.push_back(DDM);

      if (IsBinaryOrUnaryOp(NP)) {
        ASTDemangled DMA;
        if (IsBinaryOp(NP)) {
          std::vector<ASTDemangled*> DMV;
          NP = ParseBinaryOp(NP, &DMA, DMV);
          VDMT.insert(VDMT.end(), DMV.begin(), DMV.end());
          assert(VDMT.size() >= DMV.size() && "Vector insertion failed!");
        } else {
          NP = ParseType(NP, &DMA);
        }

        if (VDMT.size() && !VDMT.back())
          VDMT.pop_back();
      } else {
        ASTDemangled* DMA = new ASTDemangled();
        assert(DMA && "Could not create a valid ASTDemangled!");
        NP = ParseType(NP, DMA);
        NP = ParseName(NP, DMA);

        if (VDMT.size() && !VDMT.back())
          VDMT.pop_back();

        VDMT.push_back(DMA);
      }

      NP = SkipExpressionTerminator(NP);
      MS = NP ? NP : "";
    }
  } else if (std::regex_match(MS, SM, DCR)) {
    NP = ParseType(NP, DMP);
    NP = ParseName(NP, DMP);

    if (NP[0] == u8'D' && NP[1] == u8'G' && NP[2] == u8'M') {
      ASTDemangled DGM;
      NP = ParseType(NP, &DGM);
      NP = ParseName(NP, &DGM);
      DMP->DTD0 = DGM.TD;
    }

    MS = NP ? NP : "";
    while (!MS.empty() && std::regex_match(MS, SM, DA)) {
      ASTDemangled* DDM = new ASTDemangled();
      assert(DDM && "Could not create a valid ASTDemangled!");

      if (IsParamOrArgument(NP)) {
        NP = ParseType(NP, DDM);
      } else {
        NP = ParseType(NP, DDM);
        NP = ParseName(NP, DDM);
        VDMT.push_back(DDM);

        if (IsBinaryOrUnaryOp(NP)) {
          ASTDemangled DMA;
          if (IsBinaryOp(NP)) {
            std::vector<ASTDemangled*> DMV;
            NP = ParseBinaryOp(NP, &DMA, DMV);
            VDMT.insert(VDMT.end(), DMV.begin(), DMV.end());
            assert(VDMT.size() >= DMV.size() && "Vector insertion failed!");
          } else {
            NP = ParseType(NP, &DMA);
          }

          if (VDMT.size() && !VDMT.back())
            VDMT.pop_back();
        } else {
          ASTDemangled* DMA = new ASTDemangled();
          assert(DMA && "Could not create a valid ASTDemangled!");
          NP = ParseType(NP, DMA);
          NP = ParseName(NP, DMA);

          if (VDMT.size() && !VDMT.back())
            VDMT.pop_back();

          VDMT.push_back(DMA);
        }
      }

      NP = SkipExpressionTerminator(NP);
      MS = NP ? NP : "";
    }
  } else if (std::regex_match(MS, SM, DMCR)) {
    NP = ParseType(NP, DMP);
    NP = ParseName(NP, DMP);

    if (NP[0] == u8'D' && NP[1] == u8'G' && NP[2] == u8'M') {
      ASTDemangled DGM;
      NP = ParseType(NP, &DGM);
      NP = ParseName(NP, &DGM);
      DMP->DTD0 = DGM.TD;
    }

    MS = NP ? NP : "";
    while (!MS.empty() && std::regex_match(MS, SM, DA)) {
      ASTDemangled* DDM = new ASTDemangled();
      assert(DDM && "Could not create a valid ASTDemangled!");

      if (IsParamOrArgument(NP)) {
        NP = ParseType(NP, DDM);
      } else {
        NP = ParseType(NP, DDM);
        NP = ParseName(NP, DDM);
        VDMT.push_back(DDM);

        if (IsBinaryOrUnaryOp(NP)) {
          ASTDemangled DMA;
          if (IsBinaryOp(NP)) {
            std::vector<ASTDemangled*> DMV;
            NP = ParseBinaryOp(NP, &DMA, DMV);
            VDMT.insert(VDMT.end(), DMV.begin(), DMV.end());
            assert(VDMT.size() >= DMV.size() && "Vector insertion failed!");
          } else {
            NP = ParseType(NP, &DMA);
          }

          if (VDMT.size() && !VDMT.back())
            VDMT.pop_back();
        } else {
          ASTDemangled* DMA = new ASTDemangled();
          assert(DMA && "Could not create a valid ASTDemangled!");
          NP = ParseType(NP, DMA);
          NP = ParseName(NP, DMA);

          if (VDMT.size() && !VDMT.back())
            VDMT.pop_back();

          VDMT.push_back(DMA);
        }
      }

      NP = SkipExpressionTerminator(NP);
      MS = NP ? NP : "";
    }
  } else if (std::regex_match(MS, SM, DRCR)) {
    NP = ParseType(NP, DMP);
    NP = ParseName(NP, DMP);

    if (NP[0] == u8'D' && NP[1] == u8'G' && NP[2] == u8'M') {
      ASTDemangled DGM;
      NP = ParseType(NP, &DGM);
      NP = ParseName(NP, &DGM);
      DMP->DTD0 = DGM.TD;
    }

    MS = NP ? NP : "";
    while (!MS.empty() && std::regex_match(MS, SM, DA)) {
      ASTDemangled* DDM = new ASTDemangled();
      assert(DDM && "Could not create a valid ASTDemangled!");

      if (IsParamOrArgument(NP)) {
        NP = ParseType(NP, DDM);
      } else {
        NP = ParseType(NP, DDM);
        NP = ParseName(NP, DDM);
        VDMT.push_back(DDM);

        if (IsBinaryOrUnaryOp(NP)) {
          ASTDemangled DMA;
          if (IsBinaryOp(NP)) {
            std::vector<ASTDemangled*> DMV;
            NP = ParseBinaryOp(NP, &DMA, DMV);
            VDMT.insert(VDMT.end(), DMV.begin(), DMV.end());
            assert(VDMT.size() >= DMV.size() && "Vector insertion failed!");
          } else {
            NP = ParseType(NP, &DMA);
          }

          if (VDMT.size() && !VDMT.back())
            VDMT.pop_back();
        } else {
          ASTDemangled* DMA = new ASTDemangled();
          assert(DMA && "Could not create a valid ASTDemangled!");
          NP = ParseType(NP, DMA);
          NP = ParseName(NP, DMA);

          if (VDMT.size() && !VDMT.back())
            VDMT.pop_back();

          VDMT.push_back(DMA);
        }
      }

      NP = SkipExpressionTerminator(NP);
      MS = NP ? NP : "";
    }
  } else {
    NP = ParseType(NP, DMP);
    NP = ParseName(NP, DMP);
    NP = SkipExpressionTerminator(NP);
    MS = NP ? NP : "";
  }

  return NP;
}

bool ASTDemangler::Demangle(const std::string& N) {
  if (N.empty() || N.length() < 3) {
    std::stringstream M;
    M << "Invalid mangled string '" << N << "'.";
    QasmDiagnosticEmitter::Instance().EmitDiagnostic(
      DIAGLineCounter::Instance().GetLocation(), M.str(),
                                                 DiagLevel::Error);
    return false;
  }

  if (!N.empty()) {
    const char* NP = N.c_str();
    if (NP[0] != u8'_' && NP[1] != u8'Q') {
      std::stringstream M;
      M << "Malformed mangled string does not begin with the Quantum "
        << "mangling sequence (_Q).";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return false;
    }

    if (N[N.length() - 3] != u8'E' && N[N.length() - 2] != u8'_') {
      std::stringstream M;
      M << "Malformed mangled string does not end with the Quantum "
        << "mangling sequence (E_).";
      QasmDiagnosticEmitter::Instance().EmitDiagnostic(
        DIAGLineCounter::Instance().GetLocation(), M.str(), DiagLevel::Error);
      return false;
    }


    NP += 2;

    if (NP[0] == ':' && NP[1] == 'C' && NP[2] == ':') {
      InCalBlock = true;
      NP += 3;
    }

    const char* P = ParseType(NP, &DM);
    P = ParseName(P, &DM);

    while (P && *P) {
      if (!(P = ValidateEndOfExpression(P))) {
        if (VDMT.size())
          VDMT.push_back(nullptr);

        return true;
      }

      ASTDemangled* DDM = new ASTDemangled();
      assert(DDM && "Could not create a valid ASTDemangled instance!");

      P = ParseType(P, DDM);

      if (IsParamOrArgument(DDM->TD.Ty)) {
        VDMT.push_back(DDM);
        continue;
      }

      P = ParseName(P, DDM);
      VDMT.push_back(DDM);
    }

    if (VDMT.size() && VDMT.back())
      VDMT.push_back(nullptr);

    return true;
  }

  VDMT.push_back(nullptr);
  return false;
}

std::string ASTDemangler::AsString() {
  if (DM.TD.Ty == ASTTypeUndefined)
    return "<Unknown Type>";

  std::stringstream SR;
  bool SeenCalBlock = false;

  if (InCalBlock && !SeenCalBlock) {
    SR << "cal { ";
    SeenCalBlock = true;
  }

  switch (DM.TD.Ty) {
  case ASTTypeDefcal:
    DeserializeDefcal(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeDefcalMeasure:
    DeserializeDefcalMeasure(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeDefcalReset:
    DeserializeDefcalReset(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeGate:
    DeserializeGate(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeFunction:
    DeserializeFunction(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeKernel:
    DeserializeKernel(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeDelay:
    DeserializeNonScalar(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeBool:
  case ASTTypeChar:
  case ASTTypeUTF8:
  case ASTTypeInt:
  case ASTTypeUInt:
  case ASTTypeFloat:
  case ASTTypeDouble:
  case ASTTypeLongDouble:
  case ASTTypePointer:
  case ASTTypeVoid:
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
  case ASTTypeMPDecimal:
    DeserializeScalar(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeMPComplex:
    DeserializeNonScalar(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeQubit:
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias:
  case ASTTypeGateQubitParam:
    DeserializeQubit(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeDuration:
    DeserializeNonScalar(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeAngle:
    DeserializeNonScalar(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeBitset:
    DeserializeNonScalar(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeBinaryOp:
    DeserializeBinaryOp(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeUnaryOp:
    DeserializeUnaryOp(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeArray:
  case ASTTypeAngleArray:
  case ASTTypeBoolArray:
  case ASTTypeCBitArray:
  case ASTTypeDurationArray:
  case ASTTypeFloatArray:
  case ASTTypeIntArray:
  case ASTTypeUIntArray:
  case ASTTypeMPComplexArray:
  case ASTTypeMPDecimalArray:
  case ASTTypeMPIntegerArray:
  case ASTTypeMPUIntegerArray:
  case ASTTypeOpenPulseFrameArray:
  case ASTTypeOpenPulsePortArray:
  case ASTTypeQubitArray:
    DeserializeArray(SR, DM.TD.Ty);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  case ASTTypeBox:
  case ASTTypeBoxAs:
  case ASTTypeBoxTo:
    DeserializeNonScalar(SR);
    if (InCalBlock)
      SR <<  " }";
    return SR.str();
    break;
  default:
    // FIXME IMPLEMENT:
    break;
  }

  return "";
}

void ASTDemangler::DeserializeDefcal(std::stringstream& SR) {
  bool PC = false;

  SR << TDMM[DM.TD.Ty].Token() << ' ';

  if (VDMT[0]->TD.Ty == ASTTypeDefcalGrammar)
    SR << "\"" << VDMT[0]->TD.Name << "\" ";

  SR << DM.TD.Name;
  bool LP = false;
  bool RP = false;

  std::vector<ASTDemangled*>::const_iterator B = VDMT.begin() + 1;
  std::vector<ASTDemangled*>::const_iterator E = VDMT.end() - 1;
  unsigned I = 1U;
  unsigned QC = 0U;

  for (std::vector<ASTDemangled*>::const_iterator DI = B; DI < E; ++DI) {
    PC = false;
    if (const ASTDemangled* DMP = (*DI)) {
      if (!ASTUtils::Instance().IsQubitType(DMP->TD.Ty)) {
        if (!LP) {
          SR << '(';
          LP = true;
        }

        switch (DMP->TD.Ty) {
        case ASTTypeBool:
        case ASTTypeInt:
        case ASTTypeUInt:
        case ASTTypeFloat:
        case ASTTypeDouble:
        case ASTTypeLongDouble:
        case ASTTypeChar:
        case ASTTypeUTF8:
        case ASTTypeDuration:
        case ASTTypeLength:
        case ASTTypeMeasure:
        case ASTTypeReset:
        case ASTTypeBox:
        case ASTTypeBoxAs:
        case ASTTypeBoxTo:
        case ASTTypeStretch:
        case ASTTypeOpenPulseFrame:
        case ASTTypeOpenPulsePort:
          SR << TDMM[DMP->TD.Ty].Token();
          PC = true;
          break;
        case ASTTypeQubit:
        case ASTTypeQubitContainer:
        case ASTTypeQubitContainerAlias:
        case ASTTypeGateQubitParam:
          SR << ' ' << DMP->TD.Name;
          PC = true;
          break;
        case ASTTypeBinaryOp: {
          PC = false;
          std::vector<ASTDemangled*>::const_iterator DP = DI;
          while ((*DP) && DP != E) {
            if ((*DP)->TD.Ty == ASTTypeDefcalParam ||
                (*DP)->TD.Ty == ASTTypeDefcalArg) {
              DeserializeBinaryOp(SR, DI, DP);
              break;
            } else {
              ++DP;
              ++I;
            }
          }

          DI = DP;
          PC = true;
        }
          break;
        case ASTTypeUnaryOp: {
          PC = false;
          std::vector<ASTDemangled*>::const_iterator DP = DI;
          while ((*DP) && DP != E) {
            if ((*DP)->TD.Ty == ASTTypeDefcalParam ||
                (*DP)->TD.Ty == ASTTypeDefcalArg) {
              DeserializeUnaryOp(SR, DI, DP);
              break;
            } else {
              ++DP;
              ++I;
            }
          }

          DI = DP;
          PC = true;
        }
          break;
        default:
          if (DMP->TD.Ty != ASTTypeUndefined && !IsParamOrArgument(DMP->TD.Ty)) {
            SR << TDMM[DMP->TD.Ty].Token() << '[' << DMP->TD.UBits << ']';
            if (!DMP->TD.Name.empty())
              SR << ' ' << DMP->TD.Name;
          }
          break;
        }

        if (I < VDMT.size() - 1) {
          if (const ASTDemangled* DMN = VDMT.at(I + 1)) {
            if (ASTUtils::Instance().IsQubitType(DMN->TD.Ty)) {
              // We need UTF8 stringstreams to do this more efficiently.
              ASTStringUtils::Instance().EraseLastIfBlank(SR);

              if (!RP) {
                SR << ')';
                RP = true;
              } else {
                SR << ',';
              }
            } else if (ASTUtils::Instance().IsRightParenType(DMN->TD.OpTy) ||
                       ASTUtils::Instance().IsLeftParenType(DMN->TD.OpTy)) {
              continue;
            } else {
              if (PC) {
                ASTStringUtils::Instance().Backtrack(SR);
                SR << ", ";
              }

              continue;
            }
          }
        }
      } else {
        if (QC == 0U)
          SR << ')';
        ++QC;

        if (ASTStringUtils::Instance().IsIndexedQubit(DMP->TD.Name)) {
          SR << ' ' << ASTStringUtils::Instance().BracketedQubit(DMP->TD.Name);
        } else {
          SR << ' ' << DMP->TD.Name;
        }

        if (I < VDMT.size() - 1) {
          if (const ASTDemangled* DMT = VDMT.at(I + 1)) {
            if (ASTUtils::Instance().IsQubitType(DMT->TD.Ty))
              SR << ',';
          }
        }
      }
    }

    ++I;
  }
}

void ASTDemangler::DeserializeDefcalMeasure(std::stringstream& SR) {
  SR << TDMM[DM.TD.Ty].Token() << ' ';

  if (DM.DTD0.Ty == ASTTypeDefcalGrammar)
    SR << '"' << DM.DTD0.Name << "\" ";

  SR << "measure";
  bool RS = false;

  for (unsigned I = 1; I < VDMT.size(); ++I) {
    if (const ASTDemangled* DMP = VDMT.at(I)) {
      if (!ASTUtils::Instance().IsQubitType(DMP->TD.Ty)) {
        switch (DMP->TD.Ty) {
        case ASTTypeReturn:
          SR << " -> ";
          SR << TDMM[DMP->DTD0.Ty].Token();
          if (DMP->DTD0.IX != static_cast<unsigned>(~0x0))
            SR << '[' << DMP->DTD0.IX << ']';
          else if (DMP->DTD0.UBits > 1U)
            SR << '[' << DMP->DTD0.UBits << ']';
          RS = true;
          break;
        case ASTTypeInt:
        case ASTTypeUInt:
          if (RS) {
            SR << TDMM[DMP->TD.Ty].Token() << ' ' << DMP->TD.Name;
            if (const ASTDemangled* NDMP = VDMT.at(I + 1)) {
              if (NDMP->TD.Ty == ASTTypeDefcalParam)
                SR << ", ";
            } else {
              SR << ' ';
            }
          }
          break;
        case ASTTypeAngle:
        case ASTTypeMPComplex:
        case ASTTypeMPInteger:
        case ASTTypeMPDecimal:
        case ASTTypeBitset:
          if (RS) {
            SR << TDMM[DMP->TD.Ty].Token();
            if (DMP->TD.IX != static_cast<unsigned>(~0x0))
              SR << '[' << DMP->TD.IX << ']';
            else if (DMP->TD.UBits > 1U)
              SR << '[' << DMP->TD.UBits << ']';
          }
          break;
        default:
          continue;
          break;
        }
      } else {
        if (ASTStringUtils::Instance().IsIndexedQubit(DMP->TD.Name)) {
          SR << ASTStringUtils::Instance().BracketedQubit(DMP->TD.Name);
        } else {
          SR << ' ' << DMP->TD.Name;
        }

        if (I < VDMT.size() - 1) {
          if (const ASTDemangled* DMT = VDMT.at(I + 1)) {
            if (DMT->TD.Ty == ASTTypeDefcalParam)
              SR << ',';
          }
        }
      }
    }
  }
}

void ASTDemangler::DeserializeDefcalReset(std::stringstream& SR) {
  SR << TDMM[DM.TD.Ty].Token() << ' ';

  if (DM.DTD0.Ty == ASTTypeDefcalGrammar)
    SR << '"' << DM.DTD0.Name << "\" ";

  SR << "reset";

  for (unsigned I = 1; I < VDMT.size(); ++I) {
    if (const ASTDemangled* DMP = VDMT.at(I)) {
      if (DMP->TD.Ty == ASTTypeDefcalParam) {
        if (ASTStringUtils::Instance().IsIndexedQubit(DMP->DTD0.Name)) {
          SR << ' ' << ASTStringUtils::Instance().BracketedQubit(DMP->DTD0.Name);
        } else {
          SR << ' ' << DMP->DTD0.Name;
        }

        if (I < VDMT.size() - 1) {
          if (const ASTDemangled* DMT = VDMT.at(I + 1)) {
            if (DMT->TD.Ty == ASTTypeDefcalParam)
              SR << ',';
          }
        }
      }
    }
  }
}

void ASTDemangler::DeserializeFunction(std::stringstream& SR) {
  if (DM.DTD0.Ty == ASTTypeExtern) {
    SR << "extern " << DM.TD.Name << '(';
  } else {
    SR << "def " << DM.TD.Name << '(';
  }

  std::stringstream RSS;

  for (unsigned I = 0; I < VDMT.size(); ++I) {
    if (const ASTDemangled* DMP = VDMT.at(I)) {
      switch (DMP->TD.Ty) {
      case ASTTypeBool:
      case ASTTypeInt:
      case ASTTypeUInt:
      case ASTTypeFloat:
      case ASTTypeDouble:
      case ASTTypeLongDouble:
      case ASTTypeChar:
      case ASTTypeUTF8:
      case ASTTypeDuration:
      case ASTTypeLength:
      case ASTTypeMeasure:
      case ASTTypeQubit:
      case ASTTypeReset:
      case ASTTypeBox:
      case ASTTypeBoxAs:
      case ASTTypeBoxTo:
      case ASTTypeStretch:
      case ASTTypeOpenPulseFrame:
      case ASTTypeOpenPulsePort:
        SR << TDMM[DMP->TD.Ty].Token();
        break;
      case ASTTypeExtern:
        continue;
        break;
      case ASTTypeResult:
        SR << " -> ";
        break;
      case ASTTypeReturn:
        switch (DMP->DTD0.Ty) {
        case ASTTypeMPInteger:
        case ASTTypeMPDecimal:
        case ASTTypeMPComplex:
        case ASTTypeAngle:
        case ASTTypeQubitContainer:
        case ASTTypeQubitContainerAlias:
          RSS << "-> " << TDMM[DMP->DTD0.Ty].Token()
            << '[' << DMP->DTD0.UBits << ']';
          break;
        default:
          RSS << "-> " << TDMM[DMP->DTD0.Ty].Token();
          break;
        }
        continue;
        break;
      case ASTTypeQubitContainer:
      case ASTTypeQubitContainerAlias:
        if (DMP->TD.UBits == 1U) {
          SR << TDMM[DMP->TD.Ty].Token();
        } else {
          SR << TDMM[DMP->TD.Ty].Token() << '[' << DMP->TD.UBits << ']';
        }
        break;
      default:
        SR << TDMM[DMP->TD.Ty].Token() << '[' << DMP->TD.UBits << ']';
        if (!DMP->TD.Name.empty())
          SR << ' ' << DMP->TD.Name;
        break;
      }

      if (I < VDMT.size() - 1) {
        if (const ASTDemangled* DMT = VDMT.at(I + 1)) {
          (void) DMT;
          SR << ", ";
        }
      }
    }
  }

  SR << ") " << RSS.rdbuf();
}

void ASTDemangler::DeserializeGate(std::stringstream& SR) {
  SR << TDMM[DM.TD.Ty].Token() << ' ';
  SR << DM.TD.Name;
  bool LP = false;
  bool RP = false;

  for (unsigned I = 0; I < VDMT.size(); ++I) {
    if (const ASTDemangled* DMP = VDMT.at(I)) {
      if (!ASTUtils::Instance().IsQubitType(DMP->TD.Ty)) {
        if (!LP) {
          SR << '(';
          LP = true;
        }

        switch (DMP->TD.Ty) {
        case ASTTypeBool:
        case ASTTypeInt:
        case ASTTypeUInt:
        case ASTTypeFloat:
        case ASTTypeDouble:
        case ASTTypeLongDouble:
        case ASTTypeChar:
        case ASTTypeUTF8:
        case ASTTypeDuration:
        case ASTTypeLength:
        case ASTTypeMeasure:
        case ASTTypeReset:
        case ASTTypeBox:
        case ASTTypeBoxAs:
        case ASTTypeBoxTo:
        case ASTTypeStretch:
        case ASTTypeOpenPulseFrame:
        case ASTTypeOpenPulsePort:
          SR << TDMM[DMP->TD.Ty].Token();
          break;
        case ASTTypeGateParam:
          continue;
          break;
        case ASTTypeQubit:
        case ASTTypeQubitContainer:
        case ASTTypeQubitContainerAlias:
          if (DMP->TD.UBits == 1U) {
            SR << TDMM[DMP->TD.Ty].Token();
          } else {
            SR << TDMM[DMP->TD.Ty].Token() << '[' << DMP->TD.UBits << ']';
          }
          break;
        case ASTTypeGateAngleParam:
          SR << TDMM[ASTTypeAngle].Token() << '[' << DMP->TD.UBits << ']'
            << ' ' << DMP->TD.Name;
          break;
        case ASTTypeGateQubitParam:
          SR << DMP->TD.Name;
          break;
        default:
          SR << TDMM[DMP->TD.Ty].Token() << '[' << DMP->TD.UBits << ']';
          if (!DMP->TD.Name.empty())
            SR << ' ' << DMP->TD.Name;
          break;
        }

        if (I < VDMT.size() - 1) {
          if (const ASTDemangled* DMN0 = VDMT.at(I + 1)) {
            if (DMN0->TD.Ty == ASTTypeGateParam) {
              if (const ASTDemangled* DMN1 = VDMT.at(I + 2)) {
                if (ASTUtils::Instance().IsQubitParamType(DMN1->TD.Ty)) {
                  if (!RP) {
                    SR << ") ";
                    RP = true;
                  } else {
                    SR << ", ";
                  }
                } else {
                  SR << ", ";
                  continue;
                }
              }
            }
          }
        }
      } else {
        if (ASTStringUtils::Instance().IsIndexedQubit(DMP->TD.Name)) {
          SR << ASTStringUtils::Instance().BracketedQubit(DMP->TD.Name);
        } else {
          SR << ' ' << DMP->TD.Name;
        }

        if (I < VDMT.size() - 1) {
          if (const ASTDemangled* DMN = VDMT.at(I + 1)) {
            (void) DMN;
            SR << ',';
          }
        }
      }
    }
  }
}

void ASTDemangler::DeserializeKernel(std::stringstream& SR) {
  SR << "extern " << DM.TD.Name << '(';

  std::stringstream RSS;

  for (unsigned I = 0; I < VDMT.size(); ++I) {
    if (const ASTDemangled* DMP = VDMT.at(I)) {
      switch (DMP->TD.Ty) {
      case ASTTypeBool:
      case ASTTypeInt:
      case ASTTypeUInt:
      case ASTTypeFloat:
      case ASTTypeDouble:
      case ASTTypeLongDouble:
      case ASTTypeChar:
      case ASTTypeUTF8:
      case ASTTypeDuration:
      case ASTTypeLength:
      case ASTTypeMeasure:
      case ASTTypeReset:
      case ASTTypeBox:
      case ASTTypeBoxAs:
      case ASTTypeBoxTo:
      case ASTTypeStretch:
      case ASTTypeResult:
      case ASTTypeOpenPulseFrame:
      case ASTTypeOpenPulsePort:
        SR << TDMM[DMP->TD.Ty].Token();
        break;
      case ASTTypeExtern:
        continue;
        break;
      case ASTTypeReturn:
        switch (DMP->DTD0.Ty) {
        case ASTTypeMPInteger:
        case ASTTypeMPDecimal:
        case ASTTypeMPComplex:
        case ASTTypeAngle:
        case ASTTypeQubitContainer:
        case ASTTypeQubitContainerAlias:
          RSS << "-> " << TDMM[DMP->DTD0.Ty].Token()
            << '[' << DMP->DTD0.UBits << ']';
          break;
        default:
          RSS << "-> " << TDMM[DMP->DTD0.Ty].Token();
          break;
        }
        continue;
        break;
      case ASTTypeQubit:
      case ASTTypeQubitContainer:
      case ASTTypeQubitContainerAlias:
        if (DMP->TD.UBits == 1U) {
          SR << TDMM[DMP->TD.Ty].Token();
        } else {
          SR << TDMM[DMP->TD.Ty].Token() << '[' << DMP->TD.UBits << ']';
        }
        break;
      default:
        SR << TDMM[DMP->TD.Ty].Token() << '[' << DMP->TD.UBits << ']';
        if (!DMP->TD.Name.empty())
          SR << ' ' << DMP->TD.Name;
        break;
      }

      if (I < VDMT.size() - 1) {
        if (const ASTDemangled* DMT = VDMT.at(I + 1)) {
          (void) DMT;
          SR << ", ";
        }
      }
    }
  }

  SR << ") " << RSS.rdbuf();
}

void ASTDemangler::DeserializeDurationOf(std::stringstream& SR,
                                         const ASTDemangled& DMG) {
  bool NC = false;
  bool EI = false;
  unsigned APC = 0U;
  ASTType ITy;

  switch (DMG.DTD1.Ty) {
  case ASTTypeGate:
  case ASTTypeGateCall:
  case ASTTypeDefcal:
  case ASTTypeDefcalCall:
  case ASTTypeDefcalGroup: {
    NC = true;
    SR << " { " << DMG.DTD1.Name;
    if (VDMT.size() && !ASTUtils::Instance().IsQubitParamType(VDMT[0]->TD.Ty))
      SR << '(';

    std::vector<ASTDemangled*>::const_iterator B;
    std::vector<ASTDemangled*>::const_iterator N;

    for (std::vector<ASTDemangled*>::const_iterator I = VDMT.begin();
         I != VDMT.end() - 1; ++I) {
      switch ((*I)->TD.Ty) {
      case ASTTypeDuration:
      case ASTTypeDurationOf:
        continue;
        break;
      case ASTTypeGateArg:
      case ASTTypeGateParam:
      case ASTTypeDefcalArg:
      case ASTTypeDefcalParam:
        ITy = (*I)->TD.Ty;
        N = I;
        continue;
        break;
      case ASTTypeBinaryOp:
        B = N = I;
        while (N != VDMT.end() && (*N)->TD.Ty != ITy) {
          ++N;
          if (!(*N) || N == VDMT.end()) {
            EI = true;
            break;
          } else if ((*N)->TD.Ty == ITy) {
            EI = false;
            I = N;
            break;
          }
        }

        if (!EI) {
          if (APC) {
            SR << ", ";
            DeserializeBinaryOp(SR, B, N);
            ASTStringUtils::Instance().EraseLastIfBlank(SR);
          } else {
            DeserializeBinaryOp(SR, B, N);
            ASTStringUtils::Instance().EraseLastIfBlank(SR);
          }

          APC += 1U;
          I = N;
          B = N + 1;
          if ((*B) && ASTUtils::Instance().IsQubitParamType((*B)->TD.Ty)) {
            SR << ") ";
            APC = 0U;
          }
        }
        break;
      case ASTTypeUnaryOp:
        B = N = I;
        while (N != VDMT.end() && (*N)->TD.Ty != ITy) {
          ++N;
          if (!(*N) || N == VDMT.end()) {
            EI = true;
            break;
          } else if ((*N)->TD.Ty == ITy) {
            EI = false;
            I = N;
            break;
          }
        }

        if (!EI) {
          if (APC) {
            SR << ", ";
            DeserializeUnaryOp(SR, B, N);
            ASTStringUtils::Instance().EraseLastIfBlank(SR);
          } else {
            DeserializeUnaryOp(SR, B, N);
            ASTStringUtils::Instance().EraseLastIfBlank(SR);
          }

          APC += 1U;
          I = N;
          B = N + 1;
          if ((*B) && ASTUtils::Instance().IsQubitParamType((*B)->TD.Ty)) {
            SR << ") ";
            APC = 0U;
          }
        }
        break;
      default:
        if (ASTUtils::Instance().IsNumericType((*I)->TD.Ty)) {
          if (APC)
            SR << ", " << (*I)->TD.SVal;
          else
            SR << (*I)->TD.SVal;
        } else {
          if (APC)
            SR << ", " << (*I)->TD.Name;
          else
            SR << (*I)->TD.Name;
        }

        APC += 1U;
        break;
      }
    }

    SR << ';';
    if (NC)
      SR << " } ";
  }
    break;
  case ASTTypeGateArg:
  case ASTTypeGateParam:
  case ASTTypeDefcalArg:
  case ASTTypeDefcalParam:
    break;
  default:
    SR << DMG.DTD1.Name;
    break;
  }
}

void ASTDemangler::DeserializeDurationOf(std::stringstream& SR) {
  DeserializeDurationOf(SR, DM);
}

void ASTDemangler::DeserializeScalar(std::stringstream& SR,
                                     const ASTDemangled& DMG) {
  SR << TDMM[DMG.TD.Ty].Token();

  switch (DMG.TD.Ty) {
  case ASTTypeBool:
  case ASTTypeChar:
  case ASTTypeUTF8:
  case ASTTypeInt:
  case ASTTypeUInt:
  case ASTTypeFloat:
  case ASTTypeDouble:
  case ASTTypeLongDouble:
    if (!DMG.TD.Name.empty())
      SR << ' ' << DMG.TD.Name;
    break;
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
  case ASTTypeMPDecimal:
    SR << '[' << DMG.TD.UBits << ']';
    if (!DMG.TD.Name.empty())
      SR << ' ' << DMG.TD.Name;
    if (!DMG.TD.SVal.empty())
      SR << ' ' << DMG.TD.SVal;
    break;
  default:
    break;
  }
}

void ASTDemangler::DeserializeScalar(std::stringstream& SR) {
  DeserializeScalar(SR, DM);
}

void ASTDemangler::DeserializeNonScalar(std::stringstream& SR,
                                        const ASTDemangled& DMG) {
  switch (DMG.TD.Ty) {
  case ASTTypeMPInteger:
  case ASTTypeMPUInteger:
  case ASTTypeMPDecimal:
    SR << TDMM[DMG.TD.Ty].Token() << '[' << DMG.TD.UBits << ']';
    if (!DMG.TD.Name.empty())
      SR << ' ' << DMG.TD.Name;
    if (!DMG.TD.SVal.empty())
      SR << ' ' << DMG.TD.SVal;
    break;
  case ASTTypeMPComplex:
    SR << TDMM[DMG.TD.Ty].Token() << '[' << TDMM[DMG.DTD0.Ty].Token();

    if (DMG.DTD0.Ty == ASTTypeFloat && DMG.DTD0.UBits > 32U)
      SR << '[' << DMG.DTD0.UBits << ']';
    else if (DMG.DTD0.Ty == ASTTypeDouble && DMG.DTD0.UBits > 64U)
      SR << '[' << DMG.DTD0.UBits << ']';
    else
      SR << '[' << DMG.DTD0.UBits << ']';

    SR << ']';

    if (!DMG.TD.Name.empty())
      SR << ' ' << DMG.TD.Name;

    if (!VDMT.empty()) {
      SR << " =";
      for (std::vector<ASTDemangled*>::const_iterator DI = VDMT.begin();
           DI != VDMT.end() - 1; ++DI) {
        switch ((*DI)->TD.Ty) {
        case ASTTypeInt:
        case ASTTypeUInt:
        case ASTTypeFloat:
        case ASTTypeDouble:
        case ASTTypeLongDouble:
        case ASTTypeMPInteger:
        case ASTTypeMPUInteger:
        case ASTTypeMPDecimal:
          SR << ' ' << (*DI)->TD.SVal;
          break;
        case ASTTypeOpTy:
          SR << ' ' << (*DI)->TD.Name;
          break;
        case ASTTypeImaginary:
          SR << ' ' << (*DI)->TD.Name << ';';
          break;
        default:
          break;
        }
      }
    } else {
      if (DMG.DTD0.Name == "creal")
        SR << ' ' << DMG.DTD0.SVal << ' '
          << PrintOpTypeOperator(DMG.TD.OpTy);

      if (DMG.DTD1.Name == "cimag")
        SR << ' ' << DMG.DTD1.SVal << " im";
    }
    break;
  case ASTTypeAngle:
    SR << TDMM[DMG.TD.Ty].Token();
    if (DMG.TD.UBits == static_cast<unsigned>(~0x0))
      SR << '[' << ASTAngleNode::AngleBits << ']';
    else
      SR << '[' << DMG.TD.UBits << ']';
    if (!DMG.TD.Name.empty())
      SR << ' ' << DMG.TD.Name;
    if (!DMG.DTD0.Name.empty() && !DMG.DTD0.SVal.empty() &&
        DMG.TD.Name == DMG.DTD0.Name)
      SR << " = " << DMG.DTD0.SVal;
    break;
  case ASTTypeBitset:
    SR << TDMM[DMG.TD.Ty].Token();
    if (DMG.TD.UBits > 1)
      SR << '[' << DMG.TD.UBits << ']';
    if (!DMG.TD.Name.empty())
      SR << ' ' << DMG.TD.Name;
    break;
  case ASTTypeGateControl:
    SR << TDMM[DMG.TD.Ty].Token() << ' ';
    break;
  case ASTTypeGateInverse:
    SR << TDMM[DMG.TD.Ty].Token() << ' ';
    break;
  case ASTTypeGateNegControl:
    SR << TDMM[DMG.TD.Ty].Token() << ' ';
    break;
  case ASTTypeGatePower:
    SR << TDMM[DMG.TD.Ty].Token() << ' ';
    break;
  case ASTTypeQubit:
    SR << TDMM[DMG.TD.Ty].Token();
    if (DMG.TD.IX != static_cast<unsigned>(~0x0))
      SR << '[' << DMG.TD.IX << ']';
    SR << ' ';
    break;
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias:
    SR << TDMM[DMG.TD.Ty].Token();
    if (DMG.TD.IX != static_cast<unsigned>(~0x0))
      SR << '[' << DMG.TD.IX << ']';
    else if (DMG.TD.UBits > 1)
      SR << '[' << DMG.TD.UBits << ']';
    SR << ' ';
    break;
  case ASTTypeGateQubitParam:
    SR << DMG.TD.Name;
    if (DMG.TD.IX != static_cast<unsigned>(~0x0))
      SR << '[' << DMG.TD.IX << ']';
    else if (DMG.TD.UBits > 1)
      SR << '[' << DMG.TD.UBits << ']';
    SR << ' ';
    break;
  case ASTTypeBoundQubit:
    if (ASTStringUtils::Instance().IsIndexedQubit(DMG.TD.Name)) {
      SR << ASTStringUtils::Instance().GetBaseQubitName(DMG.TD.Name)
        << '[' << ASTStringUtils::Instance().GetQubitIndex(DMG.TD.Name)
        << ']';
    } else {
      SR << DMG.TD.Name;
      if (DMG.TD.IX != static_cast<unsigned>(~0x0))
        SR << '[' << DMG.TD.IX << ']';
      else if (DMG.TD.UBits > 1)
        SR << '[' << DMG.TD.UBits << ']';
    }
    SR << ' ';
    break;
  case ASTTypeUnboundQubit:
    if (ASTStringUtils::Instance().IsIndexedQubit(DMG.TD.Name)) {
      SR << ASTStringUtils::Instance().GetBaseQubitName(DMG.TD.Name.substr(1))
        << '[' << ASTStringUtils::Instance().GetQubitIndex(DMG.TD.Name)
        << ']';
    } else {
      SR << DMG.TD.Name.substr(1);
      if (DMG.TD.IX != static_cast<unsigned>(~0x0))
        SR << '[' << DMG.TD.IX << ']';
      else if (DMG.TD.UBits > 1)
        SR << '[' << DMG.TD.UBits << ']';
    }
    SR << ' ';
    break;
  case ASTTypeDelay:
    if (DMG.DTD0.Ty == ASTTypeStretch ||
        DMG.DTD0.Ty == ASTTypeDuration ||
        DMG.DTD0.Ty == ASTTypeDurationOf) {
      SR << TDMM[DMG.TD.Ty].Token() << '[' << DMG.DTD0.Name << ']' << ' ';
    } else {
      SR << TDMM[DMG.TD.Ty].Token() << '[' << DMG.DTD1.UIVal << DMG.DTD1.Name
        << ']' << ' ';
    }

    if (!VDMT.empty()) {
      for (unsigned I = 0; I < VDMT.size() - 1; ++I) {
        if (const ASTDemangled* DMT = VDMT.at(I)) {
          if (ASTStringUtils::Instance().IsIndexedQubit(DMG.TD.Name)) {
            SR << ASTStringUtils::Instance().GetBaseQubitName(DMG.TD.Name)
              << '[' << ASTStringUtils::Instance().GetQubitIndex(DMG.TD.Name)
              << ']';
          } else {
            SR << DMT->TD.Name;
          }

          if ((DMT = VDMT.at(I + 1)))
            SR << ", ";
        }
      }
    }
    break;
  case ASTTypeDuration:
    SR << TDMM[DMG.TD.Ty].Token() << ' ' << DMG.TD.Name << " = ";
    if (DMG.DTD0.Ty == ASTTypeTimeUnit) {
      SR << DMG.DTD0.Name << std::endl;
    } else if (DMG.DTD0.Ty == ASTTypeDurationOf) {
      SR << DMG.DTD0.Name << '(';
      DeserializeDurationOf(SR);
      SR << ");";
    } else if (DMG.DTD0.Ty == ASTTypeBinaryOp) {
      DeserializeBinaryOp(SR);
    } else if (DMG.DTD0.Ty == ASTTypeUnaryOp) {
      DeserializeUnaryOp(SR);
    }
    break;
  case ASTTypeDurationOf:
    DeserializeDurationOf(SR);
    break;
  case ASTTypeBox:
  case ASTTypeBoxAs:
    SR << TDMM[DMG.TD.Ty].Token() << ' ' << DMG.TD.Name;
    break;
  case ASTTypeBoxTo:
    SR << TDMM[DMG.TD.Ty].Token() << ' ';
    if (DMG.DTD0.Ty == ASTTypeTimeUnit) {
      SR << DMG.DTD0.Name;
    } else if (DMG.DTD0.Ty == ASTTypeDurationOf) {
      SR << DMG.DTD0.Name << '(';
      DeserializeDurationOf(SR);
      SR << ");";
    }
    break;
  case ASTTypeBarrier:
    break;
  case ASTTypeInputModifier:
    break;
  case ASTTypeOutputModifier:
    break;
  case ASTTypeLength:
  case ASTTypeLengthOf:
    break;
  case ASTTypeStretch:
    break;
  case ASTTypeReset:
    break;
  case ASTTypeMeasure:
    break;
  case ASTTypeGPhaseExpression:
    break;
  case ASTTypeOpenPulseFrame:
    break;
  case ASTTypeOpenPulsePlay:
    break;
  case ASTTypeOpenPulsePort:
    break;
  case ASTTypeOpenPulseWaveform:
    break;
  default:
    break;
  }
}

void ASTDemangler::DeserializeNonScalar(std::stringstream& SR) {
  DeserializeNonScalar(SR, DM);
}

void
ASTDemangler::DeserializeBinaryOp(std::stringstream& SR,
                                  std::vector<ASTDemangled*>::const_iterator B,
                                  std::vector<ASTDemangled*>::const_iterator E) {
  for (std::vector<ASTDemangled*>::const_iterator I = B; I != E; ++I) {
    const std::string& Op = (*I)->TD.Name;

    switch ((*I)->TD.Ty) {
    case ASTTypeBinaryOp:
      break;
    case ASTTypeUnaryOp:
      break;
    case ASTTypeOpTy:
      switch ((*I)->TD.OpTy) {
      case ASTOpTypePreInc:
      case ASTOpTypePreDec:
      case ASTOpTypePostInc:
      case ASTOpTypePostDec:
        SR << PrintOpTypeOperator((*I)->TD.OpTy, Op) << ' ';
        break;
      case ASTOpTypeBinaryLeftFold:
      case ASTOpTypeUnaryLeftFold:
        SR << PrintOpTypeOperator((*I)->TD.OpTy);
        break;
      case ASTOpTypeBinaryRightFold:
      case ASTOpTypeUnaryRightFold:
        ASTStringUtils::Instance().Backtrack(SR);
        SR << PrintOpTypeOperator((*I)->TD.OpTy) << ' ';
        break;
      default:
        SR << PrintOpTypeOperator((*I)->TD.OpTy) << ' ';
        break;
      }
      break;
    case ASTTypeBool:
      SR << TDMM[(*I)->TD.Ty].Token() << ' ';
      break;
    case ASTTypeChar:
      SR << TDMM[(*I)->TD.Ty].Token() << ' ';
      break;
    case ASTTypeUTF8:
      SR << TDMM[(*I)->TD.Ty].Token() << ' ';
      break;
    case ASTTypeInt:
      if ((*I)->TD.L) {
        if ((*I)->TD.SBits == 32)
          SR << (*I)->TD.SIVal << ' ';
        else if ((*I)->TD.SBits == 64)
          SR << (*I)->TD.SLVal << ' ';
      } else {
        if (!(*I)->TD.Name.empty())
          SR << (*I)->TD.Name << ' ';
        else
          SR << TDMM[(*I)->TD.Ty].Token() << ' ';
      }
      break;
    case ASTTypeUInt:
      if ((*I)->TD.L) {
        if ((*I)->TD.UBits == 32U)
          SR << (*I)->TD.UIVal << ' ';
        else if ((*I)->TD.UBits == 64U)
          SR << (*I)->TD.ULVal << ' ';
      } else {
        if (!(*I)->TD.Name.empty())
          SR << (*I)->TD.Name << ' ';
        else
          SR << TDMM[(*I)->TD.Ty].Token() << ' ';
      }
      break;
    case ASTTypeLong:
      if ((*I)->TD.L)
        SR << (*I)->TD.SLVal << ' ';
      else {
        if (!(*I)->TD.Name.empty())
          SR << (*I)->TD.Name << ' ';
        else
          SR << TDMM[(*I)->TD.Ty].Token() << ' ';
      }
      break;
    case ASTTypeULong:
      if ((*I)->TD.L)
        SR << (*I)->TD.ULVal << ' ';
      else {
        if (!(*I)->TD.Name.empty())
          SR << (*I)->TD.Name << ' ';
        else
          SR << TDMM[(*I)->TD.Ty].Token() << ' ';
      }
      break;
    case ASTTypeFloat:
      if ((*I)->TD.L)
        SR << (*I)->TD.FVal << ' ';
      else {
        if (!(*I)->TD.Name.empty())
          SR << (*I)->TD.Name << ' ';
        else
          SR << TDMM[(*I)->TD.Ty].Token() << ' ';
      }
      break;
    case ASTTypeDouble:
      if ((*I)->TD.L)
        SR << (*I)->TD.DVal << ' ';
      else {
        if (!(*I)->TD.Name.empty())
          SR << (*I)->TD.Name << ' ';
        else
          SR << TDMM[(*I)->TD.Ty].Token() << ' ';
      }
      break;
    case ASTTypeLongDouble:
      if ((*I)->TD.L)
        SR << (*I)->TD.DVal << ' ';
      else {
        if (!(*I)->TD.Name.empty())
          SR << (*I)->TD.Name << ' ';
        else
          SR << TDMM[(*I)->TD.Ty].Token() << ' ';
      }
      break;
    case ASTTypeMPInteger:
      if ((*I)->TD.L)
        SR << (*I)->TD.SVal << ' ';
      else
        DeserializeScalar(SR, *(*I));
      break;
    case ASTTypeMPUInteger:
      if ((*I)->TD.L)
        SR << (*I)->TD.SVal << ' ';
      else
        DeserializeScalar(SR, *(*I));
      break;
    case ASTTypeMPDecimal:
      if ((*I)->TD.L)
        SR << (*I)->TD.SVal << ' ';
      else
        DeserializeScalar(SR, *(*I));
      break;
    case ASTTypeMPComplex:
      if ((*I)->TD.L)
        SR << (*I)->TD.SVal << ' ';
      else
        DeserializeScalar(SR, *(*I));
      break;
    case ASTTypeAngle:
    case ASTTypeBitset:
    case ASTTypeGateControl:
    case ASTTypeGateInverse:
    case ASTTypeGateNegControl:
    case ASTTypeGatePower:
    case ASTTypeQubit:
    case ASTTypeQubitContainer:
    case ASTTypeQubitContainerAlias:
    case ASTTypeGateQubitParam:
    case ASTTypeBoundQubit:
    case ASTTypeUnboundQubit:
    case ASTTypeDelay:
    case ASTTypeDuration:
    case ASTTypeDurationOf:
    case ASTTypeBox:
    case ASTTypeBoxAs:
    case ASTTypeBoxTo:
    case ASTTypeBarrier:
    case ASTTypeInputModifier:
    case ASTTypeOutputModifier:
    case ASTTypeLength:
    case ASTTypeLengthOf:
    case ASTTypeStretch:
    case ASTTypeReset:
    case ASTTypeMeasure:
    case ASTTypeGPhaseExpression:
    case ASTTypeOpenPulseFrame:
    case ASTTypeOpenPulsePlay:
    case ASTTypeOpenPulsePort:
    case ASTTypeOpenPulseWaveform:
      SR << (*I)->TD.Name << ' ';
      break;
    default:
      break;
    }
  }

  // Erase last character (' ') from stringstream.
  ASTStringUtils::Instance().EraseLastIfBlank(SR);
}

void
ASTDemangler::DeserializeBinaryOp(std::stringstream& SR) {
  std::vector<ASTDemangled*>::const_iterator B = VDMT.begin();
  std::vector<ASTDemangled*>::const_iterator E = VDMT.end();
  --E;
  DeserializeBinaryOp(SR, B, E);
}

void
ASTDemangler::DeserializeUnaryOp(std::stringstream& SR,
                                 std::vector<ASTDemangled*>::const_iterator B,
                                 std::vector<ASTDemangled*>::const_iterator E) {
  unsigned AC = 0U;
  for (std::vector<ASTDemangled*>::const_iterator I = B; I != E; ++I) {
    switch ((*I)->TD.Ty) {
    case ASTTypeGateArg:
    case ASTTypeGateParam:
    case ASTTypeDefcalArg:
    case ASTTypeDefcalParam:
    case ASTTypeFuncArg:
    case ASTTypeFuncParam:
    case ASTTypeKernelArg:
    case ASTTypeKernelParam:
      break;
    case ASTTypeBinaryOp:
    case ASTTypeUnaryOp:
      break;
    default:
      if (ASTUtils::Instance().IsNumericType((*I)->TD.Ty)) {
        if (!(*I)->TD.SVal.empty()) {
          if (AC)
            SR << ", " << (*I)->TD.SVal;
          else
            SR << (*I)->TD.SVal;
        } else {
          if (AC)
            SR << ", " << (*I)->TD.Name;
          else
            SR << (*I)->TD.Name;
        }

        AC += 1U;
      } else if ((*I)->TD.OpTy == ASTOpTypeUnaryLeftFold ||
                 (*I)->TD.OpTy == ASTOpTypeUnaryRightFold) {
        SR << (*I)->TD.Name;
      } else {
        SR << (*I)->TD.Name;
      }
      break;
    }
  }
}

void ASTDemangler::DeserializeUnaryOp(std::stringstream& SR) {
  std::vector<ASTDemangled*>::const_iterator B = VDMT.begin();
  std::vector<ASTDemangled*>::const_iterator E = VDMT.end();
  --E;
  DeserializeUnaryOp(SR, B, E);
}

void ASTDemangler::DeserializeAggregate(std::stringstream& SR) {
  // FIXME: IMPLEMENT.
  // Avoid compiler warnings - for now.
  (void) SR;
}

void ASTDemangler::DeserializeQubit(std::stringstream& SR) {
  switch (DM.TD.Ty) {
  case ASTTypeQubit:
    SR << TDMM[DM.TD.Ty].Token();
    if (DM.TD.IX != static_cast<unsigned>(~0x0))
      SR << '[' << DM.TD.IX << ']';
    SR << ' ' << DM.TD.Name;
    break;
  case ASTTypeQubitContainer:
  case ASTTypeQubitContainerAlias:
    SR << TDMM[ASTTypeQubit].Token();
    if (DM.TD.IX != static_cast<unsigned>(~0x0))
      SR << '[' << DM.TD.IX << ']';
    else if (DM.TD.UBits > 1)
      SR << '[' << DM.TD.UBits << ']';
    SR << ' ' << DM.TD.Name;
    break;
  case ASTTypeGateQubitParam:
    SR << DM.TD.Name;
    if (DM.TD.IX != static_cast<unsigned>(~0x0))
      SR << '[' << DM.TD.IX << ']';
    else if (DM.TD.UBits > 1)
      SR << '[' << DM.TD.UBits << ']';
    SR << ' ' << DM.TD.Name;
    break;
  case ASTTypeBoundQubit:
    if (ASTStringUtils::Instance().IsIndexedQubit(DM.TD.Name)) {
      SR << ASTStringUtils::Instance().GetBaseQubitName(DM.TD.Name)
        << '[' << ASTStringUtils::Instance().GetQubitIndex(DM.TD.Name)
        << ']';
    } else {
      SR << DM.TD.Name;
      if (DM.TD.IX != static_cast<unsigned>(~0x0))
        SR << '[' << DM.TD.IX << ']';
      else if (DM.TD.UBits > 1)
        SR << '[' << DM.TD.UBits << ']';
    }
    SR << ' ';
    break;
  case ASTTypeUnboundQubit:
    if (ASTStringUtils::Instance().IsIndexedQubit(DM.TD.Name)) {
      SR << ASTStringUtils::Instance().GetBaseQubitName(DM.TD.Name.substr(1))
        << '[' << ASTStringUtils::Instance().GetQubitIndex(DM.TD.Name)
        << ']';
    } else {
      SR << DM.TD.Name.substr(1);
      if (DM.TD.IX != static_cast<unsigned>(~0x0))
        SR << '[' << DM.TD.IX << ']';
      else if (DM.TD.UBits > 1)
        SR << '[' << DM.TD.UBits << ']';
    }
    SR << ' ';
    break;
  default:
    break;
  }
}

void ASTDemangler::DeserializeComplex(std::stringstream& SR) {
  // FIXME: IMPLEMENT.
  // Avoid compiler warnings - for now.
  (void) SR;
  // Avoid compiler warnings - for now.
}

void ASTDemangler::Print() const {
  DM.Print();

  if (DTDM.HasType())
    DTDM.Print();

  if (!VDMT.empty()) {
    for (std::vector<ASTDemangled*>::const_iterator I = VDMT.begin();
         I != VDMT.end(); ++I) {
      if ((*I)) {
        if (IsParamOrArgument((*I)->TD.Ty)) {
          for (std::vector<ASTDemangled*>::const_iterator J =
               (*I)->DTV.begin(); J != (*I)->DTV.end(); ++J)
            if (*J) (*J)->Print();
        } else {
          (*I)->Print();
        }
      }
    }
  }
}

void ASTDemangler::DeserializeArray(std::stringstream& SR, ASTType Ty) {
  switch (Ty) {
  case ASTTypeBoolArray:
    SR << "array[bool, " << DM.TD.UBits << "] "
      << DM.TD.Name;
    break;
  case ASTTypeAngleArray:
    SR << "array[angle[" << DM.DTD0.UBits << "], "
      << DM.TD.UBits << "] " << DM.TD.Name;
    break;
  case ASTTypeCBitArray:
    SR << "array[bit";
    if (DM.DTD0.UBits > 1)
      SR << '[' << DM.DTD0.UBits << ']';
    SR << ", " << DM.TD.UBits << "] " << DM.TD.Name;
    break;
  case ASTTypeDurationArray:
    if (DM.DTD0.Ty == ASTTypeDuration) {
      SR << "array[duration[" << DM.DTD1.SVal << "], "
        << DM.TD.UBits << "] " << DM.TD.Name;
    } else if (DM.DTD0.Ty == ASTTypeDurationOf) {
      SR << "array[durationof(?)";
    }
    break;
  case ASTTypeFloatArray:
    SR << "array[float, " << DM.TD.UBits << "] "
      << DM.TD.Name;
    break;
  case ASTTypeIntArray:
    SR << "array[int, " << DM.TD.UBits << "] "
      << DM.TD.Name;
    break;
  case ASTTypeUIntArray:
    SR << "array[uint, " << DM.TD.UBits << "] "
      << DM.TD.Name;
    break;
  case ASTTypeMPComplexArray:
    SR << "array[complex[";
      if (DM.DTD1.Ty != ASTTypeUndefined) {
        SR << TDMM[DM.DTD1.Ty].Token() << '[' << DM.DTD1.UBits
          << "]], " << DM.TD.UBits << "] " << DM.TD.Name;
      }
    break;
  case ASTTypeMPDecimalArray:
    SR << "array[float[" << DM.DTD0.UBits << "], "
      << DM.TD.UBits << "] " << DM.TD.Name;
    break;
  case ASTTypeMPIntegerArray:
    SR << "array[int[" << DM.DTD0.UBits << "], "
      << DM.TD.UBits << "] " << DM.TD.Name;
    break;
  case ASTTypeMPUIntegerArray:
    SR << "array[uint[" << DM.DTD0.UBits << "], "
      << DM.TD.UBits << "] " << DM.TD.Name;
    break;
  case ASTTypeOpenPulseFrameArray:
    SR << "array[frame, " << DM.TD.UBits << "] "
      << DM.TD.Name;
    break;
  case ASTTypeOpenPulsePortArray:
    SR << "array[port, " << DM.TD.UBits << "] "
      << DM.TD.Name;
    break;
  case ASTTypeQubitArray:
    SR << "array[qubit";
    if (DM.DTD0.UBits > 1)
      SR << '[' << DM.DTD0.UBits << ']';
    SR << ", " << DM.TD.UBits << "] " << DM.TD.Name;
    break;
  default:
    break;
  }
}

std::unique_ptr<DMGate>
ASTDemangler::Gate(const std::string& N) {
  // FIXME: IMPLEMENT.
  (void) N;
  std::unique_ptr<DMGate> GP = std::unique_ptr<DMGate>();
  return GP;
}

std::unique_ptr<DMDefcal>
ASTDemangler::Defcal(const std::string& N) {
  std::unique_ptr<DMDefcal> DP = std::unique_ptr<DMDefcal>();

  if (Demangle(N)) {
    DP->Name = DM.TD.Name;
  }

  return DP;
}

void ASTDemangledRegistry::Release() {
  std::vector<const ASTDemangled*> VDM;
  VDM.insert(VDM.begin(), RS.begin(), RS.end());

  for (unsigned I = 0; I < VDM.size(); ++I) {
    delete VDM[I];
  }
}

void ASTDemangler::PrintVDMT(const std::vector<ASTDemangled*>& V) const {
  if (!V.empty()) {
    unsigned X = 0U;
    std::cerr << __PRETTY_FUNCTION__ << "-----------------------------" << std::endl;
    for (std::vector<ASTDemangled*>::const_iterator I = V.begin();
         I != V.end(); ++I) {
      if (*I) {
        std::cerr << __PRETTY_FUNCTION__ << ": [" << X << "]: TD.Ty="
          << PrintTypeEnum((*I)->TD.Ty) << " TD.OpTy="
          << PrintOpTypeEnum((*I)->TD.OpTy) << " TD.Name=" << (*I)->TD.Name
          << " TD.SVal=" << (*I)->TD.SVal << std::endl;
      }

      ++X;
    }
    std::cerr << __PRETTY_FUNCTION__ << "-----------------------------" << std::endl;
  }
}

} // namespace QASM

