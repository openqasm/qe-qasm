# Bosonic OpenQASM Syntax Reference

This guide describes the current user-facing syntax for qubit and
bosonic/qumode programs in the extended OpenQASM frontend. It is
intended for scientists writing programs and experiments rather than
developers modifying the parser.

## 1. Basic Program Structure

A program begins with the OpenQASM version declaration.

``` qasm
OPENQASM 3.0;
```

To use the bosonic gate definitions provided by the project:

``` qasm
include "cvgates.inc";
```

When running the parser, provide the include directory with `-I`:

``` bash
./bin/QasmParser -I ../../tests/include ../../tests/src/qumode/example.qasm
```

The preprocessor searches the directories supplied with `-I` for
included files.

------------------------------------------------------------------------

## 2. Qubits

A `qubit` represents a two-level quantum system.

### Single qubit

``` qasm
qubit qb;
```

### Qubit register

``` qasm
qubit[3] qb;
```

This declares three addressable qubits:

``` qasm
qb[0]
qb[1]
qb[2]
```

### Applying gates to qubits

Standard qubit gates use normal OpenQASM gate syntax:

``` qasm
qubit qb;

x qb;
z qb;
h qb;
```

For a qubit register, an individual qubit can be selected by index:

``` qasm
qubit[3] qb;

x qb[0];
z qb[1];
h qb[2];
```

Two-qubit gates can operate on selected qubits:

``` qasm
qubit[2] qb;

cx qb[0], qb[1];
```

Qubit indices select individual qubits in a register.

------------------------------------------------------------------------

## 3. Qumodes

A `qumode` represents a bosonic mode.

### Single qumode

``` qasm
qumode qm;
```

### Qumode register

``` qasm
qumode[3] qm;
```

This declares three addressable qumodes:

``` qasm
qm[0]
qm[1]
qm[2]
```

A qumode index selects a **mode**, not a Fock level.

For example:

``` qasm
disp(0.5) qm[1];
```

means "apply the displacement gate to the second qumode."

It does **not** mean "apply displacement to Fock level 1."

------------------------------------------------------------------------

## 4. Displacement Gate --- `disp`

`disp` applies a displacement operation to a qumode.

### Syntax

``` qasm
disp(alpha) qm;
```

or, for a specific mode in a register:

``` qasm
disp(alpha) qm[index];
```

`alpha` may be complex.

### Examples

``` qasm
qumode qm;

disp(0.5) qm;
```

``` qasm
qumode[2] qm;

disp(0.3 + 0.5 im) qm[0];
```

The target of `disp` must be a qumode.

------------------------------------------------------------------------

## 5. SNAP Gate --- `snap`

The SNAP gate applies independently specified phases to Fock levels of a
qumode.

The current gate definition is parameterized by the number of supplied
phase angles:

``` qasm
gate snap[uint N](array[angle, N] thetas) qumode qm {
    for i in [0:N] {
        ctrl[i] @ gphase(thetas[i]) qm;
    }
}
```

### Syntax

``` qasm
snap[N]([theta0, theta1, ..., thetaN-1]) qm;
```

### Example

``` qasm
qumode qm;

snap[3]([pi/2, 0, pi/3]) qm;
```

For a qumode register:

``` qasm
qumode[2] qm;

snap[3]([pi/2, 0, pi/3]) qm[1];
```

Here:

-   `qm[1]` selects **which qumode** receives the SNAP gate.
-   `[pi/2, 0, pi/3]` supplies the phase angles associated with the SNAP
    operation's Fock-level controls.
-   `3` specifies the size of the angle array.

Do not use the qumode index to represent a Fock level.

------------------------------------------------------------------------

## 6. Unitary Type

A `unitary` represents a user-defined unitary operation.

### Declaration

``` qasm
unitary u;
```

### Declaration with matrix initialization

``` qasm
unitary v = {{1, 0}, {0, 1}};
```

### Applying a unitary to a qubit

``` qasm
qubit qb;
unitary u;

u qb;
```

A register element can also be selected:

``` qasm
qubit[2] qb;
unitary u;

u qb[0];
```

### Applying a unitary to a qumode

``` qasm
qumode qm;
unitary u;

u qm;
```

or:

