OPENQASM 3.0;

int b = 12;

uint c = 3;

return c;

uint[128] d = 1;

return d ^= c;

int a = 16;

qubit $0;

return measure $0;

return a + b;

return -(a + b);

return -a + b;

return c += b;

return d++;

return --a;

return d << 1;
