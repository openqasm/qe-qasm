OPENQASM 3.0;

bit[32] bs;

for uint i in [0 : 4] {
  bs |= i;
}
