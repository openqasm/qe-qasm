OPENQASM 3.0;

array[qubit, 16] qa;

array[qubit[32], 16] qb;

array[angle, 16] aa;

array[angle[24], 16] ab;

array[bool, 32] ba;

array[bit, 32] cba;

array[bit[64], 32] cbb;

array[int, 8] ia;

array[int[128], 16] ib;

array[uint, 16] ua;

array[uint[128], 8] ub;

array[float, 8] fa;

array[float[128], 8] fb;

array[duration[100ns], 12] la;

qubit[32] qbb = qb[0];

