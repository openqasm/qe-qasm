OPENQASM 3.0;

uint n = 9;

def parity(bit[n] cin) -> bit {
  bit c;
  for i in [0:2:64] {
    c ^= cin[i];
  }

  return c;
}

