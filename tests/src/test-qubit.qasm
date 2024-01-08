OPENQASM 3.0;

bit[8] q;

qubit $0;
qubit $1;

qubit $q1;
qubit $q2;
qubit $q3;

// Simple Qubit identifiers.
// 0. $[a-zA-Z_][0-9]+
measure $0 -> q[0];
measure $1 -> q[1];

// More complex Qubit identifiers.
// 1. $[a-zA-Z_][0-9]+
measure $q1 -> q[2];

// 2. $[a-zA-Z_][0-9]+
measure $q2 -> q[3];

// 3. $[0-9]+
measure $q3 -> q[4];

// 4. cb[N] = measure qubit[N].
qubit[8] qb;
bit[8] cb;

cb = measure qb;

// 5. measure qubit[N] -> cb[N].
qubit[4] qbc;
bit[4] cbc;

measure qbc[0] -> cbc[0];
measure qbc[2] -> cbc[2];
measure qbc[3] -> cbc[1];
