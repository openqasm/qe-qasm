OPENQASM 3.0;

qubit $0;
qubit $1;

bit[2] a;

a[0] = measure $0;
a[1] = measure $1;

int i = a;
