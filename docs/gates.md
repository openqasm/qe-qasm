# Gates

This document describes the syntax of qumode gates in our extension to
OpenQASM 3.0, as well as the limitations of the parser on these declarations.



# Basic Qumode Gates

At the moment, the following qumode gates are supported:
- displacement `disp(alpha) qm`
- SNAP: `snap([theta_0, theta_1, ...]) qm`
- ECD: `ecd(alpha) qb, qm`

## Displacement gate

Gate signature:
```qasm
gate disp(complex[float[64]] alpha) qumode qm;
```
Example Usage:
```qasm
disp(0.5 + 0.5im) qm;
disp(0.5) qm; // equivalent to disp(0.5 + 0.0im)
```
The displacement gate is treated as a builtin gate. The user doesn't need to
include any file to use this gate.

## SNAP gate

Gate definition (`tests/include/cvgates.inc`):
```qasm
gate snap[uint N](array[angle, N] thetas) qumode qm {
    for i in [0:N] {
        ctrl[i] @ gphase(thetas[i]) qm; // controls the i-th Fock level
    }
}
```
Example Usage:
```qasm
snap([pi/2, 0, 0.3]) qm; // infer N = 3
snap[3]([pi/2, 0, 0.3]) qm; // check length == 3
```
Include `tests/include/cvgates.inc` to use this gate. Call sites may omit the
template argument when `N` can be inferred from the array literal or a sized
named array (`snap([pi/2, 0, 0.3]) qm`), or pass it explicitly (`snap[3](…)`).

## ECD gate

Gate definition:
```qasm
gate ecd(complex[float[64]] alpha) qubit qb, qumode qm {
    negctrl @ disp(-alpha/2) qb, qm;
    ctrl @ disp(alpha/2) qb, qm;
}
```
Example Usage:
```qasm
ecd(0.5 + 0.5im) qb, qm;
ecd(0.5) qb, qm; // equivalent to ecd(0.5 + 0.0im) qb, qm;
```
Include `tests/include/cvgates.inc` to use this gate.



# Gate Declaration Syntax

These qumode gates introduce new types of parameters and operands. In
OpenQASM 3.0, all parameters and operands are of type `angle` and `qubit`
respectively. This is no longer the case in our extension. Therefore,
we introduce a typed gate declaration syntax that facilitates type checking at
the syntactic level.


Currently, the supported classical types for parameters are `angle`, `float[N]`,
`complex[float[N]]`, and fixed-length arrays of these types (including lengths
bound by template parameters). Type checking for typed gate declarations is
enforced both in definitions and in call sites.

## Examples for the new syntax

The main addition to the syntax is that the types of parameters and operands are
specified before the name of the parameter or operand (à la C). For example,
the following is a valid gate declaration:
```qasm
gate ecd(complex[float[64]] alpha) qubit c, qumode t {
    negctrl @ disp(-alpha/2) c, t;
    ctrl @ disp(alpha/2) c, t;
}
```

Arrays of `angle`s, `float[N]`s, or `complex[float[N]]`s can also be used as
parameters:
```qasm
gate foo(array[angle, 3] phases) qubit q {
    ctrl @ gphase(phases[0]) q;
    ctrl @ gphase(phases[1]) q;
    ctrl @ gphase(phases[2]) q;
}
gate bar(array[complex[float[64]], 3] alphas) qumode qm {
    disp(alphas[0]) qm;
    disp(alphas[1]) qm;
    disp(alphas[2]) qm;
}
```

It is also possible to mix and match different types of parameters and operands:
```qasm
gate baz(array[complex[float[64]], 3] alphas, angle beta) qubit qb, qumode qm {
    ctrl @ gphase(beta) qb;
    ecd(alphas[0]/2) qb, qm;
    ecd(alphas[1]/3) qb, qm;
    ecd(alphas[2]/4) qb, qm;
}
```

Arrays with variable lengths can also be used as parameters through
template parameters:
```qasm
gate foo[uint N](array[float[64], N] thetas) qubit q {
    ...
}
```


Call sites may bind `N` explicitly or omit it when it can be inferred from an
array literal’s arity or a **sized** named array’s declared length

```qasm
foo([pi/2, 0, 0.3]) q;   // infer N = 3
foo[3]([pi/2, 0, 0.3]) q; // check length == 3
```

