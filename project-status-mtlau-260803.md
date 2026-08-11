# Project Status (2026-08-03)
Mu-Te Lau

This file documents the status of the project, as of 2026-08-03, focusing on the
tasks I'm in charge of.

# Stable Features
The following features work as expected:

- `qumode` declarations: mirrors qubit declaration syntaxes
  ```qasm
  qumode qm;
  qumode[3] qms;
  ```
- Basic qumode gates: displacement, SNAP, ECD gates
    - `disp(alpha) qm;`
    - `snap[N]([theta_0, theta_1, theta_2,...]) qm;`
    - `ecd(alpha) qb, qm;`
    - A type system is implemented along with the gate declaration syntaxes.
      CV gates (`disp` / `snap` / `ecd`) are no longer declared via `opaque`.
    - [docs/gates.md#type-errors-at-call-sites](docs/gates.md#type-errors-at-call-sites)
    lists the types of errors that the type system can catch.

The AST subtrees of qumodes and gate calls are considered stable and not
expected to change much. With these, I believe the compiler has enough data to
build the program level IR.


# Work-In-Progress Features
The following features are implemented, but the syntax and the ASTs are not stable yet.
- Gate declaration syntaxes: see [docs/gates.md#gate-declaration-syntax](docs/gates.md#gate-declaration-syntax)
  for more details.
    - Typed gate template parameters (`uint N` as array length) with call-site
    inference from array literals / declared named-array sizes and explicit
    `gatecall[N](…)`; uninitialized named arrays reject as used-before-assigned.
    - We proposed using angle brackets for template parameters. I decided to use
    square brackets instead to avoid parsing issues. See
    [docs/gates.md#syntax-change-for-template-parameters](docs/gates.md#syntax-change-for-template-parameters)
    for more details.
- Fock-level `ctrl[…]` / `negctrl[…]` (distinct from qubit `ctrl(n)`): level may
  be an integer, identifier, or expression (`ctrl[3]`, `ctrl[N]`, `ctrl[N-1]`);
  see [docs/gates.md#fock-level-ctrl-negctrl](docs/gates.md#fock-level-ctrl-negctrl).
- Gate-body `for` (`GateForStmt` / `ASTGateForOpNode`); SNAP is a real typed
  definition in `cvgates.inc`.

These features are implemented to support a stable gate call AST, as the
subtrees of gate declarations and gate calls are intimately related.
To be specific:
- The gate signature (names, template params, params, operands) is largely
  stable in terms of AST shapes, but the frontend syntax is not.
- Treat everything inside the gate body as unstable, including the syntax and the ASTs.

# Not Implemented: Gate-Level IR
Ended up not having time to implement it.
I decided that a proper type system and gate declaration syntax are
more important to implement first, since gate declarations and gate calls ASTs
are tightly related. Also, it would fortify the parser by allowing it to catch
type errors at call sites.

In addition, I believe the implementation of gate-level IR can be delayed.
For single-qumode programs, the DAG view that the gate-level IR would provide
is not useful yet. I believe the S+D Unitary decomposition pass can be applied
to the program level IR, and it wouldn't be too hard to migrate that pass to
the gate level IR after we have that.

# Known Issues
- Proposed array literal syntax conflicts with the OQ3 standard ([Issue #13](https://github.com/paragon-lab/qe-qasm-qumode-version/issues/13)).
- Several bugs/gaps in the codebase. All the bugs I have encountered and have
  not fixed yet are on the GitHub issues. Here are some of the more important
  ones:
  - Expression parsing is fragile and not reusable ([Issue #12](https://github.com/paragon-lab/qe-qasm-qumode-version/issues/12)).
  - Duplicate declarations are not guarded ([Issue #7](https://github.com/paragon-lab/qe-qasm-qumode-version/issues/7)).
  - Some AST components still use `Qubit` in names even though those paths also
    cover qumodes / generic quantum formals
    ([Issue #14](https://github.com/paragon-lab/qe-qasm-qumode-version/issues/14)).
  - Array literals cannot be assigned to a variable ([Issue #11](https://github.com/paragon-lab/qe-qasm-qumode-version/issues/11)).
- Unit tests only check whether the parser accepts/rejects input; they do not
  validate AST correctness. At some point we will probably want tests that
  check AST structure too.

# Some Other Notes
The things I'm not in charge of, but could be useful to note. Feel free to fix
these or post issues for them.

## Unitary / bumper feature gaps
- Lack of way to specify the precision of unitary decompositions
  (maybe `u.precision`?)
- What if `bumper` and `bumper_max` are simultaneously specified?
  ```qasm
  u.bumper = 4;
  u.bumper_max = 3; // outright contradictory
  ```
  ```qasm
  // which statement should the compiler honor? Should we just disallow this?
  u.bumper = 4;
  u.bumper_max = 5;
  ```

## Testing / tooling
- Unitary unit tests under `tests/src/unitary/` are not wired up to ctest. To
  wire them up, add tests in `tests/CMakeLists.txt`. I'd suggest adding both
  tests that should succeed and tests that should fail. You can find examples
  on how to do this in `tests/CMakeLists.txt`.
