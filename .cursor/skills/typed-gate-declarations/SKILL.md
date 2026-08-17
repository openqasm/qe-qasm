---
name: typed-gate-declarations
description: >-
  Fully-typed OpenQASM gate declarations (qubit/qumode formals + explicit
  classical types) and call-site type checking in this fork. Use when editing
  gate grammar, ProductionRule_10030, CreateGateCall, ValidateTypedGateCall,
  ASTGateType, cvgates.inc, ECD/disp/SNAP typing, or when adding typed gate
  call checks.
---

# Typed gate declarations

## Design rules

- **All-untyped** classical + bare quantum formals = legacy OpenQASM 3 (`ProductionRule_1430` / `1431`). No call-site type checks.
- **Any typed classical formals** with **typed quantum operands** = fully typed (`ProductionRule_10030`). No partial typing.
- **Templates (v1):** `gate foo[uint N](array[T, N] …)` — unsigned int templates used as array lengths. Call-site `foo([…])` infers `N` from literal arity or a sized named array’s declared length; `foo[N]([…])` checks. Uninitialized named arrays → “used before assigned.” Call Gate `TemplateParams` carry `<Value>` (NaN on decl, bound int on call); body `N` is not overwritten.
- **Fock ctrl:** `ctrl[3]` / `ctrl[N]` / `ctrl[N-1]` / `negctrl[i]` — level may be int, id, or binary/unary (or parenthesized) expression. Distinct from qubit `ctrl(n)`. See `docs/gates.md`.
- **Gate-body `for`:** `GateForStmt` + nested `GateOpList` → `ASTGateForOpNode` (`ASTTypeGateFor`). SNAP in `cvgates.inc` is `array[angle, N]` with `for i in [0:N] { ctrl[i] @ gphase(thetas[i]) qm; }`.
- **Compiler call ABI:** see `docs/gates.md` § Frozen call-node contract — `disp` / `snap` / `ecd` call fields only; decls and `GateQOpList` are unstable.
- `ctrl` is a reserved token (`TOK_CTRL`); do not use `ctrl` as an operand name.
- **Out of CV-core scope:** `while` in `GateOpList`; int/bool/duration/bit array formals; non-`uint` / non-size template params.

## Grammar / production

| Piece | Location |
|-------|----------|
| Typed quantum list | `GateTypedQuantumOperandList` in `lib/Parser/QasmParser.y` |
| Decl production | `ProductionRule_10030` in `ASTProductionFactory.cpp` |
| Template register | `ProductionRule_10031` + `ASTGateTemplateParamBuilder` |
| Shared AST build | Delegates to `ProductionRule_1430` after validating formals |
| Call entry | `ProductionRule_3500` → `CreateQOpNodeCall` → `CreateGateCall` |
| Compatibility | `ASTGateType` in `include/qasm/AST/ASTGateType.h` |

Decl-time checks in `10030`:
- Classical: `IsExplicitClassicalGateParamType` (includes explicit `angle`,
  float/complex/arrays; bare Identifier formals are also angles and allowed
  on this path when quantum operands are typed)
- Quantum: polymorphic type must be `ASTTypeQubit` or `ASTTypeQumode`
- Stores `FormalParamTypes`, `FormalParamArraySizes`,
  `FormalParamArraySizeTemplateIndices`, `TemplateParams`, `FormalQuantumTypes`

## GateType lattice (call-site)

`FormalParamTypes[i]` (+ optional size) is the type oracle. Carriers (`Params` / `ArrayParams` / `ComplexArrayParams` / `ComplexParams`) are storage only.

`ASTGateType::Compatible(F, A)`:
- Real↔angle array family (`AngleArray` / `FloatArray` / `MPDecimalArray`)
- Real→complex scalar and real-array→complex-array promotion
- Reject complex→real (scalar or array)
- Array size equality when both sides known

Hook: `ASTTypeDiscovery::ValidateTypedGateCall` from `CreateGateCall` **before** `CloneCall`, only if `GN->IsFullyTyped()`.

## AST storage (definition gate)

| Formal kind | Storage | Read for checking |
|-------------|---------|-------------------|
| `complex[…]` | `Params` as `MPComplex` | Prefer `FormalParamTypes[i]` on fully-typed gates |
| `float` / `double` / `mpdecimal` scalars | `Params` as declared type (type-faithful) | Prefer `FormalParamTypes[i]` |
| other non-float scalars (int/bool/…) | still coerced to angle `.gateparam` in `Params` | Must use `FormalParamTypes` |
| `array[float/mpdecimal, N]` | Declared STE in GSTM + same node in `Params` (no `.gatearray`) | `FormalParamTypes` + size |
| `array[angle, N]` | `Params` as `AngleArray` | `FormalParamTypes` + size |
| `array[complex[…], N]` | `Params` as `MPComplexArray` | `FormalParamTypes` + size |
| qubit / qumode | `OperandParams` STE; polymorphic type restored | Prefer `FormalQuantumTypes[i]` |
| `Operands` vector | synthetic GateQubitParam-shaped nodes | **Do not** use for qubit vs qumode |