``` qasm
qumode[2] qm;
unitary u;

u qm[1];
```

The extended frontend supports unitary operands for both qubits and
qumodes.

------------------------------------------------------------------------

## 7. Bumper States

Bumper states are associated with a `unitary` and are used when
synthesizing the unitary in a larger bosonic Hilbert space.

Bumper information is specified using attributes on the unitary.

### Set a bumper value

``` qasm
u.bumper = 4;
```

### Add another bumper value

``` qasm
u.bumper += 5;
```

For example:

``` qasm
unitary u;

u.bumper = 4;
u.bumper += 5;
```

### Reset the bumper specification

A new assignment replaces the previous bumper specification:

``` qasm
u.bumper = 2;
```

### Set a maximum number of bumper states

``` qasm
u.bumper_max = 3;
```

### Example

``` qasm
unitary u;

u.bumper = 4;
u.bumper += 5;
u.bumper_max = 3;
```

The available syntax is:

``` qasm
u.bumper = value;
u.bumper += value;
u.bumper_max = value;
```

If no bumper attribute is supplied, bumper-state selection is left to
the later compilation/synthesis stage.

------------------------------------------------------------------------

## 8. Qubit vs. Qumode Operations

Qubits and qumodes are different operand types. Operations should be
applied to the type they are defined to accept.

  Operation         Qubit   Qumode
  --------------- ------- --------
  `x`                 Yes       No
  `z`                 Yes       No
  `h`                 Yes       No
  `cx` / `cnot`       Yes       No
  `disp`               No      Yes
  `snap`               No      Yes
  `unitary`           Yes      Yes

Examples:

``` qasm
qubit qb;
qumode qm;
unitary u;

x qb;
disp(0.5) qm;
snap[3]([pi/2, 0, pi/3]) qm;

u qb;
u qm;
```

------------------------------------------------------------------------

## 9. Register Indexing

Qubit and qumode registers use similar indexing syntax, but the indices
mean different things.

### Qubit register

``` qasm
qubit[3] qb;
```

Then:

``` qasm
qb[0]
qb[1]
qb[2]
```

select individual qubits.

### Qumode register

``` qasm
qumode[3] qm;
```

Then:

``` qasm
qm[0]
qm[1]
qm[2]
```

select individual bosonic modes.

A qumode index does **not** select a Fock level.

For example:

``` qasm
snap[3]([pi/2, 0, pi/3]) qm[1];
```

selects `qm[1]` as the target mode. The SNAP parameters determine the
Fock-level-dependent phases.

------------------------------------------------------------------------

## 10. Complete Example

``` qasm
OPENQASM 3.0;

include "cvgates.inc";

qubit[2] qb;
qumode[2] qm;

unitary u = {{1, 0}, {0, 1}};

u.bumper = 4;
u.bumper += 5;

x qb[0];
z qb[1];

disp(0.3 + 0.5 im) qm[0];

snap[3]([pi/2, 0, pi/3]) qm[1];

u qb[0];
u qm[1];
```

Run the program with the include directory supplied to the parser:

``` bash
./bin/QasmParser -I ../../tests/include ../../tests/src/qumode/example.qasm
```

------------------------------------------------------------------------

## 11. Quick Reference

``` qasm
// Qubit
qubit qb;
qubit[3] qb;

// Qubit register indexing
qb[0];

// Standard qubit gates
x qb;
z qb;
h qb;
cx qb[0], qb[1];

// Qumode
qumode qm;
qumode[3] qm;

// Qumode register indexing
qm[0];

// Displacement
disp(0.5) qm;
disp(0.3 + 0.5 im) qm[0];

// SNAP
snap[3]([pi/2, 0, pi/3]) qm;
snap[3]([pi/2, 0, pi/3]) qm[1];

// Unitary declaration
unitary u;
unitary v = {{1, 0}, {0, 1}};

// Unitary application
u qb;
u qm;

// Bumper attributes
u.bumper = 4;
u.bumper += 5;
u.bumper = 2;
u.bumper_max = 3;
```

## Notes

This document describes the current user-facing syntax of the extended
OpenQASM frontend. Later compiler stages may impose additional semantic
or hardware-specific restrictions on unitary synthesis, bumper-state
allocation, and physical gate execution.
