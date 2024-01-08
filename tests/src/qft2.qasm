OPENQASM 3.0;

include "stdgates.inc";

// Quantum Fourier transform

qubit[4] q;
bit[4] cb;

reset q;

x q[0];
x q[2];
barrier q;
h q[0];
cphase(pi / 2) q[1], q[0];
h q[1];
cphase(pi / 4) q[2], q[0];
cphase(pi / 2) q[2], q[1];
h q[2];
cphase(pi / 8) q[3], q[0];
cphase(pi / 4) q[3], q[1];
cphase(pi / 2) q[3], q[2];
h q[3];

measure q -> cb;
