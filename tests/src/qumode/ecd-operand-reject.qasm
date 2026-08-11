OPENQASM 3.0;

include "cvgates.inc";

qubit[2] qb;
qumode[2] qm;

// Negative: first operand must be qubit (got qumode).
ecd(0.3 + 0.5 im) qm[0], qm[1];
