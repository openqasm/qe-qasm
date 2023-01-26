OPENQASM 3.0;

bit[32] bs;

int x = 3;

if (x < 4) {
  for i in [0 : 4] {
    bs |= i;
  }
}

