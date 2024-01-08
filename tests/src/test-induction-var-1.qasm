OPENQASM 3.0;

bit[32] bs;

for i in [0 : 4] {
  bs |= i;
}
