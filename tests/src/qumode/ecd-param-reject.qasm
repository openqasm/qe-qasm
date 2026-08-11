OPENQASM 3.0;

include "cvgates.inc";

qubit[2] qb;
qumode[2] qm;

// Negative: classical arg must be complex-compatible, not an angle array.
ecd([pi/2, 0, 0.3]) qb[0], qm[0];
