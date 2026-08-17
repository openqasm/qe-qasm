OPENQASM 3.0;

include "cvgates.inc";

qubit[2] qb;
qumode[2] qm;

// Negative: operand order must match (qubit, qumode).
ecd(0.3 + 0.5 im) qm[0], qb[0];
