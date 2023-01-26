OPENQASM 3.0;

bit[2] c;

gate cx c, t { }

qubit $0;
qubit $1;

cx $0, $1;

c[0] = measure $0;

