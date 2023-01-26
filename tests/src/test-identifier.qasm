OPENQASM 3.0;

uint m = 64;
uint n = 128;

array[float, m] af;

array[float[n], n] fi;

array[complex[float[128]], 64] jk;

array[complex[float[n]], 64] ca;

float[128] k = 15.0;

complex[float[128]] ck;

int i = 10;

extern port p0;

duration dr = 800dt;

qubit $0;

qubit q;

qubit[8] qq;

array[qubit, 32] aq;

array[qubit[8], 32] anq;

array[bit, m] ba;

array[bit[m], n] bba;

frame test_frame = newframe(p0, 5e9, 0.0);