Array formals share one ctor path in `ASTGates.cpp`: erase LSTM, bind declared STE into GSTM, then `AddParam` with the declared array type (float/mpdecimal/angle/complex).

## Call-site Params (Phase 2)

`ArgumentNodeList` ctor keeps `Params` type-faithful when `FormalParamTypes` say so:

- `FormalWantsComplex` → `MPComplex` (unchanged)
- `FormalWantsFloatArray` + `ProductionRule_10010` AngleArray literal → convert to `MPDecimalArray` (`ast-gate-mpdecimal-array-lit-*`)
- `FormalWantsFloatScalar` → keep float/double/mpdecimal (no angle coercion)
- Angle / complex-array formals → store as AngleArray / MPComplexArray
- Untyped OQ3 calls → still angle-shaped Params

## Literals (`ProductionRule_10010`)

`ASTGateType::ExpressionListHasComplex` decides complex vs angle-array literal. Call-site and builder share that classification. Real array literals stay AngleArray at 10010; float formals convert at call materialization.

Gate call `ArgsList` is `'(' ExprList ')'`. Array literals are `ExprList`
elements (`[ExprList]`), so `foo([a,b], theta)` works alongside `… im`
complex initializers and nested function calls. Do not reintroduce a
separate `GateCallArg` list — it LR-conflicts with function-call parsing.

## Do not

- Check untyped OQ3 gates.
- Treat “has `ComplexParams` alone” as fully typed (classical-only typing + bare qubits is not the fully-typed form).
- Leave stray `std::cerr` / `std::cout` debug prints in production paths.

## Array formals (SNAP-like)

Gate param lists provisionally type identifiers as `ASTTypeAngle`. `ProductionRule_822` must **rebind** Angle/Undefined → array type and allow formals in gate/function contexts (`AllowArrayInCurrentContext` / `IsGateParameterArgument`). Do not leave hard `assert(Id->GetSymbolType() == Ty)` — that SIGABRTs on `array[float[64], N] thetas`.

`GateOpList` includes `for` (`GateForStmt`). After an `array[…]` formal,
`SetCurrentType(ForStatement)` must win over leftover `PreviousType` array
state or induction vars are typed as arrays.

## Tests

- Positive: `tests/src/qumode/gate-for-fock.qasm` (gate-body `for` + Fock `ctrl[i]` + `thetas[i]`)
- Positive: `tests/src/qumode/ecd-mixed.qasm` (`ecd(0.5) qb, qm`)
- Positive: `tests/src/qumode/complex-array-param.qasm` (`array[complex[…], N]` formal + call)
- Positive: `tests/src/qumode/float-array-param.qasm` (`array[float[64], N]` body index + call)
- Positive: `tests/src/qumode/angle-array-param.qasm` (`array[angle, N]` body index + multi-gate ASTM reuse)
- Positive: `tests/src/qumode/multi-angle-array-param.qasm` (two angle arrays, different `N`)
- Positive: `tests/src/qumode/angle-scalar-param.qasm` (`angle` scalar + float array formals)
- Positive: `tests/src/qumode/gate-template-array-param.qasm` (`uint N` template + infer/`[3]`)
- Negative (expect-fail `test $? -ne 0`):
  - `ecd-param-reject.qasm` — angle array for complex formal
  - `ecd-operand-reject.qasm` — qumode where qubit expected
  - `ecd-operand-order-reject.qasm` — swapped qubit/qumode
  - `disp-qubit-reject.qasm` — builtin disp pattern
  - `array-elem-complex-reject.qasm` — complex element in float array formal
  - `array-size-mismatch-reject.qasm` — literal length ≠ formal `N` (param 0)
  - `array-size-mismatch-param1-reject.qasm` — size mismatch on second array formal
  - `gate-template-size-mismatch-reject.qasm` — `foo[2]([a,b,c])`
  - `gate-template-infer-fail-reject.qasm` — uninitialized named array (`used before assigned`)
  - `gate-template-conflict-reject.qasm` — conflicting inferred `N` from two arrays
  - `gate-template-unused-reject.qasm` — `N` not used as an array size (cannot infer)
- Include: `tests/include/cvgates.inc`
- Manual: `gate-decl.qasm` (`rz3` + `disp3`)
