OPENQASM 3.0;

include "stdgates.inc";

// 6. basis translation

// gate phase(lambda) q { U(0, 0, lambda) q; }
// gate cx c, t { CX c, t; }
// gate h a { U(pi  /  2, 0, pi) a; }

qubit q;
qubit r;
angle[3] c = 0;
reset q;
reset r;
h r;
h q;
phase(3 * pi / 16) q;
cx q, r;
phase(-3 * pi / 16) r;
cx q, r;
phase(3 * pi / 16) r;
h q;
measure q -> c[0];
c <<= 1;
reset q;
h q;
phase(3 * pi / 8) q;
cx q, r;
phase(-3 * pi / 8) r;
cx q, r;
phase(3 * pi / 8) r;
phase(-c) q;
h q;
measure q -> c[0];
c <<= 1;
reset q;
h q;
phase(6 * pi / 8) q;
cx q, r;
phase(-6 * pi / 8) r;
cx q, r;
phase(6 * pi / 8) r;
phase(-c) q;
h q;
measure q -> c[0];
c <<= 1;

