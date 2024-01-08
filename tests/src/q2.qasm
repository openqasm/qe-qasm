OPENQASM 2.0;

include "qelib1.inc";

qreg qubits[9];

ccx qubits[0],qubits[1],qubits[5];