Each template parameter must be uniquely determined by an array size at the
call. An uninitialized named array (declared but not assigned) is rejected with
a “variable used before assigned” diagnostic. v1 supports only `uint` templates
used as array sizes. Template parameters use square brackets (`gate foo[uint N]`
/ `foo[3](…)`), not angle brackets.

## Fock-level `ctrl[…]` / `negctrl[…]`

Distinct from qubit n-control `ctrl(n) @ …` (parentheses). Square brackets
select a **Fock level**:

```qasm
ctrl[3] @ gphase(pi/2) qm;
ctrl[N] @ gphase(pi/2) qm;   // N = gate template param or other int id
ctrl[N-1] @ gphase(pi/2) qm;
ctrl[(2*N)] @ gphase(pi/2) qm;  // parentheses ok for richer exprs
negctrl[i] @ disp(alpha) qm;
```

Existing forms are unchanged: `ctrl @`, `ctrl(2) @`, `negctrl @`, `negctrl(2) @`.

The level may be an integer literal, identifier, or a `+`/`-`/`*`/`/` expression
(including parentheses). Template parameters (`uint N`) are bound as gate-local
ints so `ctrl[N]` / `ctrl[N-1]` in a gate body resolve.

**Limitation:** Multi-control syntaxes `ctrl[level](n_ctrls)` are currently not
supported.

## Gate-body `for`

Gate bodies may contain `for` loops whose body is a list of gate operations
(range or integer list):

```qasm
gate snap[uint N](array[angle, N] thetas) qumode qm {
    for i in [0:N] {
        ctrl[i] @ gphase(thetas[i]) qm;
    }
}
```

Induction variables are gate-local ints. The body of the `for` loop is subject
to the same limitations as elsewhere in gate bodies. In particular, only
gate calls and `barrier`s are allowed in the body. Non-unitary operations, such
as `measure` and `reset`, are not allowed.

