OPENQASM 3.0;

angle[64] a = pi;

bit[64] ba = bit[64](a);

float[64] fa = float[64](a);

bit[128] bb = "0";

angle[64] b = angle[64](bb);

int[128] i = 15;

b = angle[64](i);

i = int[128](ba);

int[64] k = int[64](a);
