OPENQASM 3.0;

uint q = 14;

uint r = q << 1;
uint s = q + r;

r = s >> 1;
s = q & r;
