OPENQASM 3.0;

include "stdgates.inc";

// Qiskit will provide physical qubits
qubit $0;
qubit $1;

// Array of classical bits is fine
bit[2] k;

h $0;
cx $0, $1;

measure $0 -> k[0];
measure $1 -> k[1];

