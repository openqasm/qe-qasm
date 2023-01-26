OPENQASM 3.0;

uint n = 32;

def parity(bit[n] cin) -> bit {
  bit c;
  for i in [0:2:100] {
    c ^= cin[i];
  }

  return c;
}

