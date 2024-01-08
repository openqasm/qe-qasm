OPENQASM 3.0;

uint k = 32;
uint j = 16;
uint y = 8;

array[qubit[k], j] qb;

qubit[k] qbb = qb[y];
