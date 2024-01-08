OPENQASM 3.0;

array[qubit, 16, 8] qa;

array[qubit[32], 16, 32] qb;

array[angle, 16, 8] aa;

array[angle[24], 16, 32] ab;

array[bool, 32, 16] ba;

array[bit, 32, 8] cba;

array[bit[64], 32, 16] cbb;

array[int, 8, 16] ia;

array[int[128], 16, 8] ib;

array[uint, 16, 8] ua;

array[uint[128], 8, 16] ub;

array[float, 8, 32] fa;

array[float[128], 8, 16] fb;

array[complex[float[64]], 16, 32] cxb;

array[duration[100ns], 12, 24] la;

array[frame, 8, 8] far;

array[port, 16, 8] par;

array[waveform, 32, 32] war;
