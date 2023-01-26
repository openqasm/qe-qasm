OPENQASM 3.0;

include "stdgates.inc";

qubit q[8];

// Simple Qubit identifiers.
// 0. $[a-zA-Z_][0-9]+
measure $0 -> q[0];
measure $1 -> q[1];

// More complex Qubit identifiers.
// 1. $[a-zA-Z_][0-9]+
measure %q1 -> q[2];

// 2. $[a-zA-Z_][0-9]+
measure $q2 -> q[3];

// 3. $[0-9]+
measure %2 -> q[2];

// 4. cb[N] = measure qubit[N].
qubit qb[8];
cbit cb[8];
cb = measure qb;

// 5. measure qubit[N] -> cb[N].
qubit qbc[4];
cbit cbc[4];

measure qbc[0] -> cbc[0];
measure qbc[2] -> cbc[2];
measure qbc[3] -> cbc[1];

