OPENQASM 3.0;

uint j = 32;
uint k = 128;
uint y = 16;

array[int, j] xa;

int xaa = xa[2];

int xbb = xa[y];

array[int[k], j] xb;

int[k] ybb = xb[y];

int[128] zbb = xb[8];
