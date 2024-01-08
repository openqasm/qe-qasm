OPENQASM 3.0;

include "stdgates.inc";

// Qiskit will provide physical qubits
qubit $0;
qubit $1;

// Array of classical bits is fine
bit c0;
bit c1;

U(pi / 2, 0.0, pi) $0;
cx $0, $1;

measure $0 -> c0;
measure $1 -> c1;
