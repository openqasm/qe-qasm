OPENQASM 3.0;

include "stdgates.inc";

// 7. gate commutation and merging

qubit q;
qubit r;
angle[3] c = 0;
reset q;
reset r;
h r;
h q;
cx q, r;
phase(-3*pi/16) r;
cx q, r;
phase(3*pi/16) r;
phase(3*pi/16) q;
h q;
measure q -> c[0];
c <<= 1;
reset q;
h q;
cx q, r;
phase(-3*pi/8) r;
cx q, r;
phase(3*pi/8) r;
phase(3*pi/8-c) q;
h q;
measure q -> c[0];
c <<= 1;
reset q;
h q;
cx q, r;
phase(-6*pi/8) r;
cx q, r;
phase(6*pi/8) r;
phase(6*pi/8-c) q;
h q;
measure q -> c[0];
c <<= 1;

