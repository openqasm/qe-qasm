OPENQASM 3.0;

// This program has no meaning.
// It is only a syntax / AST Generator test.

gate phase(lambda) q {
  U(0.0, 0.0, lambda) q;
}

qubit q;
qubit r;

const uint n = 3;

bit[n] cb = 0;

float[n] theta = 2.0;

angle a = 0.00;

uint[n] power = 2;

ctrl @ pow(power) @ phase(theta) q;

inv @ phase(a) q;

ctrl @ inv @ phase(a) q;

measure q -> cb[0];
measure r -> cb[0];

