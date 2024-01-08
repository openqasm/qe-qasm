OPENQASM 3.0;

bit[20] b;

qubit $0;

int c;
int d;
int e;

b[0] = measure $0;

if (!b[0]) {
  c = 3;
  d = c - 1;
  e = c + d;
} else {
  c = 4;
  d = c * 2;
  e = c * d;
}
