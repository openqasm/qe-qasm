OPENQASM 3.0;

uint j = 32;
uint k = 128;
uint y = 16;

array[angle, j] xa;

angle xaa = xa[2];

angle xbb = xa[y];

array[angle[k], j] xb;

angle[k] ybb = xb[y];

angle[128] zbb = xb[8];

angle[96] tbb = xb[16];
