OPENQASM 3.0;

bit[32] b = "00000001000000010000000100000001";

uint i = 3;

if (b[7] == 1) {
  i = 9;
} else {
  i = 0;
}
