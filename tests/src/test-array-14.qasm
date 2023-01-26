OPENQASM 3.0;

uint j = 32;
uint k = 128;
uint y = 16;

array[duration[100ns], j] xa;

duration[100ns] xaa = xa[2];

duration[100ns] xbb = xa[y];

array[duration[250ms], j] xb;

duration[250ms] ybb = xb[y];

duration[250ms] zbb = xb[8];

duration[100ns] tbb = xa[16];

