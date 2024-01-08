OPENQASM 2.0;

gate xz q { U(pi, 0.0, pi) q; }

gate cx c, t { CX c, t; }

qreg qubits[8];

xz qubits[0];

cx qubits[0], qubits[1];
