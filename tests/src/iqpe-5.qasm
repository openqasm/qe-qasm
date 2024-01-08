OPENQASM 3.0;

include "stdgates.inc";

qubit q;
qubit r;
angle[3] c = 0;
reset q;
reset r;
h r;
reset q;          // <-- another pass could optimize and remove this line
h q;
cphase(3 * pi / 8) q, r;
phase(0 * pi) q;    // <-- another pass could optimize and remove this line
h q;
measure q -> c[0];
c <<= 1;
reset q;
h q;
cphase(6 *pi / 8) q, r;
phase(-c) q;
h q;
measure q -> c[0];
c <<= 1;
reset q;
h q;
cphase(12 * pi / 8) q, r;
phase(-c) q;
h q;
measure q -> c[0];
c <<= 1;
