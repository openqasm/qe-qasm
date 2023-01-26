OPENQASM 3.0;

include "stdgates.inc";

qubit[8] qq;
qubit[8] rr;
qubit[8] ss;

// Block 1
cx rr[0], rr[1];
h qq[0];

// Block 2
h ss[0];
barrier rr, qq[0];
h ss[0];

// Block 3
cx rr[1], rr[0];
cx rr[0], rr[1];