**Limitation:** While the parser can parse a forward traversal `for` loop, more
complicated loops may not parse correctly at the moment.
For example, the following two gate declarations will not parse:
```qasm
gate foo[uint N](array[angle, N] thetas) qubit q {
    for i in [0:N] {
        ctrl @ gphase(thetas[N - i - 1]) q;
    }
}
gate bar[uint N](array[angle, N] thetas) qubit q {
    for i in [0:(N/2)] {
        ctrl @ gphase(thetas[i]) q;
        ctrl @ gphase(thetas[N - i - 1]) q;
    }
}
```
This is in part due to the stock parser's defect in parsing expressions
(see [issue #12](https://github.com/paragon-lab/qe-qasm-qumode-version/issues/12)).
For now, we only support forward traversal, which is hopefully sufficient for
most use cases.

## Type Errors at Call Sites
Typed gate declarations allows the following errors to be caught at the
syntactic/semantic level:
```qasm
qubit[2] qb; qumode[2] qm;

ecd(0.5 + 0.5im) qb[0], qb[1]; // error, qb[1] is not a qumode
ecd(0.5 + 0.5im) qm[0], qm[1]; // error, qm[0] is not a qubit
snap(pi/2) qm[0]; // error, pi/2 is not an array of angles
snap[2]([pi/2, 0, 0.3]) qm[0]; // error, the array length is not 2

gate foo[uint N](array[angle, N] alphas, array[float[64], N] betas) qubit q {
    ...
}
foo([pi/2, 0, 0.3], [0.5, 0.5]) qb; // error, template parameter N cannot be inferred

gate bar[uint N](complex[float[64]] alpha) qumode qm {
   ...
}
bar(0.5 + 0.5im) qm; // error, template parameter N cannot be inferred
```

## Compatibility with OpenQASM 3.0 gate definitions

Since OpenQASM 3.0 is widely adopted, users may still want to use gate
definitions written in the untyped OpenQASM 3.0 syntax.
To facilitate this, the parser is backward compatible with
the `OpenQASM 3.0` gate declaration syntax. For example, consider this
declaration of the `rz` gate, which does not contain typing information:
```qasm
gate rz(theta) q {
    ctrl @ gphase(theta) q;
}
```
In this case, all parameters would be treated as `angle`s, and operands would be
treated as `qubit`s. That is, the above definition would be equivalent to the
following typed definition:
```qasm
gate rz(angle theta) qubit q {
    ctrl @ gphase(theta) q;
}
```
To avoid footguns, the untyped and typed gate declaration syntaxes cannot be
mixed with each other. As soon as one parameter or operand is typed, all other
parameters and operands must also be typed.
```qasm
gate ecd(complex[float[64]] alpha) qubit c, qumode t {...} // ok
gate ecd(alpha) qubit c, qumode t {...} // error, alpha is not typed
gate ecd(complex[float[64]] alpha) c, qumode t {...} // error, c is not typed
```



# Notes for developers

This section is for compiler developers. Users of the language can skip it.

## Syntax Change for Template Parameters

- **Square brackets for templates.** This matches how OpenQASM 3 already uses
  `[]` for sized types (`int[32]`, `array[int[32], 5]`).

  An earlier proposal used angle brackets (`gate foo<uint N>`), which proved
  to be problematic for parsing.
  For example, suppose we want to support expressions like `ctrl[N-1]` or
  `ctrl[3*2]`, we would have a rule like `TOK_CTRL '[' Expr ']' ...`.
  But `']'` can also be a part of `Expr`, making it very annoying to parse.
  Currently, we disallow expressions inside square brackets to avoid this
  problem. Granted, the user might not have an incentive to put expressions
  inside square brackets, but if they do, it would be hard to support that.
  This is why the current implementation uses square brackets for template
  parameters.
  It doesn't help that expression parsing in this repo is not very robust to
  begin with (see
  [issue #12](https://github.com/paragon-lab/qe-qasm-qumode-version/issues/12)).

- **Fock `ctrl[…]` vs qubit `ctrl(n)`.** An earlier proposal used `ctrl(n)`
  for Fock-level control, which conflicts with the existing   n-control form. For
  example `ctrl(2) @ x a, b, c;` already exists in OpenQASM 3.0 and is
  equivalent to a CCX gate. Therefore, for qumode Fock-level control, we use
  square brackets to avoid conflicts. We can view `ctrl[N]` as an analogy of
  the template syntax elsewhere in the language.

## AST Structure for Gate Declarations and Calls

### Frozen call-node contract (compiler-facing)

For **gate calls** (`GateCall=true`) of the shipped CV gates, treat the
following as the stable interface. Declaration syntax, `GateQOpList` bodies,
and mangled-name spelling are **not** part of this contract and may change.

| Gate | How to get it | Classical `Params` | Templates | Quantum |
|------|---------------|--------------------|-----------|---------|
| `disp` | builtin | one `MPComplex` (`alpha`; real promotes to real+0i) | none | one `qumode` (see note) |
| `snap` | `cvgates.inc` | one `AngleArray` (`thetas`) | `N` with bound `Value` = array length | one `qumode` |
| `ecd` | `cvgates.inc` | one `MPComplex` (`alpha`; real promotes) | none | `qubit`, then `qumode` |

**Note on `disp`:** As a builtin, call nodes may omit `FullyTyped` /
`FormalParamTypes` / `FormalQuantumTypes`. Identify by `Name=disp`, read
`Params` as `MPComplex`, and take the qumode from `OperandParams` (a controlled
`disp` in an `ecd` body can list the control qubit first — the target qumode is
still present). Typed `snap` / `ecd` calls always carry the Formal* fields.

**Read these fields on a call:**

1. `Name` / `GateDefinitionName` — which gate.
2. `FullyTyped` — must be `true` for the rows above.
3. `TemplateParams` — for `snap`, `Name=N` and `Value` is the bound size
   (inferred or explicit). Empty for `disp` / `ecd`.
4. `Params` — classical argument **values**. On fully typed calls,
   `Params[i]/Type` matches the formal classical type (`MPComplex`,
   `AngleArray`, …). Prefer this over re-deriving types from expressions.
5. `FormalQuantumTypes` — declared qubit vs qumode per operand slot
   (parallel to call operands).
6. `OperandParams` — call-site operand names / refs (`qb`, `qm`, `qm[0]`, …),
   same order as `FormalQuantumTypes`. Always present on typed CV calls.
7. `Operands` — when present, `Qubit` / `Qumode` nodes for those slots; may be
   absent on some call shapes. Do **not** require it; use (5)+(6).

**The Following Parts of the AST has not Stabilized**

- `GateQOpList` (body is shared with the definition; parameters are **not**
  substituted at the call site — intentional).
- `MangledName` spelling (useful for debug; not an ABI).
- Exact element node names under array literals (`ast-gate-…`).

Illegal calls are rejected by the parser; a well-typed program should not
present e.g. `ecd` with two qubits.

### The `<Gate/>` node

The AST representation of a gate declaration is enclosed in the
`<GateDeclarationNode/> --> <Gate/>` node. Inside the `<Gate/>` node, we have
- `<Name/>`: the name of the gate.
- `<MangledName/>`: the mangled name of the gate. The mangling logic resides in
  `lib/AST/ASTMangler.cpp`. Debug aid only for the call contract above.
- `<Opaque/>`: whether the gate is opaque. If the gate is opaque, the parser is
  told that the definition of the gate exists without actually seeing it. This
  option is deprecated and would typically be set to `false`.
- `<GateCall/>`: whether the gate is a gate call. This would be false at
  a gate declaration, and true at a gate call.
- `<FullyTyped/>`: whether the gate is fully typed. If the gate is fully typed,
   `<FormalParamTypes/>` and `<FormalQuantumTypes/>` would be present.
- `<TemplateParams/>`: unsigned template formals (`uint N` in `gate foo[uint N](…)`).
  Printed before classical `<Params/>` to match source order
  `gate name[N](…) qubits`. Each entry has `<Type/>`, `<Name/>`, and
  `<Value/>`. On a **declaration**, `<Value/>` is `NaN` (placeholder).
  On a **call**, `<Value/>` holds the explicit or inferred binding (e.g. `3`
  for `snap[3](…)` / inferred `snap([…])`). Body uses of `N` are **not**
  overwritten; `GateQOpList` may still mention symbolic `N`.
- `<FormalParamTypes/>`: declared classical types (and array size / size
  template metadata). Parallel to `<Params/>`. On fully typed calls,
  `Params[i]/Type` matches; keep Formal* for `ArraySize` /
  `ArraySizeTemplate` until that metadata moves onto Params.
- `<FormalQuantumTypes/>`: declared quantum operand kinds (qubit vs qumode).
- `<Params/>`: the parameters of the gate (value carrier).
  On **fully typed** definitions, float/mpdecimal scalars and
  `array[float|mpdecimal, N]` formals are stored with their declared types
  (no synthetic `.gatearray` angle view). Angle/complex formals match their
  declared types. Some other classical scalars (int/bool/…) may still appear
  as angle placeholders. On **fully typed** calls, `<Params/>` matches the
  formal type (float/mpdecimal arrays from real literals become
  `MPDecimalArray`; complex stays `MPComplex`; angle arrays stay
  `AngleArray`). Untyped OQ3 calls still coerce reals to angles.
- `<OperandParams/>`: quantum operand bindings (names / indexed refs). Printed
  for declarations and calls. Prefer this plus `<FormalQuantumTypes/>` for
  call-site quantum slots.
- `<Operands/>`: `Qubit` / `Qumode` nodes when materialized. Optional on some
  call shapes; see the frozen call contract.
  NOTE: In the original OpenQASM 3.0 parser, this field was called `<Qubits/>`.
- `<GateQOpList/>`: the list of operations in the gate body (may include
  gate-body `for` nodes). Unstable for compiler ABI when treating CV gates as
  basis operations.

### How Gate Declarations and Calls are Represented in the AST

For definitions, the parser sees a gate declaration and creates a
`<GateDeclarationNode/>` node with a `<Gate/>` node inside and populates the
fields accordingly.

For calls, this `<Gate/>` node would be enclosed in the
`<GenericGateOpNode/>/<GateOpNode/>`.

Loosely speaking, when the parser sees a gate call, it first checks if the
definition of the gate exists either as a builtin gate, a declared gate, or an
opaque gate.
- For builtin gates, the parser builds a `<Gate/>` for that gate.
- For declared and opaque gates, the parser clones the `<Gate/>` node from the
  definition, and replaces the fields in `<Params/>` and `<Operands/>` with
  the actual values from the call site. For templated gates it also fills
  `<TemplateParams/><Value/>` with the bound size(s); it does **not** rewrite
  body identifiers named `N`.
- A quirk is that the parser does **NOT** substitute the parameters and operands
  with the actual values in the gate body (the contents of `<GateQOpList/>`).
  The parameters in the body still need to be substituted with what the call
  site provides. It seems like this is left to the compiler.
  This does also mean that a gate call node can carry redundant information
  if the body is never used (e.g., if the gate is considered a basis gate).
