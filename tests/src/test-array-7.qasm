OPENQASM 3.0;

uint j = 32;
uint k = 128;
uint y = 16;

array[float, j] fa;

float faa = fa[2];

float fab = fa[y];

array[float[k], j] fb;

float[k] fbb = fb[y];

float[128] fcb = fb[10];
