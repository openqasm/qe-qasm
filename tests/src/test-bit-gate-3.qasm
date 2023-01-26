OPENQASM 3.0;

bit[2] c;

int i;

float[128] f;

gate zx c, f, t, i { }

qubit $0;
qubit $1;
qubit $2;
qubit $3;

zx $0, $1, $2, $3;

c[0] = measure $0;
c[1] = measure $3;

