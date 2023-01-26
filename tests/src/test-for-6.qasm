OPENQASM 3.0;

const int n = 64;

qubit $0;
qubit $1;

bit[16] c;

angle[n] a = 3.14159265358979323846;

uint[n] power = 1;

for i in [0 : n - 1] {
  reset $0;
  reset $1;

  measure $0 -> c[0];
  measure $1 -> c[1];

  if (c[0] == 1) {
    power <<= 1;
  } else {
    power <<= 3;
  }
}

