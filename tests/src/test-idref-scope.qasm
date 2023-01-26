OPENQASM 3.0;

bit[2] d;

qubit $0;

if (d[0] == 1) {
  d[1] = d[0];
}

d[1] = measure $0;

