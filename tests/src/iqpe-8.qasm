OPENQASM 3.0;

include "stdgates.inc";

// 8. convert angles to 32-bit fixed point mod 2*pi

qubit q;
qubit r;
angle[3] c = 0;
reset q;
reset r;
h r;
h q;
cx q, r;
phase(1.8125*pi) r;  // mod 2*pi
cx q, r;
phase(0.1875*pi) r;
phase(0.1875*pi) q;
h q;
measure q -> c[0];
c <<= 1;
reset q;
h q;
cx q, r;
phase(1.625*pi) r;  // mod 2*pi
cx q, r;
phase(0.375*pi) r;
phase(0.375*pi-c) q;
h q;
measure q -> c[0];
c <<= 1;
reset q;
h q;
cx q, r;
phase(1.25*pi) r;  // mod 2*pi
cx q, r;
phase(0.75*pi) r;
phase(0.75*pi-c) q;
h q;
measure q -> c[0];
c <<= 1;
