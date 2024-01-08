OPENQASM 3.0;

uint j = 16;
uint k = 32;
uint y = 8;

array[qubit, j] qa;

qubit qaa = qa[2];

array[qubit[k], j] qb;

qubit[k] qbb = qb[y];
