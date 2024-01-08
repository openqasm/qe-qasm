OPENQASM 3.0;

uint j = 32;
uint k = 128;
uint y = 16;

array[bit, j] ba;

bit baa = ba[2];

bit bab = ba[y];

array[bit[k], j] bb;

bit[k] bbb = bb[y];

bit[128] bcb = bb[10];
