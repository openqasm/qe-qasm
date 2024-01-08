OPENQASM 3.0;

int[32] a = 0;

for ii in [-12:2:0] {
  for jj in [0:11] {
    a += 1;
  }
}
