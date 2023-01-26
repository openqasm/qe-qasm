OPENQASM 3.0;

const uint n = 3;
qubit q;
angle[n] c = 0;
uint[n] power = 1;

for i in [0 : n << 1] {
  measure q -> c[0];
  c <<= 1;
  power <<= 1;
}

